// Drives the 1995 render pipeline headlessly: builds a small world, calls
// MyView at every graphic level, and checks that geometry reaches the GDI.

#include <cstdio>

#include "VIEW.H"
#include "newi_gdi_capture.h"

namespace
{

struct Cube
{
  MyTriplet points[8];
  MyFace faces[6];
  MyObject object;

  Cube(float half, float baseY)
  {
    const float lo = baseY;
    const float hi = baseY + 2 * half;

    points[0].SetCoordinates(-half, lo, -half);
    points[1].SetCoordinates(half, lo, -half);
    points[2].SetCoordinates(half, lo, half);
    points[3].SetCoordinates(-half, lo, half);
    points[4].SetCoordinates(-half, hi, -half);
    points[5].SetCoordinates(half, hi, -half);
    points[6].SetCoordinates(half, hi, half);
    points[7].SetCoordinates(-half, hi, half);

    static const int quads[6][4] = {
        {0, 1, 2, 3}, // bottom
        {4, 5, 6, 7}, // top
        {0, 1, 5, 4}, // front
        {1, 2, 6, 5}, // right
        {2, 3, 7, 6}, // back
        {3, 0, 4, 7}, // left
    };

    for (int f = 0; f < 6; ++f)
    {
      faces[f].InitFace(&points[quads[f][0]], &points[quads[f][1]],
                        &points[quads[f][2]], &points[quads[f][3]]);
      faces[f].SetColor(200, 60 + 20 * f, 40);
      object.AddFace(&faces[f]);
    }
    for (int p = 0; p < 8; ++p)
      object.AddPoint(&points[p]);
  }
};

// A flat strip on the ground plane, close enough to the camera to take the
// road-clipping branch of MyFace::Draw, and wide enough that ClipPoly has to
// cut it against the side edges.
struct Road
{
  MyTriplet points[4];
  MyFace face;
  MyObject object;

