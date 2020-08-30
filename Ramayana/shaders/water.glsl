[vert]

#version 120

uniform float lightRadius;

varying vec3 lightDir;
varying vec3 viewDir;

uniform bool reflectionEnabled = false;
varying vec4 reflCoord;

uniform bool shadowEnabled = false;
varying vec4 shadowCoord;

void main() {
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    gl_TexCoord[0] = gl_MultiTexCoord0;

    vec3 vertexPos = vec3(gl_ModelViewMatrix * gl_Vertex);
    
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
	
	if(reflectionEnabled)
		reflCoord = gl_TextureMatrix[5] * gl_Vertex;
	
	//Shadow
	if(shadowEnabled)
		shadowCoord = gl_TextureMatrix[7] * gl_Vertex;
}

[frag]

#version 120

uniform sampler2D colorMap;
uniform sampler2D normalMap;
varying vec3 lightDir;
varying vec3 viewDir;

varying float fogFactor;

uniform bool reflectionEnabled = false;
uniform sampler2D environmentMap;
varying vec4 reflCoord;

#define HARD_SHADOW 	0
#define PCF_3x3 		1
#define PCF_5x5 		2
#define VSM 			3
#define SHADOW_TYPE 	VSM
uniform bool shadowEnabled = false;
uniform sampler2DShadow shadowMap;
varying vec4 shadowCoord;
uniform float xPixelOffset;
uniform float yPixelOffset;

void main() {
	vec3 l = lightDir;
	float atten = max(0.0, 1.0 - dot(l, l));
	
	l = normalize(l);
	
	vec3 n = normalize(texture2D(normalMap, gl_TexCoord[0].st).xyz * 2.0 - 1.0);
	vec3 v = normalize(viewDir);
	vec3 h = normalize(l + v);
	
	float nDotL = max(0.0, dot(n, l));
	float nDotH = max(0.0, dot(n, h));
	float power = (nDotL == 0.0) ? 0.0 : pow(nDotH, gl_FrontMaterial.shininess);
	
	vec4 ambient = gl_FrontLightProduct[0].ambient * atten;
	vec4 diffuse = gl_FrontLightProduct[0].diffuse * nDotL * atten;
	vec4 specular = gl_FrontLightProduct[0].specular * power * atten;
	vec4 color = gl_FrontLightModelProduct.sceneColor + ambient + diffuse + specular;
	
	gl_FragColor = texture2D(colorMap, gl_TexCoord[0].st);
	gl_FragColor.a = 0.25;
	
	vec4 noise = vec4(n.xy * 0.03, 0.0, 0.0);
	
	//Reflection
	if(reflectionEnabled) {
		vec4 refl = texture2D(environmentMap, reflCoord.st / reflCoord.w + noise.xy);
		if(dot(v, n) < 0.7071) {					// cos(45) = 0.7071
			gl_FragColor = mix(gl_FragColor, refl, 0.75);
		} else {
			gl_FragColor = mix(gl_FragColor, refl, 0.50);
		}
	}
	
	gl_FragColor *= mix(gl_FragColor, color, 0.5);
	
	//shadow
	if(shadowEnabled) {
		vec4 shadowCoordinateWdivide = shadowCoord / shadowCoord.w + noise;
		shadowCoordinateWdivide.z += 0.0000005;
#if SHADOW_TYPE==HARD_SHADOW
		float distanceFromLight = shadow2DProj(shadowMap, shadowCoordinateWdivide).z;
		float shadow = 1.0;
		if(shadowCoord.w > 0.0)
			shadow = distanceFromLight < shadowCoordinateWdivide.z ? 0.5 : 1.0 ;
		gl_FragColor *= shadow;
#elif SHADOW_TYPE==PCF_3x3
		float shadow = 0.0;
		for(float x=-1.0; x<=1.0; x+=1.0)
			for(float y=-1.0; y<=1.0; y+=1.0)
				shadow += shadow2DProj(shadowMap, shadowCoordinateWdivide + vec4(xPixelOffset * x, yPixelOffset * y, 0, 0)).z;
		shadow = 0.5 + shadow / 18.0;
		gl_FragColor *= clamp(shadow, 0.5, 1.0);
#elif SHADOW_TYPE==PCF_5x5
		float shadow = 0.0;
		for(float x=-2.0; x<=2.0; x+=1.0)
			for(float y=-2.0; y<=2.0; y+=1.0)
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
}
