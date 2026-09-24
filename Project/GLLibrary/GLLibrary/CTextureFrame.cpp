#include "CTextureFrame.h"
#include "CCamera.h"
#include "CShader.h"

std::stack<CTextureFrame*> CTextureFrame::m_render_stack;
CTextureFrame::CTextureFrame(int width, int height, const CVector4D& clear_color, int color_rayer, GLenum* internal_formats, GLenum* formats, GLenum* types) 
{
	m_clear_color = clear_color;
	m_width = width;
	m_height = height;
	for (int i = 0; i < color_rayer; i++) {
		//カラー用テクスチャ生成
		if (types)
			m_texture_list.push_back(new CTexture(width, height, internal_formats[i], formats[i], types[i]));
		else
			m_texture_list.push_back(new CTexture(width, height, GL_RGBA, GL_RGBA));
	}
	//深度バッファ用テクスチャ生成
	mp_depth = new CTexture(width, height, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT);

	//フレームバッファ作成
	glGenFramebuffersEXT(1, &m_frame);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_frame);

	for (int i = 0; i < color_rayer; i++) {
		//テクスチャー接続（カラー）
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT+i, GL_TEXTURE_2D, m_texture_list[i]->m_bufID, 0);
	}
	//テクスチャー接続（深度）
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, mp_depth->m_bufID, 0);


	//通常描画へ戻す
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

CTextureFrame::~CTextureFrame()
{

	glDeleteFramebuffers(1, &m_frame);
	for (auto& t : m_texture_list) {
		t->Release();
		delete t;
	}
	if(mp_depth) {
		mp_depth->Release();
		delete mp_depth;
	}

}

void CTextureFrame::BeginDraw(bool clear)
{
	//フレームバッファ有効化。以降の描画処理は mp_texture,mp_depthへ書き込まれる。
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_frame);
	const GLenum bufferList[] = {
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1,
		GL_COLOR_ATTACHMENT2,
		GL_COLOR_ATTACHMENT3,
		GL_COLOR_ATTACHMENT4,
		GL_COLOR_ATTACHMENT5,
		GL_COLOR_ATTACHMENT6,
		GL_COLOR_ATTACHMENT7,
	};
	glDrawBuffers((GLsizei)m_texture_list.size(), bufferList);
	//元のビューポートを保存
	glGetIntegerv(GL_VIEWPORT, viewport_old);
	//ビューポートをテクスチャーのサイズに設定
	glViewport(0, 0, m_width, m_height);

	float back_color[4];
	glGetFloatv(GL_COLOR_CLEAR_VALUE,back_color);
	if (clear) {
		//消去
		glClearColor(m_clear_color.r, m_clear_color.g, m_clear_color.b, m_clear_color.a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	//クリアカラーを元に戻す
	glClearColor(back_color[0], back_color[1], back_color[2], back_color[3]);
	m_render_stack.push(this);
}

void CTextureFrame::EndDraw()
{
	m_render_stack.pop();
	if (!m_render_stack.empty()) {
		CTextureFrame* t = m_render_stack.top();
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, t->m_frame);
		glViewport(0,0,t->m_width,t->m_height);
	}else {
		//m_blackへの書き込み終了
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		glDrawBuffer(GL_BACK);
		//ここまでの描画命令を一旦処理
		glFlush();
		glViewport(viewport_old[0], viewport_old[1], viewport_old[2], viewport_old[3]);
	}


}

float CTextureFrame::GetWidth()const
{
	return m_width;
}

float CTextureFrame::GetHeight()const
{
	return m_height;
}

void CTextureFrame::Draw(float x, float y, float w, float h, CTexture* target)
{

	glDisable(GL_CULL_FACE);
	//ライティング無効
	glDisable(GL_LIGHTING);
	//デプステスト無効
	glDisable(GL_DEPTH_TEST);

	//
	//正射投影の行列を作成
	//
	const CMatrix& mProj = CCamera::GetCurrent()->Get2DProjectionMatrix();



	CMatrix pvm = mProj * CMatrix::MTranselate(x, y, 0) * CMatrix::MScale(w, h, 1);

	CShader* s = CShader::GetInstance("Image");



	CVector3D vertices[] = {
		CVector3D(0,0,0),
		CVector3D(1,0,0),
		CVector3D(1,1,0),
		CVector3D(0,1,0)
	};
	CVector2D texcoord[] = {
		CVector2D(0,1),
		CVector2D(1,1),
		CVector2D(1,0),
		CVector2D(0,0)
	};
	CTexture* t = target;


	s->Enable();
	t->MapTexture();

	GLint samplerId = glGetUniformLocation(s->GetProgram(), "sampler");
	glUniform1i(samplerId, 0);//GL_TEXTURE0を適用

	glUniformMatrix4fv(glGetUniformLocation(s->GetProgram(), "PVWMatrix"), 1, GL_FALSE, pvm.f);
	glUniform4fv(glGetUniformLocation(s->GetProgram(), "Color"), 1, CVector4D(1,1,1,1).v);

	glEnableVertexAttribArray(CShader::eVertexLocation);
	glEnableVertexAttribArray(CShader::eTexcoordlLocation);
	glVertexAttribPointer(CShader::eVertexLocation, 3, GL_FLOAT, GL_FALSE, 0, vertices->v);
	glVertexAttribPointer(CShader::eTexcoordlLocation, 2, GL_FLOAT, GL_FALSE, 0, texcoord->v);


	glDrawArrays(GL_QUADS, 0, 4);

	glDisableVertexAttribArray(CShader::eVertexLocation);
	glDisableVertexAttribArray(CShader::eTexcoordlLocation);

	s->Disable();
	t->UnmapTexture();

	//元の設定に戻す

	glEnable(GL_CULL_FACE);
	//ライティング無効
	glEnable(GL_LIGHTING);
	//デプステスト無効
	glEnable(GL_DEPTH_TEST);



}
