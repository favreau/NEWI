     /********************************************************************
      *                                                                  *
      *   Source File: PROJECT.cpp                                       *
      *   Description: C++ Car Simulator                                 *
      *   Date:        Thu Feb 23 10:16:52 1995                          *
      *                                                                  *
      ********************************************************************/

#include <owl     .h>
#include <commdlg .h>
#include <math    .h>
#include <objstrm .h>
#include <io      .h>
#include <alloc   .h>
#include <iostream.h>
#include <string.h>

#include "PROJECID.h"
#include "PROJECT .h"
#include "PROJECls.h"

#include "view    .h"
#include "carapi  .h"
#include "wing    .h"

// -------------------------------
// List containing all the objects
OList     WorldList;

// -------------------------------
// virtual world
MyTriplet P[70];
MyFace    F[50];
MyObject  Objects[5];
int NbObjects=4; // NUmber of objects
int NbFaces [4]; // Number of faces per object
int NbPoints[4]; // Number of points per object
int NbPts = 0;   // Total number of faces
int NbFcs = 0;   // Total number of points

// -------------------------------
// variables for replay session
POINT     ReplayPos[301];
int       ReplayOri[301];
int       NbReplayImages = 300;
int       CurrentReplayImage;
int       CurrentPlay = 0;
short     ReplayMoveCar = FALSE;
short     ReplaySession = FALSE;

// -------------------------------
// Display Information
short     GraphicLevel = 0;
short     Colored;
short     LoadBitMap = 0;

// -------------------------------
float Time = 0.30; // for the sun!...

// -------------------------------
// Car & Head information
MyTriplet CarPos;
MyTriplet SolPosition;
int   HeadDirX,HeadDirY,HeadDirZ;
float CarDirX,CarDirY,CarDirZ;
int   Speed;
int   OldSpeed;
float DrivingWheel;

// -------------------------------
// Map
short    Map     = 0;
short    INITMAP = TRUE;
long int MapView = 9000;

// -------------------------------
// Connection
short    Connected = FALSE;

// -------------------------------
// Mirrors
short    Mirrors   = FALSE;
// Back mirror View
int BkMirrorX = 0;
int BkMirrorY = 181;
int BkMirrorZ = 0;

// Right mirror View
int RtMirrorX = 0;
int RtMirrorY = 170;
int RtMirrorZ = 0;

//=======================================================================
// FORWARD Declaration

   void InitObjects();
   void EndObjects();
   void ReadWorld(LPSTR aFileName);
   short far pascal extern HeadPosition(void);

//=======================================================================
// Define application class derived from  TApplication
//=======================================================================
class TPROJECT : public TApplication
{
public:

  TPROJECT(LPSTR AName, HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)
    : TApplication(AName, hInstance, hPrevInstance, lpCmdLine, nCmdShow) {};
    virtual void InitMainWindow();
    virtual void InitInstance();
};

void TPROJECT::InitInstance()
{
   TApplication::InitInstance();
   HAccTable = LoadAccelerators(hInstance, "project");

}
// Declare TMainWindow, a TWindow descendant
class TMainWindow : public TWindow
{
public:

   TMainWindow(PTWindowsObject AParent, LPSTR ATitle);
   ~TMainWindow();
   virtual void OPEN_FILE   (RTMessage Msg) = [CM_FIRST + IDM_OPEN_FILE   ];
   virtual void MAP_SWAP    (RTMessage Msg) = [CM_FIRST + IDM_MAP_SWAP    ];
   virtual void MAP_ZOOM_IN (RTMessage Msg) = [CM_FIRST + IDM_MAP_ZOOM_IN ];
   virtual void MAP_ZOOM_OUT(RTMessage Msg) = [CM_FIRST + IDM_MAP_ZOOM_OUT];
   virtual void LESS_DETAIL (RTMessage Msg) = [CM_FIRST + IDM_LESS_DETAIL ];
   virtual void MORE_DETAIL (RTMessage Msg) = [CM_FIRST + IDM_MORE_DETAIL ];
   virtual void COLOR_ON_OFF(RTMessage Msg) = [CM_FIRST + IDM_COLOR_ON_OFF];
   virtual void RIGHT       (RTMessage Msg) = [CM_FIRST + IDM_RIGHT       ];
   virtual void LEFT        (RTMessage Msg) = [CM_FIRST + IDM_LEFT        ];
   virtual void SPEED_UP    (RTMessage Msg) = [CM_FIRST + IDM_SPEED_UP    ];
   virtual void SPEED_DOWN  (RTMessage Msg) = [CM_FIRST + IDM_SPEED_DOWN  ];
   virtual void HEAD_RESET  (RTMessage Msg) = [CM_FIRST + IDM_HEAD_RESET  ];
   virtual void HEAD_RIGHT  (RTMessage Msg) = [CM_FIRST + IDM_HEAD_RIGHT  ];
   virtual void HEAD_LEFT   (RTMessage Msg) = [CM_FIRST + IDM_HEAD_LEFT   ];
   virtual void HELP        (RTMessage Msg) = [CM_FIRST + IDM_HELP        ];
   virtual void CONNECTION  (RTMessage Msg) = [CM_FIRST + IDM_CONNECTION  ];
   virtual void MIRRORS     (RTMessage Msg) = [CM_FIRST + IDM_MIRRORS     ];
   virtual void RTMIRRORLEFT (RTMessage Msg) = [CM_FIRST + IDM_RTMIRRORS_LEFT ];
   virtual void RTMIRRORRIGHT(RTMessage Msg) = [CM_FIRST + IDM_RTMIRRORS_RIGHT];
   virtual void BKMIRRORLEFT (RTMessage Msg) = [CM_FIRST + IDM_BKMIRRORS_LEFT ];
   virtual void BKMIRRORRIGHT(RTMessage Msg) = [CM_FIRST + IDM_BKMIRRORS_RIGHT];
   virtual void REPLAY      (RTMessage Msg) = [CM_FIRST + IDM_REPLAY      ];
   virtual void MyPaint     (void);
   virtual void MyTimer     (RTMessage Msg) = [WM_TIMER];
   virtual void SetupWindow (void);
};


