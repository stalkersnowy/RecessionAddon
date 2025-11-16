#ifndef	DOF_H_INCLUDED
#define	DOF_H_INCLUDED

//#define USE_DOF

#ifndef	USE_DOF

half3	dof(float2 center, float3 c_original)
{
	return	c_original;
}

#else	//	USE_DOF

// x - near y - focus z - far w - sky distance
half4	dof_params;
half3	dof_kernel;	// x,y - resolution pre-scaled z - just kernel size

half DOFFactor( half depth)
{
	half	dist_to_focus	= depth-dof_params.y;
	half 	blur_far	= saturate( dist_to_focus
					/ (dof_params.z-dof_params.y) );
	half 	blur_near	= saturate( dist_to_focus
					/ (dof_params.x-dof_params.y) );
	half 	blur 		= blur_near+blur_far;
	blur*=blur;
	return blur;
}


//#define MAXCOF		5.h
#define MAXCOF		7.h
#define EPSDEPTH	0.0001h
half3	dof(float2 center, float3 c_original)
{
	// Scale tap offsets based on render target size
	half 	depth		= tex2D(s_position,center).z;
	if (depth <= EPSDEPTH)	depth = dof_params.w;
	half	blur 		= DOFFactor(depth);

	//half blur = 1;
	//	const amount of blur: define controlled
	//half2 	scale 	= half2	(.5f / 1024.h, .5f / 768.h) * MAXCOF * blur;
	//	const amount of blur: engine controlled
	half2 	scale 	= half2	(.5f / 1024.h, .5f / 768.h) * (dof_kernel.z * blur);
	//	amount of blur varies according to resolution
	//	but kernel size in pixels is fixed.
	//	half2 	scale 	= dof_kernel.xy * blur;

	// poisson
	half2 	o  [12];
		o[0]	= half2(-0.326212f , -0.405810f)*scale;
		o[1] 	= half2(-0.840144f , -0.073580f)*scale;
		o[2] 	= half2(-0.695914f ,  0.457137f)*scale;
		o[3] 	= half2(-0.203345f ,  0.620716f)*scale;
		o[4] 	= half2( 0.962340f , -0.194983f)*scale;
		o[5] 	= half2( 0.473434f , -0.480026f)*scale;
		o[6] 	= half2( 0.519456f ,  0.767022f)*scale;
		o[7] 	= half2( 0.185461f , -0.893124f)*scale;
		o[8] 	= half2( 0.507431f ,  0.064425f)*scale;
		o[9] 	= half2( 0.896420f ,  0.412458f)*scale;
		o[10] 	= half2(-0.321940f , -0.932615f)*scale;
		o[11] 	= half2(-0.791559f , -0.597710f)*scale;

	// sample
	half3	sum 	= c_original;
	half 	contrib	= 1.h;

   	for (int i=0; i<12; i++)
	{
		float2 	tap 		= center + o[i];
		half4	tap_color	= tex2D	(s_image,tap);
		half 	tap_depth 	= tex2D	(s_position,tap).z;
		if (tap_depth <= EPSDEPTH)	tap_depth = dof_params.w;
		half 	tap_contrib	= DOFFactor(tap_depth);
		sum 		+= tap_color	* tap_contrib;
		contrib		+= tap_contrib;
	}

	return 	half3	(sum/contrib);
}
#endif	//	USE_DOF

#endif	//	DOF_H_INCLUDED