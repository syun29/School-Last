#include "Utility.h"
#include "CCamera.h"
#include "CShader.h"
#include "CCollision.h"

CVector3D Utility::ScreenToWorld(const CVector3D &spos, const CMatrix &mProj,const CMatrix &mView, const CRect& viewport){
	
	CMatrix mViewport;
	mViewport.Viewport(viewport.m_x, viewport.m_y, viewport.m_width, viewport.m_height);
	CVector4D o = (mView.GetInverse() * mProj.GetInverse() * mViewport.GetInverse()) * CVector4D(spos.x, spos.y -GL::window_height + viewport.m_y+ viewport.m_height, spos.z, 1);
	return CVector3D(o.x/o.w,o.y/o.w,o.z/o.w);
}
CVector3D Utility::ScreenToWorld(const CVector3D &spos) {
	return ScreenToWorld(spos, CCamera::GetCurrent()->GetProjectionMatrix(), CCamera::GetCurrent()->GetViewMatrix(), CCamera::GetCurrent()->GetViewportRect());
}

CVector3D Utility::ScreenToWorld(const CVector3D &spos, const CCamera* camera) {
	return ScreenToWorld(spos, camera->GetProjectionMatrix(), camera->GetViewMatrix(), camera->GetViewportRect());
}

CVector3D Utility::WorldToScreen( const CVector3D &spos, const CMatrix &mProj, const CMatrix &mView, const CRect& viewport) {
	CMatrix mViewport;
//	mViewport.Viewport(viewport.m_x, viewport.m_y, viewport.m_width, viewport.m_height);
	mViewport.Viewport(0, 0, viewport.m_width, viewport.m_height);
	CVector4D o = mViewport * mProj * mView * CVector4D(spos.x,spos.y,spos.z,1);

	return  CVector3D(o.x/o.w, o.y/o.w,o.z/abs(o.w));

}
CVector3D Utility::WorldToScreen(const CVector3D &spos) {
	return WorldToScreen(spos, CCamera::GetCurrent()->GetProjectionMatrix(), CCamera::GetCurrent()->GetViewMatrix(), CCamera::GetCurrent()->GetViewportRect());
}
CVector3D Utility::WorldToScreen(const CVector3D &spos, const CCamera* camera) {
	return WorldToScreen(spos, camera->GetProjectionMatrix(), camera->GetViewMatrix(), camera->GetViewportRect());
}

void Utility::DrawLine(const CVector3D &s, const CVector3D &e, const  CVector4D &color,float lineWidth) {
	CMatrix pvm = CCamera::GetCurrent()->GetProjectionMatrix() * CCamera::GetCurrent()->GetViewMatrix();
	
	const CVector3D vertex[2] = { s,e };
	CShader* shader = CShader::GetInstance("Solid");
	shader->Enable();


	glUniformMatrix4fv(glGetUniformLocation(shader->GetProgram(), "PVWMatrix"), 1, GL_FALSE, pvm.f);
	glUniform4fv(glGetUniformLocation(shader->GetProgram(), "Color"), 1, color.v);

	glEnableVertexAttribArray(CShader::eVertexLocation);
	
	glVertexAttribPointer(CShader::eVertexLocation, 3, GL_FLOAT, GL_FALSE, 0, &vertex);


	glLineWidth(lineWidth);
	glDrawArrays(GL_LINES, 0, 2);
	glLineWidth(1.0f);

	glDisableVertexAttribArray(CShader::eVertexLocation);
	

	shader->Disable();




}
static const SVector3D cube_points[] = {
	-0.5f,-0.5f,-0.5f,  //v0
	 0.5f,-0.5f,-0.5f,  //v1
	 0.5f, 0.5f,-0.5f,  //v2
	-0.5f, 0.5f,-0.5f,  //v3

	-0.5f,-0.5f,0.5f, //v4
	 0.5f,-0.5f,0.5f, //v5
	 0.5f, 0.5f,0.5f, //v6
	-0.5f, 0.5f,0.5f, //v7
};
static const GLuint cube_index[] = {
	0,3,2,1,
	1,2,6,5,
	5,6,7,4,
	4,7,3,0,
	4,0,1,5,
	3,7,6,2
};

