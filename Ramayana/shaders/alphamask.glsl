[vert]

#version 110

void main()
{
	gl_Position = ftransform();
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_FrontColor = gl_Color;
}

[frag]

#version 110

uniform sampler2D colorMap;
uniform float threshold;

void main()
{
	vec4 tex = texture2D(colorMap, gl_TexCoord[0].st);
	
	if(tex.a <= threshold)
		discard;
		
	gl_FragColor = gl_Color * tex;
}
