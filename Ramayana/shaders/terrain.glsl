[vert]

#version 120

varying vec4 color;

uniform bool normalMapEnabled = false;
uniform float lightRadius = 0.0;
varying vec3 lightDir;
varying vec3 viewDir;

varying float alpha;

attribute float fogOfWar;

varying float fogFactor;

uniform bool shadowEnabled = false;
varying vec4 shadowCoord;

varying float waterHeight;

void main() {
	vec3 vertexPos = vec3(gl_ModelViewMatrix * gl_Vertex);
	
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_TexCoord[6] = gl_MultiTexCoord6;
	
	if(normalMapEnabled) {
		gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
		
		vec3 n = normalize(gl_NormalMatrix * gl_Normal);
		vec3 t = normalize(gl_NormalMatrix * gl_MultiTexCoord1.xyz);
		vec3 b = cross(n, t) * gl_MultiTexCoord1.w;
		
		mat3 tbnMatrix = mat3(t.x, b.x, n.x,
							t.y, b.y, n.y,
							t.z, b.z, n.z);
	
		lightDir = (gl_LightSource[0].position.xyz - vertexPos) / lightRadius;
		lightDir = tbnMatrix * lightDir;
	
		viewDir = -vertexPos;
		viewDir = tbnMatrix * viewDir;
	} else {
		gl_Position = ftransform();
		
		vec3 normal = gl_NormalMatrix * gl_Normal;
	
		vec3 s = normalize(gl_LightSource[0].position.xyz - vertexPos);	
		
		color = gl_FrontLightModelProduct.sceneColor + gl_FrontLightProduct[0].ambient;
		color += mix(vec4(0.0), gl_FrontLightProduct[0].diffuse + gl_FrontLightProduct[0].specular, max(dot(s, normal), 0.0));
	}
	
	//Shadow
	if(shadowEnabled)
		shadowCoord = gl_TextureMatrix[7] * gl_Vertex;
	
	//Fog
	gl_FogFragCoord = length(vertexPos);
	const float LOG2 = 1.442695;
	fogFactor = exp2(-gl_Fog.density * gl_Fog.density * gl_FogFragCoord * gl_FogFragCoord * LOG2);
	fogFactor = clamp(fogFactor, 0.0, 1.0);
	
	//Blend
	alpha = clamp(1.0 + gl_Vertex.z / 5.0, 0.0, 1.0);
	
	//Clip
	waterHeight = dot(gl_ClipPlane[0], vec4(vertexPos, 1));
}

[frag]

#version 120

uniform sampler3D colorMap;
varying vec4 color;

uniform bool normalMapEnabled = false;
uniform sampler3D normalMap;
varying vec3 lightDir;
varying vec3 viewDir;

varying float alpha;

uniform sampler2D fogOfWarMap;

varying float fogFactor;

#define HARD_SHADOW 	0
#define PCF_3x3 		1
#define PCF_5x5 		2
#define VSM 			3
#define SHADOW_TYPE 	PCF_3x3
uniform bool shadowEnabled = false;
uniform sampler2DShadow shadowMap;
varying vec4 shadowCoord;
uniform float xPixelOffset;
uniform float yPixelOffset;

uniform bool clipPlane0Enabled = false;
varying float waterHeight;

void main() {
	if(clipPlane0Enabled && waterHeight < 0.0)
		discard;
	
	if(alpha<=0.0)
		discard;
	
	if(normalMapEnabled) {
		vec3 l = lightDir;
		float atten = max(0.0, 1.0 - dot(l, l));
		
		l = normalize(l);
		
		vec3 n = normalize(texture3D(normalMap, gl_TexCoord[0].xyz).xyz * 2.0 - 1.0);
		vec3 v = normalize(viewDir);
		vec3 h = normalize(l + v);
	
		float nDotL = max(0.0, dot(n, l));
		float nDotH = max(0.0, dot(n, h));
		float power = (nDotL == 0.0) ? 0.0 : pow(nDotH, gl_FrontMaterial.shininess);
		
		vec4 ambient = gl_FrontLightProduct[0].ambient * atten;
		vec4 diffuse = gl_FrontLightProduct[0].diffuse * nDotL * atten;
		vec4 specular = gl_FrontLightProduct[0].specular * power * atten;
		vec4 fragColor = gl_FrontLightModelProduct.sceneColor + ambient + diffuse + specular;
		
		gl_FragColor.rgb = fragColor.rgb * texture3D(colorMap, gl_TexCoord[0].xyz).rgb;
	} else {
		gl_FragColor.rgb = color.rgb * texture3D(colorMap, gl_TexCoord[0].xyz).rgb;
	}
	
	//shadow
	if(shadowEnabled) {
		vec4 shadowCoordinateWdivide = shadowCoord / shadowCoord.w;
		shadowCoordinateWdivide.z += 0.0000005;
#if SHADOW_TYPE==HARD_SHADOW
		float distanceFromLight = shadow2DProj(shadowMap, shadowCoordinateWdivide).z;
		float shadow = 1.0;
		if(shadowCoord.w > 0.0)
			shadow = distanceFromLight < shadowCoordinateWdivide.z ? 0.5 : 1.0 ;
		gl_FragColor *= shadow;
#elif SHADOW_TYPE==PCF_3x3
		float shadow = 0.0;
		for(float x=-0.5; x<=0.5; x+=0.5)
			for(float y=-0.5; y<=0.5; y+=0.5)
				shadow += shadow2DProj(shadowMap, shadowCoordinateWdivide + vec4(xPixelOffset * x, yPixelOffset * y, 0, 0)).z;
		shadow = 0.5 + shadow / 18.0;
		gl_FragColor *= clamp(shadow, 0.5, 1.0);
#elif SHADOW_TYPE==PCF_5x5
		float shadow = 0.0;
		for(float x=-1.0; x<=1.0; x+=0.5)
			for(float y=-1.0; y<=1.0; y+=0.5)
				shadow += shadow2DProj(shadowMap, shadowCoordinateWdivide + vec4(xPixelOffset * x, yPixelOffset * y, 0, 0)).z;
		shadow = 0.5 + shadow / 50.0;
		gl_FragColor *= clamp(shadow, 0.5, 1.0);
#elif SHADOW_TYPE==VSM
		vec2 moments = shadow2DProj(shadowMap, shadowCoordinateWdivide).xy;
		float shadow = 1.0;
		if(shadowCoordinateWdivide.z > moments.x) {
			float variance = moments.y - (moments.x * moments.x);
			variance = max(variance, 0.1);
			float d = shadowCoordinateWdivide.z - moments.x;
			float p_max = (variance / (variance + d * d));
			shadow = max(p_max, 0.5);
		}
		gl_FragColor *= shadow;
#endif
	}
	
	//Fog of war
	gl_FragColor.rgb = gl_FragColor.rgb * texture2D(fogOfWarMap, gl_TexCoord[6].xy).rgb;
	
	//Fog
	gl_FragColor.rgb = mix(gl_Fog.color.rgb, gl_FragColor.rgb, fogFactor);
	
	//Blend
	gl_FragColor.a = alpha;
}
