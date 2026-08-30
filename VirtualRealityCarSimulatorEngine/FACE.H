
//===================================================================================================================//
// Header for FACE module
//===================================================================================================================//

#if !defined(FACE_H)
#define FACE_H

#include <math.h>
#include <alloc.h>

#include "triplet.h"
#include "MyClip.h"


class MyFace
{
public:

  struct MYPOINT
  {
    long int x;
    long int y;
  };

  MyTriplet ViewVector;
  int       NbPoints;
  int       Red,Green,Blue;

  MyFace();
  virtual ~MyFace();

  virtual void      InitFace(MyTriplet *,MyTriplet *,MyTriplet *,MyTriplet *);
  virtual void      InitFace(MyTriplet *,MyTriplet *,MyTriplet *);
  virtual void	    SetColor(int,int,int);
  virtual long int  GetMinZ(long int);
  virtual long int  GetMinZ(void);
  virtual long int  GetMaxZ(void);
  virtual void      Draw(HDC,long int,long int,long int,long int,long int,int,int,short,MyTriplet);

private:

  MyTriplet *P[4];
  MyTriplet NormalVector;

  virtual void      InitNormalVector(MyTriplet *,MyTriplet *,MyTriplet *);
  virtual float     Norme(MyTriplet);
  virtual float     Scalaire(MyTriplet,MyTriplet);
  virtual RECT      MiniMax(POINT,POINT,POINT,POINT);
  virtual short     Shadows(HDC,long int,long int,long int,long int,long int,int,MyTriplet);
};

#endif