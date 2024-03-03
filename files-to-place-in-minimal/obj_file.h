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
    float r = 1.0;
    float g = 1.0;
    float b = 1.0;
};
struct TexCoord {
    float u = 1.0;
    float v = 1.0;
};
struct Position {
    float x = 1.0;
    float y = 1.0;
    float z = 1.0;
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
        diffuseCol.r = 0.8;
        diffuseCol.g = 0.8;
        diffuseCol.b = 0.8;

        specularCol.r = 1.0;
        specularCol.g = 1.0;
        specularCol.b = 1.0;

        ambientCol.r = 0.2;
        ambientCol.g = 0.2;
        ambientCol.b = 0.2;

        alpha = 1.0;
        otherAlpha = 0.0;

        shininess = 0.0;
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

    // for sorting transparency for bbox
    int startOffsetInIndexBufferArray[3][3][3];
    int numTrianglesShownIndexBufferArray[3][3][3];

	ObjFile();
	ObjFile(string objFilename);

    virtual ~ObjFile();

    void loadBBox(string objFilename);
	void loadObjFile(string objFilename);

};

//ObjFile* cubeObjFile = NULL;

#endif