//=======================================================================
// TMainWindow implementations:
//=======================================================================

//=======================================================================
// Define TMainWindow, a TWindow constructor
//=======================================================================
TMainWindow::TMainWindow(PTWindowsObject AParent, LPSTR ATitle)
                         : TWindow(AParent, ATitle)
{
   RECT MyRect;
   Attr.X = 120;
   Attr.Y = 40;
   Attr.W = 400;
   Attr.H = 400;
   Colored = 0;
}

//=======================================================================
// Define TMainWindow destructor
//=======================================================================
TMainWindow::~TMainWindow()
{
  KillTimer(HWindow,IDM_TIMER);
}

// ========================================================================
// SETUP MAIN WINDOW
// ========================================================================
void TMainWindow::SetupWindow()
{
  // Timer
  SetTimer(HWindow,IDM_TIMER,DELAY_TIMER,NULL);
}

// ========================================================================
// OPEN FILE
// ========================================================================
void TMainWindow::OPEN_FILE(RTMessage)
{
  OPENFILENAME ofn;
  char         csFile[256],csTitle[256];
  char         szDirName[256];
  UINT         i,cbString;
  char 	       chReplace;
  char         szFilter[256];
  HFILE        hf;

  KillTimer(HWindow,IDM_TIMER);
  GetWindowsDirectory(szDirName,sizeof(szDirName));
  csFile[0] = '\0';

  memset(&ofn,0,sizeof(OPENFILENAME));
  ofn.lpstrTitle      = "Load Virtual World";
  ofn.lStructSize     = sizeof(OPENFILENAME);
  ofn.hwndOwner       = HWindow;
  ofn.lpstrFilter     = "Car Simulator (*.WLD)";
  ofn.nFilterIndex    = 1;              
  ofn.lpstrFileTitle  = "*.wld";
  ofn.nMaxFileTitle   = 12;
  ofn.lpstrFile       = csFile;
  ofn.nMaxFile        = sizeof(csFile);
  ofn.lpstrFileTitle  = csTitle;
  ofn.nMaxFileTitle   = sizeof(csTitle);
  ofn.lpstrInitialDir = szDirName;
  ofn.Flags = OFN_SHOWHELP | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
  if (GetOpenFileName(&ofn))
  {
    ReadWorld(ofn.lpstrFileTitle);
  }
  SetTimer(HWindow,IDM_TIMER,DELAY_TIMER,NULL);
}

