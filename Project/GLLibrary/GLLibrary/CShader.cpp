
#include "CShader.h"
#include "CLight.h"
#include "CRendaring.h"


//SHADOW_MAX シャドウマップ最大枚数
//LIGHT_MAX 光源の最大数 CLight::LIGHT_MAXより自動計算
//SHADOW_SIZE ディファードレンダリング用シャドウ情報 SHADOW_MAX/3
const char* mesh_vert= "#version 430\n\n"\
"#define SHADOW_MAX 12\n"\
"uniform mat4 WorldMatrix;\n"\
"uniform mat4 ModelViewMatrix;\n"\
"uniform mat4 ProjectionMatrix;\n"\
"uniform mat4 ShadowTextureMatrix[SHADOW_MAX];\n"\
"layout(location = 0) in vec3	Vertex;\n"\
"layout(location = 1) in vec3	Normal;\n"\
"layout(location = 2) in vec2	TexCoord;\n"\
"layout(location = 6) in vec3	Tangent;\n"\
"//フラグメントシェーダーに渡す変数\n"\
"out vec4 V;//座標\n"\
"out vec3 N;//法線ベクトル\n"\
"out vec2 texCoord;\n"\
"out vec3 T;//接線ベクトル\n"\
"out vec3 B;//従法線ベクトル\n"\
"out vec4 vShadowCoord[SHADOW_MAX];    //!< シャドウデプスマップの参照用座標\n"\
"uniform int usenormalMap;\n"\
"uniform int depth_tex_size;\n"\
"void main(void)\n"\
"{\n"\
"	V = WorldMatrix * vec4(Vertex, 1);\n"\
"	gl_Position = ProjectionMatrix * ModelViewMatrix * vec4(Vertex, 1);\n"\
"	N = normalize(mat3(WorldMatrix) * Normal);\n"\

"	for(int i=0;i<depth_tex_size;i++) {\n"\
"		vShadowCoord[i] = ShadowTextureMatrix[i] * V;    // 影用座標値(光源中心座標)\n"\
"	}\n"\
"	texCoord = TexCoord;\n"\
"	if(usenormalMap==1) {\n"\
"		T = normalize(mat3(WorldMatrix) * Tangent);\n"\
"		B = cross(N, T);//従法線ベクトル\n"\
"	}\n"
"}";


const char *skin_mesh_vert = "#version 430\n\n"\
"#define SHADOW_MAX 12\n"\
"uniform mat4 Transforms[224];\n"\
"uniform mat4 WorldMatrix;\n"\
"uniform mat4 LocalMatrix;\n"\
"uniform mat4 ModelViewMatrix;\n"\
"uniform mat4 ProjectionMatrix;\n"\
"uniform mat4 ShadowTextureMatrix[SHADOW_MAX];\n"\
"uniform int useSkin = 0;\n"\
"layout(location = 0) in vec3 Vertex;\n"\
"layout(location = 1) in vec3 Normal;\n"\
"layout(location = 2) in vec2 TexCoord;\n"\
"layout(location = 3) in vec4 weights;\n"\
"layout(location = 4) in vec4 indices;\n"\
"layout(location = 6) in vec3	Tangent;\n"\
"//フラグメントシェーダーに渡す変数\n"\
"out vec4 V;//座標\n"\
"out vec3 N;//法線ベクトル\n"\
"out vec3 T;//接線ベクトル\n"\
"out vec3 B;//従法線ベクトル\n"\
"out vec2 texCoord;\n"\
"out vec4 vShadowCoord[SHADOW_MAX];    //!< シャドウデプスマップの参照用座標\n"\
"uniform int usenormalMap;\n"\
"uniform int depth_tex_size;\n"\
"void main(void)\n"\
"{\n"\
"	mat4 comb = mat4(0);\n"\
"	if (useSkin==1) {\n"\
"		comb += Transforms[int(indices.x)] * weights.x;\n"\
"		comb += Transforms[int(indices.y)] * weights.y;\n"\
"		comb += Transforms[int(indices.z)] * weights.z;\n"\
"		comb += Transforms[int(indices.w)] * weights.w;\n"\
"		vec4 skinPosition = ModelViewMatrix * comb * LocalMatrix*vec4(Vertex, 1);\n"\
"		V = WorldMatrix * comb * LocalMatrix * vec4(Vertex, 1);\n"\
"		gl_Position = ProjectionMatrix * skinPosition;\n"\
"		N = normalize(mat3(WorldMatrix * comb*LocalMatrix) * Normal);\n"\
"	} else {\n"\
"		V = WorldMatrix * vec4(Vertex, 1);\n"\
"		gl_Position = ProjectionMatrix * ModelViewMatrix * LocalMatrix * vec4(Vertex, 1);\n"\
"		N = normalize(mat3(WorldMatrix) * Normal);\n"\
"	}\n"\
"	for(int i=0;i<depth_tex_size;i++) {\n"\
"		vShadowCoord[i] = ShadowTextureMatrix[i] * V;    // 影用座標値(光源中心座標)\n"\
"	}\n"\
"	if(usenormalMap==1) {\n"\
"		T = normalize(mat3(WorldMatrix * comb*LocalMatrix) * Tangent);\n"\
"		B = cross(N, T);//従法線ベクトル\n"\
"	}\n"
"	texCoord = TexCoord;\n"\
"}";

