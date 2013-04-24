/*   CS580 HW   */
#include    "stdafx.h"  
#include	"Gz.h"
#include	"disp.h"


int GzNewFrameBuffer(char** framebuffer, int width, int height)
{
/* create a framebuffer:
 -- allocate memory for framebuffer : (sizeof)GzPixel x width x height
 -- pass back pointer 
*/
   *framebuffer = (char *)malloc(sizeof(GzPixel) * width * height);
	return GZ_SUCCESS;
}

int GzNewDisplay(GzDisplay	**display, GzDisplayClass dispClass, int xRes, int yRes)
{

/* create a display:
  -- allocate memory for indicated class and resolution
  -- pass back pointer to GzDisplay object in display
*/
	*display = new GzDisplay;
    (*display)->xres = xRes;
	(*display)->yres = yRes;
	(*display)->dispClass = dispClass;
	(*display)->fbuf = (GzPixel *)malloc(sizeof(GzPixel) * xRes * yRes);
	return GZ_SUCCESS;
}


int GzFreeDisplay(GzDisplay	*display)
{
/* clean up, free memory */
  //  memset(display, 0, sizeof(display));
    delete []display;
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
	display->dispClass = GZ_RGBAZ_DISPLAY;
	display->open = 1;
for (int i=0;i<display->xres;i++)
{	
	for (int j=0;j<display->yres;j++)

	{
	   display->fbuf[i + j * display->xres].alpha=1;
	   display->fbuf[i + j * display->xres].blue=0;
       display->fbuf[i + j * display->xres].green=0;
       display->fbuf[i + j * display->xres].red=0;
	   display->fbuf[i + j * display->xres].z = INT_MAX;
	}
}

return GZ_SUCCESS;
}


int GzPutDisplay(GzDisplay *display, int i, int j, GzIntensity r, GzIntensity g, GzIntensity b, GzIntensity a, GzDepth z)
{
/* write pixel values into the display */
	int pos;

	i = i>=1?i:1;
	i = i<=display->xres?i:display->xres;

	j = j>=1?j:1;
	j = j<=display->yres?j:display->yres;

	r = r>=0?r:0;
	r = r<=4095?r:4095;
	g = g>=0?g:0;
	g = g<=4095?g:4095;
	b = b>=0?b:0;
	b = b<=4095?b:4095;
  

	pos = (i-1) + ((j-1) * display->xres);
	display->fbuf[pos].red = r;
    display->fbuf[pos].green = g;
	display->fbuf[pos].blue = b;
	display->fbuf[pos].alpha = a;
	display->fbuf[pos].z =z;
	return GZ_SUCCESS;
}


int GzGetDisplay(GzDisplay *display, int i, int j, GzIntensity *r, GzIntensity *g, GzIntensity *b, GzIntensity *a, GzDepth *z)
{
	/* pass back pixel value in the display */
	/* check display class to see what vars are valid */
	return GZ_SUCCESS;
}


int GzFlushDisplay2File(FILE* outfile, GzDisplay *display)
{

	/* write pixels to ppm file based on display class -- "P6 %d %d 255\r" */
   fprintf( outfile, "P6 %d %d 255\r", display->xres, display->yres  );
   char rr, gg, bb;
   for (int i=0; i< display->xres; i++)
   {
	   for (int j=0;j<display->yres; j++)
	   {
		  rr = (char)(display->fbuf[i*display->yres + j].red >>4);
	      gg = (char)(display->fbuf[i*display->yres + j].green >>4);
	      bb = (char)(display->fbuf[i*display->yres + j].blue >>4);
	     fprintf(outfile, "%c%c%c", rr,gg,bb);
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
	char rr, gg, bb;
    for (int i=0; i< display->xres * display->yres; i++ )
    {
		rr = (char)(display->fbuf[i].red >> 4);
		gg = (char)(display->fbuf[i].green >> 4);
		bb = (char)(display->fbuf[i].blue >> 4);
		framebuffer[3*i+0]=bb;
		framebuffer[3*i+1]=gg;
		framebuffer[3*i+2]=rr;

    }
	return GZ_SUCCESS;
}