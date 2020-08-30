
#include "stdafx.h"

#include "surface.h"

namespace graphics {

	Surface::Surface()
		: cols(0), rows(0), face(NULL), vertex(NULL), subdivided(0), nFaceGroup(0), faceGroup(NULL) {
	}
	Surface::~Surface() {
		if (vertex != NULL) {
			delete[] vertex;
		}
		if (face != NULL) {
			delete[] face;
		}
		if (faceGroup != NULL) {
			delete[] faceGroup;
		}
	}

	//		0 +---<---+ 2		t0 = {0, 1, 3}
	//		  | \  t1 |			t1 = {3, 2, 0}
	//		  v   \   ^
	//		  | t0  \ |
	//		1 +--->---+ 3
	void Surface::setVertexIndex(unsigned int quadIndex, unsigned int vertexIndex, unsigned int value) {
		switch (vertexIndex % 4) {
		case 0:
			face[quadIndex].t[0].v[0] = value;
			face[quadIndex].t[1].v[2] = value;
			break;
		case 1:
			face[quadIndex].t[0].v[1] = value;
			break;
		case 2:
			face[quadIndex].t[1].v[1] = value;
			break;
		default:
			face[quadIndex].t[0].v[2] = value;
			face[quadIndex].t[1].v[0] = value;
			break;
		}
	}
	void Surface::initSurface(unsigned int cols, unsigned int rows) {
		this->cols = cols;
		this->rows = rows;
		vertex = new Vertex[cols * rows];
		for (unsigned int x = 0; x < cols; x++) {
			for (unsigned int y = 0; y < rows; y++) {
				vertex[y * cols + x].vertex = getVertexPosition(x, y);
				vertex[y * cols + x].texcoord0 = getTexCoord0(x, y);
				vertex[y * cols + x].texcoord1 = getTexCoord1(x, y);
			}
		}
		face = new QuadPremitive[(cols - 1) * (rows - 1)];
		for (unsigned int x = 0; x < cols - 1; x++) {
			for (unsigned int y = 0; y < rows - 1; y++) {
				int index = y * (cols - 1) + x;
				setVertexIndex(index, 0, (y + 0) * cols + (x + 0));
				setVertexIndex(index, 1, (y + 0) * cols + (x + 1));
				setVertexIndex(index, 2, (y + 1) * cols + (x + 0));
				setVertexIndex(index, 3, (y + 1) * cols + (x + 1));
			}
		}
	}

	void Surface::calculateSurfaceTangentNormal() {
		Vector3D **surface_normal = new Vector3D*[(int)rows];
		Vector4D **surface_tangent = new Vector4D*[(int)rows];
		for (int r = 0; r<rows; r++) {
			surface_normal[r] = new Vector3D[(int)cols];
			surface_tangent[r] = new Vector4D[(int)cols];
		}

		for (float x = 0; x<cols - 1; x++) {
			for (float y = 0; y<rows - 1; y++) {
				int r = y, c = x;

				Vertex& v0 = vertex[(r + 0) * cols + (c + 0)];
				Vertex& v1 = vertex[(r + 1) * cols + (c + 0)];
				Vertex& v2 = vertex[(r + 0) * cols + (c + 1)];
				Vertex& v3 = vertex[(r + 1) * cols + (c + 1)];

				//calculating surface normal
				surface_normal[r][c] = getNormal(v0.vertex, v2.vertex, v1.vertex);

				//calculating surface tangent
				surface_tangent[r][c] = getTangent(
					v3.vertex, v2.vertex, v1.vertex,
					v3.texcoord0, v2.texcoord0, v1.texcoord0,
					surface_normal[r][c]);
			}
		}

		//calculating vertex normal/tangent
		for (int r = 0; r<rows; r++) {
			for (int c = 0; c<cols; c++) {
				//calculating vertex normal
				Vector3D &n0 = surface_normal[r][c];
				Vector3D &n1 = surface_normal[r][(c + 1<cols) ? (c + 1) : c];
				Vector3D &n2 = surface_normal[(r + 1<rows) ? (r + 1) : r][(c + 1<cols) ? (c + 1) : c];
				Vector3D &n3 = surface_normal[(r + 1<rows) ? (r + 1) : r][c];
				vertex[r * cols + c].normal = (n0 + n1 + n2 + n3).normalize();

				//calculating vertex tangent
				Vector4D &t0 = surface_tangent[r][c];
				Vector4D &t1 = surface_tangent[r][(c + 1<cols) ? (c + 1) : c];
				Vector4D &t2 = surface_tangent[(r + 1<rows) ? (r + 1) : r][(c + 1<cols) ? (c + 1) : c];
				Vector4D &t3 = surface_tangent[(r + 1<rows) ? (r + 1) : r][c];
				vertex[r * cols + c].tangent = (t0 + t1 + t2 + t3) / 4;
			}
		}

		//deallocating temporary storage for calculaing normals/tangents
		for (int r = 0; r<rows; r++) {
			delete[] surface_normal[r];
			delete[] surface_tangent[r];
		}
		delete[] surface_normal;
		delete[] surface_tangent;
	}