const char* mesh_frag = "#version 430\n\n"\
"#define SHADOW_MAX 12\n"\
"#define LIGHT_MAX 20\n"\
"uniform vec3 lightPos[LIGHT_MAX];\n"\
"uniform vec3 lightDir[LIGHT_MAX];\n"\
"uniform vec3 lightAmbientColor[LIGHT_MAX];\n"\
"uniform vec3 lightDiffuseColor[LIGHT_MAX];\n"\
"uniform float lightRange[LIGHT_MAX];\n"\
"uniform int lightType[LIGHT_MAX];\n"\
"uniform float lightRadiationAngle[LIGHT_MAX];\n"\
"uniform int lightUseShadow[LIGHT_MAX];\n"\
"uniform vec2 stscroll;\n"\
"uniform vec3 eyeVec;\n"\
"uniform vec3 eyePos;\n"\
"uniform vec4 Ambient;\n"\
"uniform vec4 Diffuse;\n"\
"uniform vec3 Specular;\n"\
"uniform vec3 Emissive;\n"\
"uniform float Pow;\n"\
"uniform float alpha;\n"\
"uniform int lighting;\n"\
"uniform int uSetex;\n"\
"uniform int usenormalMap;\n"\
"uniform vec4 fogColor;\n"\
"uniform float fogNear;\n"\
"uniform float fogFar;\n"\
"uniform int depth_tex_size;\n"\
"in vec4 vShadowCoord[SHADOW_MAX];    //!< シャドウデプスマップの参照用座標\n"\
"uniform sampler2D depth_tex[SHADOW_MAX];    //!< デプス値テクスチャ\n"\
"uniform float shadow_ambient;    //!< 影の濃さ\n"\
"uniform vec2 d_tex_scale[SHADOW_MAX];\n"\
"//頂点シェーダーから受け取る変数\n"\
"in vec4 V;//位置\n"\
"in vec3 N;//法線ベクトル\n"\
"in vec3 T;//接線ベクトル\n"\
"in vec3 B;//従法線ベクトル\n"\
"in vec2 texCoord;\n"\
"uniform sampler2D sampler;\n"\
"uniform sampler2D normalMap;//法線マップ\n"\
"out vec4 out_color;\n"\
"uniform float shadow_bias;\n"\
"uniform int toon;\n"\
"float restDepth(vec4 RGBA) {\n"\
"	const float rMask = 1.0;\n"\
"	const float gMask = 1.0 / 255.0;\n"\
"	const float bMask = 1.0 / (255.0 * 255.0);\n"\
"	const float aMask = 1.0 / (255.0 * 255.0 * 255.0);\n"\
"	float depth = dot(RGBA, vec4(rMask, gMask, bMask, aMask));\n"\
"	return depth;\n"\
"}\n"\
"bool shadowmap(int i,vec3 coord) {\n"\
"	if (texture2D(depth_tex[i], coord.xy).z  <  coord.z-shadow_bias "\
"		&& coord.x>0 && coord.x<1.0 && coord.y>0.0 && coord.y<1.0)\n"\
"	return true;\n"\
"	return false;\n"\
"}\n"\
"void main(void)\n"\
"{\n"\
"	vec4 texColor = vec4(1, 1, 1, 1);\n"\
"	if (uSetex != 0) {\n"\
"		texColor = texture2D(sampler, texCoord + stscroll);\n"\
"	}\n"\
"	if(texColor.a * alpha <=0) discard;\n"
"	vec3 D = vec3(0, 0, 0);\n"\
"	vec3 S = vec3(0, 0, 0);\n"\
"	vec3 color;\n"\
"	float visibility = 1.0;\n"\
"	vec3 E = normalize(eyePos - V.xyz);\n"\
"	vec3 Normal=N;\n"\
"	if (usenormalMap != 0) {\n"\
"		Normal = (texture2D(normalMap, texCoord + stscroll).xyz - 0.5) * 2.0;\n"\
"		Normal = T * Normal.x + B * Normal.y + N * Normal.z;\n"\
"		//Normal.xyz = (Normal / 2.0f) + 0.5f; //-1～1から0～1に補正\n"\
"		//Normal.w = texture2D(specurMap, texCoord + stscroll).r;\n"\
"		//Normal.w = 0;\n"\
"	}\n"\
"	if (lighting == 1) {\n"\
"		for (int i = 0; i < LIGHT_MAX; i++) {\n"\
"			if (lightType[i] == 0) continue;\n"\
"			vec3 vec = vec3(0, 0, 0);\n"\
"			float p = 0;\n"\
"			if (lightType[i] == 1) {\n"\
"				vec = -lightDir[i];\n"\
"				p = 1;\n"\
"			}else\n"\
"			if (lightType[i] == 2 || lightType[i] == 3) {\n"\
"					vec = vec3(V.xyz - lightPos[i]);\n"\
"					float l = length(vec);\n"\
"					vec = normalize(vec);\n"\
"					if (l > 0)\n"\
"						p = 1.0-clamp(pow(l / lightRange[i], 5), 0.0, 1.0);\n"\
"					else\n"\
"						p = 1.0;\n"\
"					if (lightType[i] == 3) {\n"\
"						float t = acos(dot(vec, lightDir[i]));\n"\
"						p *= 1.0-clamp(pow(t / lightRadiationAngle[i], 5), 0.0, 1.0); \n"\
"					}\n"\
"					vec = -vec;\n"\
"				}\n"\
"			vec3 L = vec; \n"\
"			float NL = max(0, dot(Normal, L)); \n"\
"			if (toon == 1)\n"\
"			//NL = (NL>0.0) ? ((NL>0.3) ? 1.0:0.95):0.80;\n"\
"			NL = (NL>0.0) ? 1.0:1-shadow_ambient;\n"\
"			vec3 R = reflect(-E, Normal);\n"\
"			S += pow(max(0, dot(R, L)), Pow) * p;\n"\
"			D += (lightDiffuseColor[i] * clamp(NL, 0.0, 1.0) + lightAmbientColor[i]) * p;\n"\
"		}\n"\
"		float l = length(eyePos - V.xyz);\n"\
"		float f = clamp((fogFar - l) / (fogFar - fogNear), 0.0, 1.0);\n"\
"		color = texColor.xyz * (visibility * Diffuse.xyz * clamp(D, 0.0, 1.0)) + visibility * Specular * clamp(S, 0.0, 1.0) + Emissive;\n"\
"		out_color = vec4(color + fogColor.xyz * (1.0 - f), clamp((texColor.w * Diffuse.w * alpha) - ((1.0 - fogColor.w) * (1.0 - f)), 0.0f, 1.0f));\n"\
"	}\n"\
"	else {\n"\
"		color = Diffuse.xyz;\n"\
"		out_color = (texColor * vec4(color, Diffuse.w * alpha));\n"\
"	}\n"\
"}";

