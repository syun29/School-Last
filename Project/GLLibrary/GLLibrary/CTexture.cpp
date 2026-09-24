
#include "GL.h"
#include "glpng.h"
#include "CTexture.h"
//#include "BmpFormat.h"
//#include "png.h"


//SDLインクルード（PNGファイル入力）
//#include "SDL_image.h"


CTexture::CTexture() : m_data(NULL),m_bufID(0), m_wrap(GL_REPEAT), m_filter(GL_LINEAR), m_filter_min(GL_LINEAR), m_type(GL_UNSIGNED_BYTE) {
}
CTexture::CTexture(int width, int height, GLenum internal_format, GLenum format, GLenum type) : CTexture() {
	m_type = type;
	m_width = width;
	m_height = height;
	m_wrap = GL_WRAP_BORDER;
	glGenTextures(1, &m_bufID);
	glBindTexture(GL_TEXTURE_2D, m_bufID);
	glTexImage2D(GL_TEXTURE_2D, 0, internal_format, m_width, m_height, 0,
		format, type, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_wrap);
	glBindTexture(GL_TEXTURE_2D, 0);

}
CTexture::~CTexture() {
}

bool CTexture::loadTiff(const char *path) {
	struct Header {
		unsigned short	oder;
		unsigned short	version;
		unsigned long	IFDOffset;
	};
	struct IFD {
		unsigned short tag;
		unsigned short type;
		unsigned long count;
		unsigned long data;
	};
	FILE *fp;
	fopen_s(&fp, path, "rb");
	if (!fp) {
		printf("%sがありません", path);
		return false;
	}
	Header header;
	fread(&header, sizeof(Header), 1, fp);
	fseek(fp, header.IFDOffset, SEEK_SET);
	unsigned short IFDCount;
	fread(&IFDCount, sizeof(unsigned short), 1, fp);
	IFD stripOffsets,stripByteCounts,bitsPerSample;
	for (int i = 0; i < IFDCount; i++) {
		IFD ifd;
		fread(&ifd, sizeof(IFD), 1, fp);
		switch (ifd.tag) {
		//ImageWidth	画像の幅
		case 0x100:
			if (ifd.type == 2) ifd.data >>= 8;
			m_width = ifd.data;
			break;
		//ImageLength	画像の高さ
		case 0x101:
			if (ifd.type == 2) ifd.data >>= 8;
			m_height = ifd.data;
			break;
		//BitsPerSample	色の深さ
		case 0x102:
			if (ifd.count != 3 && ifd.count != 4) {
				printf("RGB(A)カラーのみ対応");
			}
			bitsPerSample = ifd;
			break;
		//Compression	画像の圧縮方法
		case 0x103:
			if(ifd.data!=1) {
				printf("非圧縮のみ対応");
			}
			break;
		//PhotometricInterpretation	 画素データの最大値・最小値の設定
		case 0x106:
			ifd.data >>= 8;
			if (ifd.data != 2) {
				printf("ダイレクトカラーのみ対応");

			}
			break;
		//StripOffsets	ストリップ(画素データの切れ端)の開始位置
		case 0x111:
			if (ifd.count) {
				printf("ストリップ１つのみ対応");
			}
			if (ifd.type == 2) ifd.data >>= 8;
			stripOffsets = ifd;
			break;
		//RowsPerStrip	ストリップに含まれる行数
		case 0x116:
			break;
		//StripByteCounts	ストリップのバイト数
		case 0x117:
			if (ifd.type == 2) ifd.data >>= 8;
			stripByteCounts = ifd;
			break;
		//XResolusion	幅方向の解像度
		case 0x11A:
			break;
		//YResolusion	高さ方向の解像度
		case 0x11B:
			break;
		//ResolusionUnit	解像度における単位の設定
		case 0x128:
			break;
		//ColorMap	カラーマップ表示を選択した際のカラーマップの設定
		case 0x140:
			break;
		}

	}

	//8bit*3カラーで決め打ち

	m_data = new unsigned char[stripByteCounts.data];

	fseek(fp, stripOffsets.data, SEEK_SET);
	fread(m_data, stripByteCounts.data,1, fp);

	if (bitsPerSample.count == 3) {
		m_format = GL_RGB;
		m_internalFormat = 3;
	} else {
		m_format = GL_RGBA;
		m_internalFormat = 4;
	}
	return true;
}
bool CTexture::loadBmp(const char *path){
	FILE *fp;
	fopen_s(&fp,path,"rb");
	if (!fp) {
		printf("%sがありません", path);
		return false;
	}
	BITMAPFILEHEADER header;
	BITMAPINFOHEADER info;
	fseek(fp,0,SEEK_SET);
	fread(&header,sizeof(BITMAPFILEHEADER),1,fp);
	
	fread(&info,sizeof(BITMAPINFOHEADER),1,fp);
	
	m_width = info.biWidth;
	m_height = info.biHeight;

	int w = (m_width*3);
	w= (w%4==0) ? w : (w/4)*4+1;

	//imgSize = info.biSizeImage;
	m_imgSize = header.bfSize - header.bfOffBits;
	fseek(fp, header.bfOffBits, SEEK_SET);	m_data = new unsigned char[m_imgSize+4];
	for(int i=0;i<m_height;i++) {
		fread(&m_data[(m_height-i-1)*m_width*3],w,1,fp);

	}
	
	//fread(data,info.biSizeImage,1,fp);

	fclose(fp);

	m_internalFormat = 3;
	if(info.biBitCount==24) {
		m_format = GL_BGR_EXT;
	}else {
		printf("[%s]24bitフォーマットではありません\n",path);
		delete[] m_data;
		return false;
	}
	m_type = GL_UNSIGNED_BYTE;
	
	

	return true;
}

