#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "render_wrapper.h"

HWND winhwnd;
int window_width =800;
int window_height = 600;
BackBuffer back_buffer;
TCHAR * win_name = TEXT("Water") ;

// prototypes
LRESULT CALLBACK WndProc ( HWND, UINT, WPARAM, LPARAM );
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd );


inline void draw()
{
	HDC winhdc = GetDC( winhwnd ) ;
	render(&back_buffer);
	// Now copy the back buffer to the front buffer.
	BitBlt(

	winhdc,	// destination buffer
	0, 0,	// x, y to start writing to in the destination buffer
	// counted from top left corner of dest HDC (window)

	// Width, height to copy for
	back_buffer.width, back_buffer.height,

	back_buffer.back_dc, // SOURCE buffer.  We're taking from that back canvas

	0, 0,		// x pt, y pt to START copying from
	// SOURCE.  counted from top left again

	// And we just want a straight copy.
	SRCCOPY );

	ReleaseDC( winhwnd, winhdc ) ;
}




void InitBackBuffer()
{
	back_buffer.width = window_width;
	back_buffer.height = window_height;

	HDC winhdc = GetDC( winhwnd );

	/*

	Imagine you have 2 things:
	- a canvas on which to draw (call this your HBITMAP)
	- set of brushes you will use to draw (call this your HDC)

	1.  HBITMAP.  Canvas
	___________________
	|       *         |
	|   *             |
	|     HBITMAP     |
	|    draw here    |
	|_________________|

	2.  HDC.  Set of brushes.
	<-*
	[]-*
	()-*

	*/

	// Create the HBITMAP "canvas", or surface on which we will draw.
	back_buffer.buffer_bmp = CreateCompatibleBitmap( winhdc, back_buffer.width, back_buffer.height );
	if(back_buffer.buffer_bmp == NULL)
	printf( "failed to create BackBufferBMP" );


	// Create the HDC "device context", or collection of tools
	// and brushes that we can use to draw on our canvas
	back_buffer.back_dc = CreateCompatibleDC( winhdc );
	if(back_buffer.back_dc == NULL)
	printf( "failed to create the backDC" );

	// Permanently associate the surface on which to draw
	// ("canvas") (HBITMAP), with the "set of brushes" (HDC)

	// "select in" the backBufferBMP into the backDC
	HBITMAP oldbmp = (HBITMAP)SelectObject( back_buffer.back_dc, back_buffer.buffer_bmp );


	// Delete the "canvas" that the backDC was
	// going to draw to.  We don't care about it.
	DeleteObject( oldbmp );



	/// Release the dc of the main window itself
	ReleaseDC( winhwnd, winhdc );
}


// Just as said every C++ program starts at main,              //
// every Windows program will start at WinMain.                //
/////////////////////////////////////////////////////////////////
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nShowCmd )
{
#pragma region get window up
  WNDCLASSEX window = { 0 } ;
  window.cbSize			= sizeof(WNDCLASSEX);
  window.hbrBackground	= (HBRUSH) GetStockObject(WHITE_BRUSH);		// BLACK_BRUSH, DKGRAY_BRUSH
  window.hCursor = LoadCursor(NULL, IDC_ARROW);			// load hand cursor.. make to guitar
  window.hIcon = LoadIcon(NULL, IDI_APPLICATION);		// large icon for app
  window.hIconSm = LoadIcon(NULL, IDI_APPLICATION);		// small icon that shows up in top left of window
  window.hInstance = hInstance;							// program's instance handle that was first passed to WinMain by windows when program was first run
  window.lpfnWndProc = WndProc;								// function pointer to WndProc function
  window.lpszClassName = win_name;							// window class name
  window.lpszMenuName = NULL;									// menu name -- but our app has no menu now
  window.style = CS_HREDRAW | CS_VREDRAW;				// window style --

  if(!RegisterClassEx( &window ))
  {
    MessageBox(NULL, TEXT("Something's wrong with the WNDCLASSEX structure you defined.. quitting"), TEXT("Error"), MB_OK);
    return 1;		// return from WinMain.. i.e. quit
  }

  // CREATE THE WINDOW AND KEEP THE HANDLE TO IT.
  winhwnd = CreateWindowEx(	0/*WS_EX_TOPMOST*/,			// extended window style.. this sets the window to being always on top
    win_name,			// window class name.. defined above
    TEXT("Water"),// title bar of window
    WS_OVERLAPPEDWINDOW,// window style
    400, 200,		        // initial x, y start position of window
    window_width, window_height,	          // initial width, height of window.  Can also be CW_USEDEFAULT to let Windows choose these values
    NULL, NULL,			  	// parent window, window menu
    hInstance, NULL);		// program instance handle, creation params

  // now we show and paint our window, so it appears
  ShowWindow( winhwnd, nShowCmd );		// you have to ask that your Window be shown to see it
  UpdateWindow(winhwnd);					// paint the window
#pragma endregion

#pragma region message loop

  MSG			msg;

  init_render(window_width, window_height);

  //show some text
  HDC winhdc = GetDC(winhwnd);
  char buffer[128];
  sprintf(buffer, "Rendering...");
  TextOutA(winhdc, window_width/2 - 50, window_height/2 - 20, buffer, strlen(buffer));

  while(1)
  {
    if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
      if(msg.message == WM_QUIT)
      {
        PostQuitMessage(0);
        break;
      }
      TranslateMessage(&msg);	// translates 'character' keyboard messages
      DispatchMessage(&msg);	// send off to WndProc for processing
    }
    else
    {
      // could put more update code in here
      // before drawing.
      draw();
    }
  }
#pragma endregion

  return 0;	// end program once we exit the message loop
}




////////////////////////////////////////////////////////////////////////
// WndProc - "Window procedure" function
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_CREATE:
	{
		// As soon as the window is first created, create
		// the back buffer.
		InitBackBuffer() ;
		return 0;
	}
	break;
	case WM_PAINT:
		{
			PAINTSTRUCT	ps;

			HDC winhdc = BeginPaint(hwnd, &ps);

			// Do nothing here.  All drawing happens in draw(),
			// and you draw to the backbuffer, NOT the
			// front buffer.

			EndPaint(hwnd, &ps);
			return 0;
		}
		break;

	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}
	break;
	}

	// If message was NOT handled by us, we pass it off to
	// the windows operating system to handle it.
	return DefWindowProc(hwnd, message, wParam, lParam);
}