const char* mesh_frag_f = "#version 430\n\n"\
"#define SHADOW_MAX 12\n"\
"#define LIGHT_MAX 20\n"\
"uniform vec3 lightPos[LIGHT_MAX];\n"\
"uniform vec3 lightDir[LIGHT_MAX];\n"\
"uniform vec3 lightAmbientColor[LIGHT_MAX];\n"\
"uniform vec3 lightDiffuseColor[LIGHT_MAX];\n"\
"uniform float lightRange[LIGHT_MAX];\n"\
"uniform int lightType[LIGHT_MAX];\n"\
"uniform float lightRadiationAngle[LIGHT_MAX];\n"\
"uniform int lightUseShadow[LIGHT_MAX];\n"\
"uniform vec2 stscroll;\n"\
"uniform vec3 eyeVec;\n"\
"uniform vec3 eyePos;\n"\
"uniform vec4 Ambient;\n"\
"uniform vec4 Diffuse;\n"\
"uniform vec3 Specular;\n"\
"uniform vec3 Emissive;\n"\
"uniform float Pow;\n"\
"uniform float alpha;\n"\
"uniform int lighting;\n"\
"uniform int uSetex;\n"\
"uniform int usenormalMap;\n"\
"uniform vec4 fogColor;\n"\
"uniform float fogNear;\n"\
"uniform float fogFar;\n"\
"uniform int depth_tex_size;\n"\
"in vec4 vShadowCoord[SHADOW_MAX];    //!< シャドウデプスマップの参照用座標\n"\
"uniform sampler2D depth_tex[SHADOW_MAX];    //!< デプス値テクスチャ\n"\
"uniform float shadow_ambient;    //!< 影の濃さ\n"\
"uniform vec2 d_tex_scale[SHADOW_MAX];\n"\
"//頂点シェーダーから受け取る変数\n"\
"in vec4 V;//位置\n"\
"in vec3 N;//法線ベクトル\n"\
"in vec3 T;//接線ベクトル\n"\
"in vec3 B;//従法線ベクトル\n"\
"in vec2 texCoord;\n"\
"uniform sampler2D sampler;\n"\
"uniform sampler2D normalMap;//法線マップ\n"\
"out vec4 out_color[2];\n"\
"uniform float shadow_bias;\n"\
"uniform int toon;\n"\
"float restDepth(vec4 RGBA) {\n"\
"	const float rMask = 1.0;\n"\
"	const float gMask = 1.0 / 255.0;\n"\
"	const float bMask = 1.0 / (255.0 * 255.0);\n"\
"	const float aMask = 1.0 / (255.0 * 255.0 * 255.0);\n"\
"	float depth = dot(RGBA, vec4(rMask, gMask, bMask, aMask));\n"\
"	return depth;\n"\
"}\n"\
"bool shadowmap(int i,vec3 coord) {\n"\
"	if (texture2D(depth_tex[i], coord.xy).z  <  coord.z-shadow_bias "\
"		&& coord.x>0 && coord.x<1.0 && coord.y>0.0 && coord.y<1.0)\n"\
"	return true;\n"\
"	return false;\n"\
"}\n"\
"void main(void)\n"\
"{\n"\
"	vec4 texColor = vec4(1, 1, 1, 1);\n"\
"	if (uSetex != 0) {\n"\
"		texColor = texture2D(sampler, texCoord + stscroll);\n"\
"	}\n"\
"	if(texColor.a * alpha <=0) discard;\n"
"	vec3 D = vec3(0, 0, 0);\n"\
"	vec3 S = vec3(0, 0, 0);\n"\
"	vec3 color;\n"\
"	float visibility = 1.0;\n"\
"	vec3 E = normalize(eyePos - V.xyz);\n"\
"	vec3 Normal=N;\n"\
"	if (usenormalMap != 0) {\n"\
"		Normal = (texture2D(normalMap, texCoord + stscroll).xyz - 0.5) * 2.0;\n"\
"		Normal = T * Normal.x + B * Normal.y + N * Normal.z;\n"\
"		//Normal.xyz = (Normal / 2.0f) + 0.5f; //-1～1から0～1に補正\n"\
"		//Normal.w = texture2D(specurMap, texCoord + stscroll).r;\n"\
"		//Normal.w = 0;\n"\
"	}\n"\
"	if (lighting == 1) {\n"\
"		for (int i = 0; i < LIGHT_MAX; i++) {\n"\
"			if (lightType[i] == 0) continue;\n"\
"			vec3 vec = vec3(0, 0, 0);\n"\
"			float p = 0;\n"\
"			if (lightType[i] == 1) {\n"\
"				vec = -lightDir[i];\n"\
"				p = 1;\n"\
"			}else\n"\
"			if (lightType[i] == 2 || lightType[i] == 3) {\n"\
"					vec = vec3(V.xyz - lightPos[i]);\n"\
"					float l = length(vec);\n"\
"					vec = normalize(vec);\n"\
"					if (l > 0)\n"\
"						p = 1.0-clamp(pow(l / lightRange[i], 5), 0.0, 1.0);\n"\
"					else\n"\
"						p = 1.0;\n"\
"					if (lightType[i] == 3) {\n"\
"						float t = acos(dot(vec, lightDir[i]));\n"\
"						p *= 1.0-clamp(pow(t / lightRadiationAngle[i], 5), 0.0, 1.0); \n"\
"					}\n"\
"					vec = -vec;\n"\
"				}\n"\
"			vec3 L = vec; \n"\
"			float NL = max(0, dot(Normal, L)); \n"\
"			if (toon == 1)\n"\
"			//NL = (NL>0.0) ? ((NL>0.3) ? 1.0:0.95):0.80;\n"\
"			NL = (NL>0.0) ? 1.0:1-shadow_ambient;\n"\
"			vec3 R = reflect(-E, Normal);\n"\
"			S += pow(max(0, dot(R, L)), Pow) * p;\n"\
"			D += (lightDiffuseColor[i] * clamp(NL, 0.0, 1.0) + lightAmbientColor[i]) * p;\n"\
"			if(i==0){\n"\
"				for(int i=0;i<depth_tex_size;i++) {\n"\
"					vec3 coord = vShadowCoord[i].xyz / vShadowCoord[i].w;\n"\
"					if(coord.z<1.0)\n"\
"						for(int k=-1;k<=1;k++)\n"\
"							for(int j=-1;j<=1;j++)\n"\
"								if ( shadowmap(i,vec3(coord.xy + vec2(k,j)*d_tex_scale[i],coord.z)))\n"\
"									visibility -= shadow_ambient/9;\n"\
"				}\n"\
"			D=min(D,visibility);\n"\
"			}\n"\
"		}\n"\
"		float l = length(eyePos - V.xyz);\n"\
"		float f = clamp((fogFar - l) / (fogFar - fogNear), 0.0, 1.0);\n"\
"		color = texColor.xyz * Diffuse.xyz * D /*+ visibility * Specular * clamp(S, 0.0, 1.0) + Emissive*/;\n"\
"		out_color[0] = vec4(color + fogColor.xyz * (1.0 - f), clamp((texColor.w * Diffuse.w * alpha) - ((1.0 - fogColor.w) * (1.0 - f)), 0.0f, 1.0f));\n"\
"		out_color[1] = vec4(visibility * Specular * clamp(S, 0.0, 1.0)+ Emissive,1);\n"\
"	}\n"\
"	else {\n"\
"		color = Diffuse.xyz;\n"\
"		out_color[0] = (texColor * vec4(color, Diffuse.w * alpha));\n"\
"		out_color[1] = vec4(0,0,0,1);\n"\
"	}\n"\
"}";

const char* mesh_frag_d = "#version 430\n\n"\
"#define SHADOW_MAX 12\n"\
"uniform vec2 stscroll;\n"\
"uniform vec3 eyeVec;\n"\
"uniform vec3 eyePos;\n"\
"uniform vec4 Ambient;\n"\
"uniform vec4 Diffuse;\n"\
"uniform vec3 Specular;\n"\
"uniform vec3 Emissive;\n"\
"uniform float Pow;\n"\
"uniform float alpha;\n"\
"uniform int lighting;\n"\
"uniform int uSetex;\n"\
"uniform int usenormalMap;\n"\
"uniform vec4 fogColor;\n"\
"uniform float fogNear;\n"\
"uniform float fogFar;\n"\
"//頂点シェーダーから受け取る変数\n"\
"in vec4 V;//位置\n"\
"in vec3 N;//法線ベクトル\n"\
"in vec3 T;//接線ベクトル\n"\
"in vec3 B;//従法線ベクトル\n"\
"in vec2 texCoord;\n"\
"uniform sampler2D sampler;\n"\
"uniform sampler2D normalMap;//法線マップ\n"\
"uniform int depth_tex_size;\n"\
"in vec4 vShadowCoord[SHADOW_MAX];    //!< シャドウデプスマップの参照用座標\n"\
"uniform sampler2D depth_tex[SHADOW_MAX];    //!< デプス値テクスチャ\n"\
"uniform float shadow_ambient;    //!< 影の濃さ\n"\
"uniform vec2 d_tex_scale[SHADOW_MAX];\n"\
"uniform float shadow_bias;\n"\
"uniform int toon;\n"\
"out vec4 out_color[8];\n"\
"float restDepth(vec4 RGBA) {\n"\
"	const float rMask = 1.0;\n"\
"	const float gMask = 1.0 / 255.0;\n"\
"	const float bMask = 1.0 / (255.0 * 255.0);\n"\
"	const float aMask = 1.0 / (255.0 * 255.0 * 255.0);\n"\
"	float depth = dot(RGBA, vec4(rMask, gMask, bMask, aMask));\n"\
"	return depth;\n"\
"}\n"\
"bool shadowmap(int i,vec3 coord) {\n"\
"	if (texture2D(depth_tex[i], coord.xy).z  <  coord.z-shadow_bias "\
"		&& coord.x>0 && coord.x<1.0 && coord.y>0.0 && coord.y<1.0)\n"\
"	return true;\n"\
"	return false;\n"\
"}\n"\
"void main(void)\n"\
"{\n"\
"	vec4 texColor = vec4(1, 1, 1, 1);\n"\
"	if (uSetex != 0) {\n"\
"		texColor = texture2D(sampler, texCoord + stscroll);\n"\
"	}\n"\
"	if(texColor.a * alpha <=0) discard;\n"\
"	out_color[0] = (texColor * vec4(Diffuse.xyz, Diffuse.w * alpha));\n"\
"	if (lighting == 1) {\n"\
"		vec3 E = normalize(eyePos - V.xyz); \n"\
"		vec4 Normal = vec4(N,1.0); \n"\
"		vec4 ext = vec4(0,0,0,1); \n"\
"		if (usenormalMap != 0) {\n"\
"			Normal.xyz = (texture2D(normalMap, texCoord + stscroll).xyz - 0.5) * 2.0; \n"\
"			Normal.xyz = T * Normal.x + B * Normal.y + N * Normal.z; \n"\
"			//Normal.xyz = (Normal.xyz / 2.0f) + 0.5f; //-1～1から0～1に補正\n"\
"			//Normal.w = texture2D(specurMap, texCoord + stscroll).r;\n"\
"		}\n"\
"		if (toon == 1)\n"\
"			ext.z = 1-shadow_ambient;\n"\
"		else\n"\
"			ext.z = 1.0;\n"\
"		out_color[1] = Normal;"
"		out_color[2] = V;"
"		out_color[3].x =Emissive.x;\n"\
"		out_color[3].y = Specular.y;\n"\
"		out_color[3].z = ext.z;\n"\
"		out_color[3].w = 1;\n"\
"		for(int i=0;i<depth_tex_size;i++) {\n"\
"			vec3 coord = vShadowCoord[i].xyz / vShadowCoord[i].w;\n"\
/*シャドウマップ */
"			out_color[4+i/3][i%3] = 1.0f;\n"\
"			out_color[4+i/3].w = 1.0f;\n"\
"			if(coord.z<1.0)\n"\
"				for(int k=-1;k<=1;k++)\n"\
"					for(int j=-1;j<=1;j++)\n"\
"						if ( shadowmap(i,vec3(coord.xy + vec2(k,j)*d_tex_scale[i],coord.z)))\n"\
"							out_color[4+i/3][i%3] -= shadow_ambient/9;\n"\
"		}\n"\
"	} else {\n"\
"		out_color[1].w = 0;"
"	}\n"\
"}";

