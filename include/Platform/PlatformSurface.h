#pragma once
#include <Platform/PlatformCore.h>
#include <Platform/PlatformInput.h>


#define PLATFORM_SURFACE_API API_DECL

DEFINE_HANDLE(Surface);
DEFINE_HANDLE(CursorShape);
DEFINE_HANDLE(IconShape);

namespace eResult {
	enum : iResult
	{
		PlatformSurfaceResult = PLATFORM_SURFACE_RESULT_BIT,
		SurfaceInitFailed,
		SurfaceClassRegistrationFailed,
		SurfaceCreationFailed,
		SurfaceAlreadyInitialized,
		CursorCreationFailed,
		IconCreationFailed
	};
};

enum class eSurfaceLifecycle : uint32
{
	Created,
	Destroyed,
	QuitRequested
};

enum class eSurfaceState : uint32
{
	Normal = 0,
	Minimized,
	Maximized,
	Fullscreen
};

enum class eSurfaceMode : uint32
{
	Defualt = 0,
	Minimized,
	Maximized,
	LessBorder,
	FullscreenLessBorder,
	FullscreenBorder,
};

enum class eCursorMode : uint32
{
	Normal = 0,
	Hidden,
	Locked
};

enum class eCursorShape : uint32
{
	Arrow = 0,
	TextInput,
	Hand,
	ResizeEW,
	ResizeNS,
	ResizeAll
};

enum class eSurfaceEventType : uint32 {
	Unknown = 0,
	WindowClose,
	WindowResize,
	WindowFocus,
	DPIChange,
	KeyDown,
	KeyUp,
	TextChar,
	MouseMove,
	MouseButtonDown,
	MouseButtonUp,
	MouseScroll,
	DropFiles
};

struct SurfaceEvent {
	eSurfaceEventType	Type;
	SurfaceHandle		Handle;

	union EventData {
		struct { uint32 Width; uint32 Height; } Resize;
		struct { bool bFocused; } Focus;
		struct { float Scale; } DPI;
		struct { uint32 KeyCode; bool bRepeat; } Key;
		struct { uint32 CharCode; } Text;
		struct { int32 X; int32 Y; } MouseMove;
		struct { int32 X; int32 Y; uint32 KeyCode; } MouseButton;
		struct { float deltaX; float deltaY; } MouseScroll;
		struct { uint32 Count; c_str* Paths; } Drop;
	} Data;
};

using PFN_SurfaceEventCallback = void (*)(const SurfaceEvent& Event, void* UserData);

enum eSurfaceFlags : uint32
{
	None = 0,
	Fullscreen  = 1 << 0,
	HighDPI     = 1 << 1,	AlwaysOnTop = 1 << 2,

#if PLATFORM_DISKTOP_MANAGER
	Resizable   = 1 << 3,
	Borderless  = 1 << 4,
	Hidden      = 1 << 5,	Transparent = 1 << 6,
#endif

};

struct SurfaceRect
{
	uint32 Top, Bottom, Left, Right;
};

struct SurfaceCreateInfo
{
#if PLATFORM_DISKTOP_MANAGER
	uint32 Width	= 1280;
	uint32 Height	= 720;
	uint32 Flags	= eSurfaceFlags::None;
	const char* title = "Game Engine Surface";
#endif
	PFN_SurfaceEventCallback pCallbacks = nullptr;
	void* pUserData = nullptr;
};

struct PLATFORM_SURFACE_API PlatformSurface
{
	PlatformSurface() = delete;

public:

	static iResult Init(const char* AppName);

	static void Fini();

#if PLATFORM_DISKTOP_MANAGER
	
	static iResult Create(const SurfaceCreateInfo& info, SurfaceHandle& outSurface);

	static iResult Destroy(SurfaceHandle& hSurface);

#else
	
	static iResult GetNativeSurface(SurfaceHandle& outSurface);

#endif

	static iResult PollEvents(); //per - loop tick

	static iResult SetDPIScale(SurfaceHandle hSurface, float fDPI);

	static iResult GetDPIScale(SurfaceHandle hSurface, float& fDPI);

#if PLATFORM_DISKTOP_MANAGER

	static iResult SetSurfaceMode(SurfaceHandle hSurface, eSurfaceMode Mode);

	static iResult GetSurfaceMode(SurfaceHandle hSurface, eSurfaceMode& Mode);

	static iResult SetSize(SurfaceHandle hSurface, uint32 Width	, uint32 Height);
	
	static iResult GetSize(SurfaceHandle hSurface, uint32& Width, uint32& Height);

	static void GetMaximumDeviceSize(uint32& Width, uint32& Height);

	static void GetMinimumDeviceSize(uint32& Width, uint32& Height);

	static iResult SetVisible(SurfaceHandle hSurface, bool	bVisible);

	static iResult GetVisible(SurfaceHandle hSurface, bool& bVisible);

	static iResult SetTitle(SurfaceHandle hSurface, c_str Title);

	static iResult GetTitle(SurfaceHandle hSurface, c_str& Title);

	static iResult SetTransparency(SurfaceHandle hSurface, float Opacity);

	static iResult GetTransparency(SurfaceHandle hSurface, float& Opacity);

	static iResult SetRect(SurfaceHandle hSurface, SurfaceRect Rect);

	static iResult GetRect(SurfaceHandle hSurface, SurfaceRect& Rect);

#else

	static iResult GetNativeSize(SurfaceHandle hSurface, uint32& Width, uint32& Height);

#endif

	static iResult GetDeviceRect(SurfaceRect& Rect);

#if PLATFORM_DISKTOP_MANAGER

	static iResult RequestAttention(SurfaceHandle hSurface);

	static iResult SetCursorMode(SurfaceHandle hSurface, eCursorMode Mode);

	static iResult CreateCursorShape(const uint8* rgbaPixels, uint32 Width, uint32 Height, int32 xHotspot, int32 yHotspot, CursorShapeHandle& hCursorShape);

	static iResult DestroyCursorShape(CursorShapeHandle& hCursorShape);

	static iResult SetCursorShape(SurfaceHandle hSurface, CursorShapeHandle hCursorShape);

	static iResult SetCursorShape(SurfaceHandle hSurface, eCursorShape Shape);

	static iResult CreateCustomIcon(const uint8* rgbaPixels, uint32 Width, uint32 Height, IconShapeHandle& hIconShape);

	static iResult DestroyCustomIcon(IconShapeHandle& hIconShape);

	static iResult SetCustomIcon(SurfaceHandle hSurface, IconShapeHandle hIconShape);

	static iResult SetClipboardText(c_str Text);

	static iResult GetClipboardText(c_str& Text);

#endif

};
