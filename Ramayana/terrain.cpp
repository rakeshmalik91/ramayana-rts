#include "stdafx.h"

#include "common.h"
#include "terrain.h"
#include "interface.h"

using namespace math;
using namespace graphics;


namespace ramayana {

	Terrain::Terrain() {
		height = width = 0;
		loaded = false;

		terrainVBName = 0;
		fogOfWarMap = NULL;
	}
	Terrain::~Terrain() {
		if (loaded == true) {
			if (terrainVBName != 0) {
				glDeleteBuffersARB(1, &terrainVBName);
			}
			loaded = false;
		}
	}

	void Terrain::initFBOs(int screenWidth, int screenHeight) {
		shadowMapFrameBuffer.create(screenWidth*SHADOWMAP_RATIO, screenHeight*SHADOWMAP_RATIO);
		shadowMapFrameBuffer.attachDepthTexture();
		shadowMapFrameBuffer.dontDraw();
		shadowMapFrameBuffer.checkStatus();

		reflectionFrameBuffer.create(screenWidth, screenHeight);
		reflectionFrameBuffer.attachDepthTexture();
		reflectionFrameBuffer.attachColorTexture();
		reflectionFrameBuffer.checkStatus();
	}
	void Terrain::initTerrain() {
		terrainTexture.load(
			"special/terrain",
			Terrain::TERRAIN_TEXTURE_SIZE, Terrain::TERRAIN_TEXTURE_SIZE, Terrain::N_TERRAIN_TEXTURE,
			true,
			GL_LINEAR, GL_LINEAR,
			GL_REPEAT, GL_REPEAT, GL_CLAMP);
		terrainBumpTexture.load(
			"special/terrain_bump",
			Terrain::TERRAIN_TEXTURE_SIZE, Terrain::TERRAIN_TEXTURE_SIZE, Terrain::N_TERRAIN_TEXTURE,
			true,
			GL_LINEAR, GL_LINEAR,
			GL_REPEAT, GL_REPEAT, GL_CLAMP);
	}
	void Terrain::init(int screenWidth, int screenHeight) {
		initTerrain();
		getLogger().print("Terrain (" + toString(width) + "x" + toString(height) + ") initialized.");
		
		terrainShader.load("shaders/terrain.glsl");
		getLogger().print("Shader shaders/terrain.glsl loaded.");
		
		initFBOs(screenWidth, screenHeight);
		getLogger().print("Terrain FBO (" + toString(screenWidth) + "x" + toString(screenHeight) + ") initialized.");

		updateTerrain(true);
	}
	void Terrain::loadTerrain(string ter_filename, string tex_filename) {
		IplImage *ter = cvLoadImage(ter_filename.data(), CV_LOAD_IMAGE_GRAYSCALE);
		if (ter == NULL) {
			throw FileNotFoundException(ter_filename);
		}
		if (ter->nChannels != 1) {
			throw Exception(ter_filename + " must have single channel");
		}
		width = ter->width;
		height = ter->height;
		z = allocate<float>(width, height);
		for (int r = 0; r < height; r++) {
			for (int c = 0; c < width; c++) {
				if (r == 0 || c == 0 || r == height - 1 || c == width - 1) {
					z[r][c] = MIN_TERRAIN_HEIGHT;
				} else {
					z[r][c] = ((uchar*)ter->imageData)[(((int)height - r - 1)*ter->widthStep + c)] * (float)(MAX_TERRAIN_HEIGHT - MIN_TERRAIN_HEIGHT) / 256.0 + MIN_TERRAIN_HEIGHT;
				}
			}
		}
		cvReleaseImage(&ter);

		IplImage *tex = cvLoadImage(tex_filename.data(), CV_LOAD_IMAGE_COLOR);
		if (tex == NULL) {
			throw FileNotFoundException(tex_filename);
		}
		textureIndex = allocate<int>(width, height);
		for (int r = 0; r < height; r++) {
			for (int c = 0; c < width; c++) {
				textureIndex[r][c] = ((uchar*)tex->imageData)[(((int)height - r - 1)*tex->widthStep + c)] / (255 / N_TERRAIN_TEXTURE);
			}
		}
		for (int r = 0; r < height; r++) {
			for (int c = 0; c < width; c++) {
				Color color;
				color.r((float)((uchar*)tex->imageData)[(height - r - 1)*tex->widthStep + c*tex->nChannels + 2] / 255);
				color.g((float)((uchar*)tex->imageData)[(height - r - 1)*tex->widthStep + c*tex->nChannels + 1] / 255);
				color.b((float)((uchar*)tex->imageData)[(height - r - 1)*tex->widthStep + c*tex->nChannels + 0] / 255);
				if (color == COLOR_GRAY75)					textureIndex[r][c] = 0;
				else if (color == COLOR_DARK_GREY_GREEN)	textureIndex[r][c] = 1;
				else if (color == COLOR_SAP_GREEN)			textureIndex[r][c] = 2;
				else if (color == COLOR_LIME)				textureIndex[r][c] = 3;
				else if (color == COLOR_ROSE)				textureIndex[r][c] = 4;
				else if (color == COLOR_LIGHT_YELLOW)		textureIndex[r][c] = 5;
				else if (color == COLOR_BROWN)				textureIndex[r][c] = 6;
				else if (color == COLOR_DARK_VIRIDIAN)		textureIndex[r][c] = 7;
				else textureIndex[r][c] = 0;
			}
		}
		cvReleaseImage(&tex);

		loaded = true;
	}

