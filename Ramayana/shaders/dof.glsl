[vert]

#version 120

void main() {
	gl_Position = ftransform();
	gl_TexCoord[0] = gl_MultiTexCoord0;
}

[frag]

#version 120

uniform sampler2D colorMap;
uniform sampler2D depthMap;
uniform float aspectRatio;
uniform float aperture;
uniform float blurclamp = 5.0;

void main() {
	vec2 aspectcorrect = vec2(1.0, aspectRatio);
	
	vec4 depth = texture2D(depthMap, gl_TexCoord[0].xy);
	float focus = texture2D(depthMap, vec2(0.5, 0.5)).x;
	float factor = depth.x - focus;
	vec2 dofblur = vec2(clamp(factor * aperture, -blurclamp, blurclamp));
	
	vec4 col = vec4(0.0);
	col += texture2D(colorMap, gl_TexCoord[0].xy);
	col += texture2D(colorMap, gl_TexCoord[0].xy + (vec2( 0.00, 0.40) * aspectcorrect) * dofblur);
	col += texture2D(colorMap, gl_TexCoord[0].xy + (vec2( 0.15, 0.37) * aspectcorrect) * dofblur);
	col += texture2D(colorMap, gl_TexCoord[0].xy + (vec2( 0.29, 0.29) * aspectcorrect) * dofblur);
	col += texture2D(colorMap, gl_TexCoord[0].xy + (vec2(-0.37, 0.15) * aspectcorrect) * dofblur);       
	col += texture2D(colorMap, gl_TexCoord[0].xy + (vec2( 0.40, 0.00) * aspectcorrect) * dofblur);   
	col += texture2D(colorMap, gl_TexCoord[0].xy + (vec2( 0.37,-0.15) * aspectcorrect) * dofblur);       
	col += texture2D(colorMap, gl_TexCoord[0].xy + (vec2( 0.29,-0.29) * aspectcorrect) * dofblur);       
	col += texture2D(colorMap, gl_TexCoord[0].xy + (vec2(-0.15,-0.37) * aspectcorrect) * dofblur);
	col += texture2D(colorMap, gl_TexCoord[0].xy + (vec2( 0.00,-0.40) * aspectcorrect) * dofblur); 
	col += texture2D(colorMap, gl_TexCoord[0].xy + (vec2(-0.15, 0.37) * aspectcorrect) * dofblur);
	col += texture2D(colorMap, gl_TexCoord[0].xy + (vec2(-0.29, 0.29) * aspectcorrect) * dofblur);
	col += texture2D(colorMap, gl_TexCoord[0].xy + (vec2( 0.37, 0.15) * aspectcorrect) * dofblur); 
	col += texture2D(colorMap, gl_TexCoord[0].xy + (vec2(-0.40, 0.00) * aspectcorrect) * dofblur); 
	col += texture2D(colorMap, gl_TexCoord[0].xy + (vec2(-0.37,-0.15) * aspectcorrect) * dofblur);       
	col += texture2D(colorMap, gl_TexCoord[0].xy + (vec2(-0.29,-0.29) * aspectcorrect) * dofblur);       
	col += texture2D(colorMap, gl_TexCoord[0].xy + (vec2( 0.15,-0.37) * aspectcorrect) * dofblur);
	                                                                   
	col += texture2D(colorMap, gl_TexCoord[0].xy + (vec2( 0.15, 0.37) * aspectcorrect) * dofblur * 0.9);
	col += texture2D(colorMap, gl_TexCoord[0].xy + (vec2(-0.37, 0.15) * aspectcorrect) * dofblur * 0.9);           
	col += texture2D(colorMap, gl_TexCoord[0].xy + (vec2( 0.37,-0.15) * aspectcorrect) * dofblur * 0.9);           
	col += texture2D(colorMap, gl_TexCoord[0].xy + (vec2(-0.15,-0.37) * aspectcorrect) * dofblur * 0.9);
	col += texture2D(colorMap, gl_TexCoord[0].xy + (vec2(-0.15, 0.37) * aspectcorrect) * dofblur * 0.9);
	col += texture2D(colorMap, gl_TexCoord[0].xy + (vec2( 0.37, 0.15) * aspectcorrect) * dofblur * 0.9);            
	col += texture2D(colorMap, gl_TexCoord[0].xy + (vec2(-0.37,-0.15) * aspectcorrect) * dofblur * 0.9);   
	col += texture2D(colorMap, gl_TexCoord[0].xy + (vec2( 0.15,-0.37) * aspectcorrect) * dofblur * 0.9);   
	                                                                                           
	col += texture2D(colorMap, gl_TexCoord[0].xy + (vec2( 0.29, 0.29) * aspectcorrect) * dofblur * 0.7);
	col += texture2D(colorMap, gl_TexCoord[0].xy + (vec2( 0.40, 0.00) * aspectcorrect) * dofblur * 0.7);       
	col += texture2D(colorMap, gl_TexCoord[0].xy + (vec2( 0.29,-0.29) * aspectcorrect) * dofblur * 0.7);   
	col += texture2D(colorMap, gl_TexCoord[0].xy + (vec2( 0.00,-0.40) * aspectcorrect) * dofblur * 0.7);     
	col += texture2D(colorMap, gl_TexCoord[0].xy + (vec2(-0.29, 0.29) * aspectcorrect) * dofblur * 0.7);
	col += texture2D(colorMap, gl_TexCoord[0].xy + (vec2(-0.40, 0.00) * aspectcorrect) * dofblur * 0.7);     
	col += texture2D(colorMap, gl_TexCoord[0].xy + (vec2(-0.29,-0.29) * aspectcorrect) * dofblur * 0.7);   
	col += texture2D(colorMap, gl_TexCoord[0].xy + (vec2( 0.00, 0.40) * aspectcorrect) * dofblur * 0.7);
					                                                                             
	col += texture2D(colorMap, gl_TexCoord[0].xy + (vec2( 0.29, 0.29) * aspectcorrect) * dofblur * 0.4);
	col += texture2D(colorMap, gl_TexCoord[0].xy + (vec2( 0.40, 0.00) * aspectcorrect) * dofblur * 0.4);       
	col += texture2D(colorMap, gl_TexCoord[0].xy + (vec2( 0.29,-0.29) * aspectcorrect) * dofblur * 0.4);   
	col += texture2D(colorMap, gl_TexCoord[0].xy + (vec2( 0.00,-0.40) * aspectcorrect) * dofblur * 0.4);     
	col += texture2D(colorMap, gl_TexCoord[0].xy + (vec2(-0.29, 0.29) * aspectcorrect) * dofblur * 0.4);
	col += texture2D(colorMap, gl_TexCoord[0].xy + (vec2(-0.40, 0.00) * aspectcorrect) * dofblur * 0.4);     
	col += texture2D(colorMap, gl_TexCoord[0].xy + (vec2(-0.29,-0.29) * aspectcorrect) * dofblur * 0.4);   
	col += texture2D(colorMap, gl_TexCoord[0].xy + (vec2( 0.00, 0.40) * aspectcorrect) * dofblur * 0.4);       
				
	gl_FragColor = col / 41.0;
	gl_FragColor.a = 1.0;
}
