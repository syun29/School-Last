#include "CCamera.h"

CCamera CCamera::m_camera[CCamera::eMax];
CCamera* CCamera::m_pCurrent = &m_camera[CCamera::eMainCamera];


//カメラオブジェクトを取得
CCamera* CCamera::GetCamera(int num) {
	return &m_camera[num];
}
//カメラオブジェクトを取得
CCamera* CCamera::GetCurrent() {
	return m_pCurrent;
}
//カメラオブジェクトを取得
void CCamera::SetCurrent(int num) {
	m_pCurrent = &m_camera[num];
}
//カメラオブジェクトを取得
void CCamera::SetCurrent(CCamera *p) {
	m_pCurrent = p;
	glViewport((GLsizei)p->m_viewportRect.m_x, (GLsizei)p->m_viewportRect.m_y, (GLsizei)p->m_viewportRect.m_width, (GLsizei)p->m_viewportRect.m_height);
}
//コンストラクタ
CCamera::CCamera(){
}
//幅を取得
float CCamera::GetWhidth() const{
	return m_whidth;
}
//高さを取得
float CCamera::GetHeight() const{
	return m_height;
}
//幅と高さを設定
void CCamera::SetSize(float w,float h) {
	m_whidth = w;
	m_height = h;
	Update2DProjectionMatrix();
}
//注視カメラでビュー行列を設定
void CCamera::LookAt(const CVector3D &eye,const CVector3D &at,const CVector3D &up){
	m_pos = eye;
	m_at = at;
	m_up = up;
	m_mView.LookAtView(eye,at,up);
	m_dir = CVector3D(-m_mView.m20,-m_mView.m21,-m_mView.m22);
	m_up = CVector3D(m_mView.m10, m_mView.m11, m_mView.m12);
}
//位置と回転値でビュー行列を設定
void CCamera::SetTranseRot(const CVector3D &pos,const CVector3D &rot){
	//カメラの回転行列を作成
	CMatrix mRT,mRX,mRY,mRZ,m;
	mRX.RotationX(rot.x);
	mRY.RotationY(rot.y);
	mRZ.RotationZ(rot.z);
	mRT.Transelate(pos.x,pos.y,pos.z);
	m = mRZ*mRY*mRX;
	CVector4D at = m * CVector4D(0,0,1,1);
	LookAt(pos,CVector3D(pos.x+at.x,pos.y+at.y,pos.z+at.z),CVector3D(m.m01,m.m11,m.m21));
}

//モデル行列からビュー行列を設定
void CCamera::SetTranseRot(const CMatrix &mtx){
	CVector4D at = mtx * CVector4D(0,0,1,1);
	CVector3D pos = mtx.GetPosition();
	LookAt(pos,CVector3D(at.x,at.y,at.z),CVector3D(mtx.m01,mtx.m11,mtx.m21));
}
//ビュー行列を直接設定
void CCamera::SetViewMatrix(const CMatrix &m) {
	m_mView = m;
	m_dir = CVector3D(-m_mView.m20, -m_mView.m21, -m_mView.m22);
	m_up = CVector3D(m_mView.m10, m_mView.m11, m_mView.m12);
	
	//カメラ行列を反映
	glLoadMatrixf(m_mView.f);
}
CMatrix CCamera::GetBuilbordMatrix() const {
	CMatrix m = m_mView;
	m.m03 = m.m13 = m.m23 = 0;
	return m.GetInverse();
}
//カメラの位置を取得
CVector3D CCamera::GetPos() const{
	return m_pos;
}
//カメラの向きを取得
CVector3D CCamera::GetDir() const{
	return m_dir;
}
//カメラの上方向を取得
CVector3D CCamera::GetUp() const{
	return m_up;
}

void CCamera::Perspective(float fovy, float aspect, float zNear, float zFar) {
	m_mProj.SetIndentity();
	m_mProj.Perspective(fovy, aspect, zNear, zFar);
}
void CCamera::Ortho(float left, float right, float bottom, float top, float zNear, float zFar) {
	m_mProj.SetIndentity();
	m_mProj.Ortho(left, right, bottom, top, zNear, zFar);

}
void CCamera::SetProjectionMatrix(const CMatrix &m) {
	m_mProj = m;
}
void CCamera::SetProjectionMatrix2D(const CMatrix& m) {
	m_m2D = m;
}

void CCamera::Viewport(float x, float y, float w, float h)
{
	m_viewportRect = CRect(CVector2D(x,y), CVector2D(w,h));
	m_mViewport.Viewport(x, y, w, h);
}

void CCamera::Update2DProjectionMatrix()
{
	m_m2D = CMatrix(
		2.0f / GetWhidth(), 0.0f, 0.0f, -1.0f,
		0.0f, -2.0f / GetHeight(), 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

}
