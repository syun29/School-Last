#include "CMatrix.h"


void CMatrix::SetIndentity() {
	memset(f,0,sizeof(float)*16);
	m00=1.0f;
	m11=1.0f;
	m22=1.0f;
	m33=1.0f;
}
void CMatrix::Transelate(float x,float y, float z) {
	m03 = x;
	m13 = y;
	m23 = z;
}

void CMatrix::Transelate(const CVector3D &v) {
	m03 = v.x;
	m13 = v.y;
	m23 = v.z;

}
void CMatrix::Scale(float x,float y, float z) {
	m00=x;
	m11=y;
	m22=z;
}
void CMatrix::Scale(const CVector3D &v) {
	m00 = v.x;
	m11 = v.y;
	m22 = v.z;
}
void CMatrix::RotationX(float r){
	m11 = cosf(r);
	m22 = cosf(r);
	m12 =-sinf(r);
	m21 = sinf(r);
}
void CMatrix::RotationY(float r){
	m00 = cosf(r);
	m22 = cosf(r);
	m20 =-sinf(r);
	m02 = sinf(r);
}
void CMatrix::RotationZ(float r) {
	m00 = cosf(r);
	m11 = cosf(r);
	m01 =-sinf(r);
	m10 = sinf(r);
}
void CMatrix::Rotation(const CVector3D& r) {
	Rotation(r.x, r.y,r.z);
}
void CMatrix::Rotation(float rx, float ry, float rz) {
	*this = MRotation(rx,ry,rz);

}
CMatrix CMatrix::MTranselate(float x, float y, float z){
	CMatrix m;
	m.Transelate(x, y, z);
	return m;
}

CMatrix CMatrix::MTranselate(const CVector3D& v) {
	CMatrix m;
	m.Transelate(v.x,v.y,v.z);
	return m;
}
CMatrix CMatrix::MScale(float x, float y, float z){
	CMatrix m;
	m.Scale(x, y, z);
	return m;
}
CMatrix CMatrix::MScale(const CVector3D& v) {
	CMatrix m;
	m.Scale(v.x, v.y, v.z);
	return m;
}
CMatrix CMatrix::MRotationX(float r){
	CMatrix m;
	m.RotationX(r);
	return m;
}
CMatrix CMatrix::MRotationY(float r){
	CMatrix m;
	m.RotationY(r);
	return m;
}
CMatrix CMatrix ::MRotationZ(float r){
	CMatrix m;
	m.RotationZ(r);
	return m;
}
CMatrix CMatrix::MRotation(const CVector3D& r) {
	return MRotation(r.x, r.y, r.z);
}
CMatrix CMatrix::MRotation(float rx, float ry, float rz) {
	return MRotationZ(rz)*MRotationY(ry)*MRotationX(rx);
}
CMatrix CMatrix::MRotation(const CQuaternion& q)
{
	CMatrix ret;
	float sx = q.x * q.x;
	float sy = q.y * q.y;
	float sz = q.z * q.z;
	float cx = q.y * q.z;
	float cy = q.x * q.z;
	float cz = q.x * q.y;
	float wx = q.w * q.x;
	float wy = q.w * q.y;
	float wz = q.w * q.z;

	ret.m00 = 1.0f - 2.0f * (sy + sz);
	ret.m10 = 2.0f * (cz + wz);
	ret.m20 = 2.0f * (cy - wy);
	ret.m01 = 2.0f * (cz - wz);
	ret.m11 = 1.0f - 2.0f * (sx + sz);
	ret.m21 = 2.0f * (cx + wx);
	ret.m02 = 2.0f * (cy + wy);
	ret.m12 = 2.0f * (cx - wx);
	ret.m22 = 1.0f - 2.0f * (sx + sy);
	ret.m03 = 0.0f;
	ret.m13 = 0.0f;
	ret.m23 = 0.0f;
	return ret;
}
CMatrix CMatrix::MRotationAxis(float vx, float vy, float vz, float r){
	CMatrix m;
	m.RotationAxis(vx, vy, vz, r);
	return m;
}
CMatrix CMatrix::MRotationAxis(const CVector3D& v, float r) {
	CMatrix m;
	m.RotationAxis(v.x, v.y, v.z, r);
	return m;
}
void CMatrix::RotationAxis(float vx,float vy,float vz,float r){
	float sine	=sinf(r);
	float cosine=cosf(r);

	m00=vx*vx*(1.0f-cosine)+cosine;
	m01=vx*vy*(1.0f-cosine)-vz*sine;
	m02=vx*vz*(1.0f-cosine)+vy*sine;

	m10=vy*vx*(1.0f-cosine)+vz*sine;
	m11=vy*vy*(1.0f-cosine)+cosine;
	m12=vy*vz*(1.0f-cosine)-vx*sine;

	m20=vz*vx*(1.0f-cosine)-vy*sine;
	m21=vz*vy*(1.0f-cosine)+vx*sine;
	m22=vz*vz*(1.0f-cosine)+cosine;

}

