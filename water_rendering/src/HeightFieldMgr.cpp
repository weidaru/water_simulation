#define TOP_TRI_V0	0
#define TOP_TRI_V1	1
#define TOP_TRI_V2	2
#define BOTTOM_TRI_V0	3
#define BOTTOM_TRI_V1	4
#define BOTTOM_TRI_V2	5

#include "ImageManager.h"
#include "HeightFieldMgr.h"
#include "utilities.h"
#include "math.h"
#include "stdio.h"
#include "stdlib.h"

#include <string>

HeightFieldMgr::HeightFieldMgr(void)
{

}

HeightFieldMgr::~HeightFieldMgr(void)
{
}


/************************************************************************/
/* Generate normals                                                                     */
/************************************************************************/

int HeightFieldMgr::generateNormals (char* inFileName, char* outFileName) {
	FILE *infile;
	FILE *outfile;
	GzToken		nameListTriangle[3]; 	/* vertex attribute names */
	GzPointer	valueListTriangle[3]; 	/* vertex attribute pointers */
	GzCoord		vertexList[3];	/* vertex position coordinates */ 
	GzCoord		normalList[3];	/* vertex normals */ 
	GzCoord		newVertexList[3];
	GzCoord		newNormalList[3];
	GzTextureIndex  	uvList[3];		/* vertex texture map indices */ 
	char		dummy[256]; 

	if( (infile  = fopen( inFileName , "r" )) == NULL )
	{
		printf("error opening the input file");
		return -1;
	}
	
	if( (outfile  = fopen( outFileName , "w" )) == NULL )
	{
		printf("error opening the output file");
		return -1;
	}

	//240 tris per unit x
	//120 rects per unit x

	int triCount = 0;
	while( fscanf(infile, "%s", dummy) == 1) { 	/* read in tri word */
		fscanf(infile, "%f %f %f %f %f %f %f %f", 
			&(vertexList[0][0]), &(vertexList[0][1]),  
			&(vertexList[0][2]), 
			&(normalList[0][0]), &(normalList[0][1]), 	
			&(normalList[0][2]), 
			&(uvList[0][0]), &(uvList[0][1]) ); 
		fscanf(infile, "%f %f %f %f %f %f %f %f", 
			&(vertexList[1][0]), &(vertexList[1][1]), 	
			&(vertexList[1][2]), 
			&(normalList[1][0]), &(normalList[1][1]), 	
			&(normalList[1][2]), 
			&(uvList[1][0]), &(uvList[1][1]) ); 
		fscanf(infile, "%f %f %f %f %f %f %f %f", 
			&(vertexList[2][0]), &(vertexList[2][1]), 	
			&(vertexList[2][2]), 
			&(normalList[2][0]), &(normalList[2][1]), 	
			&(normalList[2][2]), 
			&(uvList[2][0]), &(uvList[2][1]) ); 


		//store all vertex into grid array
		memcpy(grid[triCount / (RECT_MAX_Z * 2)]/*along x axis*/[(triCount / 2) % RECT_MAX_Z]/*along z axix*/[triCount % 2]/*up or down tri*/, vertexList, 3 * sizeof(GzCoord));	
		triCount++;
	}

	if( fclose( infile ) )
		printf("the input file is not closed!");



	
	//generate normal grid
	//for each rect, there are 2 tris
	//for each tri, there are 3 vertices
	//so for each rect, there are 6 vertices, 2 of them overlapped
	for (int x = 0; x < RECT_MAX_X; x++) {
		for (int z = 0; z < RECT_MAX_Z; z++) {

			int count = 0;
			Triangle adjTris[8];
		
			count = gatherAdjTriangles(adjTris, TOP_TRI_V0, x, z);
			calculateVertexNormal(adjTris, count, x, z, 1, 0);

			count = gatherAdjTriangles(adjTris, TOP_TRI_V1, x, z);
			calculateVertexNormal(adjTris, count, x, z, 1, 1);

			count = gatherAdjTriangles(adjTris, TOP_TRI_V2, x, z);
			calculateVertexNormal(adjTris, count, x, z, 1, 2);

			count = gatherAdjTriangles(adjTris, BOTTOM_TRI_V0, x, z);
			calculateVertexNormal(adjTris, count, x, z, 0, 0);

			count = gatherAdjTriangles(adjTris, BOTTOM_TRI_V1, x, z);
			calculateVertexNormal(adjTris, count, x, z, 0, 1);

			count = gatherAdjTriangles(adjTris, BOTTOM_TRI_V2, x, z);
			calculateVertexNormal(adjTris, count, x, z, 0, 2);

		}
	}

	if( (infile  = fopen( inFileName , "r" )) == NULL )
	{
		printf("error opening the input file");
		return -1;
	}

	int triCount2 = 0;
	while( fscanf(infile, "%s", dummy) == 1) { 	/* read in tri word */
		fscanf(infile, "%f %f %f %f %f %f %f %f", 
			&(vertexList[0][0]), &(vertexList[0][1]),  
			&(vertexList[0][2]), 
			&(normalList[0][0]), &(normalList[0][1]), 	
			&(normalList[0][2]), 
			&(uvList[0][0]), &(uvList[0][1]) ); 
		fscanf(infile, "%f %f %f %f %f %f %f %f", 
			&(vertexList[1][0]), &(vertexList[1][1]), 	
			&(vertexList[1][2]), 
			&(normalList[1][0]), &(normalList[1][1]), 	
			&(normalList[1][2]), 
			&(uvList[1][0]), &(uvList[1][1]) ); 
		fscanf(infile, "%f %f %f %f %f %f %f %f", 
			&(vertexList[2][0]), &(vertexList[2][1]), 	
			&(vertexList[2][2]), 
			&(normalList[2][0]), &(normalList[2][1]), 	
			&(normalList[2][2]), 
			&(uvList[2][0]), &(uvList[2][1]) ); 


		//get normalList from normal grid
		memcpy(newNormalList, normalGrid[triCount2 / (RECT_MAX_Z * 2)]/*along x axis*/[(triCount2 / 2) % RECT_MAX_Z]/*along z axix*/[triCount2 % 2]/*up or down tri*/, 3 * sizeof(GzCoord));
		triCount2++;

		//normalize normals
		for(int i=0; i<3; i++)
		{
			Normalize(newNormalList[i]);
			Scale(newNormalList[i], -1.0f, newNormalList[i]);
		}

		fprintf(outfile, "Triangle\n");
		fprintf(outfile, "%f %f %f %f %f %f %f %f\n", 
			(vertexList[0][0]), (vertexList[0][1]),  
			(vertexList[0][2]), 
			(newNormalList[0][0]), (newNormalList[0][1]), 	
			(newNormalList[0][2]), 
			(uvList[0][0]), (uvList[0][1]) ); 
		fprintf(outfile, "%f %f %f %f %f %f %f %f\n", 
			(vertexList[1][0]), (vertexList[1][1]), 	
			(vertexList[1][2]), 
			(newNormalList[1][0]), (newNormalList[1][1]), 	
			(newNormalList[1][2]), 
			(uvList[1][0]), (uvList[1][1]) ); 
		fprintf(outfile, "%f %f %f %f %f %f %f %f\n", 
			(vertexList[2][0]), (vertexList[2][1]), 	
			(vertexList[2][2]), 
			(newNormalList[2][0]), (newNormalList[2][1]), 	
			(newNormalList[2][2]), 
			(uvList[2][0]), (uvList[2][1]) );


	}


	if( fclose( infile ) )
		printf("the input file is not closed!");

	if( fclose( outfile ) )
		printf("the output file is not closed!");


	return 1;
}


