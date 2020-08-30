#include "stdafx.h"

#include "world.h"
#include "water.h"

namespace ramayana {


	Water::Water() {
		waterTextureFrameNumber = waterBumpTextureFrameNumber = 0;
		currentWaterLevel = BASE_WATER_LEVEL;
		waterWavePhase = 0;
		waterColor.set(1, 1, 1);

		waterVBName = 0;
	}
	Water::~Water() {
		if (waterVBName != 0) {
			glDeleteBuffersARB(1, &waterVBName);
		}
	}

	void Water::init() {
		waterTexture.load(
			"special/water",
			true,
			GL_LINEAR, GL_LINEAR_MIPMAP_NEAREST,
			GL_REPEAT, GL_REPEAT);
		waterBumpTexture.load(
			"special/water_bump",
			true,
			GL_LINEAR, GL_LINEAR_MIPMAP_NEAREST,
			GL_REPEAT, GL_REPEAT);

		waterShader.load("shaders/water.glsl");

		updateWater(true);
	}

	void Water::updateWater(bool init) {
		const float border = WORLD_RADIUS;
		const int textureSize = 5;
		float level = (_sin[waterWavePhase] + 1.0) * waterWaveAmplitude;
		waterWavePhase = (waterWavePhase + 5) % 360;
		currentWaterLevel = BASE_WATER_LEVEL + level;

		//putting vertex into vertex buffer data
		waterVB[0].pos = Point3D(-border, -border, currentWaterLevel);
		waterVB[1].pos = Point3D(border + width - 1, -border, currentWaterLevel);
		waterVB[2].pos = Point3D(border + width - 1, border + height - 1, currentWaterLevel);
		waterVB[3].pos = Point3D(-border, border + height - 1, currentWaterLevel);

		if (init) {
			//putting texture coordinate into vertex buffer data
			waterVB[0].texcoord = Point2D(0, 0);
			waterVB[1].texcoord = Point2D((width + 2 * border) / textureSize, 0);
			waterVB[2].texcoord = Point2D((width + 2 * border) / textureSize, (height + 2 * border) / textureSize);
			waterVB[3].texcoord = Point2D(0, (height + 2 * border) / textureSize);

			//putting normal into vertex buffer data
			waterVB[0].normal = Vector3D(0, 0, 1);
			waterVB[1].normal = Vector3D(0, 0, 1);
			waterVB[2].normal = Vector3D(0, 0, 1);
			waterVB[3].normal = Vector3D(0, 0, 1);

			//calculating tangent
			Vector4D tangent = getTangent(
				waterVB[0].pos, waterVB[1].pos, waterVB[3].pos,
				waterVB[0].texcoord, waterVB[1].texcoord, waterVB[3].texcoord,
				waterVB[0].normal);

			//putting tangent into vertex buffer data
			waterVB[0].tangent = tangent;
			waterVB[1].tangent = tangent;
			waterVB[2].tangent = tangent;
			waterVB[3].tangent = tangent;
		}

		//Generate and bind burtex buffer
		if (init) {
			glGenBuffersARB(1, &waterVBName);
		}
		glBindBufferARB(GL_ARRAY_BUFFER, waterVBName);
		glBufferDataARB(GL_ARRAY_BUFFER, sizeof(Vertex)* 4, waterVB, GL_STATIC_DRAW);
	}
	void Water::drawWater(bool noShader, bool reflectionOn, bool shadowOn) {
		glEnable(GL_LIGHTING);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CCW);

		float ambient[] = { 0.6*waterColor.r(), 0.6*waterColor.g(), 0.6*waterColor.b(), 0.3 };
		float diffuse[] = { 0.7*waterColor.r(), 0.7*waterColor.g(), 0.7*waterColor.b(), 0.4 };
		float specular[] = { 0.8*waterColor.r(), 0.8*waterColor.g(), 0.8*waterColor.b(), 0.5 };
		float emission[] = { 0.0, 0.0, 0.0, 0.0 };
		glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
		glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
		glMaterialfv(GL_FRONT, GL_EMISSION, emission);
		glMaterialf(GL_FRONT, GL_SHININESS, 0.6);