// ========================================================================
// TIMER PROCEDURE
// ========================================================================
void TMainWindow::MyTimer(RTMessage)
{
  // Car movements
  if (Speed > 0) CarDirY += DrivingWheel;
  else           CarDirY -= DrivingWheel;
  if (CarDirY >=  360) CarDirY = 0;
  if (CarDirY <= -360) CarDirY = 0;

  if (ReplaySession == FALSE)
  {
    // Replay session
    if (CurrentPlay >= NbReplayImages)
    {
      for (int i = 0; i<NbReplayImages; i++)
      {
	ReplayPos[i] = ReplayPos[i+1];
	ReplayOri[i] = ReplayOri[i+1];
      }
      ReplayPos[NbReplayImages].x = CarPos.Px;
      ReplayPos[NbReplayImages].y = CarPos.Pz;
      ReplayOri[NbReplayImages] = CarDirY;
      CurrentPlay = NbReplayImages;
    }
    else
    {
      ReplayPos[CurrentPlay].x = CarPos.Px;
      ReplayPos[CurrentPlay].y = CarPos.Pz;
      ReplayOri[CurrentPlay]   = CarDirY;
      CurrentPlay++;
    }
  }
  if (Connected == TRUE)
  {
    // Get head postition from other computer
    HeadDirY = HeadPosition()-128;
  }
  // move the sun
  SolPosition.SetCoordinates(-10000*cos(Time),10000*sin(Time),-10000);
  Time+=0.03;
  if (Time>=2.9) Time = 0.3;
  // Move the car
  CarPos.Translation(Speed*sin(CarDirY*M_PI/180),0,Speed*cos(CarDirY*M_PI/180));
  // Paint the screen
  MyPaint();
}

// ========================================================================
// SWAP BETWEEN CAR VIEW AND PLANE VIEW
// ========================================================================
void TMainWindow::MAP_SWAP(RTMessage)
{
  Map = (Map == 1) ? 0 : 1;
  if (Map==0) INITMAP = TRUE;
}

// ========================================================================
// DECREASES THE LEVEL OF DETAILS
// ========================================================================
void TMainWindow::LESS_DETAIL(RTMessage)
{
   GraphicLevel-= (GraphicLevel>0) ? 1 : 0;
   if (GraphicLevel <= 2) Colored = 0;
}

// ========================================================================
// INCREASES THE LEVEL OF DETAILS
// ========================================================================
void TMainWindow::MORE_DETAIL(RTMessage)
{
   GraphicLevel+= (GraphicLevel<5) ? 1 : 0;
}

void TMainWindow::COLOR_ON_OFF(RTMessage)
{
  if (GraphicLevel > 1) Colored = (Colored == 1) ? 0 : 1;
}

// ========================================================================
// Turn left
// ========================================================================
void TMainWindow::LEFT(RTMessage)
{
   DrivingWheel -= (DrivingWheel >= -2) ? 0.2 : -0.2;
}

// ========================================================================
// Turn right
// ========================================================================
void TMainWindow::RIGHT(RTMessage)
{
   DrivingWheel += (DrivingWheel <= 2) ? 0.1 : -0.1;
}

// ========================================================================
// MAP ZOOM IN
// ========================================================================
void TMainWindow::MAP_ZOOM_OUT(RTMessage)
{
  MapView += (MapView < 9000) ? 100 : 0;
}

// ========================================================================
// MAP ZOOM OUT
// ========================================================================
void TMainWindow::MAP_ZOOM_IN(RTMessage)
{
   MapView -= (MapView > 1000) ? 100 : 0;
}

// ========================================================================
// Increase Speed
// ========================================================================
void TMainWindow::SPEED_UP(RTMessage)
{
  Speed+= (Speed <= 280) ? 2 : 0;
}

// ========================================================================
// decrease Speed
// ========================================================================
void TMainWindow::SPEED_DOWN(RTMessage)
{
  Speed-= (Speed >= -30) ? 2 : 0;
}

// ========================================================================
// head STRAIGHT
// ========================================================================
void TMainWindow::HEAD_RESET(RTMessage)
{
   HeadDirY = 0;
}

// ========================================================================
// Turn head left
// ========================================================================
void TMainWindow::HEAD_LEFT(RTMessage)
{
   HeadDirY -= Step;
}

// ========================================================================
// Turn head right
// ========================================================================
void TMainWindow::HEAD_RIGHT(RTMessage)
{
   HeadDirY += Step;
}

// ========================================================================
// MIRRORS ON/OFF
// ========================================================================
void TMainWindow::MIRRORS(RTMessage)
{
  Mirrors = (Mirrors == TRUE) ? FALSE : TRUE;
}

// ========================================================================
void TMainWindow::RTMIRRORLEFT(RTMessage)
{
   RtMirrorY+= (RtMirrorY < 180) ? 1 : 0;
}

// ========================================================================
void TMainWindow::RTMIRRORRIGHT(RTMessage)
{
   RtMirrorY-= (RtMirrorY > 160) ? 1 : 0;
}

// ========================================================================
void TMainWindow::BKMIRRORLEFT(RTMessage)
{
   BkMirrorY+= (BkMirrorY < 190) ? 1 : 0;
}

// ========================================================================
void TMainWindow::BKMIRRORRIGHT(RTMessage)
{
   BkMirrorY-= (BkMirrorY > 170) ? 1 : 0;
}

