#include "newi_world.h"

// The engine computes a face normal as (P1-P0) x (P2-P1) and draws the face
// only when that normal points back towards the camera. Every winding below is
// chosen so the normal faces outwards.

DemoWorld::DemoWorld()
  : roadLength(24000.0f), roadHalfWidth(420.0f)
{
  const float segment = 1200.0f;
  int index = 0;
  for (float z = 0; z < roadLength; z += segment, ++index)
  {
    AddRoadSegment(z, segment, (index % 2) ? 70 : 82);
  }

  // Buildings down both verges, alternating size and colour.
  const int palette[6][3] = {
      {170, 90, 70}, {110, 130, 165}, {150, 150, 120},
      {95, 140, 105}, {180, 150, 90}, {130, 110, 150},
  };

  index = 0;
  for (float z = 1500.0f; z < roadLength - 1500.0f; z += 2600.0f, ++index)
  {
    const int *left = palette[index % 6];
    const int *right = palette[(index + 3) % 6];

    const float leftHeight = 500.0f + (float)((index * 317) % 900);
    const float rightHeight = 450.0f + (float)((index * 523) % 1100);

    AddBox(-1250.0f, z, 380.0f, 520.0f, leftHeight, left[0], left[1], left[2]);
    AddBox(1250.0f, z + 900.0f, 420.0f, 480.0f, rightHeight, right[0],
           right[1], right[2]);
  }

  // A few low blocks close to the verge for a sense of speed.
  for (float z = 900.0f; z < roadLength - 900.0f; z += 1300.0f)
  {
    AddBox(-700.0f, z, 60.0f, 60.0f, 150.0f, 200, 200, 205);
    AddBox(700.0f, z + 650.0f, 60.0f, 60.0f, 150.0f, 200, 200, 205);
  }
}

MyObject &DemoWorld::NewObject()
{
  objects.push_back(MyObject());
  return objects.back();
}

MyTriplet &DemoWorld::NewPoint(float x, float y, float z)
{
  points.push_back(MyTriplet());
  MyTriplet &p = points.back();
  p.SetCoordinates(x, y, z);
  return p;
}

void DemoWorld::AddQuad(MyObject &object, MyTriplet &a, MyTriplet &b,
                        MyTriplet &c, MyTriplet &d, int red, int green,
                        int blue)
{
  faces.push_back(MyFace());
  MyFace &face = faces.back();
  face.InitFace(&a, &b, &c, &d);
  face.SetColor(red, green, blue);
  object.AddFace(&face);
}

void DemoWorld::AddRoadSegment(float z, float length, int shade)
{
  MyObject &object = NewObject();

  const float hw = roadHalfWidth;
  // Local coordinates run from z=0 to z=length; the object is placed at `z`.
  MyTriplet &a = NewPoint(-hw, 0, length);
  MyTriplet &b = NewPoint(hw, 0, length);
  MyTriplet &c = NewPoint(hw, 0, 0);
  MyTriplet &d = NewPoint(-hw, 0, 0);

  // Winding gives a +Y normal, so the surface is visible from above.
  AddQuad(object, a, b, c, d, shade, shade, shade + 4);

  object.AddPoint(&a);
  object.AddPoint(&b);
  object.AddPoint(&c);
  object.AddPoint(&d);

  scene.InsertHead(&object, 0, 0, (long)z);
}

void DemoWorld::AddBox(float centerX, float centerZ, float halfWidth,
                       float halfDepth, float height, int red, int green,
                       int blue)
{
  MyObject &object = NewObject();

  const float hw = halfWidth;
  const float hd = halfDepth;
  const float h = height;

  // Eight corners: 0-3 at the base, 4-7 at the top.
  MyTriplet &b0 = NewPoint(-hw, 0, -hd);
  MyTriplet &b1 = NewPoint(hw, 0, -hd);
  MyTriplet &b2 = NewPoint(hw, 0, hd);
  MyTriplet &b3 = NewPoint(-hw, 0, hd);
  MyTriplet &t0 = NewPoint(-hw, h, -hd);
  MyTriplet &t1 = NewPoint(hw, h, -hd);
  MyTriplet &t2 = NewPoint(hw, h, hd);
  MyTriplet &t3 = NewPoint(-hw, h, hd);

  const int roofR = red > 40 ? red - 40 : 0;
  const int roofG = green > 40 ? green - 40 : 0;
  const int roofB = blue > 40 ? blue - 40 : 0;

  AddQuad(object, t3, t2, t1, t0, roofR, roofG, roofB);          // +Y roof
  AddQuad(object, b0, t0, t1, b1, red, green, blue);             // -Z
  AddQuad(object, b2, t2, t3, b3, red, green, blue);             // +Z
  AddQuad(object, b1, t1, t2, b2, red - 20, green - 20, blue);   // +X
  AddQuad(object, b3, t3, t0, b0, red - 20, green - 20, blue);   // -X

  object.AddPoint(&b0);
  object.AddPoint(&b1);
  object.AddPoint(&b2);
  object.AddPoint(&b3);
  object.AddPoint(&t0);
  object.AddPoint(&t1);
  object.AddPoint(&t2);
  object.AddPoint(&t3);

  scene.InsertHead(&object, (long)centerX, 0, (long)centerZ);
}
