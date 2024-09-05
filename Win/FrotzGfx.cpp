/////////////////////////////////////////////////////////////////////////////
// Windows Frotz
// Frotz graphics class
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Dib.h"
#include "FrotzApp.h"
#include "FrotzGfx.h"
#include "FrotzWnd.h"
#include "ScaleGfx.h"

#include <math.h>
#include <setjmp.h>
#pragma warning(disable : 4611) // Ignore setjmp() warning

extern "C"
{
#include "blorblow.h"
}

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// The single application instance
extern FrotzApp theApp;

// The single output window instance
extern FrotzWnd* theWnd;

FrotzGfx::FrotzGfx()
{
  m_pixels = NULL;
  m_width = 0;
  m_height = 0;
  m_transparentColor = -1;
  m_ratioStd = 1.0;
  m_ratioMin = 0.0;
  m_ratioMax = 0.0;
  m_usesPalette = false;
  m_colorChanger = false;
}

FrotzGfx::~FrotzGfx()
{
  if (m_pixels)
    delete[] m_pixels;
}

// Get the size of the picture
CSize FrotzGfx::GetSize(double erf)
{
  double r = erf * m_ratioStd;
  if ((m_ratioMin != 0.0) && (r < m_ratioMin))
    r = m_ratioMin;
  else if ((m_ratioMax != 0.0) && (r > m_ratioMax))
    r = m_ratioMax;
  if (r <= 0.0)
    r = 1.0;

  return CSize((int)(m_width*r), (int)(m_height*r));
}

// Apply the picture's palette to the screen palette
bool FrotzGfx::ApplyPalette()
{
  bool changed = false;

  if (m_usesPalette)
  {
    int colors = (int) m_palette.GetSize();
    if (colors > 16)
      colors = 16;

    for (int i = 0; i < colors; i++)
    {
      if (m_screenPalette[i] != m_palette[i])
      {
        changed = true;
        m_screenPalette[i] = m_palette[i];
      }
    }
  }

  return changed;
}

// Draw the picture
void FrotzGfx::Paint(CDibSection& dib, CPoint point, double erf)
{
  ::GdiFlush();

  if (m_pixels != NULL)
  {
    CSize size = GetSize(erf);

    // Clip graphic to the DIB boundary
    CRect clip(point, size);

    clip &= CRect(CPoint(0, 0), dib.GetSize());

    if (m_usesPalette)
    {
      // Use simple scaling without interpolation in palette mode
      for (int yy = clip.top; yy < clip.bottom; yy++)
      {
        int ys = (yy-point.y)*m_height/size.cy;

        for (int xx = clip.left; xx < clip.right; xx++)
        {
          int xs = (xx-point.x)*m_width/size.cx;

          int index = m_pixels[ys * m_width + xs];

          if (index != m_transparentColor)
            dib.SetPixel(xx,yy,m_screenPalette[index]);
        }
      }
    }
    else
    {
      DWORD* p = (DWORD*)m_pixels;

      // Scale the image if necessary
      if (size.cx != m_width || size.cy != m_height)
      {
        p = new DWORD[size.cx*size.cy];
        ScaleGfx((COLORREF*)m_pixels,m_width,m_height,(COLORREF*)p,size.cx,size.cy);
      }

      DWORD src, dest;
      int sr, sg, sb, dr, dg, db, a;

      // Alpha blend each pixel of the graphic into the bitmap
      for (int yy = clip.top; yy < clip.bottom; yy++)
      {
        for (int xx = clip.left; xx < clip.right; xx++)
        {
          // Get the colour of the pixel
          src = CDibSection::GetPixel(p,size.cx,xx-point.x,yy-point.y);

          // Split it into red, green, blue and alpha
          sb = src & 0xFF;
          src >>= 8;
          sg = src & 0xFF;
          src >>= 8;
          sr = src & 0xFF;
          src >>= 8;
          a = src & 0xFF;
          if (a == 0)
            continue;

          // Perform alpha blending
          if (a == 255)
          {
            dr = sr;
            dg = sg;
            db = sb;
          }
          else
          {
            // Get the colour of the destination pixel
            dest = dib.GetPixel(xx,yy);

            // Split it into red, green and blue
            db = dest & 0xFF;
            dest >>= 8;
            dg = dest & 0xFF;
            dest >>= 8;
            dr = dest & 0xFF;

            // Rescale from 0..255 to 0..256
            a += a>>7;

            // Alpha blend in the linear colour space
            dr = m_fromLinear[(m_toLinear[sr]*a + m_toLinear[dr]*(256-a))>>8];
            dg = m_fromLinear[(m_toLinear[sg]*a + m_toLinear[dg]*(256-a))>>8];
            db = m_fromLinear[(m_toLinear[sb]*a + m_toLinear[db]*(256-a))>>8];
          }

          dest = (dr<<16)|(dg<<8)|db;
          dib.SetPixel(xx,yy,dest);
        }
      }

      if (p != (DWORD*)m_pixels)
        delete [] p;
    }
  }
}

