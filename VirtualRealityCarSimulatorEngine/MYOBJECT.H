
//===================================================================================================================//
// MyObject unit header
//===================================================================================================================//

#if !defined(MYOBJECT_H)
#define MYOBJECT_H

#include "FACE.H"

class MyObject
{
public:

  struct CellFace
  {
    MyFace    *Info;
    CellFace  *Next;
  };

  struct CellPoint
  {
    MyTriplet   *Info;
    CellPoint   *Next;
  };

  MyTriplet ViewVector;
  MyTriplet Center;
  short     Rotated;
  short     Sorted;

  CellFace   *HeadFace;
  CellPoint  *HeadPoint;
  long int   NbFaces;
  long int   NbPoints;
  short      Visible;

  MyObject();
  virtual ~MyObject();

  virtual void      SetView(MyTriplet,float,float,float);
  virtual long int  GetMinZ();
  virtual void      Rotation(long int,long int,long int,int,float);
  virtual void      Translation(long int,long int,long int);
  virtual void      Draw(HDC,long int,long int,long int,long int,long int,int,int,short,MyTriplet);
  virtual void      AddFace(MyFace *);
  virtual int	    DeleteFace();
  virtual void      AddPoint(MyTriplet *);
  virtual int	    DeletePoint();

private:

  virtual void      SortFaces();
};

#endif