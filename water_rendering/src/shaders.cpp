#include "shaders.h"
#include "Gz.h"
#include "utilities.h"

#include <math.h>
#include <stdio.h>
#include <string>
#include <map>

bool PixelShaderInput::lerp_normal = false;
bool PixelShaderInput::lerp_texture = false;
bool PixelShaderInput::lerp_color = false;

namespace
{
struct Image
{
	int width, height;
	GzColor* data;

	inline float* GetPixel(int x, int y)
	{
		return data[y*width + x];
	}
};

/*
* Manager to create and store ppm images.
*/
class ImageManager
{
private:
	typedef std::map<std::string, Image*> ImageMap;

public:
	ImageManager() {}
	~ImageManager()	 {}

	static ImageManager* GetSingleton()
	{
		static ImageManager manager;
		return &manager;
	}

	Image* GetImage(const std::string& name, const std::string& file_path)
	{
		if(data_.find(name) != data_.end())
			return data_[name];
		FILE* fd = fopen (file_path.c_str(), "rb");
		if (fd == NULL) {
			fprintf (stderr, "texture file not found\n");
			exit(-1);
		}
		Image* image  = new Image;
		data_[name] = image;
		char foo[8];
		unsigned char pixel[3];
		unsigned char dummy;
		fscanf (fd, "%s %d %d %c", pixel, &image->width, &image->height, &dummy);
		image->data = (GzColor*)malloc(sizeof(GzColor)*(image->width+1)*(image->height+1));
		if (image == NULL) {
			fprintf (stderr, "malloc for texture image failed\n");
			exit(-1);
		}

		for (int i = 0; i <image->width*image->height; i++) {	/* create array of GzColor values */
			fread(pixel, sizeof(pixel), 1, fd);
			image->data[i][RED] = (float)((int)pixel[RED]) * (1.0f / 255.0f);
			image->data[i][GREEN] = (float)((int)pixel[GREEN]) * (1.0f / 255.0f);
			image->data[i][BLUE] = (float)((int)pixel[BLUE]) * (1.0f / 255.0f);
		}
		fclose(fd);
		return image;
	}
	
private:
	ImageMap data_;
};
}

/* Image texture function */
static int tex_fun(float u, float v, GzColor color, const std::string& name, const std::string& file_path)
{
	/* bounds-test u,v to make sure nothing will overflow image array bounds */
	/* determine texture cell corner values and perform bilinear interpolation */
	/* set color to interpolated GzColor value and return */

	Image* image = ImageManager::GetSingleton()->GetImage(name, file_path);
	u = u > (1-1e-5) ? (1-1e-5) : u;
	u = u< 1e-5 ? 1e-5  : u;
	v = v > (1-1e-5) ? (1-1e-5) : v;
	v =  v < 1e-5  ? 1e-5  : v;
	float x = u*(image->width), y = v*(image->height);
	int nb[4][2] = { {(int)x, (int)y}, {(int)x, (int)y+1}, {(int)x+1, (int)y+1}, {(int)x+1, (int)y}};
	GzColor c[4];
	for(int i=0; i<4; i++)
	{
		float* pixel = image->GetPixel(nb[i][0],nb[i][1]);
		c[i][0] = pixel[0];
		c[i][1] = pixel[1];
		c[i][2] = pixel[2];
	}

	//interpolate y
	GzColor c01, c32;
	for(int i=0; i<3; i++)
	{
		c01[i] = c[1][i]*(y-nb[0][1]) + c[0][i]*(nb[1][1]-y);
		c32[i] = c[2][i]*(y-nb[3][1]) + c[3][i]*(nb[2][1]-y);
	}

	//interpolate x
	for(int i=0; i<3; i++)
	{
		color[i] = c32[i]*(x-nb[0][0]) + c01[i]*(nb[3][0] - x);
	}

	return 0;
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
	MatrixMultiplyVector(render->camera.Xiw, n_i, n_i);
	Normalize(n_i);
	GzCoord view_reflection, temp;
	Scale(n_i, 2*Dot(n_i, view), temp);
	VectorSubtract(temp, view, view_reflection);
	
	GzMatrix Xwi;
	MatrixInverse(render->camera.Xiw, Xwi);
	GzCoord pos_w;
	MatrixMultiplyVector(Xwi, pos_i, pos_w);
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
