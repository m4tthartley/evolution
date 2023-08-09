#include <windows.h>
#include <stdio.h>

#include "defs.h"

int64_t freq;
double getTime() {
	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);
	//double time = (double)(counter.QuadPart-lastCounter)/double(freq);
	return double(counter.QuadPart) / double(freq);
}

LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	switch(msg) {
	case WM_DESTROY:
		exit(0);
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

int CALLBACK WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd ) {
	WNDCLASS windowClass = {0};
	HWND window;
	RECT drawRect;
	RECT windowRect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};

	windowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	windowClass.hInstance = hInstance;
	windowClass.lpszClassName = "WindowClass";
	windowClass.lpfnWndProc = windowProc;
	RegisterClassA(&windowClass);
	AdjustWindowRectEx(&windowRect, WS_OVERLAPPEDWINDOW | WS_VISIBLE, FALSE, 0);
	window = CreateWindowA("WindowClass",
							"Game Window",
							WS_OVERLAPPEDWINDOW | WS_VISIBLE,
							CW_USEDEFAULT,
							CW_USEDEFAULT,
							windowRect.right-windowRect.left,
							windowRect.bottom-windowRect.top,
							0,
							0,
							hInstance,
							0);
	UpdateWindow(window);
	GetClientRect(window, &drawRect);

	unsigned int* framebuffer;
	BITMAPINFO bitmap = {0};
	HBITMAP hbmp;
	bitmap.bmiHeader.biSize = sizeof(bitmap.bmiHeader);
	bitmap.bmiHeader.biWidth = FRAMEBUFFER_WIDTH;
	bitmap.bmiHeader.biHeight = FRAMEBUFFER_HEIGHT;
	bitmap.bmiHeader.biPlanes = 1;
	bitmap.bmiHeader.biBitCount = 32;
	bitmap.bmiHeader.biCompression = BI_RGB;
	hbmp = CreateDIBSection(GetDC(window), &bitmap, DIB_RGB_COLORS, (void**)&framebuffer, 0, 0);

	LARGE_INTEGER _freq;
	QueryPerformanceFrequency(&_freq);
	freq = _freq.QuadPart/1000;
	int64_t lastCounter;
	LARGE_INTEGER c;
	QueryPerformanceCounter(&c);
	lastCounter = c.QuadPart;

	gameStart(framebuffer);

	const double targetFrameTime = 1000.0/60.0 * 1;
	double secondTime = getTime();
	int fps = 0;
	int64_t testTime = 0;

	int _i = 0;
	for(;;) {
		MSG msg;
		UpdateWindow(window);
		while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessageA(&msg);
		}

		gameUpdate();
		gameRender(framebuffer);
//  		for(int i=0; i<FRAMEBUFFER_WIDTH*FRAMEBUFFER_HEIGHT; ++i) {
//  			framebuffer[i] = rand()%255 << 16;
//  		}
		//framebuffer[_i++] = rand()%255 << 8;

		StretchDIBits(GetDC(window), 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT, framebuffer, &bitmap, DIB_RGB_COLORS, SRCCOPY);

		++fps;
		double t = getTime();
		if(t-secondTime > 1000.0) {
			printf("%i\n", fps);
			secondTime = t;
			fps = 0;
		}

		LARGE_INTEGER counter;
		QueryPerformanceCounter(&counter);
// 		if((double)(counter.QuadPart-testTime)/double(freq) > 1000.0) {
// 			printf("%i\n", fps);
// 			testTime = counter.QuadPart;
// 			fps = 0;
// 		}
		double time = (double)(counter.QuadPart-lastCounter)/double(freq);
		int timeLeft = targetFrameTime-time;
		//printf("%f %i \n", time, timeLeft);
		// double t1 = getTime();
		if(timeLeft>0) Sleep(timeLeft);
// 		double t2 = getTime();
// 		printf("%f %i\n", t2-t1, timeLeft);

		QueryPerformanceCounter(&counter);
		lastCounter = counter.QuadPart;
	}
}

int main() {
	WinMain(GetModuleHandle(NULL), NULL, NULL, 0);
}