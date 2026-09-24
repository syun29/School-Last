#include "CModelObj.h"
#include "CCollision.h"
#include "CCamera.h"
#include <string>

CMeshObj::CMeshObj() :m_buffer(0), mp_vertex_array(NULL), m_normal(NULL), m_vertexNum(0), m_colmask(NULL), m_colmin(NULL), m_colmax(NULL), m_colIdex(NULL),m_max(-1e10, -1e10, -1e10),m_min(1e10, 1e10, 1e10){
}
void CMeshObj::Render(CMaterial* m) {

	if (!m_buffer) {

		glGenBuffers(1, &m_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(SVertex) * m_vertexNum, mp_vertex_array, GL_STATIC_DRAW);
//		delete[] mp_vertex_array;
//		mp_vertex_array = nullptr;
//		mp_vertex_array = nullptr;

	}
	m->Map();
	///glBindVertexArray(m_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
	glEnableVertexAttribArray(CShader::eVertexLocation);
	glEnableVertexAttribArray(CShader::eNormalLocation);
	glEnableVertexAttribArray(CShader::eTexcoordlLocation);
	glEnableVertexAttribArray(CShader::eTangentLocation);



	glVertexAttribPointer(CShader::eVertexLocation, 3, GL_FLOAT, GL_FALSE, sizeof(CMeshObj::SVertex), 0);
	glVertexAttribPointer(CShader::eNormalLocation, 3, GL_FLOAT, GL_FALSE, sizeof(CMeshObj::SVertex), (void*)sizeof(CVector3D));
	glVertexAttribPointer(CShader::eTexcoordlLocation, 2, GL_FLOAT, GL_FALSE, sizeof(CMeshObj::SVertex), (void*)(sizeof(CVector3D) + sizeof(CVector3D)));
	glVertexAttribPointer(CShader::eTangentLocation, 3, GL_FLOAT, GL_FALSE, sizeof(CMeshObj::SVertex), (void*)(sizeof(CVector3D) + sizeof(CVector3D) + sizeof(CVector2D)));

	glDrawArrays(GL_TRIANGLES,0,m_vertexNum);

	glDisableVertexAttribArray(CShader::eVertexLocation);
	glDisableVertexAttribArray(CShader::eNormalLocation);
	glDisableVertexAttribArray(CShader::eTexcoordlLocation);
	glDisableVertexAttribArray(CShader::eTangentLocation);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glBindVertexArray(0);
	m->Unmap();

	
}
void CMeshObj::Release() {
	if(mp_vertex_array)  {delete[] mp_vertex_array; mp_vertex_array = nullptr;}
	if (m_normal) { delete[] m_normal; m_normal = nullptr; }
	if (m_colmask) {		delete m_colmask; m_colmask = nullptr; }
	if (m_colmin) {		delete[] m_colmin; m_colmin = nullptr;	}
	if (m_colmax) {		delete[] m_colmax; m_colmax = nullptr;}
	if (m_colIdex) {		delete[] m_colIdex; m_colIdex = nullptr;}
	if (m_buffer) {
		glDeleteBuffers(1, &m_buffer);
		m_buffer = 0;
	}
	//glDeleteVertexArrays(1, &m_vao);
}
void CMeshObj::CreatCollsionIndex(CVector3D cut){


	SVertex *v = mp_vertex_array;
	m_cut = cut;

	
	m_colmask = new unsigned long[(m_vertexNum / 3 / 32)+1];

	m_length.x = max(0.0f, (m_max.x - m_min.x) / m_cut.ix);
	m_length.y = max(0.0f, (m_max.y - m_min.y) / m_cut.iy);
	m_length.z = max(0.0f, (m_max.z - m_min.z) / m_cut.iz);
	//printf("cut_length(%f %f %f)\n", m_length.x, m_length.y, m_length.z);
	int xyz = m_cut.ix*m_cut.iy*m_cut.iz;
	m_colmin = new CVector3D[xyz];
	m_colmax = new CVector3D[xyz];
	m_colIdex = new std::vector<int>[xyz];
	for (int k = 0; k < m_cut.iy; k++) {
		for (int i = 0; i < m_cut.iz; i++) {
			for (int j = 0; j < m_cut.ix; j++) {
				int idx = GetIndex(j,k,i);
				m_colmin[idx] = CVector3D(m_min.x + j * m_length.x, m_min.y + k * m_length.y, m_min.z + i * m_length.z);
				m_colmax[idx] = CVector3D(m_min.x + (j + 1) * m_length.x, m_min.y + (k + 1)*m_length.y, m_min.z + (i + 1)*m_length.z);

			}
		}
	}
	m_normal = new CVector3D[m_vertexNum / 3];
	for (int i = 0; i < m_vertexNum; i += 3, v += 3) {
		
		float l = fminf(fminf(v[0].v.x, v[1].v.x), v[2].v.x);
		float r = fmaxf(fmaxf(v[0].v.x, v[1].v.x), v[2].v.x);
		float t = fmaxf(fmaxf(v[0].v.z, v[1].v.z), v[2].v.z);
		float b = fminf(fminf(v[0].v.z, v[1].v.z), v[2].v.z);
		float u = fmaxf(fmaxf(v[0].v.y, v[1].v.y), v[2].v.y);
		float d = fminf(fminf(v[0].v.y, v[1].v.y), v[2].v.y);

		CVector3D e1 = v[1].v - v[0].v;
		CVector3D e2 = v[2].v - v[0].v;

		m_normal[i / 3] = CVector3D::Cross(e1, e2).GetNormalize();
		
		CVector3D s, e;
		s.ix = max(0, min(m_cut.ix-1,(int)((l - m_min.x) / m_length.x)));
		e.ix = max(0, min(m_cut.ix - 1, (int)((r - m_min.x) / m_length.x)));

		s.iz = max(0, min(m_cut.iz - 1, (int)((b - m_min.z) / m_length.z)));
		e.iz = max(0, min(m_cut.iz - 1, (int)((t - m_min.z) / m_length.z)));

		s.iy = max(0, min(m_cut.iy - 1, (int)((d - m_min.y) / m_length.y)));
		e.iy = max(0, min(m_cut.iy - 1, (int)((u - m_min.y) / m_length.y)));

		for (int lineY = s.iy; lineY <= e.iy; lineY++) {
			for (int lineZ = s.iz; lineZ <= e.iz; lineZ++) {
				for (int lineX = s.ix; lineX <= e.ix; lineX++) {
					int idx = GetIndex(lineX, lineY, lineZ);
					m_colIdex[idx].push_back(i / 3);
				}
			}
		}

	}
	
	for (int k = 0; k < m_cut.iy; k++) {
		for (int i = 0; i < m_cut.iz; i++) {
			for (int j = 0; j < m_cut.ix; j++) {
				int idx = GetIndex(j, k, i);
			//	printf("%d ", m_colIdex[idx].size());
			}
		//	printf("\n");
		}
	//	printf("\n");
	}
				

}
bool CMeshObj::CollisionRay(CVector3D *c, CVector3D *n, const CVector3D &s, const CVector3D &e,const CVector3D& dir, float *pLength){
	/*SVertex* v = nullptr;
	if (m_buffer) {
		glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
		v = (SVertex*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
	}else {
		v = mp_vertex_array;
	}*/

	SVertex *v = mp_vertex_array;
	memset(m_colmask, 0, sizeof(unsigned long)*((static_cast<unsigned long long>(m_vertexNum / 3 / 32)) + 1));
	float l = min(s.x, e.x);
	float r = max(s.x, e.x);
	float t = max(s.z, e.z);
	float b = min(s.z, e.z);
	float u = max(s.y, e.y);
	float d = min(s.y, e.y);

	CVector3D ss, ee;
	ss.ix = max(0,min(m_cut.ix - 1, (int)((l - m_min.x) / m_length.x)));
	ee.ix = max(0, min(m_cut.ix - 1, (int)((r - m_min.x) / m_length.x)));

	ss.iz = max(0, min(m_cut.iz - 1, (int)((b - m_min.z) / m_length.z)));
	ee.iz = max(0, min(m_cut.iz - 1, (int)((t - m_min.z) / m_length.z)));

	ss.iy = max(0, min(m_cut.iy - 1, (int)((d - m_min.y) / m_length.y)));
	ee.iy = max(0, min(m_cut.iy - 1, (int)((u - m_min.y) / m_length.y)));


	bool hit = false;

	for (int lineY = ss.iy; lineY <= ee.iy; lineY++) {
		for (int lineZ = ss.iz; lineZ <= ee.iz; lineZ++) {
			for (int lineX = ss.ix; lineX <= ee.ix; lineX++) {
				int ceil = GetIndex(lineX, lineY, lineZ);
				if (!CCollision::CollisionAABBRay(NULL, NULL, m_colmin[ceil], m_colmax[ceil], s, dir)) continue;
				for (unsigned int i = 0; i < m_colIdex[ceil].size(); i++) {
					int idx = m_colIdex[ceil][i];
					if (m_colmask[idx / 32] & (1 << (idx % 32))) continue;
					if (CCollision::IntersectTriangleRay(c, s, e, v[idx * 3].v, v[idx * 3 + 1].v, v[idx * 3 + 2].v, m_normal[idx], pLength)) {

						CVector3D e1 = v[idx * 3 + 1].v - v[idx * 3].v;
						CVector3D e2 = v[idx * 3 + 2].v - v[idx * 3].v;

						*n = m_normal[idx];
						hit = true;
					}
					m_colmask[idx / 32] |= 1 << (idx % 32);
				}

			}
		}
	}
	/*
	for (int i = 0; i < m_vertexNum; i += 3, v += 3) {
		if (CCollision::IntersectTriangleRay(c, s, e, v[0], v[1], v[2], pLength)){

			CVector3D e1 = v[1] - v[0];
			CVector3D e2 = v[2] - v[0];

			*n = CVector3D::Cross(e1, e2).GetNormalize();
			hit = true;
		}
	}
	*/
	//if (m_buffer) {
	//	glUnmapBuffer(GL_ARRAY_BUFFER);
	//	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//}
	return hit;
}

