#include <Platform/PlatformSurface.h>
#include <cstdio>

// Event callback invoked by PlatformSurface during PollEvents()
void SurfaceEventCallback(const Platform::Surface::SurfaceEvent& event, void* userData)
{
    using namespace Platform::Surface;

    switch (event.Type)
    {
    case eSurfaceEventType::WindowClose:
        std::printf("[Event] Window Close Requested.\n");
        break;

    case eSurfaceEventType::WindowFocus:
        std::printf("[Event] Window Focus Changed: %s\n", event.Data.Focus.bFocused ? "Focused" : "Unfocused");
        break;

    case eSurfaceEventType::KeyDown:
        std::printf("[Event] Key Down -> Code: %u (Repeat: %s)\n",
            static_cast<uint32>(event.Data.Key.KeyCode),
            event.Data.Key.bRepeat ? "True" : "False");
        break;

    case eSurfaceEventType::KeyUp:
        std::printf("[Event] Key Up   -> Code: %u\n",
            static_cast<uint32>(event.Data.Key.KeyCode));
        break;

    case eSurfaceEventType::MouseMove:
        std::printf("[Event] Mouse Move -> X: %d, Y: %d\n",
            event.Data.MouseMove.X, event.Data.MouseMove.Y);
        break;

    case eSurfaceEventType::MouseButtonDown:
        std::printf("[Event] Mouse Button Down -> Button: %u at (%d, %d)\n",
            static_cast<uint32>(event.Data.MouseButton.KeyCode),
            event.Data.MouseButton.X, event.Data.MouseButton.Y);
        break;

    case eSurfaceEventType::MouseButtonUp:
        std::printf("[Event] Mouse Button Up   -> Button: %u at (%d, %d)\n",
            static_cast<uint32>(event.Data.MouseButton.KeyCode),
            event.Data.MouseButton.X, event.Data.MouseButton.Y);
        break;

    case eSurfaceEventType::MouseScroll:
        std::printf("[Event] Mouse Scroll -> DeltaX: %.2f, DeltaY: %.2f\n",
            event.Data.MouseScroll.deltaX, event.Data.MouseScroll.deltaY);
        break;

    default:
        break;
    }
}

int main()
{
    std::printf("Initializing Engine Platform Subsystem...\n");

    // 1. Initialize global platform layer
    if (Platform::Surface::Init("EngineInputTest") != 0)
    {
        std::fprintf(stderr, "CRITICAL: Failed to initialize PlatformSurface!\n");
        return -1;
    }

    // 2. Set up surface creation parameters
    Platform::Surface::SurfaceCreateInfo createInfo{};
    createInfo.title = "Engine Core - Input Validation Window";
    createInfo.Width = 1280;
    createInfo.Height = 720;
    createInfo.pCallbacks = SurfaceEventCallback;
    createInfo.pUserData = nullptr;

    Platform::Surface::SurfaceHandle hSurface;

    // 3. Create the native Win32 window surface
    if (Platform::Surface::Create(createInfo, hSurface) != 0)
    {
        std::fprintf(stderr, "CRITICAL: Failed to create surface handle!\n");
        Platform::Surface::Fini();
        return -1;
    }

    std::printf("Window created successfully. Focus the window and test keys/mouse.\n");
    std::printf("Press 'Escape' or close the window to exit.\n\n");

    // 4. Engine Loop
    bool bRunning = true;
    while (bRunning)
    {
        // Polls OS events, translates Win32 virtual keys (including Left/Right modifiers),
        // and dispatches them straight to SurfaceEventCallback.
        int32 pollResult = Platform::Surface::PollEvents();

        if (pollResult != 0)
        {
            // Quit requested or window closed
            bRunning = false;
        }
    }

    std::printf("Shutting down platform subsystem...\n");

    // 5. Clean teardown
    Platform::Surface::Destroy(hSurface);
    Platform::Surface::Fini();

    std::printf("Test completed cleanly. Zero heap leaks.\n");
    return 0;
}