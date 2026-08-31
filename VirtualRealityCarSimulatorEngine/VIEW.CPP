
//===================================================================================================================//
// Module used to draw the current view of the virtual world onto the screen
//===================================================================================================================//

#include "VIEW.H"

//===================================================================================================================//
// Draw the current vue onto the screen 
//===================================================================================================================//
void MyView(HDC aMDC, MyTriplet aPos, OList *aL,
	    float aX, float aY, float aZ,
	    POINT aWindowPos,
	    POINT aWindowSize,
	    short aGraphicLevel,
	    short aInverted,
	    short aMap,
	    MyTriplet aSolPos)
{
   OList   ViewList;
   float   SolCol;
   HRGN RectRgn = CreateRectRgn(aWindowPos.x,aWindowPos.y,aWindowPos.x+aWindowSize.x,aWindowPos.y+aWindowSize.y);
   SelectClipRgn(aMDC,RectRgn);

   if (aGraphicLevel > 2)
   {
     // coloured view, we need to draw the earth and the sky
     RECT Sky,Earth;

     Sky.top    = aWindowPos.y;
     Sky.left   = aWindowPos.x;
     Earth.bottom = aWindowPos.y+aWindowSize.y;
     Earth.right  = aWindowPos.x+aWindowSize.x;
     SolCol = (aGraphicLevel > 4) ? (AMBIENT_LIGHT+(255-AMBIENT_LIGHT)*aSolPos.Py/20000) : 255;
     if (aMap == 0)
     {
       Sky.bottom = aWindowPos.y+aWindowSize.y/2;
       Sky.right  = aWindowPos.x+aWindowSize.x;
       Earth.top  = aWindowPos.y+aWindowSize.y/2;
       Earth.left = aWindowPos.x;
       // draw Sky
       HBRUSH MySkyBrush = CreateSolidBrush(RGB(256-SolCol,SolCol/2,SolCol));
       FillRect(aMDC,&Sky,MySkyBrush);
       DeleteObject(MySkyBrush);
     }
     else
     {
       Earth.top    = aWindowPos.y;
       Earth.left   = aWindowPos.x;
     }
     // draw Earth
     HBRUSH MyEarthBrush = CreateSolidBrush(RGB(0,96,0));
     FillRect(aMDC,&Earth,MyEarthBrush);
     DeleteObject(MyEarthBrush);
   }
   else
   {
     // simple view, the earth and the sky are not drawn 
     RECT rc;
     rc.top = aWindowPos.y;
     rc.left = aWindowPos.x;
     rc.bottom = aWindowPos.y+aWindowSize.y;
     rc.right  = aWindowPos.x+aWindowSize.x;

     HBRUSH MyBrush = CreateSolidBrush(RGB(255,255,255));
     FillRect(aMDC,&rc,MyBrush);
     DeleteObject(MyBrush);

     if (aMap == 0)
     {
       // Horizon line
       MoveTo(aMDC,aWindowPos.x,aWindowPos.y+aWindowSize.y/2);
       LineTo(aMDC,aWindowPos.x+aWindowSize.x,aWindowPos.y+aWindowSize.y/2);
     }
   }


   // draw the new image

   // identifies the objects in the view
   aL->SetViewCenters(aPos,aX,aY,aZ);
   // Create the temporary list
   aL->SelectView(&ViewList);
   // updates the objects in the tempory list
   ViewList.SetView(aPos,aX,aY,aZ);
   // sort the temporary list
   ViewList.SortList();
   // turn the sun
   aSolPos.Reset();
   aSolPos.Rotation(aPos.Px,aPos.Py,aPos.Pz,1,-aX);
   aSolPos.Rotation(aPos.Px,aPos.Py,aPos.Pz,2,-aY);
   aSolPos.Rotation(aPos.Px,aPos.Py,aPos.Pz,3,-aZ);

   // display the temporary list
   ViewList.Draw(aMDC,aWindowPos.x+aWindowSize.x/2,aWindowPos.y+aWindowSize.y/2,0,0,0,aGraphicLevel,aInverted,aMap,aSolPos);

   if (aMap == 1)
   {
     // Arrow to represent the position and orientation of car on the map
     MoveTo(aMDC,aWindowSize.x/2  ,aWindowSize.y/2+20);
     LineTo(aMDC,aWindowSize.x/2  ,aWindowSize.y/2-20);
     MoveTo(aMDC,aWindowSize.x/2-5,aWindowSize.y/2-15);
     LineTo(aMDC,aWindowSize.x/2  ,aWindowSize.y/2-20);
     MoveTo(aMDC,aWindowSize.x/2+5,aWindowSize.y/2-15);
     LineTo(aMDC,aWindowSize.x/2  ,aWindowSize.y/2-20);
   }

   ViewList.~OList();
   DeleteObject(RectRgn);
}

//===================================================================================================================//
// Draw speed indication on speedometer 
//===================================================================================================================//
void SpeedoMeter(HDC aDC,long int aX,long int aY, short aRadius, long int aSpeed)
{
  MoveTo(aDC,aX,aY);
  LineTo(aDC,aX+aRadius*sin((fabs(aSpeed)-90)*M_PI/180),aY-aRadius*cos((fabs(aSpeed)-90)*M_PI/180));
}

