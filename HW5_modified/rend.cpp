/* CS580 Homework 3 */

#include	"stdafx.h"
#include	"stdio.h"
#include	"math.h"
#include	"Gz.h"
#include	"rend.h"


int GzRotXMat(float degree, GzMatrix mat)
{
// Create rotate matrix : rotate along x axis
// Pass back the matrix using mat value
	mat[1][1] = (float)cos(degree *3.14/180);
	mat[1][2] = (float)(-sin(degree *3.14/180));
	mat[2][1] = (float)sin(degree *3.14/180);
	mat[2][2] = (float)cos(degree *3.14/180);
	return GZ_SUCCESS;
}


int GzRotYMat(float degree, GzMatrix mat)
{
// Create rotate matrix : rotate along y axis
// Pass back the matrix using mat value
	mat[0][0] = (float)cos(degree *3.14/180);
	mat[0][2] = (float)sin(degree *3.14/180);
	mat[2][0] = (float)(-sin(degree *3.14/180));
	mat[2][2] = (float)cos(degree *3.14/180);
	return GZ_SUCCESS;
}


int GzRotZMat(float degree, GzMatrix mat)
{
// Create rotate matrix : rotate along z axis
// Pass back the matrix using mat value
	mat[0][0] = (float)cos(degree *3.14/180);
	mat[0][1] = (float)(-sin(degree *3.14/180));
	mat[1][0] = (float)sin(degree *3.14/180);
	mat[1][1] = (float)cos(degree *3.14/180);
	return GZ_SUCCESS;
}


int GzTrxMat(GzCoord translate, GzMatrix mat)
{
// Create translation matrix
// Pass back the matrix using mat value

	mat[0][3] = translate[0];
    mat[1][3] = translate[1];
    mat[2][3] = translate[2];
	return GZ_SUCCESS;
}


int GzScaleMat(GzCoord scale, GzMatrix mat)
{
// Create scaling matrix
// Pass back the matrix using mat value

	mat[0][0] = scale[0];
    mat[1][1] = scale[1];
    mat[2][2] = scale[2];
	return GZ_SUCCESS;
}


//----------------------------------------------------------
// Begin main functions

int GzNewRender(GzRender **render, GzRenderClass renderClass, GzDisplay	*display)
{
/*  
- malloc a renderer struct 
- keep closed until all inits are done 
- setup Xsp and anything only done once 
- span interpolator needs pointer to display 
- check for legal class GZ_Z_BUFFER_RENDER 
- init default camera 
*/ 
	*render =  new GzRender;
	(*render)->renderClass = renderClass;
	(*render)->open = 0;
	(*render)->display = display;
    (*render)->matlevel = 0;
	(*render)->numlights = 0;

	
	(*render)->camera.position[X] =DEFAULT_IM_X;      
	(*render)->camera.position[Y] = DEFAULT_IM_Y;
	(*render)->camera.position[Z] = DEFAULT_IM_Z;

	(*render)->camera.lookat[X] = 0;
	(*render)->camera.lookat[Y] = 0;
	(*render)->camera.lookat[Z] = 0;

	(*render)->camera.worldup[X] = 0;
	(*render)->camera.worldup[Y] = 1.0;
	(*render)->camera.worldup[Z] = 0.0;

	(*render)->camera.FOV = DEFAULT_FOV;              /* degrees */
	Normalized((*render)->camera.worldup);
	

	SetIdentity((*render)->Ximage[ (*render)->matlevel]);
	SetIdentity((*render)->Xnorm[(*render)->matlevel]);
	
	
	
	
	return GZ_SUCCESS;

}


int GzFreeRender(GzRender *render)
{
/* 
-free all renderer resources
*/
	delete render;
	return GZ_SUCCESS;
}


int GzBeginRender(GzRender *render)
{
/*  
- set up for start of each frame - clear frame buffer 
- compute Xiw and projection xform Xpi from camera definition 
- init Ximage - put Xsp at base of stack, push on Xpi and Xiw 
- now stack contains Xsw and app can push model Xforms if it want to. 
*/ 
    GzCoord Xcam, Ycam, Zcam;
    Zcam[X] = render->camera.lookat[X] - render->camera.position[X];
	Zcam[Y] = render->camera.lookat[Y] - render->camera.position[Y];
	Zcam[Z] = render->camera.lookat[Z] - render->camera.position[Z];
	Normalized(Zcam);
    
	GzCoord UP;
	UP[X] = render->camera.worldup[0];
	UP[Y] = render->camera.worldup[1];
	UP[Z] = render->camera.worldup[2];
	numDotVector(dotproduct(UP,Zcam),Zcam,Ycam);
	vectorMinus(UP,Ycam,Ycam);
    Normalized(Ycam);

	/*Ycam[X] = render->camera.worldup[X];
	Ycam[Y] = render->camera.worldup[Y];
	Ycam[Z] = render->camera.worldup[Z];
	Normalized(Ycam);*/

	CrossProduct(Ycam,Zcam,Xcam);
	memset(render->camera.Xiw,0,sizeof(GzMatrix));
    //set Xiw elemetns
	render->camera.Xiw[0][0]=Xcam[X];
	render->camera.Xiw[0][1]=Xcam[Y];
	render->camera.Xiw[0][2]=Xcam[Z];
	render->camera.Xiw[0][3]=-dotproduct(Xcam,render->camera.position);
	render->camera.Xiw[1][0]=Ycam[X];
	render->camera.Xiw[1][1]=Ycam[Y];
	render->camera.Xiw[1][2]=Ycam[Z];
    render->camera.Xiw[1][3]=-dotproduct(Ycam,render->camera.position);
	render->camera.Xiw[2][0]=Zcam[X];
	render->camera.Xiw[2][1]=Zcam[Y];
	render->camera.Xiw[2][2]=Zcam[Z];
    render->camera.Xiw[2][3]=-dotproduct(Zcam,render->camera.position);
    render->camera.Xiw[3][3]=1;
	//set Xpi elements
	float dd; //dd=1/d
	dd = (float)tan((render->camera.FOV/(float)2)*3.14159265/180);
	memset(render->camera.Xpi,0,sizeof(GzMatrix));
    render->camera.Xpi[0][0]=1;
    render->camera.Xpi[1][1]=1;
    render->camera.Xpi[2][2]=1;
	render->camera.Xpi[3][3]=1;
	render->camera.Xpi[3][2]=dd;

	//set Xsp elements
	float d =  ( float )( 1 / tan(((render)->camera.FOV ) / 2 * 3.14159265/180 ) );
	memset((render)->Xsp,0,sizeof(GzMatrix));
	(render)->Xsp[0][0] = (render->display->xres/(float)2);
	(render)->Xsp[0][3] =(render->display->xres/(float)2);
	(render)->Xsp[1][1] =(-render->display->yres/(float)2);
	(render)->Xsp[1][3] =(render->display->yres/(float)2);
	(render)->Xsp[2][2] = INT_MAX /d;
	(render)->Xsp[3][3] = 1;

    //Ximage initialization
	GzPushMatrix(render,render->Xsp);
	GzPushMatrix(render,render->camera.Xpi);
	GzPushMatrix(render,render->camera.Xiw);

	//Xnorm initialization HW4
    GzMatrix Xim;
	memcpy(Xim,render->camera.Xiw,sizeof(GzMatrix));
	ToUnitaryRotation(Xim);
    GzPushMatrixToXnorm(render, Xim);

	render->open = 1;


	return GZ_SUCCESS;
}

