// Standard.
#include <iostream>

// Custom.
#include "Application.h"

// OS.
#if defined(WIN32)
#include <Windows.h>
#include <crtdbg.h>
#endif

int main() {
    // Enable run-time memory check for debug builds (on Windows).
#if defined(WIN32) && defined(DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#elif defined(WIN32) && !defined(DEBUG)
    OutputDebugStringA("Using release build configuration, memory checks are disabled.");
#endif
    
    // Run app.
    Application app;
    try {
        app.run();
    } catch (const std::exception& exception) {
        std::cerr << exception.what() << std::endl;
        return 1;
    }

    return 0;
}
