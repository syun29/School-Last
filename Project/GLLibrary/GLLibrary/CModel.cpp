#include "CModel.h"
#include "CLight.h"
#include "CCamera.h"

#include "CModelObj.h"
#ifdef USE_ATHOR_MODEL
#include "CModelX.h"
#include "CModelFbx.h"
#endif
#include "CModelA3M.h"
#include "CModelField.h"
#include "CCollision.h"


void CModel::RenderBuilbord(const CMatrix& m)
{
	UpdateMatrix();
	CMatrix mat = m * m_matrix;
	mat.m03 = m_matrix.m03; mat.m13 = m_matrix.m13; mat.m23 = m_matrix.m23;
	Render(mat);
}

void CModel::RenderBuilbord() {
	RenderBuilbord(CCamera::GetCurrent()->GetBuilbordMatrix());
}
void CModel::UpdateMatrix() {
	CMatrix RX,RY,RZ;
	RX.RotationX(m_rot.x+m_baserot.x);
	RY.RotationY(m_rot.y+m_baserot.y);
	RZ.RotationZ(m_rot.z+m_baserot.z);
	m_rotMtx = RZ * RY * RX;
	m_transMtx.Transelate(m_pos.x,m_pos.y,m_pos.z);
	m_scaleMtx.Scale(m_scale.x,m_scale.y,m_scale.z);
	m_matrix = m_transMtx * m_rotMtx * m_scaleMtx;
}
void CModel::SendShaderParam(CShader* s, const CMatrix &m, const CMatrix &mv, const CMatrix &p) {

	
	/*
	  ワールドトランスフォーム
	*/
	int worldId = glGetUniformLocation(s->GetProgram(),"WorldMatrix"); 
	glUniformMatrix4fv(worldId,1,false,m.f);

	
	glUniformMatrix4fv(glGetUniformLocation(s->GetProgram(), "ProjectionMatrix"), 1, GL_FALSE, p.f);
	glUniformMatrix4fv(glGetUniformLocation(s->GetProgram(), "ModelViewMatrix"), 1, GL_FALSE, mv.f);
	glUniformMatrix4fv(glGetUniformLocation(s->GetProgram(), "LocalMatrix"), 1, GL_FALSE, CMatrix::indentity.f);

}

void CModel::MakePath(const char* filenPath) {
	strcpy_s(m_filePath, PATH_SIZE, filenPath);
	char* p = strrchr(m_filePath, '/');
	if(!p) p = strrchr(m_filePath, '\\');
	if (p) *(p+1) = '\0';
	else strcpy_s(m_filePath,PATH_SIZE,"");

}
CModel::CModel() : m_pos(0, 0, 0), m_rot(0, 0, 0), m_scale(1, 1, 1), m_baserot(0, 0, 0), m_filePath(""), m_shadow_bias(-0.01), m_toon(false),m_time_scale(true) {

}
std::vector<CCollTriangle> CModel::CollisionCapsule(const CVector3D& top, const CVector3D& bottom, float radius) const
{
	return CollisionCapsule(CCapsule(top, bottom, radius));
}
CModel* CModel::CreateModel(const char* path, int cut_x, int cut_y, int cut_z) {
	CModel *m = nullptr;
	const char *c = strrchr(path, '.') + 1;

	if (strcmp(c, "obj") == 0 || strcmp(c, "obb") == 0) {
		CModelObj* model = new CModelObj();
		m = model;
		if (model->Load(path, cut_x, cut_y, cut_z)) {
			return m;
		}
		else {
			delete m;
			m = nullptr;
		}

	}else if (strcmp(c, "a3m") == 0) {
		CModelA3M* model = new CModelA3M();
		m = model;
		if (model->Load(path, cut_x, cut_y, cut_z)) {
			return m;
		}
		else {
			delete m;
			m = nullptr;
		}

	} else if (strcmp(c, "field") == 0) {
		m = new CModelField();
		if (m->Load(path)) {
			return m;
		}
		else {
			delete m;
			m = nullptr;
		}
	}
#ifdef USE_ATHOR_MODEL
	else if (strcmp(c, "x") == 0) {
		
		m = new CModelX();
		if (m->Load(path)) {
			return m;
		} else {
			delete m;
			m = nullptr;
		}
	} else if (strcmp(c, "fbx") == 0 || strcmp(c, "FBX") == 0) {
		m = new CModelFBX();
		if (m->Load(path)) {
			return m;
		}
	}
	else if (strcmp(c, "toy") == 0) {
		CModelA3M*mm = new CModelA3M();
		m = mm;
		if (mm->LoadToyModel(path)) {
			return m;
		}
	}
#endif
	char str[256] = "";
	sprintf_s(str, 256, "%sの読み込みに失敗しました\n", path);
	MessageBox(GL::hWnd, str, "", MB_OK);
	return nullptr;
}

