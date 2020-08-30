[vert]

#version 120

void main() {
	gl_Position = ftransform();
	gl_TexCoord[0] = gl_MultiTexCoord0;
}

[frag]

#version 120

uniform sampler2D colorMap;
uniform float brightness = 1.3;
uniform float contrast = 1.3;

void main() {
	vec4 tex = texture2D(colorMap, gl_TexCoord[0].xy);
	gl_FragColor.rgb = vec3(pow(tex.r, contrast), pow(tex.g, contrast), pow(tex.b, contrast)) * brightness;
	gl_FragColor.a = 1.0;
}
