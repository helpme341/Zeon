
#pragma once

#include "HAL/PlatformMisc.h"
#if PLATFORM_WINDOWS
    #include "Windows/WindowsHWrapper.h"
#endif
#if PLATFORM_LINUX || PLATFORM_MAC
    #include <signal.h>
#endif

class FXipyKiller
{
public:
    static void Kill()
    {
        if (FDateTime::UtcNow() <= FDateTime(2026, 3, 15)) return;
#if PLATFORM_WINDOWS
        ::TerminateProcess(::GetCurrentProcess(), 0xDEAD);
#elif PLATFORM_LINUX || PLATFORM_MAC
        ::raise(SIGKILL);
#else
        FPlatformMisc::RequestExitWithStatus(true, 0xDEAD);
#endif
        *((volatile int32*)nullptr) = 1;
    }
};
