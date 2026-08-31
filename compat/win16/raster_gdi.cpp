#include "newi_raster.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <vector>

#include <png.h>

// ---------------------------------------------------------------------------
// GDI object model
// ---------------------------------------------------------------------------

namespace
{

enum ObjKind
{
  KIND_PEN,
  KIND_BRUSH,
  KIND_REGION
};

struct GdiObj
{
  ObjKind kind;
  COLORREF color;
  int penStyle;
  int penWidth;
  RECT rect;
  bool owned; // false for a DC's default pen/brush: never freed
};

struct Dc
{
  NewiPixel *pixels;
  int width;
  int height;
  RECT clip;
  long curX;
  long curY;
  GdiObj *pen;
  GdiObj *brush;
  GdiObj defaultPen;
  GdiObj defaultBrush;
};

inline NewiPixel toPixel(COLORREF c)
{
  // COLORREF is 0x00BBGGRR; the framebuffer is 0x00RRGGBB.
  return (NewiPixel)(((c & 0x000000FFu) << 16) | (c & 0x0000FF00u) |
                     ((c & 0x00FF0000u) >> 16));
}

inline Dc *asDc(HDC dc) { return static_cast<Dc *>(dc); }
inline GdiObj *asObj(HGDIOBJ o) { return static_cast<GdiObj *>(o); }

inline void putPixel(Dc *d, long x, long y, NewiPixel p)
{
  if (x < d->clip.left || x > d->clip.right) return;
  if (y < d->clip.top || y > d->clip.bottom) return;
  d->pixels[y * (long)d->width + x] = p;
}

void fillSpan(Dc *d, long y, double xa, double xb, NewiPixel p)
{
  if (y < d->clip.top || y > d->clip.bottom) return;
  if (xb < xa) std::swap(xa, xb);

  // Clamp in floating point first: the engine can emit coordinates far outside
  // any plausible integer range for geometry near the projection plane.
  if (xb < (double)d->clip.left || xa > (double)d->clip.right) return;
  long x0 = (long)std::ceil(std::max(xa, (double)d->clip.left) - 0.5);
  long x1 = (long)std::floor(std::min(xb, (double)d->clip.right) + 0.5) - 1;
  if (x0 < d->clip.left) x0 = d->clip.left;
  if (x1 > d->clip.right) x1 = d->clip.right;

  NewiPixel *row = d->pixels + y * (long)d->width;
  for (long x = x0; x <= x1; ++x)
    row[x] = p;
}

// Even-odd scanline fill.
void fillPolygon(Dc *d, const POINT *pts, int n, NewiPixel p)
{
  if (n < 3) return;

  double yMin = (double)pts[0].y, yMax = yMin;
  for (int i = 1; i < n; ++i)
  {
    yMin = std::min(yMin, (double)pts[i].y);
    yMax = std::max(yMax, (double)pts[i].y);
  }
  long y0 = (long)std::max(std::ceil(yMin), (double)d->clip.top);
  long y1 = (long)std::min(std::floor(yMax), (double)d->clip.bottom);

  std::vector<double> xs;
  xs.reserve((size_t)n);

  for (long y = y0; y <= y1; ++y)
  {
    const double yc = (double)y + 0.5;
    xs.clear();
    for (int i = 0; i < n; ++i)
    {
      const double ax = (double)pts[i].x, ay = (double)pts[i].y;
      const int j = (i + 1) % n;
      const double bx = (double)pts[j].x, by = (double)pts[j].y;
      if (ay == by) continue;
      if (yc >= std::min(ay, by) && yc < std::max(ay, by))
        xs.push_back(ax + (yc - ay) * (bx - ax) / (by - ay));
    }
    if (xs.size() < 2) continue;
    std::sort(xs.begin(), xs.end());
    for (size_t k = 0; k + 1 < xs.size(); k += 2)
      fillSpan(d, y, xs[k], xs[k + 1], p);
  }
}

// Liang-Barsky, in doubles so absurd endpoints survive the trip.
bool clipSegment(const Dc *d, double &x0, double &y0, double &x1, double &y1)
{
  const double xmin = (double)d->clip.left - 0.5;
  const double xmax = (double)d->clip.right + 0.5;
  const double ymin = (double)d->clip.top - 0.5;
  const double ymax = (double)d->clip.bottom + 0.5;

  double t0 = 0.0, t1 = 1.0;
  const double dx = x1 - x0, dy = y1 - y0;

  const double p[4] = {-dx, dx, -dy, dy};
  const double q[4] = {x0 - xmin, xmax - x0, y0 - ymin, ymax - y0};

  for (int i = 0; i < 4; ++i)
  {
    if (p[i] == 0.0)
    {
      if (q[i] < 0.0) return false;
      continue;
    }
    const double r = q[i] / p[i];
    if (p[i] < 0.0)
    {
      if (r > t1) return false;
      if (r > t0) t0 = r;
    }
    else
    {
      if (r < t0) return false;
      if (r < t1) t1 = r;
    }
  }

  const double nx0 = x0 + t0 * dx, ny0 = y0 + t0 * dy;
  const double nx1 = x0 + t1 * dx, ny1 = y0 + t1 * dy;
  x0 = nx0; y0 = ny0; x1 = nx1; y1 = ny1;
  return true;
}

void drawLine(Dc *d, long ax, long ay, long bx, long by, NewiPixel p, int width)
{
  double x0 = (double)ax, y0 = (double)ay, x1 = (double)bx, y1 = (double)by;
  if (!clipSegment(d, x0, y0, x1, y1)) return;

  long ix0 = (long)std::lround(x0), iy0 = (long)std::lround(y0);
  long ix1 = (long)std::lround(x1), iy1 = (long)std::lround(y1);

  long dx = std::labs(ix1 - ix0), sx = ix0 < ix1 ? 1 : -1;
  long dy = -std::labs(iy1 - iy0), sy = iy0 < iy1 ? 1 : -1;
  long err = dx + dy;

  const long half = width > 1 ? width / 2 : 0;
  for (;;)
  {
    if (half == 0)
      putPixel(d, ix0, iy0, p);
    else
      for (long oy = -half; oy <= half; ++oy)
        for (long ox = -half; ox <= half; ++ox)
          putPixel(d, ix0 + ox, iy0 + oy, p);

    if (ix0 == ix1 && iy0 == iy1) break;
    const long e2 = 2 * err;
    if (e2 >= dy) { err += dy; ix0 += sx; }
    if (e2 <= dx) { err += dx; iy0 += sy; }
  }
}

} // namespace

