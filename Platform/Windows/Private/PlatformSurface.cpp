#include <Platform/PlatformSurface.h>
#include <Platform/PlatformInput.h>

#include <windows.h>
#include <windowsx.h>


namespace Platform::Surface {

	// =========================================================================
	// Internal Win32 Surface Context (VirtualAlloc-backed)
	// =========================================================================
	struct SurfaceContext
	{
		HWND                     hwnd;
		PFN_SurfaceEventCallback pCallback;
		void* pUserData;
		uint32                   width;
		uint32                   height;
		float                    dpiScale;
		float                    opacity;
		eSurfaceMode             currentMode;
		WINDOWPLACEMENT          wpPrev; // Used for fullscreen restoration
		bool                     isVisible;
	};

	// =========================================================================
	// Module-Level Static State
	// =========================================================================
	static HINSTANCE g_hInstance = nullptr;
	static wchar_t   g_ClassName[256] = L"PlatformSurfaceClass";

	// Forward declaration of Win32 Window Procedure
	static LRESULT CALLBACK Win32WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	// Helper to convert UTF-8/ANSI title to wide string for Win32
	static void ToWideString(const char* src, wchar_t* dest, size_t destSize)
	{
		if (!src) { dest[0] = L'\0'; return; }
		MultiByteToWideChar(CP_UTF8, 0, src, -1, dest, static_cast<int>(destSize));
	}

	// =========================================================================
	// Subsystem Lifecycle
	// =========================================================================
	int32 Init(const char* AppName)
	{
		g_hInstance = GetModuleHandleW(nullptr);
		if (!g_hInstance) return eResult::Error;

		if (AppName)
		{
			ToWideString(AppName, g_ClassName, 256);
		}

		WNDCLASSEXW wc{};
		wc.cbSize = sizeof(WNDCLASSEXW);
		wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		wc.lpfnWndProc = Win32WndProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = g_hInstance;
		wc.hIcon = LoadIconW(nullptr, MAKEINTRESOURCEW(32512)); // IDI_APPLICATION
		wc.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512)); // IDC_ARROW
		wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
		wc.lpszMenuName = nullptr;
		wc.lpszClassName = g_ClassName;
		wc.hIconSm = LoadIconW(nullptr, MAKEINTRESOURCEW(32512));

		if (!RegisterClassExW(&wc))
		{
			DWORD err = GetLastError();
			if (err != ERROR_CLASS_ALREADY_EXISTS)
			{
				return eResult::Error;
			}
		}

		return eResult::OK;
	}

	void Fini()
	{
		UnregisterClassW(g_ClassName, g_hInstance);
	}

#if PLATFORM_DISKTOP_MANAGER

	// =========================================================================
	// Surface Creation & Destruction
	// =========================================================================
	int32 Create(const SurfaceCreateInfo& info, SurfaceHandle& outSurface)
	{
		outSurface.ptr = nullptr;

		SurfaceContext* ctx = static_cast<SurfaceContext*>(
			::VirtualAlloc(nullptr, sizeof(SurfaceContext), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE)
			);
		if (!ctx) return eResult::OutOfMemory;

		*ctx = SurfaceContext{};
		ctx->pCallback = info.pCallbacks;
		ctx->pUserData = info.pUserData;
		ctx->width = info.Width;
		ctx->height = info.Height;
		ctx->dpiScale = 1.0f;
		ctx->opacity = 1.0f;
		ctx->currentMode = eSurfaceMode::Default;
		ctx->isVisible = true;
		ctx->wpPrev.length = sizeof(WINDOWPLACEMENT);

		DWORD style = WS_OVERLAPPEDWINDOW;
		DWORD exStyle = WS_EX_APPWINDOW;

		if (info.Flags & eSurfaceFlags::Fullscreen)
		{
			style = WS_POPUP | WS_VISIBLE;
		}
		else
		{
			if (info.Flags & eSurfaceFlags::Resizable)  style |= WS_THICKFRAME | WS_MAXIMIZEBOX;
			if (info.Flags & eSurfaceFlags::Borderless) style = WS_POPUP | WS_VISIBLE;
			if (info.Flags & eSurfaceFlags::AlwaysOnTop) exStyle |= WS_EX_TOPMOST;
			if (info.Flags & eSurfaceFlags::Transparent) exStyle |= WS_EX_LAYERED;
		}

		wchar_t wTitle[256] = L"Game Engine Surface";
		if (info.title)
		{
			ToWideString(info.title, wTitle, 256);
		}

		RECT wr = { 0, 0, static_cast<LONG>(info.Width), static_cast<LONG>(info.Height) };
		AdjustWindowRectEx(&wr, style, FALSE, exStyle);

		int windowWidth = wr.right - wr.left;
		int windowHeight = wr.bottom - wr.top;

		HWND hwnd = CreateWindowExW(
			exStyle,
			g_ClassName,
			wTitle,
			style,
			CW_USEDEFAULT, CW_USEDEFAULT,
			windowWidth, windowHeight,
			nullptr, nullptr,
			g_hInstance,
			ctx
		);

		if (!hwnd)
		{
			::VirtualFree(ctx, 0, MEM_RELEASE);
			return eResult::WindowCreationFailed;
		}

		ctx->hwnd = hwnd;
		outSurface.ptr = ctx;

		if (!(info.Flags & eSurfaceFlags::Hidden))
		{
			ShowWindow(hwnd, SW_SHOW);
			UpdateWindow(hwnd);
		}

		return eResult::OK;
	}

	int32 Destroy(SurfaceHandle& hSurface)
	{
		if (!hSurface.ptr) return eResult::InvalidHandle;
		SurfaceContext* ctx = static_cast<SurfaceContext*>(hSurface.ptr);

		if (ctx->hwnd)
		{
			DestroyWindow(ctx->hwnd);
			ctx->hwnd = nullptr;
		}

		::VirtualFree(ctx, 0, MEM_RELEASE);
		hSurface.ptr = nullptr;
		return eResult::OK;
	}

