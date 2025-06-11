#pragma once

#include "../xr_3da/effectorPP.h"
#include "../xr_3da/effector.h"
#include "../xr_3da/CameraManager.h"

struct SShootingEffector {
	SPPInfo	ppi;
	float	time;
	float	time_attack;
	float	time_release;
};

class CShootingHitEffectorPP : public CEffectorPP {
	typedef CEffectorPP inherited;	
	const SShootingEffector*	se;
public:
					CShootingHitEffectorPP	(SShootingEffector* eff);
	virtual	BOOL	Process					(SPPInfo& pp);
};