// Does this picture need to be redrawn after screen palette changes?
bool FrotzGfx::IsColorChanger()
{
  return m_colorChanger;
}

// Get a picture from the cache or the Blorb resource map
FrotzGfx* FrotzGfx::Get(int picture, bb_map_t* map, bool permissive)
{
  FrotzGfx* gfx = NULL;
  if (m_cache.Lookup(picture,gfx))
    return gfx;

  bb_result_t result;
  bb_aux_pict_t* ratios = NULL;
  if (bb_load_resource_pict(map,bb_method_Memory,&result,picture,&ratios) == bb_err_None)
  {
    BYTE* data = (BYTE*)result.data.ptr;
    int length = result.length;
    unsigned int id = map->chunks[result.chunknum].type;

    // Look for a recognized format
    if (id == bb_make_id('P','N','G',' '))
    {
      gfx = LoadPNG(data,length);
      if ((gfx == NULL) && permissive)
        gfx = LoadJPEG(data,length);
    }
    else if (id == bb_make_id('J','P','E','G'))
    {
      gfx = LoadJPEG(data,length);
      if ((gfx == NULL) && permissive)
        gfx = LoadPNG(data,length);
    }
    else if (id == bb_make_id('R','e','c','t'))
      gfx = LoadRect(data,length);

    // Does this picture have resolution information?
    if ((gfx != NULL) && (ratios != NULL))
    {
      gfx->m_ratioStd = (double)ratios->ratnum / (double)ratios->ratden;
      if (ratios->minnum != 0)
        gfx->m_ratioMin = (double)ratios->minnum / (double)ratios->minden;
      if (ratios->maxnum != 0)
        gfx->m_ratioMax = (double)ratios->maxnum / (double)ratios->maxden;
    }

    bb_unload_chunk(map,result.chunknum);

    if (gfx != NULL && m_adaptiveMode)
    {
      // 'Adaptive' pictures have no palette of their own
      int i;
      if (m_adaptive.Lookup(picture,i))
        gfx->m_palette.RemoveAll();

      if (story_id == ARTHUR)
        if (picture == 54 || picture == 170 || picture == 171)
          gfx->m_colorChanger = true;

      if (story_id == SHOGUN)
        if (picture == 3 || picture == 59)
          gfx->m_colorChanger = true;

      if (story_id == ZORK_ZERO)
        if (picture == 440)
          gfx->m_colorChanger = true;
    }
  }

  // Store in the cache
  m_cache[picture] = gfx;
  return gfx;
}

// Attempt to load palette information
void FrotzGfx::LoadPaletteInfo(bb_map_t* map)
{
  bb_result_t result;
  unsigned int id = bb_make_id('A','P','a','l');
  m_adaptive.RemoveAll();
  m_adaptiveMode = false;
  if (bb_load_chunk_by_type(map,bb_method_Memory,&result,id,0) == bb_err_None)
  {
    for (int i = 0; i < (int)result.length; i += 4)
    {
      unsigned char* data = ((unsigned char*)result.data.ptr)+i;
      int picture = (data[0]<<24)|(data[1]<<16)|(data[2]<<8)|data[3];
      m_adaptive[picture] = 0;
    }

    m_adaptiveMode = true;

    bb_unload_chunk(map,result.chunknum);
  }
}

// Clear the cache of pictures
void FrotzGfx::ClearCache(void)
{
  POSITION pos = m_cache.GetStartPosition();
  while (pos != NULL)
  {
    int number;
    FrotzGfx* gfx;

    m_cache.GetNextAssoc(pos,number,gfx);
    delete gfx;
  }
  m_cache.RemoveAll();
  m_adaptive.RemoveAll();
  m_adaptiveMode = false;
  ::ZeroMemory(m_screenPalette,sizeof m_screenPalette);
}

// Set the screen gamma and build gamma correction tables
void FrotzGfx::SetGamma(double gamma)
{
  m_gamma = gamma;

  for (int i = 0; i < 256; i++)
    m_toLinear[i] = (int)((pow(i/255.0,gamma) * 255.0) + 0.5);

  gamma = 1.0/gamma;
  for (int i = 0; i < 256; i++)
    m_fromLinear[i] = (int)((pow(i/255.0,gamma) * 255.0) + 0.5);
}

double FrotzGfx::m_gamma = 2.2;
int FrotzGfx::m_toLinear[256];
int FrotzGfx::m_fromLinear[256];

