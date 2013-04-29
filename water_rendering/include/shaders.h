#ifndef SHADERS_H_
#define  SHADERS_H_

#include "rend.h"

struct PixelShaderInput
{
	GzCoord positon;
	GzCoord normal;
	GzTextureIndex texture;
	GzColor color;
	//alpha vary from 0 to 100
	GzIntensity alpha;	

	static bool lerp_normal;
	static bool lerp_texture;
	static bool lerp_color;

	void Assign(const GzCoord& p, const GzCoord& n, const GzTextureIndex& t, const GzColor& c)
	{
		for(int i=0; i<3; i++)
		{
			positon[i] = p[i];
			normal[i] = n[i];
			color[i] = c[i];
		}

		for(int i=0; i<2; i++)
		{
			texture[i] = t[i];
		}
	}
};

void FlatVertexShader(GzRender *render, int	numParts, const GzToken *nameList, const GzPointer *valueList, PixelShaderInput vs_output[3]);
void FlatPixelShader(GzRender* render, const PixelShaderInput& input, GzColor color);

void GouraudVertexShader(GzRender *render, int	numParts, const GzToken *nameList, const GzPointer *valueList, PixelShaderInput vs_output[3]);
void GouraudPixelShader(GzRender* render, const PixelShaderInput& input, GzColor color);

void RefractionVertexShader(GzRender *render, int	numParts, const GzToken *nameList, const GzPointer *valueList, PixelShaderInput vs_output[3]);
void RefractionPixelShader(GzRender* render, const PixelShaderInput& input, GzColor color);

void GouraudReflectionPixelShader(GzRender* render, const PixelShaderInput& input, GzColor color);

void PhongVertexShader(GzRender *render, int	numParts, const GzToken *nameList, const GzPointer *valueList, PixelShaderInput vs_output[3]);
void PhongPixelShader(GzRender* render, const PixelShaderInput& input, GzColor color);

void PhongTextureVS(GzRender *render, int	numParts, const GzToken *nameList, const GzPointer *valueList, PixelShaderInput vs_output[3]);
void PhongTexturePS(GzRender* render, const PixelShaderInput& input, GzColor color);

void GlobalReflectionVS(GzRender *render, int	numParts, const GzToken *nameList, const GzPointer *valueList, PixelShaderInput vs_output[3]);
void GlobalReflectionPS(GzRender* render, const PixelShaderInput& input, GzColor color);

void FinalWaterVS(GzRender *render, int	numParts, const GzToken *nameList, const GzPointer *valueList, PixelShaderInput vs_output[3]);
void FinalWaterPS(GzRender* render, const PixelShaderInput& input, GzColor color);

void SkyboxVS(GzRender *render, int	numParts, const GzToken *nameList, const GzPointer *valueList, PixelShaderInput vs_output[3]);
void SkyboxPS(GzRender* render, const PixelShaderInput& input, GzColor color);

#endif		//SHADERS_H_