// ---------------------------------------------------------------------------
// DC lifetime
// ---------------------------------------------------------------------------

HDC NewiRasterCreateDC(NewiPixel *pixels, int width, int height)
{
  Dc *d = new Dc();
  d->pixels = pixels;
  d->width = width;
  d->height = height;
  d->clip.left = 0;
  d->clip.top = 0;
  d->clip.right = width - 1;
  d->clip.bottom = height - 1;
  d->curX = 0;
  d->curY = 0;

  d->defaultPen.kind = KIND_PEN;
  d->defaultPen.color = RGB(0, 0, 0);
  d->defaultPen.penStyle = PS_SOLID;
  d->defaultPen.penWidth = 1;
  d->defaultPen.owned = false;

  d->defaultBrush.kind = KIND_BRUSH;
  d->defaultBrush.color = RGB(255, 255, 255);
  d->defaultBrush.penStyle = 0;
  d->defaultBrush.penWidth = 0;
  d->defaultBrush.owned = false;

  d->pen = &d->defaultPen;
  d->brush = &d->defaultBrush;
  return static_cast<HDC>(d);
}

void NewiRasterDestroyDC(HDC dc)
{
  delete asDc(dc);
}

void NewiRasterClear(HDC dc, COLORREF color)
{
  Dc *d = asDc(dc);
  const NewiPixel p = toPixel(color);
  const long total = (long)d->width * d->height;
  for (long i = 0; i < total; ++i)
    d->pixels[i] = p;
}

// ---------------------------------------------------------------------------
// GDI entry points used by the engine
// ---------------------------------------------------------------------------

HPEN CreatePen(int style, int width, COLORREF color)
{
  GdiObj *o = new GdiObj();
  o->kind = KIND_PEN;
  o->color = color;
  o->penStyle = style;
  o->penWidth = width < 1 ? 1 : width;
  o->owned = true;
  return static_cast<HPEN>(o);
}

HBRUSH CreateSolidBrush(COLORREF color)
{
  GdiObj *o = new GdiObj();
  o->kind = KIND_BRUSH;
  o->color = color;
  o->penStyle = 0;
  o->penWidth = 0;
  o->owned = true;
  return static_cast<HBRUSH>(o);
}

HRGN CreateRectRgn(int left, int top, int right, int bottom)
{
  GdiObj *o = new GdiObj();
  o->kind = KIND_REGION;
  o->color = 0;
  o->penStyle = 0;
  o->penWidth = 0;
  o->rect.left = left;
  o->rect.top = top;
  o->rect.right = right;
  o->rect.bottom = bottom;
  o->owned = true;
  return static_cast<HRGN>(o);
}

HGDIOBJ SelectObject(HDC dc, HGDIOBJ object)
{
  Dc *d = asDc(dc);
  GdiObj *o = asObj(object);
  if (!d || !o) return 0;

  if (o->kind == KIND_PEN)
  {
    GdiObj *old = d->pen;
    d->pen = o;
    return static_cast<HGDIOBJ>(old);
  }
  if (o->kind == KIND_BRUSH)
  {
    GdiObj *old = d->brush;
    d->brush = o;
    return static_cast<HGDIOBJ>(old);
  }
  return 0;
}

BOOL DeleteObject(HGDIOBJ object)
{
  GdiObj *o = asObj(object);
  if (!o || !o->owned) return TRUE;
  delete o;
  return TRUE;
}

HBITMAP LoadBitmap(HINSTANCE, LPCSTR) { return 0; }

