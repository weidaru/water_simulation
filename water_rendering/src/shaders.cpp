#include "shaders.h"
#include "Gz.h"
#include "utilities.h"

#include <math.h>

bool PixelShaderInput::lerp_normal = false;
bool PixelShaderInput::lerp_texture = false;
bool PixelShaderInput::lerp_color = false;

static void ReadInput(GzRender *render, int	numParts, const GzToken *nameList, const GzPointer *valueList, PixelShaderInput vs_output[3]);

static void ReadInput(GzRender *render, int	numParts, const GzToken *nameList, const GzPointer *valueList, PixelShaderInput vs_output[3])
{
	for(int i=0; i<numParts; i++)
	{
		switch(nameList[i])
		{
		case GZ_NULL_TOKEN:
			break;
		case GZ_RGB_COLOR:
			render->flatcolor[0] = ((float*)valueList[i])[0];
			render->flatcolor[1] = ((float*)valueList[i])[1];
			render->flatcolor[2] = ((float*)valueList[i])[2];
			break;
		case GZ_POSITION:
			do 
			{
				GzCoord* vertices_raw = (GzCoord*)valueList[i];

				MatrixMultiplyVector(render->Xsm, vertices_raw[0], vs_output[0].positon);
				MatrixMultiplyVector(render->Xsm, vertices_raw[1], vs_output[1].positon);
				MatrixMultiplyVector(render->Xsm, vertices_raw[2], vs_output[2].positon);
			} while (false);
			break;
		case GZ_NORMAL:
			do 
			{
				GzCoord* normals_raw = (GzCoord*)valueList[i];

				MatrixMultiplyVector(render->Xn, normals_raw[0], vs_output[0].normal);
				Normalize(vs_output[0].normal);
				MatrixMultiplyVector(render->Xn, normals_raw[1], vs_output[1].normal);
				Normalize(vs_output[1].normal);
				MatrixMultiplyVector(render->Xn, normals_raw[2], vs_output[2].normal);
				Normalize(vs_output[2].normal);
			} while (false);
			break;
		case GZ_TEXTURE_INDEX:
			do 
			{
				GzTextureIndex* textures = (GzTextureIndex*)valueList[i];
				for(int i=0; i<3; i++)
				{
					vs_output[i].texture[0] = textures[i][0];
					vs_output[i].texture[1] = textures[i][1];
				}
			} while (false);
			break;
		default:
			break;
		}
	}

	//Vertex ordering
	float sum = 0;
	for(int i=0; i<3; i++)
	{
		sum+= (vs_output[(i+1)%3].positon[0]- vs_output[i].positon[0])*(vs_output[(i+1)%3].positon[1] + vs_output[i].positon[1]);
	}
	if(sum < 0)
	{
		for(int i=0; i<3; i++)
		{
			float temp = vs_output[1].positon[i];
			vs_output[1].positon[i] = vs_output[2].positon[i];
			vs_output[2].positon[i] = temp;
		}
		for(int i=0; i<3; i++)
		{
			float temp = vs_output[1].normal[i];
			vs_output[1].normal[i] = vs_output[2].normal[i];
			vs_output[2].normal[i] = temp;
		}
		for(int i=0; i<2; i++)
		{
			float temp = vs_output[1].texture[i];
			vs_output[1].texture[i] = vs_output[2].texture[i];
			vs_output[2].texture[i] = temp;
		}
	}
}

void FlatVertexShader(GzRender *render, int	numParts, const GzToken *nameList, const GzPointer *valueList, PixelShaderInput vs_output[3])
{
	ReadInput(render, numParts, nameList, valueList, vs_output);
	
	for(int i=0; i<3; i++)
	{
		vs_output[i].color[0]  = render->flatcolor[0];
		vs_output[i].color[1]  = render->flatcolor[1];
		vs_output[i].color[2]  = render->flatcolor[2];
	}
}

