#ifndef PROGCONTROL_H
#define PROGCONTROL_H

//Constants
#define g			20
#define DEGRAD		float(M_PI)/180
#define RADDEG		180/float(M_PI)

//Lander
D3DXVECTOR2 landerLocalPoints[3] = {D3DXVECTOR2(-14, 16), D3DXVECTOR2(14, 16), D3DXVECTOR2(0, 15)};
D3DXVECTOR2 landerLocalPointsTransformed[3];

D3DXVECTOR2 landerPos(50, 50);
D3DXVECTOR2 landerVel(0, 0);
D3DXVECTOR2 landerAcc(0, 0);
float landerRot = 0;
float landerAngVel = 0;
float landerAngAcc = 0;

float landerThrust = 0;
PIDController landerVertVelControllerStage1(0.25f, 0, 0, 30, -30, 0);
PIDController landerThrustControllerStage1(2.0f, 0.4f, 0.5f, 50, 0, 0);
PIDController landerLatVelControllerStage1(0.25f, 0, 0, 30, -30, 0);
PIDController landerBankAngleControllerStage1(3.0f, 0.1f, 4.0f, 30, -30, 0);
PIDController landerRotThrustControllerStage1(3.0f, 0.1f, 4.0f, 15, -15, 0);

//Misc
float timeDelta = 0;
bool autoWaypointMode = false;
bool videoCaptureActive = false;
unsigned videoCaptureFrameCounter = 0;
Waypoint manualWaypoint(D3DXVECTOR2(320, 240));
Waypoint autoWaypoints[4] = {Waypoint(D3DXVECTOR2(100, 100)), Waypoint(D3DXVECTOR2(540, 100)),
							 Waypoint(D3DXVECTOR2(100, 380)), Waypoint(D3DXVECTOR2(540, 380))};
random_device rd;
default_random_engine gen(rd());
uniform_int_distribution <unsigned> rand_x_pos(20, WIDTH - 20);
uniform_int_distribution <unsigned> rand_y_pos(20, HEIGHT - 80);

#endif
