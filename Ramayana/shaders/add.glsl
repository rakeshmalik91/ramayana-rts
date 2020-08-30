[vert]

#version 120

void main()
{
	gl_Position = ftransform();
	gl_TexCoord[0] = gl_MultiTexCoord0;
}

[frag]

#version 120

uniform int n;
uniform sampler2D sampler0, sampler1, sampler2, sampler3, sampler4;

void main() {
	gl_FragColor.rgb = texture2D(sampler0, gl_TexCoord[0].st).rgb;
	gl_FragColor.rgb += texture2D(sampler1, gl_TexCoord[0].st).rgb;
	gl_FragColor.rgb += texture2D(sampler2, gl_TexCoord[0].st).rgb;
	gl_FragColor.rgb += texture2D(sampler3, gl_TexCoord[0].st).rgb;
	gl_FragColor.rgb += texture2D(sampler4, gl_TexCoord[0].st).rgb;
	gl_FragColor.a = 1.0;
}
