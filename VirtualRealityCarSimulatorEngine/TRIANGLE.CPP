
// ========================================================================
// Unit used to find the view plane
// ========================================================================
#include "triangle.h"

// ========================================================================
// return 0 is aP is in the view plane
// ========================================================================
int Triangle(MyTriplet aP)
{
  if (aP.Pz <= KHorizon)
  {
    if ((aP.Pz+200) >= abs(aP.Px))
    {
      return 0;
    }
    else
      return 1;
  }
  else
    return 1;
}

/*
int Triangle(POINT aP1,POINT aP2,POINT aP3, MyTriplet aP)
{ 
  float a;
  float b;
  float d;

  if (aP.Px == 0 && aP.Pz >= aP2.y) return 1;
  else
  {
 
    d =(aP2.x-aP.Px)*(aP1.y-aP.Pz)+(aP2.y-aP.Pz)*(aP.Px-aP1.x);
  
    if (d != 0)
    {
      b =-((aP3.x-aP.Px)*(aP1.y-aP.Pz)+(aP3.y-aP.Pz)*(aP.Px-aP1.x))/d;
    }
    else
    {
      b = 1;
    }
    d = aP.Px - aP1.x;
    if (d != 0)
    {
      a =(b*(aP2.x-aP.Px)+(aP3.x-aP.Px))/d;
    }
    else
    {
      a = 1;
    }
    if (a>=0 && b>=0)
    {
      return 0;
    }
    else
    {
      return 1;
    }
  };
}*/