CMap<int,int,FrotzGfx*,FrotzGfx*> FrotzGfx::m_cache;
CMap<int,int,int,int> FrotzGfx::m_adaptive;
bool FrotzGfx::m_adaptiveMode;
DWORD FrotzGfx::m_screenPalette[16];

/////////////////////////////////////////////////////////////////////////////
// Loader for PNG images
/////////////////////////////////////////////////////////////////////////////

#include "png.h"

namespace {

struct PNGData
{
  BYTE* gfxData;
  ULONG offset;
};

void readPNGData(png_structp png_ptr, png_bytep data, png_size_t length)
{
  PNGData* pngData = (PNGData*)png_get_io_ptr(png_ptr);
  memcpy(data,pngData->gfxData+pngData->offset,length);
  pngData->offset += length;
}

} // unnamed namespace

FrotzGfx* FrotzGfx::LoadPNG(BYTE* data, int)
{
  FrotzGfx* graphic = NULL;
  png_bytep* rowPointers = NULL;

  if (!png_check_sig(data,8))
    return NULL;

  png_structp png_ptr = png_create_read_struct
    (PNG_LIBPNG_VER_STRING,(png_voidp)NULL,NULL,NULL);
  if (!png_ptr)
    return NULL;

  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr)
  {
    png_destroy_read_struct(&png_ptr,
      (png_infopp)NULL,(png_infopp)NULL);
    return NULL;
  }

  png_infop end_info = png_create_info_struct(png_ptr);
  if (!end_info)
  {
    png_destroy_read_struct(&png_ptr,&info_ptr,(png_infopp)NULL);
    return NULL;
  }

  if (setjmp(png_jmpbuf(png_ptr)))
  {
    png_destroy_read_struct(&png_ptr,&info_ptr,&end_info);
    if (rowPointers)
      delete[] rowPointers;
    if (graphic)
      delete graphic;
    return NULL;
  }

  PNGData pngData;
  pngData.gfxData = data;
  pngData.offset = 8;
  png_set_read_fn(png_ptr,&pngData,readPNGData);

  png_set_sig_bytes(png_ptr,8);
  png_read_info(png_ptr,info_ptr);

  png_uint_32 width = png_get_image_width(png_ptr,info_ptr);
  png_uint_32 height = png_get_image_height(png_ptr,info_ptr);
  int bit_depth = png_get_bit_depth(png_ptr,info_ptr);
  int color_type = png_get_color_type(png_ptr,info_ptr);

  double gamma;
  if (png_get_gAMA(png_ptr,info_ptr,&gamma))
    png_set_gamma(png_ptr,m_gamma,gamma);

  graphic = new FrotzGfx;
  graphic->m_width = width;
  graphic->m_height = height;

  if (m_adaptiveMode && color_type == PNG_COLOR_TYPE_PALETTE && bit_depth <= 8)
  {
    graphic->m_usesPalette = true;

    png_set_packing(png_ptr);

    // Check for transparency. In practice, the transparent
    // color will always be color 0.
    png_bytep trans;
    int num_trans;
    png_color_16p trans_values;
    if (png_get_tRNS(png_ptr,info_ptr,&trans,&num_trans,&trans_values) && num_trans >= 1)
      graphic->m_transparentColor = trans[0];

    int size = width*height;
    graphic->m_pixels = new BYTE[size];

    rowPointers = new png_bytep[height];
    for (int i = 0; i < (int)height; i++)
      rowPointers[i] = graphic->m_pixels+(width*i);
    png_read_image(png_ptr,rowPointers);

    // Get the palette after reading the image, so that the gamma
    // correction is applied
    png_colorp palette;
    int num_palette;
    if (png_get_PLTE(png_ptr,info_ptr,&palette,&num_palette))
    {
      for (int i = 0; i < num_palette; i++)
      {
        DWORD colour =
          (palette[i].red<<16)|(palette[i].green<<8)|palette[i].blue;
        graphic->m_palette.Add(colour);
      }
    }
  }
  else
  {
    if (color_type == PNG_COLOR_TYPE_PALETTE && bit_depth <= 8)
      png_set_palette_to_rgb(png_ptr);
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
      png_set_expand_gray_1_2_4_to_8(png_ptr);
    if (png_get_valid(png_ptr,info_ptr,PNG_INFO_tRNS))
      png_set_tRNS_to_alpha(png_ptr);

    if (bit_depth == 16)
      png_set_strip_16(png_ptr);
    if (bit_depth < 8)
      png_set_packing(png_ptr);
    if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
      png_set_gray_to_rgb(png_ptr);

    png_set_bgr(png_ptr);
    png_set_filler(png_ptr,0xFF,PNG_FILLER_AFTER);

    int size = width*height*4;
    graphic->m_pixels = new BYTE[size];

    rowPointers = new png_bytep[height];
    for (int i = 0; i < (int)height; i++)
      rowPointers[i] = graphic->m_pixels+(width*i*4);
    png_read_image(png_ptr,rowPointers);
  }

  png_read_end(png_ptr,end_info);
  png_destroy_read_struct(&png_ptr,&info_ptr,&end_info);
  if (rowPointers)
    delete[] rowPointers;
  return graphic;
}

