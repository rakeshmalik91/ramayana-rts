[vert]

#version 120

void main()
{
	gl_Position = ftransform();
	gl_TexCoord[0] = gl_MultiTexCoord0;
}

[frag]

#version 120

uniform sampler2D colorMap;
uniform float threshold;

void main() {
	vec4 color = texture2D(colorMap, gl_TexCoord[0].st);
	float lum = dot(color.rgb, vec3(0.299, 0.587, 0.114));
	float lum2 = max(lum - threshold, 0.0f);
	gl_FragColor.rgb = color.rgb * lum2 / lum;
	gl_FragColor.a = 1.0;
}