int HeightFieldMgr::gatherAdjTriangles(Triangle adjTris[8], int type, int x, int z) {

	int count = 0;

	/* neighbor vertices */

	switch (type)
	{
	case BOTTOM_TRI_V2:
	case TOP_TRI_V1:
		//2 tris in current rect

		memcpy(adjTris[count], grid[x][z]/*current rect*/[1]/*up tri*/, sizeof(Triangle));
		count++;

		memcpy(adjTris[count], grid[x][z]/*current rect*/[0]/*down tri*/, sizeof(Triangle));
		count++;

		//up
		if ((z + 1) < RECT_MAX_Z) {
			memcpy(adjTris[count], grid[x][z + 1]/*up rect*/[0]/*down tri*/, sizeof(Triangle));
			count++;
		}

		//right
		if ((x + 1) < RECT_MAX_X) {
			memcpy(adjTris[count], grid[x + 1][z]/*right rect*/[1]/*up tri*/, sizeof(Triangle));
			count++;
		}

		//2 tris in up right rect
		if ((z + 1) < RECT_MAX_Z && (x + 1) < RECT_MAX_X) {
			memcpy(adjTris[count], grid[x + 1][z + 1]/*up right rect*/[1]/*up tri*/, sizeof(Triangle));
			count++;
			memcpy(adjTris[count], grid[x + 1][z + 1]/*up right rect*/[0]/*down tri*/, sizeof(Triangle));
			count++;
		}
		break;
	case BOTTOM_TRI_V0:
	case TOP_TRI_V0:
		//2 tris in current rect

		memcpy(adjTris[count], grid[x][z]/*current rect*/[1]/*up tri*/, sizeof(Triangle));
		count++;

		memcpy(adjTris[count], grid[x][z]/*current rect*/[0]/*down tri*/, sizeof(Triangle));
		count++;

		//down
		if ((z - 1) >= 0) {
			memcpy(adjTris[count], grid[x][z - 1]/*down rect*/[1]/*up tri*/, sizeof(Triangle));
			count++;
		}

		//left
		if ((x - 1) >= 0) {
			memcpy(adjTris[count], grid[x - 1][z]/*left rect*/[0]/*down tri*/, sizeof(Triangle));
			count++;
		}

		//2 tris in down left rect
		if ((z - 1) >= 0 && (x - 1) >= 0) {
			memcpy(adjTris[count], grid[x - 1][z - 1]/*down left rect*/[1]/*up tri*/, sizeof(Triangle));
			count++;
			memcpy(adjTris[count], grid[x - 1][z - 1]/*down left rect*/[0]/*down tri*/, sizeof(Triangle));
			count++;
		}

		break;
	case BOTTOM_TRI_V1:
		//2 tris in down rect

		//down
		if ((z - 1) >= 0) {
			memcpy(adjTris[count], grid[x][z - 1]/*down rect*/[1]/*up tri*/, sizeof(Triangle));
			count++;

			memcpy(adjTris[count], grid[x][z - 1]/*down rect*/[0]/*down tri*/, sizeof(Triangle));
			count++;
		}

		//current
		memcpy(adjTris[count], grid[x][z]/*current rect*/[0]/*up tri*/, sizeof(Triangle));
		count++;


		//down right rect
		if ((z - 1) >= 0 && (x + 1) < RECT_MAX_X) {
			memcpy(adjTris[count], grid[x + 1][z - 1]/*down right rect*/[1]/*up tri*/, sizeof(Triangle));
			count++;
		}

		//2 tris in right rect
		if ((x + 1) < RECT_MAX_X) {
			memcpy(adjTris[count], grid[x + 1][z]/*right rect*/[1]/*up tri*/, sizeof(Triangle));
			count++;
			memcpy(adjTris[count], grid[x + 1][z]/*right rect*/[0]/*down tri*/, sizeof(Triangle));
			count++;
		}

		break;
	case TOP_TRI_V2:

		//2 tris in up rect

		//up
		if ((z + 1) < RECT_MAX_Z) {
			memcpy(adjTris[count], grid[x][z + 1]/*up rect*/[1]/*up tri*/, sizeof(Triangle));
			count++;

			memcpy(adjTris[count], grid[x][z + 1]/*up rect*/[0]/*down tri*/, sizeof(Triangle));
			count++;
		}

		//current
		memcpy(adjTris[count], grid[x][z]/*current rect*/[1]/*up tri*/, sizeof(Triangle));
		count++;


		//up left rect
		if ((z + 1) < RECT_MAX_Z && (x - 1) >= 0) {
			memcpy(adjTris[count], grid[x - 1][z + 1]/*up left rect*/[0]/*down tri*/, sizeof(Triangle));
			count++;
		}

		//2 tris in left rect
		if ((x - 1) >= 0) {
			memcpy(adjTris[count], grid[x - 1][z]/*left rect*/[1]/*up tri*/, sizeof(Triangle));
			count++;
			memcpy(adjTris[count], grid[x - 1][z]/*left rect*/[0]/*down tri*/, sizeof(Triangle));
			count++;
		}
		break;
	
	}


	return count;
}