// ========================================================================
// LAUNCH THE REPLAY SESSION
// ========================================================================
void TMainWindow::REPLAY(RTMessage)
{
  if (ReplaySession == FALSE)
  {
    KillTimer(HWindow,IDM_TIMER);
    ReplaySession = TRUE;
    MessageBox(HWindow,"Beginning of the replay session","View Replay",MB_ICONINFORMATION);
    ReplayMoveCar = TRUE;
    float dy=float(ReplayOri[CurrentPlay-1]-ReplayOri[0]) / float(CurrentPlay);
    float dx=(ReplayPos[CurrentPlay-1].x-ReplayPos[0].x) / CurrentPlay;
    float dz=(ReplayPos[CurrentPlay-1].y-ReplayPos[0].y) / CurrentPlay;

    for (int i=CurrentPlay-1;i>0; i-=4)
    {
      MyPaint();
      CarDirY   = ReplayOri[0]+float(i*dy);
      CarPos.Px = ReplayPos[0].x+i*dx;
      CarPos.Py += (i < CurrentPlay/2) ? -3 : 3;
      CarPos.Pz = ReplayPos[0].y+i*dz;
    }
    ReplayMoveCar = FALSE;
    CarPos.Py = KCARHIGH;

    CurrentReplayImage = 0;

    SetTimer(HWindow,IDM_TIMER,DELAY_TIMER,NULL);
  }
}