#else

	int32 GetNativeSurface(SurfaceHandle& outSurface)
	{
		return eResult::Error;
	}

#endif

	// =========================================================================
	// Event Polling & Message Pump
	// =========================================================================
	int32 PollEvents()
	{
		MSG msg;
		while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		return eResult::OK;
	}

	// =========================================================================
	// DPI Scale
	// =========================================================================
	int32 SetDPIScale(SurfaceHandle hSurface, float fDPI)
	{
		if (!hSurface.ptr) return eResult::InvalidHandle;
		SurfaceContext* ctx = static_cast<SurfaceContext*>(hSurface.ptr);
		ctx->dpiScale = fDPI;
		return eResult::OK;
	}

	int32 GetDPIScale(SurfaceHandle hSurface, float& fDPI)
	{
		if (!hSurface.ptr) return eResult::InvalidHandle;
		SurfaceContext* ctx = static_cast<SurfaceContext*>(hSurface.ptr);
		fDPI = ctx->dpiScale;
		return eResult::OK;
	}

#if PLATFORM_DISKTOP_MANAGER

	// =========================================================================
	// Surface Modes & Geometry
	// =========================================================================
	int32 SetSurfaceMode(SurfaceHandle hSurface, eSurfaceMode Mode)
	{
		if (!hSurface.ptr) return eResult::InvalidHandle;
		SurfaceContext* ctx = static_cast<SurfaceContext*>(hSurface.ptr);
		ctx->currentMode = Mode;

		HWND hwnd = ctx->hwnd;
		if (Mode == eSurfaceMode::Minimized)
		{
			ShowWindow(hwnd, SW_MINIMIZE);
		}
		else if (Mode == eSurfaceMode::Maximized)
		{
			ShowWindow(hwnd, SW_MAXIMIZE);
		}
		else if (Mode == eSurfaceMode::Fullscreen || Mode == eSurfaceMode::FullscreenLessBorder || Mode == eSurfaceMode::FullscreenBorder)
		{
			MONITORINFO mi = { sizeof(MONITORINFO) };
			if (GetWindowPlacement(hwnd, &ctx->wpPrev) && GetMonitorInfoW(MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY), &mi))
			{
				SetWindowLongPtrW(hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
				SetWindowPos(hwnd, HWND_TOP,
					mi.rcMonitor.left, mi.rcMonitor.top,
					mi.rcMonitor.right - mi.rcMonitor.left,
					mi.rcMonitor.bottom - mi.rcMonitor.top,
					SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
			}
		}
		else
		{
			SetWindowLongPtrW(hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE);
			SetWindowPlacement(hwnd, &ctx->wpPrev);
			SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
				SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW);
		}

		return eResult::OK;
	}

	int32 GetSurfaceMode(SurfaceHandle hSurface, eSurfaceMode& Mode)
	{
		if (!hSurface.ptr) return eResult::InvalidHandle;
		SurfaceContext* ctx = static_cast<SurfaceContext*>(hSurface.ptr);
		Mode = ctx->currentMode;
		return eResult::OK;
	}

	int32 SetSize(SurfaceHandle hSurface, uint32 Width, uint32 Height)
	{
		if (!hSurface.ptr) return eResult::InvalidHandle;
		SurfaceContext* ctx = static_cast<SurfaceContext*>(hSurface.ptr);

		RECT rect = { 0, 0, static_cast<LONG>(Width), static_cast<LONG>(Height) };
		DWORD style = static_cast<DWORD>(GetWindowLongPtrW(ctx->hwnd, GWL_STYLE));
		DWORD exStyle = static_cast<DWORD>(GetWindowLongPtrW(ctx->hwnd, GWL_EXSTYLE));
		AdjustWindowRectEx(&rect, style, FALSE, exStyle);

		SetWindowPos(ctx->hwnd, nullptr, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOMOVE | SWP_NOZORDER);
		return eResult::OK;
	}

	int32 GetSize(SurfaceHandle hSurface, uint32& Width, uint32& Height)
	{
		if (!hSurface.ptr) return eResult::InvalidHandle;
		SurfaceContext* ctx = static_cast<SurfaceContext*>(hSurface.ptr);
		Width = ctx->width;
		Height = ctx->height;
		return eResult::OK;
	}

	void GetMaximumDeviceSize(uint32& Width, uint32& Height)
	{
		Width = static_cast<uint32>(GetSystemMetrics(SM_CXMAXIMIZED));
		Height = static_cast<uint32>(GetSystemMetrics(SM_CYMAXIMIZED));
	}

	void GetMinimumDeviceSize(uint32& Width, uint32& Height)
	{
		Width = static_cast<uint32>(GetSystemMetrics(SM_CXMIN));
		Height = static_cast<uint32>(GetSystemMetrics(SM_CYMIN));
	}

	int32 SetVisible(SurfaceHandle hSurface, bool bVisible)
	{
		if (!hSurface.ptr) return eResult::InvalidHandle;
		SurfaceContext* ctx = static_cast<SurfaceContext*>(hSurface.ptr);
		ctx->isVisible = bVisible;
		ShowWindow(ctx->hwnd, bVisible ? SW_SHOW : SW_HIDE);
		return eResult::OK;
	}

	int32 GetVisible(SurfaceHandle hSurface, bool& bVisible)
	{
		if (!hSurface.ptr) return eResult::InvalidHandle;
		SurfaceContext* ctx = static_cast<SurfaceContext*>(hSurface.ptr);
		bVisible = ctx->isVisible;
		return eResult::OK;
	}

	int32 SetTitle(SurfaceHandle hSurface, const char* Title)
	{
		if (!hSurface.ptr || !Title) return eResult::InvalidArgument;
		SurfaceContext* ctx = static_cast<SurfaceContext*>(hSurface.ptr);

		wchar_t wTitle[256];
		ToWideString(Title, wTitle, 256);
		SetWindowTextW(ctx->hwnd, wTitle);
		return eResult::OK;
	}

	int32 GetTitle(SurfaceHandle hSurface, const char*& Title)
	{
		if (!hSurface.ptr) return eResult::InvalidHandle;
		static char titleBuffer[256];
		SurfaceContext* ctx = static_cast<SurfaceContext*>(hSurface.ptr);

		wchar_t wTitle[256];
		GetWindowTextW(ctx->hwnd, wTitle, 256);
		WideCharToMultiByte(CP_UTF8, 0, wTitle, -1, titleBuffer, sizeof(titleBuffer), nullptr, nullptr);
		Title = titleBuffer;
		return eResult::OK;
	}

	int32 SetTransparency(SurfaceHandle hSurface, float Opacity)
	{
		if (!hSurface.ptr) return eResult::InvalidHandle;
		SurfaceContext* ctx = static_cast<SurfaceContext*>(hSurface.ptr);
		ctx->opacity = Opacity;

		LONG exStyle = GetWindowLongW(ctx->hwnd, GWL_EXSTYLE);
		SetWindowLongW(ctx->hwnd, GWL_EXSTYLE, exStyle | WS_EX_LAYERED);

		BYTE alpha = static_cast<BYTE>(Opacity * 255.0f);
		SetLayeredWindowAttributes(ctx->hwnd, 0, alpha, LWA_ALPHA);
		return eResult::OK;
	}

	int32 GetTransparency(SurfaceHandle hSurface, float& Opacity)
	{
		if (!hSurface.ptr) return eResult::InvalidHandle;
		SurfaceContext* ctx = static_cast<SurfaceContext*>(hSurface.ptr);
		Opacity = ctx->opacity;
		return eResult::OK;
	}

	int32 SetRect(SurfaceHandle hSurface, SurfaceRect Rect)
	{
		if (!hSurface.ptr) return eResult::InvalidHandle;
		SurfaceContext* ctx = static_cast<SurfaceContext*>(hSurface.ptr);
		SetWindowPos(ctx->hwnd, nullptr, Rect.Left, Rect.Top, Rect.Right - Rect.Left, Rect.Bottom - Rect.Top, SWP_NOZORDER);
		return eResult::OK;
	}

	int32 GetRect(SurfaceHandle hSurface, SurfaceRect& Rect)
	{
		if (!hSurface.ptr) return eResult::InvalidHandle;
		SurfaceContext* ctx = static_cast<SurfaceContext*>(hSurface.ptr);
		RECT rc;
		GetWindowRect(ctx->hwnd, &rc);
		Rect.Top = static_cast<uint32>(rc.top);
		Rect.Bottom = static_cast<uint32>(rc.bottom);
		Rect.Left = static_cast<uint32>(rc.left);
		Rect.Right = static_cast<uint32>(rc.right);
		return eResult::OK;
	}

