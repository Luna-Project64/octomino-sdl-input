#include <stdlib.h>

// Some functions are still getting used from SDL even though I have explicitly disabled them.
// So, I have to provide stubs for them.
void* SDL_EGL_GetProcAddress()
{
	abort();
}

int SDL_EGL_GetSwapInterval()
{
	abort();
}

int SDL_EGL_SetSwapInterval()
{
	abort();
}

void SDL_EGL_UnloadLibrary()
{
	abort();
}

void* WIN_GLES_CreateContext()
{
	abort();
}

void WIN_GLES_DeleteContext()
{
	abort();
}

int WIN_GLES_LoadLibrary()
{
	abort();
}

int WIN_GLES_MakeCurrent()
{
	abort();
}

int WIN_GLES_SetupWindow()
{
	abort();
}

int WIN_GLES_SwapWindow()
{
	abort();
}

int WIN_Vulkan_LoadLibrary()
{
	abort();
}

void WIN_Vulkan_UnloadLibrary()
{
	abort();
}

int WIN_Vulkan_GetInstanceExtensions()
{
	abort();
}

int WIN_Vulkan_CreateSurface()
{
	abort();
}
