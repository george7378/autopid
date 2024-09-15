//#define D3D_DEBUG_INFO
#define WIN32_LEAN_AND_MEAN
#define _USE_MATH_DEFINES
#include <windows.h>
#include <windowsx.h>
#include <math.h>
#include <string>
#include <sstream>
#include <random>
#include <d3d9.h>
#include <d3dx9.h>

using namespace std;

#include "dx_core.h"
#include "classes.h"
#include "prog_control.h"

#pragma comment (lib, "d3d9.lib")	//Direct3D 9
#pragma comment (lib, "d3dx9.lib")	//DirectX 9

SpriteObject spriteLander("textures/lander.png");
SpriteObject spriteTarget("textures/target.png");
SpriteObject spriteExhaust("textures/exhaust.png");

//Misc.
bool atWaypoint(Waypoint wpt)
{
	if (D3DXVec2Length(&(landerPos - D3DXVECTOR2(wpt.pos.x, wpt.pos.y + 25))) <= 3 && D3DXVec2Length(&landerVel) <= 1)
	{
		return true;
	}

	return false;
}
void resetAutoWaypoints()
{
	for (unsigned wpt = 0; wpt < 4; wpt++)
	{
		autoWaypoints[wpt].visited = false;
	}
}
void randomiseAutoWaypoints()
{
	for (unsigned wpt = 0; wpt < 4; wpt++)
	{
		autoWaypoints[wpt].pos = D3DXVECTOR2(float(rand_x_pos(gen)), float(rand_y_pos(gen)));
	}
}
bool captureVideoFrame()
{
	if (FAILED(d3ddev->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &frameCaptureSurface))){return false;}

	stringstream namestream;
	namestream << "framecapture/frame_" << videoCaptureFrameCounter << ".png";
	if (FAILED(D3DXSaveSurfaceToFile(namestream.str().c_str(), D3DXIFF_PNG, frameCaptureSurface, NULL, NULL))){return false;}

	return true;
}

//Initialisation
bool initProgram()
{


	return true;
}
bool initResources()
{
	//Sprite initialisation
	if (!spriteLander.CreateResources()){MessageBox(NULL, "Can't create resources for 'spriteLander'", "Error", MB_OK); return false;}
	if (!spriteTarget.CreateResources()){MessageBox(NULL, "Can't create resources for 'spriteTarget'", "Error", MB_OK); return false;}
	if (!spriteExhaust.CreateResources()){MessageBox(NULL, "Can't create resources for 'spriteExhaust'", "Error", MB_OK); return false;}

	//Misc.
	if (FAILED(D3DXCreateFont(d3ddev, 0, 0, 0, 0, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Arial"), &hudFont))){return false;}

	return true;
}
bool initD3D(HWND hWnd)
{
	//Create D3D9
    d3d = Direct3DCreate9(D3D_SDK_VERSION);
	if (d3d == NULL){return false;}

	//Check current display mode
	D3DDISPLAYMODE d3ddm; 
	if (FAILED(d3d->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm))){return false;}

	//DirectX parameters
    ZeroMemory(&d3dpp, sizeof(d3dpp));	
	d3dpp.BackBufferFormat = d3ddm.Format;				
	d3dpp.BackBufferWidth = WIDTH;							
	d3dpp.BackBufferHeight = HEIGHT;						
	d3dpp.BackBufferCount = 1;								
    d3dpp.Windowed = TRUE;								
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;		
	d3dpp.MultiSampleType = D3DMULTISAMPLE_4_SAMPLES;			
	d3dpp.MultiSampleQuality = 0;						
	d3dpp.hDeviceWindow = hWnd;								
    d3dpp.EnableAutoDepthStencil = TRUE;					
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;			
	d3dpp.Flags = 0;									
	d3dpp.FullScreen_RefreshRateInHz = 0;					
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE; 

	//Check shader/vertex processing caps
	D3DCAPS9 d3dCaps;
	if(FAILED(d3d->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &d3dCaps))){return false;}
	DWORD VertexProcessingMethod = d3dCaps.VertexProcessingCaps != 0 ? D3DCREATE_HARDWARE_VERTEXPROCESSING : D3DCREATE_SOFTWARE_VERTEXPROCESSING;

	//Device creation
    if(FAILED(d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, VertexProcessingMethod, &d3dpp, &d3ddev))){return false;}

	//Resource initiation
	if (!initResources()){return false;}

	return true;
}