	unsigned int Surface::getCols() {
		return cols;
	}
	unsigned int Surface::getRows() {
		return rows;
	}

	void* Surface::getVertexBuffer() {
		return vertex;
	}
	unsigned int Surface::getVertexBufferSize() {
		return cols * rows * sizeof(Vertex);
	}
	unsigned int Surface::getVertexBufferStride() {
		return sizeof(Vertex);
	}

	void* Surface::getVertexBufferVertexOffset() {
		return offsetOf(Vertex, vertex);
	}
	void* Surface::getVertexBufferTexCoord0Offset() {
		return offsetOf(Vertex, texcoord0);
	}
	void* Surface::getVertexBufferTexCoord1Offset() {
		return offsetOf(Vertex, texcoord1);
	}
	void* Surface::getVertexBufferNormalOffset() {
		return offsetOf(Vertex, normal);
	}
	void* Surface::getVertexBufferTangentOffset() {
		return offsetOf(Vertex, tangent);
	}

	void* Surface::getIndexBuffer() {
		return face;
	}
	unsigned int Surface::getIndexBufferCount() {
		return (cols - 1) * (rows - 1) * 6U;
	}

	Point3D Surface::getVertexPosition(unsigned int x, unsigned int y) {
		return Point3D(x, y, 0);
	}
	TexCoord3D Surface::getTexCoord0(unsigned int x, unsigned int y) {
		return TexCoord(x, y);
	}
	TexCoord2D Surface::getTexCoord1(unsigned int x, unsigned int y) {
		return TexCoord(x, y);
	}

