#pragma once

//#define USE_ATHOR_MODEL
#define USE_VIDEO_LIB
//óvÅFavcodecåndll
//#define USE_PHYSX_LIB
//óvÅFPhysX_64åndll

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif

#include <assert.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <stdlib.h>
#include <math.h>
#include <windows.h>
#include <algorithm>

#include "GLLibrary/GL.h"


#include "GLLibrary/CMatrix.h"

#include "GLLibrary/CTexture.h"
#include "GLLibrary/CVector.h"
#include "GLLibrary/CLight.h"
#include "GLLibrary/CCamera.h"
#include "GLLibrary/CSound.h"
#include "GLLibrary/CInput.h"
#include "GLLibrary/CImage.h"
#include "GLLibrary/CCollision.h"
#include "GLLibrary/CFPS.h"
#include "GLLibrary/CFont.h"
#include "GLLibrary/Utility.h"
#include "GLLibrary/CModel.h"
#include "GLLibrary/CModelObj.h"
#include "GLLibrary/CModelField.h"
#ifdef USE_ATHOR_MODEL

#include "GLLibrary/CModelFBX.h"
#include "GLLibrary/CModelX.h"

#endif
#include "GLLibrary/CModelA3M.h"
#include "GLLibrary/CShadow.h"
#include "GLLibrary/CResource.h"
#include "GLLibrary/CNetWork.h"
#include "GLLibrary/CMultiThread.h"
#include "GLLibrary/CTextureFrame.h"
#include "GLLibrary/CRendaring.h"
#ifdef USE_VIDEO_LIB
#include "GLLibrary/CVideo.h"
#endif
#ifdef USE_PHYSX_LIB
#include "GLLibrary/CPhysX.h"
#endif
