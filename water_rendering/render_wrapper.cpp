#include "render_wrapper.h"

#include <stdio.h>

void render(struct BackBuffer* bf)
{
	// now just draw something into it for fun
	char buf[300];
	sprintf( buf, "HELLO WATER!" );
	SetBkColor(bf->back_dc, RGB(0, 0, 0));
	SetTextColor(bf->back_dc, RGB(255,255,255));
	TextOutA( bf->back_dc, 20, 20, buf, strlen(buf) );

	//Draw a simple rectangle using SetPixel.
	for(int i = 50 ; i<=150; i++)
		for(int j = 50; j<=150; j++)
			SetPixel(bf->back_dc, i, j, RGB(255,0,0));
}