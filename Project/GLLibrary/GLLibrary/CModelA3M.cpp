#include "CModelA3M.h"
#include "CCamera.h"
#ifdef USE_ATHOR_MODEL
#include "CModelFBX.h"
#include "CModelX.h"
#include "Utility.h"
/*
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xinclude.h>
#include <libxml/xmlIO.h>

#ifdef _DEBUG
#pragma comment(lib,"libxml2_a.lib")
#else
#pragma comment(lib,"libxml2_a.lib")
#endif
*/
#endif
#include "CModelObj.h"
#include "CCollision.h"
#include "CFPS.h"

static void printMatrix(FILE* fp, CMatrix& m) {
	fprintf(fp,
		" %f %f %f %f\n"
		" %f %f %f %f\n"
		" %f %f %f %f\n"
		" %f %f %f %f\n\n",
		m.m00, m.m01, m.m02, m.m03,
		m.m10, m.m11, m.m12, m.m13,
		m.m20, m.m21, m.m22, m.m23,
		m.m30, m.m31, m.m32, m.m33);
}
struct SA3MModelHeader {
	char fileType[2];
	unsigned short version;
	unsigned long bone_num;
	unsigned long materialCnt;
	unsigned long materialOffset;
	unsigned long rootOffset;
	unsigned long animationOffset;
	CVector3D	min;
	CVector3D	max;
};
struct SA3MaterialCHKOld {
	char m_name[NAME_STR_SIZE];
	CVector4D m_ambient;	//アンビエントカラー
	CVector4D m_diffuse;	//デフューズカラー
	CVector3D m_specular;	//スペキュラーカラー
	CVector3D m_emissive;	//自己発光
	float m_shininess;		//スペキュラー係数
	float m_alpha;
	char m_texture_name[NAME_STR_SIZE];

};
struct SA3MaterialCHK2 {
	char m_name[NAME_STR_SIZE];
	CVector4D m_ambient;	//アンビエントカラー
	CVector4D m_diffuse;	//デフューズカラー
	CVector3D m_specular;	//スペキュラーカラー
	CVector3D m_emissive;	//自己発光
	float m_shininess;		//スペキュラー係数
	float m_alpha;
	char m_texture_name[NAME_STR_SIZE];
	char m_bump_name[NAME_STR_SIZE];

};

struct SA3MNodeCHK {
	int	 m_no;			//ノード番号
	int m_bone_no;
	char m_name[NAME_STR_SIZE];
	unsigned long m_node_type;	//ノードの種類
	CMatrix m_offset;
	CMatrix m_local_matrix;
	CVector3D m_pos;
	CQuaternion m_rot;
	CVector3D m_scale;
	bool m_visibility;
	unsigned long child_offset;
	unsigned long next_offset;
	unsigned long node_size;
};

struct SA3MBoneOffsetCHK {
	int no;
	CMatrix offset;
};

//.A3Mフォーマット
//<SA3MModelHeader>
//char fileType[2];
//unsigned short version;
//unsigned long bone_num;
//unsigned long materialCnt;
//unsigned long materialOffset;
//unsigned long nodeOffset;
//unsigned long animationOffset;
//CVector3D	min;
//CVector3D	max;
//</SA3MModelHeader>
//<SA3MaterialCHK>
//</SA3MaterialCHK>
//<CA3MNode>
//unsigned long node_type;
//unsigned long child_offset;
//unsigned long next_offset;
//CMatrix matrix;
//CA3MMesh mesh
//</CA3MNode>
//<CA3MMesh>
//int poly_cnt
//int use_skin
//int bone_cnt
//(int,Matrix)[] offest
//<CA3MPoly>
//int m_vertex_cnt
//int m_material
//MY_VERTEX*m_vertex_cnt
//</CA3MPoly>
//<CA3MPoly>
//...
//</CA3MPoly>
//</CA3MMesh>
//<CA3MAnimationSet>
//</CA3MAnimationSet>

CA3MNode::CA3MNode():
	m_no(0), m_bone_no(-1),m_name(""), m_node_type(A3M::eNode), m_visibility(true), m_animation_layer(0),m_bind(false), mp_mesh(nullptr),
	mp_child(nullptr), mp_parent(nullptr), mp_next(nullptr), mp_prev(nullptr){
}

CA3MNode::CA3MNode(int no,const char* name):
	m_no(no), m_bone_no(-1),m_node_type(A3M::eNode), m_visibility(true), m_animation_layer(0), m_bind(false), mp_mesh(nullptr),
	mp_child(nullptr), mp_parent(nullptr), mp_next(nullptr), mp_prev(nullptr)
{
	strcpy_s(m_name,sizeof(m_name), name);
}
CA3MNode::CA3MNode(const CA3MNode& n) :
	m_no(n.m_no), m_bone_no(n.m_bone_no), m_node_type(n.m_node_type), m_matrix(n.m_matrix), m_offset(n.m_offset), m_local_matrix(n.m_local_matrix),
	m_pos(n.m_pos), m_rot(n.m_rot), m_scale(n.m_scale), m_bind(n.m_bind), m_bind_matrix(n.m_bind_matrix), m_visibility(n.m_visibility), m_animation_layer(n.m_animation_layer), mp_mesh(n.mp_mesh),
	mp_child(nullptr), mp_parent(nullptr), mp_next(nullptr), mp_prev(nullptr){
	strcpy_s(m_name, sizeof(m_name), n.m_name);

}

CA3MNode::~CA3MNode()
{
	if (mp_child) {
		delete mp_child;
		mp_child = nullptr;
	}
	if (mp_next) {
		delete mp_next;
		mp_next = nullptr;
	}
}

void CA3MNode::Release()
{
	if (mp_child) {
		mp_child->Release();
	}
	if (mp_next) {
		mp_next->Release();
	}
	if (mp_mesh) {
		mp_mesh->Release();
		delete mp_mesh;
	}
}

void CA3MNode::PintMatrix(CA3MNode* n)
{
	CMatrix m = n->GetMatrix();
	printf("%s No%d\n"
		" %f %f %f %f\n"
		" %f %f %f %f\n"
		" %f %f %f %f\n"
		" %f %f %f %f\n", n->GetName(), n->GetNo(),
		m.m00, m.m01, m.m02, m.m03,
		m.m10, m.m11, m.m12, m.m13,
		m.m20, m.m21, m.m22, m.m23,
		m.m30, m.m31, m.m32, m.m33);
	if (n->mp_child) PintMatrix(n->mp_child);
	if (n->mp_next) PintMatrix(n->mp_next);
}

void CA3MNode::PrintNode(CA3MNode* n, int level)
{
	for (int i = 0; i < level; i++) printf(" ");
	printf("%s No%d\n", n->GetName(), n->GetNo());
	if (n->mp_child) PrintNode(n->mp_child, level + 1);
	if (n->mp_next) PrintNode(n->mp_next, level);
}


void CA3MNode::LookAt(CA3MNode* joint, CA3MNode* effector, CVector3D targetPos)
{
	CVector3D localTargetPos(0, 0, 0);
	CVector3D localEffectorPos(0, 0, 0);
	CA3MNode* effector_node = effector;
	// エフェクタの位置の取得
	CVector3D effectorPos = effector_node->m_matrix.GetPosition();
	// 注目ノードの位置の取得
	CVector3D jointPos = joint->m_matrix.GetPosition();

	// ワールド座標系から注目ノードの局所座標系への変換
	CMatrix invCoord = joint->m_matrix.GetInverse();
	// エフェクタと到達目標のローカル位置
	localEffectorPos = invCoord * CVector4D(effectorPos.x, effectorPos.y, effectorPos.z, 1);
	localTargetPos = invCoord * CVector4D(targetPos.x, targetPos.y, targetPos.z, 1);

	// (1) 基準関節→エフェクタ位置への方向ベクトル
	CVector3D basis2Effector = localEffectorPos.GetNormalize();
	// (2) 基準関節→目標位置への方向ベクトル
	CVector3D basis2Target = localTargetPos.GetNormalize();

	// ベクトル (1) を (2) に一致させるための最短回転量（Axis-Angle）
	//
	// 回転角
	float rotationDotProduct = CVector3D::Dot(basis2Target, basis2Effector);
	float rotationAngle = acos(rotationDotProduct);
	if (rotationAngle > 1.0e-5f)
	{
		float d = CVector3D::Dot(basis2Target, basis2Effector);
		// 回転軸
		CVector3D rotationAxis = CVector3D::Cross(basis2Effector, basis2Target);
		rotationAxis.Normalize();
		// 関節回転量の補正
		{
			CVector3D p = joint->m_local_matrix.GetPosition();
			CMatrix R = joint->m_local_matrix;
			R.m03 = 0; R.m13 = 0; R.m23 = 0;
			joint->m_local_matrix = CMatrix::MTranselate(p) * R * CMatrix::MRotationAxis(rotationAxis, rotationAngle/1.5f);
		}
	}
}

CA3MNode* CA3MNode::FindNode(const char* name, CA3MNode* n) {
	CA3MNode* find = NULL;
	if (strcmp(n->m_name,name) == 0) return n;
	if (n->mp_child) find = FindNode(name, n->mp_child);
	if (n->mp_next && !find) find = FindNode(name, n->mp_next);
	return find;
}
CA3MNode* CA3MNode::FindNode(const int no, CA3MNode* n) {
	CA3MNode* find = NULL;
	if (n->m_no == no) return n;
	if (n->mp_child) find = FindNode(no, n->mp_child);
	if (n->mp_next && !find) find = FindNode(no, n->mp_next);
	return find;
}

CA3MPoly::CA3MPoly(unsigned int* use_skin):mp_vertex(nullptr),mp_use_skin(use_skin),m_vertex_cnt(0),m_material(0),m_buffer(0), m_colIdex(nullptr), m_colmask(nullptr){

}
CA3MPoly::~CA3MPoly()
{
	if(m_buffer) glDeleteBuffers(1, &m_buffer);
	//	glDeleteVertexArrays(1, &vao);
	if(mp_vertex) delete[] mp_vertex;
	if (m_colIdex) delete[] m_colIdex;
	if (m_colmask) delete[] m_colmask;
}
int CA3MPoly::Load(const char* buf)
{

	const char* b = buf;
	memcpy(&m_vertex_cnt, b, sizeof(m_vertex_cnt));
	b += sizeof(m_vertex_cnt);
	memcpy(&m_material, b, sizeof(m_material));
	b += sizeof(m_material);
	const MY_VERTEX* v = (const MY_VERTEX*)b;
	mp_vertex = new SKinVertex[m_vertex_cnt];
	for (unsigned int i = 0; i < m_vertex_cnt; i += 3) {
		memcpy(&mp_vertex[i], v++, sizeof(MY_VERTEX));
		memcpy(&mp_vertex[i+1], v++, sizeof(MY_VERTEX));
		memcpy(&mp_vertex[i+2], v++, sizeof(MY_VERTEX));
		CVector3D tangent;
		CModel::CalcTangentAndBinormal(
			mp_vertex[i].v,
			mp_vertex[i].t,
			mp_vertex[i+ 1].v,
			mp_vertex[i+ 1].t,
			mp_vertex[i + 2].v,
			mp_vertex[i + 2].t,
			&tangent);
		mp_vertex[i].tan = tangent;
		mp_vertex[i + 1].tan = tangent;
		mp_vertex[i + 2].tan = tangent;
	}

	//memcpy(mp_vertex, b, sizeof(MY_VERTEX) * m_vertex_cnt);
	return sizeof(m_vertex_cnt) + sizeof(m_material) + sizeof(MY_VERTEX) * m_vertex_cnt;

}