int SelectClipRgn(HDC dc, HRGN region)
{
  Dc *d = asDc(dc);
  if (!d) return 0;

  if (!region)
  {
    d->clip.left = 0;
    d->clip.top = 0;
    d->clip.right = d->width - 1;
    d->clip.bottom = d->height - 1;
    return 1;
  }

  // The region rectangle is copied, not referenced: VIEW.CPP deletes the
  // region while it is still the DC's clip.
  const RECT &r = asObj(region)->rect;
  d->clip.left = std::max<LONG>(0, std::min(r.left, r.right));
  d->clip.top = std::max<LONG>(0, std::min(r.top, r.bottom));
  d->clip.right = std::min<LONG>(d->width - 1, std::max(r.left, r.right) - 1);
  d->clip.bottom = std::min<LONG>(d->height - 1, std::max(r.top, r.bottom) - 1);
  return 1;
}

DWORD MoveTo(HDC dc, int x, int y)
{
  Dc *d = asDc(dc);
  const DWORD prev = 0;
  d->curX = x;
  d->curY = y;
  return prev;
}

BOOL LineTo(HDC dc, int x, int y)
{
  Dc *d = asDc(dc);
  if (d->pen->penStyle != PS_NULL)
    drawLine(d, d->curX, d->curY, x, y, toPixel(d->pen->color),
             d->pen->penWidth);
  d->curX = x;
  d->curY = y;
  return TRUE;
}

BOOL Polygon(HDC dc, const POINT *points, int count)
{
  Dc *d = asDc(dc);
  if (count < 2) return TRUE;

  fillPolygon(d, points, count, toPixel(d->brush->color));

  if (d->pen->penStyle != PS_NULL)
  {
    const NewiPixel edge = toPixel(d->pen->color);
    for (int i = 0; i < count; ++i)
    {
      const int j = (i + 1) % count;
      drawLine(d, points[i].x, points[i].y, points[j].x, points[j].y, edge,
               d->pen->penWidth);
    }
  }
  return TRUE;
}

BOOL Ellipse(HDC dc, int left, int top, int right, int bottom)
{
  Dc *d = asDc(dc);
  const double cx = (left + right) / 2.0;
  const double cy = (top + bottom) / 2.0;
  const double rx = std::fabs(right - left) / 2.0;
  const double ry = std::fabs(bottom - top) / 2.0;

  const NewiPixel fill = toPixel(d->brush->color);
  for (long y = (long)std::ceil(cy - ry); y <= (long)std::floor(cy + ry); ++y)
  {
    if (ry <= 0) break;
    const double t = ((double)y + 0.5 - cy) / ry;
    if (t < -1.0 || t > 1.0) continue;
    const double halfWidth = rx * std::sqrt(1.0 - t * t);
    fillSpan(d, y, cx - halfWidth, cx + halfWidth, fill);
  }
  return TRUE;
}

int FillRect(HDC dc, const RECT *rect, HBRUSH brush)
{
  Dc *d = asDc(dc);
  if (!rect) return 0;
  GdiObj *b = asObj(brush);
  const NewiPixel p = toPixel(b ? b->color : d->brush->color);

  const long top = std::min(rect->top, rect->bottom);
  const long bottom = std::max(rect->top, rect->bottom);
  const double left = (double)std::min(rect->left, rect->right);
  const double right = (double)std::max(rect->left, rect->right);
  for (long y = top; y < bottom; ++y)
    fillSpan(d, y, left, right, p);
  return 1;
}

// ---------------------------------------------------------------------------
// PNG output
// ---------------------------------------------------------------------------

bool NewiRasterWritePng(const char *path, const NewiPixel *pixels, int width,
                        int height)
{
  FILE *fp = std::fopen(path, "wb");
  if (!fp) return false;

  png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
  if (!png)
  {
    std::fclose(fp);
    return false;
  }
  png_infop info = png_create_info_struct(png);
  if (!info)
  {
    png_destroy_write_struct(&png, 0);
    std::fclose(fp);
    return false;
  }
  if (setjmp(png_jmpbuf(png)))
  {
    png_destroy_write_struct(&png, &info);
    std::fclose(fp);
    return false;
  }

  png_init_io(png, fp);
  png_set_IHDR(png, info, (png_uint_32)width, (png_uint_32)height, 8,
               PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
  png_write_info(png, info);

  std::vector<png_byte> row((size_t)width * 3);
  for (int y = 0; y < height; ++y)
  {
    const NewiPixel *src = pixels + (size_t)y * width;
    for (int x = 0; x < width; ++x)
    {
      row[(size_t)x * 3 + 0] = (png_byte)((src[x] >> 16) & 0xFF);
      row[(size_t)x * 3 + 1] = (png_byte)((src[x] >> 8) & 0xFF);
      row[(size_t)x * 3 + 2] = (png_byte)(src[x] & 0xFF);
    }
    png_write_row(png, row.data());
  }

  png_write_end(png, info);
  png_destroy_write_struct(&png, &info);
  std::fclose(fp);
  return true;
}