int GzPutCamera(GzRender *render, GzCamera *camera)
{
/*
- overwrite renderer camera structure with new camera definition
*/
	render->camera.position[X] = camera->position[X];      
	render->camera.position[Y] =camera->position[Y];
	render->camera.position[Z] = camera->position[Z];

	render->camera.lookat[X] = camera->lookat[X];
	render->camera.lookat[Y] = camera->lookat[Y];
	render->camera.lookat[Z] = camera->lookat[Z];

	render->camera.worldup[X] = camera->worldup[X];
	render->camera.worldup[Y] =camera->worldup[Y];
	render->camera.worldup[Z] = camera->worldup[Z];

	render->camera.FOV = camera->FOV;              /* degrees */
	Normalized(render->camera.worldup);
	
	return GZ_SUCCESS;	
}

int GzPushMatrix(GzRender *render, GzMatrix	matrix)
{
/*
- push a matrix onto the Ximage stack
- check for stack overflow
*/
   GzMatrix MulMatrix;
   MatrixMultiply(render->Ximage[render->matlevel],matrix,MulMatrix);
   render->matlevel =render->matlevel + 1;
  if (render->matlevel<MATLEVELS)
  {
	  memcpy(render->Ximage[render->matlevel],MulMatrix,sizeof(GzMatrix));
      return GZ_SUCCESS;
  }
  else
  {
	  return GZ_FAILURE;
  }

	
}

int GzPopMatrix(GzRender *render)
{
/*
- pop a matrix off the Ximage stack
- check for stack underflow
*/
	SetIdentity(render->Ximage[render->matlevel]);
	render->matlevel = render->matlevel - 1;
	if(render->matlevel < 0)
	{
        render->matlevel = 0;   
		return GZ_SUCCESS;
	}
	
}


//*********************************hw4 task one: set lights and shaders*****************************//
int GzPutAttribute(GzRender	*render, int numAttributes, GzToken	*nameList, 
	GzPointer	*valueList) /* void** valuelist */
{
/*
- set renderer attribute states (e.g.: GZ_RGB_COLOR default color)
- later set shaders, interpolaters, texture maps, and lights
*/
    for (int index = 0; index < numAttributes;index++)
    {
		switch (nameList[index])
		{
//**************light attribute***************
		case GZ_DIRECTIONAL_LIGHT:
			{
				//put light direction
              render->lights[render->numlights].direction[0] = ((GzLight *)valueList[index])->direction[0];
			  render->lights[render->numlights].direction[1] = ((GzLight *)valueList[index])->direction[1];
			  render->lights[render->numlights].direction[2] = ((GzLight *)valueList[index])->direction[2];
			  //put light color
			  render->lights[render->numlights].color[0] = ((GzLight *)valueList[index])->color[0];
			  render->lights[render->numlights].color[1] = ((GzLight *)valueList[index])->color[1];
			  render->lights[render->numlights].color[2] = ((GzLight *)valueList[index])->color[2];
			 
			  render->numlights = render->numlights + 1;
			  break;
			} 
			
		case GZ_AMBIENT_LIGHT:
			{
				//put ambient light direction
				render->ambientlight.direction[0] = ((GzLight *)valueList[index])->direction[0];
				render->ambientlight.direction[1] = ((GzLight *)valueList[index])->direction[1];
				render->ambientlight.direction[2] = ((GzLight *)valueList[index])->direction[2];
				//put ambient light color
				render->ambientlight.color[0] = ((GzLight *)valueList[index])->color[0];
				render->ambientlight.color[1] = ((GzLight *)valueList[index])->color[1];
				render->ambientlight.color[2] = ((GzLight *)valueList[index])->color[2];
			    break;
			}

//*****************shading attribute*****************************
		case GZ_DIFFUSE_COEFFICIENT:
			
			{
				render->Kd[0] = ((float *)valueList[index])[0];
		     	render->Kd[1] = ((float *)valueList[index])[1];
		    	render->Kd[2] = ((float *)valueList[index])[2];
				break;
			}
		case GZ_INTERPOLATE:
			{
                render->interp_mode = *(int *)valueList[index];
				break;
			}
			
		case GZ_AMBIENT_COEFFICIENT:
			{
				render->Ka[0] = ((float *)valueList[index])[0];
				render->Ka[1] = ((float *)valueList[index])[1];
				render->Ka[2] = ((float *)valueList[index])[2];
				break;
			}
		case GZ_SPECULAR_COEFFICIENT:
			{
				render->Ks[0] = ((float *)valueList[index])[0];
				render->Ks[1] = ((float *)valueList[index])[1];
				render->Ks[2] = ((float *)valueList[index])[2];
               break;
			}
		case GZ_DISTRIBUTION_COEFFICIENT:
			{
                 render->spec = *((float *)valueList[index]);
				break;
			}

// hw5
		case GZ_TEXTURE_MAP:
			{
			    render->tex_fun = (GzTexture)valueList[index];
				break;
			}


			
		}
    }


	//FLOAT * pColor = (float *)*valueList;
	//render->flatcolor[0]= pColor[0];
	//render->flatcolor[1]= pColor[1];
	//render->flatcolor[2]= pColor[2];

	return GZ_SUCCESS;
}

int GzPutTriangle(GzRender	*render, int numParts, GzToken *nameList, 
				  GzPointer	*valueList)
