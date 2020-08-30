/*****************************************************************************************************
* Subject                   : WAVEFRPMT (*.OBJ) Loader                                              *
* Author                    : Rakesh Malik                                                          *
*****************************************************************************************************/

#include "stdafx.h"

#include "graphics.h"

namespace graphics {

	struct Material {
		string name;
		Color ambient, specular, diffuse, emission;
		float shininess, sharpness;
		unsigned char illum;
		Texture2D *texDiff, *texNorm;
		Material(string);
		bool operator==(Material&);
		bool operator!=(Material&);
		void write(ostream&, vector<Texture2D*>);
		void read(istream&, vector<Texture2D*>);
	};

	//Constructor
	Material::Material(string name = "") {
		this->name = name;
		this->diffuse.set(0.5, 0.5, 0.5);
		this->specular.set(0.3, 0.3, 0.3);
		this->shininess = 1.0;
		this->illum = 0.0;
		this->sharpness = 0.0;
		texDiff = texNorm = NULL;
	}
	//Checks if 2 materials are equal
	bool Material::operator==(Material& m) {
		return this->name == m.name;
	}
	//Checks if 2 materials are not equal
	bool Material::operator!=(Material& m) {
		return this->name != m.name;
	}
	//
	void Material::write(ostream &out, vector<Texture2D*> texLib) {
		//attributes
		out.write((const char*)&ambient, sizeof(ambient));
		out.write((const char*)&specular, sizeof(specular));
		out.write((const char*)&diffuse, sizeof(diffuse));
		out.write((const char*)&emission, sizeof(emission));
		out.write((const char*)&shininess, sizeof(shininess));
		out.write((const char*)&sharpness, sizeof(sharpness));
		out.write((const char*)&illum, sizeof(illum));
		//diffuse texture index
		int texDiffIndex = 0;
		for (; texDiffIndex < texLib.size(); texDiffIndex++) {
			if (texLib[texDiffIndex] == texDiff) {
				break;
			}
		}
		out.write((const char*)&texDiffIndex, sizeof(texDiffIndex));
		//normal texture index
		int texNormIndex = 0;
		for (; texNormIndex < texLib.size(); texNormIndex++) {
			if (texLib[texNormIndex] == texNorm) {
				break;
			}
		}
		out.write((const char*)&texNormIndex, sizeof(texNormIndex));
	}
	//
	void Material::read(istream &out, vector<Texture2D*> texLib) {
		//attributes
		out.read((char*)&ambient, sizeof(ambient));
		out.read((char*)&specular, sizeof(specular));
		out.read((char*)&diffuse, sizeof(diffuse));
		out.read((char*)&emission, sizeof(emission));
		out.read((char*)&shininess, sizeof(shininess));
		out.read((char*)&sharpness, sizeof(sharpness));
		out.read((char*)&illum, sizeof(illum));
		//diffuse texture index
		int texDiffIndex = 0;
		out.read((char*)&texDiffIndex, sizeof(texDiffIndex));
		if (texDiffIndex < texLib.size()) {
			texDiff = texLib[texDiffIndex];
		} else {
			texDiff = NULL;
		}
		//normal texture index
		int texNormIndex = 0;
		out.read((char*)&texNormIndex, sizeof(texNormIndex));
		if (texNormIndex < texLib.size()) {
			texNorm = texLib[texNormIndex];
		} else {
			texNorm = NULL;
		}
	}

	class MaterialLibrary {
		Material dummy;
		vector<Texture2D*> tex;
		vector<Material> lib;
	public:
		MaterialLibrary() {}
		void insert(const string&);
		unsigned char getIndex(const string&);
		Material& get(unsigned char);
		Material& operator[](unsigned char);
		Material& operator[](const string&);
		void generateTextures();
		unsigned char size();
		void loadTexture(string, string);
		void loadBumpTexture(string, string);
		~MaterialLibrary();
		void write(ostream&);
		void read(istream&, string texturePath);
	};

