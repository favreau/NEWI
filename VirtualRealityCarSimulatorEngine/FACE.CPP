
//--------------------------------------------------------------------------
// Unit containing every method used by the object FACE
//--------------------------------------------------------------------------
#include "face.h"

//===================================================================================================================//
// Constructor
//--------------------------------------------------------------------------
MyFace::MyFace()
{
  Red = Green = Blue = 255;
  ViewVector.SetCoordinates(0,0,100);
}
 
//===================================================================================================================//
// Destructor
//--------------------------------------------------------------------------
MyFace::~MyFace()
{
}

//===================================================================================================================//
// Initialise 4 side face with prameters
//--------------------------------------------------------------------------
void MyFace::InitFace( MyTriplet *aP1,MyTriplet *aP2, MyTriplet *aP3, MyTriplet *aP4)
{
  NbPoints = 4;
  P[0] = aP1; P[1] = aP2; P[2] = aP3; P[3] = aP4;
}

//===================================================================================================================//
// Initialise 3 side face with parameters
//--------------------------------------------------------------------------
void MyFace::InitFace( MyTriplet *aP1,MyTriplet *aP2, MyTriplet *aP3)
{
  NbPoints = 3;
  P[0] = aP1; P[1] = aP2; P[2] = aP3;
}

//===================================================================================================================//
// Set face colour
//--------------------------------------------------------------------------
void MyFace::SetColor(int aRed,int aGreen, int aBlue)
{
  Red   = aRed;
  Green = aGreen;
  Blue  = aBlue;
}

//===================================================================================================================//
// Get the minimum z-value of the face after translation on the z axis
//--------------------------------------------------------------------------
long MyFace::GetMinZ(long aZ)
{
  long a;
  if (NbPoints == 4)
  {
    a = (P[3]->Pz+aZ <= P[2]->Pz+aZ) ? P[3]->Pz+aZ : P[2]->Pz+aZ;
  }
  else
  {
    a = P[2]->Pz+aZ;
  }
  long b = (P[1]->Pz+aZ <= P[0]->Pz+aZ) ? P[1]->Pz+aZ : P[0]->Pz+aZ;
  long MinZ = (a<b) ? a : b;
  return MinZ;
}

//===================================================================================================================//
// Get the minimum z-value of the face
//===================================================================================================================//
long int MyFace::GetMinZ()
{
  long a;
  if (NbPoints == 4)
  {
    a = (P[3]->Pz <= P[2]->Pz) ? P[3]->Pz : P[2]->Pz;
  }
  else
  {
    a = P[2]->Pz;
  }
  long b = (P[1]->Pz <= P[0]->Pz) ? P[1]->Pz : P[0]->Pz;
  long MinZ = (a<b) ? a : b;
  return MinZ;
}
//===================================================================================================================//
// Get Maximum z-value of the face
//===================================================================================================================//
long int MyFace::GetMaxZ()
{
  long a;
  if (NbPoints == 4)
  {
    a = (P[3]->Pz >= P[2]->Pz) ? P[3]->Pz : P[2]->Pz;
  }
  else
  {
    a = P[2]->Pz;
  }
  long b = (P[1]->Pz >= P[0]->Pz) ? P[1]->Pz : P[0]->Pz;
  long MinZ = (a>b) ? a : b;
  return MinZ;
}

