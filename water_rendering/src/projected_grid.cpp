#include "projected_grid.h"
#include "Gz.h"
#include "TransMatrix.h"
#include "utilities.h"
#include "HeightFieldMgr.h"

static float xRes = 0.0f, yRes = 0.0f;
static float m_LeftBottom[4], m_RightBottom[4], m_LeftTop[4], m_RightTop[4];

static void LinePlaneIntersection(float hPoint1[4], float hPoint2[4], float hPoint[4])
{
	//set the plane to be y=h, h=0

	float detaX, detaY, detaZ, detaW, t;
	float h = 0;
	detaX = hPoint2[0] - hPoint1[0];
	detaY = hPoint2[1] - hPoint1[1];
	detaZ = hPoint2[2] - hPoint1[2];
	detaW = hPoint2[3] - hPoint1[3];

	t = (float)(hPoint1[3]*h-hPoint1[1])/(float)(detaY - detaW * h);

	hPoint[0] = hPoint1[0] + detaX * t;
	hPoint[1] = hPoint1[1] + detaY * t;
	hPoint[2] = hPoint1[2] + detaZ * t;
	hPoint[3] = hPoint1[3] + detaW * t;

}

static void InterpolateHomoCoord(int i, int j, float PointWorld[4])
{

	float x, y;
	x = i * xRes;
	y = j * yRes;

	//bilinear function: h(i,j)=a00+a10*x+a01*y+a11*xy
	//a00=h1, a10=h2-h1, a01=h3-h1, a11=h1-h2-h3+h4
	for (int i=0;i<4;i++)
	{
		PointWorld[i] = m_LeftBottom[i] + (m_RightBottom[i]-m_LeftBottom[i])*x + (m_LeftTop[i]-m_LeftBottom[i])*y + (m_LeftBottom[i]-m_RightBottom[i]-m_LeftTop[i]+m_RightTop[i])*x*y;
	}	

	PointWorld[0]=PointWorld[0]/PointWorld[3];
	PointWorld[1]=PointWorld[1]/PointWorld[3];
	PointWorld[2]=PointWorld[2]/PointWorld[3];
	PointWorld[3]=PointWorld[3]/PointWorld[3];


}