#else

	int32 GetNativeSize(SurfaceHandle hSurface, uint32& Width, uint32& Height)
	{
		return eResult::Error;
	}

#endif

	int32 GetDeviceRect(SurfaceRect& Rect)
	{
		Rect.Left = 0;
		Rect.Top = 0;
		Rect.Right = static_cast<uint32>(GetSystemMetrics(SM_CXSCREEN));
		Rect.Bottom = static_cast<uint32>(GetSystemMetrics(SM_CYSCREEN));
		return eResult::OK;
	}

#if PLATFORM_DISKTOP_MANAGER

	int32 RequestAttention(SurfaceHandle hSurface)
	{
		if (!hSurface.ptr) return eResult::InvalidHandle;
		SurfaceContext* ctx = static_cast<SurfaceContext*>(hSurface.ptr);
		FLASHWINFO fwi{ sizeof(FLASHWINFO), ctx->hwnd, FLASHW_ALL | FLASHW_TIMERNOFG, 3, 0 };
		FlashWindowEx(&fwi);
		return eResult::OK;
	}

	int32 SetCursorMode(SurfaceHandle hSurface, eCursorMode Mode)
	{
		if (Mode == eCursorMode::Hidden)
		{
			ShowCursor(FALSE);
		}
		else
		{
			ShowCursor(TRUE);
		}
		return eResult::OK;
	}

	int32 CreateCursorShape(const uint8* rgbaPixels, uint32 Width, uint32 Height, int32 xHotspot, int32 yHotspot, CursorShapeHandle& hCursorShape)
	{
		hCursorShape.ptr = nullptr;
		if (!rgbaPixels) return eResult::InvalidArgument;

		ICONINFO ii{};
		ii.fIcon = FALSE;
		ii.xHotspot = xHotspot;
		ii.yHotspot = yHotspot;

		BITMAPV4HEADER bi{};
		bi.bV4Size = sizeof(BITMAPV4HEADER);
		bi.bV4Width = Width;
		bi.bV4Height = -static_cast<LONG>(Height);
		bi.bV4Planes = 1;
		bi.bV4BitCount = 32;
		bi.bV4V4Compression = BI_BITFIELDS;
		bi.bV4RedMask = 0x00FF0000;
		bi.bV4GreenMask = 0x00FF00;
		bi.bV4BlueMask = 0x0000FF;
		bi.bV4AlphaMask = 0xFF000000;

		void* ppBits = nullptr;
		HDC hdc = GetDC(nullptr);
		HBITMAP hBitmap = CreateDIBSection(hdc, reinterpret_cast<BITMAPINFO*>(&bi), DIB_RGB_COLORS, &ppBits, nullptr, 0);
		ReleaseDC(nullptr, hdc);

		if (!hBitmap || !ppBits) return eResult::OutOfMemory;
		::memcpy(ppBits, rgbaPixels, Width * Height * 4);

		ii.hbmColor = hBitmap;
		ii.hbmMask = CreateBitmap(Width, Height, 1, 1, nullptr);

		HICON hCursor = CreateIconIndirect(&ii);
		DeleteObject(ii.hbmMask);
		DeleteObject(hBitmap);

		hCursorShape.ptr = hCursor;
		return eResult::OK;
	}

	int32 DestroyCursorShape(CursorShapeHandle& hCursorShape)
	{
		if (!hCursorShape.ptr) return eResult::InvalidHandle;
		DestroyIcon(static_cast<HICON>(hCursorShape.ptr));
		hCursorShape.ptr = nullptr;
		return eResult::OK;
	}

	int32 SetCursorShape(SurfaceHandle hSurface, CursorShapeHandle hCursorShape)
	{
		if (!hCursorShape.ptr) return eResult::InvalidHandle;
		SetCursor(static_cast<HCURSOR>(hCursorShape.ptr));
		return eResult::OK;
	}

	int32 SetCursorShape(SurfaceHandle hSurface, eCursorShape Shape)
	{
		LPCWSTR id = MAKEINTRESOURCEW(32512); // IDC_ARROW
		switch (Shape)
		{
		case eCursorShape::TextInput: id = MAKEINTRESOURCEW(32513); break; // IDC_IBEAM
		case eCursorShape::Hand:      id = MAKEINTRESOURCEW(32649); break; // IDC_HAND
		case eCursorShape::ResizeEW:  id = MAKEINTRESOURCEW(32644); break; // IDC_SIZEWE
		case eCursorShape::ResizeNS:  id = MAKEINTRESOURCEW(32645); break; // IDC_SIZENS
		case eCursorShape::ResizeAll: id = MAKEINTRESOURCEW(32646); break; // IDC_SIZEALL
		default: break;
		}
		SetCursor(LoadCursorW(nullptr, id));
		return eResult::OK;
	}

	int32 CreateCustomIcon(const uint8* rgbaPixels, uint32 Width, uint32 Height, IconShapeHandle& hIconShape)
	{
		hIconShape.ptr = nullptr;
		if (!rgbaPixels) return eResult::InvalidArgument;

		ICONINFO ii{};
		ii.fIcon = TRUE;
		ii.xHotspot = 0;
		ii.yHotspot = 0;

		BITMAPV4HEADER bi{};
		bi.bV4Size = sizeof(BITMAPV4HEADER);
		bi.bV4Width = Width;
		bi.bV4Height = -static_cast<LONG>(Height);
		bi.bV4Planes = 1;
		bi.bV4BitCount = 32;
		bi.bV4V4Compression = BI_BITFIELDS;
		bi.bV4RedMask = 0x00FF0000;
		bi.bV4GreenMask = 0x00FF00;
		bi.bV4BlueMask = 0x0000FF;
		bi.bV4AlphaMask = 0xFF000000;

		void* ppBits = nullptr;
		HDC hdc = GetDC(nullptr);
		HBITMAP hBitmap = CreateDIBSection(hdc, reinterpret_cast<BITMAPINFO*>(&bi), DIB_RGB_COLORS, &ppBits, nullptr, 0);
		ReleaseDC(nullptr, hdc);

		if (!hBitmap || !ppBits) return eResult::OutOfMemory;
		::memcpy(ppBits, rgbaPixels, Width * Height * 4);

		ii.hbmColor = hBitmap;
		ii.hbmMask = CreateBitmap(Width, Height, 1, 1, nullptr);

		HICON hIcon = CreateIconIndirect(&ii);
		DeleteObject(ii.hbmMask);
		DeleteObject(hBitmap);

		hIconShape.ptr = hIcon;
		return eResult::OK;
	}

	int32 DestroyCustomIcon(IconShapeHandle& hIconShape)
	{
		if (!hIconShape.ptr) return eResult::InvalidHandle;
		DestroyIcon(static_cast<HICON>(hIconShape.ptr));
		hIconShape.ptr = nullptr;
		return eResult::OK;
	}

	int32 SetCustomIcon(SurfaceHandle hSurface, IconShapeHandle hIconShape)
	{
		if (!hSurface.ptr || !hIconShape.ptr) return eResult::InvalidHandle;
		SurfaceContext* ctx = static_cast<SurfaceContext*>(hSurface.ptr);
		SendMessageW(ctx->hwnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(hIconShape.ptr));
		SendMessageW(ctx->hwnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(hIconShape.ptr));
		return eResult::OK;
	}

	int32 SetClipboardText(const char* Text)
	{
		if (!Text) return eResult::InvalidArgument;
		if (!OpenClipboard(nullptr)) return eResult::Error;

		EmptyClipboard();
		size_t size = strlen(Text) + 1;
		HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, size);
		if (hMem)
		{
			void* pMem = GlobalLock(hMem);
			::memcpy(pMem, Text, size);
			GlobalUnlock(hMem);
			SetClipboardData(CF_TEXT, hMem);
		}
		CloseClipboard();
		return eResult::OK;
	}

	int32 GetClipboardText(const char*& Text)
	{
		static char clipBuffer[512];
		clipBuffer[0] = '\0';
		Text = clipBuffer;

		if (!OpenClipboard(nullptr)) return eResult::Error;
		HANDLE hData = GetClipboardData(CF_TEXT);
		if (hData)
		{
			char* pszText = static_cast<char*>(GlobalLock(hData));
			if (pszText)
			{
				strncpy_s(clipBuffer, sizeof(clipBuffer), pszText, _TRUNCATE);
				GlobalUnlock(hData);
			}
		}
		CloseClipboard();
		return eResult::OK;
	}