static const SVector3D quad_points[] = {
	0,0,0,  //v0
	1.0f,0,0,  //v1
	1.0f,1.0f,0,  //v2
	0,1.0f,0,  //v3
};
void Utility::DrawCube(const CVector3D &pos, const float size, const CVector4D &color) {
	DrawCube(CMatrix::MTranselate(pos)* CMatrix::MScale(size, size, size),color);
}
void Utility::DrawCube(const CVector3D& pos, const CVector3D& size, const CVector4D& color)
{
	DrawCube(CMatrix::MTranselate(pos) * CMatrix::MScale(size), color);
}
void Utility::DrawAABB(const CVector3D& min, const CVector3D& max, const CVector4D& color)
{
	CVector3D v = max - min;

	DrawCube(CMatrix::MTranselate(min + v/2) * CMatrix::MScale(v), color);
}
void Utility::DrawCube(const CMatrix &mat,  const CVector4D &color) {
	CMatrix pvm = CCamera::GetCurrent()->GetProjectionMatrix() * CCamera::GetCurrent()->GetViewMatrix() * mat;

	CShader* shader = CShader::GetInstance("Solid");
	shader->Enable();


	glUniformMatrix4fv(glGetUniformLocation(shader->GetProgram(), "PVWMatrix"), 1, GL_FALSE, pvm.f);
	glUniform4fv(glGetUniformLocation(shader->GetProgram(), "Color"), 1, color.v);

	glEnableVertexAttribArray(CShader::eVertexLocation);

	glVertexAttribPointer(CShader::eVertexLocation, 3, GL_FLOAT, GL_FALSE, 0, &cube_points);

	glDrawElements(GL_QUADS, 24, GL_UNSIGNED_INT, cube_index);


	glDisableVertexAttribArray(CShader::eVertexLocation);


	shader->Disable();
}
void Utility::DrawLine(const CVector2D &s, const CVector2D &e, const CVector4D &color) {
	

	//カリング無効
	glDisable(GL_CULL_FACE);
	//ライティング無効
	glDisable(GL_LIGHTING);
	//デプステスト無効
	glDisable(GL_DEPTH_TEST);
	//
	//正射投影の行列を作成
	//
	const CMatrix& mProj = CCamera::GetCurrent()->Get2DProjectionMatrix();


	const CVector3D vertex[2] = { s,e };
	CShader* shader = CShader::GetInstance("Solid");
	shader->Enable();


	glUniformMatrix4fv(glGetUniformLocation(shader->GetProgram(), "PVWMatrix"), 1, GL_FALSE, mProj.f);
	glUniform4fv(glGetUniformLocation(shader->GetProgram(), "Color"), 1, color.v);

	glEnableVertexAttribArray(CShader::eVertexLocation);

	glVertexAttribPointer(CShader::eVertexLocation, 3, GL_FLOAT, GL_FALSE, 0, &vertex);


	glDrawArrays(GL_LINES, 0, 2);

	glDisableVertexAttribArray(CShader::eVertexLocation);


	shader->Disable();

	
	glEnable(GL_CULL_FACE);
	
	glEnable(GL_LIGHTING);
	
	glEnable(GL_DEPTH_TEST);




}
void Utility::DrawQuad(const CVector2D &pos, const CVector2D &size, const CVector4D &color) {
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	//各種機能を無効に
	//カリング無効
	glDisable(GL_CULL_FACE);
	//ライティング無効
	glDisable(GL_LIGHTING);
	//デプステスト無効
	glDisable(GL_DEPTH_TEST);
	//
	//正射投影の行列を作成
	//
	const CMatrix& mProj = CCamera::GetCurrent()->Get2DProjectionMatrix();


	CMatrix pvm = mProj *CMatrix::MTranselate(pos)* CMatrix::MScale(size.x, size.y, 1);
	CShader* shader = CShader::GetInstance("Solid");
	shader->Enable();


	glUniformMatrix4fv(glGetUniformLocation(shader->GetProgram(), "PVWMatrix"), 1, GL_FALSE, pvm.f);
	glUniform4fv(glGetUniformLocation(shader->GetProgram(), "Color"), 1, color.v);

	glEnableVertexAttribArray(CShader::eVertexLocation);

	glVertexAttribPointer(CShader::eVertexLocation, 3, GL_FLOAT, GL_FALSE, 0, &quad_points);

	glDrawArrays(GL_QUADS,0, 4);


	glDisableVertexAttribArray(CShader::eVertexLocation);

	shader->Disable();

	glEnable(GL_CULL_FACE);

	glEnable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);

	glPopAttrib();

}

