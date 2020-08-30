[vert]

#version 120

varying vec4 color;

uniform bool normalMapEnabled = false;
uniform float lightRadius;
varying vec3 lightDir;
varying vec3 viewDir;

varying float fogFactor;

varying float waterHeight;

void main()
{
	gl_TexCoord[0] = gl_MultiTexCoord0;
	
	vec3 vertexPos = vec3(gl_ModelViewMatrix * gl_Vertex);	
	
	if(normalMapEnabled) {
		gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
		
		waterHeight = dot(gl_ClipPlane[0], vec4(vertexPos, 1));
		
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
		
		waterHeight = dot(gl_ClipPlane[0], vec4(vertexPos, 1));
		
		vec3 normal = gl_NormalMatrix * gl_Normal;
		vec3 s = normalize(gl_LightSource[0].position.xyz - vertexPos);	
		
		color = gl_FrontLightModelProduct.sceneColor + mix(gl_FrontLightProduct[0].ambient, gl_FrontLightProduct[0].diffuse, clamp(dot(s, normal), 0.0, 1.0));
	}
	
	//Fog
	gl_FogFragCoord = length(vertexPos);
	const float LOG2 = 1.442695;
	fogFactor = exp2(-gl_Fog.density * gl_Fog.density * gl_FogFragCoord * gl_FogFragCoord * LOG2);
	fogFactor = clamp(fogFactor, 0.0, 1.0);
}

[frag]

#version 120

uniform sampler2D colorMap;
varying vec4 color;

uniform bool normalMapEnabled = false;
uniform sampler2D normalMap;
varying vec3 lightDir;
varying vec3 viewDir;

varying float fogFactor;

varying float waterHeight;

void main()
{
	if(waterHeight < 0.0)
		discard;
		
	vec4 tex = texture2D(colorMap, gl_TexCoord[0].st);

	if(tex.a < 0.5)
		discard;
	
	if(normalMapEnabled) {
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
		vec4 fragColor = gl_FrontLightModelProduct.sceneColor + ambient + diffuse + specular;
		
		gl_FragColor.rgb = fragColor.rgb * tex.rgb;
	} else {
		gl_FragColor.rgb = color.rgb * tex.rgb;
	}
	
	//Fog
	gl_FragColor.rgb = mix(gl_Fog.color.rgb, gl_FragColor.rgb, fogFactor);
	gl_FragColor.a = gl_FrontLightProduct[0].diffuse.a * tex.a;
}
