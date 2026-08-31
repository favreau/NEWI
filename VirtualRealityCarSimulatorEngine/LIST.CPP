

#include "LIST.H"

//--------------------------------------------------------------------------
// OList constructor
//--------------------------------------------------------------------------

OList::OList()
{
  Head = NULL;
  NbItems = 0;
}

//--------------------------------------------------------------------------
// OList destructor
//--------------------------------------------------------------------------
OList::~OList()
{
}

//--------------------------------------------------------------------------
void OList::InsertHead(MyObject *aItem,long aX,long aY,long aZ)
{
  Cell *C = (Cell *) malloc(sizeof(Cell));
  C->Info = aItem;
  C->x = aX;
  C->Px = aX;
  C->y = aY;
  C->Py = aY;
  C->z = aZ;
  C->Pz = aZ;
  C->Next = Head;
  Head = C;
  NbItems++;
}

//--------------------------------------------------------------------------
// insert aItem at the tail of the list
//--------------------------------------------------------------------------
void OList::InsertTail(MyObject *aItem,long aX,long aY,long aZ)
{
  Cell *C = (Cell *) malloc(sizeof(Cell));
  C->Info = aItem;
  C->x = C->Px = aX;
  C->y = C->Py = aY;
  C->z = C->Pz = aZ;
  C->Next = NULL;
  NbItems++;

  if (Head == NULL) // empty list
  {
    Head = C;
  }
  else
  {
    if (Head->Next == NULL) // One item
    {
      Head->Next = C;
    }
    else                       // more than one item
    {
      Cell *Current = Head;
      while (Current->Next != NULL)
      {
        Current = Current->Next;
      }
      Current->Next = C;
    }
  }
}

//--------------------------------------------------------------------------
// Delete the first item of the list
//--------------------------------------------------------------------------
int OList::DeleteHead()
{
  if (Head != NULL)
  {
    NbItems--;
    Cell *OldHead = Head;
    Head = Head->Next;
    free(OldHead);
    return 1; // non-empty list
  }
  else
  {
    return 0; // empty list
  }
}

//--------------------------------------------------------------------------
// Delete the last item of the list
//--------------------------------------------------------------------------
int OList::DeleteTail()
{
  if (Head != NULL) // empty list
  {
    NbItems--;
    if (Head->Next == NULL) // one item
    {
      free(Head);
      Head = NULL;
      return 1;
    }
    else                       // more than one item
    {
      // stop on the next-to-last cell so the tail can be unlinked
      Cell *Current = Head;
      while (Current->Next->Next != NULL)
      {
        Current = Current->Next;
      }
      free(Current->Next);
      Current->Next = NULL;
      return 1;
    }
  }
  else
    return 0;
}

//--------------------------------------------------------------------------
// Update (rotate and translate) every object depending on the view
// aP is the position of the viewer
// aX,aY,aZ are the angles along the Ox,Oy and Oz axes
//--------------------------------------------------------------------------
void OList::SetView(MyTriplet aP,float aX,float aY, float aZ)
{
  Cell *Current = Head;
  while (Current != NULL)
  {
    if (Current->Info->Rotated == FALSE)
    {
      MyTriplet lP;
      Current->Info->SetView(aP,aX,aY,aZ);
      Current->Info->Center.SetCoordinates(Current->Px,Current->Py,Current->Pz);
      Current->Info->Rotated = TRUE;
      Current->Info->Sorted  = FALSE;
    }
    Current = Current->Next;
  }
}

//--------------------------------------------------------------------------
// Update the center of each object to determine which ones are in the view
// plane
//--------------------------------------------------------------------------
void OList::SetViewCenters(MyTriplet aP,float aX,float aY, float aZ)
{
  Cell *Current = Head;
  while (Current != NULL)
  {
    MyTriplet lP;
    lP.SetCoordinates(Current->x,Current->y,Current->z);
    lP.Translation(-aP.Px,-aP.Py,-aP.Pz);
    if (aX != 0) lP.Rotation(0,0,0,1,-aX);
    if (aY != 0) lP.Rotation(0,0,0,2,-aY);
    if (aZ != 0) lP.Rotation(0,0,0,3,-aZ);
    Current->Px = lP.Px;
    Current->Py = lP.Py;
    Current->Pz = lP.Pz;
    Current->Info->Rotated = FALSE;
    Current = Current->Next;
  }
}