const char *solid_vert = "#version 430\n\n"\
"uniform mat4 PVWMatrix;\n"\
"layout(location = 0) in vec3	Vertex;\n"\
"void main(void)\n"\
"{\n"\
"	gl_Position = PVWMatrix * vec4(Vertex, 1);\n"\
"}";

const char* solid_frag = "#version 430\n\n"\
"uniform vec4 Color;\n"\
"out vec4 out_color;\n"\
"void main() {\n"\
"	out_color = Color;\n"\
"}";
const char* solid_frag_f = "#version 430\n\n"\
"uniform vec4 Color;\n"\
"out vec4 out_color[2];\n"\
"void main() {\n"\
"	out_color[0] = Color;\n"\
"	out_color[1] = vec4(0,0,0,Color.a);\n"\
"}";

const char* trail_vert ="#version 430\n"\
"uniform mat4 PVWMatrix;\n"\
"layout(location = 0) in vec3	Vertex;\n"\
"layout(location = 2) in vec2	TexCoord;\n"\
"layout(location = 5) in vec4	VertexColor;\n"\
"out vec2 texCoord;\n"\
"out vec4 vertex_color;\n"\
"void main(void)\n"\
"{\n"\
"	gl_Position = PVWMatrix * vec4(Vertex, 1);\n"\
"	vertex_color = VertexColor;\n"\
"	texCoord = TexCoord;\n"\
"}\n";

const char* trail_frag_d = "#version 430\n"\
"uniform sampler2D sampler;\n"\
"uniform vec3 Emissive;\n"\
"in vec2 texCoord;\n"\
"in vec4 vertex_color;\n"\
"out vec4 out_color[5];\n"\
"void main() {\n"\
"	vec4 texColor = vec4(1, 1, 1, 1);\n"\
"	texColor = texture2D(sampler, texCoord);\n"\
"	vec4 color = vertex_color * texColor;\n"\
"	out_color[0] = vec4(color.xyz*color.w + out_color[0].xyz*(1.0-color.w),color.w);\n"\
"	out_color[1].w = 0.0f;\n"\
"	out_color[3].x = Emissive.x;\n"\
"	out_color[3].y = 0.0;\n"\
"	out_color[3].z = 1.0;\n"\
"	out_color[3].w = 1.0;\n"\
"};\n";
const char* trail_frag_f= "#version 430\n"\
"uniform sampler2D sampler;\n"\
"uniform vec3 Emissive;\n"\
"in vec2 texCoord;\n"\
"in vec4 vertex_color;\n"\
"out vec4 out_color[2];\n"\
"void main() {\n"\
"	vec4 texColor = vec4(1, 1, 1, 1);\n"\
"	texColor = texture2D(sampler, texCoord);\n"\
"	vec4 color = vertex_color * texColor;\n"\
"	out_color[0] = vec4(color.xyz*color.w + out_color[0].xyz*(1.0-color.w),color.w);\n"\
"	out_color[1] = vec4(Emissive,1);\n"\
"};\n";

const char* trail_frag = "#version 430\n"\
"uniform sampler2D sampler;\n"\
"uniform vec3 Emissive;\n"\
"in vec2 texCoord;\n"\
"in vec4 vertex_color;\n"\
"out vec4 out_color;\n"\
"void main() {\n"\
"	vec4 texColor = vec4(1, 1, 1, 1);\n"\
"	texColor = texture2D(sampler, texCoord);\n"\
"	vec4 color = vertex_color * texColor;\n"\
"	out_color = vec4((color.xyz+Emissive.xyz)*color.w + out_color.xyz*(1.0-color.w),color.w);\n"\
"};\n";

const char* image_vert = "#version 430\n\n"\
"uniform mat4 PVWMatrix;\n"\
"layout(location = 0) in vec3	Vertex;\n"\
"layout(location = 2) in vec2	TexCoord;\n"\
"out vec2 texCoord;\n"\
"void main(void)\n"\
"{\n"\
"	gl_Position = PVWMatrix * vec4(Vertex, 1);\n"\
"	texCoord = TexCoord;\n"\
"}";
const char* image_frag = "#version 430\n\n"\
"uniform vec4 Color;\n"\
"uniform vec3 Emissive;\n"\
"in vec2 texCoord;\n"\
"uniform sampler2D sampler;\n"\
"out vec4 out_color;\n"\
"void main() {\n"\
"	vec4 texColor = vec4(1, 1, 1, 1);\n"\
"	texColor = texture2D(sampler, texCoord);\n"\
"	out_color = texColor * (Color+vec4(Emissive,0));\n"\
"}";
const char* image_frag_f = "#version 430\n\n"\
"uniform vec4 Color;\n"\
"uniform vec3 Emissive;\n"\
"in vec2 texCoord;\n"\
"uniform sampler2D sampler;\n"\
"out vec4 out_color[2];\n"\
"void main() {\n"\
"	vec4 texColor = vec4(1, 1, 1, 1);\n"\
"	texColor = texture2D(sampler, texCoord);\n"\
"	out_color[0] = texColor * Color;\n"\
"	out_color[1] = vec4(texColor.xyz * Emissive,texColor.a * Color.a);\n"\
"}";