// ========================================================================
// PAINTS THE SCREEN
// ========================================================================
void TMainWindow::MyPaint()
{
   KillTimer(HWindow,IDM_TIMER);
   // DCs Declaration
   HDC  WindowDC;
   HDC  MemoryDC;
   // DCs initialisation
   WindowDC = GetDC(HWindow);
   MemoryDC = CreateCompatibleDC(WindowDC);

   RECT rc;
   HBITMAP Bit,BitOld;

   GetClientRect(HWindow,&rc);
   // Declarations
   POINT  BackPos , BackSize;
   POINT  RightPos, RightSize;

   if (ReplaySession == TRUE && ReplayMoveCar == FALSE)
   {
     CarPos.Px = ReplayPos[CurrentReplayImage].x;
     CarPos.Pz = ReplayPos[CurrentReplayImage].y;
     CarDirY = ReplayOri[CurrentReplayImage];
     CurrentReplayImage++;
     if (CurrentReplayImage > CurrentPlay)
     {
       if (MessageBox(HWindow,"End of replay session\n\nReplay ?","View Replay",MB_YESNO) == IDYES)
       {
	 CurrentReplayImage = 0;
       }
       else
       {
         ReplaySession = FALSE;
	 CarPos.Px = ReplayPos[CurrentPlay-1].x;
         CarPos.Pz = ReplayPos[CurrentPlay-1].y;
         CarDirY = ReplayOri[CurrentPlay-1];
	 CurrentPlay = 0;
       }
     }
   }

   int PosBkMirrorX = rc.right/2-(rc.right-rc.left)/8;
   int PosBkMirrorY = rc.top+20;
   int PosRtMirrorX = rc.right - (rc.right-rc.left)/8-10;
   int PosRtMirrorY = rc.top+20;

   // Create the Bitmap of the aMDC
   if (Colored == 1)
     Bit    = CreateCompatibleBitmap(WindowDC,rc.right-rc.left,rc.bottom-rc.top);
   else
     Bit    = CreateCompatibleBitmap(MemoryDC,rc.right-rc.left,rc.bottom-rc.top);
   // Select the Bitmap in memory and save the old one into BitOld
   BitOld = HBITMAP(SelectObject(MemoryDC,Bit));

   if (Map == 0)
   {
     BackPos.x  = rc.left;
     BackPos.y  = rc.top;
     BackSize.x = rc.right-rc.left;
     BackSize.y = rc.bottom-rc.top;
  
     // MAIN VIEW
     MyView(MemoryDC,
	   CarPos, &WorldList,
  	   CarDirX+HeadDirX,CarDirY+HeadDirY,CarDirZ+HeadDirZ,
  	   BackPos, BackSize,
	   GraphicLevel, 0,
	   0,
	   SolPosition);

     if (Mirrors == TRUE) // ARE THE MIRRORS TO BE DISPLAYED ?
     {

       BackPos.x  = PosBkMirrorX-HeadDirY*10;
       BackPos.y  = PosBkMirrorY;
       BackSize.x = (rc.right-rc.left)/4;
       BackSize.y = (rc.bottom-rc.top)/8;
       Rectangle(MemoryDC,BackPos.x-2,BackPos.y-2,BackPos.x+BackSize.x+2,BackPos.y+BackSize.y+2);
       RightPos.x  = PosRtMirrorX-HeadDirY*10;
       RightPos.y  = PosRtMirrorY;
       RightSize.x = (rc.right-rc.left)/8;
       RightSize.y = (rc.bottom-rc.top)/8;
       Rectangle(MemoryDC,RightPos.x-2,RightPos.y-2,RightPos.x+RightSize.x+2,RightPos.y+RightSize.y+2);

       // BACK MIRROR

       if (BackPos.x+BackSize.x > rc.left && BackSize.x < rc.right)
       {
	 MyView(MemoryDC,
	       CarPos, &WorldList,
	       CarDirX+BkMirrorX,CarDirY+BkMirrorY,CarDirZ+BkMirrorZ,
	       BackPos, BackSize,
	       GraphicLevel, 1,
               0,
	       SolPosition);
       }

       // RIGHT MIRROR

       if (RightPos.x+RightSize.x > rc.left && RightPos.x < rc.right)
       {
	 MyView(MemoryDC,
	       CarPos, &WorldList,
	       CarDirX+RtMirrorX,CarDirY+RtMirrorY,CarDirZ+RtMirrorZ,
	       RightPos, RightSize,
	       GraphicLevel, 1,
               0,
	       SolPosition);
       }
    }
  }
  else
  {
   // Draw map
   MyTriplet ViewPos;
   ViewPos.SetCoordinates(CarPos.Px,CarPos.Py+MapView,CarPos.Pz);

   BackPos.x  = rc.left;
   BackPos.y  = rc.top;
   BackSize.x = rc.right-rc.left;
   BackSize.y = rc.bottom-rc.top;

   MyView( MemoryDC,
	   ViewPos, &WorldList,
	   90,0,-CarDirY,
	   BackPos, BackSize,
	   GraphicLevel, 0,
	   1,
	   SolPosition);

   if (INITMAP == TRUE)
   {
     INITMAP = FALSE;
     for (int i=0; i<=(rc.bottom-rc.top);i+=5)
     {
       MoveTo(MemoryDC,rc.left,rc.bottom-2);
       LineTo(MemoryDC,rc.right,rc.bottom-2);
       //WinGBitBlt(WindowDC,rc.left,rc.top,rc.right-rc.left,i,MemoryDC,rc.left,rc.bottom-i);
       BitBlt(WindowDC,rc.left,rc.top,rc.right-rc.left,i,MemoryDC,rc.left,rc.bottom-i,SRCCOPY);
     }
   }
  }
  if (ReplaySession == TRUE)
  {
    if (ReplayMoveCar == TRUE) TextOut(MemoryDC,10,10,"MOVING CAR",10);
    else		       TextOut(MemoryDC,10,10,"REPLAY",6);
  }

  // Select the new Bitmap
  SelectObject(MemoryDC,Bit);

  // Transfer it to the screen
  //WinGBitBlt(WindowDC,rc.left,rc.top,rc.right,rc.bottom,MemoryDC,0,0);
  BitBlt(WindowDC,rc.left,rc.top,rc.right,rc.bottom,MemoryDC,0,0,SRCCOPY);
  // Restore the old Bitmap in memory
  SelectObject(MemoryDC,BitOld);

  // Delete old Bitmap
  DeleteObject(Bit);

  // DCs
  DeleteDC(MemoryDC);
  ReleaseDC(HWindow,WindowDC);

  // Timer on
  SetTimer(HWindow,IDM_TIMER,DELAY_TIMER,NULL);
}

// ========================================================================
// CONNECTION ON/OFF
// ========================================================================
void TMainWindow::CONNECTION(RTMessage)
{
  KillTimer(HWindow,IDM_TIMER);
  Connected = (Connected == TRUE) ? FALSE : TRUE;
  if (Connected == TRUE)
    MessageBox(HWindow,"Connection ON","Connection",MB_ICONINFORMATION);
  else
    MessageBox(HWindow,"Connection OFF","Connection",MB_ICONINFORMATION);
  SetTimer(HWindow,IDM_TIMER,DELAY_TIMER,NULL);
}

// ========================================================================
// HELP
// ========================================================================
void TMainWindow::HELP(RTMessage)
{
  KillTimer(HWindow,IDM_TIMER);
  MessageBox(HWindow,"F1 : More Details\nF2 : Less details\nF3 : Color ON/OFF\nF4 : Connection ON/OFF\nF5 : Replay Session\nF7 : Help\nF8 : Load World\nF9 : Swap Map / Car \nF10 : Map Zoom In\nF11 : Map Zoom Out\nF12 : Mirrors ON/OFF","Help",MB_ICONINFORMATION);
  SetTimer(HWindow,IDM_TIMER,DELAY_TIMER,NULL);
}