void Utility::DrawSphere(const CVector3D &pos, const float size, const CVector4D &color) {
	DrawSphere(CMatrix::MTranselate(pos), size, color);
}

void Utility::DrawSphere(const CMatrix &mat, const float size, const CVector4D &color) {
	const int cut = 12;
	CVector3D* vertex = new CVector3D[cut*cut*4];
	float *s = new float[cut+1];
	float *c = new float[cut+1];
	for (int i = 0; i <= cut; ++i) {
		s[i] = sinf((float)M_PI * 2 * i / cut);
		c[i] = cosf((float)M_PI * 2 * i / cut);
	}

	int idx = 0;
	for (int i = 0; i < cut; ++i) {
		const float& is1 = s[i];
		const float& is2 = s[i+1];
		const float& ic1 = c[i];
		const float& ic2 = c[i+1];
		for (int j = 0; j < cut; ++j) {
			const float& js1 = s[j];
			const float& js2 = s[j+1];
			const float& jc1 = c[j];
			const float& jc2 = c[j+1];

			vertex[idx] = CVector3D(js1*is1, ic1, jc1*is1);
			vertex[idx + 1] = CVector3D(js2*is1, ic1, jc2*is1);
			vertex[idx + 2] = CVector3D(js2*is2, ic2, jc2*is2);
			vertex[idx + 3] = CVector3D(js1*is2, ic2, jc1*is2);
			idx += 4;
		}
	}
	CMatrix pvm = CCamera::GetCurrent()->GetProjectionMatrix() * CCamera::GetCurrent()->GetViewMatrix() * mat*CMatrix::MScale(size, size, size);

	CShader* shader = CShader::GetInstance("Solid");
	shader->Enable();


	glUniformMatrix4fv(glGetUniformLocation(shader->GetProgram(), "PVWMatrix"), 1, GL_FALSE, pvm.f);
	glUniform4fv(glGetUniformLocation(shader->GetProgram(), "Color"), 1, color.v);

	glEnableVertexAttribArray(CShader::eVertexLocation);

	glVertexAttribPointer(CShader::eVertexLocation, 3, GL_FLOAT, GL_FALSE, 0, vertex);

	glDrawArrays(GL_QUADS,0,cut*cut*4);


	glDisableVertexAttribArray(CShader::eVertexLocation);


	shader->Disable();

	delete[] vertex;
	delete[] s;
	delete[] c;
}
void Utility::DrawCircle(const CVector2D & pos, const float size, const CVector4D & color)
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	//各種機能を無効に
	//カリング無効
	glDisable(GL_CULL_FACE);
	//ライティング無効
	glDisable(GL_LIGHTING);
	//デプステスト無効
	glDisable(GL_DEPTH_TEST);
	//
	//正射投影の行列を作成
	//
	const CMatrix& mProj = CCamera::GetCurrent()->Get2DProjectionMatrix();

	const int cut = 8;
	CVector3D* vertex = new CVector3D[cut + 2];
	int idx = 0;
	vertex[0] = CVector3D(0, 0, 0);
	idx++;
	for (int i = 0; i <= cut; ++i,++idx) {
		float s = sinf((float)M_PI * 2 * i / cut);
		float c = cosf((float)M_PI * 2 * i / cut );
		vertex[idx] = CVector3D(s, c, 0);
	}


	CMatrix pvm = mProj * CMatrix::MTranselate(pos)* CMatrix::MScale(size, size, 1);
	CShader* shader = CShader::GetInstance("Solid");
	shader->Enable();


	glUniformMatrix4fv(glGetUniformLocation(shader->GetProgram(), "PVWMatrix"), 1, GL_FALSE, pvm.f);
	glUniform4fv(glGetUniformLocation(shader->GetProgram(), "Color"), 1, color.v);

	glEnableVertexAttribArray(CShader::eVertexLocation);

	glVertexAttribPointer(CShader::eVertexLocation, 3, GL_FLOAT, GL_FALSE, 0, vertex);

	glDrawArrays(GL_TRIANGLE_FAN, 0, cut+2);


	glDisableVertexAttribArray(CShader::eVertexLocation);

	shader->Disable();

	glEnable(GL_CULL_FACE);

	glEnable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);

	glPopAttrib();

	delete[] vertex;

}
void Utility::DrawCapsule(const CVector3D &s, const CVector3D &e, const float rad, const CVector4D &color) {
	DrawSphere(s, rad, color);
	DrawSphere(e, rad, color);
	

	CMatrix pvm = CCamera::GetCurrent()->GetProjectionMatrix() * CCamera::GetCurrent()->GetViewMatrix();

	CShader* shader = CShader::GetInstance("Solid");
	shader->Enable();
	const int cut = 16;
	float l = (e - s).Length();
	CVector3D f = (e - s).GetNormalize();
	CVector3D u,r;
	if (float d = abs(CVector3D::Dot(f, CVector3D(0, 1, 0))) > 0.99f) {
		u = CVector3D::Cross(f, CVector3D(0, 0, d > 0 ? 1.0f:-1.0f)).GetNormalize();
	} else {
		u = CVector3D::Cross(f, CVector3D(0, 1, 0)).GetNormalize();
	}
	r = CVector3D::Cross(u, f).GetNormalize();
	CMatrix mat(r.x, u.x, f.x, 0,
				r.y, u.y, f.y, 0,
				r.z, u.z, f.z, 0,
				0, 0, 0, 1);

	CVector3D *vertex = new CVector3D[(cut+1)*2];
	for (int i = 0; i <= cut; i++) {
		CVector3D n = mat*CMatrix::MRotationZ(i* (float)M_PI * 2 / cut)*CVector3D(0, 1, 0);
		CVector3D ee = e + n * rad;

		vertex[i*2] = CVector3D(ee.x, ee.y, ee.z);
		CVector3D ss = s + n * rad;
		vertex[i * 2+1] = CVector3D(ss.x, ss.y, ss.z);

	}

	glUniformMatrix4fv(glGetUniformLocation(shader->GetProgram(), "PVWMatrix"), 1, GL_FALSE, pvm.f);
	glUniform4fv(glGetUniformLocation(shader->GetProgram(), "Color"), 1, color.v);

	glEnableVertexAttribArray(CShader::eVertexLocation);

	glVertexAttribPointer(CShader::eVertexLocation, 3, GL_FLOAT, GL_FALSE, 0, vertex);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, (cut+1) * 2);


	glDisableVertexAttribArray(CShader::eVertexLocation);


	shader->Disable();
	delete[] vertex;




}
void Utility::DrawCapsule(const CVector2D& s, const CVector2D& e, const float size, const CVector4D& color)
{

	//glPushAttrib(GL_ALL_ATTRIB_BITS);
	//各種機能を無効に
	//カリング無効
	glDisable(GL_CULL_FACE);
	//ライティング無効
	glDisable(GL_LIGHTING);
	//デプステスト無効
	glDisable(GL_DEPTH_TEST);
	//
	//正射投影の行列を作成
	//
	const CMatrix& mProj = CCamera::GetCurrent()->Get2DProjectionMatrix();

	CVector3D dir = (e - s).GetNormalize();
	float a = atan2(dir.x, dir.y);
	CMatrix m = CMatrix::MRotationZ(a);

	const int cut = 8;
	CVector3D* vertex = new CVector3D[cut + 2];
	int idx = 0;
	vertex[0] = CVector3D(0, 0, 0);
	idx++;
	for (int i = 0; i <= cut; ++i, ++idx) {
		float s = sin(M_PI / 2 - M_PI * i / cut);
		float c = cos(M_PI / 2 - M_PI * i / cut);
		vertex[idx] = CVector3D(s, c, 0);
	}

	CVector3D quad_vertex[4];
	CMatrix r = CMatrix::MRotationZ(-a);
	quad_vertex[0] = CVector3D(s) + r * CVector3D(size, 0, 0);
	quad_vertex[1] = CVector3D(s) + r * CVector3D(-size, 0, 0);
	quad_vertex[2] = CVector3D(e) + r * CVector3D(-size, 0, 0);
	quad_vertex[3] = CVector3D(e) + r * CVector3D(size, 0, 0);


	CMatrix pvm = mProj;
	CShader* shader = CShader::GetInstance("Solid");
	shader->Enable();

	glUniform4fv(glGetUniformLocation(shader->GetProgram(), "Color"), 1, color.v);

	glUniformMatrix4fv(glGetUniformLocation(shader->GetProgram(), "PVWMatrix"), 1, GL_FALSE, pvm.f);
	glEnableVertexAttribArray(CShader::eVertexLocation);
	glVertexAttribPointer(CShader::eVertexLocation, 3, GL_FLOAT, GL_FALSE, 0, quad_vertex);
	glDrawArrays(GL_QUADS, 0, 4);



	pvm = mProj * CMatrix::MTranselate(s) * CMatrix::MRotationZ(M_PI - a) * CMatrix::MScale(size, size, 1.0f);
	glUniformMatrix4fv(glGetUniformLocation(shader->GetProgram(), "PVWMatrix"), 1, GL_FALSE, pvm.f);
	glVertexAttribPointer(CShader::eVertexLocation, 3, GL_FLOAT, GL_FALSE, 0, vertex);
	glDrawArrays(GL_TRIANGLE_FAN, 0, cut + 2);



	pvm = mProj * CMatrix::MTranselate(e) * CMatrix::MRotationZ(-a) * CMatrix::MScale(size, size, 1);
	glUniformMatrix4fv(glGetUniformLocation(shader->GetProgram(), "PVWMatrix"), 1, GL_FALSE, pvm.f);
	glDrawArrays(GL_TRIANGLE_FAN, 0, cut + 2);


	glDisableVertexAttribArray(CShader::eVertexLocation);

	shader->Disable();

	glEnable(GL_CULL_FACE);

	glEnable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);

	//glPopAttrib();

	delete[] vertex;
}
void Utility::DrawCapsule(const CCapsule& capsule, const CVector4D& color)
{
	DrawCapsule(capsule.GetStartPoint(), capsule.GetEndPoint(), capsule.GetRadius(), color);
}
void Utility::DrawArrow(const CVector3D& pos, const CVector3D& rot, const float size, const CVector4D& color)
{

	static SVector3D points[] = {
			{-1,-1,-1},{1,-1,-1},{1,1,-1},{-1,1,-1},{0,0,1}
	};
	static int idx[] = {
		0,1,2,
		2,3,0,
		0,4,3,
		0,1,4,
		1,2,4,
		2,3,4
	};
	static int line_idx[] = {
		0,1,1,2,2,3,3,0,0,4,1,4,2,4,3,4
	};
	//カリング無効
	glDisable(GL_CULL_FACE);
	CMatrix pvm = CCamera::GetCurrent()->GetProjectionMatrix() * CCamera::GetCurrent()->GetViewMatrix() * CMatrix::MTranselate(pos) * CMatrix::MRotation(rot) * CMatrix::MScale(size, size, size);

	CShader* shader = CShader::GetInstance("Solid");
	shader->Enable();


	glUniformMatrix4fv(glGetUniformLocation(shader->GetProgram(), "PVWMatrix"), 1, GL_FALSE, pvm.f);

	glEnableVertexAttribArray(CShader::eVertexLocation);

	glVertexAttribPointer(CShader::eVertexLocation, 3, GL_FLOAT, GL_FALSE, 0, &points);

	glUniform4fv(glGetUniformLocation(shader->GetProgram(), "Color"), 1, CVector4D(0, 0, 0, 1).v);
	glDrawElements(GL_LINES, 2 * 8, GL_UNSIGNED_INT, line_idx);


	glUniform4fv(glGetUniformLocation(shader->GetProgram(), "Color"), 1, color.v);
	glDrawElements(GL_TRIANGLES, 3 * 6, GL_UNSIGNED_INT, idx);



	glDisableVertexAttribArray(CShader::eVertexLocation);


	shader->Disable();

	glEnable(GL_CULL_FACE);
}
void Utility::DrawArrow(const CVector2D& pos, const float ang, const float size, const CVector4D& color)
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	//各種機能を無効に
	//カリング無効
	glDisable(GL_CULL_FACE);
	//ライティング無効
	glDisable(GL_LIGHTING);
	//デプステスト無効
	glDisable(GL_DEPTH_TEST);
	//
	//正射投影の行列を作成
	//
	const CMatrix& mProj = CCamera::GetCurrent()->Get2DProjectionMatrix();
	static const SVector3D points[] = {
		0,1.0f,0,  //v0
		1.0f,-1.0f,0,  //v1
		-1.0f,-1.0f,0,  //v2
	};

	CMatrix pvm = mProj * CMatrix::MTranselate(pos) * CMatrix::MRotationZ(-ang) * CMatrix::MScale(size, size, 1);
	CShader* shader = CShader::GetInstance("Solid");
	shader->Enable();


	glUniformMatrix4fv(glGetUniformLocation(shader->GetProgram(), "PVWMatrix"), 1, GL_FALSE, pvm.f);
	glUniform4fv(glGetUniformLocation(shader->GetProgram(), "Color"), 1, color.v);

	glEnableVertexAttribArray(CShader::eVertexLocation);

	glVertexAttribPointer(CShader::eVertexLocation, 3, GL_FLOAT, GL_FALSE, 0, points);

	glDrawArrays(GL_TRIANGLES, 0, 3);


	glDisableVertexAttribArray(CShader::eVertexLocation);

	shader->Disable();

	glEnable(GL_CULL_FACE);

	glEnable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);

	glPopAttrib();

}
void Utility::DrawOBB(const COBB& obb, const CVector4D& color)
{
	obb.Draw(color);
}
void Utility::DrawTriangle(const CVector3D v[], int count, const CVector4D& color, const CMatrix& mat)
{
	CMatrix pvm = CCamera::GetCurrent()->GetProjectionMatrix() * CCamera::GetCurrent()->GetViewMatrix();

	CShader* shader = CShader::GetInstance("Solid");
	shader->Enable();


	glUniformMatrix4fv(glGetUniformLocation(shader->GetProgram(), "PVWMatrix"), 1, GL_FALSE, pvm.f);
	glUniform4fv(glGetUniformLocation(shader->GetProgram(), "Color"), 1, color.v);

	glEnableVertexAttribArray(CShader::eVertexLocation);

	glVertexAttribPointer(CShader::eVertexLocation, 3, GL_FLOAT, GL_FALSE, 0, v);

	glDrawArrays(GL_TRIANGLES, 0, count);


	glDisableVertexAttribArray(CShader::eVertexLocation);


	shader->Disable();
}
void Utility::DrawQuad(const CVector3D v[], int count, const CVector4D& color, const CMatrix& mat)
{
	CMatrix pvm = CCamera::GetCurrent()->GetProjectionMatrix() * CCamera::GetCurrent()->GetViewMatrix() * mat;

	CShader* shader = CShader::GetInstance("Solid");
	shader->Enable();


	glUniformMatrix4fv(glGetUniformLocation(shader->GetProgram(), "PVWMatrix"), 1, GL_FALSE, pvm.f);
	glUniform4fv(glGetUniformLocation(shader->GetProgram(), "Color"), 1, color.v);

	glEnableVertexAttribArray(CShader::eVertexLocation);

	glVertexAttribPointer(CShader::eVertexLocation, 3, GL_FLOAT, GL_FALSE, 0, v);

	glDrawArrays(GL_QUADS, 0, count);


	glDisableVertexAttribArray(CShader::eVertexLocation);


	shader->Disable();
}
void Utility::DrawCircle(const CVector2D vertex[], const int count, const CVector2D& pos, const CVector4D& color)
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	//各種機能を無効に
	//カリング無効
	glDisable(GL_CULL_FACE);
	//ライティング無効
	glDisable(GL_LIGHTING);
	//デプステスト無効
	glDisable(GL_DEPTH_TEST);
	//
	//正射投影の行列を作成
	//
	const CMatrix& mProj = CCamera::GetCurrent()->Get2DProjectionMatrix();

	CMatrix pvm = mProj * CMatrix::MTranselate(pos);
	CShader* shader = CShader::GetInstance("Solid");
	shader->Enable();


	glUniformMatrix4fv(glGetUniformLocation(shader->GetProgram(), "PVWMatrix"), 1, GL_FALSE, pvm.f);
	glUniform4fv(glGetUniformLocation(shader->GetProgram(), "Color"), 1, color.v);

	glEnableVertexAttribArray(CShader::eVertexLocation);

	glVertexAttribPointer(CShader::eVertexLocation, 2, GL_FLOAT, GL_FALSE, 0, vertex);

	glDrawArrays(GL_LINE_STRIP, 0, count);


	glDisableVertexAttribArray(CShader::eVertexLocation);

	shader->Disable();

	glEnable(GL_CULL_FACE);

	glEnable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);

	glPopAttrib();


}