//Device handling
bool onLostDevice()
{
	if (!spriteLander.LostDevice()){return false;}
	if (!spriteTarget.LostDevice()){return false;}
	if (!spriteExhaust.LostDevice()){return false;}

	if (FAILED(hudFont->OnLostDevice())){return false;}

	return true;
}
bool onResetDevice()
{
	if (!spriteLander.ResetDevice()){return false;}
	if (!spriteTarget.ResetDevice()){return false;}
	if (!spriteExhaust.ResetDevice()){return false;}

	if (FAILED(hudFont->OnResetDevice())){return false;}

	return true;
}
bool isDeviceLost()
{
	HRESULT hr = d3ddev->TestCooperativeLevel();

	if (hr == D3DERR_DEVICELOST)
	{
		Sleep(100);
		return true;
	}

	else if (hr == D3DERR_DEVICENOTRESET)
	{
		if (!onLostDevice()){MessageBox(NULL, "Can't prepare lost device", "Error", MB_OK); return true;}
		if (FAILED(d3ddev->Reset(&d3dpp))){MessageBox(NULL, "Can't reset the present parameters for the device", "Error", MB_OK); return true;}
		if (!onResetDevice()){MessageBox(NULL, "Can't reset the device", "Error", MB_OK); return true;}
	}

	return false;
}

//Draw calls
bool draw_Caption(string caption, D3DXVECTOR2 pos, D3DCOLOR colour, LPD3DXFONT font)
{
	RECT textRect = {long(pos.x), long(pos.y), 0, 0};
	if(FAILED(font->DrawText(NULL, caption.c_str(), -1, &textRect, DT_NOCLIP, colour))){return false;}
	
	return true;
}
template <class T> bool draw_Reading(string caption, T reading, D3DXVECTOR2 pos, D3DCOLOR colour, LPD3DXFONT font)
{
	RECT textRect = {long(pos.x), long(pos.y), 0, 0};
	stringstream readingstream;
	readingstream << caption << reading;
	if(FAILED(font->DrawText(NULL, readingstream.str().c_str(), -1, &textRect, DT_NOCLIP, colour))){return false;}
	
	return true;
}
bool draw_Sprites()
{
	if (!spriteExhaust.Render(NULL, D3DXVECTOR3(2.5f, 0, 0), D3DXVECTOR3(landerPos.x + landerLocalPointsTransformed[2].x, HEIGHT - landerPos.y + landerLocalPointsTransformed[2].y, 0), DEGRAD*landerRot, D3DXVECTOR2(1, landerThrust/50), D3DCOLOR_ARGB(255, 255, 255, 255))){return false;}
	if (!spriteLander.Render(NULL, D3DXVECTOR3(16, 16, 0), D3DXVECTOR3(landerPos.x, HEIGHT - landerPos.y, 0), DEGRAD*landerRot, D3DXVECTOR2(1, 1), D3DCOLOR_ARGB(255, 255, 255, 255))){return false;}

	if (autoWaypointMode)
	{
		for (unsigned wpt = 0; wpt < 4; wpt++)
		{
			D3DCOLOR waypointColour = autoWaypoints[wpt].visited ? D3DCOLOR_ARGB(255, 255, 0, 0) : D3DCOLOR_ARGB(255, 255, 255, 0);
			if (!spriteTarget.Render(NULL, D3DXVECTOR3(4, 4, 0), D3DXVECTOR3(autoWaypoints[wpt].pos.x, HEIGHT - autoWaypoints[wpt].pos.y, 0), 0, D3DXVECTOR2(1, 1), waypointColour)){return false;}
		}
	}
	else
	{
		D3DCOLOR waypointColour = manualWaypoint.visited ? D3DCOLOR_ARGB(255, 255, 0, 0) : D3DCOLOR_ARGB(255, 255, 255, 0);
		if (!spriteTarget.Render(NULL, D3DXVECTOR3(4, 4, 0), D3DXVECTOR3(manualWaypoint.pos.x, HEIGHT - manualWaypoint.pos.y, 0), 0, D3DXVECTOR2(1, 1), waypointColour)){return false;}
	}

	return true;
}
bool draw_Text()
{
	if (!draw_Reading("Thrust: ", landerThrust, D3DXVECTOR2(10, 10), D3DCOLOR_XRGB(255, 255, 255), hudFont)){return false;}
	if (!draw_Reading("TargetLatVel: ", landerBankAngleControllerStage1.target_control_value, D3DXVECTOR2(200, 10), D3DCOLOR_XRGB(255, 255, 255), hudFont)){return false;}
	if (!draw_Reading("TargetBankAngle: ", landerRotThrustControllerStage1.target_control_value, D3DXVECTOR2(200, 30), D3DCOLOR_XRGB(255, 255, 255), hudFont)){return false;}

	return true;
}

