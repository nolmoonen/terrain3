#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include "app.h"

/// Windows entry point.
int APIENTRY WinMain(
        HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int cmdShow)
{ return run(); }