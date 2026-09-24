#pragma once
#include "GL.h"
#include <map>
#include <string>
#include <FTGL/ftgl.h>
#include "CResource.h"

#define DEFAULT_FONT "C:\\Windows\\Fonts\\msgothic.ttc"
#pragma comment(lib,"ftgl_static.lib")
#pragma comment(lib,"freetype.lib")

class CFont : public  CResource {
private:
	FTPixmapFont* m_font;
	int m_size;
	static std::map<std::string, CFont*> m_instances;
public:
	/*!
	@brief	コンストラクタ
	@param	font			[in]	フォント
	@param	size			[in]	サイズ
	**/
	CFont(const char *font_path = DEFAULT_FONT, int size=24);
	/*!
	@brief	文字の表示
	@param	x				[in]	横位置
	@param	y				[in]	縦位置
	@param	r				[in]	色（赤）
	@param	g				[in]	色（緑）
	@param	b				[in]	色（青）
	@param	format			[in]	文字列、書式
	**/
	void Draw(int x,int y,float r,float g,float b, const char *format, ...);
	void Draw(int x, int y, float r, float g, float b, const wchar_t *format, ...);

	/*!
	@brief	解放
	**/
	void Release();

	void SetFontSize(int size);

	//自分で追加しました↓------------
	int GetTextWidth(const wchar_t* text);
	int GetTextHeight() const;
	//-------------------------------

	static CFont* GetInstance(std::string name);
	static CFont* CreateInstance(std::string name, const char* font = "ＭＳゴシック", int size = 24);
	static void ClearInstance();
};

#define FONT_T()	CFont::GetInstance("default")
#define FONT(name)	CFont::GetInstance(name)
#define CREATE_FONT(name)	CFont::CreateInstance(name)
#define CREATE_FONT_F(name,font,size)	CFont::CreateInstance(name,font,size)