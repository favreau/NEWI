#ifndef NEWI_WORLD_H
#define NEWI_WORLD_H

// A procedurally built stand-in for the .CGW scene files the original
// PROJECT.EXE loaded, which are not in this repository.

#include <deque>

#include "VIEW.H"

class DemoWorld
{
public:
  DemoWorld();

  OList &Scene() { return scene; }

  // Extent of the road along +Z, so the camera can be wrapped around.
  float RoadLength() const { return roadLength; }
  float RoadHalfWidth() const { return roadHalfWidth; }

private:
  // deque, not vector: MyObject stores raw pointers to these, so growing the
  // container must never move existing elements.
  std::deque<MyTriplet> points;
  std::deque<MyFace> faces;
  std::deque<MyObject> objects;
  OList scene;

  float roadLength;
  float roadHalfWidth;

  MyObject &NewObject();
  MyTriplet &NewPoint(float x, float y, float z);

  // Adds a quad with the winding the engine's hidden-face test expects.
  void AddQuad(MyObject &object, MyTriplet &a, MyTriplet &b, MyTriplet &c,
               MyTriplet &d, int red, int green, int blue);

  void AddRoadSegment(float z, float length, int shade);
  void AddBox(float centerX, float centerZ, float halfWidth, float halfDepth,
              float height, int red, int green, int blue);
};

#endif // NEWI_WORLD_H
