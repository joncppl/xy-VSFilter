// overlay.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "assert.h"
#include "overlay.h"
#include "overlay_func.h"
#include "overlay_config.h"

#undef DEBUG
#define MAX_LOADSTRING 100
#define ALPHA_PERCENT 70


FILE *mylog = NULL;
const char logFile[] = "\\Desktop\\xy.log";

ISubRenderFrame *frame = NULL;
std::queue<std::vector<tsubdata>> subs_queue;
//std::vector<tsubdata> subs;

RECT clipRect;

// Global Variables:
HINSTANCE hInst;								// current instance
HWND hWnd;
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

BOOL overlay_is_open()
{
	return IsWindow(hWnd);
}

DWORD WINAPI _trigger_paint(LPVOID dummy)
{
	static BOOL running = FALSE;
	
	if (subs_queue.size() < 10) {
		return 0;
	}
	while (running)
	{
		Sleep(1);
	}
	running = TRUE;
	std::vector<tsubdata> subs = subs_queue.front();

	HDC hdcScreen = GetDC(NULL);
	HBITMAP       hOldBitmap;
	BITMAP        bm;

	ULONG x = GetDeviceCaps(hdcScreen, HORZRES);
	ULONG y = GetDeviceCaps(hdcScreen, VERTRES);

	//create a transparent bitmap to start with
	ULONG   ulWindowWidth = x;
	ULONG   ulWindowHeight = y;
	HBITMAP hbitmap;       // bitmap handle 
	BITMAPINFO bmi;        // bitmap header 
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = ulWindowWidth;
	bmi.bmiHeader.biHeight = ulWindowHeight;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;         // four 8-bit components 
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biSizeImage = ulWindowWidth * ulWindowHeight * 4;
	VOID *pvBits;
	HDC hdcMem2 = CreateCompatibleDC(hdcScreen);
	// create our DIB section and select the bitmap into the dc 
	hbitmap = CreateDIBSection(hdcMem2, &bmi, DIB_RGB_COLORS, &pvBits, NULL, 0x0);
	SelectObject(hdcMem2, hbitmap);
	HBITMAP hOldBitmap2 = (HBITMAP)SelectObject(hdcMem2, hbitmap);
	
	//fill the bitmap entirely with alpha
	for (UINT32 y = 0; y < ulWindowHeight; y++)
		for (UINT32 x = 0; x < ulWindowWidth; x++)
			((UINT32 *)pvBits)[x + y * ulWindowWidth] = 0x00000000;

	//iterate over all bitmaps
	for (int idx = 0; idx < subs.size(); idx++)
	{
		POINT position = subs[idx].position;
		SIZE size = subs[idx].size;
		LPVOID pixels = (LPVOID) subs[idx].pixels;
		int pitch = subs[idx].pitch;
		const int offset = (pitch - size.cx * 4) / -4;

		if (pixels != NULL)
		{
			BITMAPINFO bitmapinfo = { 0 };
			bitmapinfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			bitmapinfo.bmiHeader.biWidth = size.cx - offset;
			bitmapinfo.bmiHeader.biHeight = size.cy*-1;
			bitmapinfo.bmiHeader.biPlanes = 1;
			bitmapinfo.bmiHeader.biCompression = BI_RGB;
			bitmapinfo.bmiHeader.biBitCount = 32;
			DWORD* vImageBuff = NULL;
			HDC hdcMem = CreateCompatibleDC(hdcScreen);

			if (!hdcMem) {
#ifdef DEBUG
				int err = GetLastError();
				fprintf(mylog, "CreateCompatibleDC returned null... %d\n", err);
				fflush(mylog);
#endif
			}
			else
			{

				// create screen compatible DIB
				HBITMAP bitmap = CreateDIBSection(hdcMem, &bitmapinfo, DIB_RGB_COLORS, (void **)&vImageBuff, NULL, 0);
				SelectObject(hdcMem, bitmap);

				if (vImageBuff) {
					if (pixels != NULL)
					{
						//copy the subtitle bitmap into the screen dib
						memcpy(vImageBuff, (void *)(pixels), size.cy*(size.cx - offset) * 4);
						// we can now free the pixels
						//free( *( ( ( void **) pixels ) ) );
						free((void *)subs[idx].pixels);
						pixels = NULL;
					}

					GetObject(bitmap, sizeof(BITMAP), &bm);
					hOldBitmap = (HBITMAP)SelectObject(hdcMem, bitmap);
					RealizePalette(hdcScreen);

					// determine scaling

					double xscale = (double)x / clipRect.right;
					double yscale = (double)y / clipRect.bottom;

					int xpos = position.x;
					int ypos = position.y;

					double outAspect = (double)x / y;
					double inAspect = (double)clipRect.right / clipRect.bottom;

					double extraXScale = 1.0;
					double extraYScale = 1.0;

					//eg: 4:3 -> 16:9
					if (inAspect < outAspect)
					{
						double xp = (double)clipRect.right / clipRect.bottom * y;
						xpos *= xp / clipRect.right;
						xpos += (x - xp) / 2;

						ypos *= yscale;
						extraXScale = outAspect / inAspect;
					}
					//eg: 16:9 -> 4:3
					else if (outAspect > inAspect)
					{
						double yp = (double)x / (clipRect.right / clipRect.bottom);
						ypos *= yp / clipRect.bottom;
						ypos += (y - yp) / 2;
						xpos *= xscale;
						extraYScale = inAspect / outAspect;
					}
					//aspect ratios match so scaling is straightforward
					else
					{
						xpos *= xscale;
						ypos *= yscale;
					}

					int olddimx = bm.bmWidth + offset;
					int olddimy = bm.bmHeight;
					int newdimx = (bm.bmWidth + offset)*xscale / (extraXScale);
					int newdimy = bm.bmHeight*yscale / (extraYScale);

					BLENDFUNCTION bf;
					bf.BlendOp = AC_SRC_OVER;
					bf.BlendFlags = 0;
					bf.AlphaFormat = AC_SRC_ALPHA;  // use source alpha  
					bf.SourceConstantAlpha = 255;  // opaque (disable constant alpha) 

					// blend the bitmap into the main image
					BOOL ret = AlphaBlend(hdcMem2,
						xpos, ypos, //out position
						newdimx, newdimy,  //out size
						hdcMem,
						0, 0, //in position
						olddimx, olddimy, //in size
						bf);
#ifdef DEBUG
					int err = GetLastError();
					fprintf(mylog, "AlphaBlendFailed %d\n", err);
					fflush(mylog);
#endif

					SelectObject(hdcMem, hOldBitmap);
					DeleteObject(hOldBitmap);
					DeleteObject(hbitmap);
					DeleteObject(bitmap);
					DeleteDC(hdcMem);
				}
#ifdef DEBUG
				else
				{
					int err = GetLastError();
					fprintf(mylog, "null vImageBuff... %d\n", err);
					fflush(mylog);
				}
#endif
			}

		}
	}

	BLENDFUNCTION bf;
	bf.BlendOp = AC_SRC_OVER;
	bf.BlendFlags = 0;
	bf.AlphaFormat = AC_SRC_ALPHA;  // use source alpha  
	bf.SourceConstantAlpha = 255 * Opacity() / 100;

	POINT ptZero = { 0 };
	SIZE winsize = { ulWindowWidth, ulWindowHeight };
	
	// copy the generated bitmap into the window control, updating the window alpha
	BOOL res = UpdateLayeredWindow(hWnd, hdcScreen, NULL, &winsize, hdcMem2, &ptZero, RGB(255, 255, 255), &bf, ULW_ALPHA);
#ifdef DEBUG
	if (!res)
	{
		int err = GetLastError();
		fprintf(mylog, "UpdateLayerWindow error %d\n", err);
		fflush(mylog);
	}
#endif
	SelectObject(hdcMem2, hOldBitmap2);
	DeleteObject(hOldBitmap2);
	DeleteDC(hdcMem2);
	ReleaseDC(NULL, hdcScreen);

	subs.clear();
	subs_queue.pop();
	running = FALSE;
	return 0;
}

