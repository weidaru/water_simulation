/* CS580 Homework 3 */

#include	<stdio.h>
#include	"math.h"
#include	"Gz.h"
#include	"rend.h"
#include	"utilities.h"
#include "shaders.h"

#include <assert.h>

#define PI 3.141592653f
#define ZMAX 10000000.0f

struct PixelShaderInput;

static void ScanLineTriangleInterpolation(GzRender* render, const PixelShaderInput vs_output[3], PixelShaderInput* buffer, int* count);
static bool IsPointToLeftOfLine(int x, int y, GzCoord* v0, GzCoord* v1);
static bool DepthTest(GzDisplay* display, int x, int y, GzDepth z);
static void YBasedScanLine(GzRender* render, const PixelShaderInput p[2], PixelShaderInput* buffer, int* count);
static void DrawLine(GzRender* render,  int numLines, GzCoord* vertices, int* indices);


int GzRotXMat(float degree, GzMatrix mat)
{
// Create rotate matrix : rotate along x axis
// Pass back the matrix using mat value
	SetIdentity(mat);
	mat[1][1] = cos(degree*PI/180.0f);
	mat[1][2] = -sin(degree*PI/180.0f);
	mat[2][1] = sin(degree*PI/180.0f);
	mat[2][2] = cos(degree*PI/180.0f);
	return GZ_SUCCESS;
}


int GzRotYMat(float degree, GzMatrix mat)
{
// Create rotate matrix : rotate along y axis
// Pass back the matrix using mat value
	SetIdentity(mat);
	mat[0][0] = cos(degree*PI/180.0f);
	mat[0][2] = sin(degree*PI/180.0f);
	mat[2][0] = -sin(degree*PI/180.0f);
	mat[2][2] = cos(degree*PI/180.0f);
	return GZ_SUCCESS;
}


int GzRotZMat(float degree, GzMatrix mat)
{
// Create rotate matrix : rotate along z axis
// Pass back the matrix using mat value
	SetIdentity(mat);
	mat[0][0] = cos(degree*PI/180);
	mat[0][1] = -sin(degree*PI/180);
	mat[1][0] = sin(degree*PI/180);
	mat[1][1] = cos(degree*PI/180);
	return GZ_SUCCESS;
}


int GzTrxMat(GzCoord translate, GzMatrix mat)
{
// Create translation matrix
// Pass back the matrix using mat value
	SetIdentity(mat);
	mat[0][3] = translate[0];
	mat[1][3] = translate[1];
	mat[2][3] = translate[2];
	return GZ_SUCCESS;
}


int GzScaleMat(GzCoord scale, GzMatrix mat)
{
// Create scaling matrix
// Pass back the matrix using mat value
	SetIdentity(mat);
	mat[0][0] = scale[0];
	mat[1][1] = scale[1];
	mat[2][2] = scale[2];
	return GZ_SUCCESS;
}


//----------------------------------------------------------
// Begin main functions

int GzNewRender(GzRender **render, GzRenderClass renderClass, GzDisplay *display)
{
/* 
- malloc a renderer struct
- keep closed until BeginRender inits are done
- span interpolator needs pointer to display for pixel writes
- check for legal class GZ_Z_BUFFER_RENDER
*/
	if(renderClass != GZ_Z_BUFFER_RENDER)
		return GZ_FAILURE;
	(*render) = new GzRender;
	if(!*render)
		return GZ_FAILURE;
	(*render)->renderClass = renderClass;
	(*render)->display = display;
	(*render)->matlevel = -1;
	(*render)->v_shader = FlatVertexShader;
	(*render)->p_shader = FlatPixelShader;

	// TODO: Initial other member later
	(*render)->numlights = 0;

	return GZ_SUCCESS;
}


int GzFreeRender(GzRender *render)
{
/* 
-free all renderer resources
*/
	// TODO: Free other member if added later
	//Free light

	delete render;
	return GZ_SUCCESS;
}