static void generated_grid(const char* OutPutFile, GzRender* renderer)
{
	static float up_bias = 30.0f, forward_bias = 20.0f;

	float u, v;
	float meshTri1V1[4];
	float meshTri1V2[4];
	float meshTri1V3[4];
	float meshTri2V1[4];
	float meshTri2V2[4];
	float meshTri2V3[4];
	xRes = 1.0f/RECT_MAX_X;
	yRes = 1.0f/RECT_MAX_Z; 

    CTransMatrix MProjection;
    CTransMatrix XiwTrans, XpiTrans;

	//set Xsp, Ximage elements
	GzCoord view_vector;
	VectorSubtract(renderer->camera.lookat, renderer->camera.position, view_vector);
	Normalize(view_vector);
	if(view_vector[1] < 0.0f)			//look at y=0
	{
		GzCoord camera_pos;
		Scale(renderer->camera.position, 1.0f, camera_pos);
		renderer->camera.position[1] = camera_pos[1] + up_bias;
		renderer->camera.lookat[0] = camera_pos[0] - camera_pos[1]/view_vector[1]*view_vector[0];
		renderer->camera.lookat[1] = 0.0f;
		renderer->camera.lookat[2] = camera_pos[2] - camera_pos[1]/view_vector[1]*view_vector[2];
		GzPutCamera(renderer, &renderer->camera);
	}
	else										//look away from y=0
	{
		GzCoord camera_pos;
		Scale(renderer->camera.position, 1.0f, camera_pos);
		GzCoord camera_forward = {view_vector[0], 0.0f, view_vector[2]};
		Normalize(camera_forward);
		renderer->camera.position[1] = camera_pos[1] + up_bias;
		renderer->camera.lookat[0] = camera_pos[0] + forward_bias * camera_forward[0];
		renderer->camera.lookat[1] = 0.0f;
		renderer->camera.lookat[2] = camera_pos[2]  + forward_bias * camera_forward[2];
		GzPutCamera(renderer, &renderer->camera);
	}
	for (int i=0;i<4;i++)
	{
		for (int j=0;j<4;j++)
		{
			XiwTrans.SetElementValue(i,j,(double)renderer->camera.Xiw[i][j]);
			XpiTrans.SetElementValue(i,j,(double)renderer->camera.Xpi[i][j]);
		}
	}

	MProjection = XpiTrans;
	MProjection *=XiwTrans;
	MatrixInverse(MProjection.matrixData, MProjection.matrixData);

//transform and output the vertices

	FILE *outfile ;
	outfile = fopen ( OutPutFile , "wb" );

//Transform four corners of the grid

	float  leftbottomper[4] = {-1.0f, -1.0f, -1.0f, 1.0f}, 
			rightbottomper[4] = {1.0f, -1.0f, -1.0f, 1.0f}, 
			lefttopper[4] = {-1.0f, 1.0f, -1.0f, 1.0f}, 
			righttopper[4] = {1.0f, 1.0f, -1.0f, 1.0f};
     
    float  leftbottom1[4], rightbottom1[4], lefttop1[4], righttop1[4];
	float  leftbottom2[4], rightbottom2[4], lefttop2[4], righttop2[4];
     //transform leftbottom to world space
          //set z to -1
	       leftbottomper[2] = -1;
           MProjection.GetTransformedHVertex(leftbottomper,leftbottom1);
		   leftbottomper[2] = 1;
		   MProjection.GetTransformedHVertex(leftbottomper,leftbottom2);
           LinePlaneIntersection(leftbottom1, leftbottom2, m_LeftBottom);   
		   
    //transform rightbottom to world space
	       rightbottomper[2] = -1;
		   MProjection.GetTransformedHVertex(rightbottomper,rightbottom1);
		  rightbottomper[2] = 1;
		   MProjection.GetTransformedHVertex(rightbottomper,rightbottom2);
		   LinePlaneIntersection(rightbottom1, rightbottom2, m_RightBottom);

	//transform lefttop to world space
	       lefttopper[2] = -1;
		   MProjection.GetTransformedHVertex(lefttopper,lefttop1);
		   lefttopper[2] = 1;
		   MProjection.GetTransformedHVertex(lefttopper,lefttop2);
		   LinePlaneIntersection(lefttop1, lefttop2, m_LeftTop);

	//transform righttop to world space
		   righttopper[2] = -1;
		   MProjection.GetTransformedHVertex(righttopper,righttop1);
		  righttopper[2] = 1;
		   MProjection.GetTransformedHVertex(righttopper,righttop2);
		   LinePlaneIntersection(righttop1, righttop2, m_RightTop);

//interpolate homegeneous coordinates
   float current_Vertex[4];

for (int i=0; i<RECT_MAX_X;i++ )
	{
		for (int j=0;j<RECT_MAX_Z;j++)
		{
			  fprintf( outfile, "Triangle\r\n");         
			  InterpolateHomoCoord(i,j,meshTri1V1);
			  u = i*xRes; v= j*yRes;
			  fprintf( outfile, "%f %f %f " ,  meshTri1V1[X],  meshTri1V1[Y], meshTri1V1[Z] );
			  fprintf( outfile, "0.00 1.00 0.00 ");//normal
			  fprintf( outfile, "%f %f\r\n", u,v);

			  InterpolateHomoCoord(i+1,j,meshTri1V2);
			  u = (i+1)*xRes; v= j*yRes;
			  fprintf( outfile, "%f %f %f " ,  meshTri1V2[X],  meshTri1V2[Y], meshTri1V2[Z] );
			  fprintf( outfile, "0.00 1.00 0.00 ");//normal
			  fprintf( outfile, "%f %f\r\n", u,v);

			  InterpolateHomoCoord(i+1,j+1,meshTri1V3);
			  u = (i+1)*xRes; v= (j+1)*yRes;
			  fprintf( outfile, "%f %f %f " ,  meshTri1V3[X],  meshTri1V3[Y], meshTri1V3[Z] );
			  fprintf( outfile, "0.00 1.00 0.00 ");//normal
			  fprintf( outfile, "%f %f\r\n", u,v);
            

			  //Second triangle vertices
			  fprintf( outfile, "Triangle\r\n");  
			  u = i*xRes; v= j*yRes;
			  fprintf( outfile, "%f %f %f " ,  meshTri1V1[X],  meshTri1V1[Y], meshTri1V1[Z] );
			  fprintf( outfile, "0.00 1.00 0.00 ");//normal
			  fprintf( outfile, "%f %f\r\n", u,v);

			  u = (i+1)*xRes; v= (j+1)*yRes;
			  fprintf( outfile, "%f %f %f " ,  meshTri1V3[X],  meshTri1V3[Y], meshTri1V3[Z] );
			  fprintf( outfile, "0.00 1.00 0.00 ");//normal
			  fprintf( outfile, "%f %f\r\n", u,v);

			  InterpolateHomoCoord(i,j+1,meshTri2V3);
			  u = i*xRes; v= (j+1)*yRes;
			  fprintf( outfile, "%f %f %f " ,  meshTri2V3[X],  meshTri2V3[Y], meshTri2V3[Z] );
			  fprintf( outfile, "0.00 1.00 0.00 ");//normal
			  fprintf( outfile, "%f %f\r\n", u,v);
		}
	}
}

void generate_water_mesh(const char* output_path, GzRender* renderer)
{
	char* meshFileName = "WorldSpaceProjMesh.asc";
	char* intermediateFileName = "heightField.asc";
	char* outputFileName = "water_plane.asc";

	generated_grid(meshFileName, renderer);
	HeightFieldMgr mgr;
	mgr.generateHeightField(meshFileName, intermediateFileName);
	mgr.generateNormals(intermediateFileName, outputFileName);
}