//--------------------------------------------------------------------------
// Rotate every object of the list
//--------------------------------------------------------------------------
void OList::Rotation(long int aCenterX,long int aCenterY, long int aCenterZ,
		     int aAxe, float aAngle)
{
  Cell *Current = Head;
  while (Current != NULL)
  {
    Current->Info->Rotation(aCenterX,aCenterY,aCenterZ,aAxe,aAngle);
    Current = Current->Next;
  }
}

//--------------------------------------------------------------------------
// Translate every object of the list
//--------------------------------------------------------------------------
void OList::Translation(long int aX,long int aY, long int aZ)
{
  Cell *Current = Head;
  while (Current != NULL)
  {
    Current->Info->Translation(aX,aY,aZ);
    Current = Current->Next;
  }
}

//--------------------------------------------------------------------------
// Draw the whole list onto the screen
//--------------------------------------------------------------------------
void OList::Draw( HDC aDC,long int CenterX,long int CenterY,
		  long int aX,long int aY,long int aZ,
		  int aMode, int aInverted,
                  short aMap,
		  MyTriplet aSolPos)
{
  Cell *Current = Head;
  if (aMap == 0 && aMode == 5)
  {
    while (Current != NULL)
    {
      Current->Info->Draw(aDC,CenterX,CenterY,aX+Current->Px,aY+Current->Py,aZ+Current->Pz,aMode,aInverted,aMap,aSolPos);
      Current = Current->Next;
    }
  }
  if (aMode==5) aMode=4;
  Current = Head;
  while (Current != NULL)
  {
    Current->Info->Draw(aDC,CenterX,CenterY,aX+Current->Px,aY+Current->Py,aZ+Current->Pz,aMode,aInverted,aMap,aSolPos);
    Current = Current->Next;
  }
}

//--------------------------------------------------------------------------
// Sort the list on decreasing z-values
//--------------------------------------------------------------------------
void OList::SortList()
{
  if (Head != NULL && Head->Next != NULL)
  {
    Cell *Cur = Head;
    Cell *CurNext  = Head->Next;
    long int MaxCell = NbItems;
    long int Cpt     = 0;

    while (MaxCell != 0)
    {
      Cpt     = 0;
      MaxCell = 0;
      while (CurNext != NULL)
      {
	Cpt++;
	if (CurNext->Pz > Cur->Pz)
	{
	  MaxCell = Cpt;
          // update Pointer
	  MyObject *PTemp = CurNext->Info;
  	  CurNext->Info = Cur->Info;
  	  Cur->Info = PTemp; 
	  long int Temp;
          // Update Coordinates;
	  Temp = CurNext->x;
	  CurNext->x = Cur->x;
	  Cur->x = Temp;
	  Temp = CurNext->y;
	  CurNext->y = Cur->y;
	  Cur->y = Temp;
	  Temp = CurNext->z;
	  CurNext->z = Cur->z;
  	  Cur->z = Temp; 

	  Temp = CurNext->Px;
  	  CurNext->Px = Cur->Px;
  	  Cur->Px = Temp; 
	  Temp = CurNext->Py;
  	  CurNext->Py = Cur->Py;
  	  Cur->Py = Temp; 
	  Temp = CurNext->Pz;
  	  CurNext->Pz = Cur->Pz;
  	  Cur->Pz = Temp; 
	}
	Cur = CurNext;
	CurNext = CurNext->Next;
      }
      Cur = Head;
      CurNext  = Head->Next;
    }
  }
}

//--------------------------------------------------------------------------
// Create the list of the objects in the current view 
//--------------------------------------------------------------------------
void OList::SelectView( OList *aViewList)
{
  Cell *Current = Head;
  while (Current != NULL)
  {
    MyTriplet lP;
    lP.SetCoordinates(Current->Px,Current->Py,Current->Pz);
    if (Triangle(lP) == 0)
    {
      aViewList->InsertHead(Current->Info,Current->Px,Current->Py,Current->Pz);
    }
    Current = Current->Next;
  }
}