//===================================================================================================================//
// Draw the face onto the screen
//===================================================================================================================//
void MyFace::Draw(HDC aDC,long int CenterX,long int CenterY,long int Ax,long int Ay,long int Az,int aMode,int aInverted,short aMap,MyTriplet aSolPos)
{
  double MinZ = GetMinZ();
  float lScalaire;
  float lNormes;
  float lCos;
  float lColor;
  Poly  Face,ClipFace;

  Face.NumVert = NbPoints;

  // Clip Face on x and z if needed

  if (P[0]->Py <= 0 && P[1]->Py <= 0 && P[2]->Py <= 0 && aMap==0 && MinZ+Az<=200)
  {
    // Piece of road
    for (int i=0; i< NbPoints;i++)
    {
      Face.Verts[i].x = P[i]->Px+Ax;
      Face.Verts[i].y = P[i]->Pz+Az;
    }
    ClipFace = ClipPoly(Face);
    for (i=0; i<=ClipFace.NumVert; i++)
    {
      MyTriplet lP;
      lP.SetCoordinates(ClipFace.Verts[i].x,P[0]->Py+Ay,ClipFace.Verts[i].y);
      lP.Projection(0,0,0);
      if (aInverted == 0) ClipFace.Verts[i].x = CenterX+lP.P.x;
      else                ClipFace.Verts[i].x = CenterX-lP.P.x;
      ClipFace.Verts[i].y = CenterY-lP.P.y;
    }
  }
  else
  {
    // Object different from a piece of road
    if (MinZ+Az > 50)
    {
      MyTriplet lP;
      ClipFace.NumVert = NbPoints;
      for (int i=0; i< NbPoints; i++)
      {
	lP = *P[i];
	lP.Projection(Ax,Ay,Az);
	if (aInverted == 0) ClipFace.Verts[i].x = CenterX+lP.P.x;
	else                ClipFace.Verts[i].x = CenterX-lP.P.x;
	ClipFace.Verts[i].y = CenterY-lP.P.y;
      }
    }
    else
    {
      ClipFace.NumVert = 0;
    }
  }

  // Display clipped polygon

  ViewVector.SetCoordinates(P[0]->Px+Ax,P[0]->Py+Ay,P[0]->Pz+Az);
    switch (aMode)
    {
  
      case 1: //HIDDEN FACES
      {
        InitNormalVector(P[0],P[1],P[2]);
        lScalaire = Scalaire(ViewVector,NormalVector);
	if (lScalaire <= 0)
	{
	  // display face
	MoveTo(aDC,ClipFace.Verts[0].x,ClipFace.Verts[0].y);
	for (int i=1; i<ClipFace.NumVert; i++)
	{
	  LineTo(aDC,ClipFace.Verts[i].x,ClipFace.Verts[i].y);
	}
	LineTo(aDC,ClipFace.Verts[0].x,ClipFace.Verts[0].y);
	}
      }
      break;

      case 2: //PAINTED HIDDEN FACES
      {
        InitNormalVector(P[0],P[1],P[2]);
        lScalaire = Scalaire(ViewVector,NormalVector);
	if (lScalaire <= 0)
	{
	  // display face
	  Polygon(aDC,ClipFace.Verts,ClipFace.NumVert);
	}
      }
      break;

      case 3: //HIDDEN COLORED FACES
      {
	InitNormalVector(P[0],P[1],P[2]);
        lScalaire = Scalaire(ViewVector,NormalVector);
	if (lScalaire <= 0)
	{
	  //HBITMAP  HBmp = LoadBitmap(hInstance,"Wall");
	  HBRUSH   MyBrush, MyBrushOld;

	  //MyBrush    = CreatePatternBrush(HBmp);
	  HPEN MyPen      = CreatePen(PS_SOLID,1,RGB(Red,Green,Blue));
	  HPEN MyPenOld   = HPEN(SelectObject(aDC,MyPen));
	  MyBrush = CreateSolidBrush(RGB(Red,Green,Blue));
	  MyBrushOld = HBRUSH(SelectObject(aDC,MyBrush));
	  Polygon(aDC,ClipFace.Verts,ClipFace.NumVert);
	  SelectObject(aDC,MyBrushOld);
	  SelectObject(aDC,MyPenOld);
	  DeleteObject(MyBrush);
	  DeleteObject(MyPen);
	}
      }
      break;
  
      case 4: //LIGHTED FACES
      {

	HBRUSH    MyBrush, MyBrushOld;
	HPEN      MyPen,MyPenOld;
	float lDist = MinZ/KHorizon;
	InitNormalVector(P[0],P[1],P[2]);
	// is the face visible ?
	lScalaire = Scalaire(ViewVector,NormalVector);

	if (lScalaire <= 0)
	{
	  MyTriplet SolVector;
	  SolVector.SetCoordinates(-aSolPos.Px,-aSolPos.Py,-aSolPos.Pz);
	  lScalaire = Scalaire(SolVector,NormalVector);
	  lNormes   = Norme(SolVector)*Norme(NormalVector);
          lCos  = (lNormes != 0) ? (lScalaire / lNormes) : 0;
	  lColor = (lCos <= 0) ? fabs(lCos)*(1-lDist) : 0;
	  int R = (Red  >0) ? AMBIENT_LIGHT + (Red   - AMBIENT_LIGHT)*lColor : 0;
	  int G = (Green>0) ? AMBIENT_LIGHT + (Green - AMBIENT_LIGHT)*lColor : 0;
	  int B = (Blue >0) ? AMBIENT_LIGHT + (Blue  - AMBIENT_LIGHT)*lColor : 0;
	  //
	  MyPen      = CreatePen(PS_SOLID,1,RGB(R,G,B));
	  MyPenOld   = HPEN(SelectObject(aDC,MyPen));
	  MyBrush    = CreateSolidBrush(RGB(R,G,B));
	  MyBrushOld = HBRUSH(SelectObject(aDC,MyBrush));
	  Polygon(aDC,ClipFace.Verts,ClipFace.NumVert);
 	  SelectObject(aDC,MyBrushOld);
	  SelectObject(aDC,MyPenOld);
	  DeleteObject(MyBrush);
	  DeleteObject(MyPen);
	}
      }
      break;

      case 5: // SHADOWS
      {
	Shadows(aDC,CenterX,CenterY,Ax,Ay,Az,aInverted,aSolPos);
      }
      break;
  
      default : // SIMPLE DISPLAY
      {
	MoveTo(aDC,ClipFace.Verts[0].x,ClipFace.Verts[0].y);
	for (int i=1; i<ClipFace.NumVert; i++)
	{
	  LineTo(aDC,ClipFace.Verts[i].x,ClipFace.Verts[i].y);
	}
	LineTo(aDC,ClipFace.Verts[0].x,ClipFace.Verts[0].y);
      }
    }
}