#endif

	using eKeyCode = Platform::Input::eKeyCode;
	// =========================================================================
	// Win32 Key Translation Helpers (Platform-Specific)
	// =========================================================================
	
	struct Win32KeyMapWrapper {
		eKeyCode data[256];
	};

	constexpr Win32KeyMapWrapper BuildWin32KeyMap()
	{
		Win32KeyMapWrapper map{};
		for (int i = 0; i < 256; ++i)
		{
			map.data[i] = eKeyCode::Unknown;
		}

		// Letters
		map.data['A'] = eKeyCode::A; map.data['B'] = eKeyCode::B; map.data['C'] = eKeyCode::C;
		map.data['D'] = eKeyCode::D; map.data['E'] = eKeyCode::E; map.data['F'] = eKeyCode::F;
		map.data['G'] = eKeyCode::G; map.data['H'] = eKeyCode::H; map.data['I'] = eKeyCode::I;
		map.data['J'] = eKeyCode::J; map.data['K'] = eKeyCode::K; map.data['L'] = eKeyCode::L;
		map.data['M'] = eKeyCode::M; map.data['N'] = eKeyCode::N; map.data['O'] = eKeyCode::O;
		map.data['P'] = eKeyCode::P; map.data['Q'] = eKeyCode::Q; map.data['R'] = eKeyCode::R;
		map.data['S'] = eKeyCode::S; map.data['T'] = eKeyCode::T; map.data['U'] = eKeyCode::U;
		map.data['V'] = eKeyCode::V; map.data['W'] = eKeyCode::W; map.data['X'] = eKeyCode::X;
		map.data['Y'] = eKeyCode::Y; map.data['Z'] = eKeyCode::Z;

		// Numbers (Top Row)
		map.data['0'] = eKeyCode::Num0; map.data['1'] = eKeyCode::Num1;
		map.data['2'] = eKeyCode::Num2; map.data['3'] = eKeyCode::Num3;
		map.data['4'] = eKeyCode::Num4; map.data['5'] = eKeyCode::Num5;
		map.data['6'] = eKeyCode::Num6; map.data['7'] = eKeyCode::Num7;
		map.data['8'] = eKeyCode::Num8; map.data['9'] = eKeyCode::Num9;

		// Function Keys
		map.data[VK_F1] = eKeyCode::F1;  map.data[VK_F2] = eKeyCode::F2;
		map.data[VK_F3] = eKeyCode::F3;  map.data[VK_F4] = eKeyCode::F4;
		map.data[VK_F5] = eKeyCode::F5;  map.data[VK_F6] = eKeyCode::F6;
		map.data[VK_F7] = eKeyCode::F7;  map.data[VK_F8] = eKeyCode::F8;
		map.data[VK_F9] = eKeyCode::F9;  map.data[VK_F10] = eKeyCode::F10;
		map.data[VK_F11] = eKeyCode::F11; map.data[VK_F12] = eKeyCode::F12;
		map.data[VK_F13] = eKeyCode::F13; map.data[VK_F14] = eKeyCode::F14;
		map.data[VK_F15] = eKeyCode::F15; map.data[VK_F16] = eKeyCode::F16;
		map.data[VK_F17] = eKeyCode::F17; map.data[VK_F18] = eKeyCode::F18;
		map.data[VK_F19] = eKeyCode::F19; map.data[VK_F20] = eKeyCode::F20;
		map.data[VK_F21] = eKeyCode::F21; map.data[VK_F22] = eKeyCode::F22;
		map.data[VK_F23] = eKeyCode::F23; map.data[VK_F24] = eKeyCode::F24;

		// Control & Navigation
		map.data[VK_ESCAPE] = eKeyCode::Escape;
		map.data[VK_RETURN] = eKeyCode::Enter;
		map.data[VK_SPACE] = eKeyCode::Space;
		map.data[VK_BACK] = eKeyCode::Backspace;
		map.data[VK_TAB] = eKeyCode::Tab;
		map.data[VK_CAPITAL] = eKeyCode::CapsLock;
		map.data[VK_SNAPSHOT] = eKeyCode::PrintScreen;
		map.data[VK_SCROLL] = eKeyCode::ScrollLock;
		map.data[VK_PAUSE] = eKeyCode::Pause;
		map.data[VK_INSERT] = eKeyCode::Insert;
		map.data[VK_DELETE] = eKeyCode::Delete;
		map.data[VK_HOME] = eKeyCode::Home;
		map.data[VK_END] = eKeyCode::End;
		map.data[VK_PRIOR] = eKeyCode::PageUp;
		map.data[VK_NEXT] = eKeyCode::PageDown;

		// Arrows
		map.data[VK_UP] = eKeyCode::Up;
		map.data[VK_DOWN] = eKeyCode::Down;
		map.data[VK_LEFT] = eKeyCode::Left;
		map.data[VK_RIGHT] = eKeyCode::Right;

		// Explicit Left/Right Modifiers & Windows Keys
		map.data[VK_LSHIFT] = eKeyCode::LeftShift;
		map.data[VK_RSHIFT] = eKeyCode::RightShift;
		map.data[VK_LCONTROL] = eKeyCode::LeftControl;
		map.data[VK_RCONTROL] = eKeyCode::RightControl;
		map.data[VK_LMENU] = eKeyCode::LeftAlt;
		map.data[VK_RMENU] = eKeyCode::RightAlt;
		map.data[VK_LWIN] = eKeyCode::LeftSuper;
		map.data[VK_RWIN] = eKeyCode::RightSuper;
		map.data[VK_APPS] = eKeyCode::Menu;

		// Symbols & Punctuation
		map.data[VK_OEM_3] = eKeyCode::Grave;
		map.data[VK_OEM_MINUS] = eKeyCode::Minus;
		map.data[VK_OEM_PLUS] = eKeyCode::Equals;
		map.data[VK_OEM_4] = eKeyCode::LeftBracket;
		map.data[VK_OEM_6] = eKeyCode::RightBracket;
		map.data[VK_OEM_5] = eKeyCode::Backslash;
		map.data[VK_OEM_1] = eKeyCode::Semicolon;
		map.data[VK_OEM_7] = eKeyCode::Apostrophe;
		map.data[VK_OEM_COMMA] = eKeyCode::Comma;
		map.data[VK_OEM_PERIOD] = eKeyCode::Period;
		map.data[VK_OEM_2] = eKeyCode::Slash;

		// Numpad
		map.data[VK_NUMPAD0] = eKeyCode::Numpad0;
		map.data[VK_NUMPAD1] = eKeyCode::Numpad1;
		map.data[VK_NUMPAD2] = eKeyCode::Numpad2;
		map.data[VK_NUMPAD3] = eKeyCode::Numpad3;
		map.data[VK_NUMPAD4] = eKeyCode::Numpad4;
		map.data[VK_NUMPAD5] = eKeyCode::Numpad5;
		map.data[VK_NUMPAD6] = eKeyCode::Numpad6;
		map.data[VK_NUMPAD7] = eKeyCode::Numpad7;
		map.data[VK_NUMPAD8] = eKeyCode::Numpad8;
		map.data[VK_NUMPAD9] = eKeyCode::Numpad9;
		map.data[VK_ADD] = eKeyCode::NumpadAdd;
		map.data[VK_SUBTRACT] = eKeyCode::NumpadSubtract;
		map.data[VK_MULTIPLY] = eKeyCode::NumpadMultiply;
		map.data[VK_DIVIDE] = eKeyCode::NumpadDivide;
		map.data[VK_DECIMAL] = eKeyCode::NumpadDecimal;
		map.data[VK_NUMLOCK] = eKeyCode::NumLock;

		return map;
	}

	// Evaluated entirely at compile time, stored in read-only binary section
	static constexpr Win32KeyMapWrapper G_Win32KeyMap = BuildWin32KeyMap();

	static eKeyCode Win32ToKeyCode(uint32 nativeKey, uint32 lParam)
	{
		uint32 resolvedKey = nativeKey;
		uint32 scancode = (lParam & 0x00FF0000) >> 16;
		bool isExtended = (lParam & 0x01000000) != 0;

		switch (nativeKey)
		{
		case VK_SHIFT:
			resolvedKey = MapVirtualKeyW(scancode, MAPVK_VSC_TO_VK_EX);
			break;
		case VK_CONTROL:
			resolvedKey = isExtended ? VK_RCONTROL : VK_LCONTROL;
			break;
		case VK_MENU:
			resolvedKey = isExtended ? VK_RMENU : VK_LMENU;
			break;
		default:
			break;
		}

		return (resolvedKey < 256) ? G_Win32KeyMap.data[resolvedKey] : eKeyCode::Unknown;
	}

	// =========================================================================
	// Win32 Window Procedure & Event Routing
	// =========================================================================
	static LRESULT CALLBACK Win32WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		SurfaceContext* ctx = nullptr;
		
		if (msg == WM_CREATE)
		{
			CREATESTRUCTW* pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
			ctx = static_cast<SurfaceContext*>(pCreate->lpCreateParams);
			SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(ctx));
		}
		else
		{
			ctx = reinterpret_cast<SurfaceContext*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
		}

		if (ctx && ctx->pCallback)
		{
			SurfaceEvent ev{};
			ev.Handle.ptr = ctx;
			bool dispatch = true;

			switch (msg)
			{
			case WM_CLOSE:
				ev.Type = eSurfaceEventType::WindowClose;
				break;

			case WM_SIZE:
			{
				ctx->width = LOWORD(lParam);
				ctx->height = HIWORD(lParam);
				ev.Type = eSurfaceEventType::WindowResize;
				ev.Data.Resize.Width = ctx->width;
				ev.Data.Resize.Height = ctx->height;
			}
			break;

			case WM_SETFOCUS:
				ev.Type = eSurfaceEventType::WindowFocus;
				ev.Data.Focus.bFocused = true;
				break;

			case WM_KILLFOCUS:
				ev.Type = eSurfaceEventType::WindowFocus;
				ev.Data.Focus.bFocused = false;
				break;

			case WM_KEYDOWN:
			case WM_SYSKEYDOWN:
				ev.Type = eSurfaceEventType::KeyDown;
				ev.Data.Key.KeyCode = static_cast<uint32>(Win32ToKeyCode(static_cast<uint32>(wParam), static_cast<uint32>(lParam)));
				ev.Data.Key.bRepeat = (lParam & (1 << 30)) != 0;
				break;

			case WM_KEYUP:
			case WM_SYSKEYUP:
				ev.Type = eSurfaceEventType::KeyUp;
				ev.Data.Key.KeyCode = static_cast<uint32>(Win32ToKeyCode(static_cast<uint32>(wParam), static_cast<uint32>(lParam)));
				ev.Data.Key.bRepeat = false;
				break;

			case WM_CHAR:
				ev.Type = eSurfaceEventType::TextChar;
				ev.Data.Text.CharCode = static_cast<uint32>(wParam);
				break;

			case WM_MOUSEMOVE:
				ev.Type = eSurfaceEventType::MouseMove;
				ev.Data.MouseMove.X = GET_X_LPARAM(lParam);
				ev.Data.MouseMove.Y = GET_Y_LPARAM(lParam);
				break;

			case WM_LBUTTONDOWN:
				ev.Type = eSurfaceEventType::MouseButtonDown;
				ev.Data.MouseButton.X = GET_X_LPARAM(lParam);
				ev.Data.MouseButton.Y = GET_Y_LPARAM(lParam);
				ev.Data.MouseButton.KeyCode = 0;
				break;

			case WM_LBUTTONUP:
				ev.Type = eSurfaceEventType::MouseButtonUp;
				ev.Data.MouseButton.X = GET_X_LPARAM(lParam);
				ev.Data.MouseButton.Y = GET_Y_LPARAM(lParam);
				ev.Data.MouseButton.KeyCode = 0;
				break;

			case WM_MOUSEWHEEL:
				ev.Type = eSurfaceEventType::MouseScroll;
				ev.Data.MouseScroll.deltaX = 0.0f;
				ev.Data.MouseScroll.deltaY = static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam)) / WHEEL_DELTA;
				break;

			default:
				dispatch = false;
				break;
			}

			if (dispatch)
			{
				ctx->pCallback(ev, ctx->pUserData);
			}
		}

		return DefWindowProcW(hwnd, msg, wParam, lParam);
	}
}