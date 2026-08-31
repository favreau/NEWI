#ifndef NEWI_BORLAND_PRELUDE_H
#define NEWI_BORLAND_PRELUDE_H

// Force-included into every translation unit on non-Borland compilers.
// Borland C++ 4.x keywords for the segmented 16-bit memory model and the
// Pascal calling convention have no equivalent on flat 64-bit targets, so
// they are erased rather than emulated.

#ifndef __BORLANDC__

#define far
#define near
#define huge
#define _export
#define _loadds
#define pascal
#define cdecl

#ifndef FAR
#define FAR
#endif
#ifndef NEAR
#define NEAR
#endif
#ifndef PASCAL
#define PASCAL
#endif
#ifndef WINAPI
#define WINAPI
#endif

// M_PI is not in the C++ standard; Borland's math.h always provided it.
#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif

#endif // __BORLANDC__

#endif // NEWI_BORLAND_PRELUDE_H