/* numParts : how many names and values */
{
/*  
- pass in a triangle description with tokens and values corresponding to 
      GZ_POSITION:3 vert positions in model space 
- Xform positions of verts  
- Clip - just discard any triangle with verts behind view plane 
       - test for triangles with all three verts off-screen 
- invoke triangle rasterizer  
*/ 
   	GzCoord V1, V2, V3; //Screen Space Coordinates
	GzCoord V1Img,V2Img,V3Img;//Image Space Coordinates
	GzCoord Norm1, Norm2, Norm3; // Vertex Normal
	GzTextureIndex uv1,uv2,uv3;//vertex u,v value in image space
    GzTextureIndex Pixeluv;//added in hw5
    GzColor tex_color;
	for (int index=0;index<numParts;index++)
   {
	   switch (nameList[index])
	   {
	   case GZ_POSITION:
		   {
			   GzCoord * Vertex = (GzCoord *)valueList[index];	 
			V1Img[0] =  V1[0] = Vertex[0][0];
			V1Img[1] =  V1[1] = Vertex[0][1];
			V1Img[2] =  V1[2] = Vertex[0][2];
			V2Img[0] =  V2[0] = Vertex[1][0];
			V2Img[1] =  V2[1] = Vertex[1][1];
			V2Img[2] =  V2[2] = Vertex[1][2];
			V3Img[0] =  V3[0]= Vertex[2][0];
			V3Img[1] =  V3[1]= Vertex[2][1];
			V3Img[2] =  V3[2]= Vertex[2][2];
			   break;
		   }
	   case GZ_NORMAL:
		   {
			   GzCoord * Norm = (GzCoord *)valueList[index];	 
			   Norm1[0] = Norm[0][0];
			   Norm1[1] = Norm[0][1];
			   Norm1[2] = Norm[0][2];
			   Norm2[0] = Norm[1][0];
			   Norm2[1] = Norm[1][1];
			   Norm2[2] = Norm[1][2];
			   Norm3[0]= Norm[2][0];
			   Norm3[1]= Norm[2][1];
			   Norm3[2]= Norm[2][2];
               break;
		   }	

//hw5 step 1: read u,v
	   case GZ_TEXTURE_INDEX:
		   {
			   GzTextureIndex * uvvalue = (GzTextureIndex *)valueList[index];
               uv1[0] = uvvalue[0][0];
			   uv1[1] = uvvalue[0][1];
			   uv2[0] = uvvalue[1][0];
			   uv2[1] = uvvalue[1][1];
			   uv3[0] = uvvalue[2][0];
			   uv3[1] = uvvalue[2][1];
     		   break;
		   }
  	   }
   }



	Transform(V1,V2,V3,render);  //get screen space vertex coordinates by transformation
 	VertexSorting(V1,V2,V3,Norm1,Norm2,Norm3,uv1,uv2,uv3);
    


	float k;
    GzCoord ABC;
	PlaneEquation(V1,V2,V3,ABC);//plane equation coefficients
    float D = -ABC[0]*V1[0]-ABC[1]*V1[1]-ABC[2]*V1[2];

   float screenspaceZ[3];
   	screenspaceZ[0] = interpoloateZ(V1[0],V1[1],ABC,D);
     screenspaceZ[1] = interpoloateZ(V2[0],V2[1],ABC,D);
	 screenspaceZ[2] = interpoloateZ(V3[0],V3[1],ABC,D);

    


	float minX,minY,maxX, maxY;
    getboundingbox(&minX,&minY,&maxX,&maxY,V1,V2,V3);


	//hw 5 step 2: transform vertex u, v to screen space



	TransuvTopersp(screenspaceZ[0],uv1);
	TransuvTopersp(screenspaceZ[1],uv2);
	TransuvTopersp(screenspaceZ[2],uv3);
	
	
	//hw 5 step 2 over





//When Gouraud Interpolation
if (render->interp_mode == GZ_COLOR)
{
	GzColor V1Color, V2Color, V3Color, VColor;
	
	//step 1: transform vertex normal to image space
  TransformNorm(Norm1,Norm2,Norm3,render);
	//step 2: Calculate the color of three vertices. 
  ModifiedCalculateColor(Norm1,render,V1Color);
  ModifiedCalculateColor(Norm2,render,V2Color);
  ModifiedCalculateColor(Norm3,render,V3Color);
  GzColor KT;
  //step 3: query each pixel. If in the triangle, do color interpolation.
  for (int i= (int)(minX-1); i<ceil(maxX);i++)
  {
	  for (int j=(int)(minY-1);j<ceil(maxY);j++)
	  {
		  if (INTri(i,j,V1,V2,V3))
		  {
 			  //interpolate Z
			  k = (int)interpoloateZ(i,j,ABC,D);

			  //**********************hw 5********************************************

			  //hw 5 step 3: interpolate u,v at this pixel
			  InterpolateUVatPixel(V1,V2,V3,uv1,uv2,uv3,i,j,Pixeluv);
			  // hw 5 step 4: transform u, v back to image space
			  TransUVTouv(k, Pixeluv);
			  //hw 5 step 5: use tex_fun to calculate color
		
			  render->tex_fun(Pixeluv[0],Pixeluv[1],tex_color);
			  //hw5 step 6: use texture color as Kd and Ka, keep Ks as set by application
			 
			  KT[0] = tex_color[0];
			  KT[1] = tex_color[1];
			  KT[2] = tex_color[2];

			  //**********************************************************************

			  //compare Z buffer to remove hidden surfaces and set pixel color

			  if (k < render->display->fbuf[i + j * render->display->xres].z)
					{
                     //   Interpolation(VColor,i,j,V1,V2,V3,V1Color,V2Color,V3Color);
					// Do linear color interpolation
                        GzCoord Inter1, Inter2, Inter3;
						GzCoord ColorPlaneABC;
						Inter1[X] = V1[X];
						Inter1[Y] = V1[Y];
						Inter2[X] = V2[X];
						Inter2[Y] = V2[Y];
						Inter3[X] = V3[X];
						Inter3[Y] = V3[Y];
		
                        for (int chan=0;chan<3;chan++)
                        {
							Inter1[Z] = V1Color[chan];
							Inter2[Z] = V2Color[chan];
							Inter3[Z] = V3Color[chan];
							PlaneEquation(Inter1,Inter2,Inter3,ColorPlaneABC);//plane equation coefficients
							float ColorPlaneD = -ColorPlaneABC[0]*Inter1[0]-ColorPlaneABC[1]*Inter1[1]-ColorPlaneABC[2]*Inter1[2];
							VColor[chan] =  interpoloateZ(i,j,ColorPlaneABC,ColorPlaneD);

                        }
						Innerdotproduct(KT,VColor,VColor);
                        
						render->display->fbuf[i + j * render->display->xres].red =ctoi(VColor[RED]);
						render->display->fbuf[i + j * render->display->xres].green=ctoi(VColor[GREEN]);
						render->display->fbuf[i + j * render->display->xres].blue =ctoi(VColor[BLUE]);
						render->display->fbuf[i + j * render->display->xres].z = k;
			  } 

		  } 
	  }	  
  }	

	return GZ_SUCCESS;
}
//When Phong Interpolation
else if (render->interp_mode == GZ_NORMALS)
{
	GzCoord PixelNorm;
	GzColor PixelColor;
	//step 1: transform vertex normals to image space
     TransformNorm(Norm1,Norm2,Norm3,render);
   
	//step 2: for each pixel in the bounding box, interpolate Normal
	 
	 for (int i= (int)(minX-1); i<ceil(maxX);i++)
	 {
		 for (int j=(int)(minY-1);j<ceil(maxY);j++)
		 {
			 if (INTri(i,j,V1,V2,V3))
			 {
		        Interpolation(PixelNorm,i,j,V1,V2,V3,Norm1,Norm2,Norm3);
				//interpolate Z
				k = interpoloateZ((float)i,(float)j,ABC,D);
		      
		        Normalized(PixelNorm);

//**********************hw 5********************************************

                //hw 5 step 3: interpolate u,v at this pixel
                       

                InterpolateUVatPixel(V1,V2,V3,uv1,uv2,uv3,i,j,Pixeluv);

				// hw 5 step 4: transform u, v back to image space
                TransUVTouv(k, Pixeluv);
				//hw 5 step 5: use tex_fun to calculate color
				
                render->tex_fun(Pixeluv[0],Pixeluv[1],tex_color);
				//hw5 step 6: use texture color as Kd and Ka, keep Ks as set by application
                render->Ka[0] = render->Kd[0] = tex_color[0];
				render->Ka[1] = render->Kd[1] = tex_color[1];
				render->Ka[2] = render->Kd[2] = tex_color[2];

//**********************************************************************
    //step 3: calculate color	
				CalculateColor(PixelNorm,render,PixelColor);

				//compare Z buffer to remove hidden surfaces and set pixel color

				if (k < render->display->fbuf[i + j * render->display->xres].z)
				{
					render->display->fbuf[i + j * render->display->xres].red =ctoi(PixelColor[RED]);
					render->display->fbuf[i + j * render->display->xres].green=ctoi(PixelColor[GREEN]);
					render->display->fbuf[i + j * render->display->xres].blue =ctoi(PixelColor[BLUE]);
					render->display->fbuf[i + j * render->display->xres].z = k;
				}
			 }
		 }
	 }	



	return GZ_SUCCESS;
}
else  //flat shading
{
	GzCoord SurfNorm;
	GzColor SurfColor;
   //step 1: get surface normal
     GettriangleNormal(V1,V2,V3,SurfNorm);
   //step 2: calculate surface color
     CalculateColor(SurfNorm, render,SurfColor);
	//step 3: each pixel in the triangle has this color
	 for (int i= (int)(minX-1); i<ceil(maxX);i++)
	 {
	 	for (int j=(int)(minY-1);j<ceil(maxY);j++)
	 	{
	 		if (INTri(i,j,V1,V2,V3))
	 		{
	 			//interpolate Z
	 			k = (int)interpoloateZ(i,j,ABC,D);
	 			
	 			//compare Z buffer to remove hidden surfaces and set pixel color
	 	
	 			if (k < render->display->fbuf[i + j * render->display->xres].z	)
	 			{
	 				render->display->fbuf[i + j * render->display->xres].red =ctoi(SurfColor[0]);
	 				render->display->fbuf[i + j * render->display->xres].green=ctoi(SurfColor[1]);
	 				render->display->fbuf[i + j * render->display->xres].blue =ctoi(SurfColor[2]);
	 				render->display->fbuf[i + j * render->display->xres].z = k;
	 			} 

	 		} 
	 	}	  
	 }

	 return GZ_SUCCESS;
}
	


}

