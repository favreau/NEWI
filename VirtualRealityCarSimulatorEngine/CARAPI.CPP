//#include <owl\owlpch.h>
//#include <owl\applicat.h>
//#include <owl\window.h>
#include "CARAPI.H"
#include <conio.h>
void Init(void);
#pragma startup Init
#define B 0x1b0

unsigned short far pascal _export
Throttle()
{
return  inp(B);
}

unsigned short far pascal _export
Clutch()
{
return  inp(B + 1);
}

unsigned short far pascal _export
Brake()
{
return  (inp(B + 2) / 16 * 17);
}

unsigned short far pascal _export
Handbrake()
{
return  (inp(B + 2) % 16 * 17);
}

short far pascal _export
Gear()
{
unsigned short pascal Temp;
Temp = inp(B + 6) / 16;
if (Temp == 15)
  return -1;
else
  return Temp;
}

short far pascal _export
WheelPosition()
{
return inp(B + 4);
}

short far pascal _export
HeadPosition()
{
return inp(B + 5);
}

unsigned short far pascal _export
Ignition()
{
return inp(B + 6) % 16;
}

void Init(void)
{
 outp(B + 3, 0x9B);
 outp(B + 7, 0x9B);
}

