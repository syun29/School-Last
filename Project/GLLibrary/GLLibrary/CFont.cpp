#include "CFont.h"
#include "CCamera.h"
#include <locale.h>

std::map<std::string, CFont*> CFont::m_instances;
//コンストラクタ フォント作成
CFont::CFont(const char *font, int size)
{
	
	if (!m_font) {
		m_size = size;
		m_font = new FTPixmapFont(font);
		if (m_font->Error()) {
			delete m_font;
			m_font = nullptr;
		}
		else {
			m_font->FaceSize(m_size);
		}
	}

}

void CFont::Draw(int x,int y,float r,float g,float b, const char *format, ...)
{

	if (!m_font) 
		return;
	char buf[256];
	va_list ap;
	int length = 0;


	if (format == NULL)
	{
		return;
	}
	////文字列変換 
	//va_start(ap, format);
	//vswprintf_s(buf, format, ap);
	//va_end(ap);

	//文字列変換 
	va_start(ap, format);
	vsprintf_s(buf, format, ap);
	va_end(ap);

	setlocale(LC_CTYPE, "jpn");
	int size = (int)_mbstrlen(buf);
	wchar_t* wbuf = new wchar_t[size + 1];

	MultiByteToWideChar(CP_ACP, 0, buf, -1, wbuf, sizeof(wchar_t) * size + 1);

	Draw(x, y, r, g, b, wbuf);

	delete wbuf;
};
void CFont::Draw(int x, int y, float r, float g, float b, const wchar_t *format, ...)
{

	if (!m_font)
		return;
	wchar_t wbuf[256];
	va_list ap;
	int length = 0;


	if (format == NULL)
	{
		return;
	}
	////文字列変換 
	va_start(ap, format);
	vswprintf_s(wbuf, format, ap);
	va_end(ap);


	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	const CMatrix& mProj = CCamera::GetCurrent()->Get2DProjectionMatrix();
	glLoadMatrixf(mProj.f);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();




	glColor4f(r, g, b, 1.0f);
	glRasterPos2i(x, y);
	//glTranslatef(x, y, 0);
	m_font->Render(wbuf);



	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);




	glEnable(GL_CULL_FACE);	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);


}
void CFont::Release() {
	if(m_font) delete m_font;
}

void CFont::SetFontSize(int size)
{
	if (m_font == nullptr) return;
	m_size = size;
	m_font->FaceSize(m_size);
}

int CFont::GetTextWidth(const wchar_t* text)
{
	if (m_font && text)
	{
		return m_font->Advance(text);
	}
	return 0;
}

int CFont::GetTextHeight() const
{
	if (m_font)
	{
		return m_font->LineHeight();
	}
	return 0;
}

CFont* CFont::GetInstance(std::string name)
{
	if (m_instances.count(name) == 0) m_instances[name] = new CFont();
	return m_instances[name];
}

CFont* CFont::CreateInstance(std::string name, const char* font, int size)
{
	m_instances[name] = new CFont(font, size);
	return m_instances[name];
}

void CFont::ClearInstance()
{
	for (auto& o : m_instances) {
		delete o.second;
	}
	m_instances.clear();
}
