/* ===================================================
   Polygon clipping unit
   ===================================================*/

#include "bwcc.h"


#define XVMin -1000
#define XVMax 1000
#define YVMin 10
#define YVMax 1000

#define CLIPLEFT   1
#define CLIPRIGHT  2
#define CLIPBOTTOM 3
#define CLIPTOP    4

typedef struct Poly
{
  int   NumVert;
  POINT Verts[20];
};

void AddVertex(POINT, Poly *);
void AddVertex(POINT, Poly *);
short Inside  (POINT, int);
void FindIntersect(POINT ,POINT ,int ,POINT *);
void ClipOne(Poly , Poly *, int );
Poly ClipPoly(Poly);