int GzBeginRender(GzRender	*render)
{
/* 
- set up for start of each frame - init frame buffer
*/
	//Set up matrix**************************************************************
	//Assume that after the first GzPutTriangle is called, no more transformation will be taken.
	//If some are taken, the object to world matrix will be reset.
	//Setup transformation matrix
	GzMatrix& Xwm = render->Xwm;	
	SetIdentity(Xwm);
	while(render->matlevel != -1)
	{
		GzMatrix temp;
		MatrixCopy(Xwm, temp);
		MatrixMultiply(temp, render->Ximage[render->matlevel], Xwm);
		GzPopMatrix(render);
	}

	GzMatrix& Xsp = render->Xsp;
	SetIdentity(Xsp);
	Xsp[0][0] = render->display->xres/2.0f;
	Xsp[0][3] = render->display->xres/2.0f;
	Xsp[1][1] = render->display->yres/-2.0f;
	Xsp[1][3] = render->display->yres/2.0f;
	Xsp[2][2] = ZMAX * tan(render->camera.FOV/(2.0f*180.0f)*3.1415962f);

	GzMatrix& Xsm = render->Xsm;
	SetIdentity(Xsm);
	GzMatrix temp;
	MatrixCopy(Xsm, temp);
	MatrixMultiply(temp, render->Xsp, Xsm);
	MatrixCopy(Xsm, temp);
	MatrixMultiply(temp, render->camera.Xpi, Xsm);
	MatrixCopy(Xsm, temp);
	MatrixMultiply(temp, render->camera.Xiw, Xsm);
	MatrixCopy(Xsm, temp);
	MatrixMultiply(temp, Xwm, Xsm);

	MatrixInverse(Xsm, render->Xsm_inverse);

	MatrixMultiply(render->camera.Xiw, render->Xwm, render->Xn);
	MatrixInverse(render->Xn, render->Xn);
	MatrixTranspose(render->Xn, render->Xn);
	MatrixInverse(render->Xn, render->Xn_inverse);
	return GzInitDisplay(render->display);
}

int GzPutAttribute(GzRender	*render, int numAttributes, GzToken	*nameList, 
	GzPointer *valueList) /* void** valuelist */
{
/*
- set renderer attribute states (e.g.: GZ_RGB_COLOR default color)
- TODO : later set shaders, interpolaters, texture maps, and lights
*/
	for(int i=0; i<numAttributes; i++)
	{
		switch(nameList[i])
		{
		case GZ_DIRECTIONAL_LIGHT:
			do 
			{
				GzLight* new_light = ((GzLight*)valueList[i]);
				render->lights[render->numlights].direction[0] = new_light->direction[0];
				render->lights[render->numlights].direction[1] = new_light->direction[1];
				render->lights[render->numlights].direction[2] = new_light->direction[2];
				render->lights[render->numlights].color[0] = new_light->color[0];
				render->lights[render->numlights].color[1] = new_light->color[1];
				render->lights[render->numlights].color[2] = new_light->color[2];
				render->numlights++;
			} while (false);
			break;
		case GZ_AMBIENT_LIGHT:
			do 
			{
				GzLight* new_light = ((GzLight*)valueList[i]);
				render->ambientlight.color[0] = new_light->color[0];
				render->ambientlight.color[1] = new_light->color[1];
				render->ambientlight.color[2] = new_light->color[2];
			} while (false);
			break;
		case GZ_DIFFUSE_COEFFICIENT:
			render->Kd[0] = ((float*)valueList[i])[0];
			render->Kd[1] = ((float*)valueList[i])[1];
			render->Kd[2] = ((float*)valueList[i])[2];
			break;
		case GZ_INTERPOLATE:
			render->interp_mode  = *((int*)valueList[i]);
			do 
			{
				switch(render->interp_mode)
				{
				case GZ_COLOR:
					render->v_shader = GouraudVertexShader;
					render->p_shader = GouraudPixelShader;
					break;
				case GZ_NORMALS:
					render->v_shader = PhongVertexShader;
					render->p_shader = PhongPixelShader;
					break;
				case GZ_FLAT:
					render->v_shader = FlatVertexShader;
					render->p_shader = FlatPixelShader;
					break;
				default:
					break;
				}
			} while (false);
			break;
		case GZ_AMBIENT_COEFFICIENT:
			render->Ka[0] = ((float*)valueList[i])[0];
			render->Ka[1] = ((float*)valueList[i])[1];
			render->Ka[2] = ((float*)valueList[i])[2];
			break;
		case GZ_SPECULAR_COEFFICIENT:
			render->Ks[0] = ((float*)valueList[i])[0];
			render->Ks[1] = ((float*)valueList[i])[1];
			render->Ks[2] = ((float*)valueList[i])[2];
			break;
		case GZ_DISTRIBUTION_COEFFICIENT:
			render->spec = *((float*)valueList[i]);
			break;
		case GZ_TEXTURE_MAP:
			render->tex_fun = (GzTexture)valueList[i];
		default:
			break;
		}
	}
	return GZ_SUCCESS;
}

