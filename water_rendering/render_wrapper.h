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
void render(struct BackBuffer* bf);