	Point3D Terrain::getVertexPosition(unsigned int x, unsigned int y) {
		return Point3D(x, y, z[y][x]);
	}
	TexCoord3D Terrain::getTexCoord0(unsigned int x, unsigned int y) {
		const int textureSize = 5;
		return TexCoord(float(x) / textureSize, float(y) / textureSize, (textureIndex[y][x] + 0.5) / N_TERRAIN_TEXTURE);
	}
	TexCoord2D Terrain::getTexCoord1(unsigned int x, unsigned int y) {
		return TexCoord(float(x) / getCols(), float(y) / getRows());
	}

	void Terrain::updateFogOfWarSampler() {
		IplImage* image = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
		for (int r = 0; r < height; r++) {
			for (int c = 0; c < width; c++) {
				uchar value = (1.0 - getFog(c, r)) * 255;
				((uchar*)image->imageData)[r * image->widthStep + c * image->nChannels + 0] = value;
				((uchar*)image->imageData)[r * image->widthStep + c * image->nChannels + 1] = value;
				((uchar*)image->imageData)[r * image->widthStep + c * image->nChannels + 2] = value;
			}
		}
		cvFlip(image, image, 0);
		if (fogOfWarMap != NULL) {
			delete fogOfWarMap;
		}
		fogOfWarMap = new Texture2D();
		fogOfWarMap->make(image, "fogOfWarMap", true, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP);
		cvReleaseImage(&image);
	}
	void Terrain::updateTerrain(bool init) {
		if (init) {
			initSurface(width, height);
			if (!isEditable()) {
				subdivideSurface(2);
			}
			calculateSurfaceTangentNormal();
			groupFaces(64);
			glGenBuffersARB(1, &terrainVBName);
			getLogger().print("Terrain Vertex Buffer initialized.");
		}
		if (init || isEditable()) {
			glBindBufferARB(GL_ARRAY_BUFFER, terrainVBName);
			glBufferDataARB(GL_ARRAY_BUFFER, getVertexBufferSize(), getVertexBuffer(), GL_STATIC_DRAW);
			glGenBuffersARB(1, &terrainFogVBName);
		}
	}
	void Terrain::saveTerrain(string ter_filename, string tex_filename) {
		IplImage *ter = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
		for (int r = 0; r<height; r++)
		for (int c = 0; c<width; c++)
			((uchar*)ter->imageData)[(height - r - 1)*ter->widthStep + c] = ((float)(z[r][c] - MIN_TERRAIN_HEIGHT) / (float)(MAX_TERRAIN_HEIGHT - MIN_TERRAIN_HEIGHT))*256.0;
		cvSaveImage(ter_filename.data(), ter);
		cvReleaseImage(&ter);

		IplImage *tex = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
		for (int r = 0; r<height; r++)
		for (int c = 0; c<width; c++) {
			Color color;
			switch (textureIndex[r][c]) {
			case 0: color = COLOR_GRAY75;				break;
			case 1: color = COLOR_DARK_GREY_GREEN;	break;
			case 2: color = COLOR_SAP_GREEN;			break;
			case 3: color = COLOR_LIME;				break;
			case 4: color = COLOR_ROSE;				break;
			case 5: color = COLOR_LIGHT_YELLOW;		break;
			case 6: color = COLOR_BROWN;				break;
			case 7: color = COLOR_DARK_VIRIDIAN;		break;
			}
			((uchar*)tex->imageData)[(height - r - 1)*tex->widthStep + c*tex->nChannels + 2] = color.r() * 255;
			((uchar*)tex->imageData)[(height - r - 1)*tex->widthStep + c*tex->nChannels + 1] = color.g() * 255;
			((uchar*)tex->imageData)[(height - r - 1)*tex->widthStep + c*tex->nChannels + 0] = color.b() * 255;
		}
		cvSaveImage(tex_filename.data(), tex);
		cvReleaseImage(&tex);
	}
	void Terrain::drawTerrain(bool noShader, bool terrainBumpmapOn, bool shadowOn) {
		if (isTerrainEditMode()) {
			updateTerrain(false);
		}

		float ambient[] = { 0.4, 0.4, 0.4, 1.0 };
		float diffuse[] = { 0.5, 0.5, 0.5, 1.0 };
		float specular[] = { 0.6, 0.6, 0.6, 1.0 };
		float emission[] = { 0.0, 0.0, 0.0, 0.0 };
		glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
		glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
		glMaterialfv(GL_FRONT, GL_EMISSION, emission);
		glMaterialf(GL_FRONT, GL_SHININESS, 0.5);

		//enable gl client states
		glEnable(GL_LIGHTING);
		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CCW);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_FOG);

		if (!isEditable() && !noShader) {
			glActiveTextureARB(TEXTURE_DEFAULT);
		}
		glEnable(GL_TEXTURE_3D);
		terrainTexture.bind();

		if (!isEditable() && !noShader) {
			//init shader
			terrainShader.use();
			glUniform1i(terrainShader.getUniformLocation("clipPlane0Enabled"), glIsEnabled(GL_CLIP_PLANE0));
			glUniform1i(terrainShader.getUniformLocation("colorMap"), SAMPLER_DEFAULT);
			glUniform1i(terrainShader.getUniformLocation("normalMapEnabled"), terrainBumpmapOn ? GL_TRUE : GL_FALSE);
			glUniform1i(terrainShader.getUniformLocation("normalMap"), SAMPLER_NORMAL_MAP);
			glUniform1f(terrainShader.getUniformLocation("lightRadius"), WORLD_RADIUS);
			glUniform1i(terrainShader.getUniformLocation("shadowEnabled"), shadowOn ? GL_TRUE : GL_FALSE);
			glUniform1i(terrainShader.getUniformLocation("shadowMap"), SAMPLER_SHADOW_MAP);
			glUniform1i(terrainShader.getUniformLocation("fogOfWarMap"), SAMPLER_FOG_OF_WAR);
			glUniform1f(terrainShader.getUniformLocation("xPixelOffset"), 1.0 / ((float)shadowMapFrameBuffer.getWidth()));
			glUniform1f(terrainShader.getUniformLocation("yPixelOffset"), 1.0 / ((float)shadowMapFrameBuffer.getHeight()));

			if (terrainBumpmapOn) {
				glActiveTextureARB(TEXTURE_NORMAL_MAP);
				glEnable(GL_TEXTURE_3D);
				terrainBumpTexture.bind();
			}

			if (shadowOn) {
				glActiveTextureARB(TEXTURE_SHADOW_MAP);
				glEnable(GL_TEXTURE_2D);
				shadowMapFrameBuffer.bindDepthTexture();
			}

			glActiveTextureARB(TEXTURE_FOG_OF_WAR);
			glEnable(GL_TEXTURE_2D);
			updateFogOfWarSampler();
			fogOfWarMap->bind();
		}

		glBindBufferARB(GL_ARRAY_BUFFER, terrainVBName);
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, getVertexBufferStride(), getVertexBufferVertexOffset());

		glEnableClientState(GL_NORMAL_ARRAY);
		glNormalPointer(GL_FLOAT, getVertexBufferStride(), getVertexBufferNormalOffset());

		if (!isEditable() && !noShader) {
			glClientActiveTextureARB(TEXTURE_DEFAULT);
		}
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(3, GL_FLOAT, getVertexBufferStride(), getVertexBufferTexCoord0Offset());

		if (!isEditable() && !noShader) {
			if (terrainBumpmapOn) {
				glClientActiveTextureARB(TEXTURE_NORMAL_MAP);
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				glTexCoordPointer(4, GL_FLOAT, getVertexBufferStride(), getVertexBufferTangentOffset());
			}

			glClientActiveTextureARB(TEXTURE_FOG_OF_WAR);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glTexCoordPointer(2, GL_FLOAT, getVertexBufferStride(), getVertexBufferTexCoord1Offset());
		}

		//draw
		for (int i = 0; i < getFaceGroupCount(); i++) {
			if (frustum.sphereInFrustum(getIndexBufferCentre(i), getIndexBufferRadius(i))) {
				glDrawElements(GL_TRIANGLES, getIndexBufferCount(i), GL_UNSIGNED_INT, getIndexBuffer(i));
			}
		}

		//disable gl client states
		if (!isEditable() && !noShader) {
			glClientActiveTextureARB(TEXTURE_FOG_OF_WAR);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			if (terrainBumpmapOn) {
				glClientActiveTextureARB(TEXTURE_NORMAL_MAP);
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			}
			glClientActiveTextureARB(TEXTURE_DEFAULT);
		}
		glDisableClientState(GL_TEXTURE_COORD_ARRAY_EXT);
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);

		if (!isEditable() && !noShader) {
			ShaderProgram::useNone();

			glActiveTextureARB(TEXTURE_FOG_OF_WAR);
			Texture2D::bindNone();
			glDisable(GL_TEXTURE_2D);

			if (shadowOn) {
				glActiveTextureARB(TEXTURE_SHADOW_MAP);
				Texture2D::bindNone();
				glDisable(GL_TEXTURE_2D);
			}

			if (terrainBumpmapOn) {
				glActiveTextureARB(TEXTURE_NORMAL_MAP);
				glBindTexture(GL_TEXTURE_3D, 0);
				glDisable(GL_TEXTURE_3D);
			}

			glActiveTextureARB(TEXTURE_DEFAULT);
		}
		Texture3D::bindNone();
		glDisable(GL_TEXTURE_3D);
		glDisable(GL_FOG);
		glDisable(GL_BLEND);
		glDisable(GL_CULL_FACE);
		glDisable(GL_LIGHTING);

	}
	void Terrain::drawTerrainForShadow() {
		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CW);

		glBindBufferARB(GL_ARRAY_BUFFER, terrainVBName);

		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, getVertexBufferStride(), getVertexBufferVertexOffset());

		glDrawElements(GL_TRIANGLES, getIndexBufferCount(), GL_UNSIGNED_INT, getIndexBuffer());

		glDisableClientState(GL_VERTEX_ARRAY);

		glBindBufferARB(GL_ARRAY_BUFFER, 0);

		glDisable(GL_CULL_FACE);
		glFrontFace(GL_CCW);
	}

	float Terrain::getGroundHeight(int x, int y) const {
		x = clamp(x, 0, width - 1);
		y = clamp(y, 0, height - 1);
		return z[y][x];
	}
	float Terrain::getGroundHeight(float x, float y) const {
		int x1 = clamp(floor(x), 0, width - 1);
		int y1 = clamp(floor(y), 0, height - 1);
		int x2 = clamp(ceil(x), 0, width - 1);
		int y2 = clamp(ceil(y), 0, height - 1);
		float z00 = z[y1][x1];
		float z10 = z[y1][x2];
		float z01 = z[y2][x1];
		float z11 = z[y2][x2];
		float xpos = x - floor(x), ypos = y - floor(y);
		return (1 - ypos)*((1 - xpos)*z00 + (xpos)*z10) + (ypos)*((1 - xpos)*z01 + (xpos)*z11);
	}
	bool Terrain::isWater(int x, int y) const {
		return getGroundHeight(x, y) <= currentWaterLevel;
	}
	bool Terrain::isUneven(int x, int y, float th) const {
		float z00 = getGroundHeight(x - 1, y - 1), z01 = getGroundHeight(x - 1, y), z02 = getGroundHeight(x - 1, y + 1);
		float z10 = getGroundHeight(x, y - 1), z11 = getGroundHeight(x, y), z12 = getGroundHeight(x, y + 1);
		float z20 = getGroundHeight(x + 1, y - 1), z21 = getGroundHeight(x + 1, y), z22 = getGroundHeight(x + 1, y + 1);
		return abs(z11 - z00)>th || abs(z11 - z01)>th || abs(z11 - z02)>th || abs(z11 - z10)>th || abs(z11 - z12)>th || abs(z11 - z20)>th || abs(z11 - z21)>th || abs(z11 - z22)>th;
	}

	void Terrain::increaseHeight(int screen_width, int screen_height, int x, int y, float dh, bool plain, float level) {
		GLint viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);
		glSelectBuffer(SELECT_BUF_SIZE, selectBuf);
		glRenderMode(GL_SELECT);
		glInitNames();
		glPushName(0);

		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		gluPickMatrix((GLdouble)x, (GLdouble)(viewport[3] - y), 10, 10, viewport);
		gluPerspective(45.0, (float)screen_width / screen_height, 0.1, 1000.0);

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		setCamera();
		int obj_name = POS_NAME_START;
		for (float x = 0; x<width - 1; x++) {
			for (float y = 0; y<height - 1; y++) {
				glLoadName(obj_name++);
				glBegin(GL_QUADS);
				glVertex3f(x, y, z[(int)(y)][(int)(x)]);
				glVertex3f(x + 1, y, z[(int)(y)][(int)(x + 1)]);
				glVertex3f(x + 1, y + 1, z[(int)(y + 1)][(int)(x + 1)]);
				glVertex3f(x, y + 1, z[(int)(y + 1)][(int)(x)]);
				glEnd();
			}
		}
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();

		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glFlush();

		GLuint hits = glRenderMode(GL_RENDER);
		GLuint names, *ptr = (GLuint*)selectBuf;
		for (unsigned int i = 0; i<hits; i++) {
			names = *ptr;
			ptr += 3;
			for (unsigned int j = 0; j<names; j++) {
				if (*ptr >= POS_NAME_START) {
					int x = ((*ptr - POS_NAME_START) / (int)(height - 1));
					int y = ((*ptr - POS_NAME_START) % (int)(height - 1));
					if (plain) {
						if (z[y][x]<level) z[y][x] = (z[y][x]<level - dh) ? z[y][x] + dh : level;
						else if (z[y][x]>level) z[y][x] = (z[y][x]>level + dh) ? z[y][x] - dh : level;
					} else
						z[y][x] += dh;
				}
				ptr++;
			}
		}
		glutPostRedisplay();
	}
	void Terrain::setTexture(int screen_width, int screen_height, int x, int y, int t) {
		GLint viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);
		glSelectBuffer(SELECT_BUF_SIZE, selectBuf);
		glRenderMode(GL_SELECT);
		glInitNames();
		glPushName(0);

		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		gluPickMatrix((GLdouble)x, (GLdouble)(viewport[3] - y), 20, 20, viewport);
		gluPerspective(45.0, (float)screen_width / screen_height, 0.1, 1000.0);

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		setCamera();
		int obj_name = POS_NAME_START;
		for (float x = 0; x<width - 1; x++) {
			for (float y = 0; y<height - 1; y++) {
				glLoadName(obj_name++);
				glBegin(GL_QUADS);
				glVertex3f(x, y, z[(int)(y)][(int)(x)]);
				glVertex3f(x + 1, y, z[(int)(y)][(int)(x + 1)]);
				glVertex3f(x + 1, y + 1, z[(int)(y + 1)][(int)(x + 1)]);
				glVertex3f(x, y + 1, z[(int)(y + 1)][(int)(x)]);
				glEnd();
			}
		}
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();

		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glFlush();

		GLuint hits = glRenderMode(GL_RENDER);
		GLuint names, *ptr = (GLuint*)selectBuf;
		for (unsigned int i = 0; i<hits; i++) {
			names = *ptr;
			ptr += 3;
			for (unsigned int j = 0; j<names; j++) {
				if (*ptr >= POS_NAME_START) {
					int x = ((*ptr - POS_NAME_START) / (int)(height - 1));
					int y = ((*ptr - POS_NAME_START) % (int)(height - 1));
					textureIndex[y][x] = t;
				}
				ptr++;
			}
		}
		glutPostRedisplay();
	}

	float Terrain::getWaterLevel(int x, int y) const {
		float level = getGroundHeight(camX, camY);
		if (level < currentWaterLevel) {
			level = currentWaterLevel;
		}
		return level;
	}
}
