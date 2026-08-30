//----------------------------------------------------------------------------
// ObjectWindows - (C) Copyright 1991, 1993 by Borland International
//----------------------------------------------------------------------------
#include <owl\owlpch.h>
#include <owl\applicat.h>
#include <owl\framewin.h>
#include <owl\dc.h>
#include <cstring.h>
#include "carapi.h"
#include "calldll.h"

class TTestWindow : public TWindow {
  public:
	 TTestWindow() : TWindow(0, 0, 0) {}
	 ~TTestWindow();

	 void SetupWindow();
	 void CleanupWindow();
    void Paint(TDC&, BOOL, TRect&);

	 void EvTimer(UINT);

	 void CmThrottle();
	 void CmClutch();
	 void CmBrake();
	 void CmHandbrake();
	 void CmGear();
	 void CmWheelPosition();
	 void CmIgnition();
	 void CmHeadPosition();

  private:

	 DECLARE_RESPONSE_TABLE(TTestWindow);
};

DEFINE_RESPONSE_TABLE1(TTestWindow, TWindow)
  EV_WM_TIMER,
  EV_COMMAND(CM_THROTTLE, CmThrottle),
  EV_COMMAND(CM_CLUTCH, CmClutch),
  EV_COMMAND(CM_BRAKE, CmBrake),
  EV_COMMAND(CM_HANDBRAKE, CmHandbrake),
  EV_COMMAND(CM_GEAR, CmGear),
  EV_COMMAND(CM_WHEELPOSITION, CmWheelPosition),
  EV_COMMAND(CM_IGNITION, CmIgnition),
  EV_COMMAND(CM_HEADPOSITION, CmHeadPosition),
END_RESPONSE_TABLE;

void
TTestWindow::SetupWindow()
{
  TWindow::SetupWindow();
  if (!SetTimer(1, 100U, 0)) {
	 MessageBox("Out of Timers", GetApplication()->GetName(),
					MB_ICONEXCLAMATION|MB_OK);
  }
}

TTestWindow::~TTestWindow()
{
  KillTimer(1);      // Get rid of timer
}

void
TTestWindow::CleanupWindow()
{
  TWindow::CleanupWindow();
}

void
TTestWindow::CmThrottle()
{
  char str[25];
  int val;
  val = Throttle();
  wvsprintf(str, "The throttle is %d", &val);
  MessageBox(str, "Throttle", MB_OK);
}

void
TTestWindow::CmClutch()
{
  char str[20];
  int val;
  val = Clutch();
  wvsprintf(str, "The clutch is %d", &val);
  MessageBox(str, "Clutch", MB_OK);
}

void
TTestWindow::CmBrake()
{
  char str[20];
  int val;
  val = Brake();
  wvsprintf(str, "The brake is %d", &val);
  MessageBox(str, "Brake", MB_OK);
}

void
TTestWindow::CmGear()
{
  char str[20];
  int val;
  val = Gear();
  wvsprintf(str, "The gear is %d", &val);
  MessageBox(str, "Gear", MB_OK);
}

void
TTestWindow::CmHandbrake()
{
  char str[20];
  int val;
  val = Handbrake();
  wvsprintf(str, "The handbrake is %d", &val);
  MessageBox(str, "Handbrake", MB_OK);
}

void
TTestWindow::CmWheelPosition()
{
  char str[30];
  int val;
  val = WheelPosition();
  wvsprintf(str, "The wheel position is %d", &val);
  MessageBox(str, "Wheel Position", MB_OK);
}

void
TTestWindow::CmIgnition()
{
  char str[20];
  int val;
  val = Ignition();
  wvsprintf(str, "The ignition is %d", &val);
  MessageBox(str, "Ignition", MB_OK);
}

void
TTestWindow::CmHeadPosition()
{
  char str[30];
  int val;
  val = HeadPosition();
  wvsprintf(str, "The head position is %d", &val);
  MessageBox(str, "Head Position", MB_OK);
}

void
TTestWindow::Paint(TDC& dc, BOOL, TRect&)
{
  int val;
  char str[30];
  val = Throttle();
  wvsprintf(str, "The throttle is %d", &val);
  dc.TextOut(20, 40, str, strlen(str));
  val = Clutch();
  wvsprintf(str, "The clutch is %d", &val);
  dc.TextOut(20, 60, str, strlen(str));
  val = Brake();
  wvsprintf(str, "The brake is %d", &val);
  dc.TextOut(20, 80, str, strlen(str));
  val = Handbrake();
  wvsprintf(str, "The handbrake is %d", &val);
  dc.TextOut(20, 100, str, strlen(str));
  val = Gear();
  wvsprintf(str, "The gear is %d", &val);
  dc.TextOut(20, 120, str, strlen(str));
  val = WheelPosition();
  wvsprintf(str, "The wheel position is %d", &val);
  dc.TextOut(20, 140, str, strlen(str));
  val = Ignition();
  wvsprintf(str, "The ignition is %d", &val);
  dc.TextOut(20, 160, str, strlen(str));
  val = HeadPosition();
  wvsprintf(str, "The head position is %d", &val);
  dc.TextOut(20, 180, str, strlen(str));
}

void
TTestWindow::EvTimer(UINT)
{
Invalidate(FALSE);
}



//----------------------------------------------------------------------------

class TCallDllApp : public TApplication {
  public:
	 void InitMainWindow();
};

void
TCallDllApp::InitMainWindow()
{
  MainWindow = new TFrameWindow(0, "CallDll", new TTestWindow);
  MainWindow->AssignMenu("COMMANDS");
}

int
OwlMain(int /*argc*/, char* /*argv*/ [])
{
  int ret = TCallDllApp().Run();
  return ret;
}

