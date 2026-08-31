#ifndef NEWI_COMPAT_WINDOWS_H
#define NEWI_COMPAT_WINDOWS_H

// Stand-in for the subset of Win16 windows.h that the simulator engine uses.
// Only the GDI drawing surface is covered: enough to compile and link the
// geometry/rasterisation code away from Windows. Implementations live in
// gdi_stubs.cpp and capture calls instead of drawing.

#include <cstddef>
#include <cstdlib>

typedef int BOOL;
typedef long LONG;
typedef unsigned long DWORD;
typedef unsigned int UINT;
typedef unsigned short WORD;
typedef char *LPSTR;
typedef const char *LPCSTR;
typedef unsigned long COLORREF;

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

typedef void *HANDLE;
typedef HANDLE HGDIOBJ;
typedef HANDLE HDC;
typedef HANDLE HPEN;
typedef HANDLE HBRUSH;
typedef HANDLE HRGN;
typedef HANDLE HBITMAP;
typedef HANDLE HPALETTE;
typedef HANDLE HINSTANCE;
typedef HANDLE HWND;

typedef struct tagPOINT
{
  LONG x;
  LONG y;
} POINT;

typedef struct tagRECT
{
  LONG left;
  LONG top;
  LONG right;
  LONG bottom;
} RECT;

#define RGB(r, g, b)                                                          \
  ((COLORREF)(((DWORD)(unsigned char)(r)) |                                   \
              (((DWORD)(unsigned char)(g)) << 8) |                            \
              (((DWORD)(unsigned char)(b)) << 16)))

#define GetRValue(c) ((unsigned char)((c) & 0xFF))
#define GetGValue(c) ((unsigned char)(((c) >> 8) & 0xFF))
#define GetBValue(c) ((unsigned char)(((c) >> 16) & 0xFF))

#define PS_SOLID 0
#define PS_DASH 1
#define PS_DOT 2
#define PS_NULL 5

#define MB_OK 0x0
#define MB_ICONEXCLAMATION 0x30

// --- GDI object management -------------------------------------------------
HPEN CreatePen(int style, int width, COLORREF color);
HBRUSH CreateSolidBrush(COLORREF color);
HRGN CreateRectRgn(int left, int top, int right, int bottom);
HGDIOBJ SelectObject(HDC dc, HGDIOBJ object);
BOOL DeleteObject(HGDIOBJ object);
HBITMAP LoadBitmap(HINSTANCE instance, LPCSTR name);

// --- Drawing --------------------------------------------------------------
// MoveTo is the Win16 spelling; Win32 replaced it with MoveToEx.
DWORD MoveTo(HDC dc, int x, int y);
BOOL LineTo(HDC dc, int x, int y);
BOOL Polygon(HDC dc, const POINT *points, int count);
BOOL Ellipse(HDC dc, int left, int top, int right, int bottom);
int FillRect(HDC dc, const RECT *rect, HBRUSH brush);
int SelectClipRgn(HDC dc, HRGN region);

#endif // NEWI_COMPAT_WINDOWS_H
