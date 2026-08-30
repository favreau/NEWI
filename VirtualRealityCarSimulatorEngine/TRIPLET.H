//===================================================================================================================//
// Triplet module header
//===================================================================================================================//


#if !defined(TRIPLET_H)
#define TRIPLET_H

#include <windows.h>
#include "const.h"

class MyTriplet
{            	
public:

  float    x,y,z;
  float    Px,Py,Pz;
  POINT P;

  MyTriplet();
  virtual ~MyTriplet();

  virtual void      SetCoordinates(float,float,float);
  virtual void      Reset(void);
  virtual float  GetX(void);
  virtual float  GetY(void);
  virtual float  GetZ(void);
  virtual POINT     GetP();
  virtual void	    Translation(float,float,float);
  virtual void      Rotation(float,float,float,int,float);
  virtual void      Projection(float,float,float);
  virtual void      Draw(HDC,float,float,float,float,float);

};

#endif