int CMeshObj::CollisionRay(std::vector<CCollTriangle>& out, const CVector3D& s, const CVector3D& e, const CVector3D& dir, int matNo)
{
	SVertex* v = mp_vertex_array;
	memset(m_colmask, 0, sizeof(unsigned long) * ((static_cast<unsigned long long>(m_vertexNum / 3 / 32)) + 1));
	float l = min(s.x, e.x);
	float r = max(s.x, e.x);
	float t = max(s.z, e.z);
	float b = min(s.z, e.z);
	float u = max(s.y, e.y);
	float d = min(s.y, e.y);

	CVector3D ss, ee;
	ss.ix = max(0, min(m_cut.ix - 1, (int)((l - m_min.x) / m_length.x)));
	ee.ix = max(0, min(m_cut.ix - 1, (int)((r - m_min.x) / m_length.x)));

	ss.iz = max(0, min(m_cut.iz - 1, (int)((b - m_min.z) / m_length.z)));
	ee.iz = max(0, min(m_cut.iz - 1, (int)((t - m_min.z) / m_length.z)));

	ss.iy = max(0, min(m_cut.iy - 1, (int)((d - m_min.y) / m_length.y)));
	ee.iy = max(0, min(m_cut.iy - 1, (int)((u - m_min.y) / m_length.y)));


	int cnt=0;

	for (int lineY = ss.iy; lineY <= ee.iy; lineY++) {
		for (int lineZ = ss.iz; lineZ <= ee.iz; lineZ++) {
			for (int lineX = ss.ix; lineX <= ee.ix; lineX++) {
				int ceil = GetIndex(lineX, lineY, lineZ);
				if (!CCollision::CollisionAABBRay(NULL, NULL, m_colmin[ceil], m_colmax[ceil], s, dir)) continue;
				for (unsigned int i = 0; i < m_colIdex[ceil].size(); i++) {
					int idx = m_colIdex[ceil][i];
					if (m_colmask[idx / 32] & (1 << (idx % 32))) continue;
					CVector3D c;
					float dist = FLT_MAX;
					if (CCollision::IntersectTriangleRay(&c, s, e, v[idx * 3].v, v[idx * 3 + 1].v, v[idx * 3 + 2].v, m_normal[idx],&dist)) {
						out.push_back(CCollTriangle(
							m_normal[idx],
							v[idx * 3 + 0].v,
							v[idx * 3 + 1].v,
							v[idx * 3 + 2].v,
							c,
							dist,
							matNo));
						cnt++;
					}
					m_colmask[idx / 32] |= 1 << (idx % 32);
				}

			}
		}
	}
	return cnt;
}