void FlatPixelShader(GzRender* render, const PixelShaderInput& input, GzColor color)
{
	color[0] = input.color[0];
	color[1] = input.color[1];
	color[2] = input.color[2];
}

void GouraudVertexShader(GzRender *render, int	numParts, const GzToken *nameList, const GzPointer *valueList, PixelShaderInput vs_output[3])
{
	ReadInput(render, numParts, nameList, valueList, vs_output);
	vs_output->lerp_color = true;

	for(int i=0; i<3; i++)
	{
		GzCoord view = {0.0f, 0.0f, -1.0f};
		GzCoord n_i;
		Scale(vs_output[i].normal, 1.0, n_i);
		Normalize(n_i);

		for(int j=0; j<3; j++)
		{
			float add_value;
			//Ambient
			add_value = render->Ka[j]*render->ambientlight.color[j];	
			vs_output[i].color[j] =  Clamp(add_value, 0.0f, 1.0f);
			for(int k=0; k<render->numlights; k++)
			{
				float d_n_v = Dot(n_i, view);
				float d_n_i = Dot(n_i, render->lights[k].direction);
				GzCoord n;
				Scale(n_i, 1.0, n);
				if(d_n_v * d_n_i < 0)
					continue;
				else if(d_n_v < 0 && d_n_i < 0)
					Scale(n_i, -1.0, n);

				//Diffuse
				add_value = render->Kd[j] * render->lights[k].color[j] * Dot(n, render->lights[k].direction);
				vs_output[i].color[j] +=  Clamp(add_value, 0.0f, 1.0f);
				GzCoord reflection;
				GzCoord temp;
				Scale(n, 2*Dot(n, render->lights[k].direction), temp);
				VectorSubtract(temp, render->lights[k].direction, reflection);
				//Specular
				add_value= render->Ks[j] * render->lights[k].color[j] * pow(Dot(reflection, view), render->spec);
				vs_output[i].color[j] +=  Clamp(add_value, 0.0f, 1.0f);
			}
			vs_output[i].color[j] =  Clamp(vs_output[i].color[j], 0.0f, 1.0f);
		}
	}
}

void GouraudPixelShader(GzRender* render, const PixelShaderInput& input, GzColor color)
{
	color[0] = input.color[0];
	color[1] = input.color[1];
	color[2] = input.color[2];
}

void GouraudRefractionPixelShader(GzRender* render, const PixelShaderInput& input, GzColor color)
{
	//get world space position
	static GzMatrix m;
	static bool is_first = true;
	if(is_first)
	{
		MatrixMultiply(render->Xwm, render->Xsm_inverse, m); 
		is_first = false;
	}
	GzCoord pos_w;
	MatrixMultiplyVector(m, input.positon, pos_w);
	if(pos_w[1] >= 0.1f)			//Discard pixel above the y = 0 plane, given 0.1 error
	{
		color[0] = -1.0f;
		color[1] = -1.0f;
		color[2] = -1.0f;
		return;
	}

	color[0] = input.color[0];
	color[1] = input.color[1];
	color[2] = input.color[2];
	const_cast<PixelShaderInput&>(input).alpha = 100;
}

void GouraudReflectionPixelShader(GzRender* render, const PixelShaderInput& input, GzColor color)
{
	//get world space position
	static GzMatrix m;
	static bool is_first = true;
	if(is_first)
	{
		MatrixMultiply(render->Xwm, render->Xsm_inverse, m); 
		is_first = false;
	}
	GzCoord pos_w;
	MatrixMultiplyVector(m, input.positon, pos_w);
	if(pos_w[1] >= 0.0f)			//Discard pixel above the y = 0 plane, given 0.1 error
	{
		color[0] = -1.0f;
		color[1] = -1.0f;
		color[2] = -1.0f;
		return;
	}

	color[0] = input.color[0];
	color[1] = input.color[1];
	color[2] = input.color[2];
	const_cast<PixelShaderInput&>(input).alpha = 100;
}


