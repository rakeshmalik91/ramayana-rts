[vert]

#version 120

void main() {
	gl_Position = ftransform();
	gl_TexCoord[0] = gl_MultiTexCoord0;
}

[frag]

#version 120

uniform sampler2D blurSampler;
uniform int kernelSize = 1;
uniform bool horizontal = true;
uniform float blurSize;

const float pi = 3.14159265f;

void main() {
	//float blurSize = 1.0 / textureSize(blurSampler, 0).x;
	vec2 blurMultiplyVec = horizontal ? vec2(1.0f, 0.0f) : vec2(0.0f, 1.0f);
	float sigma = kernelSize / 2;
	float numBlurPixelsPerSide = (kernelSize - 1) / 2;
	
	// Incremental Gaussian Coefficent Calculation (See GPU Gems 3 pp. 877 - 889)
	vec3 incrementalGaussian;
	incrementalGaussian.x = 1.0f / (sqrt(2.0f * pi) * sigma);
	incrementalGaussian.y = exp(-0.5f / (sigma * sigma));
	incrementalGaussian.z = incrementalGaussian.y * incrementalGaussian.y;
	
	vec4 avgValue = vec4(0.0f, 0.0f, 0.0f, 0.0f);
	float coefficientSum = 0.0f;
	
	// Take the central sample first...
	avgValue += texture2D(blurSampler, gl_TexCoord[0].xy) * incrementalGaussian.x;
	coefficientSum += incrementalGaussian.x;
	incrementalGaussian.xy *= incrementalGaussian.yz;
	
	// Go through the remaining 8 vertical samples (4 on each side of the center)
	for (float i = 1.0f; i <= numBlurPixelsPerSide; i++) { 
		avgValue += texture2D(blurSampler, gl_TexCoord[0].xy - i * blurSize * blurMultiplyVec) * incrementalGaussian.x;         
		avgValue += texture2D(blurSampler, gl_TexCoord[0].xy + i * blurSize * blurMultiplyVec) * incrementalGaussian.x;         
		coefficientSum += 2 * incrementalGaussian.x;
		incrementalGaussian.xy *= incrementalGaussian.yz;
	}
	
	gl_FragColor = avgValue / coefficientSum;
}