int CMeshObj::CollisionSphere(std::vector<CCollTriangle>& out, const CVector3D &center, float radius,int matNo){
	/*SVertex* v = nullptr;
	if (m_buffer) {
		glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
		v = (SVertex*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
	}
	else {
		v = mp_vertex_array;
	}*/

	SVertex *v = mp_vertex_array;
	memset(m_colmask, 0, sizeof(unsigned long)*((static_cast<unsigned long long>(m_vertexNum / 3 / 32)) + 1));
	float l = center.x-radius;
	float r = center.x + radius;
	float t = center.z + radius;
	float b = center.z - radius;
	float u = center.y + radius;
	float d = center.y - radius;

	CVector3D ss, ee;
	ss.ix = max(0, min(m_cut.ix - 1, (int)((l - m_min.x) / m_length.x)));
	ee.ix = max(0, min(m_cut.ix - 1, (int)((r - m_min.x) / m_length.x)));

	ss.iz = max(0, min(m_cut.iz - 1, (int)((b - m_min.z) / m_length.z)));
	ee.iz = max(0, min(m_cut.iz - 1, (int)((t - m_min.z) / m_length.z)));

	ss.iy = max(0, min(m_cut.iy - 1, (int)((d - m_min.y) / m_length.y)));
	ee.iy = max(0, min(m_cut.iy - 1, (int)((u - m_min.y) / m_length.y)));

	int cnt = 0;
	float dist;
	for (int lineY = ss.iy; lineY <= ee.iy; lineY++) {
		for (int lineZ = ss.iz; lineZ <= ee.iz; lineZ++) {
			int ceil_x = GetIndex(0, lineY, lineZ);
			for (int lineX = ss.ix; lineX <= ee.ix; lineX++) {
				int ceil = ceil_x + lineX;
				for (unsigned int i = 0, s = m_colIdex[ceil].size(); i < s; i++) {
					int idx = m_colIdex[ceil][i];
					if (m_colmask[idx / 32] & (1 << (idx % 32))) continue;
					if (CCollision::CollisionTriangleSphere(v[idx * 3].v, v[idx * 3 + 1].v, v[idx * 3 + 2].v, m_normal[idx], center, radius, NULL, &dist)) {
						out.push_back(CCollTriangle(
							m_normal[idx],
							v[idx * 3 + 0].v,
							v[idx * 3 + 1].v,
							v[idx * 3 + 2].v,
							dist,
							matNo));
						cnt++;

					}
					m_colmask[idx / 32] |= 1 << (idx % 32);
				}
			}
		}
	}
	/*
	int cnt = 0;
	for (int i = 0; i < m_vertexNum; i+=3,v+=3)
	{
		float dist;
		if (CCollision::CollisionTriangleSphere(v[0], v[1], v[2], center, radius, NULL, &dist)) {
			out->m_dist = dist;

			CVector3D e1 = v[1] - v[0];
			CVector3D e2 = v[2] - v[0];

			out->m_normal = CVector3D::Cross(e1, e2).GetNormalize();
			out++;
			cnt++;
		}
	}
	*/
	//if (m_buffer) {
	//	glUnmapBuffer(GL_ARRAY_BUFFER);
	//	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//}
	return cnt;
}


