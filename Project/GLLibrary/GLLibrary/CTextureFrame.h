#pragma once
#include <stack>
#include "CTexture.h"
#include "Vertex.h"
class CTextureFrame {
	CVector4D m_clear_color;
	std::vector<CTexture*> m_texture_list;	//!描画先テクスチャー
	CTexture *mp_depth;	//!深度テクスチャー
	GLuint m_frame;			//!フレームバッファ
	int m_width;
	int m_height;
	GLint viewport_old[4];
	static std::stack<CTextureFrame*> m_render_stack;

public:
	/**
	* @fn コンストラクタ
	* コンストラクタ
	* @brief フレームバッファと描画先テクスチャーを生成。
	* @param width	[in]　幅
	* @param height	[in]　高さ
	* @param clear_color	[in]　画面消去色

	*/
	CTextureFrame(int width, int height, const CVector4D& clear_color, int color_rayer = 1, GLenum* internal_formats = nullptr, GLenum* formats = nullptr, GLenum* types = nullptr);
	/**
	* @fn デストラクタ
	* デストラクタ
	* @brief バッファを破棄します。
	*/
	~CTextureFrame();
	/**
	* @fn 描画開始
	* 
	* @brief BeginDraw ～　EndDrawの間は、描画先がテクスチャーになります。
	*/
	void BeginDraw(bool clear=true);
	/**
	* @fn 描画終了
	*
	* @brief BeginDraw ～　EndDrawの間は、描画先がテクスチャーになります。
	*/
	void EndDraw();

	/*!
		@brief	テクスチャーの幅を取得
		@retval	テクスチャーの幅
	**/
	float GetWidth()const;
	/*!
		@brief	テクスチャー高さを取得
		@retval	テクスチャー高さ
	**/
	float GetHeight() const;
	/*!
		@brief	カラー用テクスチャーを取得
		@retval	テクスチャーのポインタ
	**/
	CTexture* GetTexture(int idx=0) const{
		return m_texture_list[idx];
	}

	/*!
		@brief	深度用テクスチャーを取得
		@retval	テクスチャーのポインタ
	**/
	CTexture* GetDepthTexture() const{
		return mp_depth;
	}

	/*!
		@brief	デバッグ用表示
		@param  x		[in]	位置X
		@param  y		[in]	位置Y
		@param  w		[in]	幅
		@param  h		[in]	高さ
		@param  target	[in]	描画テクスチャー
		@retval	テクスチャー高さ
	**/
	static void Draw(float x, float y, float w, float h, CTexture*target);
};