/* NOT part of API - just for general assistance */

short	ctoi(float color)		/* convert float color to GzIntensity short */
{
  return(short)((int)(color * ((1 << 12) - 1)));
}



void Transform(GzCoord v1, GzCoord v2, GzCoord v3, GzRender * render)
{
	dotproductv(render->Ximage[render->matlevel],v1);
    dotproductv(render->Ximage[render->matlevel],v2);
    dotproductv(render->Ximage[render->matlevel],v3);
}

void dotproductv(GzMatrix M, GzCoord v)
{
	float v_h[4],vh4;
	v_h[0] = v[0];
	v_h[1] = v[1];
	v_h[2] = v[2];
	v_h[3] = 1;

	v[0] = M[0][0] * v_h[0] + M[0][1] * v_h[1] + M[0][2] * v_h[2] + M[0][3] * v_h[3];
	v[1] = M[1][0] * v_h[0] + M[1][1] * v_h[1] + M[1][2] * v_h[2] + M[1][3] * v_h[3];
	v[2] = M[2][0] * v_h[0] + M[2][1] * v_h[1] + M[2][2] * v_h[2] + M[2][3] * v_h[3];
	vh4 =  M[3][0] * v_h[0] + M[3][1] * v_h[1] + M[3][2] * v_h[2] + M[3][3] * v_h[3];

	v[0] = v[0]/vh4;
	v[1] = v[1]/vh4;
	v[2] = v[2]/vh4;



}


void numDotVector(float num, GzCoord v1, GzCoord v)
{
	v[0] = num * v1[0];
	v[1] = num * v1[1];
	v[2] = num * v1[2];
}



float dotproduct(GzCoord u, GzCoord v)
{
	float dotp;
	dotp = u[0] * v[0]+ u[1] * v[1]+u[2] * v[2];
	return dotp;
}

void CrossProduct(GzCoord u, GzCoord v, GzCoord w)
{
	w[0] = u[1]* v[2] - u[2]*v[1];
	w[1] = u[2] * v[0] - u[0] * v[2];
	w[2] = u[0] * v[1] - u[1] * v[0];

}