int CMeshObj::CollisionCapsule(std::vector<CCollTriangle>& out, const CCapsule& cap, int matNo){
	/*SVertex* v = nullptr;
	if (m_buffer) {
		glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
		v = (SVertex*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
	}
	else {
		v = mp_vertex_array;
	}
	*/

	SVertex *v = mp_vertex_array;
	
	memset(m_colmask, 0, sizeof(unsigned long)*((static_cast<unsigned long long>(m_vertexNum / 3 / 32)) + 1));
	float l = min(cap.GetStartPoint().x, cap.GetEndPoint().x) - cap.GetRadius();
	float r = max(cap.GetStartPoint().x, cap.GetEndPoint().x) + cap.GetRadius();
	float t = max(cap.GetStartPoint().z, cap.GetEndPoint().z) + cap.GetRadius();
	float b = min(cap.GetStartPoint().z, cap.GetEndPoint().z) - cap.GetRadius();
	float u = max(cap.GetStartPoint().y, cap.GetEndPoint().y) + cap.GetRadius();
	float d = min(cap.GetStartPoint().y, cap.GetEndPoint().y) - cap.GetRadius();

	CVector3D ss, ee;
	ss.ix = max(0, min(m_cut.ix - 1, (int)((l - m_min.x) / m_length.x)));
	ee.ix = max(0, min(m_cut.ix - 1, (int)((r - m_min.x) / m_length.x)));

	ss.iz = max(0, min(m_cut.iz - 1, (int)((b - m_min.z) / m_length.z)));
	ee.iz = max(0, min(m_cut.iz - 1, (int)((t - m_min.z) / m_length.z)));

	ss.iy = max(0, min(m_cut.iy - 1, (int)((d - m_min.y) / m_length.y)));
	ee.iy = max(0, min(m_cut.iy - 1, (int)((u - m_min.y) / m_length.y)));

	int cnt = 0;
	float dist;
	for (int lineY = ss.iy; lineY <= ee.iy; lineY++) {
		for (int lineZ = ss.iz; lineZ <= ee.iz; lineZ++) {
			for (int lineX = ss.ix; lineX <= ee.ix; lineX++) {
				int ceil = GetIndex(lineX, lineY, lineZ);
				for (unsigned int i = 0; i < m_colIdex[ceil].size(); i++) {
					int idx = m_colIdex[ceil][i];
					if (m_colmask[idx / 32] & (1 << (idx % 32))) continue;
					if (CCollision::CollisionTriangleCapsule(v[idx * 3].v, v[idx * 3 + 1].v, v[idx * 3 + 2].v, m_normal[idx],cap, NULL, &dist)) {
						out.push_back(CCollTriangle(
							m_normal[idx],
							v[idx * 3 + 0].v,
							v[idx * 3 + 1].v,
							v[idx * 3 + 2].v,
							dist,
							matNo));
						cnt++;
					}
					m_colmask[idx / 32] |= 1 << (idx % 32);
				}
			}
		}
	}
	/*
	int cnt = 0;
	for (int i = 0; i < m_vertexNum; i += 3, v += 3)
	{
		float dist;
		if (CCollision::CollisionTriangleCapsule(v[0], v[1], v[2], top, bottom, radius, NULL, &dist)) {
			out->m_dist = dist;

			CVector3D e1 = v[1] - v[0];
			CVector3D e2 = v[2] - v[0];

			out->m_normal = CVector3D::Cross(e1, e2).GetNormalize();
			out++;
			cnt++;
		}
	}
	*/
	//if (m_buffer) {
	//	glUnmapBuffer(GL_ARRAY_BUFFER);
	//	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//}
	return cnt;
}
int CMeshObj::CollisionAABB(std::vector<CCollTriangle>& out, const CVector3D& aabb_max, const CVector3D& aabb_min, const CVector3D& check_max, const CVector3D& check_min, const CMatrix& trans, int matNo)
{	/*SVertex* v = nullptr;
	if (m_buffer) {
		glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
		v = (SVertex*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
	}
	else {
		v = mp_vertex_array;
	}
	*/

	SVertex* v = mp_vertex_array;

	memset(m_colmask, 0, sizeof(unsigned long) * ((static_cast<unsigned long long>(m_vertexNum / 3 / 32)) + 1));
	float l = check_min.x;
	float r = check_max.x;
	float t = check_max.z;
	float b = check_min.z;
	float u = check_max.y;
	float d = check_min.y;

	CVector3D ss, ee;
	ss.ix = max(0, min(m_cut.ix - 1, (int)((l - m_min.x) / m_length.x)));
	ee.ix = max(0, min(m_cut.ix - 1, (int)((r - m_min.x) / m_length.x)));

	ss.iz = max(0, min(m_cut.iz - 1, (int)((b - m_min.z) / m_length.z)));
	ee.iz = max(0, min(m_cut.iz - 1, (int)((t - m_min.z) / m_length.z)));

	ss.iy = max(0, min(m_cut.iy - 1, (int)((d - m_min.y) / m_length.y)));
	ee.iy = max(0, min(m_cut.iy - 1, (int)((u - m_min.y) / m_length.y)));

	int cnt = 0;
	float dist,push;
	CVector3D normal;
	for (int lineY = ss.iy; lineY <= ee.iy; lineY++) {
		for (int lineZ = ss.iz; lineZ <= ee.iz; lineZ++) {
			for (int lineX = ss.ix; lineX <= ee.ix; lineX++) {
				int ceil = GetIndex(lineX, lineY, lineZ);
				for (unsigned int i = 0; i < m_colIdex[ceil].size(); i++) {
					int idx = m_colIdex[ceil][i];
					if (m_colmask[idx / 32] & (1 << (idx % 32))) continue;
					CVector3D vv[3] = {
						trans * CVector4D(v[idx * 3].v,1),
						trans * CVector4D(v[idx * 3 + 1].v,1),
						trans * CVector4D(v[idx * 3 + 2].v,1)
					};
					if (CCollision::CollisionAABBTriangle(aabb_max,aabb_min,vv,&normal,&dist,&push)) {
						out.push_back(CCollTriangle(
							m_normal[idx],
							v[idx * 3 + 0].v,
							v[idx * 3 + 1].v,
							v[idx * 3 + 2].v,
							dist,
							matNo));
						cnt++;
					}
					m_colmask[idx / 32] |= 1 << (idx % 32);
				}
			}
		}
	}
	return cnt;
}
CModelObj::CModelObj() :m_mesh(NULL){
}
CModelObj::CModelObj(const CModelObj& m)
{
	*this = m;

}
CModelObj& CModelObj::operator=(const CModelObj& m)
{
	m_mesh = m.m_mesh;
	m_cut = m.m_cut;
	m_materialCnt = m.m_materialCnt;
	m_material = new CMaterial * [m_materialCnt];
	for (int i = 0; i < m_materialCnt; i++) {
		m_material[i] = new CMaterial(*m.m_material[i]);
	}
	return *this;
}
CModelObj::~CModelObj() {
	if (m_material) {
		for (int i = 0; i < m_materialCnt; i++) {
			delete m_material[i];
		}
		delete[] m_material;
		m_material = nullptr;
	}
}
bool CModelObj::Load(const char *path, int cut_x, int cut_y, int cut_z) {
	MakePath(path);
	m_cut.ix = cut_x;
	m_cut.iy = cut_y;
	m_cut.iz = cut_z;
	CVector3D v_max(FLT_MIN, FLT_MIN, FLT_MIN), v_min(FLT_MAX, FLT_MAX, FLT_MAX);
	//ファイルポインタ(fp)の作成し開いたファイルのハンドルを渡す。
	FILE* fp;
	fopen_s(&fp,path,"r");
	//ファイルが開けなかった場合はエラーを表示しfalseを返す
	if(fp==NULL) {
		printf("ファイルがありません%s\n",path);
		return false;
	}
	//座標データ用カウンター
	int	VCnt=0;
	//法線データ用カウンター
	int	NCnt=0;
	//テクスチャーコード用カウンター
	int	TCnt=0;
	//面データ用カウンター
	int	FCnt=0;
	//キーワード格納用
	char key[128]="";
	//ファイルの末尾でなければ繰り返す
	CMeshObj *m=NULL;
	while(!feof(fp)) {
		//keyをクリア
		key[0]=0;
		//1単語　文字列を読み込み
		fscanf_s(fp,"%s",key,128);
		//読み込んだキーワードが「v」なら座標データカウンターをカウントアップ
		if(strcmp(key,"v")==0) {
			VCnt++;
		} else 
		if(strcmp(key,"usemtl")==0) {
			char name[128];
			fscanf_s(fp,"%s",name,128);
			for(int i=0;i<m_materialCnt;i++) {
				if(strcmp(name,m_material[i]->m_name)==0) {
					m = &m_mesh[i];
				}
			}
		}
		//読み込んだキーワードが「f」なら面データカウンターをカウントアップ
		if(strcmp(key,"f")==0) {
			//頂点数をカウントアップ
			if(m) m->m_vertexNum+=3;
#ifdef _DEBUG
			else{
				char str[256] = "";
				sprintf_s(str, 256, "[%s]マテリアル未割当のメッシュがあります。", path);
				MessageBox(GL::hWnd, str, "", MB_OK);
			}
#endif // _DEBUG
		} else 
		//読み込んだキーワードが「vn」なら法線データカウンターをカウントアップ
		if(strcmp(key,"vn")==0) {
			NCnt++;
		} else 
		//読み込んだキーワードが「vt」ならテクスチャーコードデータカウンターをカウントアップ
		if(strcmp(key,"vt")==0) {
			TCnt++;
		} else 
		//読み込んだキーワードが「mtllib」ならマテリアルファイルの読み込み
		if(strcmp(key,"mtllib")==0) {
			char str[PATH_SIZE];
			strcpy_s(str, PATH_SIZE, m_filePath);
			int l = (int)strlen(str);
			fscanf_s(fp, "%s", &str[l], PATH_SIZE-l);
			LoadMaterial(str);
		}
	}
	//printf("VCnt %d\n",VCnt);
	

	//座標データの数だけ配列を作成
	CVector3D *pVertex = new CVector3D[VCnt];
	//法線データの数だけ配列を作成
	CVector3D *pNormal = new CVector3D[NCnt];
	//テクスチャーコードデータの数だけ配列を作成
	CVector2D *pTexCoord = new CVector2D[TCnt];

	for(int i=0;i<m_materialCnt;i++) {
		//描画用頂点配列を作成
		m_mesh[i].mp_vertex_array = new CMeshObj::SVertex[m_mesh[i].m_vertexNum];
		m_mesh[i].m_vertexNum=0;

	}
	//読み込んだ座標データを0から順番に格納するため、カウンターを0に戻す
	VCnt=0;
	NCnt=0;
	TCnt=0;
	//読み書きの位置をファイルの先頭に戻す
	fseek(fp,0,SEEK_SET);
	//描画用頂点配列への座標データ格納先番号
	int idx = 0;
	//ファイルの末尾でなければ繰り返す
	while(!feof(fp)) {
		//keyをクリア
		key[0]=0;
		//キーワードの読み込み
		fscanf_s(fp,"%s",key,128);
		//「ｖ」を読み込んだら、座標データを読み込む
		if(strcmp(key,"v")==0) {
			CVector3D*vv = &pVertex[VCnt];
			fscanf_s(fp,"%f %f %f",&vv->x,&vv->y,&vv->z);
			v_min.x = min(v_min.x, vv->x);
			v_max.x = max(v_max.x, vv->x);
			v_max.z = max(v_max.z, vv->z);
			v_min.z = min(v_min.z, vv->z);
			v_max.y = max(v_max.y, vv->y);
			v_min.y = min(v_min.y, vv->y);

			//読み込み先を次のデータへ
			VCnt++;
		}else//「vn」を読み込んだら、法線データを読み込む
		if(strcmp(key,"vn")==0) {
			fscanf_s(fp,"%f %f %f",&pNormal[NCnt].x,&pNormal[NCnt].y,&pNormal[NCnt].z);
			//読み込み先を次のデータへ
			NCnt++;
		}else//「vt」を読み込んだら、テクスチャーコードデータを読み込む
		if(strcmp(key,"vt")==0) {
			fscanf_s(fp,"%f %f",&pTexCoord[TCnt].x,&pTexCoord[TCnt].y);
			pTexCoord[TCnt].y = 1.0f - pTexCoord[TCnt].y;
			//読み込み先を次のデータへ
			TCnt++;
		} else 
		if(strcmp(key,"usemtl")==0) {
			char name[NAME_STR_SIZE];
			fscanf_s(fp,"%s",name, NAME_STR_SIZE);
			for(int i=0;i<m_materialCnt;i++) {
				if(strcmp(name,m_material[i]->m_name)==0) {
					m = &m_mesh[i];
				}
			}
		}
		if(strcmp(key,"f")==0) {

			int d[3][3] = { 1, 1, 1, 1, 1, 1, 1, 1, 1 };
			char data[128]="";
			fgets(data, 255, fp);
			char *p = data + 1;
			char str[16]="";
			int strIdx = 0;
			int t = 0, c = 0;
			CVector3D v[3];
			while (*p) {
				if (*p == '/' || *p == ' ' || *p == '\n') {

					str[strIdx] = 0;
					if (strIdx != 0) d[t][c] = atoi(str);
					strIdx = 0;
					if (*p == '/') t++;
					else{
						c++;
						t = 0;
						if (c >= 3) {
							if (*(p + 1) != '\0') printf("三角形以外のポリゴンがあります");
							break;
						}
					}
				}
				else {
					str[strIdx] = *p;
					strIdx++;
				}
				p++;
			}
			m->mp_vertex_array[m->m_vertexNum].v = v[0] = pVertex[d[0][0] - 1];			//頂点１の座標データ
			m->mp_vertex_array[m->m_vertexNum + 1].v = v[1]  = pVertex[d[0][1] - 1];		//頂点２の座標データ
			m->mp_vertex_array[m->m_vertexNum + 2].v= v[2]  = pVertex[d[0][2] - 1];

			m->mp_vertex_array[m->m_vertexNum].n = pNormal[d[2][0] - 1];			//頂点１の法線データ
			m->mp_vertex_array[m->m_vertexNum + 1].n = pNormal[d[2][1] - 1];		//頂点２の法線データ
			m->mp_vertex_array[m->m_vertexNum + 2].n = pNormal[d[2][2] - 1];

			m->mp_vertex_array[m->m_vertexNum].t = pTexCoord[d[1][0] - 1];			//頂点１のテクスチャーコードデータ
			m->mp_vertex_array[m->m_vertexNum + 1].t = pTexCoord[d[1][1] - 1];		//頂点２のテクスチャーコードデータ
			m->mp_vertex_array[m->m_vertexNum + 2].t = pTexCoord[d[1][2] - 1];
			CVector3D tangent;
			CalcTangentAndBinormal(
				m->mp_vertex_array[m->m_vertexNum].v,
				m->mp_vertex_array[m->m_vertexNum].t,
				m->mp_vertex_array[m->m_vertexNum + 1].v,
				m->mp_vertex_array[m->m_vertexNum + 1].t,
				m->mp_vertex_array[m->m_vertexNum + 2].v,
				m->mp_vertex_array[m->m_vertexNum + 2].t,
				&tangent);
			m->mp_vertex_array[m->m_vertexNum].tan = tangent;
			m->mp_vertex_array[m->m_vertexNum + 1].tan = tangent;
			m->mp_vertex_array[m->m_vertexNum + 2].tan = tangent;

			/*
			int v1,v2,v3;
			int n1,n2,n3;
			int t1,t2,t3;
			fscanf_s(fp,"%d/%d/%d %d/%d/%d %d/%d/%d",&v1,&t1,&n1,&v2,&t2,&n2,&v3,&t3,&n3);
			m->m_pVertexAry[m->m_vertexNum] = pVertex[v1-1];			//頂点１の座標データ
			m->m_pVertexAry[m->m_vertexNum+1] = pVertex[v2-1];		//頂点２の座標データ
			m->m_pVertexAry[m->m_vertexNum+2] = pVertex[v3-1];		
			
			m->m_pNormalAry[m->m_vertexNum] = pNormal[n1-1];			//頂点１の法線データ
			m->m_pNormalAry[m->m_vertexNum+1] = pNormal[n2-1];		//頂点２の法線データ
			m->m_pNormalAry[m->m_vertexNum+2] = pNormal[n3-1];		
			
			m->m_pTexCoordAry[m->m_vertexNum] = pTexCoord[t1-1];			//頂点１のテクスチャーコードデータ
			m->m_pTexCoordAry[m->m_vertexNum+1] = pTexCoord[t2-1];		//頂点２のテクスチャーコードデータ
			m->m_pTexCoordAry[m->m_vertexNum+2] = pTexCoord[t3-1];		
			*/
			//データの格納先を3つ移動する
			m->m_vertexNum+=3;
		}
	}
	fclose(fp);



	for (int i = 0; i<m_materialCnt; i++) {
		m_mesh[i].m_min = v_min;
		m_mesh[i].m_max = v_max;
		if(m_cut.ix* m_cut.iy* m_cut.iz >0) m_mesh[i].CreatCollsionIndex(m_cut);
	}



	
	//一時データ解放
	delete[] pVertex;
	delete[] pNormal;
	delete[] pTexCoord;
	return true;
}