	//Inserts new material
	void MaterialLibrary::insert(const string& mtlname) {
		lib.push_back(Material(mtlname));
	}
	//Access  i-th material
	Material& MaterialLibrary::get(unsigned char i) {
		return lib[i];
	}
	//Access  i-th material
	Material& MaterialLibrary::operator[](unsigned char i) {
		return lib[i];
	}
	//Access mtlname-named material
	Material& MaterialLibrary::operator[](const string& mtlname) {
		for (int i = 0; i<lib.size(); i++)
		if (lib[i].name == mtlname)
			return lib[i];
		return dummy;
	}
	//Returns index for material name mtlname
	unsigned char MaterialLibrary::getIndex(const string& mtlname) {
		for (int i = 0; i<lib.size(); i++)
		if (lib[i].name == mtlname)
			return i;
		return -1;
	}
	//Generate textures for all loaded textures of all materials
	void MaterialLibrary::generateTextures() {
		for (vector<Material>::iterator i = lib.begin(); i != lib.end(); i++) {
			if (i->texDiff != NULL)		i->texDiff->generate();
			if (i->texNorm != NULL)		i->texNorm->generate();
		}
	}
	//Returns number of matrial
	unsigned char MaterialLibrary::size() {
		return lib.size();
	}
	//Load Diffuse texture
	void MaterialLibrary::loadTexture(string mtlname, string fname) {
		int texi = -1;
		for (int i = 0; i<tex.size(); i++)
		if (tex[i]->path == fname) {
			texi = i;
			break;
		}
		int mtli = getIndex(mtlname);
		if (texi >= 0) {
			lib[mtli].texDiff = tex[texi];
		} else {
			tex.push_back(new Texture2D());
			try {
				tex[tex.size() - 1]->load(fname.data(), false, GL_LINEAR, GL_LINEAR_MIPMAP_NEAREST, GL_REPEAT, GL_REPEAT);
			} catch (FileNotFoundException &ex) {
				tex[tex.size() - 1]->path = fname;
			}
			lib[mtli].texDiff = tex[tex.size() - 1];
		}
	}
	//Load Normal map texture
	void MaterialLibrary::loadBumpTexture(string mtlname, string fname) {
		int texi = -1;
		for (int i = 0; i<tex.size(); i++)
		if (tex[i]->path == fname) {
			texi = i;
			break;
		}
		int mtli = getIndex(mtlname);
		if (texi >= 0) {
			lib[mtli].texNorm = tex[texi];
		} else {
			tex.push_back(new Texture2D());
			try {
				tex[tex.size() - 1]->load(fname.data(), false, GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT);
			} catch (FileNotFoundException &ex) {
				tex[tex.size() - 1]->path = fname;
			}
			lib[mtli].texNorm = tex[tex.size() - 1];
		}
	}
	//Destructor
	MaterialLibrary::~MaterialLibrary() {
		for (int i = 0; i<tex.size(); i++)
			delete tex[i];
	}
	//
	void MaterialLibrary::write(ostream &out) {
		//number of textures
		int nTex = tex.size();
		out.write((const char*)&nTex, sizeof(nTex));
		//textures
		for (int i = 0; i<nTex; i++) {
			writeStringAsBinary(out, removeFilePath(tex[i]->path));
		}
		//number of materials
		int nMaterials = lib.size();
		out.write((const char*)&nMaterials, sizeof(nMaterials));
		//materials
		for (int i = 0; i < nMaterials; i++) {
			lib[i].write(out, tex);
		}
	}
	//
	void MaterialLibrary::read(istream &in, string texturePath) {
		//number of textures
		int nTex = 0;
		in.read((char*)&nTex, sizeof(nTex));
		//textures
		tex.resize(nTex);
		for (int i = 0; i<nTex; i++) {
			string texName;
			texName = readStringAsBinary(in);
			tex[i] = new Texture2D();
			tex[i]->load((texturePath + "/" + texName).c_str(), false, GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT);
		}
		//number of materials
		int nMaterials = 0;
		in.read((char*)&nMaterials, sizeof(nMaterials));
		//materials
		lib.resize(nMaterials);
		for (int i = 0; i < nMaterials; i++) {
			lib[i].read(in, tex);
		}
	}

	Texture2D *__nocolor = NULL, *__nobump = NULL;
	void createNoColorTexture() {
		IplImage *img;

		img = cvCreateImage(cvSize(1, 1), IPL_DEPTH_8U, 3);
		((uchar*)img->imageData)[2] = 255;
		((uchar*)img->imageData)[1] = 255;
		((uchar*)img->imageData)[0] = 255;
		__nocolor = new Texture2D();
		__nocolor->make(img, "nocolor", true, GL_NEAREST, GL_NEAREST, GL_CLAMP, GL_CLAMP);
		cvReleaseImage(&img);

		img = cvCreateImage(cvSize(1, 1), IPL_DEPTH_8U, 3);
		((uchar*)img->imageData)[2] = 127;
		((uchar*)img->imageData)[1] = 127;
		((uchar*)img->imageData)[0] = 255;
		__nobump = new Texture2D();
		__nobump->make(img, "nobump", true, GL_NEAREST, GL_NEAREST, GL_CLAMP, GL_CLAMP);
		cvReleaseImage(&img);
	}