int GzPutTriangle(GzRender *render, int	numParts, GzToken *nameList,
	GzPointer *valueList) 
/* numParts - how many names and values */
{


	//Vertex Shader**************************************************************
	PixelShaderInput vs_output[3];
	render->v_shader(render, numParts, nameList, valueList, vs_output);

	//Triangle Interpolation**************************************************************
	static PixelShaderInput* buffer =new PixelShaderInput[render->display->xres*render->display->yres];
	int count = 0;
	ScanLineTriangleInterpolation(render, vs_output, buffer, &count);

	//Pixel Shader**************************************************************
	for(int i = 0; i < count; i++)
	{
		PixelShaderInput& input = buffer[i];
		GzColor color;
		render->p_shader(render, input, color);
		if(DepthTest(render->display, input.positon[0], input.positon[1], input.positon[2]))
		{
			if((int)input.positon[0] == 117 && (int)input.positon[1] == 83)
			{
				int dummy = 0;
				dummy++;
			}

			GzPutDisplay(render->display, input.positon[0], input.positon[1],
				ctoi(color[0]), ctoi(color[1]), ctoi(color[2]), 1.0f, input.positon[2] );
		}
	}

	//draw wired mesh
	int indices[6] = { 0, 1, 1, 2, 2, 0};
	GzColor oldColor = {render->flatcolor[0], render->flatcolor[1], render->flatcolor[2]};
	render->flatcolor[0] = 0.0f;
	render->flatcolor[1] = 0.0f;
	render->flatcolor[2] = 0.0f;
	//DrawLine(render, 3, vertices, indices);
	render->flatcolor[0] = oldColor[0];
	render->flatcolor[1] = oldColor[1];
	render->flatcolor[2] = oldColor[2];

	return GZ_SUCCESS;
}

int GzPutCamera(GzRender *render, GzCamera *camera)
{
	/*
	- overwrite renderer camera structure with new camera definition
	*/
	render->camera.FOV = camera->FOV;
	render->camera.position[0] = camera->position[0];
	render->camera.position[1] = camera->position[1];
	render->camera.position[2] = camera->position[2];
	render->camera.lookat[0] = camera->lookat[0];
	render->camera.lookat[1] = camera->lookat[1];
	render->camera.lookat[2] = camera->lookat[2];
	render->camera.worldup[0] = camera->worldup[0];
	render->camera.worldup[1] = camera->worldup[1];
	render->camera.worldup[2] = camera->worldup[2];

	GzMatrix& Xpi = render->camera.Xpi;
	float d_inverse= tan(camera->FOV/(2.0f*180.0f)*PI);
	SetIdentity(Xpi);
	Xpi[3][2] = d_inverse;

	GzMatrix& Xiw = render->camera.Xiw;
	SetIdentity(Xiw);
	
	GzCoord forward = {camera->lookat[0] - camera->position[0], 
									camera->lookat[1] - camera->position[1],
									camera->lookat[2] - camera->position[2]};
	Normalize(forward);
	Xiw[2][0] = forward[0];
	Xiw[2][1] = forward[1];
	Xiw[2][2] = forward[2];
	Xiw[2][3] = -1.0f*Dot(camera->position, forward);

	Normalize(camera->worldup);
	float up_dot_forward = Dot(camera->worldup, forward);
	GzCoord up = {camera->worldup[0] - up_dot_forward*forward[0], 
							camera->worldup[1] - up_dot_forward*forward[1],
							camera->worldup[2] - up_dot_forward*forward[2]};
	Normalize(up);
	Xiw[1][0] = up[0];
	Xiw[1][1] = up[1];
	Xiw[1][2] = up[2];
	Xiw[1][3] = -1.0f*Dot(camera->position, up);
	GzCoord right;
	Cross( up, forward, right);
	Normalize(right);
	Xiw[0][0] = right[0];
	Xiw[0][1] = right[1];
	Xiw[0][2] = right[2];
	Xiw[0][3] = -1.0f*Dot(camera->position, right);
	Xiw[3][0] = 0.0f;
	Xiw[3][1] = 0.0f;
	Xiw[3][2] = 0.0f;
	Xiw[3][3] = 1.0f;
	
	return GZ_SUCCESS;	
}