const char* image_frag_d = "#version 430\n\n"\
"uniform vec4 Color;\n"\
"uniform vec3 Emissive;\n"\
"in vec2 texCoord;\n"\
"uniform sampler2D sampler;\n"\
"out vec4 out_color[5];\n"\
"void main() {\n"\
"	vec4 texColor = vec4(1, 1, 1, 1);\n"\
"	texColor = texture2D(sampler, texCoord);\n"\
"	out_color[0] = texColor * Color;\n"\
"	out_color[1].w = 0.0f;\n"\
"	out_color[3].x = Emissive.x;\n"\
"	out_color[3].y = 0.0;\n"\
"	out_color[3].z = 1.0;\n"\
"	out_color[3].w = 1.0;\n"\
"}";

const char* effect_vert = "#version 430\n"\
"layout(location = 0) in vec3 Vertex;\n"\
"out vec2 texCoord;\n"\
"void main(void)\n"\
"{\n"\
"	texCoord = Vertex.xy;\n"\
"	gl_Position = vec4(Vertex.xy * 2.0 - 1.0, 0.0, 1.0);\n"\
"}\n";


const char* effect_frag = "#version 430\n"\
"float rand(vec2 co) { \n"\
"	float t = fract(co.x * 43758.5453);\n"\
"	return t;\n"\
"}\n"\
"uniform sampler2D tDiffuse;\n"\
"uniform vec2 vScreenSize;\n"\
"in vec2 texCoord;\n"\
"out vec4 out_color;\n"\
"void main() {\n"\
"	float radius = 5.0;\n"\
"	float x = (texCoord.x * vScreenSize.x) + rand(vec2(texCoord.x, texCoord.y)) * radius * 2.0 - radius;\n"\
"	float y = (texCoord.y * vScreenSize.y) + rand(vec2(texCoord.y, texCoord.x)) * radius * 2.0 - radius;\n"\
"	vec4 textureColor = texture2D(tDiffuse, vec2(texCoord.x + rand(vec2(texCoord.x, texCoord.y)) * 0.01, texCoord.y + rand(vec2(texCoord.y, texCoord.x)) * 0.01));\n"\
"	out_color = textureColor;\n"\
"}\n";

const char* no_effect_frag = "#version 430\n"\
"uniform sampler2D tDiffuse;\n"\
"in vec2 texCoord;\n"\
"out vec4 out_color;\n"\
"void main() {\n"\
"	vec4 textureColor = texture2D(tDiffuse, vec2(texCoord.x, texCoord.y));\n"\
"	out_color = textureColor;\n"\
"}\n";


const char* edge_vert = "#version 430\n"\
"layout(location = 0) in vec3 Vertex;\n"\
"out vec2 texCoord;\n"\
"void main(void)\n"\
"{\n"\
"	texCoord = Vertex.xy;\n"\
"	gl_Position = vec4(Vertex.xy * 2.0 - 1.0, 0.0, 1.0);\n"\
"}\n";


const char* edge_frag = "#version 430\n"\
"uniform sampler2D depth;\n"\
"const float dx = 1.0f/1920.0f;\n"\
"const float dy = 1.0f/1080.0f;\n"\
"in vec2 texCoord;\n"\
"out vec4 out_color;\n"\
"float peek(const in float x, const in float y)\n"\
"{\n"\
"	return texture2D(depth, vec2(x, y)).r;\n"\
"}\n"\
"void main(void)\n"\
"{\n"\
"	float x = texCoord.x;\n"\
"	float y = texCoord.y;\n"\
"	mat3 m = mat3(\n"\
"		peek(x - dx, y - dy), peek(x, y - dy), peek(x + dx, y - dy),\n"\
"		peek(x - dx, y), peek(x, y), peek(x + dx, y),\n"\
"		peek(x - dx, y + dy), peek(x, y + dy), peek(x + dx, y + dy)\n"\
"	);\n"\
"	vec2 h = vec2(\n"\
"		m[0][0] - m[0][2] + (m[1][0] - m[1][2]) * 2.0 + m[2][0] - m[2][2],\n"\
"		m[0][0] - m[2][0] + (m[0][1] - m[2][1]) * 2.0 + m[0][2] - m[2][2]\n"\
"	);\n"\
"	float d = step(0.99, 1.0 - length(h));\n"\
"	out_color = vec4(vec3(0,0,0), 1.0 - d);\n"\
"}\n";



const char* gaussian_blur_vert = "#version 430\n"\
"layout(location = 0) in vec3 Vertex; \n"\
"out vec2 texCoord;\n"\
"void main(void) {\n"\
"	texCoord = Vertex.xy;\n"\
"		gl_Position = vec4(Vertex.xy * 2.0 - 1.0, 0.0, 1.0); \n"\
"}\n";


const char* gaussian_blur_frag = "#version 430\n"\
"uniform sampler2D texture;\n"\
"uniform float     weight[10];\n"\
"uniform int      horizontal;\n"\
"uniform vec2    scale;\n"\
"in vec2 texCoord;\n"\
"out vec4 out_color;\n"\
"void main(void) {\n"\
"	vec2  fc;\n"\
"	vec4  destColor = vec4(0.0);\n"\
"	if (horizontal == 1) {\n"\
"		float tFrag = scale.x;\n"\
"		fc = texCoord;\n"\
"		for (int i = 1; i <= 9; i++) {"\
"			destColor += texture2D(texture, fc + vec2(float(i),0.0) * tFrag) * weight[i]; \n"\
"			destColor += texture2D(texture, fc + vec2(-float(i),0.0) * tFrag) * weight[i]; \n"\
"		}\n"\
"		destColor += texture2D(texture, fc) * weight[0];\n"\
"	} else {\n"\
"		float tFrag = scale.y;\n"\
"		fc = texCoord;\n"\
"		for (int i = 1; i <= 9; i++) {"\
"			destColor += texture2D(texture, fc + vec2(0.0, float(i)) * tFrag) * weight[i]; \n"\
"			destColor += texture2D(texture, fc + vec2(0.0, -float(i)) * tFrag) * weight[i]; \n"\
"		}\n"\
"		destColor += texture2D(texture, fc) * weight[0];\n"\
"	}\n"\
"	out_color = destColor;\n"\
"}\n";



const char* depth_of_field_vert = "#version 430\n"\
"layout(location = 0) in vec3 Vertex; \n"\
"out vec2 texCoord;\n"\
"void main(void) {\n"\
"	texCoord = Vertex.xy;\n"\
"		gl_Position = vec4(Vertex.xy * 2.0 - 1.0, 0.0, 1.0); \n"\
"}\n";