void CMatrix::LookAt(const CVector3D& pos, const CVector3D& at, const CVector3D& up) {
	CVector3D f = (at- pos).GetNormalize();
	CVector3D r = CVector3D::Cross(up, f).GetNormalize();
	CVector3D u = CVector3D::Cross(f, r).GetNormalize();
	CVector3D t = pos;
	m00 = r.x; m01 = u.x; m02 = f.x;
	m10 = r.y; m11 = u.y; m12 = f.y;
	m20 = r.z; m21 = u.z; m22 = f.z;
	m03 = pos.x;
	m13 = pos.y;
	m23 = pos.z;

}
void CMatrix::LookAtView(const CVector3D& eye,const CVector3D& at,const CVector3D& up){
	
	CVector3D f = (eye - at).GetNormalize();
	CVector3D r = CVector3D::Cross(up,f).GetNormalize();
	CVector3D u = CVector3D::Cross(f,r).GetNormalize();
	CVector3D t = at-eye;
	m00 = r.x; m01 = r.y; m02 = r.z;
	m10 = u.x; m11 = u.y; m12 = u.z;
	m20 = f.x; m21 = f.y; m22 = f.z;
	m03 = -(eye.x * r.x + eye.y * r.y + eye.z * r.z);
	m13 = -(eye.x * u.x + eye.y * u.y + eye.z * u.z);
	m23 = -(eye.x * f.x + eye.y * f.y + eye.z * f.z);

}
void CMatrix::Perspective(float fovy,float aspect,float zNear,float zFar){
	float f = 1/tanf((fovy/2));
	m00 = f/aspect;
	m11 = f;
	m22 = (zFar+zNear)/(zNear-zFar);
	m23 = (2*zFar*zNear)/(zNear-zFar);
	m32 = -1;
	m33 = 0;
}

void CMatrix::Ortho(float left, float right, float bottom, float top, float zNear, float zFar) {
	m00 = 2 / (right - left);
	m11 = 2 / (top - bottom);
	m22 = -2 / (zFar - zNear);
	m23 = -(zFar + zNear) / (zFar - zNear);

}
void CMatrix::Viewport(float x,float y,float w,float h) {
	
	m00 = w/2.0f;
	m03 = x+w/2.0f;
	m11 = -h/2.0f;
	m13 = y+h/2.0f;
}

CMatrix CMatrix::operator +(const CMatrix& num) const {
	CMatrix ans;
	
	for(int i=0;i<16;i++)
		ans.f[i]=f[i]+num.f[i];

	return ans;
}
CMatrix CMatrix::operator *(CMatrix num) const {
	CMatrix ans;


	for(int i=0;i<4;i++){
		for(int j=0;j<4;j++){
			ans.m[i][j] =0.0f;

			//s—ñÏ
			for(int k=0;k<4;k++){
				ans.m[i][j] += num.m[i][k]*m[k][j];
			}
		}
	}

	return ans;
}
CMatrix CMatrix::operator *(const float num) const {
	CMatrix ans;
	
	for(int i=0;i<16;i++)
		ans.f[i]=f[i]*num;

	return ans;
}
CMatrix operator *(const float num,const CMatrix mtr) {
	CMatrix ans;
	
	for(int i=0;i<16;i++)
		ans.f[i]=mtr.f[i]*num;

	return ans;
}

CVector4D CMatrix::operator * (const CVector4D num) const {
	CVector4D ans;
	for(int i=0;i<4;i++) {
		ans.v[i] = 0;
		for(int j=0;j<4;j++) {
			ans.v[i]+=m[j][i]*num.v[j];
		}
	}
	return ans;
}
CVector3D CMatrix::operator * (const CVector3D num) const {
	CVector3D ans;
	for(int i=0;i<3;i++) {
		ans.v[i] = 0;
		for(int j=0;j<3;j++) {
			ans.v[i]+=m[j][i]*num.v[j];
		}
	}
	return ans;
}

CMatrix& CMatrix::operator += (const CMatrix& num){

	for (int i = 0; i<16; i++)
		f[i] += num.f[i];

	return *this;

}
CMatrix& CMatrix::operator *=(CMatrix num){
	*this = *this * num;
	return *this;
}
CMatrix& CMatrix::operator *=(const float num){
	for (int i = 0; i<16; i++)
		f[i] *= num;

	return *this;

}