void Normalized(GzCoord u)
{
	float length;
	length = sqrt(u[0] * u[0] + u[1] * u[1] + u[2] * u[2]);
	u[0] = u[0]/length;
	u[1] = u[1]/length;
	u[2] = u[2]/length;
}

void MatrixMultiply(GzMatrix A, GzMatrix B, GzMatrix C)
{
	//row 1
	C[0][0] = A[0][0] * B[0][0] +  A[0][1] * B[1][0] +  A[0][2] * B[2][0] + A[0][3] * B[3][0] ;
	C[0][1] = A[0][0] * B[0][1] +  A[0][1] * B[1][1] +  A[0][2] * B[2][1] + A[0][3] * B[3][1] ;
	C[0][2] = A[0][0] * B[0][2] +  A[0][1] * B[1][2] +  A[0][2] * B[2][2] + A[0][3] * B[3][2] ;
	C[0][3] = A[0][0] * B[0][3] +  A[0][1] * B[1][3] +  A[0][2] * B[2][3] + A[0][3] * B[3][3] ;
	//row 2
	C[1][0] = A[1][0] * B[0][0] +  A[1][1] * B[1][0] +  A[1][2] * B[2][0] + A[1][3] * B[3][0] ;
	C[1][1] = A[1][0] * B[0][1] +  A[1][1] * B[1][1] +  A[1][2] * B[2][1] + A[1][3] * B[3][1] ;
	C[1][2] = A[1][0] * B[0][2] +  A[1][1] * B[1][2] +  A[1][2] * B[2][2] + A[1][3] * B[3][2] ;
	C[1][3] = A[1][0] * B[0][3] +  A[1][1] * B[1][3] +  A[1][2] * B[2][3] + A[1][3] * B[3][3] ;
	//row 3
	C[2][0] = A[2][0] * B[0][0] +  A[2][1] * B[1][0] +  A[2][2] * B[2][0] + A[2][3] * B[3][0] ;
	C[2][1] = A[2][0] * B[0][1] +  A[2][1] * B[1][1] +  A[2][2] * B[2][1] + A[2][3] * B[3][1] ;
	C[2][2] = A[2][0] * B[0][2] +  A[2][1] * B[1][2] +  A[2][2] * B[2][2] + A[2][3] * B[3][2] ;
	C[2][3] = A[2][0] * B[0][3] +  A[2][1] * B[1][3] +  A[2][2] * B[2][3] + A[2][3] * B[3][3] ;
	//row 4
	C[3][0] = A[3][0] * B[0][0] +  A[3][1] * B[1][0] +  A[3][2] * B[2][0] + A[3][3] * B[3][0] ;
	C[3][1] = A[3][0] * B[0][1] +  A[3][1] * B[1][1] +  A[3][2] * B[2][1] + A[3][3] * B[3][1] ;
	C[3][2] = A[3][0] * B[0][2] +  A[3][1] * B[1][2] +  A[3][2] * B[2][2] + A[3][3] * B[3][2] ;
	C[3][3] = A[3][0] * B[0][3] +  A[3][1] * B[1][3] +  A[3][2] * B[2][3] + A[3][3] * B[3][3] ;

}

void SetIdentity(GzMatrix M)
{
	memset(M, 0, sizeof(GzMatrix));
	M[0][0]=1;
	M[1][1]=1;
	M[2][2]=1;
	M[3][3]=1;
}

void vectorMinus(GzCoord v1, GzCoord v2, GzCoord v)
{
	v[0] = v1[0] - v2[0];
	v[1] = v1[1] - v2[1];
	v[2] = v1[2] - v2[2];

}

void vectorAdd(GzCoord v1, GzCoord v2, GzCoord v)
{
	v[0] = v1[0] + v2[0];
	v[1] = v1[1] + v2[1];
	v[2] = v1[2] + v2[2];
}

//in the triangle, render the pixel; on the left edge, render the pixel
//BOOL INTri(int x, int y, GzPointer pv1, GzPointer pv2, GzPointer pv3)
BOOL INTri(int x, int y, GzCoord v1, GzCoord v2, GzCoord v3)
{
	
	//for edge v1v2
	float dX, dY;
	dX = v2[0] - v1[0];
	dY = v2[1] - v1[1];
	float E1 = dY * x - dX * y + (dX * v1[1] - dY*v1[0]);
	//for edge v2v3
	dX = v3[0] - v2[0];
	dY = v3[1] - v2[1];
	float E2 = dY * x - dX * y + (dX * v2[1] - dY*v2[0]);
	//for edge v3v1
	dX = v1[0] - v3[0];
	dY = v1[1] - v3[1];
	float E3 = dY * x - dX * y + (dX * v3[1] - dY*v3[0]);
	//pixel in the tri if E1*E2 > 0 AND E2*E3>0 AND E3*E1>0
	if (E1*E2>0 && E2*E3>0 && E3*E1>0)
	{
		return true; //in the tri
	} 
	else if (E1==0&&E2*E3>0)  //compare  x of v3 with the intersect point on v1-v2
	{
		if (v3[0]>IntersectPoint(v3,v1,v2))
		{
			return true;
		}
		else
		{
			return false;
		}  
	}
	else if (E2==0&&E1*E3>0)   //compare  x of v1 with the intersect point on v2-v3
	{
		if (v1[0]>IntersectPoint(v1,v2,v3))
		{
			return true;
		}
		else
		{
			return false;
		}  
	}
	else if (E3==0&&E1*E2>0)   //compare  x of v2 with the intersect point on v3-v1
	{
		if (v2[0]>IntersectPoint(v2,v3,v1))
		{
			return true;
		}
		else
		{
			return false;
		}  
	}
	else if (x==v1[0]&&y==v1[1]||x==v2[0]&&y==v2[1]||x==v3[0]&&y==v3[1])
	{
		return true;
	}
	return false;


}

float interpoloateZ(float x, float y, GzCoord abc, float d)
{
	float z;
	z= (-abc[0]*x -abc[1]*y -d)/abc[2];
	
	return z;
}


void PlaneEquation(GzCoord v1, GzCoord v2, GzCoord v3,GzCoord w)
{
	GzCoord u,v;

	u[0] = v2[0] -v1[0];
	u[1] = v2[1] -v1[1];
	u[2] = v2[2] -v1[2];
	v[0] = v3[0] -v2[0];
	v[1] = v3[1] -v2[1];
	v[2] = v3[2] -v2[2];

	CrossProduct(u,v,w);
}