bool CModelObj::LoadMaterial(char *path) {
	char key[128];
	FILE* fp;
	fopen_s(&fp, path, "r");
	//ファイルが開けなかった場合はエラーを表示しfalseを返す
	if (fp == NULL) {
		printf("ファイルがありません%s\n", path);
		return false;
	}

	m_materialCnt = 0;
	while (!feof(fp)) {
		fscanf_s(fp, "%s", key, 128);
		if (strcmp(key, "newmtl") == 0)
			m_materialCnt++;
	}
	m_mesh = new CMeshObj[m_materialCnt];
	m_material = new CMaterial * [m_materialCnt];
	m_materialCnt = 0;
	CMeshObj *m = m_mesh;
	CMaterial *mat = m_material[0];
	fseek(fp, 0, SEEK_SET);
	while (!feof(fp)) {
		key[0] = '\0';
		fscanf_s(fp, "%s", key, 128);
		if (strcmp(key, "newmtl") == 0) {
			m = &m_mesh[m_materialCnt];
			mat = m_material[m_materialCnt]= new CMaterial();
			mat->m_shader_name = "StaticMesh";
			char name[NAME_STR_SIZE];
			fscanf_s(fp, "%s", name, NAME_STR_SIZE);
			strcpy_s(mat->m_name, NAME_STR_SIZE, name);

			m_materialCnt++;
		}else
		if (strcmp(key, "Ka") == 0) {
			fscanf_s(fp, "%f %f %f", &mat->m_ambient.x, &mat->m_ambient.y, &mat->m_ambient.z);
			mat->m_ambient.a = 1.0f;
		}else
		if (strcmp(key, "Kd") == 0) {
			fscanf_s(fp, "%f %f %f", &mat->m_diffuse.x, &mat->m_diffuse.y, &mat->m_diffuse.z);
			mat->m_diffuse.a = 1.0f;
		}else
		if (strcmp(key, "Ks") == 0) {
			fscanf_s(fp, "%f %f %f", &mat->m_specular.x, &mat->m_specular.y, &mat->m_specular.z);
		}else
		if (strcmp(key, "Ke") == 0) {
			fscanf_s(fp, "%f %f %f", &mat->m_emissive.x, &mat->m_emissive.y, &mat->m_emissive.z);
		}else
		if (strcmp(key, "Ns") == 0) {
			fscanf_s(fp, "%f", &mat->m_shininess);
		}else
		if (strcmp(key, "d") == 0) {
			float aplha;
			fscanf_s(fp, "%f", &aplha);
			mat->m_ambient.a = aplha;
			mat->m_diffuse.a = aplha;
		}else
		if (strcmp(key, "map_Kd") == 0 || strcmp(key, "map_Bump") == 0){
			char tex_file[PATH_SIZE];
			char* t_file = tex_file+1;
			fgets(tex_file, PATH_SIZE, fp);
			char tag[32];
			sscanf_s(t_file, "%s", tag,32);
			while (tag[0] == '-') {
				while (t_file && *t_file) {
					t_file = strchr(t_file, ' ') + 1;
					float v;
					int n = sscanf_s(t_file, "%f", &v);
					if (n == 0) break;
				}
				sscanf_s(t_file, "%s", tag, 32);
			}
			if (char* p = strrchr(t_file, '\n')) *p = '\0';
			if (char* p = strrchr(t_file, '\r')) *p = '\0';
			strcpy_s(mat->m_texture_name, NAME_STR_SIZE, t_file);

			char str[PATH_SIZE];
			strcpy_s(str, PATH_SIZE, m_filePath);
			int l = (int)strlen(str);
			strcat_s(str, PATH_SIZE, mat->m_texture_name);
			CTexture** tt = &mat->mp_texture;
			if (strcmp(key, "map_Bump")==0) tt = &mat->mp_normal_map;
			if (*tt) {
				(*tt)->Release();
			}
			*tt = new CTexture();
			if (!(*tt)->Load(str)) {
				delete *tt;
				*tt = NULL;
			}
		}
	}
	fclose(fp);
	return true;
}
void CModelObj::Render(const CMatrix &m) {


	for(int i=0;i<m_materialCnt;i++) {

		CShader* s = m_material[i]->mp_shader = CShader::GetInstance(m_material[i]->m_shader_name);
		s->Enable();
		glUniform1f(glGetUniformLocation(s->GetProgram(), "shadow_bias"), m_shadow_bias);
		glUniform1i(glGetUniformLocation(s->GetProgram(), "toon"), m_toon ? 1 : 0);
		SendShaderParam(s,m, CCamera::GetCurrent()->GetViewMatrix() * m, CCamera::GetCurrent()->GetProjectionMatrix());
		m_mesh[i].Render(m_material[i]);
		s->Disable();
	}


}
void CModelObj::Render() {
	CModel::UpdateMatrix();
	Render(m_matrix);
}
void CModelObj::Release() {
	
	for(int i=0;i<m_materialCnt;i++) {
		m_material[i]->Release();
		delete m_material[i];
		m_mesh[i].Release();
	}
	delete[] m_material;
	m_material = nullptr;
	if (m_mesh) delete[] m_mesh;
	m_mesh = nullptr;
	//if (m_shader) delete m_shader;
}