//===================================================================================================================//

// PRIVATE METHODS

//===================================================================================================================//


//===================================================================================================================//
// Initialise Normal vector
//===================================================================================================================//
void MyFace::InitNormalVector( MyTriplet *aP1,MyTriplet *aP2,MyTriplet *aP3)
{
  NormalVector.SetCoordinates(long(aP2->Py-aP1->Py)*long(aP3->Pz-aP2->Pz) - long(aP2->Pz-aP1->Pz)*long(aP3->Py-aP2->Py),
			      long(aP2->Pz-aP1->Pz)*long(aP3->Px-aP2->Px) - long(aP2->Px-aP1->Px)*long(aP3->Pz-aP2->Pz),
			      long(aP2->Px-aP1->Px)*long(aP3->Py-aP2->Py) - long(aP2->Py-aP1->Py)*long(aP3->Px-aP2->Px));
}

//===================================================================================================================//
// Compute the length of aTriplet
//===================================================================================================================//
float MyFace::Norme(MyTriplet aTriplet)
{
  return sqrt(aTriplet.Px*aTriplet.Px + aTriplet.Py*aTriplet.Py + aTriplet.Pz*aTriplet.Pz);
}

//===================================================================================================================//
// compute the dot product of aP1 and aP2
//===================================================================================================================//
float MyFace::Scalaire(MyTriplet aP1, MyTriplet aP2)
{
  return (aP1.Px*aP2.Px + aP1.Py*aP2.Py + aP1.Pz*aP2.Pz);
}

//===================================================================================================================//
// Return the frame surrounding the face
//===================================================================================================================//
RECT MyFace::MiniMax(POINT aP1,POINT aP2,POINT aP3,POINT aP4)
{
  RECT  R;
  long int a = (aP1.x<aP2.x) ? aP1.x : aP2.x;
  long int b = (aP3.x<aP4.x) ? aP3.x : aP4.x;
  R.left = (a<b) ? a:b;

  a = (aP1.y<aP2.y) ? aP1.y : aP2.y;
  b = (aP3.y<aP4.y) ? aP3.y : aP4.y;
  R.top = (a<b) ? a:b;

  a = (aP1.x>aP2.x) ? aP1.x : aP2.x;
  b = (aP3.x>aP4.x) ? aP3.x : aP4.x;
  R.right = (a>b) ? a:b;

  a = (aP1.y>aP2.y) ? aP1.y : aP2.y;
  b = (aP3.y>aP4.y) ? aP3.y : aP4.y;
  R.bottom = (a>b) ? a:b;

return R;
}

