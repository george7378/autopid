#ifndef DXCORE_H
#define DXCORE_H

//D3D and Windows
unsigned WIDTH = 640;
unsigned HEIGHT = 480;
HWND hwnd = NULL;
LPDIRECT3D9 d3d = NULL;
LPDIRECT3DDEVICE9 d3ddev = NULL;
D3DPRESENT_PARAMETERS d3dpp;

//Text
LPD3DXFONT hudFont = NULL;

//Surfaces
LPDIRECT3DSURFACE9 frameCaptureSurface = NULL;

template <class T> void SAFE_RELEASE(T **ppT)
{
    if (*ppT)
	{
        (*ppT)->Release();
        *ppT = NULL;
    }
}

#endif
