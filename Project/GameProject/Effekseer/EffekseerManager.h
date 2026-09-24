#pragma once

/*!
*	@brief	エフェクト管理クラス
*
*	エフェクト管理クラス
*/
#include "../Base/Base.h"
#include <map>
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
#define AL_LIBTYPE_STATIC
#include <AL/alc.h>

#include <EffekseerRendererGL.h>
#include <EffekseerSoundAL.h>
#ifdef _DEBUG
#pragma comment(lib, "Effekseerd.lib")
#pragma comment(lib, "EffekseerRendererGLd.lib")
#pragma comment(lib, "EffekseerSoundALd.lib")
#else
#pragma comment(lib, "Effekseer.lib")
#pragma comment(lib, "EffekseerRendererGL.lib")
#pragma comment(lib, "EffekseerSoundAL.lib")
#endif




class EffekseerManager : public Base {
	//同時発生数
	static const int	m_inctane_size = 10;
	Effekseer::ManagerRef			m_manager;
	::EffekseerRendererGL::RendererRef	m_renderer;
	EffekseerSound::SoundRef		m_sound;
	struct Effect {
		std::vector<Effekseer::EffectRef> m_effets;
		unsigned int		m_index;
		Effect() : m_index(0) {}
	};
	std::map<std::string, Effect> m_effect_list;
	static EffekseerManager* m_instance;

public:
	/*!
	@brief	コンストラクタ　マネージャーの初期化を行う
	**/
	EffekseerManager();

	/*!
	@brief	デストラクタ　マネージャーの破棄を行う
	**/
	~EffekseerManager();

	/*!
	@brief	生成したエフェクトマネージャーを取得する
	@retval	エフェクトマネージャーのポインタ
	**/
	static EffekseerManager* GetInstance() {
		if (!m_instance) Base::Add(m_instance = new EffekseerManager());
		return m_instance;
	}
	static bool isInstance() {
		return (m_instance);
	}

	/*!
	@brief	エフェクトマネージャーを破棄する
			Taskに組み込んでいるため、破棄自体はTaskManagerが行う
	@retval	無し
	**/
	static void ClearInstance();

	/*!
	@brief	すべてのエフェクトの更新を行う
	@retval 無し
	**/
	void Update();

	/*!
	@brief	すべてのエフェクトの描画を行う
	@retval 無し
	**/
	void Render();
	/*!
	@brief	読み込んだエフェクトを取得する
	@param	name		[in] 読み込み時に設定した名前
	@retval	エフェクトオブジェクトのポインタ
	**/
	Effekseer::EffectRef GetEffect(const std::string& name);


	/*!
	@brief	Effekseerのマネージャーを取得する
	@retval Effekseer::Managerのポインタ
	**/
	Effekseer::ManagerRef GetManager() const {
		return m_manager;
	}
};