int GzPushMatrix(GzRender *render, GzMatrix	matrix)
{
	/*
	- push a matrix onto the Ximage stack
	- check for stack overflow
	*/
	if(render->matlevel >= 99)
		return GZ_FAILURE;
	render->matlevel++;
	MatrixCopy(matrix, render->Ximage[render->matlevel]);

	return GZ_SUCCESS;
}

int GzPopMatrix(GzRender *render)
{
	/*
	- pop a matrix off the Ximage stack
	- check for stack underflow
	*/
	if(render->matlevel < 0)
		return GZ_FAILURE;
	render->matlevel--;
	return GZ_SUCCESS;
}


/***************************All the following functions are not part of API**************************/

//value in indexes will be parsed into pairs
void DrawLine(GzRender* render,  int numLines, GzCoord* vertices, int* indices)
{
	//Convert to GzIntensity
	GzIntensity r=ctoi(render->flatcolor[0]),
					  g=ctoi(render->flatcolor[1]),
					  b=ctoi(render->flatcolor[2]);
	for(int i=0; i < numLines; i++)
	{
		GzCoord* p0 = vertices + indices[i*2];
		GzCoord* p1 = vertices + indices[i*2+1];

		int x0 = (int)(*p0)[0], y0 = (int)(*p0)[1], z0 = (int)(*p0)[2], x1 = (int)(*p1)[0], y1 = (int)(*p1)[1], z1 = (int)(*p1)[2];
		int dx = x1 - x0, dy = y1 - y0, dz = z1-z0;
		int x = x0, y = y0, z=z0;
		int incX = (dx==0) ? 0 : dx/abs(dx), 
			 incY = (dy==0) ? 0 : dy/abs(dy);
		

		if(DepthTest(render->display, x, y,z))
			GzPutDisplay(render->display, x,y,r,g,b,0,z);
		if(abs(dx) > abs(dy)) 
		{
			int d = 2*dy*incY-dx*incX;
			int d0 = 2*dy*incY, d1 = 2*(dy*incY - dx*incX);
			while( abs(x-x1) != 0)
			{
				if(d< 0)
				{
					d += d0;
					x = x+incX;
				}
				else
				{
					d += d1;
					x = x+incX;
					y = y+incY;
				}
				z = z0+dz*(x-x0)/dx;
				if(z>0)
					z*=0.99;
				else
					z*=1.01;

				if(DepthTest(render->display, x, y,z))
					GzPutDisplay(render->display, x,y,r,g,b,0,z);
			}
			if(DepthTest(render->display, x1, y1,z1))
				GzPutDisplay(render->display, x1,y1,r,g,b,0,z1);
		}
		else
		{
			int d = dy*incY-2*dx*incX;
			int d0 = -2*dx*incX, d1 = 2*(dy*incY - dx*incX);
			while( abs(y-y1) != 0)
			{
				if(d> 0)
				{
					d += d0;
					y = y+incY;
				}
				else
				{
					d += d1;
					y = y+incY;
					x = x+incX;
				}
				z = z0+dz*(y-y0)/dy;
				if(z>0)
					z*=0.99;
				else
					z*=1.01;

				if(DepthTest(render->display, x, y,z))
					GzPutDisplay(render->display, x,y,r,g,b,0,z);
			}
			if(DepthTest(render->display, x1, y1,z1))
				GzPutDisplay(render->display, x1,y1,r,g,b,0,z1);
		}
	}
}