/***************************************************
 * TPROJECTApp method implementations:
 ***************************************************/

// Construct the TPROJECT's MainWindow of type TMainWindow
void TPROJECT::InitMainWindow()
{
   MainWindow = new TMainWindow(NULL, Name);
}

// Main program
int PASCAL WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
   HINSTANCE hBorLibrary;
   HINSTANCE hCarLibrary;

   hBorLibrary = LoadLibrary("bwcc.dll");
   hCarLibrary = LoadLibrary("carapi.dll");

   if((UINT)hBorLibrary <= 32)
      MessageBox(NULL, "Unable to load Borland Controls", "System Error", MB_OK | MB_ICONHAND);

   if((UINT)hCarLibrary <= 32)
      MessageBox(NULL, "Unable to load Car Library", "System Error", MB_OK | MB_ICONHAND);

   InitObjects();  // Initialize list of objects
   //ReadWorld();

   TPROJECT PROJECT ("Car Simulator", hInstance, hPrevInstance,
      lpCmdLine, nCmdShow);

   PROJECT.Run();

   if((UINT)hBorLibrary > 32)
      FreeLibrary(hBorLibrary);

   if((UINT)hCarLibrary > 32)
      FreeLibrary(hCarLibrary);

   EndObjects();  // Destroy list of objects

   //REGEN_CLEANUP

   return PROJECT.Status;
}