	//Default Constructor
	WaveFrontObj::WaveFrontObj()
		: loaded(false), compiled(false), mtlLib(NULL), fname(""), ownMaterialLibrary(false),
		  xMin(0.0), xMid(0.0), xMax(0.0), yMin(0.0), yMid(0.0), yMax(0.0), zMin(0.0), zMid(0.0), zMax(0.0),
		  radius(0.0), radiusX(0.0), radiusY(0.0), radiusZ(0.0) {
	}
	//Triangle Comparator
	int triangle_comparator(const WaveFrontObj::Triangle &t1, const WaveFrontObj::Triangle &t2, void* unused) {
		float cgz1 = (t1.v[0].v.z + t1.v[1].v.z + t1.v[2].v.z) / 3;
		float cgz2 = (t2.v[0].v.z + t2.v[1].v.z + t2.v[2].v.z) / 3;
		if (cgz1<cgz2) {
			return -1;
		} else if (cgz1>cgz2) {
			return 1;
		} else {
			return 0;
		}
	}
	//Reads unitTypeInfo
	void WaveFrontObj::readOBJ() {
		vector<Point3D> vertex, normal;
		vector<Point2D> texcoord;
		ifstream in(fname.data());
		if (in.fail()) {
			throw FileNotFoundException(fname);
		}
		unsigned char current_mtl;
		while (!in.eof()) {
			string option = "", args = "";
			for (char c = in.get(); c != ' ' && c != '\n' && c != EOF; c = in.get()) {
				option += c;
			}
			if (option.size() == 0) {
				continue;
			}
			for (char c = in.get(); c != '\n' && c != EOF; c = in.get()) {
				args += c;
			}
			if (option[0] != '#') {
				if (option == "v") {							//vertex
					Point3D v;
					v.x = atof(getFirstField(args, ' ').data());
					v.y = atof(getFirstField(args, ' ').data());
					v.z = atof(getFirstField(args, ' ').data());
					vertex.push_back(v);
				} else if (option == "f") {						//face definition
					Face f;
					while (!args.empty()) {
						string field = getFirstField(args, ' ');
						int v = atoi(getFirstField(field, '/').data()), vt = 0, vn = 0;
						if (field.size()>0) {
							vt = atoi(getFirstField(field, '/').data());
						}
						if (field.size()>0) {
							vn = atoi(getFirstField(field, '/').data());
						}
						f.add(v>0 ? (v - 1) : 0, vt>0 ? (vt - 1) : 0, vn>0 ? (vn - 1) : 0);
						f.mtl = current_mtl;
					}
					if (f.numberOfVertices() >= 3) {
						group[current_mtl].vert.push_back(WaveFrontObj::Group::Vertex(vertex[f.v[0]], normal[f.vn[0]], texcoord.size()>0 ? texcoord[f.vt[0]] : Point2D()));
						group[current_mtl].vert.push_back(WaveFrontObj::Group::Vertex(vertex[f.v[1]], normal[f.vn[1]], texcoord.size()>0 ? texcoord[f.vt[1]] : Point2D()));
						group[current_mtl].vert.push_back(WaveFrontObj::Group::Vertex(vertex[f.v[2]], normal[f.vn[2]], texcoord.size()>0 ? texcoord[f.vt[2]] : Point2D()));
					}
				} else if (option == "vt") {					//texture coordinate
					Point2D vt;
					vt.x = atof(getFirstField(args, ' ').data());
					vt.y = atof(getFirstField(args, ' ').data());
					texcoord.push_back(vt);
				} else if (option == "vn") {					//normal coordinate
					Point3D vn;
					vn.x = atof(getFirstField(args, ' ').data());
					vn.y = atof(getFirstField(args, ' ').data());
					vn.z = atof(getFirstField(args, ' ').data());
					normal.push_back(vn);
				} else if (option == "usemtl") {				//used material name
					current_mtl = mtlLib->getIndex(args);
				} else if (option == "mtllib") {				//material library path
					this->mtlLibName = getFilePath(fname) + "/" + args;
					if (mtlLib == NULL) {
						this->readMTL();
					}
					group.resize(mtlLib->size());
				} else if (option == "g") {					//group name
					//Do nothing...
				} else if (option == "o") {					//unitTypeInfo name
					//Do nothing...
				}
			}
		}
		in.close();

		//Sort Triangles by Z
		for (int g = 0; g<group.size(); g++)
			quicksort((WaveFrontObj::Triangle*)group[g].vert.data(), group[g].vert.size() / 3, &triangle_comparator);

		//Calculating constants
		xMax = yMax = zMax = -FLT_MAX;
		xMin = yMin = zMin = FLT_MAX;
		for (int i = 0; i<vertex.size(); i++) {
			if (vertex[i].x>xMax) xMax = vertex[i].x;
			if (vertex[i].y>yMax) yMax = vertex[i].y;
			if (vertex[i].z>zMax) zMax = vertex[i].z;
			if (vertex[i].x<xMin) xMin = vertex[i].x;
			if (vertex[i].y<yMin) yMin = vertex[i].y;
			if (vertex[i].z<zMin) zMin = vertex[i].z;
		}
		xMid = (xMax + xMin) / 2;
		yMid = (yMax + yMin) / 2;
		zMid = (zMax + zMin) / 2;
		radiusX = radiusY = radiusZ = 0;
		for (int i = 0; i<vertex.size(); i++) {
			float dx = sqrt((vertex[i].z - zMid)*(vertex[i].z - zMid) + (vertex[i].y - yMid)*(vertex[i].y - yMid));
			float dy = sqrt((vertex[i].x - xMid)*(vertex[i].x - xMid) + (vertex[i].z - zMid)*(vertex[i].z - zMid));
			float dz = sqrt((vertex[i].x - xMid)*(vertex[i].x - xMid) + (vertex[i].y - yMid)*(vertex[i].y - yMid));
			if (dx>radiusX) radiusX = dx;
			if (dy>radiusY) radiusY = dy;
			if (dz>radiusZ) radiusZ = dz;
		}
		radius = max(max(radiusX, radiusY), radiusZ);

		//Calculate Surface Tangent
		for (int g = 0; g<group.size(); g++) {
			for (int v = 0; v<group[g].vert.size(); v += 3) {
				Vector3D normal = getNormal(group[g].vert[v].v, group[g].vert[v + 1].v, group[g].vert[v + 2].v);
				Vector4D tangent = getTangent(group[g].vert[v].v, group[g].vert[v + 1].v, group[g].vert[v + 2].v, group[g].vert[v].vt, group[g].vert[v + 1].vt, group[g].vert[v + 2].vt, normal);
				group[g].vert[v].tangent = group[g].vert[v + 1].tangent = group[g].vert[v + 2].tangent = tangent;
			}
		}

		//Calculate Vertex Tangent
		for (int g = 0; g<group.size(); g++) {
			for (int i = 0; i<vertex.size(); i++) {
				Vector4D vertexTangent = Vector4D(0, 0, 0, 0);
				int nTangents = 0;
				for (int v = 0; v<group[g].vert.size(); v++) {
					if (vertex[i] == group[g].vert[v].v) {
						vertexTangent = vertexTangent + group[g].vert[v].tangent;
						nTangents++;
					}
				}
				if (nTangents>0) {
					vertexTangent = vertexTangent / nTangents;
					for (int v = 0; v<group[g].vert.size(); v++) {
						if (vertex[i] == group[g].vert[v].v) {
							group[g].vert[v].tangent = vertexTangent;
						}
					}
				}
			}
		}

		loaded = true;
	}
	//Read metatial library
	void WaveFrontObj::readMTL() {
		mtlLib = new MaterialLibrary();
		if (this->mtlLibName == "")
			return;
		ifstream in(this->mtlLibName.data());
		if (in.fail())
			throw FileNotFoundException(this->mtlLibName);
		string mtlname;
		while (!in.eof()) {
			string option = "", args = "";
			for (char c = in.get(); c != ' ' && c != '\n' && c != EOF; c = in.get())
				option += c;
			if (option.size() == 0)
				continue;
			for (char c = in.get(); c != '\n' && c != EOF; c = in.get())
				args += c;
			if (option[0] != '#') {
				if (option == "newmtl") {						//matreial name
					mtlname = args;
					this->mtlLib->insert(mtlname);
				} else if (option == "Ka") {					//ambient light color
					(*this->mtlLib)[mtlname].ambient.r(atof(getFirstField(args, ' ').data()));
					(*this->mtlLib)[mtlname].ambient.g(atof(getFirstField(args, ' ').data()));
					(*this->mtlLib)[mtlname].ambient.b(atof(getFirstField(args, ' ').data()));
				} else if (option == "Kd") {					//diffuse light color
					(*this->mtlLib)[mtlname].diffuse.r(atof(getFirstField(args, ' ').data()));
					(*this->mtlLib)[mtlname].diffuse.g(atof(getFirstField(args, ' ').data()));
					(*this->mtlLib)[mtlname].diffuse.b(atof(getFirstField(args, ' ').data()));
				} else if (option == "Ks") {					//specular light color
					(*this->mtlLib)[mtlname].specular.r(atof(getFirstField(args, ' ').data()));
					(*this->mtlLib)[mtlname].specular.g(atof(getFirstField(args, ' ').data()));
					(*this->mtlLib)[mtlname].specular.b(atof(getFirstField(args, ' ').data()));
				} else if (option == "Ke") {					//emission light color
					(*this->mtlLib)[mtlname].emission.r(atof(getFirstField(args, ' ').data()));
					(*this->mtlLib)[mtlname].emission.g(atof(getFirstField(args, ' ').data()));
					(*this->mtlLib)[mtlname].emission.b(atof(getFirstField(args, ' ').data()));
				} else if (option == "d" || option == "Tr") {	//alpha
					float alpha = atof(getFirstField(args, ' ').data());
					(*this->mtlLib)[mtlname].ambient.a(alpha);
					(*this->mtlLib)[mtlname].diffuse.a(alpha);
					(*this->mtlLib)[mtlname].specular.a(alpha);
					(*this->mtlLib)[mtlname].emission.a(alpha);
				} else if (option == "illum") {				//illum
					(*this->mtlLib)[mtlname].illum = atoi(getFirstField(args, ' ').data());
				} else if (option == "Ns") {					//shininess
					(*this->mtlLib)[mtlname].shininess = atof(getFirstField(args, ' ').data());
				} else if (option == "Ni") {					//
					//...
				} else if (option == "sharpness") {			//sharpness
					(*this->mtlLib)[mtlname].sharpness = atof(getFirstField(args, ' ').data());
				} else if (option == "map_Kd") {				//diffuse texture
					this->mtlLib->loadTexture(mtlname, getFilePath(fname) + "/" + getFirstField(args, '\n'));
				} else if (option == "map_Ks") {				//specular texture
					//not supported
				} else if (option == "map_d") {				//...
					//not supported
				} else if (option == "map_Bump") {				//bump texture
					this->mtlLib->loadBumpTexture(mtlname, getFilePath(fname) + "/" + getFirstField(args, '\n'));
				}
			}
		}
		in.close();
	}
	//Build Vertex buffer
	void WaveFrontObj::buildVBO() {
		for (int g = 0; g<group.size(); g++) {
			glGenBuffersARB(1, &group[g].vbName);
			glBindBufferARB(GL_ARRAY_BUFFER, group[g].vbName);
			group[g].nVert = group[g].vert.size();
			GLvoid* data = (group[g].nVert>0) ? &group[g].vert[0] : NULL;
			glBufferDataARB(GL_ARRAY_BUFFER, sizeof(WaveFrontObj::Group::Vertex)*group[g].vert.size(), data, GL_STATIC_DRAW);
			glBindBufferARB(GL_ARRAY_BUFFER, 0);
			//group[g].vert.~vector<WaveFrontObj::Group::Vertex>();
			group[g].vert.clear();
		}
		glBindBufferARB(GL_ARRAY_BUFFER, 0);
	}
	//Loads from file, *.obj filename to be given
	void WaveFrontObj::load(string fname, bool toBeCompiled, MaterialLibrary *mtlLib) {
		if (loaded) {
			return;
		}

		this->fname = fname;

		//Copy matrial library if given
		if (mtlLib != NULL) {
			this->mtlLib = mtlLib;
			ownMaterialLibrary = false;
		} else {
			ownMaterialLibrary = true;
		}

		//Loads mtl, obj and makes bin
		this->readOBJ();

		//Generate loaded textures and Make VBO
		if (toBeCompiled) {
			compile();
		}
	}
	//Generte sall loaded textures of the material library
	void WaveFrontObj::compile() {
		//create no bump texture
		if (__nobump == NULL) {
			createNoColorTexture();
		}

		if (loaded && !compiled) {
			//generate textures
			if (ownMaterialLibrary) {
				mtlLib->generateTextures();
			}

			//build vertex buffer
			buildVBO();

			compiled = true;
		}
	}
	//Renders the unitTypeInfo (from gl unitTypeInfo list if compiled)
	void WaveFrontObj::render(Color varColor, Color tint) {
		bool lightingEnabled = glIsEnabled(GL_LIGHTING);
		glActiveTextureARB(GL_TEXTURE1);
		bool normalMapped = glIsEnabled(GL_TEXTURE_2D);
		glActiveTextureARB(GL_TEXTURE0);
		bool textured = glIsEnabled(GL_TEXTURE_2D);

		float buffer[4];
		for (int g = 0; g<group.size(); g++) {
			if (lightingEnabled) {
				glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, (mtlLib->get(g).ambient * tint).toArray(buffer));
				if (mtlLib->get(g).diffuse.rgb() == COLOR_BLACK.rgb()) {
					glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, (varColor * tint).toArray(buffer));
				} else {
					glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, (mtlLib->get(g).diffuse * tint).toArray(buffer));
				}
				glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, (mtlLib->get(g).specular * tint).toArray(buffer));
				glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, (mtlLib->get(g).emission * tint).toArray(buffer));
				glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mtlLib->get(g).shininess);
			} else {
				if (mtlLib->get(g).diffuse.rgb() == COLOR_BLACK.rgb()) {
					glColor4fv((mtlLib->get(g).diffuse * tint).toArray(buffer));
				} else {
					glColor4fv((varColor * tint).toArray(buffer));
				}
			}

			if (textured) {
				glActiveTextureARB(GL_TEXTURE0);
				if (mtlLib->get(g).texDiff != NULL) {
					mtlLib->get(g).texDiff->bind();
				} else {
					__nocolor->bind();
				}
			}

			if (normalMapped) {
				glActiveTextureARB(GL_TEXTURE1);
				if (mtlLib->get(g).texNorm != NULL) {
					mtlLib->get(g).texNorm->bind();
				} else {
					__nobump->bind();
				}
			}

			glBindBufferARB(GL_ARRAY_BUFFER, group[g].vbName);

			glEnableClientState(GL_VERTEX_ARRAY);
			glVertexPointer(3, GL_FLOAT, sizeof(WaveFrontObj::Group::Vertex), (GLvoid*)0);

			if (lightingEnabled) {
				glEnableClientState(GL_NORMAL_ARRAY);
				glNormalPointer(GL_FLOAT, sizeof(WaveFrontObj::Group::Vertex), (GLvoid*)(sizeof(Point3D)));
			}

			if (textured) {
				glClientActiveTextureARB(GL_TEXTURE0);
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				glTexCoordPointer(2, GL_FLOAT, sizeof(WaveFrontObj::Group::Vertex), (GLvoid*)(sizeof(Point3D)* 2));
			}

			if (normalMapped) {
				glClientActiveTextureARB(GL_TEXTURE1);
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				glTexCoordPointer(4, GL_FLOAT, sizeof(WaveFrontObj::Group::Vertex), (GLvoid*)(sizeof(Point3D)* 2 + sizeof(Point2D)));
			}

			glDrawArrays(GL_TRIANGLES, 0, group[g].nVert);

			if (normalMapped) {
				glClientActiveTextureARB(GL_TEXTURE1);
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			}
			if (textured) {
				glClientActiveTextureARB(GL_TEXTURE0);
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			}
			if (lightingEnabled) {
				glDisableClientState(GL_NORMAL_ARRAY);
			}
			glDisableClientState(GL_VERTEX_ARRAY);
		}

		glActiveTextureARB(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTextureARB(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	//Destructor
	WaveFrontObj::~WaveFrontObj() {
		if (loaded) {
			if (ownMaterialLibrary && mtlLib != NULL) {
				delete mtlLib;
			}
			if (compiled) {
				for (int g = 0; g<group.size(); g++) {
					glDeleteBuffersARB(1, &group[g].vbName);
				}
				compiled = false;
			}
			loaded = false;
		}
	}
	//query
	float WaveFrontObj::getRadius() { 
		return radius; 
	}
	float WaveFrontObj::getRadiusAcrossXPlane() { 
		return radiusX; 
	}
	float WaveFrontObj::getRadiusAcrossYPlane() { 
		return radiusY; 
	}
	float WaveFrontObj::getRadiusAcrossZPlane() { 
		return radiusZ; 
	}
	float WaveFrontObj::getXMax() { 
		return xMax; 
	}
	float WaveFrontObj::getXMid() { 
		return xMid; 
	}
	float WaveFrontObj::getXMin() { 
		return xMin; 
	}
	float WaveFrontObj::getYMax() { 
		return yMax; 
	}
	float WaveFrontObj::getYMid() { 
		return yMid; 
	}
	float WaveFrontObj::getYMin() { 
		return yMin; 
	}
	float WaveFrontObj::getZMax() { 
		return zMax; 
	}
	float WaveFrontObj::getZMid() { 
		return zMid; 
	}
	float WaveFrontObj::getZMin() {
		return zMin; 
	}
	//copy stat from another object 
	void WaveFrontObj::copyStatFrom(WaveFrontObj& obj) {
		radius = obj.radius;
		radiusX = obj.radiusX;
		radiusY = obj.radiusY;
		radiusZ = obj.radiusZ;
		xMax = obj.xMax;
		xMid = obj.xMid;
		xMin = obj.xMin;
		yMax = obj.yMax;
		yMid = obj.yMid;
		yMin = obj.yMin;
		zMax = obj.zMax;
		zMid = obj.zMid;
		zMin = obj.zMin;
	}
	//Prints info
	ostream& operator<<(ostream& out, WaveFrontObj& obj) {
		size_t mem = 0;
		for (int g = 0; g < obj.group.size(); g++) {
			mem += sizeof(WaveFrontObj::Group::Vertex)*obj.group[g].nVert;
		}
		out << "WaveFrontObj Path : " << obj.fname << endl
			<< "         Material Library : " << obj.mtlLibName << endl
			<< "         Memory           : " << mem << " bytes" << endl;
		return out;
	}
	//
	void WaveFrontObj::write(ostream &out) {
		//own material library flag
		out.write((const char*)&ownMaterialLibrary, sizeof(ownMaterialLibrary));
		//material library
		if (ownMaterialLibrary) {
			mtlLib->write(out);
		}
		//number of groups
		int nGroup = group.size();
		out.write((const char*)&nGroup, sizeof(nGroup));
		//groups
		for (int g = 0; g<nGroup; g++) {
			//number of vertices in group
			int nVert = group[g].vert.size();
			out.write((const char*)&nVert, sizeof(nVert));
			//vertices in group
			if (nVert>0) {
				out.write((const char*)&group[g].vert[0], sizeof(WaveFrontObj::Group::Vertex)*group[g].vert.size());
			}
		}
		//statistics
		out.write((const char*)&xMin, sizeof(xMin));
		out.write((const char*)&xMid, sizeof(xMid));
		out.write((const char*)&xMax, sizeof(xMax));
		out.write((const char*)&yMin, sizeof(yMin));
		out.write((const char*)&yMid, sizeof(yMid));
		out.write((const char*)&yMax, sizeof(yMax));
		out.write((const char*)&zMin, sizeof(zMin));
		out.write((const char*)&zMid, sizeof(zMid));
		out.write((const char*)&zMax, sizeof(zMax));
		out.write((const char*)&radiusX, sizeof(radiusX));
		out.write((const char*)&radiusY, sizeof(radiusY));
		out.write((const char*)&radiusZ, sizeof(radiusZ));
		out.write((const char*)&radius, sizeof(radius));
	}
	//
	void WaveFrontObj::read(istream &in, MaterialLibrary *mtlLib, string texturePath) {
		//own material library flag
		in.read((char*)&ownMaterialLibrary, sizeof(ownMaterialLibrary));
		//material library
		if (ownMaterialLibrary) {
			this->mtlLib = new MaterialLibrary();
			this->mtlLib->read(in, texturePath);
		} else {
			this->mtlLib = mtlLib;
		}
		//number of groups
		int nGroup = 0;
		in.read((char*)&nGroup, sizeof(nGroup));
		//groups
		group.resize(nGroup);
		for (int g = 0; g<nGroup; g++) {
			//number of vertices in group
			int nVert = 0;
			in.read((char*)&nVert, sizeof(nVert));
			//vertices in group
			if (nVert>0) {
				group[g].vert.resize(nVert);
				in.read((char*)&group[g].vert[0], sizeof(WaveFrontObj::Group::Vertex)*nVert);
			}
		}
		//statistics
		in.read((char*)&xMin, sizeof(xMin));
		in.read((char*)&xMid, sizeof(xMid));
		in.read((char*)&xMax, sizeof(xMax));
		in.read((char*)&yMin, sizeof(yMin));
		in.read((char*)&yMid, sizeof(yMid));
		in.read((char*)&yMax, sizeof(yMax));
		in.read((char*)&zMin, sizeof(zMin));
		in.read((char*)&zMid, sizeof(zMid));
		in.read((char*)&zMax, sizeof(zMax));
		in.read((char*)&radiusX, sizeof(radiusX));
		in.read((char*)&radiusY, sizeof(radiusY));
		in.read((char*)&radiusZ, sizeof(radiusZ));
		in.read((char*)&radius, sizeof(radius));

		loaded = true;
	}

	//Default constructor
	WaveFrontObjSequence::WaveFrontObjSequence() {
		loaded = compiled = false;
	}
	//Returns static unitTypeInfo to given frame number
	WaveFrontObj& WaveFrontObjSequence::getFrame(int frame_number) {
		if (frame_number < 0) {
			frame_number = 0;
		} else if (frame_number >= frame.size()) {
			frame_number = frame.size() - 1;
		}
		return frame[frame_number];
	}
	//Loads all *.obj files in a given directory and sorts them according to filename, copies material library of frame[0] to others
	void WaveFrontObjSequence::load(string path, bool toBeCompiled, bool loadAnimation) {
		if (loaded) {
			return;
		}

		this->path = path;

		//Read unitTypeInfo names
		DIR* dir = opendir(path.data());
		if (dir == NULL) {
			throw DirectoryNotFoundException(path);
		}
		vector<string> filenames;
		for (dirent *d; (d = readdir(dir)) != NULL;) {
			if (getExtension((string)d->d_name) == "obj" || getExtension((string)d->d_name) == "bin") {
				bool alreadyAdded = false;
				for (int i = 0; i < filenames.size(); i++) {
					if (removeExtension(filenames[i]) == path + "/" + removeExtension((string)d->d_name)) {
						alreadyAdded = true;
						break;
					}
				}
				if (!alreadyAdded) {
					filenames.push_back(path + "/" + (string)d->d_name);
				}
			}
			if (!loadAnimation && !filenames.empty()) {
				break;
			}
		}
		closedir(dir);

		//Sort by name
		if (!loadAnimation) {
			quicksort(filenames.data(), filenames.size());
		}

		//Load objects
		frame.resize(filenames.size());
		frame[0].load(filenames[0], false);
		for (int i = 1; i < frame.size(); i++) {
			frame[i].load(filenames[i], false, frame[0].mtlLib);
		}

		//compile textures if possible
		if (toBeCompiled) {
			compile();
		}
		loaded = true;
	}
	//Generates texture of first frame
	void WaveFrontObjSequence::compile() {
		if (loaded && !compiled) {
			for (int i = 0; i < frame.size(); i++) {
				frame[i].compile();
			}
			compiled = true;
		}
	}
	//Renders unitTypeInfo
	void WaveFrontObjSequence::render(int frame_number, Color varColor, Color tint) {
		if (frame_number < 0) {
			frame_number = 0;
		} else if (frame_number >= frame.size()) {
			frame_number = frame.size() - 1;
		}
		frame[frame_number].render(varColor, tint);
	}
	//Returns number of frame
	int WaveFrontObjSequence::length() {
		return frame.size();
	}
	//Destructor
	WaveFrontObjSequence::~WaveFrontObjSequence() {
		loaded = compiled = false;
	}
	//
	void WaveFrontObjSequence::write(ostream &out) {
		//number of frames
		int nFrames = frame.size();
		out.write((const char*)&nFrames, sizeof(nFrames));
		//frame
		for (int i = 0; i < frame.size(); i++) {
			frame[i].write(out);
		}
	}
	//
	void WaveFrontObjSequence::read(istream &in, string texturePath, bool loadAnimation) {
		if (loaded) {
			return;
		}

		//number of frames
		int nFrames = 0;
		in.read((char*)&nFrames, sizeof(nFrames));
		if (!loadAnimation) {
			nFrames = 1;
		}
		frame.resize(nFrames);
		//first frame
		frame[0].read(in, NULL, texturePath);
		//rest of the frames
		for (int i = 1; i < frame.size(); i++) {
			frame[i].read(in, frame[0].mtlLib, texturePath);
		}

		loaded = true;
	}
};
