#include "disp.h" /* include your own disp.h file (e.g. hw1)*/

/* Camera defaults */
#define	DEFAULT_FOV		35.0
#define	DEFAULT_IM_Z	(-10.0)  /* world coords for image plane origin */
#define	DEFAULT_IM_Y	(5.0)    /* default look-at point = 0,0,0 */
#define	DEFAULT_IM_X	(-10.0)

#define	DEFAULT_AMBIENT	{0.1, 0.1, 0.1}
#define	DEFAULT_DIFFUSE	{0.7, 0.6, 0.5}
#define	DEFAULT_SPECULAR	{0.2, 0.3, 0.4}
#define	DEFAULT_SPEC		32

#define	MATLEVELS	100		/* how many matrix pushes allowed */
#define	MAX_LIGHTS	10		/* how many lights allowed */

///* Dummy definition : change it later */
//#ifndef GzLight
//#define GzLight		GzPointer
//#endif

//#ifndef GzTexture
//#define GzTexture	GzPointer
//#endif

#ifndef GZRENDER
#define GZRENDER
typedef struct {			/* define a renderer */
  GzRenderClass	renderClass;
  GzDisplay		*display;
  short		    open;
  GzCamera		camera;
  short		    matlevel;	        /* top of stack - current xform */
  GzMatrix		Ximage[MATLEVELS];	/* stack of xforms (Xsm) */
  GzMatrix		Xnorm[MATLEVELS];	/* xforms for norms (Xim) */
  GzMatrix		Xsp;		        /* NDC to screen (pers-to-screen) */
  GzColor		flatcolor;          /* color state for flat shaded triangles */
  int			interp_mode;
  int			numlights;
  GzLight		lights[MAX_LIGHTS];
  GzLight		ambientlight;
  GzColor		Ka, Kd, Ks;
  float		    spec;		/* specular power */
  GzTexture		tex_fun;    /* tex_fun(float u, float v, GzColor color) */
}  GzRender;
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
short	ctoi(float color);

//self defined
float dotproduct(GzCoord u, GzCoord v);
void CrossProduct(GzCoord u, GzCoord v, GzCoord w);
void SetIdentity(GzMatrix M);
void Normalized(GzCoord u);
void MatrixMultiply(GzMatrix A, GzMatrix B, GzMatrix C);

float interpoloateZ(float x, float y, GzCoord abc, float d);
void PlaneEquation(GzCoord v1, GzCoord v2, GzCoord v3,GzCoord w);
void VertexSorting(GzCoord v1, GzCoord v2, GzCoord v3,GzCoord n1, GzCoord n2, GzCoord n3, GzTextureIndex uv1, GzTextureIndex uv2, GzTextureIndex uv3);
float IntersectPoint(GzPointer pv1, GzPointer pv2, GzPointer pv3);
BOOL INTri(int x, int y, GzCoord v1, GzCoord v2, GzCoord v3);
void getboundingbox(float *minX,float *minY,float *maxX,float *maxY,GzCoord V1,GzCoord V2,GzCoord V3);

void Transform(GzCoord v1, GzCoord v2, GzCoord v3, GzRender * render);
void dotproductv(GzMatrix M, GzCoord v);
void numDotVector(float num, GzCoord v1, GzCoord v);
void vectorMinus(GzCoord v1, GzCoord v2, GzCoord v);
void vectorAdd(GzCoord v1, GzCoord v2, GzCoord v);

//hw4
void ToUnitaryRotation(GzMatrix M);
int GzPushMatrixToXnorm(GzRender *render, GzMatrix	matrix);
void TransformNorm(GzCoord n1, GzCoord n2, GzCoord n3, GzRender * render);
void Innerdotproduct(GzCoord v1, GzCoord v2, GzCoord v3);
void   CalculateColor(GzCoord N, GzRender * render, GzColor color);
void Interpolation(GzCoord n, int i, int j, GzCoord v1, GzCoord v2, GzCoord v3, GzCoord n1, GzCoord n2, GzCoord n3);
void GettriangleNormal(GzCoord v1, GzCoord v2, GzCoord v3, GzCoord norm);


//hw5
void TransuvTopersp(float Vzs, GzTextureIndex uv);
void TransUVTouv(float Vzs, GzTextureIndex UV);
void InterpolateUVatPixel(GzCoord v1, GzCoord v2, GzCoord v3, GzTextureIndex uv1, GzTextureIndex uv2, GzTextureIndex uv3, int i, int j, GzTextureIndex Pixeluv);
void   ModifiedCalculateColor(GzCoord N, GzRender * render, GzColor color);

