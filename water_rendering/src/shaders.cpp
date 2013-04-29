#include "shaders.h"
#include "Gz.h"
#include "utilities.h"
#include "ImageManager.h"

#include <math.h>
#include <stdio.h>
#include <assert.h>

#include <string>
#include <map>

bool PixelShaderInput::lerp_normal = false;
bool PixelShaderInput::lerp_texture = false;
bool PixelShaderInput::lerp_color = false;


static float lerp(float v1, float v2, float t)
{
	return v1*t + v2*(1-t);
}

/* Image texture function */
static int tex_fun(float u, float v, GzColor color, const std::string& name, const std::string& file_path)
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

static int tex_fun(float u, GzColor color, const std::string& name, const std::string& file_path)
{
	Image* image = ImageManager::GetSingleton()->GetImage(name, file_path);
	u = u > (1-1e-5) ? (1-1e-5) : u;
	u = u< 1e-5 ? 1e-5  : u;
	float x = u*(image->width-2) + 1;
	int x1 = x;
	int x2 = (x+1) > image->width-1 ? image->width-1 : (x+1);
	
	GzColor c[2];
	const float* pixel = image->GetPixel(x1, 0);
	c[0][0] = pixel[0];
	c[0][1] = pixel[1];
	c[0][2] = pixel[2];
	pixel = image->GetPixel(x2, 0);
	c[1][0] = pixel[0];
	c[1][1] = pixel[1];
	c[1][2] = pixel[2];
	for(int i=0; i<3; i++)
	{
		color[i] = lerp(c[0][i], c[1][i], x-x1);
	}

	return 1;
}

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

void RefractionVertexShader(GzRender *render, int	numParts, const GzToken *nameList, const GzPointer *valueList, PixelShaderInput vs_output[3])
{
	ReadInput(render, numParts, nameList, valueList, vs_output);
	vs_output->lerp_normal = true;
	vs_output->lerp_texture = true;
}

void RefractionPixelShader(GzRender* render, const PixelShaderInput& input, GzColor color)
{
	//get world space position
	static GzMatrix m;
	static float fade_distance = 10.0f;
	static bool is_first = true;
	if(is_first)
	{
		MatrixMultiply(render->Xwm, render->Xsm_inverse, m); 
		is_first = false;
	}

	GzColor tex;
	tex_fun(input.texture[0], input.texture[1], tex, "IslandTexture", "");

	GzCoord pos_w;
	MatrixMultiplyVector(m, input.positon, pos_w);
	//TODO: error should vary according to height field
	float error = 2.0f;
	if(pos_w[1] > error)			//Discard pixel above the y = 0 plane, given 1.0 error
	{
		color[0] = -1.0f;
		color[1] = -1.0f;
		color[2] = -1.0f;
		return;
	}

	assert(pos_w[1] <= error);
	color[0] = tex[0];
	color[1] = tex[1];
	color[2] = tex[2];
	float alpha = 1.0f;
	if(error - pos_w[1] < fade_distance)
	{
		float temp =(error - pos_w[1]) /  fade_distance;
		alpha = pow(1-temp, 3.0f);
	}
		
	alpha = Clamp(alpha, 0.0f, 1.0f);
	const_cast<PixelShaderInput&>(input).alpha = alpha * 100;
}

void PhongVertexShader(GzRender *render, int	numParts, const GzToken *nameList, const GzPointer *valueList, PixelShaderInput vs_output[3])
{
	ReadInput(render, numParts, nameList, valueList, vs_output);
	vs_output->lerp_normal = true;
	vs_output->lerp_texture = true;
}

