/* Texture functions for cs580 GzLib	*/
#include	<stdio.h>
#include	"Gz.h"
#include <time.h>
#include <math.h>
#include <stdlib.h>

/* Image texture function */
int tex_fun(float u, float v, GzColor color)
{
	static GzColor	*image;
	static int xs, ys;
	static int reset = 1;

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
		image[i][RED] = (float)((int)pixel[RED]) * (1.0f / 255.0f);
		image[i][GREEN] = (float)((int)pixel[GREEN]) * (1.0f / 255.0f);
		image[i][BLUE] = (float)((int)pixel[BLUE]) * (1.0f / 255.0f);
		}

	reset = 0;          /* init is done */
	fclose(fd);
	}

	/* bounds-test u,v to make sure nothing will overflow image array bounds */
	/* determine texture cell corner values and perform bilinear interpolation */
	/* set color to interpolated GzColor value and return */
	u = u > (1-1e-5) ? (1-1e-5) : u;
	u = u< 1e-5 ? 1e-5  : u;
	v = v > (1-1e-5) ? (1-1e-5) : v;
	v =  v < 1e-5  ? 1e-5  : v;
	float x = u*(xs-1), y = v*(ys-1);
	int nb[4][2] = { {(int)x, (int)y}, {(int)x, (int)y+1}, {(int)x+1, (int)y+1}, {(int)x+1, (int)y}};
	GzColor c[4];
	for(int i=0; i<4; i++)
	{
		c[i][0] = image[nb[i][1]*xs+nb[i][0]][0];
		c[i][1] = image[nb[i][1]*xs+nb[i][0]][1];
		c[i][2] = image[nb[i][1]*xs+nb[i][0]][2];
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