CVector4D operator * (const CVector4D num,const CMatrix mtr){
	CVector4D ans;
	for(int i=0;i<4;i++) {
		ans.v[i] = 0;
		for(int j=0;j<4;j++) {
			ans.v[i]+=num.v[j] * mtr.m[j][i];
		}
	}
	return ans;
}
CVector3D operator * (const CVector3D num,const CMatrix mtr){
	CVector3D ans;
	for(int i=0;i<3;i++) {
		ans.v[i] = 0;
		for(int j=0;j<3;j++) {
			ans.v[i]+=num.v[j] * mtr.m[j][i];
		}
	}
	return ans;
}

void CMatrix::SetQuaternion(float x,float y, float z,float w){
/*
	float sx = x * x;
    float sy = y * y;
    float sz = z * z;
    float cx = y * z;
    float cy = x * z;
    float cz = x * y;
    float wx = w * x;
    float wy = w * y;
    float wz = w * z;


	m00 = 1.0 - 2.0 * (sy + sz);
    m10 = 2.0 * (cz + wz);
    m20 = 2.0 * (cy - wy);
    m01 = 2.0 * (cz - wz);
    m11 = 1.0 - 2.0 * (sx + sz);
    m21 = 2.0 * (cx + wx);
    m02 = 2.0 * (cy + wy);
    m12 - 2.0 * (cx - wx);
    m22 = 1.0 - 2.0 * (sx + sy);
    m03 = 0.0;
    m13 = 0.0;
    m23 = 0.0;
*/
	/*
	m00 = 1-2*y*y-2*z*z;	m01 = 2*x*y+2*w*z;		m02 = 2*x*z-2*w*y;		m03 = 0;
	m10 = 2*x*y-2*w*z;		m11 = 1-2*x*x-2*z*z;	m12 = 2*y*z+2*w*x;		m13 = 0;
	m20 = 2*x*z+2*w*y;		m21 = 2*y*z-2*w*x;		m22 = 1-2*x*x-2*y*y;	m23 = 0;
	m30 = 0;				m31 = 0;				m32 = 0;				m33 = 1;
	*/
	
	float x2 = x + x, y2 = y + y, z2 = z + z;
	float xx = x * x2, xy = x * y2, xz = x * z2;
	float yy = y * y2, yz = y * z2, zz = z * z2;
	float wx = w * x2, wy = w * y2, wz = w * z2;

	m00 = 1 - (yy + zz);
	m01= xy - wz;
	m02 = xz + wy;

	m10 = xy + wz;
	m11 = 1 - (xx + zz);
	m12 = yz - wx;

	m20 = xz - wy;
	m21 = yz + wx;
	m22 = 1 - (xx + yy);

	// last column
	m30 = 0;
	m31 = 0;
	m32 = 0;

	// bottom row
	m03 = 0;
	m13 = 0;
	m23 = 0;
	m33 = 1;

}

void CMatrix::SetQuaternion(const CQuaternion& q)
{
	SetQuaternion(q.x, q.y, q.z, q.w);
}

CMatrix CMatrix::MQuaternion(CQuaternion& q)
{
	CMatrix m;
	m.SetQuaternion(q.x, q.y, q.z, q.w);
	return m;
}

