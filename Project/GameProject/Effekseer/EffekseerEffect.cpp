#include "EffekseerEffect.h"
#include <functional>

void EffekseerEffect::RemoveCallback(Effekseer::Manager*, Effekseer::Handle, bool)
{
	

}

EffekseerEffect::EffekseerEffect(const std::string &name, const CVector3D& pos, const CVector3D& rot, const CVector3D& scale, int start,int end,bool loop,CMatrix* parent) :
	Base(eEffect)
{
	m_name = name;
	m_ref = EffekseerManager::GetInstance()->GetEffect(name);
	//エフェクトを発生。ハンドルを取得する。
	m_handle = EffekseerManager::GetInstance()->GetManager()->Play(m_ref, Effekseer::Vector3D(pos.x, pos.y, pos.z), m_start);
	//位置、回転値、拡縮値、親を設定
	m_pos = pos;
	m_rot = rot;
	m_scale = scale;
	m_parent = parent;
	m_loop = loop;
	m_start = start;
	m_end = end;
	m_frame = 0;
	m_generate = false;
	//行列を更新
	UpdateMatrix();
	EffekseerManager::GetInstance()->GetManager()->SetRemovingCallback(m_handle, std::bind(&EffekseerEffect::RemoveCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

}
EffekseerEffect::~EffekseerEffect() {
	// エフェクトの停止
	if(EffekseerManager::isInstance()) EffekseerManager::GetInstance()->GetManager()->StopEffect(m_handle);
}
void EffekseerEffect::Update() {

	

	if (m_loop > 0) {


		m_frame += CFPS::GetDeltaTime() * 60;
		if (m_frame >= m_end-1) {
			EffekseerManager::GetInstance()->GetManager()->StopEffect(m_handle);
			m_generate = true;
			m_frame = 0;
		}

		if (m_generate) {
			m_handle = EffekseerManager::GetInstance()->GetManager()->Play(m_ref, Effekseer::Vector3D(m_pos.x, m_pos.y, m_pos.z), m_start);
			m_generate = false;
		}
	}
	else {
		//再生終了チェック
		if (!EffekseerManager::isInstance() || !EffekseerManager::GetInstance()->GetManager()->Exists(m_handle)) {
			SetKill();
		}
	}

}
void EffekseerEffect::Render() {
	//行列の更新
	UpdateMatrix();
	//描画はEffectManagerまかせ

}

void EffekseerEffect::UpdateMatrix()
{
	//モデル行列を作成
	m_matrix = CMatrix::MTranselate(m_pos) * CMatrix::MRotation(m_rot) * CMatrix::MScale(m_scale);

	//親がいる場合
	if (m_parent) {
		//子になるよう親の行列を掛ける
		m_matrix = *m_parent * m_matrix;
	}
	//Effekseer用行列にコピー
	Effekseer::Matrix43 mat;
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 3; j++)
			mat.Value[i][j] = m_matrix.m[i][j];

	//モデル行列を反映
	EffekseerManager::GetInstance()->GetManager()->SetMatrix(m_handle, mat);
}
