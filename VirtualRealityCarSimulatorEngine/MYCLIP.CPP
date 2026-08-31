
#include "MYCLIP.H"

// ========================================================================
// POLYGON CLIPPING UNIT
// ========================================================================

// ========================================================================
// Add vertex to the clipped polygon
// ========================================================================
void AddVertex(POINT aP,Poly *aC)
{
  aC->Verts[aC->NumVert] = aP;
  aC->NumVert++;
}

// ========================================================================
// Test if aP is inside the window
// ========================================================================
short Inside(POINT aP, int aSide)
{
  short ReturnValue = FALSE;
  switch (aSide)
  {
    case CLIPTOP    :
    {
      ReturnValue = (aP.y >= YVMin) ? TRUE : FALSE ;
    }
    break;
    case CLIPBOTTOM :
    {
      ReturnValue = (aP.y <= YVMax) ? TRUE : FALSE ;
    }
    break;
    case CLIPLEFT   :
    {
      ReturnValue = (aP.x >= XVMin) ? TRUE : FALSE ;
    }
    break;
    case CLIPRIGHT  :
    {
      ReturnValue = (aP.x <= XVMax) ? TRUE : FALSE ;
    }
    break;
  }
  return ReturnValue;
}

// ========================================================================
// Find intersection between the egde and the polygon and the edge of the
// window
// ========================================================================
void FindIntersect(POINT aFirst,POINT aLast, int aSide, POINT *aIntersect)
{
  if (aFirst.x == aLast.x)  // Vertical line
  {
    if (aSide == CLIPBOTTOM)
    {
      aIntersect->x = aFirst.x;
      aIntersect->y = YVMax;
    }
    else
      if (aSide == CLIPTOP)
      {
        aIntersect->x = aFirst.x;
        aIntersect->y = YVMin;
      }
  }
  else // Not Vertical Line
  {
    if (aFirst.y == aLast.y) // Horizontal Line
    {
      if (aSide == CLIPLEFT)
      {
        aIntersect->x = XVMin;
        aIntersect->y = aFirst.y;
      }
      else
      {
	if (aSide == CLIPRIGHT)
	{
  	  aIntersect->x = XVMax;
          aIntersect->y = aFirst.y;
	}
      }
    } // Not Horizontal Line
    else // Diagonal
    {
      float a = float(aLast.y-aFirst.y)/float(aLast.x-aFirst.x);
      float b = aFirst.y - a*aFirst.x;
      switch (aSide)
      {
	case CLIPLEFT:
	{
	  aIntersect->x = XVMin;
          aIntersect->y = a*XVMin+b;
	}
	break;
	case CLIPRIGHT:
	{
	  aIntersect->x = XVMax;
	  aIntersect->y = a*XVMax+b;
	}
	break;
	case CLIPTOP:
	{
	  aIntersect->x = (YVMin-b)/a;
          aIntersect->y = YVMin;
	}
	break;
	case CLIPBOTTOM:
	{
	  aIntersect->x = (YVMax-b)/a;
	  aIntersect->y = YVMax;
	}
        break;
      }
    }
  }
}

// ========================================================================
// Clip the polygon aP1 on the side given in parameter (aSide). The clipped
// polygon is return in aP2
// ========================================================================
void ClipOne(Poly aP1, Poly *aP2, int aSide)
{
  POINT First,Last;
  POINT Intersect;
  int   VertexNumber;

  aP2->NumVert = 0;
  // A previous pass can clip the polygon away entirely; without this the
  // wrap-around below would read Verts[-1].
  if (aP1.NumVert <= 0) return;
  First = aP1.Verts[aP1.NumVert-1];
  for (VertexNumber = 0; VertexNumber < aP1.NumVert; VertexNumber++)
  {
    Last = aP1.Verts[VertexNumber];
    if (Inside(Last,aSide) == TRUE)
    {
      if (Inside(First,aSide) == TRUE)
      {
        AddVertex(Last,aP2);
      }
      else
      {

	FindIntersect(Last,First,aSide,&Intersect);
        AddVertex(Intersect,aP2);
	AddVertex(Last,aP2);
      }
    }
    else
    {
      if (Inside(First,aSide) == TRUE)
      {
        FindIntersect(First,Last,aSide,&Intersect);
        AddVertex(Intersect,aP2);
      }
    }
    First = Last;
  }
}

// ========================================================================
// Polygon Clipping main procedure
// ========================================================================
Poly ClipPoly(Poly aP)
{
  Poly aC;
  for (int Side=CLIPLEFT; Side<=CLIPTOP; Side++)
  {
    ClipOne(aP,&aC,Side);
    aP = aC;
  }
  return aC;
}