// ========================================================================
// VIRTUAL WORLD INITIALISATION
// ========================================================================
void InitObjects()
{
   GraphicLevel = 1;
   SolPosition.SetCoordinates(-10000,10000,1000);
   CarPos.SetCoordinates(-1000,KCARHIGH,0);
   HeadDirX = 0;
   HeadDirY = 0;
   HeadDirZ = 0;
   CarDirX = 0;
   CarDirY = 90;
   CarDirZ = 0;
   Speed = 0;
   OldSpeed= Speed;
   DrivingWheel = 0;

   NbFaces [0] = 13;
   NbPoints[0] = 20;
   NbFaces [1] = 4;
   NbPoints[1] = 9;
   NbFaces [2] = 11;
   NbPoints[2] = 16;
   NbFaces [3] = 1;
   NbPoints[3] = 4;

   // door
   P[ 0].SetCoordinates(-20,0 ,-32);
   P[ 1].SetCoordinates(-20,50,-32);
   P[ 2].SetCoordinates(-0 ,50,-32);
   P[ 3].SetCoordinates(-0 ,0 ,-32);
   // window
   P[ 4].SetCoordinates(10,15,-32);
   P[ 5].SetCoordinates(10,50,-32);
   P[ 6].SetCoordinates(20,50,-32);
   P[ 7].SetCoordinates(20,15,-32);
   // house
   P[ 8].SetCoordinates(-30,60,-30); // -  -
   P[ 9].SetCoordinates(30,60,-30); //   -
   P[10].SetCoordinates(-30,0,-30); // - - -
   P[11].SetCoordinates(30,0,-30); //  - -
   P[12].SetCoordinates(-30,60,30); // -
   P[13].SetCoordinates(30,60,30); //
   P[14].SetCoordinates(-30,0,30); // - -
   P[15].SetCoordinates(30,0,30); //  -
   // rooF
   P[16].SetCoordinates(-5,95,-5); // -  -
   P[17].SetCoordinates(5,95,5); //
   P[18].SetCoordinates(-5,95,5); // -
   P[19].SetCoordinates(5,95,-5); //   -

   F[0].InitFace(&P[ 8],&P[ 9],&P[11],&P[10]);
   F[0].SetColor(255,0,0);
   F[1].InitFace(&P[13],&P[12],&P[14],&P[15]);
   F[1].SetColor(255,0,0);
   F[2].InitFace(&P[ 9],&P[13],&P[15],&P[11]);
   F[2].SetColor(255,0,0);
   F[3].InitFace(&P[15],&P[13],&P[12],&P[14]);
   F[3].SetColor(255,0,0);
   F[4].InitFace(&P[10],&P[11],&P[15],&P[14]);
   F[4].SetColor(255,0,0);
   F[5].InitFace(&P[12],&P[ 8],&P[10],&P[14]);
   F[5].SetColor(255,0,0);

   F[6].InitFace(&P[ 8],&P[16],&P[19],&P[ 9]);
   F[6].SetColor(64,64,64);
   F[7].InitFace(&P[12],&P[18],&P[16],&P[ 8]);
   F[7].SetColor(64,64,64);
   F[8].InitFace(&P[13],&P[17],&P[18],&P[12]);
   F[8].SetColor(64,64,64);
   F[9].InitFace(&P[ 9],&P[19],&P[17],&P[13]);
   F[9].SetColor(64,64,64);
   F[10].InitFace(&P[16],&P[18],&P[17],&P[19]);
   F[10].SetColor(64,64,64);

   F[11].InitFace(&P[ 0],&P[ 1],&P[ 2],&P[ 3]);
   F[11].SetColor(128,64,0);
   F[12].InitFace(&P[ 4],&P[ 5],&P[ 6],&P[ 7]);
   F[12].SetColor(128,255,255);

   // road
   P[20].SetCoordinates(   0,0,   0); // 8  32
   P[21].SetCoordinates(-100,0,-100); // 9  33
   P[22].SetCoordinates(   0,0,-100); // 10 34
   P[23].SetCoordinates( 100,0,-100); // 11 35
   P[24].SetCoordinates( 100,0,   0); // 8  32
   P[25].SetCoordinates( 100,0, 100); // 9  33
   P[26].SetCoordinates(   0,0, 100); // 10 34
   P[27].SetCoordinates(-100,0, 100); // 11 35
   P[28].SetCoordinates(-100,0,   0); // 8  32

   F[13].InitFace(&P[22],&P[21],&P[28],&P[20]);
   F[13].SetColor(128,128,128);
   F[14].InitFace(&P[22],&P[20],&P[24],&P[23]);
   F[14].SetColor(128,128,128);
   F[15].InitFace(&P[20],&P[26],&P[25],&P[24]);
   F[15].SetColor(128,128,128);
   F[16].InitFace(&P[20],&P[28],&P[27],&P[26]);
   F[16].SetColor(128,128,128);

   // Trunc
   P[29].SetCoordinates(-5,60,-5); // 8  32
   P[30].SetCoordinates( 5,60,-5); // 9  33
   P[31].SetCoordinates(-5, 0,-5); // 10 34
   P[32].SetCoordinates( 5, 0,-5); // 11 35
   P[33].SetCoordinates(-5,60, 5); // 12 36
   P[34].SetCoordinates( 5,60, 5); // 13 37
   P[35].SetCoordinates(-5, 0, 5); // 14 38
   P[36].SetCoordinates( 5, 0, 5); // 15 39
   // leaves
   P[37].SetCoordinates(-20,60,-20); // - + -
   P[38].SetCoordinates( 20,60,-20); // + + -
   P[39].SetCoordinates(-20,60, 20); // - + +
   P[40].SetCoordinates( 20,60, 20); // + + +
   P[41].SetCoordinates(0,200,0); // - + -


   F[17].InitFace(&P[29],&P[30],&P[32],&P[31]);
   F[17].SetColor(128,64,0);
   F[18].InitFace(&P[34],&P[33],&P[35],&P[36]);
   F[18].SetColor(128,64,0);
   F[19].InitFace(&P[30],&P[34],&P[36],&P[32]);
   F[19].SetColor(128,64,0);
   F[20].InitFace(&P[36],&P[34],&P[33],&P[35]);
   F[20].SetColor(128,64,0);
   F[21].InitFace(&P[31],&P[32],&P[36],&P[35]);
   F[21].SetColor(128,64,0);
   F[22].InitFace(&P[33],&P[29],&P[31],&P[35]);
   F[22].SetColor(128,64,0);

   F[23].InitFace(&P[37],&P[41],&P[38]);
   F[23].SetColor(0,255,0);
   F[24].InitFace(&P[38],&P[41],&P[40]);
   F[24].SetColor(0,255,0);
   F[25].InitFace(&P[41],&P[39],&P[40]);
   F[25].SetColor(0,255,0);
   F[26].InitFace(&P[41],&P[37],&P[39]);
   F[26].SetColor(0,255,0);
   F[27].InitFace(&P[37],&P[38],&P[40],&P[39]);
   F[27].SetColor(0,255,0);

   // water
   P[42].SetCoordinates(-100,0,-100); // 8  32
   P[43].SetCoordinates( 100,0,-100); // 9  33
   P[44].SetCoordinates( 100,0, 100); // 10 34
   P[45].SetCoordinates(-100,0, 100); // 11 35

   F[28].InitFace(&P[45],&P[44],&P[43],&P[42]);
   F[28].SetColor(0,0,255);

// objects initialisation
   for (int h=0; h< NbObjects; h++)
   {
     for (int i=0; i< NbFaces[h]; i++)
     {
       Objects[h].AddFace(&F[NbFcs+i]);
     }
     for (i=0; i< NbPoints[h]; i++)
     {
       Objects[h].AddPoint(&P[NbPts+i]);
     }
     NbPts += NbPoints[h];
     NbFcs += NbFaces[h];
   }

   randomize();
   for (int i=-1000; i<=1000; i+=200)
   {
     WorldList.InsertHead(&Objects[1],1000,0,i); // |
     WorldList.InsertHead(&Objects[1],i,0, 1000); // |
     WorldList.InsertHead(&Objects[1],i,0,-1000); // |
     WorldList.InsertHead(&Objects[1],-1000,0,i); // |
   }
   for (i=-800; i<=800; i+=200)
   {
     WorldList.InsertHead(&Objects[1],i,0,0); // |
     WorldList.InsertHead(&Objects[1],0,0,i); // |
   }
   for (i=300; i<=700; i+=200)
   {
     WorldList.InsertHead(&Objects[3],i,0,400); // water
     WorldList.InsertHead(&Objects[3],i,0,600); // water
     WorldList.InsertHead(&Objects[2],i,0,800); // tree
   }
   WorldList.InsertHead(&Objects[0],700,0,-700); // House
   WorldList.InsertHead(&Objects[0],300,0,-700); // House
   WorldList.InsertHead(&Objects[0],700,0,-300); // House
   WorldList.InsertHead(&Objects[0],300,0,-300); // House
   WorldList.InsertHead(&Objects[2],-700,0,700); // Tree
   WorldList.InsertHead(&Objects[2],-300,0,700); // Tree
   WorldList.InsertHead(&Objects[2],-700,0,300); // Tree
   WorldList.InsertHead(&Objects[2],-300,0,300); // Tree
}