void PhongVertexShader(GzRender *render, int	numParts, const GzToken *nameList, const GzPointer *valueList, PixelShaderInput vs_output[3])
{
	ReadInput(render, numParts, nameList, valueList, vs_output);
	vs_output->lerp_normal = true;
	vs_output->lerp_texture = true;
}

void PhongPixelShader(GzRender* render, const PixelShaderInput& input, GzColor color)
{
	GzCoord view = {0.0f, 0.0f, -1.0f};
	GzCoord n_i;
	Scale(input.normal, 1.0, n_i);
	Normalize(n_i);

	for(int j=0; j<3; j++)
	{
		float add_value;
		//Ambient
		add_value = render->Ka[j]*render->ambientlight.color[j];	
		color[j] =  Clamp(add_value, 0.0f, 1.0f);
		for(int k=0; k<render->numlights; k++)
		{
			float d_n_v = Dot(n_i, view);
			float d_n_i = Dot(n_i, render->lights[k].direction);
			GzCoord n;
			Scale(n_i, 1.0, n);
			if(d_n_v * d_n_i < 0)
				continue;
			else if(d_n_v < 0 && d_n_i < 0)
				Scale(n_i, -1.0, n);

			//Diffuse
			add_value = render->Kd[j] * render->lights[k].color[j] * Dot(n, render->lights[k].direction);
			color[j] +=  Clamp(add_value, 0.0f, 1.0f);
			GzCoord reflection;
			GzCoord temp;
			Scale(n, 2*Dot(n, render->lights[k].direction), temp);
			VectorSubtract(temp, render->lights[k].direction, reflection);
			//Specular
			add_value= render->Ks[j] * render->lights[k].color[j] * pow(Dot(reflection, view), render->spec);
			color[j] +=  Clamp(add_value, 0.0f, 1.0f);
		}
		color[j] =  Clamp(color[j], 0.0f, 1.0f);
	}
}

void PhongTextureVS(GzRender *render, int	numParts, const GzToken *nameList, const GzPointer *valueList, PixelShaderInput vs_output[3])
{
	ReadInput(render, numParts, nameList, valueList, vs_output);
	vs_output->lerp_normal = true;
	vs_output->lerp_texture = true;
}

void PhongTexturePS(GzRender* render, const PixelShaderInput& input, GzColor color)
{
	GzCoord view = {0.0f, 0.0f, -1.0f};
	GzCoord n_i;
	Scale(input.normal, 1.0, n_i);
	Normalize(n_i);

	GzColor tex;
	render->tex_fun[0](input.texture[0], input.texture[1], tex);

	for(int j=0; j<3; j++)
	{
		float add_value;
		//Ambient
		add_value = tex[j]*render->ambientlight.color[j];	
		color[j] =  Clamp(add_value, 0.0f, 1.0f);
		for(int k=0; k<render->numlights; k++)
		{
			float d_n_v = Dot(n_i, view);
			float d_n_i = Dot(n_i, render->lights[k].direction);
			GzCoord n;
			Scale(n_i, 1.0, n);
			if(d_n_v * d_n_i < 0)
				continue;
			else if(d_n_v < 0 && d_n_i < 0)
				Scale(n_i, -1.0, n);

			//Diffuse
			add_value = tex[j] * render->lights[k].color[j] * Dot(n, render->lights[k].direction);
			color[j] +=  Clamp(add_value, 0.0f, 1.0f);
			GzCoord reflection;
			GzCoord temp;
			Scale(n, 2*Dot(n, render->lights[k].direction), temp);
			VectorSubtract(temp, render->lights[k].direction, reflection);
			//Specular
			add_value= render->Ks[j] * render->lights[k].color[j] * pow(Dot(reflection, view), render->spec);
			color[j] +=  Clamp(add_value, 0.0f, 1.0f);
		}
		color[j] =  Clamp(color[j], 0.0f, 1.0f);
	}
}