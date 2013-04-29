#include "render_wrapper.h"

#include <stdio.h>
#include <time.h>
#include <assert.h>

#include "rend.h"
#include "utilities.h"
#include "Model.h"
#include "ModelFactory.h"
#include "shaders.h"
#include "ImageManager.h"
#include "projected_grid.h"

//render using GzRender
GzRender *renderer;
GzDisplay *refraction_display;
GzDisplay *reflection_display;

//camera
GzCamera default_camera;
bool need_update;

//model related
Model *water_plane_model = NULL, *island_model = NULL, * mirror_island_model = NULL, *skybox_model = NULL;
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
	GzColor material_color = {0.32f, 0.549f, 0.12f};
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
	mirror_island_position[Y]= - island_position[Y];
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

static int render_skybox(GzRender* in_renderer)
{
	in_renderer->v_shader = SkyboxVS;
	in_renderer->p_shader = SkyboxPS;

	GzMatrix m;
	GzCoord scale = {5.0f, 5.0f, 5.0f};
	GzScaleMat(scale, m);
	GzPushMatrix(in_renderer, m);
	GzCoord translate = {0.0f, -2.0f, 0.0f};
	GzTrxMat(translate, m);
	GzPushMatrix(in_renderer, m);

	GzBeginRender(in_renderer);

	GzToken		nameListTriangle[4];		/* vertex attribute names */
	GzPointer	valueListTriangle[4]; 			/* vertex attribute pointers */
	nameListTriangle[0] = GZ_POSITION; 
	nameListTriangle[1] = GZ_NORMAL; 
	nameListTriangle[2] = GZ_TEXTURE_INDEX;  
	nameListTriangle[3] = GZ_RGB_COLOR;

	int triangle_size = skybox_model->GetTriangleCount();
	assert(triangle_size == 12);
	char tex_name[12][64]  = {"SkyBoxLeft", "SkyBoxLeft",
											"SkyBoxDown", "SkyBoxDown",
											"SkyBoxBack", "SkyBoxBack",
											"SkyBoxRight", "SkyBoxRight",
											"SkyBoxUp", "SkyBoxUp",
											"SkyBoxFront", "SkyBoxFront",};
	for(int i=0; i<triangle_size; i++) 
	{ 
		strcpy(in_renderer->tex_name[0], tex_name[i]);
		const Triangle& t = skybox_model->GetData(i);
		valueListTriangle[0] = (GzPointer)t.vertices; 
		valueListTriangle[1] = (GzPointer)t.normals; 
		valueListTriangle[2] = (GzPointer)t.uvs; 
		GzPutTriangle(in_renderer, 3, nameListTriangle, valueListTriangle); 
	}

	return GZ_SUCCESS;
}