	//Subdividing surface using Catmull-Clerk Algorithm
	void Surface::subdivideSurface(unsigned int iter) {
		for (int i = 0; i < iter; i++) {
			//Create new surface
			Surface *newSurf = new Surface();
			newSurf->cols = cols * 2 - 1;
			newSurf->rows = rows * 2 - 1;

			//Create vertex buffer
			try {
				newSurf->vertex = new Vertex[newSurf->cols * newSurf->rows];
			} catch (bad_alloc) {
				showMessage("Can't allocate Vertex[" + toString(newSurf->cols * newSurf->rows) + "]", "bad_alloc", true);
			}
			//Copy original-points
			for (int x = 0; x < newSurf->cols; x += 2) {
				for (int y = 0; y < newSurf->rows; y += 2) {
					newSurf->vertex[y * newSurf->cols + x].vertex = vertex[(y / 2) * cols + (x / 2)].vertex;
					newSurf->vertex[y * newSurf->cols + x].texcoord0 = vertex[(y / 2) * cols + (x / 2)].texcoord0;
					newSurf->vertex[y * newSurf->cols + x].texcoord1 = vertex[(y / 2) * cols + (x / 2)].texcoord1;
				}
			}
			//Calculate face-points
			for (int x = 1; x < newSurf->cols; x += 2) {
				for (int y = 1; y < newSurf->rows; y += 2) {
					Vertex &v = newSurf->vertex[y * newSurf->cols + x];
					//average of immediate original-points
					Vertex &v0 = newSurf->vertex[(y - 1) * newSurf->cols + (x - 1)];
					Vertex &v1 = newSurf->vertex[(y - 1) * newSurf->cols + (x + 1)];
					Vertex &v2 = newSurf->vertex[(y + 1) * newSurf->cols + (x + 1)];
					Vertex &v3 = newSurf->vertex[(y + 1) * newSurf->cols + (x - 1)];
					v.vertex = (v0.vertex + v1.vertex + v2.vertex + v3.vertex) / 4;
					v.texcoord0 = (v0.texcoord0 + v1.texcoord0 + v2.texcoord0 + v3.texcoord0) / 4;
					v.texcoord1 = (v0.texcoord1 + v1.texcoord1 + v2.texcoord1 + v3.texcoord1) / 4;
				}
			}
			//Calculate Vertical Edge-points
			for (int x = 0; x < newSurf->cols; x += 2) {
				for (int y = 1; y < newSurf->rows; y += 2) {
					Vertex &v = newSurf->vertex[y * newSurf->cols + x];
					//average of immediate original-points & immediate face-points
					Vertex &v0 = newSurf->vertex[(y - 1) * newSurf->cols + x];
					Vertex &v1 = newSurf->vertex[(y + 1) * newSurf->cols + x];
					v.vertex = v0.vertex + v1.vertex;
					v.texcoord0 = v0.texcoord0 + v1.texcoord0;
					v.texcoord1 = v0.texcoord1 + v1.texcoord1;
					int n = 2;
					if (x - 1 >= 0 && x + 1 < newSurf->getCols()) {
						Vertex &f0 = newSurf->vertex[y * newSurf->cols + (x + 1)];
						Vertex &f1 = newSurf->vertex[y * newSurf->cols + (x - 1)];
						v.vertex += f0.vertex + f1.vertex;
						v.texcoord0 += f0.texcoord0 + f1.texcoord0;
						v.texcoord1 += f0.texcoord1 + f1.texcoord1;
						n += 2;
					}
					v.vertex /= n;
					v.texcoord0 /= n;
					v.texcoord1 /= n;
				}
			}
			//Calculate Horizontal Edge-points
			for (int x = 1; x < newSurf->cols; x += 2) {
				for (int y = 0; y < newSurf->rows; y += 2) {
					Vertex &v = newSurf->vertex[y * newSurf->cols + x];
					//average of immediate original-points & immediate face-points
					Vertex &v0 = newSurf->vertex[y * newSurf->cols + (x + 1)];
					Vertex &v1 = newSurf->vertex[y * newSurf->cols + (x - 1)];
					v.vertex = v0.vertex + v1.vertex;
					v.texcoord0 = v0.texcoord0 + v1.texcoord0;
					v.texcoord1 = v0.texcoord1 + v1.texcoord1;
					int n = 2;
					if (y - 1 >= 0 && y + 1 < newSurf->getRows()) {
						Vertex &f0 = newSurf->vertex[(y - 1) * newSurf->cols + x];
						Vertex &f1 = newSurf->vertex[(y + 1) * newSurf->cols + x];
						v.vertex += f0.vertex + f1.vertex;
						v.texcoord0 += f0.texcoord0 + f1.texcoord0;
						v.texcoord1 += f0.texcoord1 + f1.texcoord1;
						n += 2;
					}
					v.vertex /= n;
					v.texcoord0 /= n;
					v.texcoord1 /= n;
				}
			}
			//Recalculate original-points
			for (int x = 0; x < newSurf->cols; x += 2) {
				for (int y = 0; y < newSurf->rows; y += 2) {
					//F = average of immediate face-points
					int nFacePoints = 0;
					Point3D F(0, 0, 0);
					if (x - 1 >= 0 && y - 1 >= 0) {
						F += newSurf->vertex[(y - 1) * newSurf->cols + (x - 1)].vertex;
						nFacePoints++;
					}
					if (x + 1 < newSurf->getCols() && y - 1 >= 0) {
						F += newSurf->vertex[(y - 1) * newSurf->cols + (x + 1)].vertex;
						nFacePoints++;
					}
					if (x + 1 < newSurf->getCols() && y + 1 < newSurf->getRows()) {
						F += newSurf->vertex[(y + 1) * newSurf->cols + (x + 1)].vertex;
						nFacePoints++;
					}
					if (x - 1 >= 0 && y + 1 < newSurf->getRows()) {
						F += newSurf->vertex[(y + 1) * newSurf->cols + (x - 1)].vertex;
						nFacePoints++;
					}
					F /= nFacePoints;
					//R = average of immediate edge-points
					int nEdgePoints = 0;
					Point3D R(0, 0, 0);
					if (x - 1 >= 0) {
						R += newSurf->vertex[y * newSurf->cols + (x - 1)].vertex;
						nEdgePoints++;
					}
					if (x + 1 < newSurf->getCols()) {
						R += newSurf->vertex[y * newSurf->cols + (x + 1)].vertex;
						nEdgePoints++;
					}
					if (y - 1 >= 0) {
						R += newSurf->vertex[(y - 1) * newSurf->cols + x].vertex;
						nEdgePoints++;
					}
					if (y + 1 < newSurf->getRows()) {
						R += newSurf->vertex[(y + 1) * newSurf->cols + x].vertex;
						nEdgePoints++;
					}
					R /= nEdgePoints;
					//P = original--point
					Point3D P = newSurf->vertex[y * newSurf->cols + x].vertex;
					//weighted bary-centre of F, R, P
					newSurf->vertex[y * newSurf->cols + x].vertex = (F + R * 2 + P) / 4;
				}
			}

			//Create index buffer
			try {
				newSurf->face = new QuadPremitive[(newSurf->cols - 1) * (newSurf->rows - 1)];
			} catch (bad_alloc) {
				showMessage("Can't allocate QuadPremitive[" + toString((newSurf->cols - 1) * (newSurf->rows - 1)) + "]", "bad_alloc", true);
			}
			for (unsigned int x = 0; x < newSurf->cols - 1; x++) {
				for (unsigned int y = 0; y < newSurf->rows - 1; y++) {
					unsigned int index = y * (newSurf->cols - 1) + x;
					newSurf->setVertexIndex(index, 0, (y + 0) * newSurf->cols + (x + 0));
					newSurf->setVertexIndex(index, 1, (y + 0) * newSurf->cols + (x + 1));
					newSurf->setVertexIndex(index, 2, (y + 1) * newSurf->cols + (x + 0));
					newSurf->setVertexIndex(index, 3, (y + 1) * newSurf->cols + (x + 1));
				}
			}

			//replace everything
			cols = newSurf->cols;
			rows = newSurf->rows;
			delete[] vertex;
			vertex = newSurf->vertex;
			delete[] face;
			face = newSurf->face;

			subdivided++;
		}
	}