  Road()
  {
    // Wound so the surface normal points up, otherwise the hidden-face test
    // culls the road as seen from above.
    points[0].SetCoordinates(-2000, 0, 400);
    points[1].SetCoordinates(2000, 0, 400);
    points[2].SetCoordinates(2000, 0, 0);
    points[3].SetCoordinates(-2000, 0, 0);

    face.InitFace(&points[0], &points[1], &points[2], &points[3]);
    face.SetColor(90, 90, 90);
    object.AddFace(&face);
    for (int p = 0; p < 4; ++p)
      object.AddPoint(&points[p]);
  }
};

bool renderAtLevel(short graphicLevel, short map)
{
  Cube cube(150.0f, 60.0f);
  Road road;

  OList world;
  world.InsertHead(&cube.object, 0, 0, 2500);
  world.InsertHead(&road.object, 0, 0, 0);

  MyTriplet carPos;
  carPos.SetCoordinates(0, KCARHIGH, 0);

  MyTriplet sunPos;
  sunPos.SetCoordinates(-10000, 10000, 1000);

  POINT windowPos;
  windowPos.x = 0;
  windowPos.y = 0;
  POINT windowSize;
  windowSize.x = 640;
  windowSize.y = 480;

  HDC dc = NewiGdiCreateStubDC();

  NewiGdiResetStats();
  MyView(dc, carPos, &world, 0, 0, 0, windowPos, windowSize, graphicLevel, 0,
         map, sunPos);
  const NewiGdiStats &s = NewiGdiGetStats();

  const long drawCalls = s.polygons + s.lines;
  std::printf("  level %d map %d : polygons=%ld verts=%ld lines=%ld "
              "moves=%ld fills=%ld pens=%ld brushes=%ld\n",
              graphicLevel, map, s.polygons, s.polygonVertices, s.lines,
              s.moves, s.fillRects, s.pensCreated, s.brushesCreated);

  bool ok = true;

  if (drawCalls == 0)
  {
    std::printf("    FAIL: nothing was drawn\n");
    ok = false;
  }
  if (s.hasBounds)
  {
    std::printf("    bounds x[%ld..%ld] y[%ld..%ld]\n", (long)s.minX,
                (long)s.maxX, (long)s.minY, (long)s.maxY);
  }

  // Every pen, brush and region the frame creates must be released, or a
  // long session exhausts the GDI heap.
  const long created = s.pensCreated + s.brushesCreated + s.regionsCreated;
  if (s.objectsDeleted != created)
  {
    std::printf("    FAIL: leaked GDI objects (created=%ld deleted=%ld)\n",
                created, s.objectsDeleted);
    ok = false;
  }
  return ok;
}

bool checkGeometry()
{
  MyTriplet t;
  t.SetCoordinates(100, 200, 1000);
  t.Projection(0, 0, 0);
  // KDistance == 1000, so a point at z == 1000 projects 1:1.
  if (t.P.x != 100 || t.P.y != 200)
  {
    std::printf("  FAIL: projection gave (%ld,%ld), expected (100,200)\n",
                (long)t.P.x, (long)t.P.y);
    return false;
  }

  // Right-handed rotation about Y: +X maps onto -Z.
  t.SetCoordinates(1000, 0, 0);
  t.Rotation(0, 0, 0, 2, 90);
  if (t.Pz < -1010 || t.Pz > -990 || t.Px < -10 || t.Px > 10)
  {
    std::printf("  FAIL: Y rotation gave (%f,%f), expected ~(0,-1000)\n", t.Px,
                t.Pz);
    return false;
  }

  MyTriplet inView;
  inView.SetCoordinates(0, 0, 5000);
  MyTriplet behind;
  behind.SetCoordinates(0, 0, 50000);
  if (Triangle(inView) != 0 || Triangle(behind) != 1)
  {
    std::printf("  FAIL: view-frustum test misclassified a point\n");
    return false;
  }
  return true;
}

// ClipPoly must cut against all four window edges and never emit a vertex
// outside them.
bool checkClipping()
{
  Poly wide;
  wide.NumVert = 4;
  wide.Verts[0].x = -3000; wide.Verts[0].y = 200;
  wide.Verts[1].x = 3000;  wide.Verts[1].y = 200;
  wide.Verts[2].x = 3000;  wide.Verts[2].y = 800;
  wide.Verts[3].x = -3000; wide.Verts[3].y = 800;

  Poly clipped = ClipPoly(wide);
  std::printf("  clipped a 6000-wide quad down to %d vertices\n",
              clipped.NumVert);
  if (clipped.NumVert <= 0)
  {
    std::printf("  FAIL: clipping discarded a polygon crossing the window\n");
    return false;
  }
  for (int i = 0; i < clipped.NumVert; ++i)
  {
    if (clipped.Verts[i].x < -1000 || clipped.Verts[i].x > 1000)
    {
      std::printf("  FAIL: vertex %d escaped the window at x=%ld\n", i,
                  (long)clipped.Verts[i].x);
      return false;
    }
  }

  // Entirely outside: must come back empty rather than reading Verts[-1].
  Poly outside;
  outside.NumVert = 3;
  outside.Verts[0].x = 5000; outside.Verts[0].y = 5000;
  outside.Verts[1].x = 6000; outside.Verts[1].y = 5000;
  outside.Verts[2].x = 6000; outside.Verts[2].y = 6000;
  Poly gone = ClipPoly(outside);
  if (gone.NumVert != 0)
  {
    std::printf("  FAIL: fully-outside polygon survived with %d vertices\n",
                gone.NumVert);
    return false;
  }
  return true;
}

// Exercises the list/object deletion paths, which used to corrupt the heap.
bool checkDeletion()
{
  Cube a(100.0f, 10.0f), b(100.0f, 10.0f), c(100.0f, 10.0f);

  OList list;
  list.InsertHead(&a.object, 0, 0, 100);
  list.InsertHead(&b.object, 0, 0, 200);
  list.InsertTail(&c.object, 0, 0, 300);

  if (list.NbItems != 3)
  {
    std::printf("  FAIL: expected 3 items, got %ld\n", list.NbItems);
    return false;
  }

  if (!list.DeleteTail() || !list.DeleteHead())
  {
    std::printf("  FAIL: deletion reported an empty list\n");
    return false;
  }

  long walked = 0;
  for (OList::Cell *cur = list.Head; cur != NULL; cur = cur->Next)
    ++walked;

  std::printf("  list: 3 inserted, tail+head removed, %ld reachable "
              "(NbItems=%ld)\n", walked, list.NbItems);
  if (walked != 1 || list.NbItems != 1)
  {
    std::printf("  FAIL: list length and NbItems disagree\n");
    return false;
  }

  if (!list.DeleteHead() || list.Head != NULL)
  {
    std::printf("  FAIL: list did not drain cleanly\n");
    return false;
  }

  // MyObject owns malloc'd cells for its faces and points too.
  Cube d(100.0f, 10.0f);
  while (d.object.DeleteFace()) {}
  while (d.object.DeletePoint()) {}
  if (d.object.NbFaces != 0 || d.object.NbPoints != 0)
  {
    std::printf("  FAIL: object face/point lists did not drain\n");
    return false;
  }
  return true;
}

} // namespace

int main()
{
  bool ok = true;

  std::printf("geometry primitives:\n");
  ok = checkGeometry() && ok;

  std::printf("polygon clipping:\n");
  ok = checkClipping() && ok;

  std::printf("list and object deletion:\n");
  ok = checkDeletion() && ok;

  std::printf("render passes:\n");
  for (short level = 1; level <= 5; ++level)
    ok = renderAtLevel(level, 0) && ok;
  ok = renderAtLevel(4, 1) && ok;

  std::printf("%s\n", ok ? "engine smoke test PASSED" : "engine smoke test FAILED");
  return ok ? 0 : 1;
}
