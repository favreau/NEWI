#include "newi_gdi_capture.h"

namespace
{

NewiGdiStats gStats;

// Handles only need to be distinct and non-null; nothing dereferences them.
HANDLE nextHandle()
{
  static char pool[4096];
  static size_t next = 0;
  if (next >= sizeof(pool))
    next = 0;
  return static_cast<HANDLE>(&pool[next++]);
}

void trackPoint(LONG x, LONG y)
{
  if (!gStats.hasBounds)
  {
    gStats.minX = gStats.maxX = x;
    gStats.minY = gStats.maxY = y;
    gStats.hasBounds = TRUE;
    return;
  }
  if (x < gStats.minX) gStats.minX = x;
  if (x > gStats.maxX) gStats.maxX = x;
  if (y < gStats.minY) gStats.minY = y;
  if (y > gStats.maxY) gStats.maxY = y;
}

} // namespace

const NewiGdiStats &NewiGdiGetStats() { return gStats; }

void NewiGdiResetStats()
{
  NewiGdiStats empty = {};
  gStats = empty;
}

HDC NewiGdiCreateStubDC() { return nextHandle(); }

HPEN CreatePen(int, int, COLORREF)
{
  gStats.pensCreated++;
  return nextHandle();
}

HBRUSH CreateSolidBrush(COLORREF)
{
  gStats.brushesCreated++;
  return nextHandle();
}

HRGN CreateRectRgn(int, int, int, int)
{
  gStats.regionsCreated++;
  return nextHandle();
}

HGDIOBJ SelectObject(HDC, HGDIOBJ) { return nextHandle(); }

BOOL DeleteObject(HGDIOBJ)
{
  gStats.objectsDeleted++;
  return TRUE;
}

HBITMAP LoadBitmap(HINSTANCE, LPCSTR) { return nextHandle(); }

DWORD MoveTo(HDC, int x, int y)
{
  gStats.moves++;
  trackPoint(x, y);
  return 0;
}

BOOL LineTo(HDC, int x, int y)
{
  gStats.lines++;
  trackPoint(x, y);
  return TRUE;
}

BOOL Polygon(HDC, const POINT *points, int count)
{
  gStats.polygons++;
  gStats.polygonVertices += count;
  for (int i = 0; i < count; ++i)
    trackPoint(points[i].x, points[i].y);
  return TRUE;
}

BOOL Ellipse(HDC, int left, int top, int right, int bottom)
{
  gStats.ellipses++;
  trackPoint(left, top);
  trackPoint(right, bottom);
  return TRUE;
}

int FillRect(HDC, const RECT *rect, HBRUSH)
{
  gStats.fillRects++;
  if (rect)
  {
    trackPoint(rect->left, rect->top);
    trackPoint(rect->right, rect->bottom);
  }
  return 1;
}

int SelectClipRgn(HDC, HRGN) { return 1; }
