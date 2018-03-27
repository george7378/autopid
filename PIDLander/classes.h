#ifndef CLASSES_H
#define CLASSES_H

//Sprite
class SpriteObject
{
private:
	LPD3DXSPRITE objectSprite;
	LPDIRECT3DTEXTURE9 spriteTexture;
public:
	string texturepath;
	
	SpriteObject(string path)
	{texturepath = path;};

	bool CreateResources()
	{
		if (FAILED(D3DXCreateTextureFromFileEx(d3ddev, texturepath.c_str(), D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, 0, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_FILTER_NONE, D3DX_DEFAULT, 0, NULL, NULL, &spriteTexture))){return false;}
		if (FAILED(D3DXCreateSprite(d3ddev, &objectSprite))){return false;}
		
		return true;
	}

	bool Render(RECT* rect, D3DXVECTOR3 centre, D3DXVECTOR3 pos, float rot, D3DXVECTOR2 scaling, D3DCOLOR colour)
	{
		if (FAILED(objectSprite->Begin(D3DXSPRITE_ALPHABLEND | D3DXSPRITE_DO_NOT_ADDREF_TEXTURE | D3DXSPRITE_SORT_TEXTURE))){return false;}
		
		D3DXMATRIX matTransform;
		D3DXMatrixTransformation2D(&matTransform, &D3DXVECTOR2(pos.x, pos.y), 0, &scaling, &D3DXVECTOR2(pos.x, pos.y), rot, NULL);
		objectSprite->SetTransform(&matTransform);
		
		if (FAILED(objectSprite->Draw(spriteTexture, rect, &centre, &pos, colour))){return false;}
		
		if (FAILED(objectSprite->End())){return false;}
		
		return true;
	}

	bool LostDevice()
	{
		if (FAILED(objectSprite->OnLostDevice())){return false;}
		
		return true;
	}

	bool ResetDevice()
	{
		if (FAILED(objectSprite->OnResetDevice())){return false;}
		
		return true;
	}

	void Clean()
	{
		SAFE_RELEASE(&objectSprite);
		SAFE_RELEASE(&spriteTexture);
	}
};

//Misc
class PIDController
{
private:
	bool integral_saturated;
	float integral_value;
	float prev_control_error;
public:
	float kp, ki, kd;
	float max_output_value, min_output_value;
	float target_control_value;

	PIDController(float in_kp, float in_ki, float in_kd, float in_max_output, float in_min_output, float in_ctrl_target)
	{kp = in_kp; ki = in_ki; kd = in_kd;
	 max_output_value = in_max_output; min_output_value = in_min_output;
	 target_control_value = in_ctrl_target;
	 integral_saturated = false; integral_value = 0; prev_control_error = 0;};

	float Update(float current_control_value, float dt)
	{
		float control_value_error = target_control_value - current_control_value;
		if (!integral_saturated)
		{
			integral_value += control_value_error*dt;
		}
		float differential_value = (control_value_error - prev_control_error)/dt;
		float new_output_value = kp*control_value_error + ki*integral_value + kd*differential_value;
		
		integral_saturated = false;
		if (new_output_value < min_output_value)
		{
			new_output_value = min_output_value;
			integral_saturated = true;
		}
		else if (new_output_value > max_output_value)
		{
			new_output_value = max_output_value;
			integral_saturated = true;
		}
		prev_control_error = control_value_error;

		return new_output_value;
	}

	void Reset()
	{
		integral_saturated = false;
		integral_value = 0;
	}
};
struct Waypoint
{
	D3DXVECTOR2 pos;
	bool visited;

	Waypoint(D3DXVECTOR2 in_pos)
	{pos = in_pos; visited = false;};
};

#endif