//===================================================================================================================//
// Draw the shadow of the face
// returns 0 if no shadow
//===================================================================================================================//
short MyFace::Shadows(HDC aDC,long int CenterX,long int CenterY,long int Ax,long int Ay,long int Az,int aInverted,MyTriplet aSolPos)
{
  short ReturnValue = 1;
  MyTriplet SolVector;
  InitNormalVector(P[0],P[1],P[2]);
  SolVector.SetCoordinates(-aSolPos.Px,-aSolPos.Py,-aSolPos.Pz);
  if (Scalaire(SolVector,NormalVector) < 0 && !(P[0]->Py == 0 && P[1]->Py == 0 && P[2]->Py == 0))
  {
    MyTriplet ShadowPoints[4];
    POINT     ShadowDis[4];
    for (int i=0; i< NbPoints; i++)
    {
      if (P[i]->Px+Ax == aSolPos.Px)
      {
	ShadowPoints[i].SetCoordinates(P[i]->Px+Ax,0,P[i]->Pz+Az);
      } // endif
      else
        if (P[i]->Py == aSolPos.Py)
        {
          ReturnValue = 0;
        }  // endif
        else
  	if (P[i]->Pz+Az == aSolPos.Pz)
  	{
	  float a1 = (P[i]->Py-aSolPos.Py)/(P[i]->Px+Ax-aSolPos.Px);
	  float b1 = P[i]->Py-a1*(P[i]->Px+Ax);
	  ShadowPoints[i].SetCoordinates(-b1/a1,0,P[i]->Pz+Az);
  	}
	else
          {
	  float a1 = float(P[i]->Py-aSolPos.Py)/float(P[i]->Px+Ax-aSolPos.Px);
	  float b1 = P[i]->Py-a1*(P[i]->Px+Ax);
	  float a2 = float(P[i]->Pz+Az-aSolPos.Pz)/float(P[i]->Px+Ax-aSolPos.Px);
	  float b2 = P[i]->Pz+Az-a2*(P[i]->Px+Ax);
	  long int x = -b1 / a1;
  	  long int z = x*a2+b2;
	  ShadowPoints[i].SetCoordinates(x,-KCARHIGH,z);
  	}  // endif
    }
    Poly  Face,ClipFace;

    Face.NumVert = NbPoints;
    // Clip Face on x and z
    for (i=0; i< NbPoints;i++)
    {
      Face.Verts[i].x = ShadowPoints[i].Px;
      Face.Verts[i].y = ShadowPoints[i].Pz;
    }
    ClipFace = ClipPoly(Face);
    for (i=0; i < ClipFace.NumVert; i++)
    {
      MyTriplet lP;
      lP.SetCoordinates(ClipFace.Verts[i].x,0,ClipFace.Verts[i].y);
      lP.Projection(0,Ay,0);
      if (aInverted == 0) ClipFace.Verts[i].x = CenterX+lP.P.x;
      else                ClipFace.Verts[i].x = CenterX-lP.P.x;
      ClipFace.Verts[i].y = CenterY-lP.P.y;
    }
    // Shadows
    HPEN MyPen      = CreatePen(PS_SOLID,1,RGB(0,0,0));
    HPEN MyPenOld   = HPEN(SelectObject(aDC,MyPen));
    HBRUSH MyBrush    = CreateSolidBrush(RGB(0,0,0));
    HBRUSH MyBrushOld = HBRUSH(SelectObject(aDC,MyBrush));
    Polygon(aDC,ClipFace.Verts,ClipFace.NumVert);
    SelectObject(aDC,MyBrushOld);
    DeleteObject(MyBrush);
    SelectObject(aDC,MyPenOld);
    DeleteObject(MyPen);
  }
  return ReturnValue;
}