void Utility::DrawSector(const CMatrix& mat, const float start, const float end, const float size, const CVector4D& color) {
	const int cut = 45;
	CVector3D* vertex = new CVector3D[cut * 3];
	float* s = new float[cut + 1];
	float* c = new float[cut + 1];
	for (int i = 0; i <= cut; ++i) {
		s[i] = sinf(Lerp(start, end, (float)i / cut));
		c[i] = cosf(Lerp(start, end, (float)i / cut));
	}

	int idx = 0;
	for (int i = 0; i < cut; ++i)
	{
		const float& is1 = s[i];
		const float& is2 = s[i + 1];
		const float& ic1 = c[i];
		const float& ic2 = c[i + 1];
		vertex[idx] = CVector3D::zero;
		vertex[idx + 1] = CVector3D(is1, 0.0f, ic1);
		vertex[idx + 2] = CVector3D(is2, 0.0f, ic2);
		idx += 3;
	}
	CMatrix pvm = CCamera::GetCurrent()->GetProjectionMatrix() * CCamera::GetCurrent()->GetViewMatrix() * mat * CMatrix::MScale(size, 1.0f, size);

	CShader* shader = CShader::GetInstance("Solid");
	shader->Enable();


	glUniformMatrix4fv(glGetUniformLocation(shader->GetProgram(), "PVWMatrix"), 1, GL_FALSE, pvm.f);
	glUniform4fv(glGetUniformLocation(shader->GetProgram(), "Color"), 1, color.v);

	glEnableVertexAttribArray(CShader::eVertexLocation);

	glVertexAttribPointer(CShader::eVertexLocation, 3, GL_FLOAT, GL_FALSE, 0, vertex);

	glDrawArrays(GL_TRIANGLES, 0, cut * 3);


	glDisableVertexAttribArray(CShader::eVertexLocation);


	shader->Disable();

	delete[] vertex;
	delete[] s;
	delete[] c;
}
void Utility::DrawSmoothCircle(const CVector3D& pos, const float size, const CVector4D& color){
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	//各種機能を無効に
	//カリング無効
	glDisable(GL_CULL_FACE);
	//ライティング無効
	glDisable(GL_LIGHTING);
	//デプステスト無効
	glDisable(GL_DEPTH_TEST);
	//
	//正射投影の行列を作成
	//

	const int cut = 32;
	CVector3D* vertex = new CVector3D[cut + 2];
	int idx = 0;
	vertex[0] = CVector3D(0.0f, 0.0f, 0.0f);
	idx++;
	for (int i = 0; i <= cut; ++i, ++idx) {
		float s = sinf((float)M_PI * 2 * i / cut);
		float c = cosf((float)M_PI * 2 * i / cut);
		vertex[idx] = CVector3D(s, 0.0f, c);
	}

	CMatrix pvm = CCamera::GetCurrent()->GetProjectionMatrix() * CCamera::GetCurrent()->GetViewMatrix() * CMatrix::MTranselate(pos) * CMatrix::MScale(size, size, size);

	CShader* shader = CShader::GetInstance("Solid");
	shader->Enable();

	glUniformMatrix4fv(glGetUniformLocation(shader->GetProgram(), "PVWMatrix"), 1, GL_FALSE, pvm.f);
	glUniform4fv(glGetUniformLocation(shader->GetProgram(), "Color"), 1, color.v);

	glEnableVertexAttribArray(CShader::eVertexLocation);

	glVertexAttribPointer(CShader::eVertexLocation, 3, GL_FLOAT, GL_FALSE, 0, vertex);

	glDrawArrays(GL_TRIANGLE_FAN, 0, cut + 2);

	glDisableVertexAttribArray(CShader::eVertexLocation);

	shader->Disable();

	glEnable(GL_CULL_FACE);

	glEnable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);

	glPopAttrib();

	delete[] vertex;
}



