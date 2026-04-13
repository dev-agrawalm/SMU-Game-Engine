#pragma once

#include<d3d11.h>
#include<dxgi.h>

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "dxgi.lib")

#define D3X_SAFE_RELEASE(x) if(x != nullptr){ x->Release(); x = nullptr;}