CMaterial* CModelObj::GetMaterial(int no) const
{
	return m_material[no];	
}

CMaterial* CModelObj::GetMaterial(const char* name) const
{
	for (int i = 0; i < GetMaterialSize(); i++) {
		if (strcmp(name, m_material[i]->m_name) == 0) return m_material[i];
	}
	return nullptr;
}

bool CModelObj::CollisionRay(CVector3D *pCross, CVector3D *pNormal, const CVector3D &s, const CVector3D &e)const {
#ifdef _DEBUG
	if (m_cut.ix == 0) {
		char str[256] = "";
		sprintf_s(str, 256, "モデルの衝突はLoad,CreatModel呼び出し時に分割数を1以上で設定してください。");
		MessageBox(GL::hWnd, str, "", MB_OK);
	}
#endif // _DEBUG
	CVector3D cross, normal;
	float length = 1e10;
	bool hit = false;
	CVector3D c, n;

	CMatrix inv = m_matrix.GetInverse();
	CVector3D s2 = inv * CVector4D(s, 1);
	CVector3D e2 = inv * CVector4D(e, 1);
	CVector3D dir = (e2 - s2).GetNormalize();

	for (int i = 0; i<m_materialCnt; i++) {
		if (m_mesh[i].CollisionRay(&c, &n, s2,e2, dir,&length))
			hit = true;
	}
	if (hit){
		if (pCross) {
			*pCross = m_matrix*CVector4D(c , 1);
		}
		if (pNormal) {
			*pNormal = CVector3D(m_matrix*CVector4D(n, 0)).GetNormalize();
		}
	}
	return hit;

}