void ScanLineTriangleInterpolation(GzRender* render, const PixelShaderInput vs_output[3], PixelShaderInput* buffer, int* count)
{
	GzCoord vertices[3];
	GzCoord normals[3];
	GzTextureIndex textures[3];
	GzColor colors[3];
	for(int i=0; i<3; i++)
	{
		for(int j=0; j<3; j++)
		{
			vertices[i][j] = vs_output[i].positon[j];
			normals[i][j] = vs_output[i].normal[j];
			colors[i][j] = vs_output[i].color[j];
		}
		for(int j=0; j<2; j++)
			textures[i][j] = vs_output[i].texture[j];

	}

	//Get the min y value and scan from top to bottom
	int min_y_index = 0,  min_y = vertices[0][1];
	for(int i=1; i <3; i++)
	{
		if(min_y>vertices[i][1])
		{
			min_y = vertices[i][1];
			min_y_index = i;
		}
	}

	//swap min_y_index and 0
	if(min_y_index != 0)
	{
		for(int i=0; i<3; i++)
		{
			float temp = vertices[0][i];
			vertices[0][i] = vertices[min_y_index][i];
			vertices[min_y_index][i] = temp;
			temp = normals[0][i];
			normals[0][i] = normals[min_y_index][i];
			normals[min_y_index][i] = temp;
			temp = colors[0][i];
			colors[0][i] = colors[min_y_index][i];
			colors[min_y_index][i] = temp;

		}
		for(int i=0; i<2;i++)
		{	
			float temp = textures[0][i];
			textures[0][i] = textures[min_y_index][i];
			textures[min_y_index][i] = temp;
		}
	}

	//Prepare variables
	if(vertices[1][1] < vertices[2][1])
	{
		for(int i=0; i<3; i++)
		{
			float temp = vertices[1][i];
			vertices[1][i] = vertices[2][i];
			vertices[2][i] = temp;
			temp = normals[1][i];
			normals[1][i] = normals[2][i];
			normals[2][i] = temp;
			temp = colors[1][i];
			colors[1][i] = colors[2][i];
			colors[2][i] = temp;

		}
		for(int i=0; i<2; i++)
		{	
			float temp = textures[1][i];
			textures[1][i] = textures[2][i];
			textures[2][i] = temp;
		}
	}
	assert(vertices[1][1] >= vertices[2][1]);

	//perspective correction
	GzCoord vertices_c[3], normals_c[3];
	for(int i=0; i<3; i++)
	{
		float c = ZMAX/(ZMAX-vertices[i][2]);
		for(int j=0; j<3; j++)
		{
			vertices_c[i][j] = vertices[i][j] * c;
			normals_c[i][j] = normals[i][j] * c;
		}
	}
	
	//clamp x and y
	vertices[0][0] = (int)vertices[0][0];
	vertices[0][1] = (int)vertices[0][1];
	vertices[1][0] = (int)vertices[1][0];
	vertices[1][1] = (int)vertices[1][1];
	vertices[2][0] = (int)vertices[2][0];
	vertices[2][1] = (int)vertices[2][1];
	PixelShaderInput p[2];
	p[0].Assign(vertices[0], normals[0], textures[0], colors[0]);
	p[1].Assign(vertices[0], normals[0], textures[0], colors[0]);

	YBasedScanLine(render, p, buffer, count);

	float y = (vertices[0][1]+1.0f);
	float x0 = vertices[0][0], x1 = vertices[1][0], x2 = vertices[2][0];
	float y0 = vertices[0][1], y1 = vertices[1][1], y2 = vertices[2][1];
	float z0 = vertices[0][2], z1 = vertices[1][2], z2 = vertices[2][2];
	float y0_c = y0*ZMAX/(ZMAX-z0), 
			y1_c = y1*ZMAX/(ZMAX-z1), 
			y2_c = y2*ZMAX/(ZMAX-z2);

	for(; y <= y2; y++)
	{
		float x_0= x0 + (y-y0)/(y1-y0)*(x1-x0);
		float y_0 = y;
		float z_0 = z0 + (y-y0)/(y1-y0)*(z1-z0);
		float x_1 = 0;
		if(y2 == y0)
			x_1 = x0;
		else
			x_1 = x0 + (y-y0)/(y2-y0)*(x2-x0);
		float y_1 = y;
		float z_1 = 0;
		if(y2 == y0)
			z_1 = z0;
		else
			z_1 =z0 + (y-y0)/(y2-y0)*(z2-z0);

		float x_0_c = x_0 * ZMAX/(ZMAX - z_0);
		float y_0_c = y_0 * ZMAX/(ZMAX - z_0);
		float z_0_c = z_0 * ZMAX/(ZMAX - z_0);
		float x_1_c = x_1 * ZMAX/(ZMAX - z_1);
		float y_1_c = y_1 * ZMAX/(ZMAX - z_1);
		float z_1_c = z_1* ZMAX/(ZMAX - z_1);

		float dy_0_p0 = y_0_c - y0_c;
		float dy_0_1p = y1_c - y_0_c;
		float b_0_0 = dy_0_1p/(dy_0_1p + dy_0_p0);
		float b_0_1 = 1 - b_0_0;

		float dy_1_p0 = y_1_c - y0_c;
		float dy_1_2p = y2_c - y_1_c;
		float b_1_0 = dy_1_2p/(dy_1_2p + dy_1_p0);
		float b_1_2 = 1 - b_1_0;
		
		p[0].positon[0] = x_0;
		p[0].positon[1] = y_0;
		p[0].positon[2] = z_0;
		p[1].positon[0] = x_1;
		p[1].positon[1] = y_1;
		p[1].positon[2] = z_1;

		for(int i=0; i<3; i++)
		{
			if(PixelShaderInput::lerp_normal)
			{
				p[0].normal[i] = (normals[0][i]*b_0_0 + normals[1][i]*b_0_1);
				p[1].normal[i] = (normals[0][i]*b_1_0 + normals[2][i]*b_1_2);
			}
			else
			{
				p[0].normal[i] = normals[0][i];
				p[1].normal[i] = normals[0][i];
			}

			if(PixelShaderInput::lerp_color)
			{
				p[0].color[i] = colors[0][i]*b_0_0 + colors[1][i]*b_0_1;
				p[1].color[i] = colors[0][i]*b_1_0 + colors[2][i]*b_1_2;
			}
			else
			{
				p[0].color[i] = colors[0][i];
				p[1].color[i] = colors[0][i];
			}
		}
		
		for(int i=0; i<2; i++)
		{
			if(PixelShaderInput::lerp_texture)
			{
				p[0].texture[i] = textures[0][i]*b_0_0 + textures[1][i]*b_0_1;
				p[1].texture[i] = textures[0][i]*b_1_0 + textures[2][i]*b_1_2;
			}
			else
			{
				p[0].texture[i] = textures[0][i];
				p[1].texture[i] = textures[0][i];
			}
		}

		YBasedScanLine(render, p, buffer, count);
	}

	for(; y <= y1; y++)
	{
		float x_0 = x0 + (y-y0)/(y1-y0)*(x1-x0);
		float y_0 = y;
		float z_0 = z0 + (y-y0)/(y1-y0)*(z1-z0);
		float x_1 = x2 + (y-y2)/(y1-y2)*(x1-x2);
		float y_1  = y;
		float z_1 = z2 + (y-y2)/(y1-y2)*(z1-z2);
		float x_0_c = x_0 * ZMAX/(ZMAX - z_0);
		float y_0_c = y_0 * ZMAX/(ZMAX - z_0);
		float z_0_c = z_0* ZMAX/(ZMAX - z_0);
		float x_1_c = x_1 * ZMAX/(ZMAX - z_1);
		float y_1_c = y_1 * ZMAX/(ZMAX - z_1);
		float z_1_c = z_1 * ZMAX/(ZMAX - z_1);

		float dy_0_p0 = y_0_c - y0_c;
		float dy_0_1p = y1_c - y_0_c;
		float b_0_0 = dy_0_1p/(dy_0_1p + dy_0_p0);
		float b_0_1 = 1 - b_0_0;

		float dy_1_p2 = y_1_c - y2_c;
		float dy_1_1p = y1_c - y_1_c;
		float b_1_2 = dy_1_1p/(dy_1_1p + dy_1_p2);
		float b_1_1 = 1 - b_1_2;

		p[0].positon[0] = x_0;
		p[0].positon[1] = y_0;
		p[0].positon[2] = z_0;
		p[1].positon[0] = x_1;
		p[1].positon[1] = y_1;
		p[1].positon[2] = z_1;

		for(int i=0; i<3; i++)
		{
			if(PixelShaderInput::lerp_normal)
			{
				p[0].normal[i] = (normals[0][i]*b_0_0 + normals[1][i]*b_0_1);
				p[1].normal[i] = (normals[2][i]*b_1_2 + normals[1][i]*b_1_1);
			}
			else
			{
				p[0].normal[i] = normals[0][i];
				p[1].normal[i] = normals[2][i];
			}

			if(PixelShaderInput::lerp_color)
			{
				p[0].color[i] = colors[0][i]*b_0_0 + colors[1][i]*b_0_1;
				p[1].color[i] = colors[2][i]*b_1_2 + colors[1][i]*b_1_1;
			}
			else
			{
				p[0].color[i] = colors[0][i];
				p[1].color[i] = colors[2][i];
			}

		}
		for(int i=0; i<2; i++)
		{
			if(PixelShaderInput::lerp_texture)
			{
				p[0].texture[i] = textures[0][i]*b_0_0 + textures[1][i]*b_0_1;
				p[1].texture[i] = textures[2][i]*b_1_2 + textures[1][i]*b_1_1;
			}
			else
			{
				p[0].texture[i] = textures[0][i];
				p[1].texture[i] = textures[2][i];
			}
		}
		YBasedScanLine(render, p, buffer, count);
	}
}

