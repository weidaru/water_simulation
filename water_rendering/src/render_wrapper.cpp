#include "render_wrapper.h"

#include <stdio.h>
#include <time.h>

#include "rend.h"
#include "Model.h"
#include "ModelFactory.h"
#include "shaders.h"

//render using GzRender
GzRender *renderer;
GzDisplay *refraction_display;
GzDisplay *reflection_display;

//camera
GzCamera default_camera;

//model related
Model *teapot_model, *water_plane_model, *island_model, * mirror_island_model;
GzCoord teapot_scale, teapot_position, teapot_rotation;
GzCoord island_scale, island_position, island_rotation;

static inline char GzIntensityToChar(GzIntensity g) 
{
	int gi = (int)(g/4095.0f*255.0f);
	return gi>255 ? 255:gi;
}

static void flush_display(GzDisplay* display, BackBuffer* bf)
{
	BITMAP bitmap;
	GetObject(bf->buffer_bmp, sizeof(BITMAP), &bitmap);
	static char* buffer = new char[bitmap.bmWidthBytes*bitmap.bmHeight];

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

static int render_model(GzRender* in_renderer, Model* model)
{
	GzBeginRender(in_renderer);

	GzToken		nameListTriangle[4];		/* vertex attribute names */
	GzPointer	valueListTriangle[4]; 			/* vertex attribute pointers */
	nameListTriangle[0] = GZ_POSITION; 
	nameListTriangle[1] = GZ_NORMAL; 
	nameListTriangle[2] = GZ_TEXTURE_INDEX;  
	nameListTriangle[3] = GZ_RGB_COLOR;

	int triangle_size = model->GetTriangleCount();
	for(int i=0; i<triangle_size; i++) 
	{ 
		const Triangle& t = model->GetData(i);
		valueListTriangle[0] = (GzPointer)t.vertices; 
		valueListTriangle[1] = (GzPointer)t.normals; 
		valueListTriangle[2] = (GzPointer)t.uvs; 
		GzPutTriangle(in_renderer, 3, nameListTriangle, valueListTriangle); 
	}

	return GZ_SUCCESS;
}

static int render_island(GzRender* in_renderer)
{
	//use the same material for ambient, diffuse and specular
	GzColor material_color = {0.635f, 0.549f, 0.223f};
	for(int i=0; i<3; i++)
	{
		in_renderer->Ka[i] = material_color[i];
		in_renderer->Kd[i] = material_color[i];
		in_renderer->Ks[i] = material_color[i];
	}
	in_renderer->spec = 32;

	//set up transform

	GzMatrix m;
	GzTrxMat(island_position,m);
	GzPushMatrix(in_renderer, m);
	GzRotYMat(180.0f, m);
	GzPushMatrix(in_renderer, m);
	GzScaleMat(island_scale,m);
	GzPushMatrix(in_renderer, m);

	GzPutCamera(renderer, &default_camera);

	return render_model(in_renderer, island_model);
}


static int render_mirror_island(GzRender* in_renderer)
{
	//use the same material for ambient, diffuse and specular
	GzColor material_color = {0.635f, 0.549f, 0.223f};
	for(int i=0; i<3; i++)
	{
		in_renderer->Ka[i] = material_color[i];
		in_renderer->Kd[i] = material_color[i];
		in_renderer->Ks[i] = material_color[i];
	}
	in_renderer->spec = 32;

	//set up transform

	GzMatrix m;
	GzCoord mirror_island_position;
	mirror_island_position[X]=island_position[X];
	mirror_island_position[Y]=- island_position[Y];
	mirror_island_position[Z]=island_position[Z];

	GzTrxMat(mirror_island_position,m);
	GzPushMatrix(in_renderer, m);
	GzRotYMat(180.0f, m);
	GzPushMatrix(in_renderer, m);
	GzScaleMat(island_scale,m);
	GzPushMatrix(in_renderer, m);

	GzPutCamera(renderer, &default_camera);

	return render_model(in_renderer, mirror_island_model);
}




static int render_water_plane(GzRender* in_renderer)
{
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
	GzCoord scale = {1.6f, 1.0f, 1.0f};
	GzMatrix m;
	GzScaleMat(scale,m);
	GzPushMatrix(in_renderer, m);

	GzToken		nameListTriangle[4];		/* vertex attribute names */
	GzPointer	valueListTriangle[4]; 			/* vertex attribute pointers */
	nameListTriangle[0] = GZ_POSITION; 
	nameListTriangle[1] = GZ_NORMAL; 
	nameListTriangle[2] = GZ_TEXTURE_INDEX;  
	nameListTriangle[3] = GZ_RGB_COLOR;

	return render_model(in_renderer, water_plane_model);
}

int render(BackBuffer* bf)
{
	BitBlt(bf->back_dc, 0, 0, bf->width, bf->height, NULL, 0, 0, BLACKNESS );
	
	//Toggle to show wireframe
	//renderer->show_wireframe = true;

	//	Render to refraction texture
	GzInitDisplay(refraction_display);
	GzInitDisplay(renderer->display);
	renderer->v_shader = GouraudVertexShader;
	renderer->p_shader = GouraudRefractionPixelShader;
	island_scale[1] = 1.0f/1.33f;
	render_island(renderer);
	island_scale[1] = 1.0f;

	GzCopyDisplay(refraction_display, renderer->display);

	// Render to reflection texture
	GzInitDisplay(reflection_display);
	GzInitDisplay(renderer->display);
	renderer->v_shader = GouraudVertexShader;
	renderer->p_shader = GouraudReflectionPixelShader;
	render_mirror_island(renderer);
	GzCopyDisplay(reflection_display, renderer->display);
	
	GzInitDisplay(renderer->display);

	renderer->v_shader = GlobalReflectionVS;
	renderer->p_shader = GlobalReflectionPS;
	render_water_plane(renderer);

	//blend
	for(int i=0; i<refraction_display->xres; i++)
	{
		for(int j=0; j<refraction_display->yres; j++)
		{
			GzIntensity r,g,b,a;
			GzDepth z;
			GzGetDisplay(refraction_display, i, j, &r, &g, &b, &a, &z);

			GzIntensity r0,g0,b0,a0;
			GzDepth z0;
			GzGetDisplay(reflection_display, i, j, &r0, &g0, &b0, &a0, &z0);

			if(a > 0 || a0 > 0)
			{
				GzIntensity r1,g1,b1,a1;
				GzDepth z1;
				GzGetDisplay(renderer->display, i, j, &r1, &g1, &b1, &a1, &z1);
				//blend with 0.6, 0.4
				float blend_des = 0.5f, blend_src = 0.25f, blend_refl = 0.25f;
				r = r1*blend_des+ r*blend_src + r0*blend_refl;
				g = g1*blend_des + g*blend_src + g0*blend_refl;
				b = b1*blend_des + b*blend_src + b0*blend_refl;
				GzPutDisplay(renderer->display, i,j,r,g,b,a,z);
			}
		}
	}

	renderer->v_shader = GouraudVertexShader;
	renderer->p_shader = GouraudPixelShader;
	render_island(renderer);

	flush_display(renderer->display, bf);

	return 0;
}

int init_render(int x_res, int y_res)
{
	GzDisplay* display;
	GzNewDisplay(&display, 0, x_res, y_res);
	GzNewRender(&renderer, GZ_Z_BUFFER_RENDER, display);

	//init camera
	default_camera.position[X] =0.0f;      
	default_camera.position[Y] = 10.0f;
	default_camera.position[Z] = -17.3f;

	default_camera.lookat[X] = 0.0f;
	default_camera.lookat[Y] = 0.0f;
	default_camera.lookat[Z] = 0.0f;

	default_camera.worldup[X] = 0.0f;
	default_camera.worldup[Y] = 0.866f;
	default_camera.worldup[Z] = 0.5f;

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
	teapot_model = ModelFactory::CreateModel("POT4.ASC", "asc");
	teapot_scale[0] = 0.3f;
	teapot_scale[1] = 0.3f;
	teapot_scale[2] = 0.3f;
	teapot_position[0] = 0.0f;
	teapot_position[1] = -3.0f;
	teapot_position[2] = -1.0f;
	teapot_rotation[0] = 0.0f;
	teapot_rotation[1] = 0.0f;
	teapot_rotation[2] = 0.0f;

	water_plane_model = ModelFactory::CreateModel("simple_water_plane.obj", "obj");

	island_model = ModelFactory::CreateModel("island.obj", "obj");
	island_scale[0] = 0.8f;
	island_scale[1] = 1.0f;
	island_scale[2] = 0.8f;
	island_position[0] = 0.0f;
	island_position[1] = -1.0f;
	island_position[2] = 0.0f;
	island_rotation[0] = 0.0f;
	island_rotation[1] = 0.0f;
	island_rotation[2] = 0.0f;

	//Get the mirrored island
	mirror_island_model = ModelFactory::CreateModel("island.obj", "obj");
	for (int num=0; num<mirror_island_model->GetTriangleCount(); num++)
	{
		const_cast <Triangle &> (mirror_island_model->GetData(num)).vertices[0][Y] = - mirror_island_model->GetData(num).vertices[0][Y];
		const_cast <Triangle &> (mirror_island_model->GetData(num)).vertices[1][Y] = - mirror_island_model->GetData(num).vertices[1][Y];
		const_cast <Triangle &> (mirror_island_model->GetData(num)).vertices[2][Y] = - mirror_island_model->GetData(num).vertices[2][Y];

		const_cast <Triangle &> (mirror_island_model->GetData(num)).normals[0][Y] = - mirror_island_model->GetData(num).normals[0][Y];
		const_cast <Triangle &> (mirror_island_model->GetData(num)).normals[1][Y] = - mirror_island_model->GetData(num).normals[1][Y];
		const_cast <Triangle &> (mirror_island_model->GetData(num)).normals[2][Y] = - mirror_island_model->GetData(num).normals[2][Y];
	}

	//setup texture display
	GzNewDisplay(&refraction_display, GZ_Z_BUFFER_RENDER, x_res, y_res);
	GzNewDisplay(&reflection_display, GZ_Z_BUFFER_RENDER, x_res, y_res);

	return 0;
}

int release_render()
{
	GzFreeDisplay(renderer->display);
	GzFreeRender(renderer);
	return 0;
}