int HeightFieldMgr::calculateVertexNormal(Triangle triList[8], int count, int x, int z, int udTri, int nthVertex) {

	//sumFaceNormal
	GzCoord sumFaceNormal = {0, 0, 0};
	//for current vertex neighbor's face normal
	GzCoord faceNormals[8];

	for (int i = 0; i < count; i++) {
		getFaceNormal(triList[i], faceNormals[i]);
	}


	for (int i = 0; i < count; i++) {
		sumFaceNormal[0] += faceNormals[i][0];
		sumFaceNormal[1] += faceNormals[i][1];
		sumFaceNormal[2] += faceNormals[i][2];
	}

	//final vertex normal
	sumFaceNormal[0] /= count;
	sumFaceNormal[1] /= count;
	sumFaceNormal[2] /= count;

	//normalize(sumFaceNormal);

	memcpy(normalGrid[x][z]/*current rect*/[udTri]/*up or down tri*/[nthVertex]/*nth vertex*/, sumFaceNormal, sizeof(GzCoord));

	return 1;
}

int HeightFieldMgr::normalize(GzCoord vector) {

	float len = sqrt(vector[0] * vector[0] + vector[1] * vector[1] + vector[2] * vector[2]);
	vector[0] /= len;
	vector[1] /= len;
	vector[2] /= len;

	return 1;
}




