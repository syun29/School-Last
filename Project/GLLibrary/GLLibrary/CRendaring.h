#pragma once
#include "../GLLibrary.h"


class CRendaring {
public:
	enum class RendringType {
		eNone,
		eForward,
		eDeferred,
	};
protected:
	static RendringType m_type;
	CVector3D m_focus;
	bool m_edge;
	bool m_dof;
	bool m_glare;
	float m_glare_pow;
	float m_glare_pix;
	int m_width, m_height;
	static CRendaring* m_instance;
	void DrawFillQuad();
	void Blur(CTexture* src, CTextureFrame* tmp, CTextureFrame* result, float pow, float pix = 1.0f);
public:

	CRendaring(int screen_width, int screen_height, CVector3D focus);
	virtual ~CRendaring();

	static void CreatInstance(int screen_width, int screen_height, CVector3D focus = CVector3D(0.2f,0.85f,0.0f), RendringType type = RendringType::eForward);
	static CRendaring* GetInstance();
	static void ClearInstance();

	virtual void Render(std::function<void()> render,bool update_shadow=true) = 0;

	void SetFocus(CVector3D f) {
		m_focus = f;
	}
	void SetEdge(bool edge) { m_edge = edge; }
	void SetDOF(bool dof) { m_dof = dof; }
	void SetGlare(bool glare, float pow, float pix) {
		m_glare = glare; m_glare_pow = pow; m_glare_pix = pix;
	}
	static RendringType GetType() {
		return m_type;
	}
	static bool CheckType(RendringType type) {
		return (type == m_type);
	}

};


class CFowardRendering :public CRendaring {
private:

	CTextureFrame* m_gbuffer;
	CTextureFrame* m_scene, * m_tmp_buffer;
	CTextureFrame* m_blurtmp, * m_blur, * m_blur1, * m_blur2;
public:

	CFowardRendering(int screen_width, int screen_height, CVector3D focus);
	~CFowardRendering();
	void Render(std::function<void()> render,bool update_shadow=true);

};


class CDeferredRendaring :public CRendaring {
private:

	CTextureFrame* m_gbuffer;
	CTextureFrame* m_tmp_buffer;
	CTextureFrame* m_ligting_buffer;
	CTextureFrame* m_scene;
	CTextureFrame* m_blurtmp, *m_blur, *m_blur1, *m_blur2;
public:

	CDeferredRendaring(int screen_width, int screen_height, CVector3D focus);
	~CDeferredRendaring();

	void Render(std::function<void()> render,bool update_shadow=true);

};