	//Group indexes for frustum culling
	void Surface::groupFaces(unsigned int groupSize) {
		this->groupSize = groupSize;
		QuadPremitive *groupedFace = new QuadPremitive[(cols - 1) * (rows - 1)];
		unsigned int groupCols = ceil(float(cols - 1) / groupSize);
		unsigned int groupRows = ceil(float(rows - 1) / groupSize);
		nFaceGroup = groupCols * groupRows;
		faceGroup = new FaceGroup[nFaceGroup];

		//group faces
		unsigned int offset = 0;
		for (unsigned int yo = 0; yo < groupRows; yo++) {
			for (unsigned int xo = 0; xo < groupCols; xo++) {
				unsigned int groupIndex = yo * groupCols + xo;
				faceGroup[groupIndex].nFaces = 0;
				faceGroup[groupIndex].ptr = &groupedFace[offset];
				unsigned int groupWidth = min(groupSize, (cols - 1 - xo * groupSize));
				unsigned int groupHeight = min(groupSize, (rows - 1 - yo * groupSize));
				for (unsigned int yi = 0; yi < groupHeight; yi++) {
					for (unsigned int xi = 0; xi < groupWidth; xi++) {
						faceGroup[groupIndex].ptr[yi * groupWidth + xi] = face[(yo * groupSize + yi) * (cols - 1) + (xo * groupSize + xi)];
						faceGroup[groupIndex].nFaces++;
					}
				}
				//centre
				Point3D sum = Point3D(0, 0, 0);
				for (unsigned int yi = 0; yi < groupHeight; yi++) {
					for (unsigned int xi = 0; xi < groupWidth; xi++) {
						sum = sum + vertex[(yo * groupSize + yi) * cols + (xo * groupSize + xi)].vertex;
					}
				}
				faceGroup[groupIndex].centre = sum / faceGroup[groupIndex].nFaces;
				//radius
				faceGroup[groupIndex].radius = 0;
				for (unsigned int yi = 0; yi < groupHeight; yi++) {
					for (unsigned int xi = 0; xi < groupWidth; xi++) {
						float d = dist(faceGroup[groupIndex].centre, vertex[(yo * groupSize + yi) * cols + (xo * groupSize + xi)].vertex);
						faceGroup[groupIndex].radius = max(faceGroup[groupIndex].radius, d);
					}
				}
				offset += faceGroup[groupIndex].nFaces;
			}
		}

		//replace faces
		delete[] face;
		face = groupedFace;
	}

	unsigned int Surface::getFaceGroupSize() {
		return groupSize;
	}
	unsigned int Surface::getFaceGroupCount() {
		return nFaceGroup;
	}
	void* Surface::getIndexBuffer(unsigned int groupIndex) {
		return faceGroup[groupIndex].ptr;
	}
	unsigned int Surface::getIndexBufferCount(unsigned int groupIndex) {
		return faceGroup[groupIndex].nFaces * 6;
	}
	Point3D Surface::getIndexBufferCentre(unsigned int groupIndex) {
		return faceGroup[groupIndex].centre;
	}
	float Surface::getIndexBufferRadius(unsigned int groupIndex) {
		return faceGroup[groupIndex].radius;
	}
	
}
