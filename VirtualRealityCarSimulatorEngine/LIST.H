
//===================================================================================================================//
// Header of the list module
//===================================================================================================================//

#if !defined(LIST_H)
#define LIST_H

#include <stdio.h>
#include <alloc.h>
#include <string.h>
#include <conio.h>

#include "myobject.h"
#include "triangle.h"

class OList
{
public:

  struct Cell
  {
    MyObject  *Info;
    long x ,y ,z;
    long Px,Py,Pz;
    Cell      *Next;
  };

  Cell *Head;
  long int NbItems;

  OList();
  virtual ~OList();

  virtual void InsertHead(MyObject *,long int,long int,long int);
  virtual void InsertTail(MyObject *,long int,long int,long int);
  virtual int  DeleteHead();
  virtual int  DeleteTail();
  virtual void SetView(MyTriplet,float,float,float);
  virtual void SetViewCenters(MyTriplet,float,float,float);
  virtual void Rotation(long int,long int,long int,int,float);
  virtual void Translation(long int,long int,long int);
  virtual void Draw(HDC,long int,long int,long int,long int,long int,int,int,short,MyTriplet);
  virtual void SelectView(OList *);
  virtual void SortList();

private:
};

#endif