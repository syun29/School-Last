
#include "EffekseerManager.h"

EffekseerManager *EffekseerManager::m_instance = nullptr;

EffekseerManager::EffekseerManager() :
	Base(eEffectManager)
{
	//エフェクトの先読み
	struct EFK_DATA {
		std::string name;
		std::u16string efk;
	}
	efk_data[] = {
		{ "Arrow1",u"Effect/tktk01/Arrow1.efk" },
		{ "Arrow2",u"Effect/tktk01/Arrow2.efk" },
		{ "Blow1",u"Effect/tktk01/Blow1.efk" },
		{ "Blow2",u"Effect/tktk01/Blow2.efk" },
		{ "Cure7",u"Effect/tktk01/Cure7.efk" },
	};


	// Create a manager of effects
	// エフェクトのマネージャーの作成
	m_manager = ::Effekseer::Manager::Create(8000);

	// Create a  graphics device
	// 描画デバイスの作成
	auto graphicsDevice = ::EffekseerRendererGL::CreateGraphicsDevice(::EffekseerRendererGL::OpenGLDeviceType::OpenGL3);
	// Create a renderer of effects
	// エフェクトのレンダラーの作成
	m_renderer = ::EffekseerRendererGL::Renderer::Create(graphicsDevice,8000);
	
	
	//std::shared_ptr<EffekseerRenderer::ExternalShaderSettings> shader_setting(new EffekseerRenderer::ExternalShaderSettings());
    //shader_setting->StandardShader = graphicsDevice->CreateShaderFromCodes({ model_unlit_vs_gl3 }, { model_unlit_ps_gl3 });
   // shader_setting->Blend = Effekseer::AlphaBlendType.Blend;
    //m_renderer->SetExternalShaderSettings(shader_setting);
	
	
	// Sprcify rendering modules
	// 描画モジュールの設定
	m_manager->SetSpriteRenderer(m_renderer->CreateSpriteRenderer());
	m_manager->SetRibbonRenderer(m_renderer->CreateRibbonRenderer());
	m_manager->SetRingRenderer(m_renderer->CreateRingRenderer());																							
	m_manager->SetModelRenderer(m_renderer->CreateModelRenderer());

	//右手座標系に設定
	m_manager->SetCoordinateSystem(Effekseer::CoordinateSystem::RH);

	// Specify a texture, model, curve and material loader
	// It can be extended by yourself. It is loaded from a file on now.
	// テクスチャ、モデル、カーブ、マテリアルローダーの設定する。
	// ユーザーが独自で拡張できる。現在はファイルから読み込んでいる。
	// 描画用インスタンスからテクスチャの読込機能を設定
	// 独自拡張可能、現在はファイルから読み込んでいる。
	m_manager->SetTextureLoader(m_renderer->CreateTextureLoader());
	m_manager->SetModelLoader(m_renderer->CreateModelLoader());
	m_manager->SetMaterialLoader(m_renderer->CreateMaterialLoader());
	m_manager->SetCurveLoader(Effekseer::MakeRefPtr<Effekseer::CurveLoader>());

	
	// 音再生用インスタンスの生成
	m_sound = EffekseerSound::Sound::Create(32);

	// 音再生用インスタンスから再生機能を指定
	m_manager->SetSoundPlayer(m_sound->CreateSoundPlayer());

	// 音再生用インスタンスからサウンドデータの読込機能を設定
	// 独自拡張可能、現在はファイルから読み込んでいる。
	m_manager->SetSoundLoader(m_sound->CreateSoundLoader());


	//エフェクトの先読み
	unsigned int efk_data_size = sizeof(efk_data) / sizeof(efk_data[0]);
	for (int i = 0; i < efk_data_size; i++) {
		EFK_DATA *d = &efk_data[i];
		for(int i=0;i< m_inctane_size;i++)
			m_effect_list[d->name].m_effets.push_back(Effekseer::Effect::Create(m_manager, d->efk.c_str()));
	}
	
	if (!m_instance) m_instance = this;
}

EffekseerManager::~EffekseerManager()
{
	// エフェクトの停止
	m_manager->StopAllEffects();

	for (auto& effects : m_effect_list) {
		for (auto& e : effects.second.m_effets) {
			// エフェクトの破棄しなくともclear時に破棄される
			//ES_SAFE_RELEASE(e);
			//e->Release();
			//break;
		}
		effects.second.m_effets.clear();
	}
	m_effect_list.clear();
	m_sound.Reset();
	m_renderer.Reset();
	m_manager.Reset();
	m_instance = nullptr;

}

void EffekseerManager::ClearInstance()
{
	if (!m_instance) return;
	EffekseerManager::GetInstance()->SetKill();
}
void EffekseerManager::Update() {
	// エフェクトの更新処理を行う
	//m_manager->Update();
	CVector3D pos = CCamera::GetCurrent()->GetPos();
	CVector3D at = pos + CCamera::GetCurrent()->GetDir();
	CVector3D up = CCamera::GetCurrent()->GetUp();
	m_sound->SetListener(Effekseer::Vector3D(pos.x, pos.y, pos.z), Effekseer::Vector3D(at.x, at.y, at.z), Effekseer::Vector3D(up.x, up.y, up.z));


	// Update the manager
	// マネージャーの更新
	Effekseer::Manager::UpdateParameter updateParameter;
	updateParameter.DeltaFrame = CFPS::GetDeltaTime() * 60;	
	m_manager->Update(updateParameter);

}
void EffekseerManager::Render()
{
	

	if (CShadow::isDoShadow()) return;
	// 投影行列を設定
	::Effekseer::Matrix44 mat;
	memcpy(mat.Values, CCamera::GetCurrent()->GetProjectionMatrix().f, sizeof(float) * 16);
	m_renderer->SetProjectionMatrix(mat);
	memcpy(mat.Values, CCamera::GetCurrent()->GetViewMatrix().f, sizeof(float) * 16);
	// カメラ行列を設定
	m_renderer->SetCameraMatrix(mat);

	
	// Render effects
	// エフェクトの描画を行う。
	m_renderer->BeginRendering();
	Effekseer::Manager::DrawParameter drawParameter;
	drawParameter.ZNear = 0.0f;
	drawParameter.ZFar = 1.0f;
	drawParameter.ViewProjectionMatrix = m_renderer->GetCameraProjectionMatrix();
	// エフェクトの描画を行う。
	m_manager->Draw(drawParameter);

	// エフェクトの描画終了処理を行う。
	m_renderer->EndRendering();
}

Effekseer::EffectRef EffekseerManager::GetEffect(const std::string & name)
{
	Effect* e = &m_effect_list[name];
	Effekseer::EffectRef ret = e->m_effets[e->m_index];
	e->m_index = (e->m_index + 1) % e->m_effets.size();
	return ret;
}
