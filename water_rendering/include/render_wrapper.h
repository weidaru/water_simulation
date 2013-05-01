#include <windows.h>

struct BackBuffer
{
	int width, height;
	HDC back_dc;
	HBITMAP buffer_bmp;
};

/*
* All the rendering related stuffs goes here
*/
int render(BackBuffer* bf);

/*
* Initialize the render
*/
int init_render(int x_res, int y_res);

/*
* Load config file
*/
int init_config();

/*
* Release the render
*/
int release_render();