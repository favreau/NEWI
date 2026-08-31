#ifndef NEWI_RASTER_H
#define NEWI_RASTER_H

// A real software rasteriser behind the Win16 GDI surface. Where gdi_stubs.cpp
// counts calls for the tests, this one actually fills pixels, so the 1995
// engine can draw into a plain memory framebuffer.

#include <windows.h>

// 0x00RRGGBB, one word per pixel, top row first.
typedef unsigned int NewiPixel;

// Creates a device context that renders into `pixels` (width*height words).
// The caller owns the buffer and must keep it alive for the life of the DC.
HDC NewiRasterCreateDC(NewiPixel *pixels, int width, int height);
void NewiRasterDestroyDC(HDC dc);

void NewiRasterClear(HDC dc, COLORREF color);

// Writes the framebuffer as a PNG. Returns false on I/O or encoder failure.
bool NewiRasterWritePng(const char *path, const NewiPixel *pixels, int width,
                        int height);

#endif // NEWI_RASTER_H
