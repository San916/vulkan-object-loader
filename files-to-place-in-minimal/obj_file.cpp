#define _HAS_STD_BYTE 0

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <unordered_map>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

using namespace std;

#include "obj_file.h"
#include "my_object.h"

namespace std {
    template<> struct hash<MyVertex> {
        size_t operator()(MyVertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.position) ^ (hash<glm::vec2>()(vertex.texCoord) << 1)));
        }
    };
}

bool MyVertex::operator==(const MyVertex& other) const {
    return position == other.position && texCoord == other.texCoord;
}

ObjFile::ObjFile() {
    std::cout << "ObjFile::ObjFile() <~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;

    std::cout << "ObjFile::ObjFile() <xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" << endl;
}

ObjFile::ObjFile(string objFilename) {
    std::cout << "ObjFile::ObjFile(...) <~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
    loadObjFile(objFilename);
    std::cout << "ObjFile::ObjFile(...) <xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" << endl;
}

ObjFile::~ObjFile() {
    std::cout << "ObjFile::~ObjFile() <~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;

	for (int i = 0; i < vIndicesOfFaces.size(); i++) {
		delete[] vIndicesOfFaces[i];
	}
	vector<int*> vIndicesOfFaces;

	for (int i = 0; i < vtIndicesOfFaces.size(); i++) {
		delete[] vtIndicesOfFaces[i];
	}
	vector<int*> vtIndicesOfFaces;

	for (int i = 0; i < vertices.size(); i++) {
		delete[] vertices[i];
	}
	vector<float*> vertices;

	for (int i = 0; i < textures.size(); i++) {
		delete[] textures[i];
	}
	vector<float*> textures;

	for (int i = 0; i < normals.size(); i++) {
		delete[] normals[i];
	}
	vector<float*> normals;
    std::cout << "ObjFile::~ObjFile() <xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" << endl;
}

void ObjFile::loadBBox(string objFilename) {
    std::cout << "ObjFile::loadBBox(...) <~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;

    loadObjFile(objFilename); // load bbox

    std::cout << "ObjFile::loadBBox(...) <xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" << endl;
}

