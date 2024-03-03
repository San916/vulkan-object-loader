#define _HAS_STD_BYTE 0

#include <iostream>
#include <string>

#define DBG_MSG_OBJ_FILE_TURN_ON

using namespace std;

#include "MD5Model.h"
#include "vertex.h"
#include "obj_file.h"
#include "my_object.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

//==============================================================================
//==============================================================================
//==============================================================================
// class GameObject
//==============================================================================
//==============================================================================
//==============================================================================
GameObject::GameObject() {
}

GameObject::~GameObject() {
}

bool GameObject::isObjFile() {
    return false;
}

bool GameObject::isMD5File() {
    return false;
}

//==============================================================================
//==============================================================================
//==============================================================================
// class MyMD5Object : child class of MyObject
//==============================================================================
//==============================================================================
//==============================================================================
MyObject::MyObject() {
    std::cout << "MyObject::MyObject() <~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;

    objFile = NULL;
}

MyObject::MyObject(std::string objFilename) {
    std::cout << "MyObject::MyObject(...) <~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;

    if (isObjFile()) {
        VkBuffer* newStagingBufferForVB = new VkBuffer[NUM_STAGING_BUFFER_FOR_VB];
        VkDeviceMemory* newStagingBufferMemoryForVB = new VkDeviceMemory[NUM_STAGING_BUFFER_FOR_VB];
        stagingBufferForVB.push_back(newStagingBufferForVB);
        stagingBufferMemoryForVB.push_back(newStagingBufferMemoryForVB);

        VkBuffer* newVertexBuffers = new VkBuffer[NUM_STAGING_BUFFER_FOR_VB];
        VkDeviceMemory* newVertexBufferMemories = new VkDeviceMemory[NUM_STAGING_BUFFER_FOR_VB];
        vertexBuffers.push_back(newVertexBuffers);
        vertexBufferMemories.push_back(newVertexBufferMemories);
    }

#ifndef DBG_MSG_OBJ_FILE_TURN_ON
    std::cout.setstate(std::ios_base::failbit);
#endif
    objFile = new ObjFile(objFilename);
#ifndef DBG_MSG_OBJ_FILE_TURN_ON
    std::cout.clear();
#endif
}

bool MyObject::isObjFile() {
    return true;
}

//==============================================================================
//==============================================================================
//==============================================================================
// class MyMD5Object : child class of MyObject
//==============================================================================
//==============================================================================
//==============================================================================

MyMD5Object::MyMD5Object() {
    std::cout << "MyMD5Object::MyMD5Object() <~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;

    std::cout << "MyMD5Object::MyMD5Object() <xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" << endl;
}

MyMD5Object::MyMD5Object(std::string md5Filename) {
    std::cout << "MyMD5Object::MyMD5Object() <~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;

    drawObject = true;

    curFrame = 0;

    std::string pathWithoutExtension = md5Filename.substr(0, md5Filename.length() - 8);

    md5Model = new MD5Model();
    md5Model->LoadModel(md5Filename);
    md5Model->LoadAnim(pathWithoutExtension + ".md5anim");

    Vertex tempVertex;

    for (int i = 0; i < md5Model->m_Meshes.size(); i++) {
        MyObject *tempObj = new MyObject();

        for (int j = 0; j < md5Model->m_Animation.m_iNumFrames; j++) {
            VkBuffer* newStagingBufferForVB = new VkBuffer[NUM_STAGING_BUFFER_FOR_VB];
            VkDeviceMemory* newStagingBufferMemoryForVB = new VkDeviceMemory[NUM_STAGING_BUFFER_FOR_VB];
            VkBuffer* newVertexBuffers = new VkBuffer[NUM_STAGING_BUFFER_FOR_VB];
            VkDeviceMemory* newVertexBufferMemories = new VkDeviceMemory[NUM_STAGING_BUFFER_FOR_VB];

            tempObj->stagingBufferForVB.push_back(newStagingBufferForVB);
            tempObj->stagingBufferMemoryForVB.push_back(newStagingBufferMemoryForVB);
            tempObj->vertexBuffers.push_back(newVertexBuffers);
            tempObj->vertexBufferMemories.push_back(newVertexBufferMemories);
        }

        for (int j = 0; j < md5Model->m_Meshes[i].m_Verts.size(); j++) {
            tempVertex.pos.x = md5Model->m_Meshes[i].m_Verts[j].m_Pos.x;
            tempVertex.pos.y = md5Model->m_Meshes[i].m_Verts[j].m_Pos.y;
            tempVertex.pos.z = md5Model->m_Meshes[i].m_Verts[j].m_Pos.z;
            tempVertex.texCoord.s = md5Model->m_Meshes[i].m_Verts[j].m_Tex0.s;
            tempVertex.texCoord.t = md5Model->m_Meshes[i].m_Verts[j].m_Tex0.t;
            tempVertex.color.r = 0.0f;
            tempVertex.color.g = 0.0f;
            tempVertex.color.b = 0.0f;
            tempObj->vertices.push_back(tempVertex);
        }
        myObjects.push_back(tempObj);
    }
    for (int i = 0; i < md5Model->m_Meshes.size(); i++) {
        for (int j = 0; j < md5Model->m_Meshes[i].m_Tris.size(); j++) {
            myObjects[i]->indices.push_back(md5Model->m_Meshes[i].m_Tris[j].m_Indices[0]);
            myObjects[i]->indices.push_back(md5Model->m_Meshes[i].m_Tris[j].m_Indices[1]);
            myObjects[i]->indices.push_back(md5Model->m_Meshes[i].m_Tris[j].m_Indices[2]);
        }
    }

    std::cout << "MyMD5Object::MyMD5Object() <xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" << endl;
}

bool MyMD5Object::isMD5File() {
    return true;
}
