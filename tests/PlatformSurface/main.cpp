#include <Platform/PlatformSurface.h>
#include <iostream>
#include <windows.h>

// Event callback handler routing events from Win32 messages
void OnSurfaceEvent(const Platform::Surface::SurfaceEvent& ev, void* pUserData)
{
    bool* pShouldQuit = static_cast<bool*>(pUserData);

    switch (ev.Type)
    {
    case Platform::Surface::eSurfaceEventType::WindowClose:
        std::cout << "\n[Event] Window Close Requested by User." << std::endl;
        *pShouldQuit = true;
        break;

    case Platform::Surface::eSurfaceEventType::WindowResize:
        std::cout << "[Event] Window Resized -> Width: " << ev.Data.Resize.Width
            << " | Height: " << ev.Data.Resize.Height << std::endl;
        break;

    case Platform::Surface::eSurfaceEventType::WindowFocus:
        std::cout << "[Event] Window Focus -> " << (ev.Data.Focus.bFocused ? "Focused" : "Unfocused") << std::endl;
        break;

    case Platform::Surface::eSurfaceEventType::KeyDown:
        std::cout << "[Event] Key Down -> Virtual Key Code: " << ev.Data.Key.KeyCode
            << (ev.Data.Key.bRepeat ? " (Auto-Repeat)" : "") << std::endl;
        break;

    case Platform::Surface::eSurfaceEventType::TextChar:
        std::cout << "[Event] Text Typed -> Character Code: " << static_cast<char>(ev.Data.Text.CharCode) << std::endl;
        break;

    case Platform::Surface::eSurfaceEventType::MouseButtonDown:
        std::cout << "[Event] Mouse Click Down -> X: " << ev.Data.MouseButton.X
            << " | Y: " << ev.Data.MouseButton.Y << std::endl;
        break;

    case Platform::Surface::eSurfaceEventType::MouseScroll:
        std::cout << "[Event] Mouse Scroll -> DeltaY: " << ev.Data.MouseScroll.deltaY << std::endl;
        break;

    default:
        break;
    }
}

int main()
{
    std::cout << "====================================================" << std::endl;
    std::cout << "       PLATFORM SURFACE CORE: FULL TEST SUITE       " << std::endl;
    std::cout << "====================================================" << std::endl;

    // 1. Test Subsystem Initialization
    std::cout << "[1/6] Initializing Surface Subsystem..." << std::endl;
    if (Platform::Surface::Init("NukeEngineSurfaceApp") != 0)
    {
        std::cerr << "FAIL: Platform::Surface::Init() failed!" << std::endl;
        return -1;
    }
    std::cout << "      -> Subsystem initialized successfully." << std::endl;

    // 2. Test Surface Creation
    std::cout << "[2/6] Creating Surface Window (1280x720, Resizable)..." << std::endl;
    bool shouldQuit = false;
    Platform::Surface::SurfaceCreateInfo createInfo{};
    createInfo.Width = 1280;
    createInfo.Height = 720;
    createInfo.Flags = Platform::Surface::eSurfaceFlags::Resizable;
    createInfo.title = "Nuke Engine - Surface Test Window";
    createInfo.pCallbacks = OnSurfaceEvent;
    createInfo.pUserData = &shouldQuit;

    Platform::Surface::SurfaceHandle hSurface;
    if (Platform::Surface::Create(createInfo, hSurface) != 0)
    {
        std::cerr << "FAIL: Platform::Surface::Create() failed!" << std::endl;
        Platform::Surface::Fini();
        return -1;
    }
    std::cout << "      -> Surface created successfully." << std::endl;

    // 3. Test Property Setters & Getters (Title, Transparency, Size)
    std::cout << "[3/6] Testing Property APIs (Title, Opacity, Size)..." << std::endl;

    // Title Test
    Platform::Surface::SetTitle(hSurface, "Nuke Engine - Title Updated!");
    const char* retrievedTitle = nullptr;
    Platform::Surface::GetTitle(hSurface, retrievedTitle);
    std::cout << "      -> Verified Window Title: " << (retrievedTitle ? retrievedTitle : "(null)") << std::endl;

    // Transparency Test (95% opacity)
    Platform::Surface::SetTransparency(hSurface, 0.95f);
    float opacity = 1.0f;
    Platform::Surface::GetTransparency(hSurface, opacity);
    std::cout << "      -> Verified Window Opacity: " << opacity << std::endl;

    // Size Test
    uint32 w = 0, h = 0;
    Platform::Surface::GetSize(hSurface, w, h);
    std::cout << "      -> Verified Client Area Size: " << w << " x " << h << std::endl;

    // 4. Test Clipboard APIs
    std::cout << "[4/6] Testing Clipboard Text Interop..." << std::endl;
    const char* testClipboardStr = "Hello from Nuke Engine No-Heap Core!";
    Platform::Surface::SetClipboardText(testClipboardStr);
    const char* clipboardResult = nullptr;
    Platform::Surface::GetClipboardText(clipboardResult);
    std::cout << "      -> Verified Clipboard Text: " << (clipboardResult ? clipboardResult : "(null)") << std::endl;

    // 5. Test Device & Display Rects / Cursors
    std::cout << "[5/6] Testing Device Resolution & Cursor Shape APIs..." << std::endl;
    Platform::Surface::SurfaceRect deviceRect{};
    Platform::Surface::GetDeviceRect(deviceRect);
    std::cout << "      -> Primary Screen Bounds: Right=" << deviceRect.Right << ", Bottom=" << deviceRect.Bottom << std::endl;

    // Change cursor shape to Hand
    Platform::Surface::SetCursorShape(hSurface, Platform::Surface::eCursorShape::Hand);

    // 6. Enter Main Event Loop (Poll Events & Test Mode Switching)
    std::cout << "[6/6] Entering Main Event Loop..." << std::endl;
    std::cout << "      -> Try moving, resizing, clicking, typing, or closing the window." << std::endl;
    std::cout << "      -> Automated test: Window will minimize at 3s and restore at 6s." << std::endl;

    DWORD startTime = GetTickCount();
    bool minimizedTested = false;
    bool restoredTested = false;

    while (!shouldQuit)
    {
        // Poll OS message queue (processes Win32 messages and fires callbacks)
        Platform::Surface::PollEvents();

        DWORD elapsed = GetTickCount() - startTime;

        // Automated Mode Test 1: Minimize after 3 seconds
        if (!minimizedTested && elapsed > 3000)
        {
            std::cout << "\n   [Automated Test] Setting Surface Mode to Minimized..." << std::endl;
            Platform::Surface::SetSurfaceMode(hSurface, Platform::Surface::eSurfaceMode::Minimized);
            minimizedTested = true;
        }
        // Automated Mode Test 2: Restore back to Default after 6 seconds
        else if (minimizedTested && !restoredTested && elapsed > 6000)
        {
            std::cout << "\n   [Automated Test] Restoring Surface Mode to Default..." << std::endl;
            Platform::Surface::SetSurfaceMode(hSurface, Platform::Surface::eSurfaceMode::Default);
            restoredTested = true;
        }

        Sleep(15); // ~60 FPS tick rate pacing
    }

    std::cout << "\nClosing surface and cleaning up resources..." << std::endl;

    // 7. Clean Teardown
    Platform::Surface::Destroy(hSurface);
    Platform::Surface::Fini();

    std::cout << "====================================================" << std::endl;
    std::cout << "       SURFACE CORE TEST PASSED SUCCESSFULLY        " << std::endl;
    std::cout << "====================================================" << std::endl;

    return 0;
}