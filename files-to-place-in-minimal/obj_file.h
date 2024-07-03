#ifndef OBJ_FILE_H
#define OBJ_FILE_H

#include <string>
#include <vector>
#include <fstream>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

using namespace std;

struct Color {
    float r = 1.0f;
    float g = 1.0f;
    float b = 1.0f;
};
struct TexCoord {
    float u = 1.0f;
    float v = 1.0f;
};
struct Position {
    float x = 1.0f;
    float y = 1.0f;
    float z = 1.0f;
};

struct MyVertex {
    glm::vec3 position;
    glm::vec2 texCoord;

    bool operator==(const MyVertex &other) const;
};

//#define CUBE_OBJ_FILENAME "cube\\cube.obj"
class Material {
public:
    string mtlName;

    vector<int> indices;
    Color diffuseCol;
    Color specularCol;
    Color ambientCol;
    Color lightEmissionCol;
    float ni;
    float alpha;
    float otherAlpha;

    float shininess;

    string fileNameOfTextureMap;

    int illumination;

public:
    Material() {
        diffuseCol.r = 0.8f;
        diffuseCol.g = 0.8f;
        diffuseCol.b = 0.8f;

        specularCol.r = 1.0f;
        specularCol.g = 1.0f;
        specularCol.b = 1.0f;

        ambientCol.r = 0.2f;
        ambientCol.g = 0.2f;
        ambientCol.b = 0.2f;

        alpha = 1.0f;
        otherAlpha = 0.0f;

        shininess = 0.0f;
    }
};

class MyObject;

class ObjFile {
public:
    string pathOnly;
    string objFilename;

	int numVertices;

    vector<MyVertex> vertexBuffer;
    vector<int> indexBuffer;
	vector<int*> vIndicesOfFaces;
	vector<int*> vtIndicesOfFaces;
    vector<int*> vnIndicesOfFaces;
	vector<float*> vertices;
	vector<float*> textures;
	vector<float*> normals;
	vector<string> fileNames;
	vector<string> mtlNames;
    vector<Material *> materials;

	ObjFile();
	ObjFile(string objFilename);

    virtual ~ObjFile();

	void loadObjFile(string objFilename);

};

//ObjFile* cubeObjFile = NULL;

#endif