std::vector<CCollTriangle> CModelObj::CollisionRay(const CVector3D& s, const CVector3D& e) const
{
#ifdef _DEBUG
	if (m_cut.ix == 0) {
		char str[256] = "";
		sprintf_s(str, 256, "モデルの衝突はLoad,CreatModel呼び出し時に分割数を1以上で設定してください。");
		MessageBox(GL::hWnd, str, "", MB_OK);
	}
#endif // _DEBUG
	CVector3D c, n;
	float scale = CVector3D(m_matrix.m00, m_matrix.m10, m_matrix.m20).Length();

	CMatrix inv = m_matrix.GetInverse();
	CVector3D s2 = inv * CVector4D(s, 1);
	CVector3D e2 = inv * CVector4D(e, 1);
	CVector3D dir = (e2 - s2).GetNormalize();

	std::vector<CCollTriangle> out;
	for (int i = 0; i < m_materialCnt; i++) {
		m_mesh[i].CollisionRay(out, s2, e2, dir,i);
	}
	for(auto& v:out){	
		v.Transform(m_matrix);
		v.m_dist *= scale;
		v.m_cross = m_matrix * CVector4D(v.m_cross, 1);
	}
	return out;

}


std::vector<CCollTriangle>  CModelObj::CollisionSphere(const CVector3D &center, float radius)const {
#ifdef _DEBUG
	if (m_cut.ix == 0) {
		char str[256] = "";
		sprintf_s(str, 256, "モデルの衝突はLoad,CreatModel呼び出し時に分割数を1以上で設定してください。");
		MessageBox(GL::hWnd, str, "", MB_OK);
	}
#endif // _DEBUG
	float scale = CVector3D(m_matrix.m00, m_matrix.m10, m_matrix.m20).Length();
	int cnt = 0;
	std::vector<CCollTriangle> out;
	CVector3D p = m_matrix.GetInverse() * CVector4D(center, 1);
	float r = radius / scale;

	for (int i = 0; i<m_materialCnt; i++) {
		m_mesh[i].CollisionSphere(out, p, r,i);

	}


	for (auto &v : out) {
		v.m_dist *= scale;
		v.Transform(m_matrix);
	}
	return out;
}