int HeightFieldMgr::getFaceNormal(Triangle tri, GzCoord normal) {
	GzCoord a, b;

	for (int c = 0; c < 3; c++) {
		a[c] = tri[0][c] - tri[1][c];
		b[c] = tri[1][c] - tri[2][c];
	}
	crossProduct(a, b, normal);

	return 1;
	
}

//a¡Áb=(aybz-azby)i+(azbx-axbz)j+(axby-aybx)k
int HeightFieldMgr::crossProduct(GzCoord a, GzCoord b, GzCoord c) {

	c[0] = a[1] * b[2] - a[2] * b[1];//ay*bz-az*by
	c[1] = a[2] * b[0] - a[0] * b[2];//az*bx-ax*bz
	c[2] = a[0] * b[1] - a[1] * b[0];//ax*by-ay*bx

	return 1;
}





/************************************************************************/
/* Generate height field                                                                     */
/************************************************************************/



int HeightFieldMgr::generateHeightField(char* inFileName, char* outFileName) {
	GzToken		nameListTriangle[3]; 	/* vertex attribute names */
	GzPointer	valueListTriangle[3]; 	/* vertex attribute pointers */
	GzCoord		vertexList[3];	/* vertex position coordinates */ 
	GzCoord		normalList[3];	/* vertex normals */ 
	GzCoord		newVertexList[3];
	GzCoord		newNormalList[3];
	GzCoord		heightList;
	GzTextureIndex  	uvList[3];		/* vertex texture map indices */ 
	char		dummy[256]; 


	FILE *infile;
	if( (infile  = fopen( inFileName , "r" )) == NULL )
	{
		printf("error opening the input file");
		return -1;
	}

	FILE *outfile;
	if( (outfile  = fopen( outFileName , "w" )) == NULL )
	{
		printf("error opening the output file");
		return -1;
	}


	int triCount = 0;
	while( fscanf(infile, "%s", dummy) == 1) { 	/* read in tri word */
		fscanf(infile, "%f %f %f %f %f %f %f %f", 
			&(vertexList[0][0]), &(vertexList[0][1]),  
			&(vertexList[0][2]), 
			&(normalList[0][0]), &(normalList[0][1]), 	
			&(normalList[0][2]), 
			&(uvList[0][0]), &(uvList[0][1]) ); 
		fscanf(infile, "%f %f %f %f %f %f %f %f", 
			&(vertexList[1][0]), &(vertexList[1][1]), 	
			&(vertexList[1][2]), 
			&(normalList[1][0]), &(normalList[1][1]), 	
			&(normalList[1][2]), 
			&(uvList[1][0]), &(uvList[1][1]) ); 
		fscanf(infile, "%f %f %f %f %f %f %f %f", 
			&(vertexList[2][0]), &(vertexList[2][1]), 	
			&(vertexList[2][2]), 
			&(normalList[2][0]), &(normalList[2][1]), 	
			&(normalList[2][2]), 
			&(uvList[2][0]), &(uvList[2][1]) ); 

	
		//generate height

		for (int i = 0; i < 3; i++) {
			float u = uvList[i][0];
			float v = uvList[i][1];
			u *= (X_RES - 1);
			v *= (Y_RES - 1);

			heightList[i] = turbulence(u, v);
		}


		//calculate new vertex
		for (int i = 0; i < 3; i++) {
			getHeightField (vertexList[i], normalList[i], heightList[i], newVertexList[i]);
		}


		fprintf(outfile, "Triangle\n");
		fprintf(outfile, "%f %f %f %f %f %f %f %f\n", 
			(newVertexList[0][0]), (newVertexList[0][1]),  
			(newVertexList[0][2]), 
			(normalList[0][0]), (normalList[0][1]), 	
			(normalList[0][2]), 
			(uvList[0][0]), (uvList[0][1]) ); 
		fprintf(outfile, "%f %f %f %f %f %f %f %f\n", 
			(newVertexList[1][0]), (newVertexList[1][1]), 	
			(newVertexList[1][2]), 
			(normalList[1][0]), (normalList[1][1]), 	
			(normalList[1][2]), 
			(uvList[1][0]), (uvList[1][1]) ); 
		fprintf(outfile, "%f %f %f %f %f %f %f %f\n", 
			(newVertexList[2][0]), (newVertexList[2][1]), 	
			(newVertexList[2][2]), 
			(normalList[2][0]), (normalList[2][1]), 	
			(normalList[2][2]), 
			(uvList[2][0]), (uvList[2][1]) );
	}


	if( fclose( infile ) )
		printf("the input file is not closed!");

	if( fclose( outfile ) )
		printf("the output file is not closed!");
	return 1;
}