void ObjFile::loadObjFile(string objFilename) {
    std::cout << "ObjFile::loadObjFile(...) <~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;

    string buffer;
	this->objFilename = objFilename;

    bool isVn = false; // For the case where there is no vn

	ifstream cubeObjectFile;
    std::cout << "objFilename: " << objFilename << endl;

	cubeObjectFile.open(objFilename);

    if (cubeObjectFile.fail()) {
        std::cout << "ObjFile::loadObjFile(): error: file does not exist: " << objFilename << endl;
        exit(1);
    }

	char buf[1024];

    for (int i = objFilename.size() - 1; i >= 0; i--) {
        if (objFilename[i] == '/') {
            pathOnly = objFilename.substr(0, i + 1);
            break;
        }
    }

	while (true) {
		cubeObjectFile.getline(buf, 1024);
		if (cubeObjectFile.eof()) {
			break;
		}
#ifdef DBG_MSG_OBJ_FILE_BUF
        std::cout << "buf: " << buf << endl;
#endif
		// cut tokens
		bool skipLine = false;
		if (buf[0] == '#') {
			skipLine = true;
		}
		if (!skipLine) {
			//string buffer;
			buffer = buf;
			if (buf[0] == 'v' && buf[1] == ' ') { //vertex
				float* vertex = new float[3];
				int j = 2;

				for (int i = 0; i < 3; i++) {
					string temp = "";
					while (j < strlen(buf) && buf[j] != ' ') {
						temp += buf[j];
						j++;
					}
					vertex[i] = atof(temp.c_str());
#ifdef DBG_MSG_OBJ_FILE_V
                    std::cout << "vertex position coords: " << vertex[i] << endl;
#endif
					j++;
				}
				vertices.push_back(vertex);
			}
			else if (buf[0] == 'v' && buf[1] == 't') { //vertex texture
				float* texture = new float[2];
				int j = 3;

				for (int i = 0; i < 2; i++) {
					string temp;
					while (j < strlen(buf) && buf[j] != ' ') {
						temp += buf[j];
						j++;
					}

                    if (i == 1) { // invert y axis
                        texture[i] = 1.0f - atof(temp.c_str());
                    }
                    else {
                        texture[i] = atof(temp.c_str());
                    }
#ifdef DBG_MSG_OBJ_FILE_VT
                    std::cout << "vertex texture coords: " << temp << endl;
#endif
					j++;
				}
				textures.push_back(texture);
			}
			else if (buf[0] == 'v' && buf[1] == 'n') { //vertex normal
                if (!isVn) {
                    isVn = true;
                }
				float* normal = new float[3];
				int j = 3;

				for (int i = 0; i < 3; i++) {
					string temp;
					while (j < strlen(buf) && buf[j] != ' ') {
						temp += buf[j];
						j++;
					}
					normal[i] = atof(temp.c_str());
                    //std::cout << "vertex normal: " << temp << endl;
					j++;
				}
				normals.push_back(normal);
			}
			else if (buf[0] == 'f' && buf[1] == ' ') { //face
				int* vIndices = new int[3];
				int* vtIndices = new int[3];
                int* vnIndices = new int[3];

				int j = 2;

				for (int i = 0; i < 3; i++) { // once for each vertex
					int countSlashes = 0;
					string temp;
					while (j < strlen(buf) && buf[j] != ' ') {
						if (buf[j] != '/') {
                            if (!isVn && buf[j + 1] == '/') {
                                temp += buf[j];
                                vIndices[i] = atoi(temp.c_str()) - 1;
                                temp = "";
                            }
                            else if (!isVn && buf[j + 1] == ' ') {
                                temp += buf[j];
                                vtIndices[i] = atoi(temp.c_str()) - 1;
                                temp = "";
                            }
                            else {
                                temp += buf[j];
#ifdef DBG_MSG_OBJ_FILE_F
                                std::cout << "buf[j]: " << buf[j] << endl;
#endif
                                if (j == strlen(buf) - 1) {
                                    // the very last vertex in a face
                                    if (isVn) {
                                        vnIndices[i] = atoi(temp.c_str()) - 1;
                                        temp = "";
                                    }
                                    else {
                                        vtIndices[i] = atoi(temp.c_str()) - 1;
                                        temp = "";
                                    }
                                }
                            }
						}
						else if (isVn) {
							if (countSlashes == 0) {
								vIndices[i] = atoi(temp.c_str()) - 1;
								temp = "";
							}
							else if (countSlashes == 1) {
								vtIndices[i] = atoi(temp.c_str()) - 1;
								temp = "";
							}
                            else if (countSlashes == 2) {
                                vnIndices[i] = atoi(temp.c_str()) - 1;
                                temp = "";
                            }
							countSlashes++;
						}
						j++;
					}
#ifdef DBG_MSG_OBJ_FILE_F
                    std::cout << "vIndices[" << i << "] : " << vIndices[i] << endl;
                    std::cout << "vtIndices[" << i << "] : " << vtIndices[i] << endl;
#endif
					j++;
				}
				vIndicesOfFaces.push_back(vIndices);
				vtIndicesOfFaces.push_back(vtIndices);
                vnIndicesOfFaces.push_back(vnIndices);
			}
			else if (buffer.substr(0, 6) == "mtllib") {
				int i = 7;
				string fileName;
				while (i < strlen(buf) && buf[i] != ' ') {
					fileName += buf[i];
					i++;
				}
				fileNames.push_back(fileName);
			}
			else if (buffer.substr(0, 6) == "usemtl") {
				int i = 7;
				string mtlName;
				while (i < strlen(buf) && buf[i] != ' ') {
					mtlName += buf[i];
					i++;
				}
                std::cout << "mtlName:" << mtlName << endl;
				mtlNames.push_back(mtlName);
			}
		}
	}

    for (int i = 0; i < mtlNames.size(); i++) {
        //cout << "ObjFile::loadObjFile: i: " << i << endl;
        Material *material = new Material();
        for (int j = 0; j < fileNames.size(); j++) {
            ifstream mtlObjectFile;
            mtlObjectFile.open(pathOnly + fileNames.at(j));

            if (mtlObjectFile.fail()) {
                std::cout << "ObjFile::loadObjFile(): mtlfile error: " << pathOnly + fileNames.at(j) << endl;
                while (1) {};
                exit(1);
            }

            std::cout << "pathOnly + fileName.at(j): " << pathOnly << " | "<< fileNames.at(j) << endl;
            bool materialFound = false;

            int count = 0;

            //string buffer;
            while (true) {
                count++;

                if (count > 100) {
                    std::cout << "ObjFile::loadObjFile(): error: " << objFilename << endl;
                    while (1) {};
                    exit(1);
                }

                mtlObjectFile.getline(buf, 256);
                if (mtlObjectFile.eof()) {
                    break;
                }

                std::cout << "buf: |" << buf << "|" << endl;

                buffer = buf;

                if (buffer == "") {
                    continue;
                }

                // cut tokens
                bool skipLine = false;
                if (buf[0] == '#') {
                    skipLine = true;
                }
                std::cout << "test1" << endl;
                std::cout << "buffer:" << buffer << endl;
                int mtlNameSize = mtlNames.at(j).size();
                if (!skipLine) {
                    std::cout << "!skipLine" << endl;
                    if (buffer.substr(0, 6) == "newmtl") {
                            materialFound = true;
                            break;
                    }
                }
            }
            if (materialFound) {
                std::cout << "materialFound == true" << endl;
                while (true) {
                    std::cout << "Hello1" << endl;

                    mtlObjectFile.getline(buf, 256);
                    if (mtlObjectFile.eof()) {
                        break;
                    }

                    string buffer;
                    buffer = buf;

                    std::cout << "buf: |" << buf << "|" << endl;

                    if (buffer.substr(0, 6) == "newmtl") {
                        break;
                    }
                    bool skipLine = false;
                    if (buf[0] == '#') {
                        skipLine = true;
                    }
                    if (!skipLine) {
                        if (buffer.substr(0, 3) == "Ka ") {
                            std::cout << "buffer.substr(0, 3) == Ka" << endl;
                            int l = 3;
                            for (int k = 0; k < 3; k++) {
                                string temp;
                                while (l < strlen(buf) && buf[l] != ' ') {
                                    temp += buf[l];
                                    l++;
                                }
                                if (k == 0) {
                                    material->ambientCol.r = atof(temp.c_str());
                                    std::cout << "ambient color r: " << (float)(material->ambientCol.r) << endl;
                                }
                                else if (k == 1) {
                                    material->ambientCol.g = atof(temp.c_str());
                                    std::cout << "ambient color g: " << (float)(material->ambientCol.g) << endl;
                                }
                                else {
                                    material->ambientCol.b = atof(temp.c_str());
                                    std::cout << "ambient color b: " << (float)(material->ambientCol.b) << endl;
                                }
                                l++;
                            }
                        }
                        else if (buffer.substr(0, 3) == "Kd ") {
                            std::cout << "buffer.substr(0, 3) == Kd" << endl;
                            int l = 3;
                            for (int k = 0; k < 3; k++) {
                                string temp;
                                while (l < strlen(buf) && buf[l] != ' ') {
                                    temp += buf[l];
                                    l++;
                                }
                                if (k == 0) {
                                    material->diffuseCol.r = atof(temp.c_str());
                                    std::cout << "diffuse color r: " << (float)(material->diffuseCol.r) << endl;
                                }
                                else if (k == 1) {
                                    material->diffuseCol.g = atof(temp.c_str());
                                    std::cout << "diffuse color g: " << (float)(material->diffuseCol.g) << endl;
                                }
                                else {
                                    material->diffuseCol.b = atof(temp.c_str());
                                    std::cout << "diffuse color b: " << (float)(material->diffuseCol.b) << endl;
                                }
                                l++;
                            }
                        }
                        else if (buffer.substr(0, 3) == "Ks ") {
                            std::cout << "buffer.substr(0, 3) == Ks" << endl;
                            int l = 3;
                            for (int k = 0; k < 3; k++) {
                                string temp;
                                while (l < strlen(buf) && buf[l] != ' ') {
                                    temp += buf[l];
                                    l++;
                                }
                                if (k == 0) {
                                    material->specularCol.r = atof(temp.c_str());
                                    std::cout << "specular color r: " << (float)(material->specularCol.r) << endl;
                                }
                                else if (k == 1) {
                                    material->specularCol.g = atof(temp.c_str());
                                    std::cout << "specular color g: " << (float)(material->specularCol.g) << endl;
                                }
                                else {
                                    material->specularCol.b = atof(temp.c_str());
                                    std::cout << "specular color b: " << (float)(material->specularCol.b) << endl;
                                }
                                l++;
                            }
                        }
                        else if (buffer.substr(0, 3) == "Ke ") {
                            std::cout << "buffer.substr(0, 3) == Ke" << endl;
                            int l = 3;
                            for (int k = 0; k < 3; k++) {
                                string temp;
                                while (l < strlen(buf) && buf[l] != ' ') {
                                    temp += buf[l];
                                    l++;
                                }

                                if (k == 0) {
                                    material->specularCol.r = atof(temp.c_str());
                                    std::cout << "light emission color r: " << (float)(material->lightEmissionCol.r) << endl;
                                }
                                else if (k == 1) {
                                    material->specularCol.g = atof(temp.c_str());
                                    std::cout << "light emission color g: " << (float)(material->lightEmissionCol.g) << endl;
                                }
                                else {
                                    material->specularCol.b = atof(temp.c_str());
                                    std::cout << "light emission color b: " << (float)(material->lightEmissionCol.b) << endl;
                                }
                                l++;
                            }
                        }
                        else if (buffer.substr(0, 2) == "d ") {
                            std::cout << "buffer.substr(0, 3) == d" << endl;
                            int k = 2;
                            string temp;
                            while (k < strlen(buf) && buf[k] != ' ') {
                                temp += buf[k];
                                k++;
                            }
                            material->alpha = atof(temp.c_str());
                            std::cout << "alpha: " << (float)(material->alpha) << endl;
                        }
                        else if (buffer.substr(0, 3) == "Tr ") {
                            int k = 3;
                            string temp;
                            while (k < strlen(buf) && buf[k] != ' ') {
                                temp += buf[k];
                                k++;
                            }
                            material->alpha = atof(temp.c_str());
                            std::cout << "other alpha: " << (float)(material->otherAlpha) << endl;
                        }
                        else if (buffer.substr(0, 6) == "illum ") {
                            std::cout << "buffer.substr(0, 3) == illum" << endl;
                            int k = 6;
                            string temp;
                            while (k < strlen(buf) && buf[k] != ' ') {
                                temp += buf[k];
                                k++;
                            }
                            material->illumination = atoi(temp.c_str());
                            std::cout << "illumination: " << material->illumination << endl;
                        }
                        else if (buffer.substr(0, 7) == "map_Kd ") {
                            std::cout << "buffer.substr(0, 3) == map_Kd" << endl;
                            int k = 7;
                            string temp;
                            while (k < strlen(buf) && buf[k] != ' ') {
                                temp += buf[k];
                                k++;
                            }
                            material->fileNameOfTextureMap = temp;
                            std::cout << "fileNameOfTextureMap: " << material->fileNameOfTextureMap << endl;
                        }
                        else if (buffer.substr(0, 3) == "Ns ") {
                            std::cout << "buffer.substr(0, 3) == Ns" << endl;
                            int k = 3;
                            string temp;
                            while (k < strlen(buf) && buf[k] != ' ') {
                                temp += buf[k];
                                k++;
                            }
                            material->shininess = atof(temp.c_str());
                            std::cout << "shininess: " << (float)(material->shininess) << endl;
                        }
                        else if (buffer.substr(0, 3) == "Ni ") {
                            std::cout << "buffer.substr(0, 3) == Ni" << endl;
                            int k = 3;
                            string temp;
                            while (k < strlen(buf) && buf[k] != ' ') {
                                temp += buf[k];
                                k++;
                            }
                            std::cout << "temp = |" << temp << "|" << endl;

                            material->ni = atof(temp.c_str());
                            std::cout << "Ni: " << (float)(material->ni) << endl;
                        }
                    }
                }
            }
            std::cout << "end of material parser" << endl;
        }
        materials.push_back(material);
    }

    std::unordered_map<MyVertex, uint32_t> verticesHashMap = {};

    std::cout << "vIndicesOfFaces.size(): " << vIndicesOfFaces.size() << endl;
    for (int i = 0; i < vIndicesOfFaces.size(); i++) {
        if (i % 1000 == 0) {
            //std::cout << "ObjFile::loadObjFile(): i: " << i << endl;
        }
        for (int j = 0; j < 3; j++) {
            //std::cout << ObjFile::loadObjFile(): j: " << j << endl;
            MyVertex curVertex = {};
            int a, b, c;
            a = vIndicesOfFaces[i][j];
            b = vtIndicesOfFaces[i][j];

            float* pos = vertices.at(a);
            float x, y, z;
            x = pos[0];
            y = pos[1];
            z = pos[2];

            float* texCoords = textures.at(b);
            float u, v;
            u = texCoords[0];
            v = texCoords[1];

            curVertex.position.x = x;
            curVertex.position.y = y;
            curVertex.position.z = z;

            curVertex.texCoord.s = u;
            curVertex.texCoord.t = v;

            if (verticesHashMap.count(curVertex) == 0) {
                verticesHashMap[curVertex] = vertexBuffer.size();
                indexBuffer.push_back(vertexBuffer.size());

                vertexBuffer.push_back(curVertex);
                //cout << "Vertex[" << (vertexBuffer.size() - 1) << "].x: " << x << endl;
                //cout << "Vertex[" << (vertexBuffer.size() - 1) << "].y: " << y << endl;
                //cout << "Vertex[" << (vertexBuffer.size() - 1) << "].z: " << z << endl;
                //cout << "Vertex[" << (vertexBuffer.size() - 1) << "].u: " << u << endl;
                //cout << "Vertex[" << (vertexBuffer.size() - 1) << "].v: " << v << endl;
            }
            else {
                indexBuffer.push_back(verticesHashMap[curVertex]);
            }
        }
    }
    numVertices = vertices.size();
    cout << "vertices.size(): " << vertices.size() << endl;

    std::cout << "ObjFile::loadObjFile(...) <xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" << endl;
}
