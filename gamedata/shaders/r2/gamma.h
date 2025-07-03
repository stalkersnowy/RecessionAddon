#ifndef	GAMMA_H_INCLUDED
#define	GAMMA_H_INCLUDED

#ifndef SHADER_GAMMA

half4 gamma(half4 img)
{
    return img;
}

#else	//	SHADER_GAMMA

uniform half3 gamma_params;
#define	_EPS 0.00001

half4 gamma(half4 img)
{
    float og = 1.0f / (gamma_params.x + _EPS);

    float B = gamma_params.y / 2.0f;
    float C = gamma_params.z / 2.0f;

    half3 corrected = (C + 0.5f) * pow(img.rgb, og) + 
                       (B - 0.5f) * 0.5f - C * 0.5f + 0.25f;

    corrected = clamp(corrected, 0.0f, 1.0f);

    return half4(corrected, img.a);
}

#endif	//	SHADER_GAMMA

#endif	//	GAMMA_H_INCLUDED