#ifndef RENDER_H_
#define RENDER_H_

#include "Gz.h"
#include "disp.h" /* include your own disp.h file (e.g. hw1)*/

#define	MATLEVELS	100		/* how many matrix pushes allowed */
#define	MAX_LIGHTS	10		/* how many lights allowed */
#define MAX_TEXTURE 10

#ifndef GZRENDER
#define GZRENDER

struct PixelShaderInput;
struct GzRender;

struct GzRender {			/* define a renderer */
	typedef void (*VertexShader)(GzRender *, int, const GzToken *, const GzPointer *, PixelShaderInput[3]);
	typedef void (*PixelShader)(GzRender* , const PixelShaderInput&, GzColor);

	GzRenderClass	renderClass;
	GzDisplay		*display;
	short		    open;
	GzCamera		camera;
	short		    matlevel;	        /* top of stack - current xform */
	GzMatrix		Ximage[MATLEVELS];	/* stack of xforms (Xsm) */
	GzMatrix		Xnorm[MATLEVELS];	/* xforms for norms (Xim) */
	GzMatrix		Xsp;		        /* NDC to screen (pers-to-screen) */
	GzMatrix		Xwm;
	GzMatrix		Xsm;
	GzMatrix		Xsm_inverse;
	GzMatrix		Xn;				//Transformation for normals from Object to World space.
	GzMatrix		Xn_inverse;
	GzColor		flatcolor;          /* color state for flat shaded triangles */
	int			interp_mode;
	int			numlights;
	GzLight		lights[MAX_LIGHTS];
	GzLight		ambientlight;
	GzColor		Ka, Kd, Ks;
	float		    spec;		/* specular power */
	GzTexture		tex_fun[MAX_TEXTURE];    /* tex_fun(float u, float v, GzColor color) */
	char tex_name[MAX_TEXTURE][64];
	int tex_count;
	VertexShader v_shader;
	PixelShader p_shader;
	bool show_wireframe;
	GzDisplay* texture_display[MAX_TEXTURE];
};
#endif

// Function declaration
// HW2
int GzNewRender(GzRender **render, GzRenderClass renderClass, GzDisplay *display);
int GzFreeRender(GzRender *render);
int GzBeginRender(GzRender	*render);
int GzPutAttribute(GzRender	*render, int numAttributes, GzToken	*nameList, 
	GzPointer *valueList);
int GzPutTriangle(GzRender *render, int	numParts, GzToken *nameList,
	GzPointer *valueList);

// HW3
int GzPutCamera(GzRender *render, GzCamera *camera);
int GzPushMatrix(GzRender *render, GzMatrix	matrix);
int GzPopMatrix(GzRender *render);

// Object Translation
int GzRotXMat(float degree, GzMatrix mat);
int GzRotYMat(float degree, GzMatrix mat);
int GzRotZMat(float degree, GzMatrix mat);
int GzTrxMat(GzCoord translate, GzMatrix mat);
int GzScaleMat(GzCoord scale, GzMatrix mat);


#endif		//RENDER_H_