const char* depth_of_field_frag = "#version 430\n"\
"uniform sampler2D depthTexture;\n"\
"uniform sampler2D sceneTexture;\n"\
"uniform sampler2D blurTexture1;\n"\
"uniform sampler2D blurTexture2;\n"\
"uniform vec3     offset;\n"\
"in vec2 texCoord;\n"\
"out vec4 out_color;\n"\
"float restDepth(vec4 RGBA) {\n"\
"	const float rMask = 1.0;\n"\
"	const float gMask = 1.0 / 255.0;\n"\
"	const float bMask = 1.0 / (255.0 * 255.0);\n"\
"	const float aMask = 1.0 / (255.0 * 255.0 * 255.0);\n"\
"	float depth = dot(RGBA, vec4(rMask, gMask, bMask, aMask));\n"\
"	return depth;\n"\
"}\n"\
"float convCoord(float depth, vec3 offset) {\n"\
"	float d = pow(clamp(depth-offset.z, 0.0, 1.0),0.1);\n"\
"	if (d > offset.y) {\n"\
"		d = 1.0/(1.0-offset.y) * (1.0 - d);\n"\
"	} else if (d >= offset.x) {\n"\
"		d = 1.0;\n"\
"	} else {\n"\
"		d = pow(d/offset.x,3);\n"\
"	}\n"\
"	return d;\n"\
"}\n"\
"void main(void) {\n"\
"	float d = restDepth(texture2D(depthTexture, texCoord));\n"\
"	d = convCoord(d, offset);\n"\
"	float coef = 1.0 - d;\n"\
"	float blur1Coef = coef * d;\n"\
"	float blur2Coef = coef * coef;\n"\
"	vec4 sceneColor = texture2D(sceneTexture, texCoord);\n"\
"	vec4 blur1Color = texture2D(blurTexture1, texCoord);\n"\
"	vec4 blur2Color = texture2D(blurTexture2, texCoord);\n"\
"	vec4 destColor = sceneColor * d + blur1Color * blur1Coef + blur2Color * blur2Coef;\n"\
"	out_color = destColor;\n"\
"}\n";

const char* glare_mix_vert = "#version 430\n"\
"layout(location = 0) in vec3 Vertex; \n"\
"out vec2 texCoord;\n"\
"void main(void) {\n"\
"	texCoord = Vertex.xy;\n"\
"		gl_Position = vec4(Vertex.xy * 2.0 - 1.0, 0.0, 1.0); \n"\
"}\n";


const char* glare_mix_frag = "#version 430\n"\
"uniform sampler2D texture1;\n"\
"uniform sampler2D texture2;\n"\
"in vec2 texCoord;\n"\
"out vec4 out_color;\n"\
"void main(void) {\n"\
"	vec4  destColor = texture2D(texture1, texCoord);\n"\
"	vec4  smpColor = texture2D(texture2, texCoord);\n"\
"	destColor += smpColor * 1.0;\n"\
"	out_color = destColor;\n"\
"}";
const char* lighting_vert = "#version 430\n"\
"layout(location = 0) in vec3 Vertex; \n"\
"out vec2 texCoord;\n"\
"void main(void) {\n"\
"	texCoord = Vertex.xy;\n"\
"		gl_Position = vec4(Vertex.xy * 2.0 - 1.0, 0.0, 1.0); \n"\
"}\n";


const char* lighting_frag = "#version 430\n"\
"#define SHADOW_SIZE 4\n"\
"uniform sampler2D color_texture;\n"\
"uniform sampler2D normal_texture;\n"\
"uniform sampler2D worldpos_texture;\n"\
"uniform sampler2D extra_texture;\n"\
"uniform sampler2D shadowmap_texture[SHADOW_SIZE];\n"\
"#define LIGHT_MAX 20\n"\
"uniform vec3 lightSPos[LIGHT_MAX];\n"\
"uniform vec3 lightPos[LIGHT_MAX];\n"\
"uniform vec3 lightDir[LIGHT_MAX];\n"\
"uniform vec3 lightAmbientColor[LIGHT_MAX];\n"\
"uniform vec3 lightDiffuseColor[LIGHT_MAX];\n"\
"uniform float lightRange[LIGHT_MAX];\n"\
"uniform int lightType[LIGHT_MAX];\n"\
"uniform float lightRadiationAngle[LIGHT_MAX];\n"\
"uniform int lightUseShadow[LIGHT_MAX];\n"\
"uniform vec3 eyeVec;\n"\
"uniform vec3 eyePos;\n"\
"in vec2 texCoord;\n"\
"out vec4 out_color[2];\n"\
"void main(void) {\n"\
"	vec4 BaseColor = texture2D(color_texture, texCoord);\n"\
"	vec3 NormalColor = texture2D(normal_texture, texCoord).xyz;\n"\
"	float isNormal = texture2D(normal_texture, texCoord).w;\n"\
"	float SpecPower = 5.0;\n"\
"	vec4 WorldPos = texture2D(worldpos_texture, texCoord);\n"\
"	vec4 excolor = texture2D(extra_texture, texCoord);\n"\
"	float emissive = excolor.x;\n"\
"	float metalic = excolor.y;\n"\
"	float roughNess = excolor.y;\n"\
"	float ext = excolor.z;\n"\
"	//NormalColor = (NormalColor * 2.0) - 1.0f;\n"\
"	vec3 diffuse = vec3(0, 0, 0);\n"\
"	float spec = 0;\n"\
"	vec3 EyeVec = normalize(eyePos - WorldPos.xyz);\n"\
"	//法線無しはライティングしない\n"\
"	if (isNormal == 0) {\n"\
"		out_color[0] = BaseColor;\n"\
"		out_color[1] = vec4(BaseColor.xyz, 0);\n"\
"	} else {\n"\
"		for (int i = 0; i < LIGHT_MAX; i++) {\n"\
"			if (lightType[i] == 0) continue;\n"\
"			vec3 L = vec3(0, 0, 0);\n"\
"			float p = 0;\n"\
"			if (lightType[i] == 1) {\n"\
"				L = -lightDir[i];\n"\
"				p = 1;\n"\
"			} else\n"\
"			if (lightType[i] == 2 || lightType[i] == 3) {\n"\
"					L = WorldPos.xyz - lightPos[i];\n"\
"					float l = length(L);\n"\
"					L = normalize(L);\n"\
"					if (l > 0)\n"\
"						p = 1.0-clamp(pow(l / lightRange[i], 5), 0.0, 1.0);\n"\
"					else\n"\
"						p = 1;\n"\
"					if (lightType[i] == 3) {\n"\
"						float t = acos(dot(L, lightDir[i]));\n"\
"						p *= 1.0-clamp(pow(t / lightRadiationAngle[i], 5), 0.0, 1.0); \n"\
"						//if(t<1) p = 0;\n"\
"						//p *= pow(t, 20);\n"\
"					}\n"\
"					L = -L;\n"\
"				}\n"\
"			float NL = dot(NormalColor, L);\n"\
"			vec3 R = reflect(-EyeVec, NormalColor);\n"\
"			spec += metalic * pow(max(0, dot(R, L)), SpecPower) * p;\n"\
"			if (ext < 1.0)\n"\
"				NL = NL > 0 ? 1.0 : 0.9;\n"\
"			else\n"\
"				NL = clamp(NL, 0.0, 1.0);\n"\
"			if (lightUseShadow[i] >= 0){\n"\
"				vec4 Shadow = texture2D(shadowmap_texture[lightUseShadow[i]/3], texCoord);\n"\
"				NL = min(NL, Shadow[lightUseShadow[i]%3]); \n"\
"			}\n"\
"			diffuse += lightDiffuseColor[i] * NL * p;\n"\
"			diffuse += lightAmbientColor[i].xyz * p;\n"\
"		}\n"\
"		out_color[0] = vec4(BaseColor.xyz * min(diffuse,1.0), 1/*BaseColor.w*/);\n"\
"		out_color[1] = vec4(BaseColor.xyz * spec + BaseColor.xyz * emissive, BaseColor.w);\n"\
"	}\n"\
"}\n";

