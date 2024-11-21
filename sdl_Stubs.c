#include <stdlib.h>
#include "render/SDL_sysrender.h"

static bool stub_CreateRenderer(SDL_Renderer* renderer, SDL_Window* window, SDL_PropertiesID create_props)
{
    return false;
}

SDL_RenderDriver D3D11_RenderDriver = {
    stub_CreateRenderer, "direct3d11"
};

SDL_RenderDriver D3D12_RenderDriver = {
    stub_CreateRenderer, "direct3d11"
};

SDL_RenderDriver D3D_RenderDriver = {
    stub_CreateRenderer, "direct3d"
};

SDL_RenderDriver GLES2_RenderDriver = {
    stub_CreateRenderer, "opengles2"
};