bool CTexture::loadPng(const char *path){
	pngRawInfo info;
	
//	m_bufID = pngBind (path, PNG_NOMIPMAP, PNG_ALPHA, &info, GL_REPEAT, GL_NEAREST, GL_NEAREST);
//	m_bufID = pngBind (path, PNG_NOMIPMAP, PNG_ALPHA, &info, GL_REPEAT, GL_LINEAR, GL_LINEAR);
	if (!pngLoadRaw(path,&info)) {
		printf("%sがありません", path);
		return false;
	}
	m_width = info.Width;
	m_height = info.Height;
	m_data = info.Data;
	m_internalFormat = info.Components;
	if (m_internalFormat == 3) {
		if (info.Depth == 16)
			m_internalFormat = GL_RGB16;
		else
			m_internalFormat = GL_RGB;
		m_format = GL_RGB;
	} else {
		if (info.Depth == 16)
			m_internalFormat = GL_RGBA16;
		else
			m_internalFormat = GL_RGBA;

		m_format = GL_RGBA;

	}
	if(info.Depth==16) 
		m_type = GL_UNSIGNED_SHORT;
	else
		m_type = GL_UNSIGNED_BYTE;

	return true;

}

CTexture* CTexture::LoadTexture(const char *path) {
	CTexture *t = new CTexture();
	if (t->Load(path)) return t;
	delete t;
	return nullptr;
}
bool CTexture::Load(const char *path){

	char *c = strrchr((char*)path, '.') + 1;
	m_filter_min = GL_LINEAR_MIPMAP_LINEAR;
	if (strcmp(c, "bmp") == 0) {
		return loadBmp(path);
	}else
	if (strcmp(c, "tiff") == 0) {
		return loadTiff(path);
	}else
	/*if (strcmp(c, "png") == 0) */{
		return loadPng(path);
	}/* else {
		printf("bmp png tiff以外は非対応");
	}*/
	return false;
	
}
void CTexture::MapTexture() {

	glEnable(GL_TEXTURE_2D);
	if (m_bufID == 0) {
		if (!m_data) return;
		glGenTextures(1, &m_bufID);
		glBindTexture(GL_TEXTURE_2D, m_bufID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_filter_min);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_wrap);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_wrap);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 7);
		glTexImage2D(GL_TEXTURE_2D, 0, m_internalFormat, m_width, m_height, 0, m_format, m_type, m_data);
		//glTexStorage2D(GL_TEXTURE_2D, 8, m_internalFormat, m_width, m_height);
		//glTexSubImage2D(GL_TEXTURE_2D, 0, 0,0, m_width, m_height, m_format, m_type, m_data);
		glGenerateMipmap(GL_TEXTURE_2D);

		delete[] m_data;
		m_data = nullptr;
	}
	//glRasterPos2f(-1.0f,-1.0f);
	glBindTexture(GL_TEXTURE_2D,m_bufID);
}

void CTexture::UnmapTexture() {
	if(m_bufID==0) return;
	glBindTexture(GL_TEXTURE_2D,0);
	glDisable(GL_TEXTURE_2D);
}
void CTexture::Draw( void )
{
	glRasterPos2f(0.0f,0.0f);
	glDrawPixels(m_width,m_height,m_format,GL_UNSIGNED_BYTE,m_data);
};

void CTexture::Release() {
	if (m_bufID) {
		glDeleteTextures(1, &m_bufID);
		m_bufID = 0;
	}
	if (m_data) {
		delete[] m_data;
		m_data = nullptr;
	}
}

void CTexture::SetWrapST(int wrapst) {
	m_wrap = wrapst;
	if (m_bufID) {
		glBindTexture(GL_TEXTURE_2D, m_bufID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapst);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapst);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

}
void CTexture::SetFilter(int filter) {
	m_filter = filter;
	if (m_bufID) {
		glBindTexture(GL_TEXTURE_2D, m_bufID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}