void CModel::CalcTangentAndBinormal(
	const CVector3D& v0, const CVector2D& uv0,
	const CVector3D& v1, const CVector2D& uv1,
	const CVector3D& v2, const CVector2D& uv2,
	CVector3D* outTangent, CVector3D* outBinormal
) {
	CVector3D deltaPos1 = v1 - v0;
	CVector3D deltaPos2 = v2 - v0;

	CVector3D deltaUV1 = uv1 - uv0;
	CVector3D deltaUV2 = uv2 - uv0;

	float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
	*outTangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
	if (outBinormal) *outBinormal = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;

}


CMaterial::CMaterial() : m_ambient(1, 1, 1, 1), m_diffuse(1, 1, 1, 1), m_specular(0, 0, 0),
	m_emissive(0, 0, 0), m_shininess(1), m_alpha(1.0f), m_texture_name(""), m_normal_map_name(""),m_st(0, 0), mp_texture(NULL), mp_normal_map(NULL), mp_shader(nullptr), m_shader_name("") {
	memset(m_name, 0, sizeof(m_name));
	memset(m_texture_name, 0, sizeof(m_texture_name));
	memset(m_normal_map_name, 0, sizeof(m_normal_map_name));

}
CMaterial::CMaterial(const CMaterial & mat) :m_ambient(mat.m_ambient),m_diffuse(mat.m_diffuse),m_specular(mat.m_specular),
		m_emissive(mat.m_emissive),m_shininess(mat.m_shininess),m_alpha(mat.m_alpha), m_st(mat.m_st), mp_texture(mat.mp_texture),mp_normal_map(mat.mp_normal_map), mp_shader(mat.mp_shader),m_shader_name(mat.m_shader_name)
{
	strcpy_s(m_name,sizeof(m_name), mat.m_name);
	strcpy_s(m_texture_name,sizeof(m_texture_name),mat.m_texture_name);
	strcpy_s(m_normal_map_name, sizeof(m_normal_map_name), mat.m_normal_map_name);

}
void CMaterial::Release(){
	if (mp_texture) {
		mp_texture->Release();
		delete mp_texture;
		mp_texture = NULL;
	}
}
void CMaterial::SendFragParam() {
	if (mp_shader) {
		/*
		視線設定
		*/
		CVector3D vec = CCamera::GetCurrent()->GetDir();
		int eyeId = glGetUniformLocation(mp_shader->GetProgram(), "eyeVec");  //カメラ向きを設定
		glUniform3fv(eyeId, 1, (float*)&vec);
		/*
		視線設定
		*/
		CVector3D pos = CCamera::GetCurrent()->GetPos();
		int eyePosId = glGetUniformLocation(mp_shader->GetProgram(), "eyePos");  //カメラ位置を設定
		glUniform3fv(eyePosId, 1, (float*)&pos);



		/*
		ライト設定
		*/
		if (!CRendaring::CheckType(CRendaring::RendringType::eDeferred)) {
			glUniform3fv(glGetUniformLocation(mp_shader->GetProgram(), "lightPos"), CLight::LIGHT_MAX, (float*)CLight::GetPosPointer());
			glUniform3fv(glGetUniformLocation(mp_shader->GetProgram(), "lightDir"), CLight::LIGHT_MAX, (float*)CLight::GetDirPointer());
			glUniform3fv(glGetUniformLocation(mp_shader->GetProgram(), "lightAmbientColor"), CLight::LIGHT_MAX, (float*)CLight::GetAmbientColorPointer());
			glUniform3fv(glGetUniformLocation(mp_shader->GetProgram(), "lightDiffuseColor"), CLight::LIGHT_MAX, (float*)CLight::GetDiffuseColorPointer());
			glUniform1fv(glGetUniformLocation(mp_shader->GetProgram(), "lightRange"), CLight::LIGHT_MAX, (float*)CLight::GetRangePointer());
			glUniform1fv(glGetUniformLocation(mp_shader->GetProgram(), "lightRadiationAngle"), CLight::LIGHT_MAX, (float*)CLight::GetRadiationAnglePointer());
			glUniform1iv(glGetUniformLocation(mp_shader->GetProgram(), "lightType"), CLight::LIGHT_MAX, (int*)CLight::GetTypeColorPointer());
			glUniform1iv(glGetUniformLocation(mp_shader->GetProgram(), "lightUseShadow"), CLight::LIGHT_MAX, (int*)CLight::m_UseShadow);
		}

		const CLight::SFog& fog = CLight::GetFogParam();
		glUniform4fv(glGetUniformLocation(mp_shader->GetProgram(), "fogColor"), 1, (float*)&fog.m_Color);
		glUniform1f(glGetUniformLocation(mp_shader->GetProgram(), "fogNear"), fog.m_Near);
		glUniform1f(glGetUniformLocation(mp_shader->GetProgram(), "fogFar"), fog.m_Far);
		//出力色設定
		glBindFragDataLocation(mp_shader->GetProgram(), 0, "out_color"); // 色が格納されるout変数名



		int AmbientId = glGetUniformLocation(mp_shader->GetProgram(), "Ambient");
		glUniform4fv(AmbientId, 1, (GLfloat*)&m_ambient);

		int DiffuseId = glGetUniformLocation(mp_shader->GetProgram(), "Diffuse");
		glUniform4fv(DiffuseId, 1, (GLfloat*)&m_diffuse);

		int PowId = glGetUniformLocation(mp_shader->GetProgram(), "Pow");
		glUniform1f(PowId, m_shininess);

		int SpecularId = glGetUniformLocation(mp_shader->GetProgram(), "Specular");
		glUniform3fv(SpecularId, 1, (GLfloat*)&m_specular);


		int EmissiveId = glGetUniformLocation(mp_shader->GetProgram(), "Emissive");
		glUniform3fv(EmissiveId, 1, (GLfloat*)&m_emissive);
		glUniform1f(glGetUniformLocation(mp_shader->GetProgram(), "alpha"), m_alpha);

		if (mp_texture) {
			glActiveTexture(GL_TEXTURE0);
			glUniform1i(glGetUniformLocation(mp_shader->GetProgram(), "uSetex"), 1);
			glUniform2fv(glGetUniformLocation(mp_shader->GetProgram(), "stscroll"), 1, m_st.v);
			mp_texture->MapTexture();
		}
		else {
			glUniform1i(glGetUniformLocation(mp_shader->GetProgram(), "uSetex"), 0);
		}

		if (mp_normal_map) {
			glActiveTexture(GL_TEXTURE1);
			glUniform1i(glGetUniformLocation(mp_shader->GetProgram(), "usenormalMap"), 1);
			mp_normal_map->MapTexture();
		}
		else {
			glUniform1i(glGetUniformLocation(mp_shader->GetProgram(), "usenormalMap"), 0);
		}
	//	glUniform1i(glGetUniformLocation(mp_shader->GetProgram(), "usenormalMap"), 0);
		glActiveTexture(GL_TEXTURE0);
//		float inv = 1.0;//凹凸反転
//		float fGrad = 5.0;//勾配強調
//		float fDisp = 0.15;//変位量調整
//		float bias = 0.3;//面変位量調整

//		glUniform1f(glGetUniformLocation(mp_shader->GetProgram(), "fDisp"), fDisp);
//		glUniform1f(glGetUniformLocation(mp_shader->GetProgram(), "inv"), inv);
//		glUniform1f(glGetUniformLocation(mp_shader->GetProgram(), "bias"), bias);

		glUniform1i(glGetUniformLocation(mp_shader->GetProgram(), "sampler"), 0);//GL_TEXTURE0を適用
		glUniform1i(glGetUniformLocation(mp_shader->GetProgram(), "normalMap"), 1);//GL_TEXTURE1を適用

		glUniformMatrix4fv(glGetUniformLocation(mp_shader->GetProgram(), "ShadowTextureMatrix"), CShadow::GetTextureSize(), FALSE,(float*)CShadow::GetShadowMatrixPointer());
		if (CShadow::isDoShadow()) {
			glUniform1i(glGetUniformLocation(mp_shader->GetProgram(), "depth_tex_size"), 0);
			glUniform1i(glGetUniformLocation(mp_shader->GetProgram(), "lighting"), 0);
		} else {
			glUniform1i(glGetUniformLocation(mp_shader->GetProgram(), "depth_tex_size"), CShadow::GetTextureSize());
			glUniform1i(glGetUniformLocation(mp_shader->GetProgram(), "lighting"), CLight::GetLighting());
		}
		glUniform1iv(glGetUniformLocation(mp_shader->GetProgram(), "depth_tex"), CShadow::GetTextureSize(),(int*)CShadow::GetShadowTexIdxPointer());
		glUniform1f(glGetUniformLocation(mp_shader->GetProgram(), "shadow_ambient"), CShadow::GetShadowAmbient());
		if (CShadow::GetTextureSize()>0)
			glUniform2fv(glGetUniformLocation(mp_shader->GetProgram(), "d_tex_scale"), CShadow::GetTextureSize(), (float*)CShadow::GetShadowInvTexSizePointer());

	}
	else {
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, (GLfloat*)&m_ambient);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, (GLfloat*)&m_diffuse);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, (GLfloat*)&m_specular);
		if (mp_texture) {
			mp_texture->MapTexture();
		}
	}
}void CMaterial::Map()
{
	mp_shader->SendFragParam(this);
}
void CMaterial::Unmap(){
	if (mp_texture) {
		mp_texture->UnmapTexture();
	}
}