void CA3MPoly::Draw(const CShader* shader)
{
	if (m_buffer == 0){
		glGenBuffers(1, &m_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(SKinVertex) * m_vertex_cnt, mp_vertex, GL_STATIC_DRAW);
		delete[] mp_vertex;
		mp_vertex = nullptr;
	} else {
		glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
	}

	BYTE* offset = 0;
	glEnableVertexAttribArray(CShader::eVertexLocation);
	glVertexAttribPointer(CShader::eVertexLocation, 3, GL_FLOAT, GL_FALSE, sizeof(SKinVertex), reinterpret_cast<const void*>(offset));
	offset += sizeof(SVector3D);


	glEnableVertexAttribArray(CShader::eNormalLocation);
	glVertexAttribPointer(CShader::eNormalLocation, 3, GL_FLOAT, GL_FALSE, sizeof(SKinVertex), reinterpret_cast<const void*>(offset));
	offset += sizeof(SVector3D);

	glEnableVertexAttribArray(CShader::eTexcoordlLocation);
	glVertexAttribPointer(CShader::eTexcoordlLocation, 2, GL_FLOAT, GL_FALSE, sizeof(SKinVertex), reinterpret_cast<const void*>(offset));
	offset += sizeof(SVector2D);


	if (*mp_use_skin == 1) {
		glEnableVertexAttribArray(CShader::eWeightsLocation);
		glVertexAttribPointer(CShader::eWeightsLocation, 4, GL_FLOAT, GL_TRUE, sizeof(SKinVertex), reinterpret_cast<const void*>(offset));
		offset += sizeof(float) * 4;


		glEnableVertexAttribArray(CShader::eIndicesLocation);
		glVertexAttribPointer(CShader::eIndicesLocation, 4, GL_FLOAT, GL_FALSE, sizeof(SKinVertex), reinterpret_cast<const void*>(offset));
		offset += sizeof(float) * 4;
	} else {
		offset += sizeof(float) * 4 + sizeof(float) * 4;

	}

	glEnableVertexAttribArray(CShader::eTangentLocation);
	glVertexAttribPointer(CShader::eTangentLocation, 3, GL_FLOAT, GL_FALSE, sizeof(SKinVertex), reinterpret_cast<const void*>(offset));


	glDrawArrays(GL_TRIANGLES, 0, m_vertex_cnt);


	glDisableVertexAttribArray(CShader::eVertexLocation);
	glDisableVertexAttribArray(CShader::eNormalLocation);
	glDisableVertexAttribArray(CShader::eTexcoordlLocation);
	glDisableVertexAttribArray(CShader::eWeightsLocation);
	glDisableVertexAttribArray(CShader::eIndicesLocation);
	glDisableVertexAttribArray(CShader::eTangentLocation);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void CA3MPoly::CreateCollisionIndex(const CA3MColl*c) {
	SKinVertex* v = mp_vertex;
	/*if (m_buffer) {
		glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
		v = (MY_VERTEX*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
	}
	else {
		v = mp_vertex;
	}*/
	m_col_cut = c;


	int xyz = c->m_cut.ix * c->m_cut.iy * c->m_cut.iz;
	m_colIdex = new std::vector<int>[xyz];
	m_colmask = new unsigned long[(m_vertex_cnt / 3 / 32) + 1];

	for (unsigned int i = 0; i < m_vertex_cnt; i += 3, v += 3) {


		float l = fminf(fminf(v[0].v.x, v[1].v.x), v[2].v.x);
		float r = fmaxf(fmaxf(v[0].v.x, v[1].v.x), v[2].v.x);
		float t = fmaxf(fmaxf(v[0].v.z, v[1].v.z), v[2].v.z);
		float b = fminf(fminf(v[0].v.z, v[1].v.z), v[2].v.z);
		float u = fmaxf(fmaxf(v[0].v.y, v[1].v.y), v[2].v.y);
		float d = fminf(fminf(v[0].v.y, v[1].v.y), v[2].v.y);

		CVector3D e1 = v[1].v - v[0].v;
		CVector3D e2 = v[2].v - v[0].v;

		v[0].n = CVector3D::Cross(e1, e2).GetNormalize();
		CVector3D s, e;
		s.ix = max(0, min(c->m_cut.ix - 1, (int)((l - c->m_min.x) / c->m_length.x)));
		e.ix = max(0, min(c->m_cut.ix - 1, (int)((r - c->m_min.x) / c->m_length.x)));

		s.iz = max(0, min(c->m_cut.iz - 1, (int)((b - c->m_min.z) / c->m_length.z)));
		e.iz = max(0, min(c->m_cut.iz - 1, (int)((t - c->m_min.z) / c->m_length.z)));

		s.iy = max(0, min(c->m_cut.iy - 1, (int)((d - c->m_min.y) / c->m_length.y)));
		e.iy = max(0, min(c->m_cut.iy - 1, (int)((u - c->m_min.y) / c->m_length.y)));

		for (int lineY = s.iy; lineY <= e.iy; lineY++) {
			for (int lineZ = s.iz; lineZ <= e.iz; lineZ++) {
				for (int lineX = s.ix; lineX <= e.ix; lineX++) {
					int idx = c->GetIndex(lineX, lineY, lineZ);
					m_colIdex[idx].push_back(i / 3);
				}
			}
		}

	}
	/*
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
	*/

}
bool CA3MPoly::CollisionRay(CVector3D * c, CVector3D * n, const CVector3D & s, const CVector3D & e, const CVector3D & dir, float* pLength, const CVector3D &start, const CVector3D& end) {
	SKinVertex* v = mp_vertex;
	/*if (m_buffer) {
		glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
		v = (MY_VERTEX*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
	} else {
		v = mp_vertex;
	}*/

	//	SVertex *v = mp_vertex_array;
	memset(m_colmask, 0, sizeof(unsigned long) * ((static_cast<size_t>(m_vertex_cnt) / 3 / 32) + 1));


	bool hit = false;

	for (int lineY = start.iy; lineY <= end.iy; lineY++) {
		for (int lineZ = start.iz; lineZ <= end.iz; lineZ++) {
			for (int lineX = start.ix; lineX <= end.ix; lineX++) {
				int ceil = m_col_cut->GetIndex(lineX, lineY, lineZ);
				if (!CCollision::CollisionAABBRay(NULL, NULL, m_col_cut->m_colmin[ceil], m_col_cut->m_colmax[ceil], s, dir)) continue;
				for (unsigned int i = 0; i < m_colIdex[ceil].size(); i++) {
					int idx = m_colIdex[ceil][i];
					if (m_colmask[idx / 32] & (1 << (idx % 32))) continue;
					if (CCollision::IntersectTriangleRay(c, s, e, v[idx * 3].v, v[idx * 3 + 1].v, v[idx * 3 + 2].v, pLength)) {

						CVector3D e1 = v[idx * 3 + 1].v - v[idx * 3].v;
						CVector3D e2 = v[idx * 3 + 2].v - v[idx * 3].v;

						*n = CVector3D::Cross(e1, e2).GetNormalize();
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

	//glUnmapBuffer(GL_ARRAY_BUFFER);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	return hit;
}

void CA3MPoly::CollisionRay(std::vector<CCollTriangle>& out, const CVector3D& s, const CVector3D& e, const CVector3D& dir, const CVector3D& start, const CVector3D& end)
{
	SKinVertex* v = mp_vertex;
	/*if (m_buffer) {
		glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
		v = (MY_VERTEX*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
	} else {
		v = mp_vertex;
	}*/

	//	SVertex *v = mp_vertex_array;
	memset(m_colmask, 0, sizeof(unsigned long) * ((static_cast<size_t>(m_vertex_cnt) / 3 / 32) + 1));



	for (int lineY = start.iy; lineY <= end.iy; lineY++) {
		for (int lineZ = start.iz; lineZ <= end.iz; lineZ++) {
			for (int lineX = start.ix; lineX <= end.ix; lineX++) {
				int ceil = m_col_cut->GetIndex(lineX, lineY, lineZ);
				if (!CCollision::CollisionAABBRay(NULL, NULL, m_col_cut->m_colmin[ceil], m_col_cut->m_colmax[ceil], s, dir)) continue;
				for (unsigned int i = 0; i < m_colIdex[ceil].size(); i++) {
					int idx = m_colIdex[ceil][i];
					if (m_colmask[idx / 32] & (1 << (idx % 32))) continue;
					float dist = FLT_MAX;
					if (CCollision::IntersectTriangleRay(nullptr, s, e, v[idx * 3].v, v[idx * 3 + 1].v, v[idx * 3 + 2].v, &dist)) {

						out.push_back(CCollTriangle(
							v[idx * 3].n,
							v[idx * 3 + 0].v,
							v[idx * 3 + 1].v,
							v[idx * 3 + 2].v,
							dist,
							m_material));
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

	//glUnmapBuffer(GL_ARRAY_BUFFER);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);

}


void CA3MPoly::CollisionSphere(std::vector<CCollTriangle>& out, const CVector3D & center, float radius,const CVector3D& start, const CVector3D& end) {
	SKinVertex* v = mp_vertex;
	/*if (m_buffer) {
		glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
		v = (MY_VERTEX*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
	}
	else {
		v = mp_vertex;
	}*/

	//	SVertex *v = mp_vertex_array;
	memset(m_colmask, 0, sizeof(unsigned long) * ((static_cast<size_t>(m_vertex_cnt) / 3 / 32) + 1));

	int cnt = 0;
	float dist;
	for (int lineY = start.iy; lineY <= end.iy; lineY++) {
		for (int lineZ = start.iz; lineZ <= end.iz; lineZ++) {
		//	int ceil_x = m_col_cut->GetIndex(0, lineY, lineZ);
			for (int lineX = start.ix; lineX <= end.ix; lineX++) {
			//	int ceil = ceil_x + lineX;
				int ceil = m_col_cut->GetIndex(lineX, lineY, lineZ);
				for (unsigned int i = 0, s = (unsigned int)m_colIdex[ceil].size(); i < s; i++) {
					int idx = m_colIdex[ceil][i];
					if (m_colmask[idx / 32] & (1 << (idx % 32))) continue;
					if (CCollision::CollisionTriangleSphere(v[idx * 3].v, v[idx * 3 + 1].v, v[idx * 3 + 2].v, v[idx * 3].n, center, radius, NULL, &dist)) {

						out.push_back(CCollTriangle(
							v[idx * 3].n,
							v[idx * 3 + 0].v,
							v[idx * 3 + 1].v,
							v[idx * 3 + 2].v,
							dist,
							m_material));
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

	//glUnmapBuffer(GL_ARRAY_BUFFER);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	//return cnt;
}

void CA3MPoly::CollisionCapsule(std::vector<CCollTriangle>& out, const CCapsule& cap,const CVector3D& start, const CVector3D& end) {
	SKinVertex* v = mp_vertex;
	/*if (m_buffer) {
		glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
		v = (MY_VERTEX*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
	}
	else {
		v = mp_vertex;
	}*/


	memset(m_colmask, 0, sizeof(unsigned long) * ((static_cast<size_t>(m_vertex_cnt) / 3 / 32) + 1));

	int cnt = 0;
	float dist;
	for (int lineY = start.iy; lineY <= end.iy; lineY++) {
		for (int lineZ = start.iz; lineZ <= end.iz; lineZ++) {
			for (int lineX = start.ix; lineX <= end.ix; lineX++) {
				int ceil = m_col_cut->GetIndex(lineX, lineY, lineZ);
				for (unsigned int i = 0; i < m_colIdex[ceil].size(); i++) {
					int idx = m_colIdex[ceil][i];
					if (m_colmask[idx / 32] & (1 << (idx % 32))) continue;
					if (CCollision::CollisionTriangleCapsule(v[idx * 3].v, v[idx * 3 + 1].v, v[idx * 3 + 2].v, v[idx * 3].n, cap, NULL, &dist)) {
						
						out.push_back(CCollTriangle(
							v[idx * 3].n,
							v[idx * 3 + 0].v,
							v[idx * 3 + 1].v,
							v[idx * 3 + 2].v,
							dist,
							m_material ));

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

	//glUnmapBuffer(GL_ARRAY_BUFFER);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	//return cnt;
}

CA3MMesh::CA3MMesh(bool* p_enable_animation):m_use_skin(0), mp_enable_animation(p_enable_animation)
{
}
/*
CA3MMesh::CA3MMesh(const CA3MMesh& mesh): m_col_cut(mesh.m_col_cut), mp_enable_animation(nullptr)
{
	m_poly_list = mesh.m_poly_list;
	m_poly_cnt = mesh.m_poly_cnt;
	m_use_skin = mesh.m_use_skin;
}
*/
CA3MMesh::~CA3MMesh()
{
}

CA3MMesh& CA3MMesh::operator=(const CA3MMesh& mesh)
{
	m_poly_list = mesh.m_poly_list;
	m_poly_cnt = mesh.m_poly_cnt;
	m_use_skin = mesh.m_use_skin;
	m_offset = mesh.m_offset;
	return *this;
}

int CA3MMesh::Load(const char* buf)
{
	const char* b = buf;
	int idx = 0,size;
	size = sizeof(int);
	memcpy(&m_poly_cnt,b,size);
	b += size;
	idx += size;

	size = sizeof(int);
	memcpy(&m_use_skin, b, size);
	b += size;
	idx += size;

	size = sizeof(int);
	int offset_cnt;
	memcpy(&offset_cnt, b, size);
	b += size;
	idx += size;


	if (offset_cnt > 0) {
		SA3MBoneOffsetCHK* of = new SA3MBoneOffsetCHK[offset_cnt];

		size = sizeof(SA3MBoneOffsetCHK) * offset_cnt;
		memcpy(of, b, size);
		b += size;
		idx += size;
		for (int i = 0; i < offset_cnt; i++) {
			//CA3MNode* node = m->GetNode(of[i].no);
			//m_offset.push_back(SBoneOffset(node, of[i].offset));
			m_offset.push_back(SBoneOffset(of[i].no, of[i].offset));
		}
		delete[] of;
	}
	for (unsigned int i = 0; i < m_poly_cnt; i++) {
		CA3MPoly* p = new CA3MPoly(&m_use_skin);
		size = p->Load(b);
		b += size;
		idx += size;
		m_poly_list.push_back(p);
	}

	return idx;

}

void CA3MMesh::Release()
{
	for (auto it = m_poly_list.begin(); it != m_poly_list.end(); it++) {
		if (*it) delete (*it);
	}
	m_poly_list.clear();

}


void CA3MMesh::Draw(std::vector<CMaterial*>& materialList, CMatrix* send_matrix, CMatrix* bone_matrix, int bone_size, const CMatrix& mv, const CMatrix& m, const CMatrix& lm, float shadow, bool toon) 
{

	if (send_matrix) {
		for (const auto& v : m_offset) {
			send_matrix[v.m_bone_no] = bone_matrix[v.m_bone_no] * v.m_offset;
		}
	}
	for (auto it = m_poly_list.begin(); it != m_poly_list.end(); it++) {
		CMaterial* mat = materialList[(*it)->m_material];
		mat->mp_shader = CShader::GetInstance(mat->m_shader_name);
		mat->mp_shader->Enable();
		glUniform1f(glGetUniformLocation(mat->mp_shader->GetProgram(), "shadow_bias"), shadow);
		glUniform1i(glGetUniformLocation(mat->mp_shader->GetProgram(), "toon"), toon ? 1 : 0);
		CModel::SendShaderParam(mat->mp_shader,CMatrix::indentity, CCamera::GetCurrent()->GetViewMatrix(), CCamera::GetCurrent()->GetProjectionMatrix());

		if (send_matrix) {
			int MatrixLocation = glGetUniformLocation(mat->mp_shader->GetProgram(), "Transforms");
			glUniformMatrix4fv(MatrixLocation, bone_size, GL_FALSE, send_matrix[0].f);
		}
		//if (m_vertex_type == A3M::eSkinMesh) {
//} else {
		glUniformMatrix4fv(glGetUniformLocation(mat->mp_shader->GetProgram(), "LocalMatrix"), 1, GL_FALSE, lm.f);
		glUniformMatrix4fv(glGetUniformLocation(mat->mp_shader->GetProgram(), "ModelViewMatrix"), 1, GL_FALSE, mv.f);
		glUniformMatrix4fv(glGetUniformLocation(mat->mp_shader->GetProgram(), "WorldMatrix"), 1, false, m.f);
		//}

		glUniform1i(glGetUniformLocation(mat->mp_shader->GetProgram(), "useSkin"), *mp_enable_animation ? m_use_skin : 0);


		mat->Map();
		(*it)->Draw(mat->mp_shader);
		mat->Unmap();
		mat->mp_shader->Disable();
	}
}

void CA3MMesh::CreateCollisionIndex(const CA3MColl* c)
{
	m_col_cut = c;
	for (auto it = m_poly_list.begin(); it != m_poly_list.end(); it++) {
		(*it)->CreateCollisionIndex(c);
	}
}

bool CA3MMesh::CollisionRay(CVector3D* c, CVector3D* n, const CVector3D& s, const CVector3D& e, float* pLength, const CMatrix& matrix)
{
	bool hit = false;
	float scale = CVector3D(matrix.m00, matrix.m10, matrix.m20).Length();
	CMatrix inv = matrix.GetInverse();
	CVector3D ls = inv * s;
	CVector3D le = inv * e;
	float llengh = *pLength / scale;
	CVector3D dir = (le - ls).GetNormalize();


	
	CAABB aabb(
		CVector3D(min(ls.x, le.x), min(ls.y, le.y), min(ls.z, le.z)),
		CVector3D(max(ls.x, le.x), max(ls.y, le.y), max(ls.z, le.z)));


	CVector3D start, end;
	m_col_cut->CalcRange(&start, &end, aabb);

	for (auto it = m_poly_list.begin(); it != m_poly_list.end(); it++) {
		if((*it)->CollisionRay(c, n, ls, le, dir, &llengh,start,end))
			hit = true;
	}
	if (hit) {
		*c = matrix * *c;
		*n = (matrix * *n).GetNormalize();
		*pLength = llengh * scale;
	}
	return hit;
}

void CA3MMesh::CollisionRay(std::vector<CCollTriangle>& out, const CVector3D& s, const CVector3D& e, const CMatrix& matrix)
{
	bool hit = false;
	float scale = CVector3D(matrix.m00, matrix.m10, matrix.m20).Length();
	CMatrix inv = matrix.GetInverse();
	CVector3D ls = inv * s;
	CVector3D le = inv * e;
	CVector3D dir = (le - ls).GetNormalize();



	CAABB aabb(
		CVector3D(min(ls.x, le.x), min(ls.y, le.y), min(ls.z, le.z)),
		CVector3D(max(ls.x, le.x), max(ls.y, le.y), max(ls.z, le.z)));


	CVector3D start, end;
	m_col_cut->CalcRange(&start, &end, aabb);

	std::vector<CCollTriangle> add_tri;

	for (auto& v : m_poly_list) {
		v->CollisionRay(add_tri, ls, le, dir, start, end);

	}
	for (auto& v : add_tri)
	{
		v.m_dist *= scale;
		v.Transform(matrix);
	}
	out.insert(out.end(), add_tri.begin(), add_tri.end()); // 連結
}

void CA3MMesh::CollisionSphere(std::vector<CCollTriangle>& out,const CVector3D& center, float radius, const CMatrix& matrix)
{

	float scale = CVector3D(matrix.m00, matrix.m10, matrix.m20).Length();
	int cnt = 0;
	CVector3D p = matrix.GetInverse() * CVector4D(center, 1);
	float r = radius / scale;
	CAABB aabb(
		CVector3D(p.x - r, p.y - r, p.z - r),
		CVector3D(p.x + r, p.y + r, p.z + r));


	CVector3D start, end;
	m_col_cut->CalcRange(&start, &end, aabb);

	std::vector<CCollTriangle> add_tri;

	for (auto& v : m_poly_list) {
		v->CollisionSphere(add_tri, p, r,start,end);
	}
	for (auto& v : add_tri)
	{
		v.m_dist *= scale;
		v.Transform(matrix);
	}
	out.insert(out.end(), add_tri.begin(), add_tri.end()); // 連結

}

void CA3MMesh::CollisionCapsule(std::vector<CCollTriangle>& out, const CCapsule& cap, const CMatrix& matrix)
{
	float scale = CVector3D(matrix.m00, matrix.m10, matrix.m20).Length();
	int cnt = 0;
	CMatrix inv = matrix.GetInverse();
	CVector3D t = inv * CVector4D(cap.GetStartPoint(), 1);
	CVector3D b = inv * CVector4D(cap.GetEndPoint(), 1);
	float r = cap.GetRadius() / scale;


	CAABB aabb(
	CVector3D(min(t.x, b.x) - cap.GetRadius(), min(t.y, b.y) - cap.GetRadius(), min(t.z, b.z) - cap.GetRadius()),
	CVector3D(max(t.x, b.x) + cap.GetRadius(), max(t.y, b.y) + cap.GetRadius(), max(t.z, b.z) + cap.GetRadius()));


	CVector3D start, end;
	m_col_cut->CalcRange(&start, &end, aabb);

	std::vector<CCollTriangle> add_tri;

	for (auto& v : m_poly_list) {
		v->CollisionCapsule(out, CCapsule(t,b,r), start, end);
	}
	for (auto& v : add_tri)
	{
		v.m_dist *= scale;
		v.Transform(matrix);
	}
	out.insert(out.end(), add_tri.begin(), add_tri.end()); // 連結
}

CA3MNode* CModelA3M::LoadNode(FILE *fp, CA3MNode* parent, CA3MNode* prev)
{
	SA3MNodeCHK nc;
	fread(&nc, sizeof(SA3MNodeCHK), 1, fp);

	CA3MNode* node = new CA3MNode();
	m_node_list.push_back(node);
	if (nc.m_node_type == A3M::eMesh) {
		CA3MMesh* m = new CA3MMesh(&m_enable_animation);
		node->mp_mesh = m;
		node->m_node_type = A3M::eMesh;
		m_mesh_list.push_back(node);
		unsigned int buf_size = nc.node_size - sizeof(SA3MNodeCHK);
		char* buf = (char*)malloc(buf_size);
		fread(buf,buf_size, 1, fp);
		m->Load(buf);
		free(buf);
	}
	node->m_no = nc.m_no;
	node->m_bone_no = nc.m_bone_no;
	strcpy_s(node->m_name,sizeof(node->m_name),nc.m_name);
	node->m_node_type = (A3M::E_NODE_TYPE)nc.m_node_type;
	node->m_local_matrix = nc.m_local_matrix;
	node->m_offset = nc.m_offset;
	node->m_pos = nc.m_pos;
	node->m_rot = nc.m_rot;
	node->m_scale = nc.m_scale;
	node->m_visibility = nc.m_visibility;
	//memcpy(node, &nc, sizeof(SA3MNodeCHK)-sizeof(nc.node_size));

	if (node->mp_parent = parent) {
		if (!parent->mp_child) parent->mp_child = node;
	}
	if (node->mp_prev = prev) prev->mp_next = node;
	if (nc.child_offset) {
		LoadNode(fp,node,nullptr);
	}
	if(nc.next_offset){
		LoadNode(fp, parent,node);
	}


	return node;
}

int CModelA3M::LoadAnimation(FILE* fp)
{

	fread(&m_animation_size, sizeof(m_animation_size), 1, fp);
	for (int i = 0; i < m_animation_size; i++) {
		CA3MAnimationSet* s = new CA3MAnimationSet();
		fread(s, sizeof(s->m_name) + sizeof(float) + sizeof(int) * 2, 1, fp);
		unsigned int bone_size;
		fread(&bone_size, sizeof(unsigned int), 1, fp);
		for (unsigned int k = 0; k < bone_size; k++) {
			CA3MAnimationAction* aa = new CA3MAnimationAction();
			int no;
			fread(&no, sizeof(int), 1, fp);
			fread(&aa->m_size, sizeof(int), 1, fp);
			aa->m_keys = new CA3MAnimationKey[aa->m_size];
			fread(aa->m_keys, sizeof(CA3MAnimationKey) * aa->m_size, 1, fp);
			s->m_bones[no] = aa;
		}
		m_animation_list.push_back(s);
	}
	return 0;
}

void CModelA3M::DrawMesh(CA3MNode* node,const CMatrix&view_matrix)
{
	//CMatrix world_matrix = parent_matrix*node->m_local_matrix;
	if (node->mp_mesh && node->m_visibility) {
		CA3MMesh* m = node->mp_mesh;
		m->Draw(m_material_list, m_send_matrix, m_bone_matrix, m_bone_num, (view_matrix * m_matrix), m_matrix, node->m_matrix, m_shadow_bias, m_toon);
	}
	if (node->mp_child) DrawMesh(node->mp_child,view_matrix);
	if (node->mp_next) DrawMesh(node->mp_next,view_matrix);
}


void CModelA3M::CalcBoneMatrix(CA3MNode* node,bool reset)
{
	if (m_enable_animation && reset) {
		CA3MAnimation* a = &m_animation[node->m_animation_layer];
		CA3MAnimationSet* set = m_animation_list[a->m_jam];
		CA3MAnimationAction* aa = set->m_bones[node->m_bone_no];
		if (aa) {
			CA3MAnimationKey* key = &aa->m_keys[(int)a->m_frame];
			CA3MAnimationKey* key2 = &aa->m_keys[((int)a->m_frame+1) % aa->m_size];
			//node->m_local_matrix = CMatrix::MTranselate(key->m_position) * CMatrix::MRotation(key->m_rotation) * CMatrix::MScale(key->m_scale);
			float d;
			float t = modff(a->m_frame, &d);
			node->m_local_matrix = key->m_matrix*(1-t) + key2->m_matrix * t;
		}
		else {

		}
		

	} else {

	}
	/*
	if (node->m_px_actor && !(node->m_px_actor->getRigidBodyFlags() & physx::PxRigidBodyFlag::eKINEMATIC)) {
		CMatrix pm = node->m_matrix;
		CVector3D scale(pm.GetLeft().Length(), pm.GetUp().Length(), pm.GetFront().Length());
		CMatrix m = m_matrix.GetInverse() * ToMatrix(physx::PxMat44(node->m_px_actor->getGlobalPose())) * CMatrix::MScale(scale);
		node->m_matrix = m;
			if (::GetKeyState('Z') & 0x80) {
				node->m_px_actor->clearForce();
				node->m_px_actor->clearTorque();
			}

	} else */ {

		if (node->mp_parent) {
			if (node->m_bind) {

				CMatrix pm = m_matrix;
				CVector3D scale(pm.GetLeft().Length(), pm.GetUp().Length(), pm.GetFront().Length());
				CMatrix mm;
				mm.Scale(scale);

				////親の行列から回転行列のみ抽出した行列
				CMatrix cc = pm * mm.GetInverse();
				cc.Transelate(0, 0, 0);

				////親の行列から回転成分を排除した行列
				/*pm.m00 = scale.x; pm.m01 = 0; pm.m02 = 0;
				pm.m10 = 0; pm.m11 = scale.y; pm.m12 = 0;
				pm.m20 = 0; pm.m21 = 0; pm.m22 = scale.z;

				pm.Scale(scale);
				*/
				CMatrix ll = m_matrix * node->mp_parent->m_matrix * node->m_local_matrix;
				CMatrix l = ll;
				l.m00 = 1; l.m01 = 0; l.m02 = 0;
				l.m10 = 0; l.m11 = 1; l.m12 = 0;
				l.m20 = 0; l.m21 = 0; l.m22 = 1;

				node->m_matrix = m_matrix.GetInverse() * l * node->m_bind_matrix * cc.GetInverse() * l.GetInverse() * ll;

				/*
				CMatrix pm = m_matrix;

				////親の行列から回転成分を排除した行列
				CMatrix mm;
				mm.Scale(pm.GetRight().Length(), pm.GetUp().Length(), pm.GetFront().Length());

				////親の行列から回転行列のみ抽出した行列
				CMatrix cc = pm * mm.GetInverse();
				cc.Transelate(0, 0, 0);

				CMatrix lt = node->m_local_matrix;
				lt.m00 = 1; lt.m01 = 0; lt.m02 = 0;
				lt.m10 = 0; lt.m11 = 1; lt.m12 = 0;
				lt.m20 = 0; lt.m21 = 0; lt.m22 = 1;

				CMatrix pl = node->mp_parent->m_matrix* node->m_local_matrix;
				CMatrix plt = pl;
				plt.m00 = 1; plt.m01 = 0; plt.m02 = 0;
				plt.m10 = 0; plt.m11 = 1; plt.m12 = 0;
				plt.m20 = 0; plt.m21 = 0; plt.m22 = 1;
				CMatrix pltiv = plt.GetInverse();

				CMatrix plr = pltiv*pl;

				CMatrix pt = CMatrix::MTranselate(node->mp_parent->m_matrix.GetPosition());

				CMatrix pp = pltiv * pl;

				CMatrix m1 = pltiv * pl;
				CMatrix m2 = node->m_bind_matrix * m1;
				CMatrix m3 = plt * m2;
				node->m_matrix = cc.GetInverse()*m3;
				*/
			}
			else {
				node->m_matrix = node->mp_parent->m_matrix * node->m_local_matrix;

			}
		} else {
			//node->m_matrix = m_matrix * node->m_local_matrix;
			node->m_matrix = node->m_local_matrix;
		}


	}
	if (m_bone_matrix && node->m_bone_no >= 0) {
		//		m_bone_matrix[node->m_bone_no] = node->m_matrix *node->m_offset;
		float t = 1.0f;
		if(m_animation.size()>0) t = m_animation[node->m_animation_layer].m_blend;
		m_bone_matrix[node->m_bone_no] = m_old_matrix[node->m_bone_no] * (1 - t) + node->m_matrix * t;
	}
	if (node->mp_child) CalcBoneMatrix(node->mp_child, reset);
	if (node->mp_next) CalcBoneMatrix(node->mp_next, reset);
}

void CModelA3M::CreateCollisionIndex(int cutx, int cuty, int cutz)
{

	m_col_cut->m_cut.ix = cutx;
	m_col_cut->m_cut.iy = cuty;
	m_col_cut->m_cut.iz = cutz;
	m_col_cut->m_length.x = max(0.0f, (m_col_cut->m_max.x - m_col_cut->m_min.x) / m_col_cut->m_cut.ix);
	m_col_cut->m_length.y = max(0.0f, (m_col_cut->m_max.y - m_col_cut->m_min.y) / m_col_cut->m_cut.iy);
	m_col_cut->m_length.z = max(0.0f, (m_col_cut->m_max.z - m_col_cut->m_min.z) / m_col_cut->m_cut.iz);
	//printf("cut_length(%f %f %f)\n", m_length.x, m_length.y, m_length.z);
	int xyz = m_col_cut->m_cut.ix * m_col_cut->m_cut.iy * m_col_cut->m_cut.iz;
	m_col_cut->m_colmin = new CVector3D[xyz];
	m_col_cut->m_colmax = new CVector3D[xyz];
	for (int k = 0; k < m_col_cut->m_cut.iy; k++) {
		for (int i = 0; i < m_col_cut->m_cut.iz; i++) {
			for (int j = 0; j < m_col_cut->m_cut.ix; j++) {
				int idx = m_col_cut->GetIndex(j, k, i);
				m_col_cut->m_colmin[idx] = CVector3D(m_col_cut->m_min.x + j * m_col_cut->m_length.x, m_col_cut->m_min.y + k * m_col_cut->m_length.y, m_col_cut->m_min.z + i * m_col_cut->m_length.z);
				m_col_cut->m_colmax[idx] = CVector3D(m_col_cut->m_min.x + (j + 1) * m_col_cut->m_length.x, m_col_cut->m_min.y + (k + 1) * m_col_cut->m_length.y, m_col_cut->m_min.z + (i + 1) * m_col_cut->m_length.z);

			}
		}
	}
	for (auto it = m_mesh_list.begin(); it != m_mesh_list.end(); it++) {
		(*it)->mp_mesh->CreateCollisionIndex(m_col_cut);
	}

}

CA3MNode* CModelA3M::DuplicateNode(CA3MNode* pNode, CA3MNode* parent, CA3MNode* prev)
{
	CA3MNode* node = new CA3MNode(*pNode);


	m_node_list.push_back(node);
	if (pNode->GetNodeType() == A3M::eMesh) {
		m_mesh_list.push_back(node);
		node->m_node_type = A3M::eMesh;
	}

	if (node->mp_parent = parent) {
		if (!parent->mp_child) parent->mp_child = node;
		node->m_matrix = parent->m_matrix * node->m_local_matrix;
	}
	else {
		node->m_matrix = node->m_local_matrix;
	}
	if (node->mp_prev = prev) prev->mp_next = node;


	if (pNode->mp_child) {
		DuplicateNode(pNode->mp_child, node, nullptr);
	}
	CA3MNode* child = node;
	if (pNode->mp_next) {
		child = DuplicateNode(pNode->mp_next, node->mp_parent, child);
	}


	return node;
}

void CModelA3M::ReleaseNode(CA3MNode* pNode) {

	

	if (pNode->mp_child) {
		ReleaseNode(pNode->mp_child);
	}
	if (pNode->mp_next) {
		ReleaseNode(pNode->mp_child);
	}

	pNode->Release();


	
}
CModelA3M::CModelA3M(): mp_root(nullptr), m_bone_num(0), m_send_matrix(nullptr),m_bone_matrix(nullptr), m_animation_size(0)
{
	m_shadow_bias = 0.0001f;
}

CModelA3M::~CModelA3M()
{	
	if (mp_root) delete mp_root;
	if (m_send_matrix) delete[] m_send_matrix;
	if (m_bone_matrix) delete[] m_bone_matrix;
	if (m_old_matrix) delete[] m_old_matrix;
	for (auto &m : m_material_list)
		delete m;

#ifdef USE_PHYSX_LIB
	for (auto& j : m_joint_list)
		delete j;
#endif
}

CModelA3M::CModelA3M(const CModelA3M& m)
{
	*this = m;
}
void CModelA3M::operator=(const CModelA3M& m)
{

	for (auto it = m.m_material_list.begin(); it != m.m_material_list.end(); it++)
		m_material_list.push_back(new CMaterial(**it));
	mp_root = DuplicateNode(m.mp_root, nullptr, nullptr);
	m_bone_num = m.m_bone_num;
	m_send_matrix = new CMatrix[m_bone_num];
	m_bone_matrix = new CMatrix[m_bone_num];
	m_old_matrix = new CMatrix[m_bone_num];
	memcpy(m_send_matrix, m.m_send_matrix, sizeof(CMatrix) * m_bone_num);
	memcpy(m_bone_matrix, m.m_bone_matrix, sizeof(CMatrix) * m_bone_num);
	memcpy(m_old_matrix, m.m_old_matrix, sizeof(CMatrix) * m_bone_num);
	m_animation_list = m.m_animation_list;
	m_animation_size = m.m_animation_size;
	if (m.m_enable_animation) {
		m_animation.push_back(&m_animation_list);
		m_animation.push_back(&m_animation_list);
		m_enable_animation = m.m_enable_animation;
	}
	m_col_cut = m.m_col_cut;
	m_shadow_bias = m.m_shadow_bias;
	m_matrix = m.m_matrix;
	UpdateMatrix();
	CJointBone* prev = nullptr;

#ifdef USE_PHYSX_LIB
	for (auto& jont : m.m_joint_list) {
		CJointBone* j = new CJointBone(GetNode(jont->m_joint_parent->GetNo()), prev, prev ? prev->m_px_actor : nullptr, GetNode(jont->m_joint_child->GetNo()), m_matrix);
		m_joint_list.push_back(j);
		prev = j;
	}
#endif
}
CModelA3M::CModelA3M(const CModelObj& m) : m_bone_num(0), m_bone_matrix(nullptr), m_animation_size(0),m_enable_animation(false),m_old_matrix(nullptr), m_send_matrix(nullptr)
{

	CA3MMesh* mesh = new CA3MMesh(&m_enable_animation);
	CA3MNode* node = new CA3MNode();
	node->mp_mesh = mesh;
	node->m_node_type = A3M::eMesh;
	m_node_list.push_back(node);
	m_mesh_list.push_back(node);
	mp_root = node;
	int array_size = 0;


	//int poly_cnt;
	array_size += sizeof(int);
	//int vetex_type;
	array_size += sizeof(int);
	//int bone_offset_cnt;
	array_size += sizeof(unsigned int);
	for (int i = 0; i < m.m_materialCnt; i++) {
		CMeshObj* o = &m.m_mesh[i];
		//m_vertex_cnt+m_material+m_vertex_cnt*MY_VERTEX;
		array_size += sizeof(unsigned long) + sizeof(unsigned long) +
			o->m_vertexNum * sizeof(MY_VERTEX);
	}

	char* buf = (char*)malloc(array_size);
	int idx = 0;
	memcpy(&buf[idx], &m.m_materialCnt, sizeof(int));
	idx += sizeof(int);
	unsigned int	use_skin = 0;
	memcpy(&buf[idx], &use_skin, sizeof(use_skin));
	idx += sizeof(int);
	unsigned int bone_offset_cnt = 0;
	memcpy(&buf[idx], &bone_offset_cnt, sizeof(bone_offset_cnt));
	idx += sizeof(unsigned int);
	m_col_cut = new CA3MColl;
	m_col_cut->m_max = CVector3D(FLT_MIN, FLT_MIN, FLT_MIN);
	m_col_cut->m_min = CVector3D(FLT_MAX, FLT_MAX, FLT_MAX);
	for (int i = 0; i < m.m_materialCnt; i++) {
		CMeshObj* o = &m.m_mesh[i];

		unsigned int	vertex_cnt = o->m_vertexNum;
		unsigned int	material = i;

		memcpy(&buf[idx], &vertex_cnt, sizeof(vertex_cnt)); idx += sizeof(vertex_cnt);
		memcpy(&buf[idx], &material, sizeof(material)); idx += sizeof(material);
		unsigned short vertex_size = sizeof(SVector3D) + sizeof(SVector3D) + sizeof(SVector2D);
		CMeshObj::SVertex* v = nullptr;
		if (o->m_buffer) {
			glBindBuffer(GL_ARRAY_BUFFER, o->m_buffer);
			v = (CMeshObj::SVertex*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
		}
		else {
			v = o->mp_vertex_array;
		}

		for (unsigned int j = 0; j < vertex_cnt; j++, v++) {
			memcpy(&buf[idx], v, vertex_size); idx += sizeof(MY_VERTEX);


			m_col_cut->m_min.x = min(m_col_cut->m_min.x, v->v.x);
			m_col_cut->m_max.x = max(m_col_cut->m_max.x, v->v.x);
			m_col_cut->m_max.z = max(m_col_cut->m_max.z, v->v.z);
			m_col_cut->m_min.z = min(m_col_cut->m_min.z, v->v.z);
			m_col_cut->m_max.y = max(m_col_cut->m_max.y, v->v.y);
			m_col_cut->m_min.y = min(m_col_cut->m_min.y, v->v.y);

		}
		if (o->m_buffer) {
			glUnmapBuffer(GL_ARRAY_BUFFER);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}
		CMaterial* mat = new CMaterial(*m.m_material[i]);
		mat->m_shader_name = "SkinMesh";
		m_material_list.push_back(mat);

	}
	mesh->Load(buf);
	free(buf);

	if(m.m_cut.ix * m.m_cut.iy * m.m_cut.ix>0)
	CreateCollisionIndex(m.m_cut.ix, m.m_cut.iy, m.m_cut.iz);
	m_shadow_bias = m.m_shadow_bias;

}

#ifdef USE_ATHOR_MODEL

CA3MNode* CModelA3M::DuplicateFBXMesh(FbxNode* pNode,int& no,CA3MNode* parent,CA3MNode* prev)
{
	CA3MNode* node=nullptr;
	CFrameFBX* pFrameFBX = (CFrameFBX*)pNode->GetUserDataPtr();
	
	
	if(!node)
		node = new CA3MNode();
	m_node_list.push_back(node);

	if (pNode->GetNodeAttribute()) {
		FbxNodeAttribute::EType lAttributeType;
		lAttributeType = pNode->GetNodeAttribute()->GetAttributeType();

		switch (lAttributeType) {
		//case FbxNodeAttribute::eSkeleton:


		//	break;
		case FbxNodeAttribute::eMesh:
		{

			CMeshFBX* pMeshFBX = (CMeshFBX*)pNode->GetUserDataPtr();
			CA3MMesh* mesh = new CA3MMesh(&m_enable_animation);
			m_mesh_list.push_back(node);
			node->mp_mesh = mesh;
			node->m_node_type = A3M::eMesh;
			unsigned int material_size = pMeshFBX->m_material_cnt;

			int array_size = 0;
			//int poly_cnt;
			array_size += sizeof(int);
			//int use_skin;
			array_size += sizeof(int);
			//int bone_cnt
			array_size += sizeof(int);
			//int offset
			array_size += sizeof(SA3MBoneOffsetCHK) * pMeshFBX->m_numBone;
			

			for (unsigned int i = 0; i < material_size; i++) {
				//m_vertex_cnt+m_material+m_vertex_cnt*MY_VERTEX;
				array_size += sizeof(unsigned long) + sizeof(unsigned long) +
					static_cast<size_t>(pMeshFBX->m_numFace[i])*3 * sizeof(MY_VERTEX);
			}


			char* buf = (char*)malloc(array_size);
			int idx = 0;
			memcpy(&buf[idx], &material_size, sizeof(int));
			idx += sizeof(int);
			unsigned int	use_skin = 0;
			if (pMeshFBX->m_numBone > 0) {
				use_skin = 1;
			}
			memcpy(&buf[idx], &use_skin, sizeof(use_skin));
			idx += sizeof(int);


			memcpy(&buf[idx], &pMeshFBX->m_numBone, sizeof(int));
			idx += sizeof(int);
			for (int i = 0; i < pMeshFBX->m_numBone; i++)
			{
				CFrameFBX* pFrame = (CFrameFBX*)pMeshFBX->m_ppCluster[i]->GetLink()->GetUserDataPtr();
				SA3MBoneOffsetCHK of;
				of.no = pFrame->m_no;
				of.offset = pMeshFBX->m_BoneArray[i].mOffset;
				memcpy(&buf[idx], &of, sizeof(SA3MBoneOffsetCHK));
				idx += sizeof(SA3MBoneOffsetCHK);
			}

			MY_VERTEX* v = nullptr;
			if (pMeshFBX->m_vertexBuffer) {
				glBindBuffer(GL_ARRAY_BUFFER, pMeshFBX->m_vertexBuffer);
				v = (MY_VERTEX*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
			} else {
				v = pMeshFBX->mp_Vertex;
			}

			for (unsigned int i = 0; i < material_size; i++) {
				

				unsigned int	vertex_cnt = pMeshFBX->m_numFace[i]*3;
				unsigned int	material = pMeshFBX->m_material[i];
				
			
				memcpy(&buf[idx], &vertex_cnt, sizeof(vertex_cnt)); idx += sizeof(vertex_cnt);
				memcpy(&buf[idx], &material, sizeof(material)); idx += sizeof(material);
				unsigned short vertex_size = sizeof(SVector3D) + sizeof(SVector3D) + sizeof(SVector2D);
				
				int* ib = nullptr;
				if (pMeshFBX->m_indexBuffers[i]) {
					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pMeshFBX->m_indexBuffers[i]);
					ib = (int*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_READ_ONLY);
				} else {
					ib = pMeshFBX->mp_Indexs[i];
				}
				for (unsigned int j = 0; j < vertex_cnt; j++, ib++) {
					MY_VERTEX* vv = (MY_VERTEX*)&v[*ib];
					memcpy(&buf[idx],vv, sizeof(MY_VERTEX)); idx += sizeof(MY_VERTEX);

					m_col_cut->m_min.x = min(m_col_cut->m_min.x, vv->vPos.x);
					m_col_cut->m_max.x = max(m_col_cut->m_max.x, vv->vPos.x);
					m_col_cut->m_max.z = max(m_col_cut->m_max.z, vv->vPos.z);
					m_col_cut->m_min.z = min(m_col_cut->m_min.z, vv->vPos.z);
					m_col_cut->m_max.y = max(m_col_cut->m_max.y, vv->vPos.y);
					m_col_cut->m_min.y = min(m_col_cut->m_min.y, vv->vPos.y);
					


				}

				if (pMeshFBX->m_indexBuffers[i]) {
					glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
				}

			}
			if (pMeshFBX->m_vertexBuffer) {
				glUnmapBuffer(GL_ARRAY_BUFFER);
				glBindBuffer(GL_ARRAY_BUFFER, 0);
			}
			mesh->Load(buf);
			free(buf);

			/*
			for (int i = 0; i < pMeshFBX->materialCnt; i++) {
				pMeshFBX->m_vertexBuffer

			}*/
		
		}
		break;
		default:

			break;
		}
	}
	node->m_no = no++;
	node->m_bone_no = node->m_no;
	strcpy_s(node->m_name,256, pFrameFBX->m_name);
	node->m_offset = pFrameFBX->matrixOffset;


	if (pNode->GetNodeAttribute() && pNode->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eMesh) {
		node->m_local_matrix  = CMatrix::indentity;
	} else {
		node->m_local_matrix = pFrameFBX->localMatrix;
	}
	if (node->mp_parent = parent) {
		if(!parent->mp_child) parent->mp_child = node;
		node->m_matrix = parent->m_matrix * node->m_local_matrix;
	} else {
		node->m_matrix = node->m_local_matrix;
	}
	if (node->mp_prev = prev) prev->mp_next = node;
	CA3MNode* child = nullptr;
	for (int i = 0; i < pNode->GetChildCount(); i++) {
		child = DuplicateFBXMesh(pNode->GetChild(i),no, node , child);
		if (i == 0) node->mp_child = child;
	}
	return node;


}

static void CopyMatrix(CMatrix* d, FbxAMatrix* s) {
	for (int y = 0; y < 4; ++y)
		for (int x = 0; x < 4; ++x)
			d->m[y][x] = (float)s->Get(y, x);
}
bool CModelA3M::DuplicateFBXAnimation(FbxNode* pNode,const CModelFBX::AnimationInfo *info,unsigned int anim_num)
{
	CFrameFBX* f = (CMeshFBX*)pNode->GetUserDataPtr();
	CA3MNode* n;
	if (f) {
		n = CA3MNode::FindNode(f->m_no, mp_root);
	} else {
		n = CA3MNode::FindNode(pNode->GetName(), mp_root);
	}
	if (!n) return false;
	//メッシュノードは除く
	if (!n->mp_mesh) {
		CMatrix mat;
		FbxAMatrix matA;

		FbxTime::EMode time_mode = info->m_lScene->GetGlobalSettings().GetTimeMode();

		FbxTime::SetGlobalTimeMode(time_mode);


		CA3MAnimationAction* aa = new CA3MAnimationAction();
		info->m_lScene->SetCurrentAnimationStack(info->m_AnimationStack);
		aa->m_size = (int)info->m_end - (int)info->m_start+1;
		aa->m_keys = new CA3MAnimationKey[aa->m_size];

		int key_frame = (int)info->m_start;


		FbxNode* p = info->m_lScene->FindNodeByName(pNode->GetName());
		FbxTime time;
		CA3MAnimationKey* k = aa->m_keys;
		int frame = -1;
		for (; key_frame <= info->m_end; key_frame++, k++) {
			frame++;
			time.SetTime(0, 0, 0, (int)key_frame, 0, 0, time_mode);
			matA = p->EvaluateLocalTransform(time);
			CopyMatrix(&k->m_matrix, &matA);
			/*
			fbxsdk::FbxQuaternion rot = matA.GetQ();
			fbxsdk::FbxVector4 pos = matA.GetT();
			fbxsdk::FbxVector4 scale = matA.GetS();
			memcpy(&k->m_rotation, &rot.mData,sizeof(CQuaternion));
			memcpy(&k->m_position, &pos.mData, sizeof(CVector3D));
			memcpy(&k->m_scale, &scale.mData, sizeof(CVector3D));
			*/
			k->m_frame = frame;
		}
		CA3MAnimationSet* set = nullptr;
		if (m_animation_list.size() <= anim_num) {
			set = new CA3MAnimationSet();
			set->m_start = 0;
			set->m_fps = (float)FbxTime::GetFrameRate(time_mode);
			set->m_end = set->m_end_base = frame;
			strcpy_s(set->m_name, sizeof(set->m_name), info->m_AnimationStack->GetName());
			m_animation_list.push_back(set);
		}
		else {
			set = m_animation_list[anim_num];
		}
		set->m_bones[n->m_bone_no] = aa;


		FbxTime::SetGlobalTimeMode(FbxTime::eFrames60);
	}


	for (int i = 0; i < pNode->GetChildCount(); i++) {
		DuplicateFBXAnimation(pNode->GetChild(i), info, anim_num);
	}
	return true;
}
CModelA3M::CModelA3M(const CModelFBX& m): m_bone_num(0), m_bone_matrix(nullptr), m_animation_size(0)
{
	strcpy_s(m_filePath,sizeof(m_filePath),m.m_filePath);
	int no=0;
	//m_vertex_type = A3M::eStaticMesh;
	for (auto it = m.m_materialList.begin(); it != m.m_materialList.end(); it++) {
		CMaterial* mat = new CMaterial(**it);
		if (mat->mp_texture) {
			CTexture* tex = new CTexture();
			char str[PATH_SIZE];
			strcpy_s(str, PATH_SIZE, m_filePath);
			strcat_s(str, PATH_SIZE, mat->m_texture_name);
			if (tex->Load(str)) {
			}
			mat->mp_texture = tex;
		}
		if (mat->mp_normal_map) {
			CTexture* tex = new CTexture();
			char str[PATH_SIZE];
			strcpy_s(str, PATH_SIZE, m_filePath);
			strcat_s(str, PATH_SIZE, mat->m_normal_map_name);
			if (tex->Load(str)) {
			}
			mat->mp_normal_map = tex;
		}
		m_material_list.push_back(mat);

	}
	m_col_cut = new CA3MColl;
	m_col_cut->m_max=CVector3D(FLT_MIN, FLT_MIN, FLT_MIN);
	m_col_cut->m_min=CVector3D(FLT_MAX, FLT_MAX, FLT_MAX);
	mp_root = DuplicateFBXMesh(m.m_lScene->GetRootNode(),no,nullptr,nullptr);
	
	//CreateCollisionIndex(4, 4, 4);
	m_bone_num = no;
	m_send_matrix = new CMatrix[m_bone_num];
	m_bone_matrix = new CMatrix[m_bone_num];
	m_old_matrix = new CMatrix[m_bone_num];
	if (m.GetAnimationSize() > 0) {
		for (int i = 0; i < m.GetAnimationSize(); i++) {
			if (DuplicateFBXAnimation(m.m_lScene->GetRootNode(), m.m_animation_list[i], m_animation_size))
				m_animation_size++;
		}
		m_animation.push_back(&m_animation_list);
		m_animation.push_back(&m_animation_list);
		m_animation_size = (int)m_animation_list.size();
		m_enable_animation = true;
	} else {
		m_enable_animation = false;
	}
	CalcBoneMatrix(mp_root);

}

bool CModelA3M::AddAnimationFBX(const char* filepath)
{
	FbxManager* lSdkManager = FbxManager::Create();
	FbxIOSettings* ios = FbxIOSettings::Create(lSdkManager, IOSROOT);
	lSdkManager->SetIOSettings(ios);


	FbxImporter* lImporter = FbxImporter::Create(lSdkManager, "");



	bool lImportStatus = lImporter->Initialize(filepath, -1, lSdkManager->GetIOSettings());
	if (!lImportStatus) {
		printf("Call to FbxImporter::Initialize() failed.\n");
		printf("Error returned: %s\n\n", lImporter->GetStatus().GetErrorString());
		return false;
	}

	int lFileMajor, lFileMinor, lFileRevision;


	lImporter->GetFileVersion(lFileMajor, lFileMinor, lFileRevision);
	printf("%d.%d.%d\n", lFileMajor, lFileMinor, lFileRevision);



	FbxScene* lScene = FbxScene::Create(lSdkManager, "myScene");

	int anim_size = lImporter->GetAnimStackCount();

	lImporter->Import(lScene);


	for (int i = 0; i < anim_size; i++) {
		if (DuplicateFBXAnimation(lScene->GetRootNode(),
			&CModelFBX::AnimationInfo(lSdkManager, lScene,
				lScene->GetSrcObject<FbxAnimStack>(i), lImporter->GetTakeInfo(i)), m_animation_size))
			m_animation_size++;
	}

	lImporter->Destroy();
	ios->Destroy();

	if (lScene) {
		lScene->Destroy(); lScene = NULL;
	}
	if (lSdkManager) { lSdkManager->Destroy(); lSdkManager = NULL; }

	if (m_animation.size()==0 && m_animation_size > 0) {
		m_animation.push_back(&m_animation_list);
		m_animation.push_back(&m_animation_list);
		m_enable_animation = true;
	}

	return true;

}

bool CModelA3M::DuplicateA3MAnimation(CA3MAnimationSet* set, CModelA3M* model)
{

	CA3MAnimationSet* as = new CA3MAnimationSet();
	m_animation_list.push_back(as);

	strcpy_s(as->m_name, sizeof(as->m_name), set->m_name);
	as->m_start = set->m_start;
	as->m_end = set->m_end;
	as->m_end_base = set->m_end_base;
	as->m_fps = set->m_fps;
	model->UpdateMatrix();

	int start = 0, end;
	for (auto it = set->m_bones.begin(); it != set->m_bones.end(); it++) {
		CA3MAnimationAction* a = it->second;
		if (!a) continue;
		CA3MAnimationAction* aaa = new CA3MAnimationAction();

		CA3MNode* node = CA3MNode::FindNode(model->GetNode(it->first)->m_name, mp_root);
		CA3MNode* anode = model->GetNode(it->first);

		if (!node) continue;
		aaa->m_size = a->m_size;
		aaa->m_keys = new CA3MAnimationKey[aaa->m_size];
		memcpy(aaa->m_keys, a->m_keys, sizeof(CA3MAnimationKey) * aaa->m_size);
		as->m_bones[node->m_bone_no] = aaa;
	}
	return true;
}
bool CModelA3M::DuplicateXAnimation(CXAnimationSet* set, float fps)
{

	CA3MAnimationSet* as = new CA3MAnimationSet();
	m_animation_list.push_back(as);

	strcpy_s(as->m_name, sizeof(as->m_name), set->GetName());
	int start = 0, end;
	for (auto it = set->animations.begin(); it != set->animations.end(); it++) {
		CXLAnimation* xaa = (*it);

		CA3MAnimationAction* aaa = new CA3MAnimationAction();

		CA3MNode* node = CA3MNode::FindNode(xaa->boneName, mp_root);

		if (!node) continue;
		CXAnimationKey* xkey = xaa->animationkey;
		start = xkey->keys[0].time;
		end = xkey->keys[xkey->nKeys - 1].time;
		as->m_start = 0;
		as->m_end = end - start;
		as->m_fps = fps;
		aaa->m_size = as->m_end + 1;
		aaa->m_keys = new CA3MAnimationKey[aaa->m_size];
		as->m_bones[node->m_bone_no] = aaa;
		CXTimedFloatKeys* kb = nullptr;
		CA3MAnimationKey* ak = aaa->m_keys;
		int frame = 0;
		for (int k = 0; k < xkey->nKeys; k++) {
			CXTimedFloatKeys* key = &xkey->keys[k];
			ak->m_matrix = key->matrix;
			ak->m_frame = frame;
			ak++;
			frame++;
		}
	}
	return true;
}
CA3MNode* CModelA3M::DuplicateXMesh(const CModelX* model, CFrameBone* pNode, int& no, CA3MNode* parent, CA3MNode* prev)
{
	CA3MNode* node = new CA3MNode();	
	m_node_list.push_back(node);

	if (pNode->GetType() == E_FRAME_TYPE::eMesh) {
		CA3MMesh* mesh = new CA3MMesh(&m_enable_animation);
		m_mesh_list.push_back(node);
		node->mp_mesh = mesh;
		node->m_node_type = A3M::eMesh;
		CXMesh* m = pNode->pMesh;
		int material_size = (int)m->polyList.size();
		int use_skin = 0;
		if (material_size > 0 && m->polyList[0]->bWeight) {
			use_skin = 1;
		}
		int array_size = 0;
		//int poly_cnt;
		array_size += sizeof(int);
		//int use_skin;
		array_size += sizeof(int);
		//int bone_offset;
		array_size += sizeof(unsigned int);
		array_size += sizeof(SA3MBoneOffsetCHK)* m_bone_num;
		for (auto it=m->polyList.begin(); it != m->polyList.end(); it++) {
			//m_vertex_cnt+m_material+m_vertex_cnt*MY_VERTEX;
			array_size += sizeof(unsigned long) + sizeof(unsigned long) +
				static_cast<size_t>((*it)->faceCnt) * 3 * sizeof(MY_VERTEX);
		}


		char* buf = (char*)malloc(array_size);
		int idx = 0;
		memcpy(&buf[idx], &material_size, sizeof(int));
		idx += sizeof(int);

		memcpy(&buf[idx], &use_skin, sizeof(use_skin));
		idx += sizeof(int);

		memcpy(&buf[idx], &m_bone_num, sizeof(m_bone_num));
		idx += sizeof(unsigned int);
		for (unsigned int i = 0; i < m_bone_num; i++)
		{
			CFrameBone* pFrame = model->GetFrameBone(i);
			SA3MBoneOffsetCHK of;
			of.no = pFrame->GetNo();
			of.offset = pFrame->GetMatrixOffset();
			memcpy(&buf[idx], &of, sizeof(SA3MBoneOffsetCHK));
			idx += sizeof(SA3MBoneOffsetCHK);
		}
		for (auto it = m->polyList.begin(); it != m->polyList.end(); it++) {

			CXPoly* p = *it;
			MY_VERTEX* v = nullptr;
			if (p->buffer) {
				glBindBuffer(GL_ARRAY_BUFFER, p->buffer);
				v = (MY_VERTEX*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
			}
			else {
				v = p->m_pVertex;
			}



			unsigned int	vertex_cnt = p->faceCnt * 3;
			unsigned int	material = p->material;


			memcpy(&buf[idx], &vertex_cnt, sizeof(vertex_cnt)); idx += sizeof(vertex_cnt);
			memcpy(&buf[idx], &material, sizeof(material)); idx += sizeof(material);
			//unsigned short vertex_size = sizeof(SVector3D) + sizeof(SVector3D) + sizeof(SVector2D);

			for (unsigned int j = 0; j < vertex_cnt; j++, v++) {
				memcpy(&buf[idx], v, sizeof(MY_VERTEX)); idx += sizeof(MY_VERTEX);
				

				m_col_cut->m_min.x = min(m_col_cut->m_min.x, v->vPos.x);
				m_col_cut->m_max.x = max(m_col_cut->m_max.x, v->vPos.x);
				m_col_cut->m_max.z = max(m_col_cut->m_max.z, v->vPos.z);
				m_col_cut->m_min.z = min(m_col_cut->m_min.z, v->vPos.z);
				m_col_cut->m_max.y = max(m_col_cut->m_max.y, v->vPos.y);
				m_col_cut->m_min.y = min(m_col_cut->m_min.y, v->vPos.y);



			}
			if (p->buffer) {
				glUnmapBuffer(GL_ARRAY_BUFFER);
				glBindBuffer(GL_ARRAY_BUFFER, 0);
			}

		}
		mesh->Load(buf);
		free(buf);

	}
	

	node->m_bone_no = node->m_no = no++;
	strcpy_s(node->m_name, sizeof(node->m_name), pNode->GetName());
	node->m_offset = pNode->matrixOffset;
	node->m_local_matrix = pNode->matrix;

	if (node->mp_parent = parent) {
		if (!parent->mp_child) parent->mp_child = node;
		node->m_matrix = parent->m_matrix * node->m_local_matrix;
	}
	else {
		node->m_matrix = node->m_local_matrix;
	}
	if (node->mp_prev = prev) prev->mp_next = node;


	if (pNode->GetChild()) {
		DuplicateXMesh(model,(CFrameBone*)pNode->GetChild(), no, node, nullptr);
	}
	CA3MNode* child = node;
	if (pNode->GetNext()) {
		child = DuplicateXMesh(model, (CFrameBone*)pNode->GetNext(), no, node->mp_parent, child);
	}
	

	return node;


}
CModelA3M::CModelA3M(const CModelX& m)
{
	int no = 0;

	m_bone_num = m.boneNum;
	for (auto it = m.pMaterialList->begin(); it != m.pMaterialList->end(); it++)
		m_material_list.push_back(*it);
	m_col_cut = new CA3MColl;
	m_col_cut->m_max = CVector3D(FLT_MIN, FLT_MIN, FLT_MIN);
	m_col_cut->m_min = CVector3D(FLT_MAX, FLT_MAX, FLT_MAX);
	mp_root = DuplicateXMesh(&m,m.pFrameRoot, no, nullptr, nullptr);
	//CreateCollisionIndex(4, 4, 4);
	m_bone_num = no;
	m_send_matrix = new CMatrix[m_bone_num];
	m_bone_matrix = new CMatrix[m_bone_num];
	m_old_matrix = new CMatrix[m_bone_num];


	if (m.GetAnimationSize() > 0) {
		CXAnimation* a = &m.animation[0];
		for (int i = 0; i < m.GetAnimationSize(); i++) {
			if(DuplicateXAnimation((*a->pAnimation)[i],a->FPS))
				m_animation_size++;
		}
		m_animation.push_back(&m_animation_list);
		m_animation.push_back(&m_animation_list);
		m_animation_size = (int)m_animation_list.size();
		m_enable_animation = true;
	} else {
		m_enable_animation = false;
	}
	CalcBoneMatrix(mp_root);
}

bool CModelA3M::AddAnimationX(const char* filepath)
{
	CXModelLoader loader(filepath);

	for (auto it = loader.animationSet.begin(); it != loader.animationSet.end(); it++) {
		if (DuplicateXAnimation(*it, loader.fps))  m_animation_size++;
	}

	if (m_animation.size()==0 && m_animation_size > 0) {
		m_animation.push_back(&m_animation_list);
		m_animation.push_back(&m_animation_list);
		m_enable_animation = true;
	}

	return true;
}

bool CModelA3M::AddAnimationA3M(const char* filepath)
{
	CModelA3M m;
	m.Load(filepath,0,0,0);
	for each (auto & a in m.m_animation_list)
	{
		DuplicateA3MAnimation(a, &m);
	}
	m_animation_size = (int)m_animation_list.size();
	return true;

}

void CModelA3M::ReadMatrix(CMatrix* mat, pugi::xml_node& pNode) {
	const char* c = pNode.child_value();
	c = strchr(c, '\n') + 1;
	for (int i = 0; i < 4; i++) {
		sscanf_s(c, "%f,%f,%f,%f,\n", &mat->m[i][0], &mat->m[i][1], &mat->m[i][2], &mat->m[i][3]);
		c = strchr(c, '\n') + 1;
	}

}

void CModelA3M::ReadMatrix(CA3MNode* node, pugi::xml_node pNode) {
	sscanf_s(pNode.child("Translate").child_value(), "%f,%f,%f", &node->m_pos.x, &node->m_pos.y, &node->m_pos.z);
	CVector3D r;
	CMatrix mat_rot;
	sscanf_s(pNode.child("Angle").child_value(), "%f,%f,%f", &r.x, &r.y, &r.z);
	sscanf_s(pNode.child("Scale").child_value(), "%f,%f,%f", &node->m_scale.x, &node->m_scale.y, &node->m_scale.z);
	const char* c = pNode.child("EulerOrder").child_value();
	if(c) memcpy(node->m_euler_order, c, 3);
	/*
	for (int i = 0; i < 3; i++,c++) {
		switch (*c) {
		case 'X':
			mat_rot *= CMatrix::MRotationX(r.x);
			break;
		case 'Y':
			mat_rot *= CMatrix::MRotationY(r.y);
			break;
		case 'Z':
			mat_rot *= CMatrix::MRotationZ(r.z);
			break;

		}

	}
	*/
	mat_rot = CMatrix::MRotation(r);
	node->m_rot.SetRotateMatrix(mat_rot);
	node->m_local_matrix = node->m_matrix = CMatrix::MTranselate(node->m_pos)* mat_rot * CMatrix::MScale(node->m_scale);
}
void CModelA3M::LoadToyMesh(pugi::xml_node& pNode, CA3MNode* node, CA3MColl* col_cut)
{
	const char* visible = pNode.child("Visible").child_value();
	if (visible && strcmp(visible, "False") == 0) {
		node->m_visibility = false;
		return;
	}

	CA3MMesh* mesh = new CA3MMesh(&m_enable_animation);
	node->mp_mesh = mesh;
	node->m_node_type = A3M::eMesh;
	m_mesh_list.push_back(node);
	std::vector<int> bone_no;
	/*
	if (pugi::xml_node boneinfo_node = pNode.child("BindPose")) {
		auto bones = boneinfo_node.child("Objects").children();
		auto postures = boneinfo_node.child("Postures").children();
		auto n = bones.begin();
		auto p = postures.begin();
		for (auto n = bones.begin(); n != bones.end(); n++, p++) {
			const char* bone_name = strrchr(n->child_value(), '\\') + 1;
			CA3MNode* bone_node = GetNode(bone_name);
			if (!bone_node) {
				MessageBox(GL::hWnd, "エラー", "ボーンが見つかりません", MB_OK);
			}
			CVector3D pos;
			CQuaternion rot;
			CVector3D scale;
			CMatrix offset;
			ReadMatrix(&pos, &rot, &scale, &offset, *p);
			bone_node->m_offset = offset.GetInverse();
			if (bone_node->mp_parent) bone_node->m_offset = bone_node->mp_parent->m_offset * bone_node->m_offset;
		}

	}
	*/
	if (pugi::xml_node boneinfo_node = pNode.child("BoneInfo")) {
		auto bones = boneinfo_node.child("Bones").children();
		auto bind = boneinfo_node.child("BoneBindTransforms").children();
		auto n = bones.begin();
		auto b = bind.begin();
		for (auto n = bones.begin(); n != bones.end(); n++, b++) {
			const char* bone_name = strrchr(n->child_value(), '\\')+1;
			CA3MNode* bone_node = GetNode(bone_name);
			if (!bone_node) {
				MessageBox(GL::hWnd, "エラー", "ボーンが見つかりません", MB_OK);
			}
			CMatrix offset;
			ReadMatrix(&offset, *b);
			offset = offset.GetInverse();
			//bone_node->m_offset = bone_node->m_offset.GetInverse();
			mesh->m_offset.push_back(CA3MMesh::SBoneOffset(bone_node->m_bone_no, offset));
			/*CMatrix &m = bone_node->m_offset;
			CMatrix o;
			o.m00 = m.m22; o.m01 = m.m21; o.m02 = m.m20; o.m03 =-m.m03;
			o.m10 = m.m12; o.m11 = m.m11; o.m12 = m.m10; o.m13 = -m.m13;
			o.m20 = m.m02; o.m21 = m.m01; o.m22 = m.m00; o.m23 = -m.m23;
			bone_node->m_offset = o;*/
			//if (bone_node->mp_parent) bone_node->m_offset = bone_node->mp_parent->m_offset * bone_node->m_offset;
			bone_no.push_back(bone_node->m_bone_no);
		}
	}
	CVector3D* pVertex = nullptr;
	CVector3D* pNormal = nullptr;
	int vertex_cnt=0;
	int bone_n = (int)bone_no.size();
	if (pugi::xml_node vertices_node = pNode.child("Vertices")) {
		pugi::xml_attribute count_attr = vertices_node.attribute("Count");
		vertex_cnt = atoi(count_attr.value());
		pVertex = new CVector3D[vertex_cnt];
		CVector3D* v = pVertex;
		const char* c = vertices_node.child_value();
		c = strchr(c, '\n') + 1;
		for (int i = 0; i < vertex_cnt; i++,v++) {
			sscanf_s(c, "%f,%f,%f", &v->x, &v->y, &v->z);
			col_cut->m_min.x = min(col_cut->m_min.x, v->x);
			col_cut->m_max.x = max(col_cut->m_max.x, v->x);
			col_cut->m_max.z = max(col_cut->m_max.z, v->z);
			col_cut->m_min.z = min(col_cut->m_min.z, v->z);
			col_cut->m_max.y = max(col_cut->m_max.y, v->y);
			col_cut->m_min.y = min(col_cut->m_min.y, v->y);

			c = strchr(c, '\n')+1;
		}	
	}
	CVector4D* pWeight = nullptr;
	CVector4D* pIndex = nullptr;

	if (pugi::xml_node boneweight_node = pNode.child("BoneWeights")) {
		int vertex_cnt = atoi(boneweight_node.attribute("Count").value());
		const char* c = boneweight_node.child_value();
		pWeight = new CVector4D[vertex_cnt];
		pIndex = new CVector4D[vertex_cnt];
		CVector4D* w = pWeight;
		CVector4D* idx = pIndex;
		c = strchr(c, '\n') + 1;
		for (int i = 0; i < vertex_cnt; i++,w++, idx++) {
			sscanf_s(c, "%f,%f,%f,%f", &idx->x, &idx->y, &idx->z, &idx->w);
			for (int k = 0; k < 4; k++) {
				if (idx->v[k] >= bone_n) {
					idx->v[k] = 0;
					continue;
				}
				idx->v[k] = (float)bone_no[(int)idx->v[k]];
			}
			c = strchr(c, '\n')+1;
			sscanf_s(c, "%f,%f,%f,%f", &w->x, &w->y, &w->z, &w->w);
			c = strchr(c, '\n')+1;
		}
		mesh->m_use_skin = 1;
	} else {
		mesh->m_use_skin = 0;
	}
	std::map<int, int> poly_cnt;
//	std::map<int, int> poly_cnt2;
	std::map<int, CA3MPoly*> poly_list;
	std::vector<int>* verttex_link = new std::vector<int>[vertex_cnt];
	int smoos = 0;
	if (pugi::xml_node face_node = pNode.child("Faces")) {
		int face_cnt = atoi(face_node.attribute("Count").value());
		bool use_uv = strcmp(face_node.attribute("UV").value(),"True")==0;
		const char* c = face_node.child_value();
		c = strchr(c, '\n') + 1;
		CVector3D*n =  pNormal = new CVector3D[face_cnt];
		for (int i = 0; i < face_cnt; i++,n++) {
			int poly = 0;
			sscanf_s(c, "%d", &poly);
			c = strchr(c, '\n') + 1;	//面
			int idx[64] = { 0, };
			for (int k = 0; k < poly; k++) {
				sscanf_s(c, "%d", &idx[k]);
				verttex_link[idx[k]].push_back(i);
				c = strchr(c, ',')+1;
			}
			c = strchr(c, '\n') + 1;	//Index
			*n = CVector3D::Cross(pVertex[idx[1]] - pVertex[idx[0]], pVertex[idx[2]] - pVertex[idx[0]]).GetNormalize();
			if(use_uv) c = strchr(c, '\n')+1;	//UV
			c = strchr(c, '\n')+1;	//255謎データ飛ばし
			sscanf_s(c, "%d", &smoos); ////スムース、フラットシェーディング
			c = strchr(c, '\n')+1;
			int material=-1;
			sscanf_s(c, "%d", &material);
			c = strchr(c, '\n')+1;	//マテリアル
			/*if (material == -1) {
				MessageBox(GL::hWnd, "エラー", "マテリアルエラー", MB_OK);
			}*/
			poly_cnt[material] += poly - 2;
		}
		for (auto it = poly_cnt.begin(); it != poly_cnt.end(); it++) {
			CA3MPoly* p = new CA3MPoly(&mesh->m_use_skin);
			p->mp_vertex = new SKinVertex[it->second*3];
			p->m_material = it->first;
			mesh->m_poly_list.push_back(p);
			poly_list[it->first] = p;
		}
		mesh->m_poly_cnt = (int)mesh->m_poly_list.size();
		c = face_node.child_value();
		c = strchr(c, '\n')+1;
		for (int i = 0; i < face_cnt; i++) {
			int poly = 0;
			sscanf_s(c, "%d", &poly);
			c = strchr(c, '\n')+1;
			//if (poly >= 64) {
			//	MessageBox(GL::hWnd, "エラー", "面エラー", MB_OK);
			//}
			int idx[64] = { 0, };
			float uv[64][2] = { 0.0f, };
			for (int k = 0; k < poly; k++) {
				sscanf_s(c, "%d", &idx[k]);

				/*if (idx[k]>vertex_cnt) {
					MessageBox(GL::hWnd, "エラー", "UVエラー", MB_OK);

				}*/
				c = strchr(c, ',')+1;
			}
			c = strchr(c, '\n')+1;
			if (use_uv) {
				for (int k = 0; k < poly * 2; k++) {
					float* p_uv = &uv[k / 2][k % 2];
					sscanf_s(c, "%f", p_uv);
					c = strchr(c, ',')+1;
				}
				c = strchr(c, '\n') + 1;
			}
			c = strchr(c, '\n')+1;	//255謎データ飛ばし
			c = strchr(c, '\n')+1;	//スムース、フラットシェーディング
			int material;
			sscanf_s(c, "%d", &material);
			c = strchr(c, '\n')+1;
			CA3MPoly*p = poly_list[material];
			SKinVertex* v = &p->mp_vertex[p->m_vertex_cnt];
			int l = 0;
			int last=0;
			for (int h = 0; h < poly - 2; h++) {
				int k = 0;
				while (1) {
					int r = l % poly;
					v[k].v = pVertex[idx[r]];
					if (smoos==0) {	//0でスムースシェーディング
						auto link = &verttex_link[idx[r]];
						CVector3D normal(0, 0, 0);
						for (auto it = link->begin(); it != link->end(); it++) {
							normal += pNormal[*it];
						}
						v[k].n = normal.GetNormalize();
					}
					else {
						v[k].n = pNormal[i];
					}
					if (use_uv)
						v[k].t = CVector2D(uv[r][0], uv[r][1]);
					if (pWeight && pIndex) {
						float ww = 0;
						for (int t = 0; t < 4; t++) {
							v[k].bBoneIndex[t] = pIndex[idx[r]].v[t];
							v[k].bBoneWeight[t] = pWeight[idx[r]].v[t];
							ww += v[k].bBoneWeight[t];
						}
						//if (ww > 1.1f || ww<0.9f) {
						//	MessageBox(GL::hWnd, "エラー", "ウエイトエラー", MB_OK);
						//}
					}
					else {
						memset(v[k].bBoneIndex, 0, sizeof(v[k].bBoneIndex));
						memset(v[k].bBoneWeight, 0, sizeof(v[k].bBoneWeight));
					}
					p->m_vertex_cnt++;
					if (++k == 3) break;
					l += (l / poly) + 1;
				}
				//後でスムース対応

				//
				v += 3;
				/*poly_cnt2[material] +=  3;
				if (p->m_vertex_cnt > poly_cnt[material]*3) {
					MessageBox(GL::hWnd, "エラー", "ポリゴンオーバーエラー", MB_OK);

				}*/
			}
		}
	}

	if (verttex_link) delete[] verttex_link;
	if(pWeight) delete[] pWeight;
	if(pIndex) delete[] pIndex;
	if(pVertex) delete[] pVertex;
	if(pNormal) delete[] pNormal;
}

void CModelA3M::LeapAnimationKey(float* key, int start, int end,float vs,float ve,int max,int step) {
	int size = end - start;
	if (end > max) {
		int s = max - start;
		float t = (float)(s / size);
		size = s;
		ve = vs * (1 - t) + ve * t;
		if (size <= 0) return;
	}
	float* k = &key[start * step];
	for (int i = 0; i <= size; i++, k+= step) {
		float t = (float)i / (size);
		*k = vs*(1-t) + ve*t;
	}
}
void CModelA3M::LeapAnimationKey(CA3MAnimationKey* key, int start, int end) {
	int size = end - start;
	CA3MAnimationKey* s = &key[start];
	CA3MAnimationKey* e = &key[end];
	CA3MAnimationKey* k = &key[start+1];
	
	for (int i = 1; i < size; i++,k++) {
		float t = (float)i / (size);
		k->m_position = CVector3D::Leap(s->m_position, e->m_position, t);
		k->m_rotation = CQuaternion::Slerp(s->m_rotation, e->m_rotation, t);
		k->m_scale = CVector3D::Leap(s->m_scale, e->m_scale, t);
	}
}
void CModelA3M::LoadToyAnimationSet(pugi::xml_node& pNode,CA3MAnimationSet *set)
{

	strcpy_s(set->m_name, sizeof(set->m_name), pNode.child("Name").child_value());
	set->m_fps = (float)atof(pNode.child("Fps").child_value());
	int start = atoi(pNode.child("FirstFrame").child_value());
	int end = atoi(pNode.child("LastFrame").child_value());
	int size = end - start+1;
	set->m_start = 0;
	set->m_end = set->m_end_base = size-1;
	auto bone_list = pNode.child("ObjectItems").children("Object");
	static FILE* fp;
	fopen_s(&fp, "log.txt", "w");

	for (auto it = bone_list.begin(); it != bone_list.end(); it++) {

		const char* name = strrchr(it->child("Name").child_value(),'\\')+1;
		CA3MNode* node = GetNode(name);
		CA3MAnimationAction* aa = new CA3MAnimationAction();
		aa->m_size = size;
		CA3MAnimationKey* key = new CA3MAnimationKey[size];
		CA3MAnimationKey init_key(node->m_pos,node->m_rot,node->m_scale);
		for (int i = 0; i < size; i++) {
			memcpy(&key[i], &init_key, sizeof(init_key));
		}
		auto item_list = it->child("Items").children("Item");
		std::map<std::string, std::map<int, float> > key_frame;

		for (auto item = item_list.begin(); item != item_list.end(); item++) {
			const char* c = item->child_value();
			c = strchr(c, '\n') + 1;
			int cnt;
			sscanf_s(c, "%d", &cnt);
			c = strchr(c, '\n') + 1;
			const char* type = item->attribute("Type").value();
			
			auto& key_param = key_frame[type];

			for (int i = 0; i < cnt; i++) {
				float key = 0;
				float value = 0;
				sscanf_s(c, "%f,%f", &key,&value);
				c = strchr(c, '\n') + 1;	//キーフレーム 値

				c = strchr(c, '\n') + 1;	//謎データ飛ばし
				c = strchr(c, '\n') + 1;
				c = strchr(c, '\n') + 1;
				int idx = (int)key - (int)start;
				key_param[idx] = value;
				if (idx >= size || idx<0) {
					//MessageBox(GL::hWnd, "エラー", "アニメーションエラー", MB_OK);
					continue;
				}
			}

		}
		
		struct {
			const char* key;
			int offset;
		} target[] = {
			{"TranslateX",1},
			{"TranslateY",2},
			{"TranslateZ",3},
			{"RotateX",4},
			{"RotateY",5},
			{"RotateZ",6},
			{"ScaleX",8},
			{"ScaleY",9},
			{"ScaleZ",10},
		};
		std::map<std::string, int> param;
		for (int i = 0; i < 9; i++ ) {
			param[target[i].key] = target[i].offset;
		}
		for (auto it = key_frame.begin(); it != key_frame.end(); it++) {
			int idx = 0;
			const int step = sizeof(CA3MAnimationKey) / sizeof(float);
			auto k = it->second.begin();
			float* v = ((float*)key) + param[it->first];
			for (; k != it->second.end(); k++) {
				auto next = k;
				next++;
				idx = k->first;
				if (next == it->second.end()) break;
				//if (k->first >= size || next->first >= size) {
				//	MessageBox(GL::hWnd, "エラー", "アニメーション", MB_OK);
				//}
				LeapAnimationKey(v, k->first, next->first,k->second,next->second, size-1,step);

			}
			float e = k->second;
			v = &v[k->first * step];
			for (int i = idx; i < size; i++, v += step) {
				*v = e;
			}
		}
		
		/*
		int idx = 0;
		for (auto k = key_frame.begin(); k!= key_frame.end(); k++) {
			auto next = k;
			next++;
			idx = k->second;
			if (next == key_frame.end()) break;
			LeapAnimationKey(key, k->second, next->second);
		}

		for (int i = idx+1; i < size; i++) {
			memcpy(&key[i], &key[idx], sizeof(key[i]));
		}
		*/
		CA3MAnimationKey* k = key;
		aa->m_keys = key;
		fprintf(fp, "%s\n", node->m_name);
		for (int i = 0; i < size; i++,k++) {
			fprintf(fp, "pos:%f %f %f\n", k->m_position.x, k->m_position.y, k->m_position.z);
			fprintf(fp, "rot:%f %f %f\n", k->m_rotation.x, k->m_rotation.y, k->m_rotation.z);
			fprintf(fp, "scale:%f %f %f\n", k->m_scale.x, k->m_scale.y, k->m_scale.z);

			k->m_frame = i;
			//k->m_matrix = init_key.m_matrix;
			//非クォータニオン
			CMatrix mat_rot;
			/*
			const char* c = node->m_euler_order;
			for (int i = 0; i < 3; i++, c++) {
				switch (*c) {
				case 'X':
					mat_rot *= CMatrix::MRotationX(DtoR(k->m_rotation.x));
					break;
				case 'Y':
					mat_rot *= CMatrix::MRotationY(DtoR(k->m_rotation.y));
					break;
				case 'Z':
					mat_rot *= CMatrix::MRotationZ(DtoR(k->m_rotation.z));
					break;

				}

			}
			*/
			mat_rot = CMatrix::MRotationZ(DtoR(k->m_rotation.z)) * CMatrix::MRotationY(DtoR(k->m_rotation.y)) * CMatrix::MRotationX(DtoR(k->m_rotation.x));
			k->m_matrix = CMatrix::MTranselate(k->m_position) * mat_rot * CMatrix::MScale(k->m_scale);

		}

		set->m_bones[node->m_bone_no] = aa;
	}
	fclose(fp);
}
void CModelA3M::LoadToyAnimation(pugi::xml_node& pNode)
{
	auto anim_list = pNode.children("Anim");
	for (auto it = anim_list.begin(); it != anim_list.end(); it++) {
		CA3MAnimationSet* set = new CA3MAnimationSet();
		m_animation_list.push_back(set);
		LoadToyAnimationSet(*it, set);
	}
	m_animation_size = (int)m_animation_list.size();

	if (m_animation_size > 0) {
		m_animation.push_back(&m_animation_list);
		m_animation.push_back(&m_animation_list);
		m_enable_animation = true;
	} else {
		m_enable_animation = false;
	}

}
void CModelA3M::LoadToyMaterial(pugi::xml_node& pNode)
{

	pugi::xml_node color_node = pNode.child("Material");
	if (!color_node) return;
	CMaterial* mat = new CMaterial();
	mat->m_shader_name = "SkinMesh";

	pugi::xml_attribute color_attr = color_node.attribute("Color");
	CVector3D color;
	mat->m_alpha = 1.0f;
	sscanf_s(color_attr.value(), "%f,%f,%f,%f", &color.r, &color.g, &color.b, &mat->m_alpha);
	pugi::xml_attribute diffuse_attr = color_node.attribute("Diffuse");
	float diffuse = (float)atof(diffuse_attr.value());
	mat->m_diffuse = CVector4D(color * diffuse, 1);
	pugi::xml_attribute ambient_attr = color_node.attribute("Ambient");
	float ambient = (float)atof(ambient_attr.value());
	mat->m_ambient = CVector4D(color * ambient, 1);
	pugi::xml_attribute emissive_attr = color_node.attribute("Emissive");
	float emissive = (float)atof(emissive_attr.value());
	mat->m_emissive = color * emissive;
	pugi::xml_attribute specular_attr = color_node.attribute("Specular");
	float specular = (float)atof(specular_attr.value());
	mat->m_specular = CVector4D(color * specular, 1);
	pugi::xml_attribute power_attr = color_node.attribute("Power");
	mat->m_shininess = (float)atof(power_attr.value());
	
	if (pugi::xml_node tex_attr = pNode.child("Textures")) {
		strcpy_s(mat->m_texture_name, sizeof(mat->m_texture_name),tex_attr.first_child().attribute("Path").value());
		char str[PATH_SIZE];
		strcpy_s(str, PATH_SIZE, m_filePath);
		strcat_s(str, PATH_SIZE, mat->m_texture_name);
		CTexture* tex = new CTexture();
		if (tex->Load(str)) {
			mat->mp_texture = tex;
		} else {
			delete tex;
		}

	}

	
	pugi::xml_node name_node = pNode.child("Name");
	strcpy_s(mat->m_name, sizeof(mat->m_name), name_node.child_value());
	m_material_list.push_back(mat);
}

CA3MNode* CModelA3M::LoadToyNode(pugi::xml_node& pNode, int& no,int &bone_no, CA3MNode* parent, CA3MNode* prev)
{
	CA3MNode* node = new CA3MNode();
	m_node_list.push_back(node);
	node->m_no = no++;
	node->m_bone_no = node->m_no;
	//if (strcmp(pNode.name(), "Joint") == 0) node->m_bone_no = bone_no++;
	pugi::xml_node name_node = pNode.child("Name");
	strcpy_s(node->m_name, sizeof(node->m_name), name_node.child_value());
	//node->m_offset = pNode.matrixOffset;
	ReadMatrix(node,pNode);
	if (strcmp(pNode.name(), "Mesh") == 0) {
		m_toy_mesh_list.push_back(SMeshNodeSet(pNode,node));
	}
	if (strcmp(pNode.name(), "Model") == 0) {
		if (pNode.child("Objects")) {
			LoadToyNode(pNode.child("Objects").first_child(), no, bone_no, node, nullptr);
		}
	} else {
		if (node->mp_parent = parent) {
			if (!parent->mp_child) parent->mp_child = node;
			node->m_matrix = parent->m_matrix * node->m_local_matrix;
		}
		else {
			node->m_matrix = node->m_local_matrix;
		}
		if (node->mp_prev = prev) prev->mp_next = node;
		if (pNode.child("Objects")) {
			LoadToyNode(pNode.child("Objects").first_child(), no, bone_no, node, nullptr);
		}
		CA3MNode* child = node;
		if (pNode.next_sibling()) {
			child = LoadToyNode(pNode.next_sibling(), no, bone_no, node->mp_parent, child);
		}
	}
	
	return node;
}

bool CModelA3M::LoadToyModel(const char* filename)
{
	MakePath(filename);
	pugi::xml_document doc;

	if (!doc.load_file(filename)) return false;
	m_col_cut = new CA3MColl;
	m_col_cut->m_max = CVector3D(FLT_MIN, FLT_MIN, FLT_MIN);
	m_col_cut->m_min = CVector3D(FLT_MAX, FLT_MAX, FLT_MAX);


	pugi::xml_node root = doc.root().first_child().child("Model");
	auto material_list = root.child("Attribs").children("Attrib");
	if (!root.child("Attribs")) {
		MessageBox(GL::hWnd, "マテリアルがありません", "エラー", MB_OK);
		return false;
	}
	for (auto it = material_list.begin(); it != material_list.end(); it++) {
		LoadToyMaterial(*it);
	}
	int no = 0;
	int bone_no = 0;
	mp_root = LoadToyNode(root,no, bone_no,nullptr, nullptr);
	for (auto it = m_toy_mesh_list.begin(); it != m_toy_mesh_list.end(); it++) {
		LoadToyMesh(it->mp_toy_node, it->mp_node,m_col_cut);
	}
	//CreateCollisionIndex(4,4,4);
	LoadToyAnimation(root.child("Anims"));
	//m_bone_num = bone_no;
	m_bone_num = no;
	m_send_matrix = new CMatrix[m_bone_num];
	m_bone_matrix = new CMatrix[m_bone_num];
	m_old_matrix = new CMatrix[m_bone_num];

	return true;
}

int CModelA3M::WriteNode(FILE* fp, CA3MNode* node)
{

	SA3MNodeCHK nc;
	nc.m_no = node->m_no;
	nc.m_bone_no = node->m_bone_no;
	nc.m_node_type = node->m_node_type;
	strcpy_s(nc.m_name, sizeof(nc.m_name), node->m_name);
	nc.m_local_matrix = node->m_local_matrix;
	nc.m_offset = node->m_offset;
	nc.m_pos = node->m_pos;
	nc.m_rot = node->m_rot;
	nc.m_scale = node->m_scale;
	nc.m_visibility = node->m_visibility;

	long node_offset = ftell(fp);


	fseek(fp, sizeof(SA3MNodeCHK), SEEK_CUR);
	if (node->m_node_type == A3M::eMesh) {
		CA3MMesh* m = node->mp_mesh;
		fwrite(&m->m_poly_cnt, sizeof(int), 1, fp);
		fwrite(&m->m_use_skin, sizeof(int), 1, fp);
		unsigned int size = (int)m->m_offset.size();
		fwrite(&size, sizeof(unsigned int), 1, fp);

		for (const auto& v : m->m_offset) {
			SA3MBoneOffsetCHK of;
			of.no = v.m_bone_no;
			of.offset = v.m_offset;
			fwrite(&of, sizeof(SA3MBoneOffsetCHK), 1, fp);
		}

		for (auto it = m->m_poly_list.begin(); it != m->m_poly_list.end(); it++) {
			CA3MPoly* p = *it;
			fwrite(&p->m_vertex_cnt, sizeof(p->m_vertex_cnt), 1, fp);
			fwrite(&p->m_material, sizeof(p->m_material), 1, fp);
			SKinVertex* v = nullptr;
			if (p->mp_vertex) {
				v = p->mp_vertex;
			}
			else {
				glBindBuffer(GL_ARRAY_BUFFER, p->m_buffer);
				v = (SKinVertex*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
			}
			for (int i = 0; i < p->m_vertex_cnt; i++)
				fwrite(&v[i], sizeof(MY_VERTEX), 1, fp);
			if (p->m_buffer) {
				glUnmapBuffer(GL_ARRAY_BUFFER);
				glBindBuffer(GL_ARRAY_BUFFER, 0);
			}
		}

	}
	nc.node_size = ftell(fp) - node_offset;
	if (node->mp_child) {
		nc.child_offset = ftell(fp);
		WriteNode(fp, node->mp_child);
	}
	else {
		nc.child_offset = 0;
	}
	if (node->mp_next) {
		nc.next_offset = ftell(fp);
		WriteNode(fp, node->mp_next);
	}
	else {
		nc.next_offset = 0;
	}
	long offset = ftell(fp);

	fseek(fp, node_offset, SEEK_SET);
	fwrite(&nc, sizeof(SA3MNodeCHK), 1, fp);



	fseek(fp, offset, SEEK_SET);
	return offset;
}

int CModelA3M::WriteAnimation(FILE* fp)
{

	fwrite(&m_animation_size, sizeof(m_animation_size), 1, fp);
	for (int i = 0; i < m_animation_size; i++) {
		CA3MAnimationSet* s = m_animation_list[i];
		fwrite(s, sizeof(s->m_name) + sizeof(float) + sizeof(int) * 2, 1, fp);
		unsigned int bone_size = 0;
		for (auto it = s->m_bones.begin(); it != s->m_bones.end(); it++) {
			if (it->second) {
				bone_size++;
			}

		}
		fwrite(&bone_size, sizeof(unsigned int), 1, fp);
		for (auto it = s->m_bones.begin(); it != s->m_bones.end(); it++) {
			if (it->second) {
				CA3MAnimationAction* aa = it->second;
				fwrite(&it->first, sizeof(int), 1, fp);
				fwrite(&aa->m_size, sizeof(int), 1, fp);
				fwrite(aa->m_keys, sizeof(CA3MAnimationKey) * aa->m_size, 1, fp);
			}
		}
	}
	return 0;
}

void CModelA3M::Write(const char* filePath)
{
	FILE* fp = nullptr;
	fopen_s(&fp, filePath, "wb");
	if (!fp) return;
	SA3MModelHeader header;
	header.fileType[0] = 'A';
	header.fileType[1] = 'M';

	header.version = A3MVERSION;
	header.materialCnt = (int)m_material_list.size();
	header.materialOffset = sizeof(SA3MModelHeader);
	header.bone_num = m_bone_num;
	fseek(fp, header.materialOffset, SEEK_SET);
	for (unsigned int i = 0; i < header.materialCnt; i++) {
		CMaterial* m = m_material_list[i];
		fwrite((const void*)m->m_name, sizeof(SA3MaterialCHK2), 1, fp);
	}

	header.rootOffset = header.materialOffset + sizeof(SA3MaterialCHK2) * header.materialCnt;
	fseek(fp, header.rootOffset, SEEK_SET);
	WriteNode(fp, mp_root);
	header.min = m_min;
	header.max = m_max;
	header.animationOffset = ftell(fp);
	fseek(fp, header.animationOffset, SEEK_SET);
	WriteAnimation(fp);

	fseek(fp, 0, SEEK_SET);
	fwrite(&header, sizeof(SA3MModelHeader), 1, fp);

	fclose(fp);
}

#endif
void CModelA3M::RemoveAnimation(int jam)
{
	if (jam >= m_animation_size) return;
	int i = 0;
	auto it = m_animation_list.begin();
	while (i < jam) {
		it++;
		i++;
	}
	delete (*it);
	m_animation_list.erase(it);
	m_animation_size = (int)m_animation_list.size();
	if (m_animation_size == 0) {
		m_animation.clear();
		m_enable_animation = false;
	}
}

void CModelA3M::Release()
{

	if (mp_root) {
		mp_root->Release();
		delete mp_root;
		mp_root = nullptr;
	}
	if (m_send_matrix) {
		delete[] m_send_matrix;
		m_send_matrix = nullptr;
	}
	if (m_bone_matrix) {
		delete[] m_bone_matrix;
		m_bone_matrix = nullptr;
	}
	if (m_old_matrix) {
		delete[] m_old_matrix;
		m_old_matrix = nullptr;
	}


	for (auto it = m_material_list.begin(); it != m_material_list.end(); it++)
	{
		(*it)->Release();
		delete *it;
	}
	while (m_animation_size > 0)
		RemoveAnimation(m_animation_size - 1);

	m_material_list.clear();
	m_animation.clear();
	m_enable_animation = false;
	if (m_col_cut) {
		delete m_col_cut;
		m_col_cut = nullptr;
	}
}
void CModelA3M::LoadTexture(CTexture** tex, const char* name)
{
	if (strlen(name) == 0) return;
	CTexture* t = new CTexture();

	char str[PATH_SIZE];
	strcpy_s(str, PATH_SIZE, m_filePath);
	strcat_s(str, PATH_SIZE, name);

	if (t->Load(str)) {
		if (*tex) {
			(*tex)->Release();
			delete (*tex);
		}
		*tex = t;
		return;
	}
	delete t;
}

bool CModelA3M::Load(const char* filePath, int cut_x, int cut_y, int cut_z)
{
	MakePath(filePath);
	FILE* fp;
	fopen_s(&fp, filePath, "rb");
	if (!fp) {
		printf("ファイルがありません[%s]\n",filePath);
		return false;
	}

	SA3MModelHeader header;
	fread(&header, sizeof(header), 1, fp);
	if (header.fileType[0] != 'A' || header.fileType[1] != 'M') {
		printf("ファイルの種類が違います[%s][%c%c]\n", filePath, header.fileType[0], header.fileType[1]);
		return false;
	}
	if (header.version != A3MVERSION && header.version != A3MVERSION_OLD) {
		char buf[256];
		sprintf_s(buf,256,"ファイルのバージョンが違います[%s][%x %x]\n", filePath, header.version, A3MVERSION);
		MessageBox(GL::hWnd,  buf, "エラー", MB_OK);
		return false;
	}
	fseek(fp, header.materialOffset, SEEK_SET);
	for (unsigned int i = 0; i < header.materialCnt; i++) {
		CMaterial* m = new CMaterial();
		if(header.version == A3MVERSION_OLD)
			fread(m->m_name, sizeof(SA3MaterialCHKOld), 1, fp);
		else
			fread(m->m_name, sizeof(SA3MaterialCHK2), 1, fp);

		m->m_shader_name = "SkinMesh";
		if (strlen(m->m_texture_name) > 0) {
			m->mp_texture = new CTexture();

			char str[PATH_SIZE];
			strcpy_s(str, PATH_SIZE, m_filePath);
			strcat_s(str, PATH_SIZE, m->m_texture_name);

			m->mp_texture->Load(str);
		}
		if (strlen(m->m_normal_map_name) > 0) {
			m->mp_normal_map = new CTexture();

			char str[PATH_SIZE];
			strcpy_s(str, PATH_SIZE, m_filePath);
			strcat_s(str, PATH_SIZE, m->m_normal_map_name);

			m->mp_normal_map->Load(str);
		}
		m_material_list.push_back(m);
	}


	m_bone_num = header.bone_num;
	if (m_bone_num > 0) {
		m_send_matrix = new CMatrix[m_bone_num];
		m_bone_matrix = new CMatrix[m_bone_num];
		m_old_matrix = new CMatrix[m_bone_num];
	}
	fseek(fp, header.rootOffset, SEEK_SET);
	mp_root = LoadNode(fp,nullptr, nullptr);


	fseek(fp, header.animationOffset, SEEK_SET);
	LoadAnimation(fp);
	fclose(fp);

	if (m_animation_size > 0) {

		m_animation.push_back(&m_animation_list);
		m_animation.push_back(&m_animation_list);

		m_enable_animation = true;
	} else {
		m_enable_animation = false;

	}

	CalcBoneMatrix(mp_root);
	m_min = header.min;
	m_max = header.max;
	if (cut_x * cut_y * cut_z > 0) {
		m_col_cut = new CA3MColl;
		m_col_cut->m_max = header.max;
		m_col_cut->m_min = header.min;
		CreateCollisionIndex(cut_x, cut_y, cut_z);
	}
	return true;
}


void CModelA3M::Render()
{
	CModel::UpdateMatrix();
	Render(m_matrix);
}
void CModelA3M::Render(const CMatrix& m)
{
	UpdateMatrix(m);
	

	DrawMesh(mp_root, CCamera::GetCurrent()->GetViewMatrix());
	
	
}

void CModelA3M::UpdateMatrix()
{
	CModel::UpdateMatrix();
	UpdateMatrix(m_matrix);
}
void CModelA3M::UpdateMatrix(const CMatrix &m)
{
	m_matrix = m;
	CalcBoneMatrix(mp_root);

#ifdef USE_PHYSX_LIB
	for (auto& j : m_joint_list) {
		j->Update(m_matrix);
		m_bone_matrix[j->m_joint_parent->m_bone_no] = j->m_joint_parent->m_matrix;
	}
	for (auto& j : m_joint_list) {
		if (!j->m_parent_joint) CalcBoneMatrix(j->m_joint_parent,false);
	}
#endif
}

void CModelA3M::UpdateAnimation()
{
	for(auto& v : m_animation)
		v.UpdateAnimation(m_time_scale);
}

bool CModelA3M::isAnimationEnd()
{

	return isAnimationEnd(0);
}

bool CModelA3M::isAnimationEnd(int layer)
{
	if ((int)m_animation.size() < layer+1) return true;
	return m_animation[layer].isAnimationEnd();
}

int CModelA3M::GetAnimationJam()
{
	return GetAnimationJam(0);
}

int CModelA3M::GetAnimationJam(int layer)
{
	if ((int)m_animation.size() < layer + 1) return 0;
	return m_animation[layer].GetAnimationJam();
}

void CModelA3M::SetAnimationSpeed(float s)
{
	SetAnimationSpeed(0, s);
}

void CModelA3M::SetAnimationSpeed(int layer,float s)
{
	if ((int)m_animation.size() < layer + 1) return;
	m_animation[layer].SetAnimationSpeed(s);
}

void CModelA3M::ChangeAnimation(int jam, bool loop, bool check, float time, float blendtime)
{
	ChangeAnimation(0, jam, loop, check, time, blendtime);
}

void CModelA3M::ChangeAnimation(int layer,int jam, bool loop, bool check, float time, float blendtime)
{
	if (m_animation.size() < (unsigned int)layer + 1) return;
	if (jam >= m_animation_size) return;
	if (m_animation[layer].ChangeAnimation(jam, loop, check, time, blendtime)) {
		for (auto& node : m_node_list) {
			if (node->m_animation_layer == layer) {
				m_old_matrix[node->m_bone_no] = m_bone_matrix[node->m_bone_no];
			}
		}
	}
}
float CModelA3M::GetAnimationFrame(int layer)
{
	return (float)m_animation[layer].GetAnimationFrame();
}
void CModelA3M::SetAnimationFrame(float frame)
{
	m_animation[0].SetAnimationFrame(frame);
}
void CModelA3M::SetAnimationFrame(int layer,float frame)
{
	m_animation[layer].SetAnimationFrame(frame);
}

int CModelA3M::GetAnimationSize() const
{
	return m_animation_size;
}

CMatrix CModelA3M::GetFrameMatrix(const char* name, bool local)const
{
	CA3MNode* n = CA3MNode::FindNode(name, mp_root);
	if (!n) return CMatrix();
	return local ? m_bone_matrix[n->m_bone_no] : m_matrix* m_bone_matrix[n->m_bone_no];
}

CMatrix CModelA3M::GetFrameMatrix(int no, bool local)const
{
	CA3MNode* n = CA3MNode::FindNode(no, mp_root);
	if (!n) return CMatrix();
	return local ? m_bone_matrix[n->m_bone_no] : m_matrix* m_bone_matrix[n->m_bone_no];
}

CA3MNode* CModelA3M::GetNode(int no) const
{
	return CA3MNode::FindNode(no, mp_root);
}
CA3MNode* CModelA3M::GetNode(const char* name) const
{
	return CA3MNode::FindNode(name, mp_root);
}

void CModelA3M::BindFrameMatrix(int no, const CMatrix& mat) {

	CA3MNode* n = CA3MNode::FindNode(no, mp_root);
	if (!n) return;
	n->m_bind = true;
	n->m_bind_matrix = mat;
}
void CModelA3M::BindFrameMatrix(const char* name, const CMatrix& mat) {

	CA3MNode* n = CA3MNode::FindNode(name, mp_root);
	if (!n) return;
	n->m_bind = true;
	n->m_bind_matrix = mat;
}

void CModelA3M::UnbindFrameMatrix(int no) {
	CA3MNode* n = CA3MNode::FindNode(no, mp_root);
	if (!n) return;
	n->m_bind = false;
}
void CModelA3M::UnbindFrameMatrix(const char* name) {
	CA3MNode* n = CA3MNode::FindNode(name, mp_root);
	if (!n) return;
	n->m_bind = false;
}

CMaterial* CModelA3M::GetMaterial(int no) const
{
	return m_material_list[no];
}

CMaterial* CModelA3M::GetMaterial(const char* name) const
{
	for (auto mat : m_material_list) {
		if (strcmp(name, mat->m_name)==0) return mat;
	}
	return nullptr;
}
void CModelA3M::SetMaterial(int no,CMaterial *m)
{
	m_material_list[no]=m;
}
bool CModelA3M::CollisionRay(CVector3D* pCross, CVector3D* pNormal, const CVector3D& s, const CVector3D& e)const
{
	if (!m_col_cut) return 0;
	CVector3D cross, normal;
	float length = 1e10;
	bool hit = false;
	CVector3D c, n;

	CMatrix inv = m_matrix.GetInverse();
	CVector3D s2 = inv * CVector4D(s, 1);
	CVector3D e2 = inv * CVector4D(e, 1);
	CVector3D dir = (e2 - s2).GetNormalize();
	for (auto it = m_mesh_list.begin(); it != m_mesh_list.end(); it++) {
		if ((*it)->mp_mesh->CollisionRay(&c, &n, s, e, &length, m_matrix*(*it)->m_matrix))
			hit = true;
	}
	if (hit) {
		if (pCross) {
			*pCross = c;
		}
		if (pNormal) {
			*pNormal = n;
		}
	}
	return hit;
}
std::vector<CCollTriangle> CModelA3M::CollisionRay(const CVector3D& s, const CVector3D& e) const
{
	std::vector<CCollTriangle> out;
	if (!m_col_cut) return out;
	for (auto& v : m_mesh_list) {
		v->mp_mesh->CollisionRay(out, s, e, m_matrix * v->m_matrix);
	}
	return out;
}
std::vector<CCollTriangle> CModelA3M::CollisionSphere(const CVector3D& center, float radius) const
{
	std::vector<CCollTriangle> out;
	if (!m_col_cut) return out;
	for (auto& v: m_mesh_list) {
		v->mp_mesh->CollisionSphere(out, center, radius, m_matrix*v->m_matrix);
	}
	return out;
}
;

std::vector<CCollTriangle>  CModelA3M::CollisionCapsule(const CCapsule& cap) const
{
	std::vector<CCollTriangle> out;
	if (!m_col_cut) return out;


	for (auto& v : m_mesh_list) {
		v->mp_mesh->CollisionCapsule(out, cap, m_matrix*v->m_matrix);

	}

	return out;
}

void CModelA3M::OutputFrameInfo(FILE* fp, CA3MNode* node,int depth)
{

	char indent[64] = "";
	for (int i = 0; i < depth; i++) {
		strcat_s(indent, 32, "  ");
	}
	fprintf(fp, "%s%s\n", indent, node->GetName());
	CMatrix& m = node->m_local_matrix;
	for (int i = 0; i < 4; i++) {
		fprintf(fp, " %s%f %f %f %f\n", indent, m.m[0][i], m.m[1][i], m.m[2][i], m.m[3][i]);

	}

	if (node->mp_child)
		OutputFrameInfo(fp, node->mp_child,depth+1);
	if (node->mp_next)
		OutputFrameInfo(fp, node->mp_next,depth);
}

void CModelA3M::OutputFrameInfo(const char* file_name)
{
	FILE* fp;
	fopen_s(&fp, file_name, "w");
	OutputFrameInfo(fp, mp_root, 0);
	fclose(fp);
}
void CModelA3M::SwapAnimationSet(int a, int b) {
	if (a < 0 || b<0 || a >= m_animation_size || b>m_animation_size) return;
	CA3MAnimationSet * pa = m_animation_list[a];
	CA3MAnimationSet * pb = m_animation_list[b];
	m_animation_list[a] = pb;
	m_animation_list[b] = pa;
}

void CModelA3M::LoadTexture(int no, const char* tex_name, const char* normal_name)
{
	CMaterial* mat = m_material_list[no];
	LoadTexture(&mat->mp_texture, tex_name);
	LoadTexture(&mat->mp_normal_map, normal_name);
}

CA3MAnimation::CA3MAnimation(std::vector<CA3MAnimationSet*>* animation_list):m_jam(0),m_speed(1.0), m_loop(false), m_frame(0)
{
	mp_animation_list = animation_list;
	ChangeAnimation(0, true, false, 0, 0);
}

CA3MAnimation::~CA3MAnimation()
{
}



bool CA3MAnimation::ChangeAnimation(int jam, bool loop, bool check, float time, float blend_time)
{
	if (check && jam == m_jam) return false;
	m_jam = jam;
	m_frame = time + (*mp_animation_list)[m_jam]->m_start;
	m_loop = loop;
	m_blend = blend_time > 0 ? 0.0f : 1.0f;
	m_blend_rate = blend_time >0 ? 1 / blend_time: 1.0f;
	return true;
}

void CA3MAnimation::UpdateAnimation(bool t)
{
	m_frame += m_speed * (*mp_animation_list)[m_jam]->m_fps * CFPS::GetDeltaTime()*(t?CFPS::GetTimeScale():1);
	int end = (*mp_animation_list)[m_jam]->m_end;
	if (m_frame > end) {
		if (m_loop) {
			m_frame = (float)(*mp_animation_list)[m_jam]->m_start;
		}
		else {
			m_frame = (float)end;
		}
	}

	m_blend=min(1.0f,m_blend + m_blend_rate*CFPS::GetDeltaTime());
}

bool CA3MAnimation::isAnimationEnd()
{
	CA3MAnimationSet* s = (*mp_animation_list)[m_jam];
	return (m_frame >= s->m_end);
}


int CA3MAnimation::GetAnimationJam()
{
	return m_jam;
}

void CA3MAnimation::SetAnimationSpeed(float s)
{
	m_speed = s;
}

int CA3MAnimation::GetAnimationFrame()
{
	return (int)m_frame;
}
void CA3MAnimation::SetAnimationFrame(float frame)
{
	m_frame = frame;
}

CA3MColl::CA3MColl():m_max(FLT_MIN, FLT_MIN, FLT_MIN),m_min(FLT_MAX, FLT_MAX, FLT_MAX), m_cut(0.0f,0.0f,0.0f), m_length(0.0f,0.0f,0.0f),m_colmax(nullptr),m_colmin(nullptr)
{
}

CA3MColl::~CA3MColl()
{
	if (m_colmin) {
		delete m_colmin;
		m_colmin = nullptr;
	}
	if (m_colmax) {
		delete m_colmax;
		m_colmax = nullptr;
	}
}

void CA3MColl::CalcRange(CVector3D* min, CVector3D* max, const CAABB& aabb)const
{
		min->ix = max(0, min(m_cut.ix - 1, (int)((aabb.min.x - m_min.x) / m_length.x)));
		max->ix = max(0, min(m_cut.ix - 1, (int)((aabb.max.x - m_min.x) / m_length.x)));

		min->iz = max(0, min(m_cut.iz - 1, (int)((aabb.min.z - m_min.z) / m_length.z)));
		max->iz = max(0, min(m_cut.iz - 1, (int)((aabb.max.z - m_min.z) / m_length.z)));

		min->iy = max(0, min(m_cut.iy - 1, (int)((aabb.min.y - m_min.y) / m_length.y)));
		max->iy = max(0, min(m_cut.iy - 1, (int)((aabb.max.y - m_min.y) / m_length.y)));
	
}


#ifdef USE_PHYSX_LIB
void CModelA3M::CreatJoint(CA3MNode* root, CA3MNode* target, const CMatrix& root_matrix)
{
	CA3MNode* p = root;
	CJointBone* prev=nullptr;
	while (p != target) {
		CJointBone* j = new CJointBone(p, prev,prev ? prev->m_px_actor:nullptr, p->mp_child, root_matrix);
		m_joint_list.push_back(j);
		prev = j;
		p = p->mp_child;
	}
}
CJointBone::CJointBone(CA3MNode* parent, CJointBone* parent_joint,physx::PxRigidDynamic* parent_actor, CA3MNode* child, const CMatrix& root_matrix)
{
	m_joint_child = child;
	m_joint_parent = parent;
	m_parent_joint = parent_joint;
	auto physics = CPhysX::GetInstance()->GetPxPhysics();
	auto scene = CPhysX::GetInstance()->GetScene();
	CMatrix parent_m = root_matrix * parent->m_matrix;
	CVector3D parent_pos = parent_m.GetPosition();

	m_offset = parent->m_matrix.GetInverse();
	CMatrix m = root_matrix * child->m_matrix;
	CVector3D pos = (root_matrix * child->m_matrix).GetPosition();

	m_px_actor = physx::PxCreateDynamic(*physics, physx::PxTransform(ToPxMat44(CMatrix::MTranselate(pos))), physx::PxSphereGeometry(0.01f), *physics->createMaterial(1.0f, 1.0f, 1.0f), 1.f);
	m_px_actor->setAngularDamping(10.5f);
	m_px_actor->setMass(0.0000001f);
	m_px_actor->setLinearVelocity(physx::PxVec3(0, 0, 0));
	scene->addActor(*m_px_actor);


	CVector3D vec = ToVector3(m_px_actor->getGlobalPose().p) - parent_pos;
	m_px_joint = PxD6JointCreate(*physics,
		parent_actor, parent_actor ? physx::PxTransform(ToPxVec3(vec / 2)) : physx::PxTransform(ToPxVec3(parent_pos)),
		m_px_actor, physx::PxTransform(ToPxVec3(-vec / 2)));
	//actor->m_px_joint->setLimitCone(physx::PxJointLimitCone(physx::PxPi / 6, physx::PxPi / 6, physx::PxSpring(1000000, 1000000)));
	//actor->m_px_joint->setSphericalJointFlag(physx::PxSphericalJointFlag::eLIMIT_ENABLED, true);

	m_px_joint->setMotion(physx::PxD6Axis::eX, physx::PxD6Motion::eLOCKED);
	m_px_joint->setMotion(physx::PxD6Axis::eY, physx::PxD6Motion::eLOCKED);
	m_px_joint->setMotion(physx::PxD6Axis::eZ, physx::PxD6Motion::eLOCKED);
	m_px_joint->setLinearLimit(physx::PxJointLinearLimit(0.1f));
	//actor->m_px_joint->setDistanceLimit(physx::PxJointLinearLimit(vec.Length(), physx::PxSpring(1.0, 0.4)));

	m_px_joint->setMotion(physx::PxD6Axis::eSWING1, physx::PxD6Motion::eLIMITED);
	m_px_joint->setMotion(physx::PxD6Axis::eSWING2, physx::PxD6Motion::eLIMITED);
	m_px_joint->setMotion(physx::PxD6Axis::eTWIST, physx::PxD6Motion::eLOCKED);

	m_px_joint->setSwingLimit(physx::PxJointLimitCone((float)M_PI / 40, (float)M_PI / 40, physx::PxSpring(0.05f, 0.04f)));
	m_px_joint->setTwistLimit(physx::PxJointAngularLimitPair(-(float)M_PI / 8, (float)M_PI / 8));
}
CJointBone::~CJointBone()
{
	auto scene = CPhysX::GetInstance()->GetScene();
	m_px_joint->release();
	scene->removeActor(*m_px_actor);
	m_px_actor->release();
}

void CJointBone::Update(const CMatrix& root_matrix)
{
	auto f = m_px_joint->getConstraintFlags();
	if (f.isSet(physx::PxConstraintFlag::eBROKEN)) {
		printf("brael;");
	}

	if (m_joint_child) {

		m_joint_parent->LookAt(m_joint_parent, m_joint_child, root_matrix.GetInverse() * CVector4D(ToVector3(m_px_actor->getGlobalPose().p), 1));

	}

	if (!m_parent_joint && m_joint_parent && m_joint_child) {
		CMatrix pm = root_matrix * m_joint_parent->m_matrix * m_offset;
		CVector3D scale(pm.GetLeft().Length(), pm.GetUp().Length(), pm.GetFront().Length());
		CMatrix R = CMatrix::MScale(1 / scale.x, 1 / scale.y, 1 / scale.z) * pm;
		R.m03 = 0; R.m13 = 0; R.m23 = 0;
		m_px_joint->setLocalPose(physx::PxJointActorIndex::eACTOR0, physx::PxTransform(ToPxMat44(CMatrix::MTranselate((root_matrix * m_joint_child->m_matrix).GetPosition()) * R)));
		//node->m_px_joint->setLocalPose(physx::PxJointActorIndex::eACTOR0, physx::PxTransform(ToPxMat44(CMatrix::MTranselate(pm.GetPosition()))));
	}
}
#endif