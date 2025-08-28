#include "stdafx.h"

#ifdef DEBUG
	ECORE_API BOOL bDebug	= FALSE;
#endif

// Video
float		psBaseWidth			= 1024.f;
u32			psCurrentVidMode[2] = { 0, 0 };
u32			psCurrentBPP		= 32;
// release version always has "mt_*" enabled
Flags32		psDeviceFlags		= {rsDetails|mtPhysics|mtSound|mtNetwork|rsDrawStatic|rsDrawDynamic};
u32			psScreenMode		= 1;

// textures 
int			psTextureLOD		= 0;
