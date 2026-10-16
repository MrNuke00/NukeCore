#pragma once
#include <Platform/PlatformCore.h>

namespace Platform::Surface {

    enum eResult : int32
    {
        PLATFORM_RESULT(PLATFORM_SURFACE_RESULT_BIT),
        WindowCreationFailed,
        DeviceNotFound,
        NotInitialized,
        InvalidHandle,
        OutOfMemory
    };

    DEFINE_HANDLE(Surface);
    DEFINE_HANDLE(CursorShape);
    DEFINE_HANDLE(IconShape);

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
        Default = 0,
        Minimized,
        Maximized,
        Fullscreen,
        LessBorder,
        FullscreenLessBorder,
        FullscreenBorder
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

    enum class eSurfaceEventType : uint32
    {
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

    struct SurfaceEvent
    {
        eSurfaceEventType   Type;
        SurfaceHandle       Handle;

        union EventData
        {
            struct { uint32 Width; uint32 Height; } Resize;
            struct { bool bFocused; } Focus;
            struct { float Scale; } DPI;
            struct { uint32 KeyCode; bool bRepeat; } Key;
            struct { uint32 CharCode; } Text;
            struct { int32 X; int32 Y; } MouseMove;
            struct { int32 X; int32 Y; uint32 KeyCode; } MouseButton;
            struct { float deltaX; float deltaY; } MouseScroll;
            struct { uint32 Count; const char** Paths; } Drop;
        } Data;
    };

    using PFN_SurfaceEventCallback = void (*)(const SurfaceEvent& Event, void* UserData);

    enum eSurfaceFlags : uint32
    {
        None = 0,
        Fullscreen = 1 << 0,
        HighDPI = 1 << 1,
        AlwaysOnTop = 1 << 2,

#if PLATFORM_DISKTOP_MANAGER
        Resizable = 1 << 3,
        Borderless = 1 << 4,
        Hidden = 1 << 5,
        Transparent = 1 << 6,
#endif
    };

    struct SurfaceRect
    {
        uint32 Top, Bottom, Left, Right;
    };

    struct SurfaceCreateInfo
    {
#if PLATFORM_DISKTOP_MANAGER
        uint32 Width = 1280;
        uint32 Height = 720;
        uint32 Flags = eSurfaceFlags::None;
        const char* title = "Game Engine Surface";
#endif
        PFN_SurfaceEventCallback pCallbacks = nullptr;
        void* pUserData = nullptr;
    };


   int32 Init(const char* AppName);

   void Fini();

#if PLATFORM_DISKTOP_MANAGER

   int32 Create(const SurfaceCreateInfo& info, SurfaceHandle& hSurface);

   int32 Destroy(SurfaceHandle& hSurface);

#else

   int32 GetNativeSurface(SurfaceHandle& hSurface);

#endif

   int32 PollEvents(); // Per-loop tick

   int32 SetDPIScale(SurfaceHandle hSurface, float fDPI);

   int32 GetDPIScale(SurfaceHandle hSurface, float& fDPI);

#if PLATFORM_DISKTOP_MANAGER

   int32 SetSurfaceMode(SurfaceHandle hSurface, eSurfaceMode Mode);

   int32 GetSurfaceMode(SurfaceHandle hSurface, eSurfaceMode& Mode);

   int32 SetSize(SurfaceHandle hSurface, uint32 Width, uint32 Height);

   int32 GetSize(SurfaceHandle hSurface, uint32& Width, uint32& Height);

   void GetMaximumDeviceSize(uint32& Width, uint32& Height);

   void GetMinimumDeviceSize(uint32& Width, uint32& Height);

   int32 SetVisible(SurfaceHandle hSurface, bool bVisible);

   int32 GetVisible(SurfaceHandle hSurface, bool& bVisible);

   int32 SetTitle(SurfaceHandle hSurface, const char* Title);

   int32 GetTitle(SurfaceHandle hSurface, const char*& Title);

   int32 SetTransparency(SurfaceHandle hSurface, float Opacity);

   int32 GetTransparency(SurfaceHandle hSurface, float& Opacity);

   int32 SetRect(SurfaceHandle hSurface, SurfaceRect Rect);

   int32 GetRect(SurfaceHandle hSurface, SurfaceRect& Rect);

#else

   int32 GetNativeSize(SurfaceHandle hSurface, uint32& Width, uint32& Height);

#endif

   int32 GetDeviceRect(SurfaceRect& Rect);

#if PLATFORM_DISKTOP_MANAGER

   int32 RequestAttention(SurfaceHandle hSurface);

   int32 SetCursorMode(SurfaceHandle hSurface, eCursorMode Mode);

   int32 CreateCursorShape(const uint8* rgbaPixels, uint32 Width, uint32 Height, int32 xHotspot, int32 yHotspot, CursorShapeHandle& hCursorShape);

   int32 DestroyCursorShape(CursorShapeHandle& hCursorShape);

   int32 SetCursorShape(SurfaceHandle hSurface, CursorShapeHandle hCursorShape);

   int32 SetCursorShape(SurfaceHandle hSurface, eCursorShape Shape);

   int32 CreateCustomIcon(const uint8* rgbaPixels, uint32 Width, uint32 Height, IconShapeHandle& hIconShape);

   int32 DestroyCustomIcon(IconShapeHandle& hIconShape);

   int32 SetCustomIcon(SurfaceHandle hSurface, IconShapeHandle hIconShape);

   int32 SetClipboardText(const char* Text);

   int32 GetClipboardText(const char*& Text);

#endif

}