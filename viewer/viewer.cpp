// Viewer for the 1995 car simulator engine.
//
// The original front end was PROJECT.EXE, a Borland OWL application that
// cannot be rebuilt without OWL. This driver calls exactly the same entry
// point that PROJECT.EXE's Paint handler did -- MyView() -- but renders into a
// memory framebuffer that is either shown in an X11 window or written to PNG.

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "newi_raster.h"
#include "newi_world.h"

#ifdef NEWI_HAVE_X11
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <ctime>
#endif

namespace
{

struct Options
{
  int width = 960;
  int height = 600;
  short level = 4;
  short map = 0;
  bool mirrors = false;
  long mapZoom = 9000; // matches PROJECT.CPP's MapView default
  int frames = 6;
  bool headless = false;
  std::string pngPrefix;
};

struct Camera
{
  float x = 0.0f;
  float y = 130.0f; // eye height above the road surface
  float z = 300.0f;
  float heading = 0.0f; // degrees about Y
  float pitch = 0.0f;
  float speed = 0.0f;
  float wheel = 0.0f;
};

void Advance(Camera &cam, DemoWorld &world, float dt)
{
  cam.heading += cam.wheel * dt * 45.0f;
  if (cam.heading >= 360.0f) cam.heading -= 360.0f;
  if (cam.heading < 0.0f) cam.heading += 360.0f;

  const double rad = cam.heading * M_PI / 180.0;
  cam.x += (float)(std::sin(rad) * cam.speed * dt);
  cam.z += (float)(std::cos(rad) * cam.speed * dt);

  // Loop the road so a long session never drives off the end.
  const float len = world.RoadLength();
  if (cam.z > len) cam.z -= len;
  if (cam.z < 0) cam.z += len;
}

void RenderFrame(HDC dc, DemoWorld &world, const Camera &cam,
                 const Options &opt)
{
  NewiRasterClear(dc, RGB(0, 0, 0));

  MyTriplet sun;
  sun.SetCoordinates(-10000, 10000, 1000);

  POINT windowPos;
  windowPos.x = 0;
  windowPos.y = 0;
  POINT windowSize;
  windowSize.x = opt.width;
  windowSize.y = opt.height;

  if (opt.map)
  {
    // As PROJECT.CPP drew it: lift the camera straight up by MapView, look
    // down (pitch 90) and spin the world so the car's heading points up.
    MyTriplet eye;
    eye.SetCoordinates(cam.x, cam.y + (float)opt.mapZoom, cam.z);
    MyView(dc, eye, &world.Scene(), 90.0f, 0.0f, -cam.heading, windowPos,
           windowSize, opt.level, 0, 1, sun);
    return;
  }

  MyTriplet eye;
  eye.SetCoordinates(cam.x, cam.y, cam.z);

  MyView(dc, eye, &world.Scene(), cam.pitch, cam.heading, 0.0f, windowPos,
         windowSize, opt.level, 0, 0, sun);

  if (opt.mirrors)
  {
    // aInverted = 1 mirrors the image horizontally, as the original did for
    // both mirrors. The angles are PROJECT.CPP's mirror offsets.
    POINT mirrorPos, mirrorSize;

    mirrorPos.x = (opt.width - BackMirrorW) / 2;
    mirrorPos.y = 0;
    mirrorSize.x = BackMirrorW;
    mirrorSize.y = BackMirrorH;
    MyView(dc, eye, &world.Scene(), cam.pitch, cam.heading + 181.0f, 0.0f,
           mirrorPos, mirrorSize, opt.level, 1, 0, sun);

    mirrorPos.x = opt.width - RightMirrorW - 8;
    mirrorPos.y = 0;
    mirrorSize.x = RightMirrorW;
    mirrorSize.y = RightMirrorH;
    MyView(dc, eye, &world.Scene(), cam.pitch, cam.heading + 170.0f, 0.0f,
           mirrorPos, mirrorSize, opt.level, 1, 0, sun);

    // MyView leaves the clip set to the last viewport.
    SelectClipRgn(dc, 0);
  }

  // Needle sweeps with speed, exactly as the original dashboard did.
  SpeedoMeter(dc, 70, opt.height - 60, 45, (long)(cam.speed / 8.0f));
}

bool WritePng(const Options &opt, const NewiPixel *pixels, int frame)
{
  char path[512];
  std::snprintf(path, sizeof(path), "%s_%03d.png", opt.pngPrefix.c_str(),
                frame);
  if (!NewiRasterWritePng(path, pixels, opt.width, opt.height))
  {
    std::fprintf(stderr, "failed to write %s\n", path);
    return false;
  }
  std::printf("wrote %s\n", path);
  return true;
}

int RunHeadless(const Options &opt)
{
  DemoWorld world;
  std::vector<NewiPixel> fb((size_t)opt.width * opt.height, 0);
  HDC dc = NewiRasterCreateDC(fb.data(), opt.width, opt.height);

  Camera cam;
  cam.speed = 900.0f;

  bool ok = true;
  for (int f = 0; f < opt.frames; ++f)
  {
    RenderFrame(dc, world, cam, opt);
    if (!opt.pngPrefix.empty())
      ok = WritePng(opt, fb.data(), f) && ok;
    Advance(cam, world, 0.9f);
  }

  NewiRasterDestroyDC(dc);
  return ok ? 0 : 1;
}

#ifdef NEWI_HAVE_X11

double NowSeconds()
{
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

int RunInteractive(Options opt)
{
  Display *dpy = XOpenDisplay(0);
  if (!dpy)
  {
    std::fprintf(stderr,
                 "cannot open display '%s'; falling back to PNG output\n",
                 std::getenv("DISPLAY") ? std::getenv("DISPLAY") : "(unset)");
    if (opt.pngPrefix.empty()) opt.pngPrefix = "frame";
    return RunHeadless(opt);
  }

  const int screen = DefaultScreen(dpy);
  Visual *visual = DefaultVisual(dpy, screen);
  const int depth = DefaultDepth(dpy, screen);

  Window win = XCreateSimpleWindow(
      dpy, RootWindow(dpy, screen), 0, 0, (unsigned)opt.width,
      (unsigned)opt.height, 0, BlackPixel(dpy, screen),
      BlackPixel(dpy, screen));

  XStoreName(dpy, win, "Virtual Reality Car Simulator (1995 engine)");
  XSelectInput(dpy, win,
               ExposureMask | KeyPressMask | KeyReleaseMask |
                   StructureNotifyMask);
  XMapWindow(dpy, win);

  GC gc = XCreateGC(dpy, win, 0, 0);

  std::vector<NewiPixel> fb((size_t)opt.width * opt.height, 0);
  XImage *image = XCreateImage(dpy, visual, (unsigned)depth, ZPixmap, 0,
                               (char *)fb.data(), (unsigned)opt.width,
                               (unsigned)opt.height, 32, 0);
  if (!image)
  {
    std::fprintf(stderr, "XCreateImage failed\n");
    XCloseDisplay(dpy);
    return 1;
  }

  DemoWorld world;
  HDC dc = NewiRasterCreateDC(fb.data(), opt.width, opt.height);
  Camera cam;

  std::printf("controls: arrows drive/steer, 1-5 detail, M map, +/- map zoom, "
              "V mirrors, P screenshot, Q quit\n");

  bool running = true;
  bool accel = false, brake = false, left = false, right = false;
  int shot = 0;
  double last = NowSeconds();

  while (running)
  {
    while (XPending(dpy))
    {
      XEvent ev;
      XNextEvent(dpy, &ev);
      if (ev.type == KeyPress || ev.type == KeyRelease)
      {
        const KeySym ks = XLookupKeysym(&ev.xkey, 0);
        const bool down = (ev.type == KeyPress);
        switch (ks)
        {
        case XK_Up: accel = down; break;
        case XK_Down: brake = down; break;
        case XK_Left: left = down; break;
        case XK_Right: right = down; break;
        default:
          if (!down) break;
          if (ks == XK_q || ks == XK_Escape) running = false;
          else if (ks == XK_m) opt.map = opt.map ? 0 : 1;
          else if (ks == XK_v) opt.mirrors = !opt.mirrors;
          else if (ks >= XK_1 && ks <= XK_5) opt.level = (short)(ks - XK_1 + 1);
          else if (ks == XK_equal || ks == XK_plus)
            opt.mapZoom -= (opt.mapZoom > 1000) ? 500 : 0;
          else if (ks == XK_minus)
            opt.mapZoom += (opt.mapZoom < 9000) ? 500 : 0;
          else if (ks == XK_p)
          {
            Options shotOpt = opt;
            shotOpt.pngPrefix = "screenshot";
            WritePng(shotOpt, fb.data(), shot++);
          }
          break;
        }
      }
      else if (ev.type == ConfigureNotify)
      {
        // Fixed-size framebuffer: ignore resizes rather than draw garbage.
      }
    }

    const double now = NowSeconds();
    float dt = (float)(now - last);
    last = now;
    if (dt > 0.1f) dt = 0.1f;

    if (accel) cam.speed += 900.0f * dt;
    if (brake) cam.speed -= 1400.0f * dt;
    cam.speed -= cam.speed * 0.35f * dt; // drag
    if (cam.speed < 0.0f) cam.speed = 0.0f;
    if (cam.speed > 4000.0f) cam.speed = 4000.0f;

    cam.wheel = 0.0f;
    if (left) cam.wheel -= 1.0f;
    if (right) cam.wheel += 1.0f;

    Advance(cam, world, dt);
    RenderFrame(dc, world, cam, opt);

    XPutImage(dpy, win, gc, image, 0, 0, 0, 0, (unsigned)opt.width,
              (unsigned)opt.height);
    XFlush(dpy);

    struct timespec nap = {0, 8 * 1000 * 1000};
    nanosleep(&nap, 0);
  }

  NewiRasterDestroyDC(dc);
  image->data = 0; // the framebuffer is owned by the vector
  XDestroyImage(image);
  XFreeGC(dpy, gc);
  XDestroyWindow(dpy, win);
  XCloseDisplay(dpy);
  return 0;
}

#endif // NEWI_HAVE_X11

void Usage(const char *argv0)
{
  std::printf(
      "usage: %s [options]\n"
      "  --png PREFIX   render frames to PREFIX_000.png and exit\n"
      "  --frames N     number of frames in PNG mode (default 6)\n"
      "  --width W      framebuffer width (default 960)\n"
      "  --height H     framebuffer height (default 600)\n"
      "  --level 1..5   detail: 1 wireframe, 3 flat colour, 4 lit, 5 shadows\n"
      "  --map          overhead map view\n"
      "  --zoom N       map altitude, 1000..9000 (default 9000)\n"
      "  --mirrors      overlay the back and right mirrors\n"
      "  --headless     never open a window\n",
      argv0);
}

} // namespace

int main(int argc, char **argv)
{
  Options opt;

  for (int i = 1; i < argc; ++i)
  {
    const char *a = argv[i];
    const bool hasNext = (i + 1 < argc);
    if (!std::strcmp(a, "--png") && hasNext)
    {
      opt.pngPrefix = argv[++i];
      opt.headless = true;
    }
    else if (!std::strcmp(a, "--frames") && hasNext) opt.frames = std::atoi(argv[++i]);
    else if (!std::strcmp(a, "--width") && hasNext) opt.width = std::atoi(argv[++i]);
    else if (!std::strcmp(a, "--height") && hasNext) opt.height = std::atoi(argv[++i]);
    else if (!std::strcmp(a, "--level") && hasNext) opt.level = (short)std::atoi(argv[++i]);
    else if (!std::strcmp(a, "--map")) opt.map = 1;
    else if (!std::strcmp(a, "--mirrors")) opt.mirrors = true;
    else if (!std::strcmp(a, "--zoom") && hasNext) opt.mapZoom = std::atol(argv[++i]);
    else if (!std::strcmp(a, "--headless")) opt.headless = true;
    else
    {
      Usage(argv[0]);
      return std::strcmp(a, "--help") == 0 ? 0 : 2;
    }
  }

  if (opt.width < 64) opt.width = 64;
  if (opt.height < 64) opt.height = 64;
  if (opt.level < 1) opt.level = 1;
  if (opt.level > 5) opt.level = 5;
  if (opt.mapZoom < 1000) opt.mapZoom = 1000;
  if (opt.mapZoom > 9000) opt.mapZoom = 9000;

#ifdef NEWI_HAVE_X11
  if (!opt.headless) return RunInteractive(opt);
#endif

  if (opt.pngPrefix.empty()) opt.pngPrefix = "frame";
  return RunHeadless(opt);
}
