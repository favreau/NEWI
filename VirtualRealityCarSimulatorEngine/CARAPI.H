/* CAR.H */

#if !defined(CAR_H)
#define CAR_H

unsigned short far pascal Throttle(void);

  /* 0 = zero throttle postion
   255 = maximum throttle position */

unsigned short far pascal Clutch(void);

  /* 0 = zero clutch postion
   255 = maximum clutch position */

unsigned short far pascal Brake(void);

  /* 0 = zero brake postion
   255 = maximum brake position */

unsigned short far pascal Handbrake(void);

  /* 0 = zero handbrake postion
   255 = maximum handbrake position */

short far pascal Gear(void);
  /* 0 = neutral
     1 .. 5 = forward gears
    -1 = reverse */

short far pascal WheelPosition(void);
  /* -110 to +110 = -55 to +55 degrees */

unsigned short far pascal Ignition(void);
  /* 0 = off
     1 = on
     2 = start */

short far pascal HeadPosition(void);
  /* -128 to +127 = -256 to +254 degrees */

#endif