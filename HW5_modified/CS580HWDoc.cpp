// CS580HWDoc.cpp : implementation of the CCS580HWDoc class
//

#include "stdafx.h"
#include "CS580HW.h"
#include	"Gz.h"
#include "TransMatrix.h"

#include "CS580HWDoc.h"
#include "CS580HWView.h"
#include "MainFrm.h"
#include "Application5.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCS580HWDoc

IMPLEMENT_DYNCREATE(CCS580HWDoc, CDocument)

BEGIN_MESSAGE_MAP(CCS580HWDoc, CDocument)
	//{{AFX_MSG_MAP(CCS580HWDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_RENDER_GENMESH, &CCS580HWDoc::OnRenderGenmesh)
	ON_COMMAND(ID_RENDER_GENPROJMESH, &CCS580HWDoc::OnRenderGenprojmesh)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCS580HWDoc construction/destruction

CCS580HWDoc::CCS580HWDoc()
{
	// TODO: add one-time construction code here

}

CCS580HWDoc::~CCS580HWDoc()
{
}

BOOL CCS580HWDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CCS580HWDoc serialization

void CCS580HWDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CCS580HWDoc diagnostics

#ifdef _DEBUG
void CCS580HWDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CCS580HWDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CCS580HWDoc commands

void CCS580HWDoc::OnRenderGenmesh()
{
	// TODO: Add your command handler code here
    // The plane was placed at xz plane in the world space
	// a grid with the resolution of 31x31 vertices
	// grid dimension:24x24 in world space
    float u, v;
	float meshTri1V1[3];
	float meshTri1V2[3];
	float meshTri1V3[3];
    float meshTri2V1[3];
	float meshTri2V2[3];
	float meshTri2V3[3];
	xNum = zNum = 31;
    xDim = zDim = 24;
	xRes = (float)xDim/(float)(xNum - 1);
	zRes = (float)zDim/(float)(zNum - 1); 

	CString OutPutFile;
	OutPutFile = "WorldSpaceMesh.txt";
	FILE *outfile ;
	outfile = fopen ( OutPutFile , "wb" );


         for (int i=0; i<xNum-1;i++ )
         {
			 for (int j=0;j<zNum-1;j++)
			 {

			 		
             //First triangle vertices
				 meshTri1V1[X] = i*xRes;
				 meshTri1V1[Y] = 0;
				 meshTri1V1[Z] = j*zRes;
				 meshTri1V2[X] = (i+1)*xRes;
				 meshTri1V2[Y] = 0;
				 meshTri1V2[Z] = j*zRes;
				 meshTri1V3[X] = (i+1)*xRes;
				 meshTri1V3[Y] = 0;
				 meshTri1V3[Z] = (j+1)*zRes;
			 //Second triangle vertices
				 meshTri2V1[X] = i*xRes;
				 meshTri2V1[Y] = 0;
				 meshTri2V1[Z] = j*zRes;
				 meshTri2V2[X] = (i+1)*xRes;
				 meshTri2V2[Y] = 0;
				 meshTri2V2[Z] = (j+1)*zRes;
				 meshTri2V3[X] = i*xRes;
				 meshTri2V3[Y] = 0;
				 meshTri2V3[Z] = (j+1)*zRes;
		   fprintf( outfile, "Triangle\r\n");
		   fprintf( outfile, "%f %f %f " ,  meshTri1V1[X],  meshTri1V1[Y], meshTri1V1[Z] );
		   u = i*xRes/xDim; v= j*zRes/zDim;
		   fprintf( outfile, "0.00 1.00 0.00 ");//normal
		   fprintf( outfile, "%f %f\r\n", u,v);


		   fprintf( outfile, "%f %f %f " ,  meshTri1V2[X],  meshTri1V2[Y], meshTri1V2[Z] );
		   u = (i+1)*xRes/xDim; v= j*zRes/zDim;
		   fprintf( outfile, "0.00 1.00 0.00 ");//normal
		   fprintf( outfile, "%f %f\r\n", u,v);


		   fprintf( outfile, "%f %f %f " ,  meshTri1V3[X],  meshTri1V3[Y], meshTri1V3[Z] );
		   u = (i+1)*xRes/xDim; v= (j+1)*zRes/zDim;
		   fprintf( outfile, "0.00 1.00 0.00 ");//normal
		   fprintf( outfile, "%f %f\r\n", u,v);


		   fprintf( outfile, "Triangle\r\n");
		   fprintf( outfile, "%f %f %f " ,  meshTri2V1[X],  meshTri2V1[Y], meshTri2V1[Z] );
		   u = i*xRes/xDim; v= j*zRes/zDim;
		   fprintf( outfile, "0.00 1.00 0.00 ");//normal
		   fprintf( outfile, "%f %f\r\n", u,v);

		   fprintf( outfile, "%f %f %f " ,  meshTri2V2[X],  meshTri2V2[Y], meshTri2V2[Z] );
		   u = (i+1)*xRes/xDim; v= (j+1)*zRes/zDim;
		   fprintf( outfile, "0.00 1.00 0.00 ");//normal
		   fprintf( outfile, "%f %f\r\n", u,v);

		   fprintf( outfile, "%f %f %f " ,  meshTri2V3[X],  meshTri2V3[Y], meshTri2V3[Z] );
		   u = i*xRes/xDim; v= (j+1)*zRes/zDim;
		   fprintf( outfile, "0.00 1.00 0.00 ");//normal
		   fprintf( outfile, "%f %f\r\n", u,v);

			 }
         }
		 AfxMessageBox("World Space Mesh grid is generated!");

}