int HeightFieldMgr::getHeightField(GzCoord vertex, GzCoord normal, float height, GzCoord newVertex) {
	for (int i = 0; i < 3; i++) {
		newVertex[i] = vertex[i] + height * normal[i];
	}
	return 1;
}

namespace
{
float lerp(float v1, float v2, float t)
{
	return v1*t + v2*(1-t);
}

/* Image texture function */
int tex_fun(float u, float v, GzColor color, const std::string& name, const std::string& file_path)
{
	/* bounds-test u,v to make sure nothing will overflow image array bounds */
	/* determine texture cell corner values and perform bilinear interpolation */
	/* set color to interpolated GzColor value and return */

	Image* image = ImageManager::GetSingleton()->GetImage(name, file_path);
	u = u > (1-1e-5) ? 1.0f : u;
	u = u< 1e-5 ? 0.0f  : u;
	v = v > (1-1e-5) ? 1.0f : v;
	v =  v < 1e-5  ? 0.0f  : v;
	float x = u*(image->width-2)+1, y = v*(image->height-2)+1;
	int nb[4][2] = { {(int)x, (int)y}, {(int)x, (int)y+1}, {(int)x+1, (int)y+1}, {(int)x+1, (int)y}};
	for(int i=0; i<4; i++)
	{
		nb[i][0] = nb[i][0] < 0 ? 0 : nb[i][0];
		nb[i][0] = nb[i][0] >= image->width ? image->width-1 : nb[i][0];
		nb[i][1] = nb[i][1] < 0 ? 0 : nb[i][1];
		nb[i][1] = nb[i][1] >= image->height ? image->height-1 : nb[i][1];
	}
	GzColor c[4];
	for(int i=0; i<4; i++)
	{
		const float* pixel = image->GetPixel(nb[i][0],nb[i][1]);
		c[i][0] = pixel[0];
		c[i][1] = pixel[1];
		c[i][2] = pixel[2];
	}

	//interpolate y
	GzColor c01, c32;
	for(int i=0; i<3; i++)
	{
		c01[i] = lerp(c[1][i], c[0][i], y-nb[0][1]); 
		c32[i] = lerp(c[2][i], c[3][i], y-nb[3][1]);
	}

	//interpolate x
	for(int i=0; i<3; i++)
	{
		color[i] = lerp(c32[i], c01[i], x-nb[0][0]);
	}

	return 0;
}
}

