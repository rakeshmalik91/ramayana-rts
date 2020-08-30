#ifndef __SURFACE_H
#define __SURFACE_H

#include "math.h"
#include "graphics.h"

namespace graphics {

	class Surface {
		struct Vertex {
			Point3D vertex;
			TexCoord3D texcoord0;
			TexCoord2D texcoord1;
			Vector3D normal;
			Vector4D tangent;
		};
		struct TriadPremitive {
			unsigned int v[3];
		};
		struct QuadPremitive {
			TriadPremitive t[2];
		};

		unsigned int rows, cols;
		unsigned int subdivided;
		Vertex *vertex;
		QuadPremitive *face;
		struct FaceGroup {
			QuadPremitive *ptr;
			Point3D centre;
			float radius;
			unsigned int nFaces;
		} *faceGroup;
		unsigned int nFaceGroup, groupSize;
		void setVertexIndex(unsigned int, unsigned int, unsigned int);
		float getInterpolatedFog(float x, float y);
	public:
		Surface();
		void initSurface(unsigned int cols, unsigned int rows);
		void calculateSurfaceTangentNormal();
		~Surface();
		unsigned int getCols();
		unsigned int getRows();
		void* getVertexBuffer();
		unsigned int getVertexBufferSize();
		unsigned int getVertexBufferStride();
		void* getVertexBufferVertexOffset();
		void* getVertexBufferTexCoord0Offset();
		void* getVertexBufferTexCoord1Offset();
		void* getVertexBufferNormalOffset();
		void* getVertexBufferTangentOffset();
		void* getIndexBuffer();
		unsigned int getIndexBufferCount();
		unsigned int getFaceGroupSize();
		void subdivideSurface(unsigned int iter = 1);
		virtual Point3D getVertexPosition(unsigned int x, unsigned int y);
		virtual TexCoord3D getTexCoord0(unsigned int x, unsigned int y);
		virtual TexCoord2D getTexCoord1(unsigned int x, unsigned int y);
		void groupFaces(unsigned int groupSize);
		unsigned int getFaceGroupCount();
		void* getIndexBuffer(unsigned int groupIndex);
		unsigned int getIndexBufferCount(unsigned int groupIndex);
		Point3D getIndexBufferCentre(unsigned int groupIndex);
		float getIndexBufferRadius(unsigned int groupIndex);
	};
}

#endif