void PhongPixelShader(GzRender* render, const PixelShaderInput& input, GzColor color)
{
	GzCoord view = {0.0f, 0.0f, 1.0f};
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
	tex_fun(input.texture[0], input.texture[1], tex, "IslandTexture", "");

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

void GlobalReflectionVS(GzRender *render, int	numParts, const GzToken *nameList, const GzPointer *valueList, PixelShaderInput vs_output[3])
{
	ReadInput(render, numParts, nameList, valueList, vs_output);
	vs_output->lerp_normal = true;
	vs_output->lerp_texture = true;
}

void GlobalReflectionPS(GzRender* render, const PixelShaderInput& input, GzColor color)
{
	static GzMatrix Xis;
	static bool first_time = true;
	if(first_time)
	{
		MatrixMultiply(render->Xsp, render->camera.Xpi, Xis);
		MatrixInverse(Xis, Xis);
	}

	GzCoord pos_i;
	MatrixMultiplyVector(Xis, input.positon, pos_i);
	GzCoord view;
	Scale(pos_i, -1.0f, view);
	Normalize(view);

	GzCoord n_i = {0.0f, 1.0f, 0.0f};
	MatrixMultiplyVector(render->camera.Xiw, n_i, n_i, true);
	Normalize(n_i);
	GzCoord view_reflection, temp;
	Scale(n_i, 2*Dot(n_i, view), temp);
	VectorSubtract(temp, view, view_reflection);
	
	GzMatrix Xwi;
	MatrixInverse(render->camera.Xiw, Xwi);
	MatrixMultiplyVector(Xwi, view_reflection, view_reflection, true);			//Get view_reflection in world space
	Normalize(view_reflection);

	float epsilon = 1e-2f;
	if(	view_reflection[0] - view_reflection[1] <= -epsilon &&
		view_reflection[0] + view_reflection[1] >= epsilon &&
		view_reflection[2] - view_reflection[1] <= -epsilon &&
		view_reflection[2] + view_reflection[1] >= epsilon)						//find in UP face
	{
		float u = (view_reflection[0]/view_reflection[1] + 1.0f)/2.0f;
		float v =  (view_reflection[2]/view_reflection[1] +1.0f)/2.0f;
		tex_fun(u, v, color, "SkyBoxUp", "cloudy_noon_UP.ppm");
	}
	else
	{
		float sum = view_reflection[0] + view_reflection[2];
		float sub = view_reflection[0] - view_reflection[2];
		if(sum <= -epsilon && sub >=epsilon)				//find in back face
		{
			float u = (-view_reflection[0]/view_reflection[2] + 1.0f)/2.0f;
			float v =  (-view_reflection[1]/view_reflection[2] - 1.0f)/-2.0f;
			tex_fun(u, v, color, "SkyBoxBack", "cloudy_noon_BK.ppm");
		}
		else if(sum >= epsilon && sub >= epsilon)		//find in right face
		{
			float u = (view_reflection[2]/view_reflection[0] - 1.0f)/-2.0f;
			float v =  (view_reflection[1]/view_reflection[0] - 1.0f)/-2.0f;
			tex_fun(u, v, color, "SkyBoxRight", "cloudy_noon_RT.ppm");
		}
		else if(sum <= -epsilon && sub >= epsilon )	//find in left face
		{
			float u = (-view_reflection[2]/view_reflection[0] + 1.0f)/2.0f;
			float v =  (-view_reflection[1]/view_reflection[0] - 1.0f)/-2.0f;
			tex_fun(u, v, color, "SkyBoxLeft", "cloudy_noon_LF.ppm");
		}
		else
		{
			float u = (view_reflection[0]/view_reflection[2] + 1.0f)/2.0f;
			float v =  (view_reflection[1]/view_reflection[2] - 1.0f)/-2.0f;
			tex_fun(u, v, color, "SkyBoxFront", "cloudy_noon_FR.ppm");
			
		}
	}
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
	//TODO: error should vary according to height field
	float error  = 1.0f;
	if(pos_w[1] >= error)			//Discard pixel above the y = 0 plane, given 0.1 error
	{
		color[0] = -1.0f;
		color[1] = -1.0f;
		color[2] = -1.0f;
		return;
	}
	color[0] = input.color[0];
	color[1] = input.color[1];
	color[2] = input.color[2];
}

void FinalWaterVS(GzRender *render, int	numParts, const GzToken *nameList, const GzPointer *valueList, PixelShaderInput vs_output[3])
{
	ReadInput(render, numParts, nameList, valueList, vs_output);
	vs_output->lerp_normal = true;
	vs_output->lerp_texture = true;
}

void FinalWaterPS(GzRender* render, const PixelShaderInput& input, GzColor color)
{
	//leave this for now
	static bool first_time = true;
	static float global_transparentcy = 0.0f, sun_strength = 3.0f, sun_shineness = 512;
	static GzColor sun_color = {1.2f, 0.9f, 0.6f}, water_color = {0.22f, 0.51f, 0.63f};
	static GzCoord sun_vec;
	static GzMatrix Xis;

	if(first_time)
	{
		for(int i=0; i<3; i++)
			sun_vec[i] = render->lights[0].direction[i];
		MatrixMultiplyVector(render->camera.Xiw, sun_vec, sun_vec, true);
		Normalize(sun_vec);
		MatrixMultiply(render->Xsp, render->camera.Xpi, Xis);
		MatrixInverse(Xis, Xis);
		first_time = false;
	}

	GzCoord pos_i;
	MatrixMultiplyVector(Xis, input.positon, pos_i);
	GzCoord view;
	Scale(pos_i, -1.0f, view);
	Normalize(view);

	GzCoord n;
	Scale(input.normal, 1.0f, n);
	Normalize(n);

	GzColor global_ref_tex, global_ref_sun, local_ref_color, refraction_color;
	float local_ref_alpha = 0.0f, refraction_alpha = 0.0f;
	float fresnel = 0.0f;

	GlobalReflectionPS(render, input, global_ref_tex);
	GzCoord reflection;
	GzCoord temp;
	Scale(n, 2*Dot(n, view), temp);
	VectorSubtract(temp, view, reflection);
	{
		//calculate fresnel
		GzColor dummy;
		tex_fun(Dot(view, n), dummy, "Fresnel", "fresnel.ppm");

		fresnel = Clamp(dummy[0]*2.0f, 0.0f, 1.0f);
	}
	for(int i=0; i<3; i++)
	{
		global_ref_sun[i] = sun_strength * sun_color[i] * pow(Clamp(Dot(reflection, sun_vec), 0.0f, 1.0f), sun_shineness);
		global_ref_tex[i] = lerp(global_ref_tex[i] * Clamp(Dot(n, sun_vec), 0.0f, 1.0f),
									 global_ref_tex[i],
									 global_transparentcy);
	}

	{
		GzDepth dummy;
		GzIntensity r,g,b,a;


		GzCoord textcoord_refl;
		float rippling_strength = 0.1f;
		textcoord_refl[0] = input.positon[0] + input.normal[0] * rippling_strength * render->display->xres;
		textcoord_refl[1] = input.positon[1] + input.normal[2] * rippling_strength * render->display->yres;



		int x = Clamp(textcoord_refl[0], 0.0f, render->display->xres - 1);
		int y = Clamp(textcoord_refl[1], 0.0f, render->display->yres - 1);
		GzGetDisplay(render->texture_display[0], x, y, 
							 &r, &g, &b, &a, &dummy);
		local_ref_color[0] = Clamp(r/4095.0f, 0.0f, 1.0f);
		local_ref_color[1] = Clamp(g/4095.0f, 0.0f, 1.0f);
		local_ref_color[2] = Clamp(b/4095.0f, 0.0f, 1.0f);
		local_ref_alpha = Clamp(a/100.0f, 0.0f, 1.0f);
		Clamp(local_ref_alpha, 0.0f, 1.0f);
		x = Clamp(input.positon[0], 0.0f, render->display->xres - 1);
		y = Clamp(input.positon[1], 0.0f, render->display->yres - 1);

		GzGetDisplay(render->texture_display[1], x, y,
							 &r, &g, &b, &a, &dummy);
		refraction_color[0] = Clamp(r/4095.0f, 0.0f, 1.0f);
		refraction_color[1] = Clamp(g/4095.0f, 0.0f, 1.0f);
		refraction_color[2] = Clamp(b/4095.0f, 0.0f, 1.0f);
		refraction_alpha = Clamp(a/100.0f, 0.0f, 1.0f);
		if(refraction_alpha <= 1e-4)
		{
			refraction_color[0] = water_color[0];
			refraction_color[1] = water_color[1];
			refraction_color[2] = water_color[2];
			refraction_alpha = 0.6f;
		}
	}

	for(int i=0; i<3; i++)
	{
		float reflection_color = lerp(local_ref_color[i], global_ref_sun[i] + global_ref_tex[i], local_ref_alpha); 
		//Pick anyone you like to see the effect.
		color[i] = lerp(reflection_color, refraction_color[i] * refraction_alpha, fresnel);		//final color
		//color[i] = global_ref_sun[i];																					//sun color
		//color[i] = fresnel;																								//fresnel
		//color[i] = global_ref_tex[i];																						//global reflection(sky only)
		//color[i] = local_ref_color[i];																					//local reflection
		//color[i] = refraction_color[i];																				//refraction
	}
}

void SkyboxVS(GzRender *render, int numParts, const GzToken *nameList, const GzPointer *valueList, PixelShaderInput vs_output[3])
{
	ReadInput(render, numParts, nameList, valueList, vs_output);
	vs_output->lerp_normal = false;
	vs_output->lerp_color = false;
	vs_output->lerp_texture = true;
}

void SkyboxPS(GzRender* render, const PixelShaderInput& input, GzColor color)
{
	tex_fun(input.texture[0], input.texture[1], color, render->tex_name[0], "");
}