const char* water_flag = "#version 430\n"\
"uniform sampler2D mirrorSampler;\n"\
"uniform float alpha = 0.4;\n"\
"uniform float time;\n"\
"uniform float size = 0.1;\n"\
"uniform float distortionScale = 1.0;\n"\
"uniform sampler2D normalSampler;\n"\
"uniform vec3 sunDirection = vec3(0, -0.7, 0.7);\n"\
"uniform vec3 eyePos;\n"\
"uniform vec3 waterColor = vec3(0.6, 0.6, 1.0);\n"\
"uniform vec3 lightAmbientColor;\n"\
"uniform vec3 lightDiffuseColor;\n"\
"in vec4 V;\n"\
"in vec3 N;\n"\
"in vec3 T;\n"\
"in vec3 B;\n"\
"in vec2 texCoord;\n"\
"//varying vec4 mirrorCoord;\n"\
"out vec4 out_color;\n"\
"vec4 getNoise(vec2 uv) {"\
"	vec2 uv0 = (uv / 103.0) + vec2(time / 17.0, time / 29.0);\n"\
"	vec2 uv1 = uv / 107.0 - vec2(time / -19.0, time / 31.0);\n"\
"	vec2 uv2 = uv / vec2(8907.0, 9803.0) + vec2(time / 101.0, time / 97.0);\n"\
"	vec2 uv3 = uv / vec2(1091.0, 1027.0) - vec2(time / 109.0, time / -113.0);\n"\
"	vec4 noise = texture2D(normalSampler, uv0) +"\
"		texture2D(normalSampler, uv1) +\n"\
"		texture2D(normalSampler, uv2) +\n"\
"		texture2D(normalSampler, uv3);\n"\
"	return noise * 0.5 - 1.0;\n"\
"}\n"\
"void sunLight(const vec3 surfaceNormal, const vec3 eyeDirection, float shiny, float spec, float diffuse, inout vec3 diffuseColor, inout vec3 specularColor) {\n"\
"	vec3 reflection = normalize(reflect(sunDirection, surfaceNormal));\n"\
"	float direction = max(0.0, dot(eyeDirection, reflection));\n"\
"	specularColor += pow(direction, shiny) * lightDiffuseColor * spec;\n"\
"	diffuseColor += max(dot(sunDirection, surfaceNormal), 0.0) * lightDiffuseColor * diffuse;\n"\
"}\n"\
"void main() {\n"\
"	vec4 noise = getNoise(V.xz * size);\n"\
"	vec3 surfaceNormal = normalize(noise.xzy * vec3(1.5, 1.0, 1.5));\n"\
"	vec3 diffuseLight = lightDiffuseColor;\n"\
"	vec3 specularLight = vec3(0.0);\n"\
"	vec3 worldToEye = eyePos - V.xyz;\n"\
"	vec3 eyeDirection = normalize(worldToEye);\n"\
"	sunLight(surfaceNormal, eyeDirection, 10.0, 0.4, 0.6, diffuseLight, specularLight);\n"\
"	vec3 scatter = max(0.0, dot(surfaceNormal, eyeDirection)) * waterColor;\n"\
"	//float s = getShadowMask();\n"\
"	float s = 1.0f;\n"\
"	vec3 albedo = ((scatter + lightAmbientColor * waterColor) * s);\n"\
"	vec3 outgoingLight = albedo;\n"\
"	out_color = vec4(outgoingLight + specularLight, alpha);\n"\
"}";

const char* water_flag_f = "#version 430\n"\
"uniform sampler2D mirrorSampler;\n"\
"uniform float alpha = 0.4;\n"\
"uniform float time;\n"\
"uniform float size = 0.1;\n"\
"uniform float distortionScale = 1.0;\n"\
"uniform sampler2D normalSampler;\n"\
"uniform vec3 sunDirection = vec3(0, -0.7, 0.7);\n"\
"uniform vec3 eyePos;\n"\
"uniform vec3 waterColor = vec3(0.6, 0.6, 1.0);\n"\
"uniform vec3 lightAmbientColor;\n"\
"uniform vec3 lightDiffuseColor;\n"\
"in vec4 V;\n"\
"in vec3 N;\n"\
"in vec3 T;\n"\
"in vec3 B;\n"\
"in vec2 texCoord;\n"\
"//varying vec4 mirrorCoord;\n"\
"out vec4 out_color[2];\n"\
"vec4 getNoise(vec2 uv) {"\
"	vec2 uv0 = (uv / 103.0) + vec2(time / 17.0, time / 29.0);\n"\
"	vec2 uv1 = uv / 107.0 - vec2(time / -19.0, time / 31.0);\n"\
"	vec2 uv2 = uv / vec2(8907.0, 9803.0) + vec2(time / 101.0, time / 97.0);\n"\
"	vec2 uv3 = uv / vec2(1091.0, 1027.0) - vec2(time / 109.0, time / -113.0);\n"\
"	vec4 noise = texture2D(normalSampler, uv0) +"\
"		texture2D(normalSampler, uv1) +"\
"		texture2D(normalSampler, uv2) +"\
"		texture2D(normalSampler, uv3);\n"\
"	return noise * 0.5 - 1.0;\n"\
"}\n"\
"void sunLight(const vec3 surfaceNormal, const vec3 eyeDirection, float shiny, float spec, float diffuse, inout vec3 diffuseColor, inout vec3 specularColor) {"\
"	vec3 reflection = normalize(reflect(sunDirection, surfaceNormal));\n"\
"	float direction = max(0.0, dot(eyeDirection, reflection));\n"\
"	specularColor += pow(direction, shiny) * lightDiffuseColor * spec;\n"\
"	diffuseColor += max(dot(sunDirection, surfaceNormal), 0.0) * lightDiffuseColor * diffuse;\n"\
"}\n"\
"void main() {\n"\
"	vec4 noise = getNoise(V.xz * size);\n"\
"	vec3 surfaceNormal = normalize(noise.xzy * vec3(1.5, 1.0, 1.5));\n"\
"	vec3 diffuseLight = lightDiffuseColor;\n"\
"	vec3 specularLight = vec3(0.0);\n"\
"	vec3 worldToEye = eyePos - V.xyz;\n"\
"	vec3 eyeDirection = normalize(worldToEye);\n"\
"	sunLight(surfaceNormal, eyeDirection, 10.0, 0.4, 0.6, diffuseLight, specularLight);\n"\
"	vec3 scatter = max(0.0, dot(surfaceNormal, eyeDirection)) * waterColor;\n"\
"	//float s = getShadowMask();\n"\
"	float s = 1.0f;\n"\
"	vec3 albedo = ((scatter + lightAmbientColor * waterColor) * s);\n"\
"	vec3 outgoingLight = albedo;\n"\
"	out_color[0] = vec4(outgoingLight, alpha);\n"\
"	out_color[1] = vec4(specularLight, 1);\n"\
"}";

