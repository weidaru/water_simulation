#include "render_wrapper.h"

#include <stdio.h>
#include <time.h>

#include "rend.h"
#include "model.h"

//render using GzRender
GzRender *bf_render;
Model model;

static char GzIntensityToChar(GzIntensity g) 
{
	int gi = (int)(g/4095.0f*255.0f);
	return gi>255 ? 255:gi;
}

static void flush_display(GzDisplay* display, BackBuffer* bf)
{
	BITMAP bitmap;
	GetObject(bf->buffer_bmp, sizeof(BITMAP), &bitmap);
	char* buffer = new char[bitmap.bmWidthBytes*bitmap.bmHeight];

	BITMAPINFO binfo;
	ZeroMemory(&binfo,sizeof(BITMAPINFO)); 
	binfo.bmiHeader.biBitCount=bitmap.bmBitsPixel;  
	binfo.bmiHeader.biCompression=0; 
	binfo.bmiHeader.biHeight=bitmap.bmHeight; 
	binfo.bmiHeader.biPlanes=1; 
	binfo.bmiHeader.biSizeImage=0; 
	binfo.bmiHeader.biSize=sizeof(BITMAPINFOHEADER); 
	binfo.bmiHeader.biWidth=bitmap.bmWidth;

	GetDIBits(bf->back_dc,bf->buffer_bmp, 0, bitmap.bmHeight, buffer, &binfo, DIB_RGB_COLORS);
	
	int stride = (binfo.bmiHeader.biWidth * (binfo.bmiHeader.biBitCount / 8) + 3) & ~3;
	for(int i=0; i<display->yres; i++)
	{
		int row_base = i*stride;
		for(int j = 0; j<display->xres; j++)
		{
			int col_base = j*(binfo.bmiHeader.biBitCount / 8);
			GzIntensity r,g,b,a;
			GzDepth z;
			GzGetDisplay(display, j, display->yres-1 - i, &r, &g, &b, &a, &z);
			buffer[row_base + col_base ] = GzIntensityToChar(b);
			buffer[row_base + col_base + 1] = GzIntensityToChar(g);
			buffer[row_base +col_base + 2] = GzIntensityToChar(r);
		}
	}

	SetDIBits(bf->back_dc, bf->buffer_bmp, 0, bitmap.bmHeight, buffer, &binfo, DIB_RGB_COLORS);
}

//TODO: Just for test, remove later
static void shade(GzCoord norm, GzCoord color)
{
	GzCoord	light;
	float		coef;

	light[0] = 0.707f;
	light[1] = 0.5f;
	light[2] = 0.5f;

	coef = light[0]*norm[0] + light[1]*norm[1] + light[2]*norm[2];
	if (coef < 0) 	coef *= -1;

	if (coef > 1.0)	coef = 1.0;
	color[0] = coef*0.95f;
	color[1] = coef*0.65f;
	color[2] = coef*0.88f;
}

static int render_to_bf(BackBuffer* bf)
{
	GzBeginRender(bf_render);

	GzToken		nameListTriangle[4];		/* vertex attribute names */
	GzPointer	valueListTriangle[4]; 			/* vertex attribute pointers */
	nameListTriangle[0] = GZ_POSITION; 
	nameListTriangle[1] = GZ_NORMAL; 
	nameListTriangle[2] = GZ_TEXTURE_INDEX;  
	nameListTriangle[3] = GZ_RGB_COLOR;
	
	const Model::TriangleVector& triangles = model.GetData();
	for(Model::TriangleVector::const_iterator it = triangles.begin(); it != triangles.end(); it++) 
	{ 	/* read in tri word */
	    /* 
	     * Set the value pointers to the first vertex of the 	
	     * triangle, then feed it to the renderer 
	     * NOTE: this sequence matches the nameList token sequence
	     */ 
	     valueListTriangle[0] = (GzPointer)(*it)->vertices; 
		 valueListTriangle[1] = (GzPointer)(*it)->normals; 
		 valueListTriangle[2] = (GzPointer)(*it)->uvs; 
		 GzColor	color; 
		 shade((*it)->normals[0], color);/* shade based on the norm of vert0 */
		 valueListTriangle[3] = (GzPointer)color; 
		 GzPutTriangle(bf_render, 4, nameListTriangle, valueListTriangle); 
	}

	flush_display(bf_render->display, bf);
	return GZ_SUCCESS;
}

int render(BackBuffer* bf)
{
	BitBlt(bf->back_dc, 0, 0, bf->width, bf->height, NULL, 0, 0, BLACKNESS );
	
	static float angle = 0;
	angle += 1.0f;
	GzMatrix rotation;
	GzRotYMat(angle, rotation);
	GzPushMatrix(bf_render, rotation);
	render_to_bf(bf);

	return 0;
}

int init_render(int x_res, int y_res)
{
	GzDisplay* display;
	GzNewDisplay(&display, 0, x_res, y_res);
	GzNewRender(&bf_render, GZ_Z_BUFFER_RENDER, display);

	//Init camera
	/*
	GzCamera camera;
	camera.position[X] = 0.0f;
	camera.position[Y] = 4.0f;
	camera.position[Z] = 4.0f;

	camera.lookat[X] = 0.0f;
	camera.lookat[Y] = 0.0f;
	camera.lookat[Z] = 0.0f;

	camera.worldup[X] = 0.0f;
	camera.worldup[Y] = 1.0f;
	camera.worldup[Z] = 0.0f;

	camera.FOV = 53.7; 
	*/
	GzCamera camera;
	camera.position[X] =-10;      
	camera.position[Y] = 5;
	camera.position[Z] = -10;

	camera.lookat[X] = 0;
	camera.lookat[Y] = 0;
	camera.lookat[Z] = 0;

	camera.worldup[X] = 0;
	camera.worldup[Y] = 1.0;
	camera.worldup[Z] = 0.0;

	camera.FOV = 35.0;              // degrees 

	GzPutCamera(bf_render, &camera);


	//read in model
	model.ReadMesh("POT4.ASC");

	return 0;
}

int release_render()
{
	GzFreeDisplay(bf_render->display);
	GzFreeRender(bf_render);
	return 0;
}