//Rendering
void preRender()
{
	//Lander target handling
	D3DXVECTOR2 activeTargetPos;
	if (autoWaypointMode)
	{
		bool everyWaypointVisited = true;

		for (unsigned wpt = 0; wpt < 4; wpt++)
		{
			if (atWaypoint(autoWaypoints[wpt]))
			{
				autoWaypoints[wpt].visited = true;
			}

			if (!autoWaypoints[wpt].visited)
			{
				activeTargetPos = autoWaypoints[wpt].pos;
				everyWaypointVisited = false;
				break;
			}
		}

		if (everyWaypointVisited)
		{
			resetAutoWaypoints();
		}
	}
	else
	{
		if (GetAsyncKeyState(VK_UP)){manualWaypoint.pos.y += 2;}
		else if (GetAsyncKeyState(VK_DOWN)){manualWaypoint.pos.y -= 2;}
		if (GetAsyncKeyState(VK_RIGHT)){manualWaypoint.pos.x += 2;}
		else if (GetAsyncKeyState(VK_LEFT)){manualWaypoint.pos.x -= 2;}

		manualWaypoint.visited = false;
		if (atWaypoint(manualWaypoint))
		{
			manualWaypoint.visited = true;
		}

		activeTargetPos = manualWaypoint.pos;
	}

	landerVertVelControllerStage1.target_control_value = activeTargetPos.y + 25;
	landerLatVelControllerStage1.target_control_value = activeTargetPos.x;

	//Angular stuff
	landerBankAngleControllerStage1.target_control_value = landerLatVelControllerStage1.Update(landerPos.x, timeDelta);
	landerRotThrustControllerStage1.target_control_value = landerBankAngleControllerStage1.Update(landerVel.x, timeDelta);
	landerAngAcc = landerRotThrustControllerStage1.Update(landerRot, timeDelta);

	landerAngVel += landerAngAcc*timeDelta;
	landerRot += landerAngVel*timeDelta - 0.5f*landerAngAcc*timeDelta*timeDelta;

	//Local point transformations
	D3DXMATRIX matRotation;
	D3DXMatrixTransformation2D(&matRotation, NULL, 0, NULL, NULL, DEGRAD*landerRot, NULL);
	for (unsigned lpt = 0; lpt < 3; lpt++)
	{
		D3DXVECTOR4 lptTransformed;
		D3DXVec2Transform(&lptTransformed, &landerLocalPoints[lpt], &matRotation);
		landerLocalPointsTransformed[lpt] = D3DXVECTOR2(lptTransformed.x, lptTransformed.y);
	}

	//Linear stuff
	landerThrustControllerStage1.target_control_value = landerVertVelControllerStage1.Update(landerPos.y, timeDelta);
	landerThrust = landerThrustControllerStage1.Update(landerVel.y, timeDelta);
	landerAcc = D3DXVECTOR2(landerThrust*sin(DEGRAD*landerRot), landerThrust*cos(DEGRAD*landerRot) - g);

	landerVel += landerAcc*timeDelta;
	landerPos += landerVel*timeDelta - 0.5f*landerAcc*timeDelta*timeDelta;
}
void renderFrame()
{
//PRERENDERING
	preRender();

//PASS ONE: render sprites
	d3ddev->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, D3DCOLOR_XRGB(0, 0, 0), 1, 0);
	d3ddev->BeginScene();

		draw_Sprites();
		draw_Text();
	
	d3ddev->EndScene();

	d3ddev->Present(NULL, NULL, NULL, NULL);

	if (videoCaptureActive)
	{
		captureVideoFrame();
		videoCaptureFrameCounter += 1;
	}
}

