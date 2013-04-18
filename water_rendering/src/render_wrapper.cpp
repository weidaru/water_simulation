#include "render_wrapper.h"

#include <stdio.h>
#include <time.h>

#include "rend.h"
#include "model.h"
#include "shaders.h"

//render using GzRender
GzRender *renderer;
GzDisplay *refraction_display;

//camera
GzCamera default_camera;

//model related
Model teapot_model, water_plane_model;
GzCoord teapot_scale;
GzCoord teapot_position;
GzCoord teapot_rotation;

static inline char GzIntensityToChar(GzIntensity g) 
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

static int render_teapot(GzRender* in_renderer)
{
	//setup shader and teapot material
	in_renderer->v_shader = GouraudVertexShader;
	in_renderer->p_shader = GouraudPixelShader;

	//use the same material for ambient, diffuse and specular
	GzColor material_color = {0.7f, 0.3f, 0.1f};
	for(int i=0; i<3; i++)
	{
		in_renderer->Ka[i] = material_color[i];
		in_renderer->Kd[i] = material_color[i];
		in_renderer->Ks[i] = material_color[i];
	}
	in_renderer->spec = 32;

	//setup transform
	GzMatrix m;
	GzScaleMat(teapot_scale, m);
	GzPushMatrix(in_renderer, m);
	GzTrxMat(teapot_position, m);
	GzPushMatrix(in_renderer, m);
	GzTrxMat(teapot_rotation, m);
	GzPushMatrix(in_renderer, m);

	GzPutCamera(renderer, &default_camera);

	GzBeginRender(in_renderer);

	GzToken		nameListTriangle[4];		/* vertex attribute names */
	GzPointer	valueListTriangle[4]; 			/* vertex attribute pointers */
	nameListTriangle[0] = GZ_POSITION; 
	nameListTriangle[1] = GZ_NORMAL; 
	nameListTriangle[2] = GZ_TEXTURE_INDEX;  
	nameListTriangle[3] = GZ_RGB_COLOR;
	
	const Model::TriangleVector& triangles = teapot_model.GetData();
	for(Model::TriangleVector::const_iterator it = triangles.begin(); it != triangles.end(); it++) 
	{ 	
	     valueListTriangle[0] = (GzPointer)(*it)->vertices; 
		 valueListTriangle[1] = (GzPointer)(*it)->normals; 
		 valueListTriangle[2] = (GzPointer)(*it)->uvs; 
		 GzPutTriangle(in_renderer, 3, nameListTriangle, valueListTriangle); 
	}

	return GZ_SUCCESS;
}

static int render_water_plane(GzRender* in_renderer)
{
	//setup shader and teapot material
	in_renderer->v_shader = PhongVertexShader;
	in_renderer->p_shader = PhongPixelShader;

	//use the same material for ambient, diffuse and specular
	GzColor material_color = {0.04f, 0.4f, 0.6f};
	for(int i=0; i<3; i++)
	{
		in_renderer->Ka[i] = material_color[i];
		in_renderer->Kd[i] = material_color[i];
		in_renderer->Ks[i] = material_color[i];
	}
	in_renderer->spec = 32;

	GzPutCamera(renderer, &default_camera);

	//setup transform
	GzCoord scale = {3.0f, 1.0f, 1.5f};
	GzMatrix m;
	GzScaleMat(scale,m);
	GzPushMatrix(in_renderer, m);

	GzBeginRender(in_renderer);

	GzToken		nameListTriangle[4];		/* vertex attribute names */
	GzPointer	valueListTriangle[4]; 			/* vertex attribute pointers */
	nameListTriangle[0] = GZ_POSITION; 
	nameListTriangle[1] = GZ_NORMAL; 
	nameListTriangle[2] = GZ_TEXTURE_INDEX;  
	nameListTriangle[3] = GZ_RGB_COLOR;

	const Model::TriangleVector& triangles = water_plane_model.GetData();
	for(Model::TriangleVector::const_iterator it = triangles.begin(); it != triangles.end(); it++) 
	{ 	
		valueListTriangle[0] = (GzPointer)(*it)->vertices; 
		valueListTriangle[1] = (GzPointer)(*it)->normals; 
		valueListTriangle[2] = (GzPointer)(*it)->uvs; 
		GzPutTriangle(in_renderer, 3, nameListTriangle, valueListTriangle); 
	}

	return GZ_SUCCESS;
}

