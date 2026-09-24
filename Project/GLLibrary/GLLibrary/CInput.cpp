#include "GL.h"
#include "CInput.h"
#include "Utility.h"

unsigned char		CInput::m_key_code[PAD_MAX][eKeyMax] = { 0 };
unsigned char		CInput::m_pad_code[PAD_MAX][16] = {
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
};
unsigned long CInput::m_key_old[PAD_MAX]={0};
unsigned long CInput::m_key_state[PAD_MAX][CInput::eStateMax]={0};
CVector2D CInput::m_mouse_vec;
int CInput::m_mouse_wheel= 0;
int CInput::m_mouse_wheelOld= 0;
bool CInput::m_mouse_inside = false;
LPDIRECTINPUT8 CInput::m_pDinput=NULL;
LPDIRECTINPUTDEVICE8 CInput::m_pMouse=NULL;
CInput::SPadDevice CInput::m_device[PAD_MAX] = { { 0, }, };


//利用可能なジョイスティックを列挙するコールバック関数
bool CALLBACK CInput::_padCallback(const LPDIDEVICEINSTANCE lpddi, LPVOID pvRe)
{

//	HRESULT ret;
	static int d_num = 0;
	SPadDevice* pd = &((SPadDevice*)pvRe)[d_num];
	LPDIRECTINPUTDEVICE8* ppDevice = &pd->m_pPadDevice;
	//ジョイスティック用にデバイスオブジェクトを作成
	if (m_pDinput->CreateDevice(lpddi->guidInstance, ppDevice, NULL) != DI_OK) 
		return DIENUM_CONTINUE;

	(*ppDevice)->QueryInterface(IID_IDirectInputDevice2, (LPVOID *)&ppDevice);

	pd->no = d_num;
	if (!pd->m_pPadDevice) return DIENUM_CONTINUE;
	// デバイスをジョイスティックに設定
	if (FAILED(pd->m_pPadDevice->SetDataFormat(&c_dfDIJoystick2))) 	return DIENUM_CONTINUE;
	// 協調レベルの設定
	if (FAILED(pd->m_pPadDevice->SetCooperativeLevel(GL::hWnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND)))  return DIENUM_CONTINUE;
	// デバイスを「取得」する
	pd->m_pPadDevice->Acquire();
	

	d_num++;
	return (d_num >= PAD_MAX) ? DIENUM_STOP : DIENUM_CONTINUE;
}
bool CInput::Init(){

	// 「DirectInput」オブジェクトの作成
	if (FAILED(DirectInput8Create(GetModuleHandle(NULL),DIRECTINPUT_VERSION, IID_IDirectInput8, (VOID**)&m_pDinput, NULL)))
		return false;
	
	//利用可能なゲームコントローラーの列挙関数を実行
	if (FAILED(m_pDinput->EnumDevices(DI8DEVCLASS_GAMECTRL, (LPDIENUMDEVICESCALLBACK)CInput::_padCallback,&m_device, DIEDFL_ATTACHEDONLY)))
		return false;

	
	if (FAILED(m_pDinput->CreateDevice(GUID_SysMouse, &m_pMouse, NULL)))
		return false;

	// データ・フォーマットを設定
	if (FAILED(m_pMouse->SetDataFormat(&c_dfDIMouse)))
	{
		return false;
	}
	// 協調モードを設定（フォアグラウンド＆非排他モード）
	if (FAILED(m_pMouse->SetCooperativeLevel(GL::hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND)))
	{
		return false;
	}
	return true;


}
void CInput::ClearInstance() {

	if (m_pMouse) m_pMouse->Release();
	if(m_pDinput) m_pDinput->Release();
	m_pDinput = nullptr;
}
void CInput::SetMouseInside(bool f) {
	m_mouse_inside = f;
	UpdateClipCursor(true);
}
void CInput::ShowCursor(bool f)
{
	if (f) {
		while (::ShowCursor(true) < 0);
	} else {
		while (::ShowCursor(false) >= 0);
	}
}
CVector2D CInput::GetRStick(int no)
{
	DIJOYSTATE2* pd = GetPadData(no);
	return  pd ? CVector2D(toFloat(pd->lZ), toFloat(pd->lRz)) :CVector2D::zero;
}
CVector2D CInput::GetLStick(int no)
{

	DIJOYSTATE2* pd = GetPadData(no);
	return pd ? CVector2D(toFloat(pd->lX), toFloat(pd->lY)):CVector2D::zero;
}
void CInput::Update() {
	for (int i = 0; i < PAD_MAX; i++) {
		unsigned long key = 0;
		unsigned long state = 0;
		for (int j = 0; j < eKeyMax; j++) {
			if (::GetKeyState(m_key_code[i][j]) & 0x80) {
				key |= 1 << j;
			}
		}

		if (m_device[i].m_pPadDevice) {
			DIJOYSTATE2 *js = &m_device[i].js;
			HRESULT hr = m_device[i].m_pPadDevice->Acquire();
			if ((hr == DI_OK) || (hr == S_FALSE))
			{
				m_device[i].m_pPadDevice->GetDeviceState(sizeof(DIJOYSTATE2), js);
				if (js->rgdwPOV[0] != 0xFFFFFFFF)
				{
					// 角度を利用した方法
					float rad = DtoR((js->rgdwPOV[0] / 100.0f));
					float x = sinf(rad);
					float y = cosf(rad);
					if (y > 0.01f)  key |= 1 << eUp;
					if (y < -0.01f)   key |= 1 << eDown;
					if (x < -0.01f) key |= 1 << eLeft;
					if (x > 0.01f)  key |= 1 << eRight;
				}
				for (int j = 0; j < eUp; j++) {
					if (js->rgbButtons[m_pad_code[i][j]] & 0x80) key |= 1 << j;
				}

	
			}
		}
		m_key_state[i][ePush] = (~m_key_old[i]) & key;
		m_key_state[i][eHold] = m_key_old[i] & key;
		m_key_state[i][ePull] = m_key_old[i] & (~key);
		m_key_state[i][eFree] = (~m_key_old[i]) & (~key);
		m_key_old[i] = key;
	}
	
	DIMOUSESTATE state;
	HRESULT hr = m_pMouse->Poll();
	if (FAILED(hr))
	{
		hr = m_pMouse->Acquire();
		while (hr == DIERR_INPUTLOST)
			hr = m_pMouse->Acquire();
	}else
	if (!FAILED(m_pMouse->GetDeviceState(sizeof(DIMOUSESTATE), &state))) {
		m_mouse_vec = CVector2D((float)state.lX, (float)state.lY);
	}


	m_mouse_wheelOld = m_mouse_wheel;
	m_mouse_wheel = 0;
	
}

 void CInput::UpdateClipCursor(int f) {
	 if (f && m_mouse_inside) {
		 static RECT rect;
		 rect = GL::window_rect;
		 rect.bottom -= 32;
		 ClipCursor(&rect);
	 } else {
		 ClipCursor(NULL);
	 }

}