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
//#include <glm/gtc/quaternion.hpp>
//#include <glm/gtx/quaternion.hpp>
//==============================================================================
//==============================================================================
//==============================================================================
// class GameObject
//==============================================================================
//==============================================================================
//==============================================================================
GameObject::GameObject() {
    translationVelocity = glm::vec3(0.0f, 0.0f, 0.0f);
    angularVelocity = glm::vec3(0.0f, 0.0f, 0.0f);
    acceleration = glm::vec3(0.0f, 0.0f, 0.0f);
    objectDensity = 1.0f;
    forceNet = acceleration * boundingBoxVolume * objectDensity;
    freeFall = true;
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
    GameObject();
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

    GameObject();

    std::cout << "MyMD5Object::MyMD5Object() <xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" << endl;
}

bool MyMD5Object::isMD5File() {
    return true;
}

// Return the center of mass after going through its transformations
glm::vec4 GameObject::transformCenterOfMass(glm::mat4 rotMat) {
    rotMat = rotMat * glm::mat4(1.0f);

    glm::vec4 actualCenterOfMass = glm::vec4(centerOfMass.x, centerOfMass.y, centerOfMass.z, 0);
    actualCenterOfMass = rotMat * actualCenterOfMass;
    actualCenterOfMass.x += offset3D.x;
    actualCenterOfMass.y += offset3D.y;
    actualCenterOfMass.z += offset3D.z;

    return actualCenterOfMass;
}

// Return the bounding box after going through its transformations
std::vector<glm::vec3> GameObject::transformBoundingBox(glm::mat4 rotMat) {
    std::vector<glm::vec3> transformedBoundingBox;

    // Create the rotation matrix for the transformed vertices
    rotMat = rotMat * glm::mat4(1.0f);

    // We use a bounding box of the object to calculate the physics
    for (int j = 0; j < boundingBox.size(); j++) {
        // Get a copy of the base vertex with no transformations
        glm::vec4 tempVector = glm::vec4(boundingBox.at(j).x, boundingBox.at(j).y, boundingBox.at(j).z, 0);

        // First rotate the object
        tempVector = rotMat * tempVector;

        // Then translate
        tempVector.x += offset3D.x;
        tempVector.y += offset3D.y;
        tempVector.z += offset3D.z;

        transformedBoundingBox.push_back(glm::vec3(tempVector.x, tempVector.y, tempVector.z));
    }
    return transformedBoundingBox;
}

void MyObject::createBoundingBox() {
    if (boundingBox.size() > 0) { // This makes it such that we can reset the bounding box if needed
        boundingBox.clear();
        boundingBoxIndices.clear();
    }
    float lowestX, lowestY, lowestZ;
    float highestX, highestY, highestZ;

    for (int i = 0; i < vertices.size(); i++) {
        glm::vec3 curVector = vertices.at(i).pos;
        if (i == 0) {
            lowestX = curVector.x;
            highestX = curVector.x;
            lowestY = curVector.y;
            highestY = curVector.y;
            lowestZ = curVector.z;
            highestZ = curVector.z;
        }
        else {
            if (curVector.x < lowestX) {
                lowestX = curVector.x;
            }
            if (curVector.x > highestX) {
                highestX = curVector.x;
            }
            if (curVector.y < lowestY) {
                lowestY = curVector.y;
            }
            if (curVector.y > highestY) {
                highestY = curVector.y;
            }
            if (curVector.z < lowestZ) {
                lowestZ = curVector.z;
            }
            if (curVector.z > highestZ) {
                highestZ = curVector.z;
            }
        }
    }
    boundingBox.push_back(glm::vec3(lowestX, lowestY, lowestZ));
    boundingBox.push_back(glm::vec3(lowestX, lowestY, highestZ));
    boundingBox.push_back(glm::vec3(lowestX, highestY, lowestZ));
    boundingBox.push_back(glm::vec3(lowestX, highestY, highestZ));
    boundingBox.push_back(glm::vec3(highestX, lowestY, lowestZ));
    boundingBox.push_back(glm::vec3(highestX, lowestY, highestZ));
    boundingBox.push_back(glm::vec3(highestX, highestY, lowestZ));
    boundingBox.push_back(glm::vec3(highestX, highestY, highestZ));

    boundingBoxIndices.push_back(0);
    boundingBoxIndices.push_back(1);
    boundingBoxIndices.push_back(2);

    boundingBoxIndices.push_back(1);
    boundingBoxIndices.push_back(2);
    boundingBoxIndices.push_back(3);

    boundingBoxIndices.push_back(4);
    boundingBoxIndices.push_back(5);
    boundingBoxIndices.push_back(6);

    boundingBoxIndices.push_back(5);
    boundingBoxIndices.push_back(6);
    boundingBoxIndices.push_back(7);

    boundingBoxIndices.push_back(2);
    boundingBoxIndices.push_back(6);
    boundingBoxIndices.push_back(7);

    boundingBoxIndices.push_back(2);
    boundingBoxIndices.push_back(3);
    boundingBoxIndices.push_back(7);

    boundingBoxIndices.push_back(0);
    boundingBoxIndices.push_back(4);
    boundingBoxIndices.push_back(5);

    boundingBoxIndices.push_back(0);
    boundingBoxIndices.push_back(5);
    boundingBoxIndices.push_back(1);

    boundingBoxIndices.push_back(3);
    boundingBoxIndices.push_back(5);
    boundingBoxIndices.push_back(7);

    boundingBoxIndices.push_back(3);
    boundingBoxIndices.push_back(5);
    boundingBoxIndices.push_back(1);

    boundingBoxIndices.push_back(2);
    boundingBoxIndices.push_back(6);
    boundingBoxIndices.push_back(4);

    boundingBoxIndices.push_back(2);
    boundingBoxIndices.push_back(4);
    boundingBoxIndices.push_back(0);

    for (int i = 0; i < boundingBoxIndices.size(); i += 3) {
        int* temp = new int[3];
        temp[0] = boundingBoxIndices.at(i);
        temp[1] = boundingBoxIndices.at(i + 1);
        temp[2] = boundingBoxIndices.at(i + 2);

        boundingBoxFaces.push_back(temp);
    }

    centerOfMass = glm::vec3(lowestX + highestX, lowestY + highestY, lowestZ + highestZ) / 2.0f;
    boundingBoxVolume = (highestX - lowestX) * (highestY - lowestY) * (highestY - lowestY);

    // Not correct MOI calculation
    float objectMass = objectDensity * boundingBoxVolume;
    float averageEdgeLength = (highestX - lowestX) + (highestY - lowestY) + (highestY - lowestY);
    averageEdgeLength /= 3.0f;
    cout << "averageEdgeLength: " << averageEdgeLength << endl;
    cout << "objectMass: " << objectMass << endl;
    momentOfInertia = 2 * objectMass * pow(averageEdgeLength, 2.0) / 3.0;
    cout << "momentOfInertia: " << momentOfInertia << endl;
}

void GameObject::createBoundingBox() {

}
