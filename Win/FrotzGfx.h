/////////////////////////////////////////////////////////////////////////////
// Windows Frotz
// Frotz graphics class
/////////////////////////////////////////////////////////////////////////////

#pragma once

class CDibSection;

extern "C"
{
#include "blorb.h"
}

class FrotzGfx
{
public:
  FrotzGfx();
  ~FrotzGfx();

  // Get the width of the picture
  int GetWidth(int scale);
  // Get the height of the picture
  int GetHeight(int scale);
  // Draw the picture
  void Paint(CDibSection& dib, int x, int y, int scale);
  // Draw the picture, scaled to the target
  void PaintScaled(CDibSection& dib);

  // Get a picture from the cache or the Blorb resource map
  static FrotzGfx* Get(int picture, bb_map_t* map, bool permissive);
  // Attempt to load palette information
  static void LoadPaletteInfo(bb_map_t* map);
  // Clear the cache of pictures
  static void ClearCache(void);
  // Set the screen gamma and build gamma correction tables
  static void SetGamma(double gamma);

protected:
  static FrotzGfx* LoadPNG(BYTE* data, int length);
  static FrotzGfx* LoadJPEG(BYTE* data, int length);
  static FrotzGfx* LoadRect(BYTE* data, int length);

protected:
  BYTE* m_pixels;
  BITMAPINFOHEADER *m_header;

  int m_width;
  int m_height;

  bool m_adapt;
  CArray<DWORD,DWORD> m_palette;
  CMap <DWORD,DWORD,int,int> m_invPalette;

  static double m_gamma;
  static int m_toLinear[256];
  static int m_fromLinear[256];

  static CMap<int,int,FrotzGfx*,FrotzGfx*> m_cache;
  static CMap<int,int,int,int> m_adaptive;
  static CArray<DWORD,DWORD> m_currentPalette;
};
