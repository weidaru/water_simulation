/*   CS580 HW   */
#include	"Gz.h"
#include	"disp.h"
#include "stdio.h"

namespace
{
GzPixel* GetPixelFromDisplay(GzDisplay* display, int row, int col) 
{
	return display->fbuf + row*display->xres + col;
}

char GzIntensityToChar(GzIntensity g) 
{
	int gi = (int)(g/4095.0f*255.0f);
	return gi>255 ? 255:gi;
}
}


int GzNewFrameBuffer(char** framebuffer, int width, int height)
{
/* create a framebuffer:
 -- allocate memory for framebuffer : (sizeof)GzPixel x width x height
 -- pass back pointer 
*/
	*framebuffer = new char[width * height * 3];
	if(!*framebuffer)
		return GZ_FAILURE;
	else
		return GZ_SUCCESS;
}

int GzNewDisplay(GzDisplay	**display, GzDisplayClass dispClass, int xRes, int yRes)
{

/* create a display:
  -- allocate memory for indicated class and resolution
  -- pass back pointer to GzDisplay object in display
*/
	*display = new GzDisplay;
	if(!*display)
		return GZ_FAILURE;
	(*display)->dispClass = dispClass;
	(*display)->xres = xRes;
	(*display)->yres = yRes;
	(*display)->fbuf = new GzPixel[xRes*yRes];
	if(!*display)
		return GZ_FAILURE;
	return GZ_SUCCESS;
}


int GzFreeDisplay(GzDisplay	*display)
{
/* clean up, free memory */
	delete[] display->fbuf;
	delete display;
	return GZ_SUCCESS;
}


int GzGetDisplayParams(GzDisplay *display, int *xRes, int *yRes, GzDisplayClass	*dispClass)
{
/* pass back values for an open display */
	*xRes = display->xres;
	*yRes = display->yres;
	*dispClass = display->dispClass;
	return GZ_SUCCESS;
}


int GzInitDisplay(GzDisplay	*display)
{
/* set everything to some default values - start a new frame */
	for(int i=0; i<(display->yres); i++)
	{
		for(int j=0; j<(display->xres); j++)
		{
			GzPixel* p = GetPixelFromDisplay(display, i, j);
			p->red = 4095;
			p->green = 4095;
			p->blue = 4095;
			p->alpha = 0;
			p->z = 2147483647;
		}
	}
	return GZ_SUCCESS;
}


int GzPutDisplay(GzDisplay *display, int i, int j, GzIntensity r, GzIntensity g, GzIntensity b, GzIntensity a, GzDepth z)
{
/* write pixel values into the display */

	if(i < 0 || i > display->xres || j < 0 || j > display->yres)
		return GZ_SUCCESS;
	GzPixel* p = GetPixelFromDisplay(display, j, i);
	if(!p)
		return GZ_FAILURE;
	p->red = r;
	p->green = g;
	p->blue = b;
	p->alpha = a;
	p->z = z;

	return GZ_SUCCESS;
}


int GzGetDisplay(GzDisplay *display, int i, int j, GzIntensity *r, GzIntensity *g, GzIntensity *b, GzIntensity *a, GzDepth *z)
{
	/* pass back pixel value in the display */
	/* check display class to see what vars are valid */
	GzPixel* p = GetPixelFromDisplay(display, j, i);
	if(!p)
		return GZ_FAILURE;
	*r = p->red;
	*g = p->green;
	*b = p->blue;
	*a = p->alpha;
	*z = p->z;
	return GZ_SUCCESS;
}


int GzFlushDisplay2File(FILE* outfile, GzDisplay *display)
{

	/* write pixels to ppm file based on display class -- "P6 %d %d 255\r" */
	fprintf(outfile, "P6 %d %d 255\r", display->xres, display->yres);
	for(int i=0; i<(display->yres); i++)
	{
		for(int j=0; j<(display->xres); j++)
		{
			GzPixel* p = GetPixelFromDisplay(display, i, j);

			char r,g,b;
			r = GzIntensityToChar(p->red);
			g = GzIntensityToChar(p->green);
			b = GzIntensityToChar(p->blue);

			fprintf(outfile, "%c%c%c",r,g,b);
		}
	}
	return GZ_SUCCESS;
}

int GzFlushDisplay2FrameBuffer(char* framebuffer, GzDisplay *display)
{

	/* write pixels to framebuffer: 
		- Put the pixels into the frame buffer
		- Caution: store the pixel to the frame buffer as the order of blue, green, and red 
		- Not red, green, and blue !!!
	*/
	for(int i=0; i<(display->yres); i++)
	{
		for(int j=0; j<(display->xres); j++)
		{
			GzPixel* p = GetPixelFromDisplay(display, i, j);

			char r,g,b,a,z;
			r = GzIntensityToChar(p->red);
			g = GzIntensityToChar(p->green);
			b = GzIntensityToChar(p->blue);

			int curPos = (i*display->xres+j)*3;
			*(framebuffer + curPos) = b;
			*(framebuffer + curPos + 1) = g;
			*(framebuffer + curPos + 2) = r;
		}
	}
	return GZ_SUCCESS;
}