// ========================================================================
// VIRTUAL WORLD DESTRUCTION
// ========================================================================
void EndObjects()
{
   WorldList.~OList();
   for (int i=0; i<NbObjects; i++) Objects[i].~MyObject();
   for (    i=0; i< NbFcs   ; i++) F[i].~MyFace();
   for (    i=0; i< NbPts   ; i++) P[i].~MyTriplet();
}


// ========================================================================
//READ THE WORLD FILE AND DISPLAY
// ========================================================================
void ReadWorld(LPSTR aFileName)
{
 
 GraphicLevel = 1;
 SolPosition.SetCoordinates(-10000,10000,1000);
 CarPos.SetCoordinates(0,KCARHIGH,0);
 HeadDirX = 0;
 HeadDirY = 0;
 HeadDirZ = 0;
 CarDirX = 0;
 CarDirY = 0;
 CarDirZ = 0;
 Speed = 0;

 ifpstream is(aFileName);
 if (is.bad()) return;
 //read number of objects
 is>>NbObjects; // Number of objects

 int CptPts = 0;
 int CptFcs = 0;
 int Objx,Objy,Objz; // Object Coordinates

 for (int objctr=0;objctr<NbObjects;objctr++)
 {
  //add x,y,z world co-ords
  is>>Objx;
  is>>Objy;
  is>>Objz;
  // add counts for vertices and surfaces
  is>>NbPoints[objctr];
  is>>NbFaces[objctr];

  //add verteces
  for(int loopctr=0;loopctr<NbPoints[objctr];loopctr++)
  {
    int Tempx,Tempy,Tempz;

    is>>Tempx;
    is>>Tempy;
    is>>Tempz;
    P[CptPts++].SetCoordinates(Tempx,Tempy,Tempz);
    Objects[objctr].AddPoint(&P[CptPts]);
  }

  //add surfaces
  for (loopctr=0;loopctr<NbFaces[objctr];loopctr++)
  {
    // add color
    int lr,lg,lb;
    int DebPts = CptPts;
    is>>lr;
    is>>lg;
    is>>lb;
    //add vertex references
    int TempPts[4];
    int       lCtr = 0;
    is>>TempPts[0];
    while(TempPts[0]!=9999)
    {
      is>>TempPts[++lCtr];
    }
    //add surface to surface list
    if (lCtr == 3)
    {
      // 3 sides
      F[CptFcs].InitFace(&P[DebPts+TempPts[0]],&P[DebPts+TempPts[1]],&P[DebPts+TempPts[2]]);
      Objects[objctr].AddFace(&F[CptFcs]);
      CptFcs++;
    }
    else
    {
      // 4 sides
      F[CptFcs].InitFace(&P[DebPts+TempPts[0]],&P[DebPts+TempPts[1]],&P[DebPts+TempPts[2]],&P[DebPts+TempPts[3]]);
      Objects[objctr].AddFace(&F[CptFcs]);
      CptFcs++;
    }
  }
  //add new object to world list
  WorldList.InsertHead(&Objects[objctr],Objx,Objy,Objz);
 }
}
