#ifndef NEWI_COMPAT_CONIO_H
#define NEWI_COMPAT_CONIO_H

// Borland <conio.h>. inp/outp are direct x86 port I/O, used by CARAPI.CPP to
// talk to the simulator rig on port 0x1B0. Unprivileged processes cannot issue
// IN/OUT, so these are inert: reads report a centred/idle rig.

#ifndef __BORLANDC__

inline int inp(unsigned port)
{
  (void)port;
  return 0;
}

inline int outp(unsigned port, int value)
{
  (void)port;
  return value;
}

inline unsigned inpw(unsigned port)
{
  (void)port;
  return 0;
}

inline unsigned outpw(unsigned port, unsigned value)
{
  (void)port;
  return value;
}

#endif // __BORLANDC__

#endif // NEWI_COMPAT_CONIO_H
