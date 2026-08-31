#ifndef NEWI_GDI_CAPTURE_H
#define NEWI_GDI_CAPTURE_H

// Instrumentation for the stub GDI. Lets a host program verify that the
// engine's projection, clipping and hidden-surface passes actually emit
// geometry, without a display.

#include <windows.h>

struct NewiGdiStats
{
  long polygons;
  long polygonVertices;
  long lines;
  long moves;
  long ellipses;
  long fillRects;
  long pensCreated;
  long brushesCreated;
  long regionsCreated;
  long objectsDeleted;

  // Bounding box of every vertex handed to Polygon/LineTo/MoveTo.
  LONG minX, minY, maxX, maxY;
  BOOL hasBounds;
};

const NewiGdiStats &NewiGdiGetStats();
void NewiGdiResetStats();

// A non-null HDC to hand to the engine.
HDC NewiGdiCreateStubDC();

#endif // NEWI_GDI_CAPTURE_H