std::vector<CCollTriangle>  CModelObj::CollisionCapsule(const CCapsule& cap)const {
#ifdef _DEBUG
	if (m_cut.ix == 0) {
		char str[256] = "";
		sprintf_s(str, 256, "モデルの衝突はLoad,CreatModel呼び出し時に分割数を1以上で設定してください。");
		MessageBox(GL::hWnd, str, "", MB_OK);
	}
#endif // _DEBUG
	float scale = CVector3D(m_matrix.m00, m_matrix.m10, m_matrix.m20).Length();
	int cnt = 0;
	std::vector<CCollTriangle> out;
	CMatrix inv = m_matrix.GetInverse();
	CVector3D t = inv * CVector4D(cap.GetStartPoint(), 1);
	CVector3D b = inv * CVector4D(cap.GetEndPoint(), 1);
	float r = cap.GetRadius() / scale;
	CCapsule c = CCapsule(t, b, r);
	for (int i = 0; i<m_materialCnt; i++) {
		m_mesh[i].CollisionCapsule(out,c ,i);

	}
	for (auto& v : out) {
		v.m_dist *= scale;
		v.Transform(m_matrix);

	}
	return out;
}
std::vector<CCollTriangle> CModelObj::CollisionOBB(const COBB& obb)const
{
#ifdef _DEBUG
	if (m_cut.ix == 0) {
		char str[256] = "";
		sprintf_s(str, 256, "モデルの衝突はLoad,CreatModel呼び出し時に分割数を1以上で設定してください。");
		MessageBox(GL::hWnd, str, "", MB_OK);
	}
#endif // _DEBUG
	float scale = CVector3D(m_matrix.m00, m_matrix.m10, m_matrix.m20).Length();
	int cnt = 0;
	std::vector<CCollTriangle> out;
	CMatrix inv = m_matrix.GetInverse();
	COBB obb2 = obb;
	obb2.Transeform(inv);
	CMatrix mat(obb2.m_axis[0].x, obb2.m_axis[1].x, obb2.m_axis[2].x,obb2.m_center.x,
		obb2.m_axis[0].y, obb2.m_axis[1].y, obb2.m_axis[2].y, obb2.m_center.y, 
		obb2.m_axis[0].z, obb2.m_axis[1].z, obb2.m_axis[2].z, obb2.m_center.z, 
		0,0,0,1);

	CVector3D v[2] = {
		mat * CVector4D(obb2.m_length.x,obb2.m_length.y,obb2.m_length.z,1),
		mat * CVector4D(-obb2.m_length.x,-obb2.m_length.y,-obb2.m_length.z,1),
	};
	CVector3D check_max =CVector3D(max(v[0].x, v[1].x), max(v[0].y, v[1].y), max(v[0].z, v[1].z));
	CVector3D check_min = CVector3D(min(v[0].x, v[1].x), min(v[0].y, v[1].y), min(v[0].z, v[1].z));
	for (int i = 0; i < m_materialCnt; i++) {
		m_mesh[i].CollisionAABB(out, obb2.m_length, -obb2.m_length,check_max, check_min, mat.GetInverse(),i);

	}
	for (auto& v : out) {
		v.m_dist *= scale;
		v.Transform(m_matrix);

	}
	return out;

}
CModelObj* CModelObj::CreateModel(char *path, int cut_x, int cut_y, int cut_z) {
	CModelObj *o = new CModelObj();
	if (o->Load(path, cut_x, cut_y, cut_z)) {
		return o;
	}
	delete o;
	return nullptr;
}