//Cleaning
void cleanD3D()
{
	SAFE_RELEASE(&d3ddev);
	SAFE_RELEASE(&d3d);

	spriteLander.Clean();
	spriteTarget.Clean();
	spriteExhaust.Clean();

	SAFE_RELEASE(&hudFont);
	SAFE_RELEASE(&frameCaptureSurface);
}

//Win32
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
    {
		case WM_KEYDOWN:
			switch (wParam)
			{
				case VK_SPACE:
					autoWaypointMode = !autoWaypointMode;
					resetAutoWaypoints();
					break;

				case 0x52:
					randomiseAutoWaypoints();
					resetAutoWaypoints();
					break;

				case VK_RETURN:
					videoCaptureActive = !videoCaptureActive;
					videoCaptureFrameCounter = 0;
					break;
			}
			break;

        case WM_DESTROY:
            PostQuitMessage(WM_QUIT);
			break;
	}

	 return DefWindowProc(hwnd, msg, wParam, lParam);
}
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	//Step 1: Registering the Window Class
	WNDCLASSEX wc;
    ZeroMemory(&wc, sizeof(WNDCLASSEX));
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = "WindowClass";

    if(!RegisterClassEx(&wc))
	{MessageBox(NULL, "The window could not be registered!", "Error", MB_ICONEXCLAMATION | MB_OK); return 0;}

	//Step 2: Creating the Window
	RECT clientRect = {0, 0, WIDTH, HEIGHT};
	AdjustWindowRect(&clientRect, WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE);
    hwnd = CreateWindow("WindowClass", "PIDLander", WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, 0, 0, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top, NULL, NULL, hInstance, NULL);
	if(hwnd == NULL){MessageBox(NULL, "Window Creation Failed!", "Error!", MB_OK); return 0;}

	if (!initD3D(hwnd)){MessageBox(NULL, "Direct3D failed to initialise", "Error", MB_ICONEXCLAMATION | MB_OK); return 0;}
	if (!initProgram()){return false;}

	ShowWindow(hwnd, nCmdShow);

	//Initiate timing variables
	LARGE_INTEGER countFrequency;
	QueryPerformanceFrequency(&countFrequency);
	float secsPerCount = 1/float(countFrequency.QuadPart);

	LARGE_INTEGER lastTime;
	QueryPerformanceCounter(&lastTime);

    //Step 3: The Message Loop
	MSG Msg;
	while (TRUE)
    {
        while (PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&Msg);
            DispatchMessage(&Msg);
        }

        if (Msg.message == WM_QUIT)
            break;
 
		LARGE_INTEGER currTime;
		QueryPerformanceCounter(&currTime);
		timeDelta = videoCaptureActive ? 1/30.0f : (currTime.QuadPart - lastTime.QuadPart)*secsPerCount;
		if (!isDeviceLost()){renderFrame();}
		lastTime = currTime;
    }

	    cleanD3D();
	    return Msg.wParam;
}