		glBindBufferARB(GL_ARRAY_BUFFER, waterVBName);
		if (!isEditable() && !noShader) {
			waterShader.use();
			glUniform1i(waterShader.getUniformLocation("colorMap"), SAMPLER_DEFAULT);
			glUniform1i(waterShader.getUniformLocation("normalMap"), SAMPLER_NORMAL_MAP);
			glUniform1f(waterShader.getUniformLocation("lightRadius"), WORLD_RADIUS);
			glUniform1i(waterShader.getUniformLocation("reflectionEnabled"), reflectionOn ? GL_TRUE : GL_FALSE);
			glUniform1i(waterShader.getUniformLocation("environmentMap"), SAMPLER_ENV_MAP);
			glUniform1i(waterShader.getUniformLocation("shadowEnabled"), shadowOn ? GL_TRUE : GL_FALSE);
			glUniform1i(waterShader.getUniformLocation("shadowMap"), SAMPLER_SHADOW_MAP);
			glUniform1f(waterShader.getUniformLocation("xPixelOffset"), 1.0 / ((float)shadowMapFrameBuffer.getWidth()));
			glUniform1f(waterShader.getUniformLocation("yPixelOffset"), 1.0 / ((float)shadowMapFrameBuffer.getHeight()));

			if (reflectionOn) {
				glActiveTextureARB(TEXTURE_ENV_MAP);
				glEnable(GL_TEXTURE_2D);
				reflectionFrameBuffer.bindColorTexture();
			}

			if (shadowOn) {
				glActiveTextureARB(TEXTURE_SHADOW_MAP);
				glEnable(GL_TEXTURE_2D);
				shadowMapFrameBuffer.bindDepthTexture();
			}

			glActiveTextureARB(TEXTURE_NORMAL_MAP);
			glEnable(GL_TEXTURE_2D);
			waterBumpTexture.bind(waterBumpTextureFrameNumber);
			if (!isPaused()) {
				waterBumpTextureFrameNumber = (waterBumpTextureFrameNumber + 1) % waterBumpTexture.length();
			}

			glActiveTextureARB(TEXTURE_DEFAULT);
		}
		glEnable(GL_TEXTURE_2D);
		waterTexture.bind(waterTextureFrameNumber);
		if (!isPaused()) {
			waterTextureFrameNumber = (waterTextureFrameNumber + 1) % waterTexture.length();
		}

		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, sizeof(Vertex), (GLvoid*)&((Vertex*)0)->pos);

		glEnableClientState(GL_NORMAL_ARRAY);
		glNormalPointer(GL_FLOAT, sizeof(Vertex), (GLvoid*)&((Vertex*)0)->normal);

		glClientActiveTextureARB(TEXTURE_DEFAULT);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), (GLvoid*)&((Vertex*)0)->texcoord);

		if (!isEditable() && !noShader) {
			glClientActiveTextureARB(TEXTURE_NORMAL_MAP);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glTexCoordPointer(4, GL_FLOAT, sizeof(Vertex), (GLvoid*)&((Vertex*)0)->tangent);
		}

		glDrawArrays(GL_QUADS, 0, 4);

		if (!isEditable() && !noShader) {
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			glClientActiveTextureARB(TEXTURE_DEFAULT);
		}
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);

		glBindBufferARB(GL_ARRAY_BUFFER, 0);

		if (!isEditable() && !noShader) {
			ShaderProgram::useNone();

			if (reflectionOn) {
				glActiveTextureARB(TEXTURE_ENV_MAP);
				Texture2D::bindNone();
				glDisable(GL_TEXTURE_2D);
			}

			if (shadowOn) {
				glActiveTextureARB(TEXTURE_SHADOW_MAP);
				Texture2D::bindNone();
				glDisable(GL_TEXTURE_3D);
			}

			glActiveTextureARB(TEXTURE_NORMAL_MAP);
			Texture2D::bindNone();
			glDisable(GL_TEXTURE_2D);

			glActiveTextureARB(TEXTURE_DEFAULT);
		}
		Texture2D::bindNone();
		glDisable(GL_TEXTURE_2D);

		glDisable(GL_CULL_FACE);
		glDisable(GL_BLEND);
		glDisable(GL_LIGHTING);
	}

	int Water::getWaterWavePhase() const {
		return waterWavePhase;
	}
	float Water::getWaterWaveAmplitude() const {
		return waterWaveAmplitude;
	}
}