void VertexSorting(GzCoord v1, GzCoord v2, GzCoord v3,GzCoord n1, GzCoord n2, GzCoord n3, GzTextureIndex uv1, GzTextureIndex uv2, GzTextureIndex uv3)
{
	GzCoord temp,tempN;
	GzTextureIndex tempUV;

	if (v1[1]>v2[1])
	{
		temp[0]=v1[0];
		temp[1]=v1[1];
		temp[2]=v1[2];
		tempN[0]=n1[0];
		tempN[1]=n1[1];
		tempN[2]=n1[2];
		tempUV[0]=uv1[0];
		tempUV[1]=uv1[1];
		v1[0]=v2[0];
		v1[1]=v2[1];
		v1[2]=v2[2];
		n1[0]=n2[0];
		n1[1]=n2[1];
		n1[2]=n2[2];
        uv1[0]=uv2[0];
		uv1[1]=uv2[1];
		v2[0]=temp[0];
		v2[1]=temp[1];
		v2[2]=temp[2];
		n2[0]=tempN[0];
		n2[1]=tempN[1];
		n2[2]=tempN[2];
		uv2[0]=tempUV[0];
		uv2[1]=tempUV[1];
	}
	if (v1[1]>v3[1])
	{
		temp[0]=v1[0];
		temp[1]=v1[1];
		temp[2]=v1[2];
		tempN[0]=n1[0];
		tempN[1]=n1[1];
		tempN[2]=n1[2];
		tempUV[0]=uv1[0];
		tempUV[1]=uv1[1];
		v1[0]=v3[0];
		v1[1]=v3[1];
		v1[2]=v3[2];
		n1[0]=n3[0];
		n1[1]=n3[1];
		n1[2]=n3[2];
		uv1[0]=uv3[0];
		uv1[1]=uv3[1];
		v3[0]=temp[0];
		v3[1]=temp[1];
		v3[2]=temp[2];
		n3[0]=tempN[0];
		n3[1]=tempN[1];
		n3[2]=tempN[2];
		uv3[0]=tempUV[0];
		uv3[1]=tempUV[1];
	}
	if (v2[1]>v3[1])
	{
		temp[0]=v2[0];
		temp[1]=v2[1];
		temp[2]=v2[2];
		tempN[0]=n2[0];
		tempN[1]=n2[1];
		tempN[2]=n2[2];
		tempUV[0]=uv2[0];
		tempUV[1]=uv2[1];
		v2[0]=v3[0];
		v2[1]=v3[1];
		v2[2]=v3[2];
		n2[0]=n3[0];
		n2[1]=n3[1];
		n2[2]=n3[2];
		uv2[0]=uv3[0];
		uv2[1]=uv3[1];
		v3[0]=temp[0];
		v3[1]=temp[1];
		v3[2]=temp[2];
		n3[0]=tempN[0];
		n3[1]=tempN[1];
		n3[2]=tempN[2];
		uv3[0]=tempUV[0];
		uv3[1]=tempUV[1];
	}



	if (v1[0]<IntersectPoint(v1,v2,v3))
	{
		temp[0]=v2[0];
		temp[1]=v2[1];
		temp[2]=v2[2];
		tempN[0]=n2[0];
		tempN[1]=n2[1];
		tempN[2]=n2[2];
		tempUV[0]=uv2[0];
		tempUV[1]=uv2[1];
		v2[0]=v3[0];
		v2[1]=v3[1];
		v2[2]=v3[2];
		n2[0]=n3[0];
		n2[1]=n3[1];
		n2[2]=n3[2];
		uv2[0]=uv3[0];
		uv2[1]=uv3[1];
		v3[0]=temp[0];
		v3[1]=temp[1];
		v3[2]=temp[2];
		n3[0]=tempN[0];
		n3[1]=tempN[1];
		n3[2]=tempN[2];
		uv3[0]=tempUV[0];
		uv3[1]=tempUV[1];
	} 

}

//the x coordinate of the point on edge v2 v3 with the same y coordinate as v1

float IntersectPoint(GzPointer pv1, GzPointer pv2, GzPointer pv3)
{

	GzCoord *init_v1 = (GzCoord *)pv1;
	GzCoord *init_v2 = (GzCoord *)pv2;
	GzCoord *init_v3 = (GzCoord *)pv3;

	GzCoord v1,v2,v3;
	v1[0] = (*init_v1)[0];
	v1[1] = (*init_v1)[1];
	v1[2] = (*init_v1)[2];
	v2[0] = (*init_v2)[0];
	v2[1] = (*init_v2)[1];
	v2[2] = (*init_v2)[2];
	v3[0] = (*init_v3)[0];
	v3[1] = (*init_v3)[1];
	v3[2] = (*init_v3)[2];

	float dX=v3[0]-v2[0];
	float dY=v3[1]-v2[1];

	float x_p;
	if (dY==0)
	{
		x_p = v2[0];
	} 
	else
	{
		x_p = ((dX*v1[1]+dY*v2[0]-dX*v2[1])/dY);
	}

	return x_p;
}

void getboundingbox(float *minX,float *minY,float *maxX,float *maxY,GzCoord V1,GzCoord V2,GzCoord V3)
{
	*minX = min(V1[0],V2[0]);
	*minX = min(*minX,V3[0]);
	*minY = min(V1[1],V2[1]);
	*minY = min(*minY,V3[1]);
	*maxX = max(V1[0],V2[0]);
	*maxX = max(*maxX,V3[0]);
	*maxY = max(V1[1],V2[1]);
	*maxY = max(*maxY,V3[1]);
	if (*minX<1)
	{
		*minX = 1;
	}
	if (*minY<1)
	{
		*minY = 1;
	}
	if (*maxX >256)
	{
		*maxX =256;
	}
	if (*maxY >256)
	{
		*maxY =256;
	}
}

//hw4
void ToUnitaryRotation(GzMatrix M)
{
 // First strip translations from the matrix
   M[0][3]=M[1][3]=M[2][3]=0;
// Compute a scale factor
   float K = 1/sqrt(M[0][0]*M[0][0]+M[0][1]*M[0][1]+M[0][2]*M[0][2]);
//Apply to all elements in the matrix
   for (int i=0;i<3;i++)
   {
	   for (int j=0;j<3;j++)
	   {
		   M[i][j] = M[i][j]*K;
	   }
   }
}


