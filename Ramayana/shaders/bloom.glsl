[vert]

#version 120

void main() {
	gl_Position = ftransform();
	gl_TexCoord[0] = gl_MultiTexCoord0;
}

[frag]

#version 120

uniform sampler2D colorMap;
uniform float aspectRatio;
uniform float blur = 0.002;
uniform float exposure = 0.25;

void main() {
	vec2 aspectcorrect = vec2(1.0, aspectRatio);
	
	vec4 sum = vec4(0.0);
	int i, j;
	for(i = -3; i < 3; i++)
		for (j = -3; j < 3; j++)
			sum += texture2D(colorMap, gl_TexCoord[0].st + vec2(j, i) * aspectcorrect * blur) * exposure;
	
	vec4 tex = texture2D(colorMap, gl_TexCoord[0].st);
	if (tex.r < 0.3)
		gl_FragColor = sum * sum * 0.0120 + tex;
	else if (tex.r < 0.5)
		gl_FragColor = sum * sum * 0.0090 + tex;
	else
		gl_FragColor = sum * sum * 0.0075 + tex;
}