std::map<std::string, std::map<int,CShader*>> CShader::m_instances;
CShader* CShader::GetInstance(std::string type)
{
	if (!m_instances[type][(int)CRendaring::GetType()]) m_instances[type][(int)CRendaring::GetType()] = new CShader(type);
	return m_instances[type][(int)CRendaring::GetType()];
}
void CShader::ClearInstance()
{
	for (auto& s : m_instances)
		for (auto& ss : s.second) {
			if (ss.second)
				delete ss.second;
		}
	m_instances.clear();
}
CShader* CShader::CreateInstance(std::string type)
{
	return new CShader(type);
}
CShader::CShader():program(0){

}
CShader::CShader(std::string type) {
	static std::map<std::string, std::vector<std::vector<const char*>>> list = {
		{
			"SkinMesh",
			{
				{skin_mesh_vert,mesh_frag},
				{skin_mesh_vert,mesh_frag_f},
				{skin_mesh_vert,mesh_frag_d},
			}
		},
		{
			"StaticMesh",
			{
				{mesh_vert,mesh_frag},
				{mesh_vert,mesh_frag_f},
				{mesh_vert,mesh_frag_d},
			}
		},
		{
			"Edge",
			{
				{edge_vert,edge_frag},
				{edge_vert,edge_frag},
				{edge_vert,edge_frag},
			}
		},
		{
			"Image",
			{
				{image_vert,image_frag},
				{image_vert,image_frag_f},
				{image_vert,image_frag_d},
			}
		},
		{
			"Solid",
			{
				{solid_vert,solid_frag},
				{solid_vert,solid_frag_f},
				{solid_vert,solid_frag},
			}
		},
		{
			"Trail",
			{
				{trail_vert,trail_frag},
				{trail_vert,trail_frag_f},
				{trail_vert,trail_frag_d},
			}
		},
		{
			"GaussianBlur",
			{
				{gaussian_blur_vert,gaussian_blur_frag},
				{gaussian_blur_vert,gaussian_blur_frag},
				{gaussian_blur_vert,gaussian_blur_frag},
			}
		},
		{
			"DepthOfField",
			{
				{depth_of_field_vert,depth_of_field_frag},
				{depth_of_field_vert,depth_of_field_frag},
				{depth_of_field_vert,depth_of_field_frag},
			}
		},
		{
			"GlareMix",
			{
				{glare_mix_vert,glare_mix_frag},
				{glare_mix_vert,glare_mix_frag},
				{glare_mix_vert,glare_mix_frag},
			}
		},
		{
			"Lighting",
			{
				{lighting_vert,lighting_frag},
				{lighting_vert,lighting_frag},
				{lighting_vert,lighting_frag},
			}
		},{
			"Water",
			{
				{mesh_vert,water_flag},
				{mesh_vert,water_flag_f},
				{mesh_vert,water_flag_f},
			}
		},
	};
	if (list[type].size() == 0) {

		MessageBox(GL::hWnd, type.c_str(), "シェーダ無し", MB_OK);
		return;
	}
	ReadCode(list[type][(int)CRendaring::GetType()][0], list[type][(int)CRendaring::GetType()][1]);


}
CShader::CShader(const char* vertexPath,const char* pixelPath){
	Load(vertexPath,pixelPath);
}

bool CShader::ReadCode(GLuint shader, const char* code) {
	GLint compiled;
	int size = strlen(code);
	char* str = new char[size + 1];
	strcpy_s(str, size + 1,code);
	if (char* p = strstr(str, "LIGHT_MAX")) {
		char* end = strchr(p, '\n')-1;
		int lm = CLight::LIGHT_MAX;
		while (isdigit(*end)) {
			*end = (lm % 10)+'0';
			lm /= 10;
			end--;
		}
	}

	glShaderSource(shader, 1, (const GLchar * *)& str, &size);
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled) {
		GLint length;
		GLchar* log;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
		if (length > 0) {
			log = new GLchar[length];
			glGetShaderInfoLog(shader, length, &length, log);
			MessageBox(GL::hWnd, log, "エラー", MB_OK);
			delete log;
		}
	}
	delete[] str;

	glAttachShader(program, shader);
	glLinkProgram(program);

	glGetProgramiv(program, GL_LINK_STATUS, &linked);

	if (linked) {
		//	glUseProgram(program);
	}
	else {
		GLint length;
		GLchar* log;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
		if (length > 0) {
			log = new GLchar[length];
			glGetShaderInfoLog(shader, length, &length, log);
			fprintf(stderr, "compile log='%s'\n", log);
			delete log;
		}
	}


	glDeleteShader(shader);
	//glDeleteShader(fragShader);

	return true;

}
bool CShader::Load(GLuint shader, const char *file) {
	FILE *fp;
	fopen_s(&fp,file, "rb");
	if(!fp) {
		printf("ファイルが開けません[%s]\n",file);
		return false;
	}
	fseek( fp, 0, SEEK_END );
	int size = ftell( fp );
	fseek( fp, 0, SEEK_SET );
	GLchar *code = new char[size+1];
	fread(code,size,1,fp);
	fclose(fp);
	code[size] = 0;
	bool r = ReadCode(shader, code);
	delete code;
	return r;
 
}
bool CShader::ReadCode(const char* vertexcode, const char* fragcode) {

	program = glCreateProgram();
	if (vertexcode) {

		bool ret = ReadCode(glCreateShader(GL_VERTEX_SHADER), vertexcode);
		if (!ret) return false;
	}
	if (fragcode) {

		bool ret = ReadCode(glCreateShader(GL_FRAGMENT_SHADER), fragcode);
		if (!ret) return false;
	}


	return true;

}
bool CShader::Load(const char* vertexPath,const char* fragPath){

	program = glCreateProgram();
	if(vertexPath) {

		bool ret = Load(glCreateShader(GL_VERTEX_SHADER),vertexPath);
		if(!ret) return false;
	}
	if(fragPath) {
		
		bool ret = Load(glCreateShader(GL_FRAGMENT_SHADER),fragPath);
		if(!ret) return false;
	}
	

	return true;
	
}
void CShader::Add(std::string type, int render_type, CShader* shader)
{
	m_instances[type][render_type] = shader;
}
bool CShader::Load(const char* path, GLuint type){
	if (!program) program = glCreateProgram();
	if (path) {
		bool ret = Load(glCreateShader(type), path);
		if (!ret) return false;
	}
	return true;
}
bool CShader::ReadCode(const char* path, GLuint type) {
	if (!program) program = glCreateProgram();
	if (path) {
		bool ret = ReadCode(glCreateShader(type), path);
		if (!ret) return false;
	}
	return true;
}
CShader::~CShader(){
	if (program>0) glDeleteProgram(program);

}
void CShader::ReadVertexShader(std::string type)
{
	if (type == "SkinMesh") {
		ReadCode(skin_mesh_vert, GL_VERTEX_SHADER);
	} else
	if (type == "StaticMesh") {
		ReadCode(mesh_vert, GL_VERTEX_SHADER);
	} else
	if (type == "Effect") {
	//	load("shader\\effect.vert", "shader\\effect.frag");
	}else
	if (type == "NoEffect") {
	//	load("shader\\effect.vert", "shader\\noeffect.frag");
	}else
	if (type == "Depth") {
	//	load("shader\\showdepth.vert", "shader\\showdepth.frag");
	}else
	if (type == "Image") {
		ReadCode(image_vert, GL_VERTEX_SHADER);
	}else
	if (type == "Solid") {
		ReadCode(solid_vert, GL_VERTEX_SHADER);
	}else
	if (type == "Trail") {
		ReadCode(trail_vert, GL_VERTEX_SHADER);
	//	load("shader\\trail.vert", "shader\\trail.frag");
	}
	else {
		printf("\nシェーダー無し\n");
	}

}
void CShader::Enable(){
	glUseProgram(program);
}
void CShader::Disable(){
	glUseProgram(0);
}