//
//void Utility::DrawCapsule(CVector2D & s, CVector2D &e, float size, CVector4D & color)
//{
//	glPushAttrib(GL_ALL_ATTRIB_BITS);
//	//各種機能を無効に
//	//カリング無効
//	glDisable(GL_CULL_FACE);
//	//ライティング無効
//	glDisable(GL_LIGHTING);
//	//デプステスト無効
//	glDisable(GL_DEPTH_TEST);
//	//
//	//正射投影の行列を作成
//	//
//	CMatrix	mProj = CMatrix(
//		2.0f / CCamera::GetCurrent()->GetWhidth(), 0.0f, 0.0f, -1.0f,
//		0.0f, -2.0f / CCamera::GetCurrent()->GetHeight(), 0.0f, 1.0f,
//		0.0f, 0.0f, 1.0f, 0.0f,
//		0.0f, 0.0f, 0.0f, 1.0f
//	);
//
//	//投影行列を専用の物に設定
//	glMatrixMode(GL_PROJECTION);
//	glPushMatrix();
//	glLoadMatrixf(mProj.f);
//
//	//モデル＆ビュー行列を再設定
//	glMatrixMode(GL_MODELVIEW);
//	glPushMatrix();
//
//	glLoadIdentity();
//	//カラー設定
//	glColor4fv(color.v);
//
//	CVector2D v = e - s;
//	CVector2D dir = v.GetNormalize();
//	CVector3D cross = CVector3D::Cross(CVector3D(dir.x, dir.y, 0), CVector3D(0, 0, 1));
//	CVector2D c = CVector2D(cross.x, cross.y);
//	CVector2D *p[2] = { &s,&e };
//	float ang = atan2(c.x, c.y);
//
//	glDisable(GL_CULL_FACE);
//	glCullFace(GL_NONE);
//	glBegin(GL_QUADS);
//	glVertex2fv(CVector2D(s + c * size).v);
//	glVertex2fv(CVector2D(s + c * -size).v);
//	glVertex2fv(CVector2D(e + c * -size).v);
//	glVertex2fv(CVector2D(e + c * size).v);
//	glEnd();
//
//	for (int i = -1, idx = 0; i < 2; i += 2, idx++) {
//		glBegin(GL_TRIANGLE_FAN);
//		glVertex2fv(CVector2D(*p[idx]).v);
//		for (int j = 0; j <= 8; j++) {
//			float a = ang + j * M_PI / 8;
//			glVertex2fv(CVector2D(*p[idx] + CVector2D(sin(a), cos(a))*(-i * size)).v);
//		}
//		glEnd();
//	}
//	//元の行列と設定に戻す
//	glPopMatrix();
//	glMatrixMode(GL_PROJECTION);
//	glPopMatrix();
//	glMatrixMode(GL_MODELVIEW);
//
//
//	glPopAttrib();
//
//
//
//}