static int render_teapot_refraction(GzRender* in_renderer)
{
	//setup shader and teapot material
	in_renderer->v_shader = GouraudVertexShader;
	in_renderer->p_shader = GouraudAlphaPixelShader;

	//use the same material for ambient, diffuse and specular
	GzColor material_color = {0.7f, 0.3f, 0.1f};
	for(int i=0; i<3; i++)
	{
		in_renderer->Ka[i] = material_color[i];
		in_renderer->Kd[i] = material_color[i];
		in_renderer->Ks[i] = material_color[i];
	}
	in_renderer->spec = 32;

	//setup transform
	GzMatrix m;
	GzScaleMat(teapot_scale, m);
	GzPushMatrix(in_renderer, m);
	GzCoord pos = {teapot_position[0], teapot_position[1]/1.33f, teapot_position[2]};
	GzTrxMat(pos, m);
	GzPushMatrix(in_renderer, m);
	GzTrxMat(teapot_rotation, m);
	GzPushMatrix(in_renderer, m);

	GzPutCamera(renderer, &default_camera);

	GzBeginRender(in_renderer);

	GzToken		nameListTriangle[4];		/* vertex attribute names */
	GzPointer	valueListTriangle[4]; 			/* vertex attribute pointers */
	nameListTriangle[0] = GZ_POSITION; 
	nameListTriangle[1] = GZ_NORMAL; 
	nameListTriangle[2] = GZ_TEXTURE_INDEX;  
	nameListTriangle[3] = GZ_RGB_COLOR;

	const Model::TriangleVector& triangles = teapot_model.GetData();
	for(Model::TriangleVector::const_iterator it = triangles.begin(); it != triangles.end(); it++) 
	{ 	
		valueListTriangle[0] = (GzPointer)(*it)->vertices; 
		valueListTriangle[1] = (GzPointer)(*it)->normals; 
		valueListTriangle[2] = (GzPointer)(*it)->uvs; 
		GzPutTriangle(in_renderer, 3, nameListTriangle, valueListTriangle); 
	}

	return GZ_SUCCESS;
}

int render(BackBuffer* bf)
{
	BitBlt(bf->back_dc, 0, 0, bf->width, bf->height, NULL, 0, 0, BLACKNESS );
	
	//Render teapot to refraction texture
	render_teapot_refraction(renderer);
	GzCopyDisplay(refraction_display, renderer->display);

	//Render the model itself.
	//render_teapot(renderer);			//skip rendering the teapot to save some time
	render_water_plane(renderer);

	//blend

	for(int i=0; i<refraction_display->xres; i++)
	{
		for(int j=0; j<refraction_display->yres; j++)
		{
			GzIntensity r,g,b,a;
			GzDepth z;
			GzGetDisplay(refraction_display, i, j, &r, &g, &b, &a, &z);
			if(a > 0)
			{
				GzIntensity r1,g1,b1,a1;
				GzDepth z1;
				GzGetDisplay(renderer->display, i, j, &r1, &g1, &b1, &a1, &z1);
				//blend with 0.6, 0.4
				float blend_des = 0.6f, blend_src = 0.4f;
				r = r1*blend_des+ r*blend_src;
				g = g1*blend_des + g*blend_src;
				b = b1*blend_des + b*blend_src;
				GzPutDisplay(renderer->display, i,j,r,g,b,a,z);
			}
		}
	}

	flush_display(renderer->display, bf);

	return 0;
}

int init_render(int x_res, int y_res)
{
	GzDisplay* display;
	GzNewDisplay(&display, 0, x_res, y_res);
	GzNewRender(&renderer, GZ_Z_BUFFER_RENDER, display);

	//init camera
	default_camera.position[X] =0.0;      
	default_camera.position[Y] = 10.0;
	default_camera.position[Z] = 10.0;

	default_camera.lookat[X] = 0.0;
	default_camera.lookat[Y] = 0.0;
	default_camera.lookat[Z] = 0.0;

	default_camera.worldup[X] = 0.0;
	default_camera.worldup[Y] = 1.0;
	default_camera.worldup[Z] = 0.0;

	default_camera.FOV = 53.7;              // degrees 

	GzPutCamera(renderer, &default_camera);


	//setup light
	GzToken     nameListLights[10];
	GzPointer   valueListLights[10];
	GzLight ambient_light = {{0.0f, 0.0f, 0.0f}, {0.9f, 0.9f, 0.9f}};
	GzLight direction_light = { {0.0f, -0.707f, -0.707f} , {0.9f, 0.9f, 0.9f}};
	nameListLights[0] = GZ_AMBIENT_LIGHT;
	valueListLights[0] = (GzPointer)&ambient_light;
	nameListLights[1] = GZ_DIRECTIONAL_LIGHT;
	valueListLights[1] = (GzPointer)&direction_light;
	GzPutAttribute(renderer, 2, nameListLights, valueListLights);

	//read in model
	teapot_model.ReadMesh("POT4.ASC");
	teapot_scale[0] = 0.3f;
	teapot_scale[1] = 0.3f;
	teapot_scale[2] = 0.3f;
	teapot_position[0] = 0.0f;
	teapot_position[1] = -3.0f;
	teapot_position[2] = -1.0f;
	teapot_rotation[0] = 0.0f;
	teapot_rotation[1] = 0.0f;
	teapot_rotation[2] = 0.0f;

	water_plane_model.ReadMesh("water_plane.asc");

	//setup texture display
	GzNewDisplay(&refraction_display, GZ_Z_BUFFER_RENDER, x_res, y_res);

	return 0;
}

int release_render()
{
	GzFreeDisplay(renderer->display);
	GzFreeRender(renderer);
	return 0;
}