void YBasedScanLine(GzRender* render, const PixelShaderInput p_raw[2], PixelShaderInput* buffer, int* count)
{
	assert(abs(p_raw[0].positon[1] - p_raw[1].positon[1]) < 1e-5);

	PixelShaderInput p[2];
	if(p_raw[0].positon[0] > p_raw[1].positon[0])
	{
		p[1].Assign(p_raw[0].positon, p_raw[0].normal,p_raw[0].texture,p_raw[0].color);
		p[0].Assign(p_raw[1].positon, p_raw[1].normal,p_raw[1].texture,p_raw[1].color);
	}
	else
	{
		p[0].Assign(p_raw[0].positon, p_raw[0].normal,p_raw[0].texture,p_raw[0].color);
		p[1].Assign(p_raw[1].positon, p_raw[1].normal,p_raw[1].texture,p_raw[1].color);
	}

	float x0 = p[0].positon[0], x1 = p[1].positon[0];
	float y0 = p[0].positon[1], y1 = p[1].positon[1];
	float z0 = p[0].positon[2], z1 = p[1].positon[2];
	float c0 = ZMAX/(ZMAX - z0), c1 = ZMAX/(ZMAX - z1);
	float c0_inv = 1.0f/c0, c1_inv = 1.0f/c1;
	float x0_c = x0 * c0, x1_c = x1 * c1;
	
	buffer[*count].Assign(p[0].positon, p[0].normal, p[0].texture, p[0].color);
	(*count)++;


	for(float x = x0+1, y = p[0].positon[1]; x<x1; x++)
	{
		float z = z0 + (x-x0)/(x1-x0)*(z1-z0);
		float x_c = x * ZMAX/(ZMAX - z);

		float b0 = (x1_c - x_c)/(x1_c - x0_c);
		float b1 = (x_c - x0_c)/(x1_c - x0_c);

		buffer[*count].positon[0] = x;
		buffer[*count].positon[1] = y;
		buffer[*count].positon[2] = z;
		for(int i=0;i<3; i++)
		{
			if(PixelShaderInput::lerp_normal)
				buffer[*count].normal[i] = (b0*p[0].normal[i]+ b1*p[1].normal[i]);
			else
				buffer[*count].normal[i] = p[0].normal[i];
			if(PixelShaderInput::lerp_color)
				buffer[*count].color[i] = b0*p[0].color[i] + b1*p[1].color[i];
			else
				buffer[*count].color[i] = p[0].color[i];
		}
		for(int i=0; i<2; i++)
		{
			if(PixelShaderInput::lerp_texture)
				buffer[*count].texture[i] = b0*p[0].texture[i] + b1*p[1].texture[i];
			else
				buffer[*count].texture[i] = p[0].texture[i];
		}
		(*count)++;
	}
	buffer[*count].Assign(p[1].positon, p[1].normal, p[1].texture, p[1].color);
	(*count)++;
}

bool IsPointToLeftOfLine(int x, int y, GzCoord* v0, GzCoord* v1)
{
	float dx = ((*v1)[0]-(*v0)[0]), dy = ((*v1)[1] - (*v0)[1]);
	float a= dy, b = -dx, c = (dx*(*v0)[1] - dy*(*v0)[0]);
	float d = a*x + b*y+c;
	return d>0.0;
}

//Test whether z is no greater than the depth value in frame buffer
bool DepthTest(GzDisplay* display, int x, int y, GzDepth z)
{
	if(x>=0 && x <display->xres &&y>=0 && y<display->yres)
	{				
		GzIntensity or, og, ob, oa;		//old display
		GzDepth oz;
		GzGetDisplay(display, x, y, &or, &og, &ob,&oa,&oz);
		if(oz>=(z+ZMAX*1e-3) && z>0)
			return true;
		else
			return false;
	}
	else
		return false;
}