float HeightFieldMgr::noise(int u, int v) {
	double pi = 3.1415926;
	return ((X_RES + Y_RES)) / 2 * sin( (u + v - ((X_RES + Y_RES)) / 2) * (pi / 2) *  (180 / (pi)) );

}


float HeightFieldMgr::smoothNoise(float u, float v) {
	/*
	float corners = ( noise(u - 1, v - 1) + noise(u + 1, v - 1) + noise(u - 1, v + 1) + noise(u + 1, v + 1) ) / 16;
	float sides   = ( noise(u - 1, v) + noise(u + 1, v) + noise(u, v - 1) + noise(u, v + 1) ) /  8;
	float center  =  noise(u, v) / 4;

	return (corners + sides + center);
	*/
	float ui  = ((int)u & 127)/127.0f, vi = ((int)v & 127) / 127.0f;
	GzColor tex;
	tex_fun(ui, vi ,tex, "Noise", "noise.ppm");
	float result = tex[0] + tex[1] + tex[2];
	return result;
}


float HeightFieldMgr::interpNoise(float u, float v) {
	int iU = (int) u;
	int iV = (int) v;

#define SMOOTH(v) v*v*v*(v*(v*6.0 - 15.0) + 10.0)
	float fractional_U = u - iU;
	fractional_U = SMOOTH(fractional_U);
	float fractional_V = v - iV;
	fractional_V = SMOOTH(fractional_V);
#undef SMOOTH

	float nA = smoothNoise (iU, iV);
	float nB = smoothNoise (iU + 1, iV);
	float nC = smoothNoise (iU + 1, iV + 1);
	float nD = smoothNoise (iU, iV + 1);
	
	//bi-linear
	return fractional_U * fractional_V * nC + (1 - fractional_U) * fractional_V * nD + fractional_U * (1 - fractional_V) * nB + (1 - fractional_U) * (1 - fractional_V) * nA;
}


float HeightFieldMgr::turbulence(float u, float v) {
	float total = 0;
	float p = 0.5f;
	int n = 6;

	for (int i = 0; i < n; i++) {
		int frequency = pow(2.0, i);
		float amplitude = pow(p, i);

		total += interpNoise(u * frequency, v * frequency) * amplitude*0.3f;
	}
	total += interpNoise(u * 128.0, v * 128.0) * 0.15f;

	return total;
}