int GzPushMatrixToXnorm(GzRender *render, GzMatrix	matrix)
{
	/*
	- push a matrix onto the Xnorm stack
	- check for stack overflow
	*/
	GzMatrix MulMatrix;
	MatrixMultiply(render->Xnorm[render->matlevel - 3],matrix,MulMatrix);
	if (render->matlevel<MATLEVELS)
	{
		memcpy(render->Xnorm[render->matlevel - 2],MulMatrix,sizeof(GzMatrix));
		return GZ_SUCCESS;
	}
	else
	{
		return GZ_FAILURE;
	}


}


void TransformNorm(GzCoord n1, GzCoord n2, GzCoord n3, GzRender * render)
{
	dotproductv(render->Xnorm[render->matlevel - 2],n1);
	dotproductv(render->Xnorm[render->matlevel - 2],n2);
	dotproductv(render->Xnorm[render->matlevel - 2],n3);
}

void   CalculateColor(GzCoord N, GzRender * render, GzColor color)
{
	memset(color,0,sizeof(GzColor));
	GzColor LightSpecular,LightDiffuse,LightAmbient,TempSpecular,TempDiffuse;
	memset(LightSpecular,0,sizeof(GzColor));
	memset(LightDiffuse,0,sizeof(GzColor));
	memcpy(LightAmbient,render->ambientlight.color,sizeof(GzColor));
	memset(TempSpecular,0,sizeof(GzColor));
	memset(TempDiffuse,0,sizeof(GzColor));
	GzCoord Norm;


	GzCoord Eeye = {0,0,-1}; // vector Eeye is the eye direction. It is known in image space as (0,0,-1)
	float NdotL,NdotE; 
	NdotE = dotproduct(N,Eeye);
	for (int index=0; index<render->numlights;index++)
	{
		//Check the signs of N*L and N*E for each vertex
		//if both positive, compute lighting model
		// if both negative, flip normal and compute light model
		//if different sign, skip it
        NdotL = dotproduct(N, render->lights[index].direction);
		if (NdotE >0 && NdotL>0)
		{
            Norm[0] = N[0];
            Norm[1] = N[1];
		    Norm[2] = N[2];
		}
		else if (NdotE<0 && NdotL <0)
		{
			Norm[0] = -N[0];
			Norm[1] = -N[1];
			Norm[2] = -N[2];
		}
		else
		{
              continue;
		}
		
		//Compute lighting model
       GzCoord Reflection; //vector Reflection is reflection ray. R=2(N*L)N-L
       numDotVector(2*dotproduct(Norm, render->lights[index].direction),Norm,Reflection);	  
	   vectorMinus( Reflection,render->lights[index].direction,Reflection);
	   Normalized(Reflection);
	  
	   //clamping R*E to [0,1]
	   float RdotE = dotproduct(Reflection,Eeye);
	   if (RdotE<0)
	   {
		   RdotE = 0;
	   }
	   if (RdotE >1)
	   {
		   RdotE = 1;
	   }
	   numDotVector(pow(RdotE, render->spec), render->lights[index].color, TempSpecular);
       numDotVector(dotproduct(Norm,render->lights[index].direction),render->lights[index].color, TempDiffuse);
	   vectorAdd(LightSpecular, TempSpecular, LightSpecular);
	   vectorAdd(LightDiffuse, TempDiffuse, LightDiffuse);
	}
	Innerdotproduct(render->Ks,LightSpecular,LightSpecular);
	Innerdotproduct(render->Kd,LightDiffuse,LightDiffuse);
	Innerdotproduct(render->Ka,LightAmbient,LightAmbient);
	vectorAdd(LightSpecular,LightDiffuse,color);
	vectorAdd(color, LightAmbient, color);


	// check overflow or underflow
	if(color[RED]>1.0f)	color[RED]=1.0f;
	else if(color[RED]<0)	color[RED]=0;
	if(color[GREEN]>1.0f)	color[GREEN]=1.0f;
	else if(color[GREEN]<0)	color[GREEN]=0;
	if(color[BLUE]>1.0f)	color[BLUE]=1.0f;
	else if(color[BLUE]<0)	color[BLUE]=0;



}

void Innerdotproduct(GzCoord v1, GzCoord v2, GzCoord v3)
{
	 v3[0] = v1[0] * v2[0];
	 v3[1] = v1[1] * v2[1];
	 v3[2] = v1[2] * v2[2];
}

void Interpolation(GzCoord n, int i, int j, GzCoord v1, GzCoord v2, GzCoord v3, GzCoord n1, GzCoord n2, GzCoord n3)
{
      GzCoord target;
	  target[X] = i;
	  target[Y] = j;
	  target[Z] = 0;

     float x[3];  //used to find intersection on two edges
	 GzCoord normalinterp[3];//used to find intersection on two edges
	 int index=0;//used to find intersection on two edges


	 float x1, x2, x3;
     GzColor temp1, temp2,temp3;

	  x1 = IntersectPoint(target, v1, v2);
	  x2 = IntersectPoint(target, v2, v3);
	  x3 = IntersectPoint(target, v3, v1);

	  float f1, f2,f3;
	  if (x1<=max(v1[X],v2[X]) && x1>=min(v1[X],v2[X]))
	  {
		  f1 = sqrt(pow((x1 - v2[X]),2) + pow((target[Y]-v2[Y]),2))/sqrt(pow(v2[X]-v1[X],2)+pow(v2[Y]-v1[Y],2));
		  vectorMinus(n1, n2,temp1);
		  numDotVector(f1, temp1, temp1);
		  vectorAdd(temp1, n2, temp1);
          x[index] = x1;
		  normalinterp[index][RED] = temp1[RED];
		  normalinterp[index][GREEN] = temp1[GREEN];
		  normalinterp[index][BLUE] = temp1[BLUE];
		  index++;

	  }
	  else
		  f1 = 0;
	  
	  if (x2<=max(v2[X],v3[X]) && x2>=min(v2[X],v3[X]))
	  {
		 f2 = sqrt(pow((x2 - v3[X]),2) + pow((target[Y]-v3[Y]),2))/sqrt(pow(v3[X]-v2[X],2)+pow(v3[Y]-v2[Y],2));
		 vectorMinus(n2, n3, temp2);
		 numDotVector(f2, temp2, temp2);
		 vectorAdd(temp2, n3, temp2);
		 x[index] = x2;
		 normalinterp[index][RED] = temp2[RED];
		 normalinterp[index][GREEN] = temp2[GREEN];
		 normalinterp[index][BLUE] = temp2[BLUE];
		 index++;
	  }
	  else
		  f2 = 0;
	  
	  if (x3<=max(v1[X],v3[X]) && x3>=min(v1[X],v3[X]))
	  {
		  f3 = sqrt(pow((x3 - v1[X]),2) + pow((target[Y]-v1[Y]),2))/sqrt(pow(v1[X]-v3[X],2)+pow(v1[Y]-v3[Y],2));
		  vectorMinus(n3, n1, temp3);
		  numDotVector(f3, temp3, temp3);
		  vectorAdd(temp3, n1, temp3);
		  x[index] = x3;
		  normalinterp[index][RED] = temp3[RED];
		  normalinterp[index][GREEN] = temp3[GREEN];
		  normalinterp[index][BLUE] = temp3[BLUE];
		  index++;
	  }
	  else
		  f3 = 0;
	  
	  
	//interpolate between two normals
	 

     float f4 = abs((target[X] - x[0])/(x[1] - x[0]));
     //color = f4*(C[1]-C[0])+C[0]
	 vectorMinus(normalinterp[1],normalinterp[0], n);
	 numDotVector(f4, n, n);
	 vectorAdd(n, normalinterp[0], n);
}


