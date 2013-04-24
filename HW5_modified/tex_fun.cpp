/* Texture functions for cs580 GzLib	*/
#include    "stdafx.h" 
#include	"stdio.h"
#include	"Gz.h"
#include <math.h>


GzColor	*image;
int xs, ys;
int reset = 1;




/* Image texture function */
int tex_fun(float u, float v, GzColor color)
{
  unsigned char		pixel[3];
  unsigned char     dummy;
  char  		foo[8];
  int   		i, j;
  FILE			*fd;

  if (reset) {          /* open and load texture file */
    fd = fopen ("texture", "rb");
    if (fd == NULL) {
      fprintf (stderr, "texture file not found\n");
      exit(-1);
    }
    fscanf (fd, "%s %d %d %c", foo, &xs, &ys, &dummy);
    image = (GzColor*)malloc(sizeof(GzColor)*(xs+1)*(ys+1));
    if (image == NULL) {
      fprintf (stderr, "malloc for texture image failed\n");
      exit(-1);
    }

    for (i = 0; i < xs*ys; i++) {	/* create array of GzColor values */
      fread(pixel, sizeof(pixel), 1, fd);
      image[i][RED] = (float)((int)pixel[RED]) * (1.0 / 255.0);
      image[i][GREEN] = (float)((int)pixel[GREEN]) * (1.0 / 255.0);
      image[i][BLUE] = (float)((int)pixel[BLUE]) * (1.0 / 255.0);
      }

    reset = 0;          /* init is done */
	fclose(fd);
  }


 
#if 1

//hw5, image is the texture file
/* bounds-test u,v to make sure nothing will overflow image array bounds */
  u=u>1?1:u;
  u=u<0?0:u;
  v=v>1?1:v;
  v=v<0?0:v;
/* determine texture cell corner values and perform bilinear interpolation */
 u = u * (xs - 1);
 v = v * (ys - 1); 
 GzColor A, B, C, D;
 int minx,miny,maxx,maxy;
 float s,t;
 minx = floor(u);
 miny = floor(v);
 maxx = ceil(u);
 maxy = ceil(v);
 s =  u - (float)minx;
 t = v - (float)miny;

A[RED] = image[minx + xs * miny][RED];
A[GREEN] = image[minx + xs * miny][GREEN];
A[BLUE] = image[minx + xs * miny][BLUE];

B[RED] = image[maxx + xs * miny][RED];
B[GREEN] = image[maxx + xs * miny][GREEN];
B[BLUE] = image[maxx + xs * miny][BLUE];

C[RED] = image[maxx + xs * maxy][RED];
C[GREEN] = image[maxx + xs * maxy][GREEN];
C[BLUE] = image[maxx + xs * maxy][BLUE];

D[RED] = image[minx + xs * maxy][RED];
D[GREEN] = image[minx + xs * maxy][GREEN];
D[BLUE] = image[minx + xs * maxy][BLUE];




/* set color to interpolated GzColor value and return */
for (int index = RED; index<3;index++)
{
	color[index] = s*t*C[index] + (1-s)*t*D[index] + s*(1-t)*B[index] + (1-s)*(1-t)*A[index];
}



#endif
return GZ_SUCCESS;
}


typedef float Complex[2];
void ComplexMultiply(Complex A, Complex B, Complex AB);
void ComplexAdd(Complex A, Complex B, Complex AB);
float NormComplex(Complex A);


/* Procedural texture function */
int ptex_fun(float u, float v, GzColor color)
{


//Use Julia Set to do the procedural texture mapping
//step 1:  Set the initial value of complex number XX
    Complex XX;
	Complex C;
	XX[0] = u;
	XX[1] = v;
	C[0] = -0.12375;
	C[1] = 0.56805;	
//step 2: Do N iterations and get the result XX
    int N=150;
    int index = 0;
	do 
	{
		ComplexMultiply(XX, XX, XX);
		ComplexAdd(XX, C,XX);
		index++;
	} while (index < N);
     float Scale = NormComplex(XX);
	 if (Scale > 2)
	 {
		 Scale =2;
	 }

color[0] =(u+v)/2;
color[1] =sin(Scale * 3.14 /2);
color[2]= Scale/2;


	

////step 3: Map length of X to a color
//	//step 3.1: Define a LUT
//	 GzColor LUT[11];
//     for (int i = 0; i<11;i++)
//     {
//		 LUT[i][RED] = (0.1 * i + XX[0])/2;
//		 LUT[i][GREEN] =(0.1 * i + XX[1])/2;
//		 LUT[i][BLUE] = 0.1 * i;
//     }
//    //step 3.2: Do the interpolation
//	 int indexdown, indexup;
//	 indexdown = floor(Scale / 0.2);
//	 indexup = ceil(Scale /0.2);
//	 float a_inter = ((float)(indexup * 0.2) - Scale)/((float)(indexup * 0.2) - (float)(indexdown * 0.2));
//	 float b_inter = (Scale - (float)(indexdown * 0.2))/((float)(indexup * 0.2) - (float)(indexdown * 0.2));
//     for (int chan=0;chan<3;chan++)
//     {
//		 color[chan] = a_inter * LUT[indexdown][chan] + b_inter * LUT[indexup][chan];
//     }


  return GZ_SUCCESS;
}


void ComplexMultiply(Complex A, Complex B, Complex AB)
{
     AB[0] = A[0] * B[0] - A[1] * B[1];
	 AB[1] = A[0] * B[1] + A[1] * B[0];
}

void ComplexAdd(Complex A, Complex B, Complex AB)
{
         AB[0] = A[0] + B[0];
		 AB[1] = A[1] + B[1];
}

float NormComplex(Complex A)
{
	return sqrt(A[0] * A[0] + A[1] * A[1]);
}