static int render_water_plane(GzRender* in_renderer)
{
	//use the same material for ambient, diffuse and specular
	GzColor material_color = {0.04f, 0.45f, 0.6f};
	float coeffcient[3] = {0.8f, 0.8f, 0.5f};
	for(int i=0; i<3; i++)
	{
		in_renderer->Ka[i] = material_color[i]*coeffcient[0];
		in_renderer->Kd[i] = material_color[i]*coeffcient[1];
		in_renderer->Ks[i] = material_color[i]*coeffcient[2];
	}
	in_renderer->spec = 64;

	GzPutCamera(renderer, &default_camera);

	//setup transform
	GzCoord scale = {1.0f, 1.0f, 1.0f};
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
	if(!need_update)
		return 1;
	else
		need_update = false;

	BitBlt(bf->back_dc, 0, 0, bf->width, bf->height, NULL, 0, 0, BLACKNESS );
	
	//Toggle to show wireframe
	//renderer->show_wireframe = true;
	//	Render to refraction texture
	GzInitDisplay(refraction_display);
	GzInitDisplay(renderer->display);
	renderer->v_shader = RefractionVertexShader;
	renderer->p_shader = RefractionPixelShader;
	ImageManager::GetSingleton()->GetImage("IslandTexture", "island_tex.ppm");
	strcpy(renderer->tex_name[0], "IslandTexture");
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
	
	//Put everything together.
	GzInitDisplay(renderer->display);
	renderer->texture_display[0] = reflection_display;
	renderer->texture_display[1] = refraction_display;
	renderer->v_shader = FinalWaterVS;
	renderer->p_shader = FinalWaterPS;
	render_water_plane(renderer);

	renderer->v_shader = PhongTextureVS;
	renderer->p_shader = PhongTexturePS;
	ImageManager::GetSingleton()->GetImage("IslandTexture", "island_tex.ppm");
	strcpy(renderer->tex_name[0], "IslandTexture");
	renderer->Ks[0] = 0.1f;
	renderer->Ks[1] = 0.1f;
	renderer->Ks[2] = 0.1f;
	render_island(renderer);

	render_skybox(renderer);
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
	default_camera.position[Y] = 5.0f;
	default_camera.position[Z] = -17.3f;

	default_camera.lookat[X] = 0.0f;
	default_camera.lookat[Y] = 0.0f;
	default_camera.lookat[Z] = 0.0f;

	default_camera.worldup[X] = 0.0f;
	default_camera.worldup[Y] = 0.961f;
	default_camera.worldup[Z] = 0.278f;

	default_camera.FOV = 53.7;              // degrees 

	GzPutCamera(renderer, &default_camera);


	//setup light
	GzToken     nameListLights[10];
	GzPointer   valueListLights[10];
	GzLight ambient_light = {{0.0f, 0.0f, 0.0f}, {0.9f, 0.9f, 0.9f}};
	GzLight direction_light = { {-0.2f, 0.2f, 0.8f} , {0.9f, 0.9f, 0.9f}};
	Normalize(direction_light.direction);
	float smoothness = 128;
	nameListLights[0] = GZ_AMBIENT_LIGHT;
	valueListLights[0] = (GzPointer)&ambient_light;
	nameListLights[1] = GZ_DIRECTIONAL_LIGHT;
	valueListLights[1] = (GzPointer)&direction_light;
	nameListLights[2] = GZ_SPECULAR_COEFFICIENT;
	valueListLights[2] = (GzPointer)&smoothness;
	GzPutAttribute(renderer, 3, nameListLights, valueListLights);

	generate_water_mesh("water_plane.asc", renderer);
	water_plane_model = ModelFactory::CreateModel("water_plane.asc", "asc");

	island_model = ModelFactory::CreateModel("island.obj", "obj");
	island_scale[0] = 0.8f;
	island_scale[1] = 1.0f;
	island_scale[2] = 0.8f;
	island_position[0] = 0.0f;
	island_position[1] = 0.0f;
	island_position[2] = 0.0f;
	island_rotation[0] = 0.0f;
	island_rotation[1] = 0.0f;
	island_rotation[2] = 0.0f;

	//Get the mirrored island
	mirror_island_model = ModelFactory::CreateModel("island.obj", "obj");
	for (int num=0; num<mirror_island_model->GetTriangleCount(); num++)
	{
		const_cast <Triangle &> (mirror_island_model->GetData(num)).vertices[0][Y] =2 - mirror_island_model->GetData(num).vertices[0][Y];
		const_cast <Triangle &> (mirror_island_model->GetData(num)).vertices[1][Y] =2 - mirror_island_model->GetData(num).vertices[1][Y];
		const_cast <Triangle &> (mirror_island_model->GetData(num)).vertices[2][Y] =2 - mirror_island_model->GetData(num).vertices[2][Y];

		const_cast <Triangle &> (mirror_island_model->GetData(num)).normals[0][Y] = - mirror_island_model->GetData(num).normals[0][Y];
		const_cast <Triangle &> (mirror_island_model->GetData(num)).normals[1][Y] = - mirror_island_model->GetData(num).normals[1][Y];
		const_cast <Triangle &> (mirror_island_model->GetData(num)).normals[2][Y] = - mirror_island_model->GetData(num).normals[2][Y];
	}

	//setup skybox
	skybox_model = ModelFactory::CreateModel("skybox.asc", "asc");

	//setup texture display and textures
	ImageManager::GetSingleton()->GetImage("SkyBoxUp", "cloudy_noon_UP.ppm");
	ImageManager::GetSingleton()->GetImage("SkyBoxDown", "cloudy_noon_DN.ppm");
	ImageManager::GetSingleton()->GetImage("SkyBoxLeft", "cloudy_noon_LF.ppm");
	ImageManager::GetSingleton()->GetImage("SkyBoxRight", "cloudy_noon_RT.ppm");
	ImageManager::GetSingleton()->GetImage("SkyBoxFront", "cloudy_noon_FR.ppm");
	ImageManager::GetSingleton()->GetImage("SkyBoxBack", "cloudy_noon_BK.ppm");
	GzNewDisplay(&refraction_display, GZ_Z_BUFFER_RENDER, x_res, y_res);
	GzNewDisplay(&reflection_display, GZ_Z_BUFFER_RENDER, x_res, y_res);

	need_update = true;

	return 0;
}

int release_render()
{
	GzFreeDisplay(renderer->display);
	GzFreeRender(renderer);
	return 0;
}