void CCS580HWDoc::OnRenderGenprojmesh()
{
	// TODO: Add your command handler code here

	// The plane was placed at xz plane in the NDC space
	// a grid with the resolution of 31x31 vertices
	// grid dimension:1x1 in world space

	float u, v;
	float meshTri1V1[4];
	float meshTri1V2[4];
	float meshTri1V3[4];
	float meshTri2V1[4];
	float meshTri2V2[4];
	float meshTri2V3[4];
	xNum = yNum = 31;
	xDim = yDim = 1;
	xRes = (float)xDim/(float)(xNum - 1);
	yRes = (float)yDim/(float)(yNum - 1); 

    CTransMatrix MProjection;
    CTransMatrix XiwTrans, XpiTrans;

	//set Xsp, Ximage elements

	CMainFrame * pMframe = (CMainFrame *) AfxGetApp()->m_pMainWnd;
    CCS580HWView * pView = (CCS580HWView *)pMframe->GetActiveView();
	Application5* pApp5 = (Application5 *)pView->m_pApplication ;


   for (int i=0;i<4;i++)
   {
	   for (int j=0;j<4;j++)
	   {
		   XiwTrans.SetElementValue(i,j,(double)pApp5->m_pRender->camera.Xiw[i][j]);
           XpiTrans.SetElementValue(i,j,(double)pApp5->m_pRender->camera.Xpi[i][j]);
	   }
   }

   MProjection = XpiTrans;
   MProjection *=XiwTrans;
  // MProjection.Inverse(); //The inverse result is wrong. Calculate it by hand
   MProjection.SetElementValue(0,0,-0.333);
   MProjection.SetElementValue(0,1,0.000);
   MProjection.SetElementValue(0,2,0.000);
   MProjection.SetElementValue(0,3,0.000);
   MProjection.SetElementValue(1,0,0.000);
   MProjection.SetElementValue(1,1,0.707);
   MProjection.SetElementValue(1,2,-5.769);
   MProjection.SetElementValue(1,3,10.000);
   MProjection.SetElementValue(2,0,0.000);
   MProjection.SetElementValue(2,1,-0.471);
   MProjection.SetElementValue(2,2,-3.846);
   MProjection.SetElementValue(2,3,6.667);
   MProjection.SetElementValue(3,0,0.000);
   MProjection.SetElementValue(3,1,0.000);
   MProjection.SetElementValue(3,2,-0.506);
   MProjection.SetElementValue(3,3,1.000);



 //  //**********************
 //   XspTrans.Inverse();
	//MProjection = XspTrans;
	//MProjection *= XimageTemp; 
	//MProjection.Inverse();//get the projection matrix

//transform and output the vertices

	CString OutPutFile;
	OutPutFile = "WorldSpaceProjMesh.txt";
	FILE *outfile ;
	outfile = fopen ( OutPutFile , "wb" );

//Transform four corners of the grid

	float  leftbottomper[4], rightbottomper[4], lefttopper[4], righttopper[4];
    leftbottomper[0] = leftbottomper[1] = rightbottomper[1]= lefttopper[0] = 0;
	rightbottomper[0] = lefttopper[1] = righttopper[0] = righttopper[1] = 1;
	leftbottomper[3] = rightbottomper[3] = lefttopper[3] = righttopper[3] = 1;
     
	
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

  



for (int i=0; i<xNum-1;i++ )
	{
		for (int j=0;j<yNum-1;j++)
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

AfxMessageBox("Projected mesh grid is generated!");

}

void CCS580HWDoc::LinePlaneIntersection(float hPoint1[4], float hPoint2[4], float hPoint[4])
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

void CCS580HWDoc::InterpolateHomoCoord(int i, int j, float PointWorld[4])
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