CQuaternion CMatrix::GetQuaternion() {
	CQuaternion ret;
	return ret;
}
/** ‹ts—ñŽæ“¾ **/
CMatrix CMatrix::GetInverse( void ) const
{
	float det = 0.0f;
	det += m00*m11*m22*m33 + m00*m12*m23*m31 + m00*m13*m21*m32;
	det += m01*m10*m23*m32 + m01*m12*m20*m33 + m01*m13*m22*m30;
	det += m02*m10*m21*m33 + m02*m11*m23*m30 + m02*m13*m20*m31;
	det += m03*m10*m22*m31 + m03*m11*m20*m32 + m03*m12*m21*m30;

	det -= m00*m11*m23*m32 + m00*m12*m21*m33 + m00*m13*m22*m31;
	det -= m01*m10*m22*m33 + m01*m12*m23*m30 + m01*m13*m20*m32;
	det -= m02*m10*m23*m31 + m02*m11*m20*m33 + m02*m13*m21*m30;
	det -= m03*m10*m21*m32 + m03*m11*m22*m30 + m03*m12*m20*m31;

	CMatrix b;

	b.m00 = m11*m22*m33 + m12*m23*m31 + m13*m21*m32 - m11*m23*m32 - m12*m21*m33 - m13*m22*m31;
	b.m01 = m01*m23*m32 + m02*m21*m33 + m03*m22*m31 - m01*m22*m33 - m02*m23*m31 - m03*m21*m32;
	b.m02 = m01*m12*m33 + m02*m13*m31 + m03*m11*m32 - m01*m13*m32 - m02*m11*m33 - m03*m12*m31;
	b.m03 = m01*m13*m22 + m02*m11*m23 + m03*m12*m21 - m01*m12*m23 - m02*m13*m21 - m03*m11*m22;

	b.m10 = m10*m23*m32 + m12*m20*m33 + m13*m22*m30 - m10*m22*m33 - m12*m23*m30 - m13*m20*m32;
	b.m11 = m00*m22*m33 + m02*m23*m30 + m03*m20*m32 - m00*m23*m32 - m02*m20*m33 - m03*m22*m30;
	b.m12 = m00*m13*m32 + m02*m10*m33 + m03*m12*m30 - m00*m12*m33 - m02*m13*m30 - m03*m10*m32;
	b.m13 = m00*m12*m23 + m02*m13*m20 + m03*m10*m22 - m00*m13*m22 - m02*m10*m23 - m03*m12*m20;

	b.m20 = m10*m21*m33 + m11*m23*m30 + m13*m20*m31 - m10*m23*m31 - m11*m20*m33 - m13*m21*m30;
	b.m21 = m00*m23*m31 + m01*m20*m33 + m03*m21*m30 - m00*m21*m33 - m01*m23*m30 - m03*m20*m31;
	b.m22 = m00*m11*m33 + m01*m13*m30 + m03*m10*m31 - m00*m13*m31 - m01*m10*m33 - m03*m11*m30;
	b.m23 = m00*m13*m21 + m01*m10*m23 + m03*m11*m20 - m00*m11*m23 - m01*m13*m20 - m03*m10*m21;

	b.m30 = m10*m22*m31 + m11*m20*m32 + m12*m21*m30 - m10*m21*m32 - m11*m22*m30 - m12*m20*m31;
	b.m31 = m00*m21*m32 + m01*m22*m30 + m02*m20*m31 - m00*m22*m31 - m01*m20*m32 - m02*m21*m30;
	b.m32 = m00*m12*m31 + m01*m10*m32 + m02*m11*m30 - m00*m11*m32 - m01*m12*m30 - m02*m10*m31;
	b.m33 = m00*m11*m22 + m01*m12*m20 + m02*m10*m21 - m00*m12*m21 - m01*m10*m22 - m02*m11*m20;

	CMatrix tmp = b * (1/det);


	return tmp;
};

CMatrix CMatrix::GetTranspose(void) const {
	CMatrix tmp;
	tmp.m00 = m00; tmp.m01 = m10; tmp.m02 = m20; tmp.m03 = m30;
	tmp.m10 = m01; tmp.m11 = m11; tmp.m12 = m21; tmp.m13 = m31;
	tmp.m20 = m02; tmp.m21 = m12; tmp.m22 = m22; tmp.m23 = m32;
	tmp.m30 = m03; tmp.m31 = m13; tmp.m32 = m23; tmp.m33 = m33;
	return tmp;
}
static float _clamp(float t, float min, float max) {
	return (t<min) ? min : (t>max) ? max : t;
}
CVector3D CMatrix::GetEuler(int type) const {
	CVector3D ret;
	switch (type) {
	case eZYX:
		ret.y = asinf(-_clamp(m20, -1, 1));

		if (fabsf(m20) < 0.99999f) {

			ret.x = atan2f(m21, m22);
			ret.z = atan2f(m10, m00);

		}
		else {

			ret.x = 0;
			ret.z = atan2f(-m02, m11);

		}
		break;
	case eYXZ:

		ret.x = asinf(-_clamp(m12, -1, 1));

		if (fabsf(m12) < 0.99999f) {

			ret.y = atan2f(m02, m22);
			ret.z = atan2f(m10, m11);

		}
		else {

			ret.y = atan2f(-m20, m00);
			ret.z = 0;

		}
		break;

	}
	/*
	ret.y = asin(_clamp(m02, -1, 1));

	if (fabs(m02) < 0.99999) {

		ret.x = atan2(-m12, m22);
		ret.z = atan2(-m01, m00);

	}
	else {

		ret.x = atan2(m21, m11);
		ret.z = 0;

	}
	*/
	return ret;

}

CVector3D CMatrix::GetScale() const {
	return CVector3D(this->GetLeft().Length(), this->GetUp().Length(), this->GetFront().Length());
}
const CMatrix CMatrix::indentity(1,0,0,0,
							0,1,0,0,
							0,0,1,0,
							0,0,0,1);