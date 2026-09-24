#include "CShadow.h"
#include "Utility.h"
#include "CRendaring.h"
std::vector<CShadow*> CShadow::m_shadow_list;
int CShadow::m_state = eNone;
int CShadow::m_shadow_tex_cnt = 0;
std::vector<int> CShadow::m_depth_tex_idx;
std::vector<CVector2D> CShadow::m_depth_inv_tex_size;
std::vector<CMatrix> CShadow::m_shadowMatrix;
float CShadow::m_shadow_ambient = 0.8f;
CShadow::CShadow(float length, float height, int texWidth, int texHeight,int use_light){
	
	m_depthtexHeight = texHeight;
	m_depthtexWidth = texWidth;
	m_lightViewLength = length;
	m_lightHeight = height;
	m_use_light = use_light;
	int texcnt = 1;
	m_shadow_idx = m_shadow_tex_cnt;
	CLight::m_UseShadow[use_light] = m_shadow_idx;
	if (CLight::GetTypeColorPointer()[use_light] == CLight::eLight_Point)
		texcnt = 6;
	for (int i = 0; i < texcnt; i++) {
		mp_render_target.push_back(new CTextureFrame(m_depthtexWidth, m_depthtexHeight, CVector4D(1, 1, 1, 1), 1));
		m_shadowMatrix.push_back(CMatrix::indentity);
		m_depth_tex_idx.push_back(m_shadow_tex_cnt + shadow_tex_offset);
		m_depth_inv_tex_size.push_back(CVector2D(1.0f/m_depthtexWidth, 1.0f/m_depthtexHeight));
		m_shadow_tex_cnt++;
	}
	m_shadow_list.push_back(this);
}
CShadow::~CShadow() {
	for(auto  var : mp_render_target)	{
		delete var;
	}
	mp_render_target.clear();

}
void CShadow::Render(std::function<void()> render, CTextureFrame* gbuffer,bool update_shadow) {
	if (update_shadow) {
		CCamera* back = CCamera::GetCurrent();
		//シャドウマップ描画準備
		CVector3D cam_pos = CCamera::GetCurrent()->GetPos();
		//シャドウマップ用　光源からのカメラに切り替え
		CCamera::SetCurrent(CCamera::GetCamera(CCamera::eShadowCamera));

		//テクスチャ行列
		static  const CMatrix texMat(0.5f, 0.0f, 0.0f, 0.5f,
			0.0f, 0.5f, 0.0f, 0.5f,
			0.0f, 0.0f, 0.5f, 0.5f,
			0.0f, 0.0f, 0.0f, 1.0f);
		CLight::SetLighting(false);
		//前面を描画しない
		glCullFace(GL_FRONT);
		m_state = eShadow;

		for(auto& s : m_shadow_list)
		{
			if (CLight::GetTypeColorPointer()[s->m_use_light] == CLight::eLight_Point) {
				//投影行列
				CCamera::GetCurrent()->Perspective((float)M_PI_2, 1.0f, 6.0, 45.0f);
				for (int i = 0; i < 6; i++) {
					CVector3D dir[6] = {
						CVector3D(DtoR(90),0,0),
						CVector3D(DtoR(-90),0,0),
						CVector3D(0,0,0),
						CVector3D(0,DtoR(90),0),
						CVector3D(0,DtoR(180),0),
						CVector3D(0,DtoR(-90),0),
					};
					//ビュー行列
					CCamera::GetCurrent()->SetTranseRot(CLight::GetPosPointer()[s->m_use_light] + CMatrix::MRotation(dir[i]) * CVector3D(0, 0, -5), dir[i]);
					/*float zFar = 45.0f;
				float zNear = 5.0f;
				float f = 1 / tanf((DtoR(90) / 2));
				CMatrix m(	f, 0, 0, 0,
							0, f, 0, 0,
							0, 0, (zFar + zNear) / (zNear - zFar), (2 * zFar * zNear) / (zNear - zFar),
							0, 0, -1, 0);
	//				m.m11 = i==0 ? 1:1;
					*/
					//シャドウマップ変換行列
					//CShadow::m_shadowMatrix[s->m_shadow_idx+i] = texMat * m * CCamera::GetCurrent()->GetViewMatrix();
					//シャドウマップ変換行列
					CShadow::m_shadowMatrix[s->m_shadow_idx + i] = texMat * CCamera::GetCurrent()->GetProjectionMatrix() * CCamera::GetCurrent()->GetViewMatrix();
					//シャドウマップ用描画
					s->mp_render_target[i]->BeginDraw();
					render();
					s->mp_render_target[i]->EndDraw();
					glActiveTexture(GL_TEXTURE0 + shadow_tex_offset + s->m_shadow_idx + i);
					glBindTexture(GL_TEXTURE_2D, s->mp_render_target[i]->GetDepthTexture()->m_bufID);
					glEnable(GL_TEXTURE_2D);
					//0番テクスチャー操作に戻す
					glActiveTexture(GL_TEXTURE0);
				}
			}
			else if (CLight::GetTypeColorPointer()[s->m_use_light] == CLight::eLight_Spot) {
				CCamera::GetCurrent()->Perspective(CLight::GetRadiationAnglePointer()[s->m_use_light] * 2, 1.0f, 0.3f, 45.0f);
				//ライトの向き
				CVector3D ld = CLight::GetDirPointer()[s->m_use_light];
				CVector3D up = CVector3D::up;
				if (abs(ld.y) == 1.0) up = CVector3D::front;
				//ビュー行列
				CCamera::GetCurrent()->LookAt(CLight::GetPos(s->m_use_light), CLight::GetPos(s->m_use_light) + ld, up);

				//シャドウマップ変換行列
				CShadow::m_shadowMatrix[s->m_shadow_idx] = texMat * CCamera::GetCurrent()->GetProjectionMatrix() * CCamera::GetCurrent()->GetViewMatrix();
				//シャドウマップ用描画
				s->mp_render_target[0]->BeginDraw();
				render();
				s->mp_render_target[0]->EndDraw();
				glActiveTexture(GL_TEXTURE0 + shadow_tex_offset + s->m_shadow_idx);
				glBindTexture(GL_TEXTURE_2D, s->mp_render_target[0]->GetDepthTexture()->m_bufID);
				glEnable(GL_TEXTURE_2D);
				//0番テクスチャー操作に戻す
				glActiveTexture(GL_TEXTURE0);
			}
			else {
				CCamera::GetCurrent()->Ortho(-s->m_lightViewLength, s->m_lightViewLength, -s->m_lightViewLength, s->m_lightViewLength, 1.0f, 500.0f);
				//ライトの向き
				CVector3D ld = CLight::GetDirPointer()[s->m_use_light];
				//光源の高さ
				const float dist = s->m_lightHeight;
				CVector3D up = CVector3D::up;
				if (abs(ld.y) == 1.0) up = CVector3D::front;
				//ビュー行列
				CCamera::GetCurrent()->LookAt(cam_pos - ld * dist, cam_pos, up);

				//シャドウマップ変換行列
				CShadow::m_shadowMatrix[s->m_shadow_idx] = texMat * CCamera::GetCurrent()->GetProjectionMatrix() * CCamera::GetCurrent()->GetViewMatrix();
				//シャドウマップ用描画
				s->mp_render_target[0]->BeginDraw();
				render();
				s->mp_render_target[0]->EndDraw();
				glActiveTexture(GL_TEXTURE0 + shadow_tex_offset + s->m_shadow_idx);
				glBindTexture(GL_TEXTURE_2D, s->mp_render_target[0]->GetDepthTexture()->m_bufID);
				glEnable(GL_TEXTURE_2D);
				//0番テクスチャー操作に戻す
				glActiveTexture(GL_TEXTURE0);
			}

		}
		//背面を描画しない
		glCullFace(GL_BACK);
		CLight::SetLighting(true);
		//元のカメラに戻す
		CCamera::SetCurrent(back);
	}
	if (gbuffer) gbuffer->BeginDraw();
	m_state = eNone;
	//通常描画
	render();
	if (gbuffer) gbuffer->EndDraw();
}

void CShadow::CreateInscance(float length, float height, int texWidth, int texHeight,int use_light)
{
	if (use_light != 0 && !CRendaring::CheckType(CRendaring::RendringType::eDeferred)) {
		MessageBox(GL::hWnd, "ライト0番以外の影には遅延レンダリング方式への変更が必要。CRendaring::CreatInstance(SCREEN_WIDTH, SCREEN_HEIGHT, 0.0 , CRendaring::RendringType::eDeferred)", "Shadow生成エラー", MB_OK);
		return;
	}
	new CShadow(length, height, texWidth, texHeight,use_light);
}

CShadow* CShadow::GetInstance(int idx)
{
	if (m_shadow_list.empty()) return nullptr;
	return m_shadow_list[idx];
}

void CShadow::ClearInstance()
{
	for(auto s  : m_shadow_list)
	{
		delete s;
		s = nullptr;
	}
	m_shadow_list.clear();
	m_depth_tex_idx.clear();
	m_shadowMatrix.clear();
	CLight::Init();
}