void trigger_paint()
{
	CreateThread(0, NULL, _trigger_paint, NULL, NULL, NULL);
}

void clear_screen()
{
	std::vector<tsubdata> empty;
	subs_queue.push(empty);
	//subs.clear();
	trigger_paint();
}

DWORD WINAPI init_overlay(LPVOID lpParam)
{
	HINSTANCE hInstance = GetModuleHandle(NULL);
	MSG msg;
	HACCEL hAccelTable;

#ifdef DEBUG
	char* docdir = getenv("USERPROFILE");
	char logFileName[256];
	sprintf(&logFileName[0], "%s%s", docdir, logFile);
	mylog = fopen(logFileName, "a");
#endif

	// Initialize global strings
	wcscpy(szTitle, _T("overlay"));
	wcscpy(szWindowClass, _T("OVERLAY"));
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance))
	{
		return FALSE;
	}

	//This is probably failing... oh well
	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_OVERLAY));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL;// LoadIcon(hInstance, MAKEINTRESOURCE(IDI_OVERLAY));
	wcex.hCursor = NULL;// LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));//(HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName = NULL;// MAKEINTRESOURCE(IDC_OVERLAY);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = NULL;// LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance)
{
	hInst = hInstance; // Store instance handle in our global variable

	// the primary monitor resolution 
	int x = GetSystemMetrics(SM_CXSCREEN);
	int y = GetSystemMetrics(SM_CYSCREEN);
	//TODO: support for non-primary monitor
	//Check EnumDisplayMonitors https://msdn.microsoft.com/en-us/library/windows/desktop/dd162610(v=vs.85).aspx

	// create a window with properties
	// Layered: allows alpha
	// transparent: it can be clicked through
	// topmost: always on top
	// make the window the same size as the monitor
	hWnd = CreateWindowEx(WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST, szWindowClass, NULL, WS_POPUP | WS_VISIBLE,
		0, 0, x, y, NULL, NULL, hInstance, NULL);


	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, SW_SHOWNA);
	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//even though this doesn't actually do anything, it's included to make windows confident the window is running smoothly
	
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc = NULL;

	switch (message)
	{
	case WM_COMMAND:
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	case WM_PAINT:
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_MOUSEMOVE:
		//SetForegroundWindow(hWnd);
		break;

	case WM_LBUTTONDOWN:
		//SetForegroundWindow(hWnd);
		break;

	case WM_NCACTIVATE:
		return false;

	case WM_ACTIVATEAPP:
	{
		BOOL wActive = (bool)wParam;

		if (wActive == false)
			return 0;
		else
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}