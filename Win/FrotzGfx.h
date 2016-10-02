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
  // Get the size of the picture
  CSize GetSize(double erf);
  // Draw the picture
  void Paint(CDibSection& dib, CPoint point, double erf);
  // Apply the picture's palette to the screen palette
  bool ApplyPalette();
  // Does this picture need to be redrawn after screen palette changes?
  bool IsColorChanger();

  // Get a picture from the cache or the Blorb resource map
  static FrotzGfx* Get(int picture, bb_map_t* map, bool permissive);
  // Attempt to load palette information
  static void LoadPaletteInfo(bb_map_t* map);
  // Clear the cache of pictures
  static void ClearCache(void);
  // Set the screen gamma and build gamma correction tables
  static void SetGamma(double gamma);

protected:
  FrotzGfx();
  ~FrotzGfx();

  static FrotzGfx* LoadPNG(BYTE* data, int length);
  static FrotzGfx* LoadJPEG(BYTE* data, int length);
  static FrotzGfx* LoadRect(BYTE* data, int length);

protected:
  BYTE* m_pixels;

  int m_width;
  int m_height;

  int m_transparentColor;

  double m_ratioStd;
  double m_ratioMin;
  double m_ratioMax;

  CArray<DWORD,DWORD> m_palette;

  bool m_usesPalette;
  bool m_colorChanger;

  static double m_gamma;
  static int m_toLinear[256];
  static int m_fromLinear[256];

  static CMap<int,int,FrotzGfx*,FrotzGfx*> m_cache;
  static CMap<int,int,int,int> m_adaptive;
  static bool m_adaptiveMode;
  static DWORD m_screenPalette[16];
};
