#ifndef SLOAD_H
#define SLOAD_H

#include "common.h"


//////////////////////////////////////////////////////////////////////////////////////////
// Bumped surface loader                //
//////////////////////////////////////////////////////////////////////////////////////////
struct        surface_bumped
{
        half4         	base;
        half3         	normal;
        half         	gloss;
        half         	height;

};

float4   tbase                 (float2 tc)        {
        return                 tex2D                (s_base,         tc);
}
#if defined(ALLOW_STEEPPARALLAX) && defined(USE_STEEPPARALLAX)
surface_bumped                sload_i         ( p_bumped I)
{
        surface_bumped      S;
        const int maxSamples = 25;
  		const int minSamples = 5;
		float fParallaxOffset = -0.013;
 		int nNumSteps = lerp( maxSamples, minSamples, normalize(I.eye).z );
 		float fStep = 1.0 / nNumSteps;
  		float2 vDelta = normalize(I.eye).xy * fParallaxOffset*1.2;
   		float fStepSize   = 1.0 / nNumSteps;
 	 	float fPrevHeight = 1.0;
   		int    nStepIndex = 0;
   		float2 vTexOffsetPerStep = fStepSize * vDelta;
   		double2 vTexCurrentOffset = I.tcdh;
   		float fCurrHeight = 0;
   		float  fCurrentBound     = 1.0;
   		float  fParallaxAmount   = 0.0;
   		float fDelta2;
   		float fDelta1;
   		float2 vParallaxOffset;
   		float2 vTexCoord;
   		for(;fCurrHeight < fCurrentBound;  fCurrentBound -= fStepSize){
        vTexCurrentOffset += vTexOffsetPerStep;
        fCurrHeight = tex2Dlod( s_base, float4(vTexCurrentOffset.xy,0,0) ).a; }
   		vTexCurrentOffset -= vTexOffsetPerStep;
   		fPrevHeight = tex2Dlod( s_base, float4(vTexCurrentOffset.xy,0,0) ).a;
   		fDelta2 = ((fCurrentBound + fStepSize) - fPrevHeight);
   		fDelta1 = (fCurrentBound - fCurrHeight);
   		fParallaxAmount = (fCurrentBound * fDelta2 - (fCurrentBound + fStepSize) * fDelta1 ) / ( fDelta2 - fDelta1 );
#if BLOOM_MODE == 0
		const float fParallaxStartFade = 8.0f;
		const float fParallaxStopFade = 12.0f;
		float	fParallaxFade 	= smoothstep(fParallaxStopFade, fParallaxStartFade, I.position.z);
		vParallaxOffset = vDelta * ((1- fParallaxAmount )*fParallaxFade);
#else
   		vParallaxOffset = vDelta * (1- fParallaxAmount );
#endif
   		vTexCoord = I.tcdh + vParallaxOffset;
        float4       Nu      =       tex2D		(s_bump,         vTexCoord);                // IN:  normal.gloss
        float4       NuE     =       tex2D      	(s_bumpX,       vTexCoord);                // IN:         normal_error.height
        S.base              =       tbase                (vTexCoord);                                // IN:  rgb.a
        S.normal            = 		Nu.wzyx + (NuE.xyz - 1.0h);							 // (Nu.wzyx - .5h) + (E-.5)
//        S.normal            = 		half3(1,0,0);
#if BLOOM_MODE > 2
		S.gloss				= 		Nu.x		;
#else
        S.gloss             =       Nu.x*Nu.x	;                                        //        S.gloss             =        Nu.x*Nu.x;
#endif
        S.height            =       NuE.z       ;

#ifdef 		  USE_TDETAIL
#ifdef        USE_TDETAIL_BUMP
	half4 NDetail		= tex2D( s_detailBump, vTexCoord * dt_params);
	half4 NDetailX		= tex2D( s_detailBumpX, vTexCoord * dt_params);
#if BLOOM_MODE > 6
	S.gloss 			+= NDetail.x;
#else
	S.gloss				= S.gloss * NDetail.x * 2;
#endif
	//S.normal			+= NDetail.wzy-.5;
	S.normal			+= NDetail.wzy + NDetailX.xyz - 1.0h; //	(Nu.wzyx - .5h) + (E-.5)

	half4 detail		= tex2D( s_detail, vTexCoord * dt_params);
	S.base.rgb			= S.base.rgb * detail.rgb * 2;

//	S.base.rgb			= float3(1,0,0);
#else        //	USE_TDETAIL_BUMP
#if BLOOM_MODE > 2
		float4 		 detail  = 		tex2D(s_bumpD,vTexCoord * dt_params).wzyx;
	    S.normal			= 		Nu.wzyx + (NuE.xyz - (1.0h + 0.5h*def_dbumph)) + detail.xyz * def_dbumph ;
#if BLOOM_MODE > 6
		S.gloss 			+= 		detail.w							;
#else
        S.gloss             =  		S.gloss * detail.w * 2				;
#endif
#else
        float4       detail  =		tex2D(s_detail,vTexCoord * dt_params )        	;
        S.base.rgb          =		S.base.rgb     * detail.rgb*2		;
        S.gloss             =  		S.gloss * detail.w * 2				;
#endif        //	USE_TDETAIL_BUMP
#endif
		#endif

        return                S;
}
#elif	defined(USE_PARALLAX) || defined(USE_STEEPPARALLAX)
surface_bumped                sload_i         ( p_bumped I)        // + texld, mad, nrm(3), mad  = 1+1+3+1 = 6, 15+6=21, OK
{

        surface_bumped      S;
        half        height	=       tex2D      (s_bumpX, I.tcdh).w;                                //
                    height  =       height*(parallax.x) + (parallax.y);                        //
        float2		new_tc  =       I.tcdh + height*normalize        (I.eye);                //
        half4       Nu      =       tex2D		(s_bump,         new_tc);                // IN:  normal.gloss
        half4       NuE     =       tex2D      	(s_bumpX,       new_tc);                // IN:         normal_error.height
        S.base              =       tbase                (new_tc);                                // IN:  rgb.a
        S.normal            = 		Nu.wzyx + (NuE.xyz - 1.0h);							 // (Nu.wzyx - .5h) + (E-.5)
//        S.normal            = 		half3(1,0,0);
#if BLOOM_MODE > 2
		S.gloss				= 		Nu.x		;
#else
        S.gloss             =       Nu.x*Nu.x	;                                        //        S.gloss             =        Nu.x*Nu.x;
#endif
        S.height            =       NuE.z       ;


#ifdef        USE_TDETAIL
#ifdef        USE_TDETAIL_BUMP
	half4 NDetail		= tex2D( s_detailBump, I.tcdbump);
	half4 NDetailX		= tex2D( s_detailBumpX, I.tcdbump);
#if BLOOM_MODE > 6
	S.gloss 			+= NDetail.x;
#else
	S.gloss				= S.gloss * NDetail.x * 2;
#endif
	//S.normal			+= NDetail.wzy-.5;
	S.normal			+= NDetail.wzy + NDetailX.xyz - 1.0h; //	(Nu.wzyx - .5h) + (E-.5)

	half4 detail		= tex2D( s_detail, I.tcdbump);
	S.base.rgb			= S.base.rgb * detail.rgb * 2;

//	S.base.rgb			= float3(1,0,0);
#else        //	USE_TDETAIL_BUMP
#if BLOOM_MODE > 2
		half4 		detail 	= 		tex2D(s_bumpD,I.tcdbump).wzyx;
	   S.normal 		   = 		Nu.wzyx + (NuE.xyz - (1.0h + 0.5h*def_dbumph)) + detail.xyz * def_dbumph ;
#if BLOOM_MODE > 6
		S.gloss 			+= 		detail.w							;
#else
        S.gloss             =  		S.gloss * detail.w * 2				;
#endif
#else
        half4       detail  =		tex2D(s_detail,I.tcdbump)        	;
       S.base.rgb          =		S.base.rgb     * detail.rgb*2		;
        S.gloss             =  		S.gloss * detail.w * 2				;
#endif
#endif        //	USE_TDETAIL_BUMP
#endif

        return                S;
}
#else
surface_bumped                sload_i         ( p_bumped I)
{
        surface_bumped        S;
          half4 Nu 			=		tex2D                (s_bump, I.tcdh);                        // IN:  normal.gloss
        half4 NuE           =		tex2D                (s_bumpX,I.tcdh);                        // IN:         normal_error.height
        S.base              =		tbase                (I.tcdh)		;                         // IN:  rgb.a
        S.normal            =		Nu.wzyx + (NuE.xyz - 1.0h)			;
//        S.normal            = 		half3(1,0,0);
#if BLOOM_MODE > 2
		S.gloss				= 		Nu.x								;
#else
        S.gloss             =		Nu.x*Nu.x							;                         //        S.gloss             =        Nu.x*Nu.x;
#endif
        S.height            = 		NuE.z;

#ifdef        USE_TDETAIL
#ifdef        USE_TDETAIL_BUMP
	half4 NDetail		= tex2D( s_detailBump, I.tcdbump);
	half4 NDetailX		= tex2D( s_detailBumpX, I.tcdbump);
#if BLOOM_MODE > 6
	S.gloss 			+= NDetail.x;
#else
	S.gloss				= S.gloss * NDetail.x * 2;
#endif
	//S.normal			+= NDetail.wzy-.5;
	S.normal			+= NDetail.wzy + NDetailX.xyz - 1.0h; //	(Nu.wzyx - .5h) + (E-.5)

	half4 detail		= tex2D( s_detail, I.tcdbump);
	S.base.rgb			= S.base.rgb * detail.rgb * 2;

//	S.base.rgb			= float3(1,0,0);
#else        //	USE_TDETAIL_BUMP
#if BLOOM_MODE > 2
		half4  detail 		= 		 tex2D(s_bumpD,I.tcdbump).wzyx;
		S.normal 			= 		Nu.wzyx + (NuE.xyz - (1.0h + 0.5h*def_dbumph)) + detail.xyz * def_dbumph ;
#if BLOOM_MODE > 6
		S.gloss 			+= 		detail.w;
#else
        S.gloss             =  		S.gloss * detail.w * 2			;
#endif
#else
        half4 detail		=        tex2D(s_detail,I.tcdbump)    ;
        S.base.rgb          =      	S.base.rgb*detail.rgb        	*2      ;
        S.gloss             =  		S.gloss * detail.w * 2			;
#endif
#endif        //	USE_TDETAIL_BUMP
#endif
        return              S;
}
#endif
surface_bumped              sload                 ( p_bumped I)
{
        surface_bumped      S   = sload_i	(I);
#if BLOOM_MODE < 3
		S.normal.z			*=	0.5;		//. make bump twice as contrast (fake, remove me if possible)
#endif

        return              S;
}

#endif