/////////////////////////////////////////////////////////////////////////////
// Loader for JPEG images
/////////////////////////////////////////////////////////////////////////////

extern "C" {
#include "jpeglib.h"
}

namespace {

// Error Handling

struct JPEGErrorInfo
{
  struct jpeg_error_mgr base;
  jmp_buf errorJump;
};

void errorJPEGExit(j_common_ptr cinfo)
{
  (*cinfo->err->output_message)(cinfo);
  struct JPEGErrorInfo* error = (struct JPEGErrorInfo*)cinfo->err;
  longjmp(error->errorJump,1);
}

void outputJPEGMessage(j_common_ptr cinfo)
{
  char buffer[JMSG_LENGTH_MAX];
  (*cinfo->err->format_message)(cinfo,buffer);
  TRACE("JPEG: %s\n",buffer);
}

// Memory Data Source

void memJPEGInit(j_decompress_ptr)
{
}

boolean memJPEGFillInput(j_decompress_ptr)
{
  return FALSE;
}

void memJPEGSkipInput(j_decompress_ptr cinfo, long num_bytes)
{
  if (num_bytes > 0)
  {
    if (num_bytes > (long)cinfo->src->bytes_in_buffer)
      num_bytes = (long)cinfo->src->bytes_in_buffer;

    cinfo->src->next_input_byte += num_bytes;
    cinfo->src->bytes_in_buffer -= num_bytes;
  }
}

void memJPEGTerm(j_decompress_ptr)
{
}

} // unnamed namespace

FrotzGfx* FrotzGfx::LoadJPEG(BYTE* data, int length)
{
  FrotzGfx* graphic = NULL;
  struct jpeg_decompress_struct info;
  struct JPEGErrorInfo error;

  info.err = jpeg_std_error(&(error.base));
  error.base.error_exit = errorJPEGExit;
  error.base.output_message = outputJPEGMessage;
  if (setjmp(error.errorJump))
  {
    jpeg_destroy_decompress(&info);
    if (graphic)
      delete graphic;
    return NULL;
  }

  jpeg_create_decompress(&info);

  info.src = (struct jpeg_source_mgr*)(info.mem->alloc_small)
    ((j_common_ptr)(&info),JPOOL_PERMANENT,sizeof(jpeg_source_mgr));
  info.src->init_source = memJPEGInit;
  info.src->fill_input_buffer = memJPEGFillInput;
  info.src->skip_input_data = memJPEGSkipInput;
  info.src->resync_to_restart = jpeg_resync_to_restart;
  info.src->term_source = memJPEGTerm;
  info.src->bytes_in_buffer = length;
  info.src->next_input_byte = data;

  jpeg_read_header(&info,TRUE);
  jpeg_calc_output_dimensions(&info);
  int width = info.output_width;
  int height = info.output_height;

  graphic = new FrotzGfx;
  graphic->m_width = width;
  graphic->m_height = height;
  graphic->m_pixels = new BYTE[width*height*4];
    
  // Force RGB output
  info.out_color_space = JCS_RGB;

  // Get an output buffer
  JSAMPARRAY buffer = (*info.mem->alloc_sarray)
    ((j_common_ptr)&info,JPOOL_IMAGE,width*3,1);

  jpeg_start_decompress(&info);
  while ((int)info.output_scanline < height)
  {
    jpeg_read_scanlines(&info,buffer,1);

    BYTE* pixelRow = graphic->m_pixels+
      (width*(info.output_scanline-1)*4);
    for (int i = 0; i < width; i++)
    {
      pixelRow[(i*4)+0] = (*buffer)[(i*3)+2];
      pixelRow[(i*4)+1] = (*buffer)[(i*3)+1];
      pixelRow[(i*4)+2] = (*buffer)[(i*3)+0];
      pixelRow[(i*4)+3] = 0xFF;
    }
  }
  jpeg_finish_decompress(&info);
  jpeg_destroy_decompress(&info);
  return graphic;
}

/////////////////////////////////////////////////////////////////////////////
// Loader for simple rectangles
/////////////////////////////////////////////////////////////////////////////

FrotzGfx* FrotzGfx::LoadRect(BYTE* data, int)
{
  FrotzGfx* graphic = new FrotzGfx;
  graphic->m_width = (data[0]<<24)|(data[1]<<16)|(data[2]<<8)|data[3];
  graphic->m_height = (data[4]<<24)|(data[5]<<16)|(data[6]<<8)|data[7];
  return graphic;
}
