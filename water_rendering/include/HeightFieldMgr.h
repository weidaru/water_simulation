#ifndef HEIGHT_FIELD_MANAGER_H_
#define HEIGHT_FIELD_MANAGER_H_

#include "Gz.h"

class HeightFieldMgr
{
private:
	typedef GzCoord Triangle[3];
	typedef Triangle Rect[2];	/* 1st element for down tri, 2nd for up tri*/

public:
	HeightFieldMgr();
	virtual ~HeightFieldMgr(void);

	int generateHeightField (char* inFileName, char* outFileName);
	int generateNormals (char* inFileName, char* outFileName);

private:
	Rect grid[RECT_MAX_X][RECT_MAX_Z];
	Rect normalGrid[RECT_MAX_X][RECT_MAX_Z];
	

	float turbulence(float u, float v);
	float noise (int u, int v);

	float smoothNoise (float u, float v);
	float interpNoise(float u, float v);

	int getHeightField(GzCoord vertex, GzCoord normal, float height, GzCoord newVertex);

	int getFaceNormal(Triangle tri, GzCoord normal);
	int crossProduct(GzCoord a, GzCoord b, GzCoord c);
	int normalize(GzCoord vector);


	int calculateVertexNormal(Triangle triList[8], int count, int x, int z, int udTri, int nthVertex);
	int gatherAdjTriangles(Triangle adjTris[8], int type, int x, int z);


};

#endif 			//HEIGHT_FIELD_MANAGER_H_