void GettriangleNormal(GzCoord v1, GzCoord v2, GzCoord v3, GzCoord norm)
{
   GzCoord edge1, edge2;
   vectorMinus(v2,v1,edge1);
   vectorMinus(v3,v2,edge2);
   CrossProduct(edge1,edge2, norm);
   Normalized(norm);
}

//hw5

void TransuvTopersp(float Vzs, GzTextureIndex uv)
{
	float Vzprime;
	Vzprime = Vzs / (INT_MAX - Vzs);
	uv[0] = uv[0] / ( Vzprime + 1 );
	uv[1] = uv[1] / ( Vzprime + 1 );
}

void TransUVTouv(float Vzs, GzTextureIndex UV)
{
    float Vzprime;
    Vzprime = Vzs / (INT_MAX  - Vzs);
	UV[0] = UV[0] * ( Vzprime + 1 );
	UV[1] = UV[1] * ( Vzprime + 1 );
}

void InterpolateUVatPixel(GzCoord v1, GzCoord v2, GzCoord v3, GzTextureIndex uv1, GzTextureIndex uv2, GzTextureIndex uv3, int i, int j, GzTextureIndex Pixeluv)
{

	// Do linear uv interpolation
	GzCoord Inter1, Inter2, Inter3;
	GzCoord uvPlaneABC;
	float uvPlaneD;
	Inter1[X] = v1[X];
	Inter1[Y] = v1[Y];
	Inter2[X] = v2[X];
	Inter2[Y] = v2[Y];
	Inter3[X] = v3[X];
	Inter3[Y] = v3[Y];
 
	for (int chan=0;chan<2;chan++)
	{
		Inter1[Z] = uv1[chan];
		Inter2[Z] = uv2[chan];
		Inter3[Z] = uv3[chan];
		PlaneEquation(Inter1,Inter2,Inter3,uvPlaneABC);//plane equation coefficients
		uvPlaneD = -uvPlaneABC[0]*Inter1[0]-uvPlaneABC[1]*Inter1[1]-uvPlaneABC[2]*Inter1[2];
		Pixeluv[chan] =  interpoloateZ((float)i,(float)j,uvPlaneABC,uvPlaneD);
	}

}



void   ModifiedCalculateColor(GzCoord N, GzRender * render, GzColor color)
{
	memset(color,0,sizeof(GzColor));
	GzColor LightSpecular,LightDiffuse,LightAmbient,TempSpecular,TempDiffuse;
	memset(LightSpecular,0,sizeof(GzColor));
	memset(LightDiffuse,0,sizeof(GzColor));
	memcpy(LightAmbient,render->ambientlight.color,sizeof(GzColor));
	memset(TempSpecular,0,sizeof(GzColor));
	memset(TempDiffuse,0,sizeof(GzColor));
	GzCoord Norm;


	GzCoord Eeye = {0,0,-1}; // vector Eeye is the eye direction. It is known in image space as (0,0,-1)
	float NdotL,NdotE; 
	NdotE = dotproduct(N,Eeye);
	for (int index=0; index<render->numlights;index++)
	{
		//Check the signs of N*L and N*E for each vertex
		//if both positive, compute lighting model
		// if both negative, flip normal and compute light model
		//if different sign, skip it
		NdotL = dotproduct(N, render->lights[index].direction);
		if (NdotE >0 && NdotL>0)
		{
			Norm[0] = N[0];
			Norm[1] = N[1];
			Norm[2] = N[2];
		}
		else if (NdotE<0 && NdotL <0)
		{
			Norm[0] = -N[0];
			Norm[1] = -N[1];
			Norm[2] = -N[2];
		}
		else
		{
			continue;
		}

		//Compute lighting model
		GzCoord Reflection; //vector Reflection is reflection ray. R=2(N*L)N-L
		numDotVector(2*dotproduct(Norm, render->lights[index].direction),Norm,Reflection);	  
		vectorMinus( Reflection,render->lights[index].direction,Reflection);
		Normalized(Reflection);

		//clamping R*E to [0,1]
		float RdotE = dotproduct(Reflection,Eeye);
		if (RdotE<0)
		{
			RdotE = 0;
		}
		/*   if (RdotE >1)
		{
		RdotE = 1;
		}*/
		numDotVector(pow(RdotE, render->spec), render->lights[index].color, TempSpecular);
		numDotVector(dotproduct(Norm,render->lights[index].direction),render->lights[index].color, TempDiffuse);
		vectorAdd(LightSpecular, TempSpecular, LightSpecular);
		vectorAdd(LightDiffuse, TempDiffuse, LightDiffuse);
	}
	//Innerdotproduct(render->Ks,LightSpecular,LightSpecular);
	//Innerdotproduct(render->Kd,LightDiffuse,LightDiffuse);
	//Innerdotproduct(render->Ka,LightAmbient,LightAmbient);
	vectorAdd(LightSpecular,LightDiffuse,color);
	vectorAdd(color, LightAmbient, color);

	// check overflow or underflow
	if(color[RED]>1.0f)	color[RED]=1.0f;
	else if(color[RED]<0)	color[RED]=0;
	if(color[GREEN]>1.0f)	color[GREEN]=1.0f;
	else if(color[GREEN]<0)	color[GREEN]=0;
	if(color[BLUE]>1.0f)	color[BLUE]=1.0f;
	else if(color[BLUE]<0)	color[BLUE]=0;

}


