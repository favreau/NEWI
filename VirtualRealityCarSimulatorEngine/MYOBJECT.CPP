
#include <alloc.h>
#include "MYOBJECT.H"
#include "TRIANGLE.H"

//===================================================================================================================//
// Constructor
//===================================================================================================================//
MyObject::MyObject()
{
  NbFaces     = 0;
  NbPoints    = 0;
  HeadFace    = NULL;
  HeadPoint   = NULL;
  Rotated     = FALSE;
  Sorted      = FALSE;
}
 
//===================================================================================================================//
// Destructor
//===================================================================================================================//
MyObject::~MyObject()
{
}

//===================================================================================================================//
// Update object depending on the view
//===================================================================================================================//
void MyObject::SetView(MyTriplet aPosition, float aOrientationX, float aOrientationY, float aOrientationZ)
{
  CellPoint *Cur = HeadPoint;
  while (Cur != NULL)
  {
    Cur->Info->Reset();
    if (aOrientationX != 0) Cur->Info->Rotation(0,0,0,1,-aOrientationX);
    if (aOrientationY != 0) Cur->Info->Rotation(0,0,0,2,-aOrientationY);
    if (aOrientationZ != 0) Cur->Info->Rotation(0,0,0,3,-aOrientationZ);
    Cur = Cur->Next;
  }
}

//===================================================================================================================//
// Rotate object
//===================================================================================================================//
void MyObject::Rotation(long int aXCenter, long int aYCenter, long int aZCenter, int aAxe, float aAngle)
{
  CellPoint *Cur = HeadPoint;
  while (Cur != NULL)
  {
    Cur->Info->Rotation(aXCenter,aYCenter,aZCenter,aAxe,aAngle);
    Cur = Cur->Next;
  }
}

//===================================================================================================================//
// Translate object
//===================================================================================================================//
void MyObject::Translation(long int aX,long int aY,long int aZ)
{
  CellPoint *Cur = HeadPoint;
  while (Cur != NULL)
  {
    Cur->Info->Translation(aX,aY,aZ);
    Cur = Cur->Next;
  }
}

//===================================================================================================================//
// Draw object onto the screen
//===================================================================================================================//
void MyObject::Draw(HDC aDC,long int aCenterX,long int aCenterY,long int aX,long int aY,long int aZ,int aMode,int aInverted,short aMap,MyTriplet aSolPos)
{
  if (aMode != 1 && Sorted == FALSE) SortFaces();
  CellFace *Cur = HeadFace;
  while (Cur != NULL)
  {
      Cur->Info->Draw(aDC,aCenterX,aCenterY,aX,aY,aZ,aMode,aInverted,aMap,aSolPos);
      Cur = Cur->Next;
  }
}

//===================================================================================================================//
// Add face to the object
//===================================================================================================================//
void MyObject::AddFace(MyFace *aItem)
{
  CellFace *C = (CellFace *) malloc(sizeof(CellFace));
  C->Info = aItem;
  C->Next = HeadFace;
  HeadFace = C;
  NbFaces++;
}

//===================================================================================================================//
// Add point to the object
//===================================================================================================================//
void MyObject::AddPoint(MyTriplet *aItem)
{
  CellPoint *C = (CellPoint *) malloc(sizeof(CellPoint));
  C->Info = aItem;
  C->Next = HeadPoint;
  HeadPoint = C;
  NbPoints++;
}

//===================================================================================================================//
// Delete first face of the face list
//===================================================================================================================//
int MyObject::DeleteFace()
{
  if (HeadFace != NULL)
  {
    NbFaces--;
    CellFace *OldHeadFace = HeadFace;
    HeadFace = HeadFace->Next;
    free(OldHeadFace);
    return 1; // non-empty list
  }
  else
  {
    return 0; // empty list
  }
}

//===================================================================================================================//
// Delete first point of the point list
//===================================================================================================================//
int MyObject::DeletePoint()
{
  if (HeadPoint != NULL)
  {
    NbPoints--;
    CellPoint *OldHeadPoint = HeadPoint;
    HeadPoint = HeadPoint->Next;
    free(OldHeadPoint);
    return 1; // non-empty list
  }
  else
  {
    return 0; // empty list
  }
}

//===================================================================================================================//
// Sort faces on decreasing z-values
//===================================================================================================================//
void MyObject::SortFaces()
{
  if (HeadFace != NULL && HeadFace->Next != NULL)
  {
    CellFace *Cur = HeadFace;
    CellFace *CurNext  = HeadFace->Next;
    long int MaxCellFace = NbFaces;
    long int Cpt     = 0;

    while (MaxCellFace != 0)
    {
      Cpt     = 0;
      MaxCellFace = 0;
      while (CurNext != NULL)
      {
        Cpt++;
	if (CurNext->Info->GetMinZ() < Cur->Info->GetMinZ())
	{
	  MaxCellFace = Cpt;
	  MyFace *Temp = CurNext->Info;
	  CurNext->Info = Cur->Info;
  	  Cur->Info = Temp; 
	}

	Cur = CurNext;
	CurNext = CurNext->Next;
      }
      Cur = HeadFace;
      CurNext  = HeadFace->Next;
    }
  }
  Sorted = TRUE;
}

//===================================================================================================================//
// Get minimum z-value of the object
//===================================================================================================================//
long int MyObject::GetMinZ()
{
  long int MinZ = 0;
  if (HeadPoint != NULL)
  {
    CellPoint *Cur = HeadPoint;
    MinZ = HeadPoint->Info->Pz;
    while (Cur != NULL)
    {
      long int lz = Cur->Info->Pz;
      MinZ = (lz < MinZ) ? lz : MinZ;
      Cur = Cur->Next;
    }
  }
  return MinZ;
}
