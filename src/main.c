#define STRICT
#define UNICODE
#define WIN32_LEAN_AND_MEAN 1
#define NOMINMAX            1

#include <windows.h>
#include <timeapi.h>
#include <shlwapi.h>

#undef STRICT
#undef UNICODE
#undef WIN32_LEAN_AND_MEAN
#undef NOMINMAX
#undef far
#undef near

#include <intrin.h>

typedef signed __int8  i8;
typedef signed __int16 i16;
typedef signed __int32 i32;
typedef signed __int64 i64;

typedef unsigned __int8  u8;
typedef unsigned __int16 u16;
typedef unsigned __int32 u32;
typedef unsigned __int64 u64;

typedef u8 bool;
#define true ((bool)1)
#define false ((bool)0)

typedef float f32;
typedef union Float_Bits
{
  float f;
  u32 bits;
} Float_Bits;

typedef i64 imm;
typedef u64 umm;

#define ASSERT(EX) ((EX) ? 1 : (*(volatile int*)0 = 0, 0))

#define NOT_IMPLEMENTED ASSERT(!"NOT_IMPLEMENTED")

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

#include "gol.h"

LRESULT
WndProc(HWND window, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if (msg == WM_QUIT || msg == WM_CLOSE || msg == WM_DESTROY)
	{
		PostQuitMessage(0);
		return 0;
	}
	else return DefWindowProcW(window, msg, wparam, lparam);
}

void __stdcall
WinMainCRTStartup()
{
	HINSTANCE instance = GetModuleHandle(0);

  u32 screen_width;
  u32 screen_height;
  u32* screen_backbuffer = 0;
	{ /// Rendering stuff
		screen_width  = (u32)GetSystemMetrics(SM_CXSCREEN);
		screen_height = (u32)GetSystemMetrics(SM_CYSCREEN);

		if (screen_width == 0 || screen_height == 0)
		{
			//// ERROR: This is stupid
			NOT_IMPLEMENTED;
		}
		else
		{
			screen_backbuffer = VirtualAlloc(0, (u64)screen_width * (u64)screen_height * sizeof(u32), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
			if (screen_backbuffer == 0)
			{
				//// ERROR
				NOT_IMPLEMENTED;
			}
		}
	}

	WNDCLASSEXW window_class_info = {
		.cbSize        = sizeof(WNDCLASSEXW),
		.style         = CS_OWNDC,
		.lpfnWndProc   = &WndProc,
		.hInstance     = instance,
		.hIcon         = LoadIcon(0, IDI_APPLICATION),
		.hCursor       = LoadCursor(0, IDC_ARROW),
		.lpszClassName = L"THE_DUMB_SMOOD_WINDOW_CLASS_NAME",
	};

	if (!RegisterClassExW(&window_class_info)) NOT_IMPLEMENTED;
	else
	{
		HWND window = CreateWindowW(window_class_info.lpszClassName, L"Smood", WS_POPUP, 0, 0, screen_width, screen_height, 0, 0, 0, 0);

		if (window == 0) NOT_IMPLEMENTED;
		else
		{
			timeBeginPeriod(1);

			LARGE_INTEGER perf_counter_freq = {0};
			LARGE_INTEGER flip_time         = {0};
			QueryPerformanceFrequency(&perf_counter_freq);
			QueryPerformanceCounter(&flip_time);

			ShowWindow(window, SW_SHOW);

      u32 grid_width  = screen_width / 8;
      u32 grid_height = screen_height / 8;
      f32* old_grid   = VirtualAlloc(0, (u64)grid_width*(u64)grid_height*sizeof(f32), MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
      f32* new_grid   = VirtualAlloc(0, (u64)grid_width*(u64)grid_height*sizeof(f32), MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

      if (old_grid == 0 || new_grid == 0)
      {
        //// ERROR
        NOT_IMPLEMENTED;
      }

      Init(old_grid, grid_width, grid_height);

			bool is_running = true;
			while (is_running)
			{
				MSG msg;
				while (PeekMessageW(&msg, 0, 0, 0, PM_REMOVE))
				{
					if (msg.message == WM_QUIT || msg.message == WM_CLOSE)
					{
						is_running = false;
						break;
					}
					else DispatchMessage(&msg);
				}

        Tick(screen_backbuffer, screen_width, screen_height, old_grid, new_grid, grid_width, grid_height);
        f32* tmp = old_grid;
        old_grid = new_grid;
        new_grid = tmp;

				HDC dc = GetDC(window);
        
				BITMAPINFO bitmap_info = {
					.bmiHeader = {
						.biSize          = sizeof(BITMAPINFOHEADER),
						.biWidth         = screen_width,
						.biHeight        = -(i32)screen_height,
						.biPlanes        = 1,
						.biBitCount      = 32,
						.biCompression   = BI_RGB,
					},
				};
				SetDIBitsToDevice(dc, 0, 0, screen_width, screen_height, 0, 0, 0, screen_height, screen_backbuffer, &bitmap_info, DIB_RGB_COLORS);

				ReleaseDC(window, dc);

				{ /// Frame time
					LARGE_INTEGER end_time;
					QueryPerformanceCounter(&end_time);

					f32 frame_time = (f32)(end_time.QuadPart - flip_time.QuadPart) / (f32)perf_counter_freq.QuadPart;
          
          if (frame_time*1000 <= 15) Sleep((u32)(16 - frame_time*1000));

					QueryPerformanceCounter(&flip_time);
				}
			}
		}
	}
	ExitProcess(0);
}
