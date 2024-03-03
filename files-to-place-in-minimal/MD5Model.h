#pragma once;

#include <string>
#include <vector>
#include <fstream>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include "MD5Animation.h"

class MD5Model
{
public:
    MD5Model();
    virtual ~MD5Model();

    bool LoadModel(const std::string& filename);
    bool LoadAnim(const std::string& filename);
    void Update(float fDeltaTime);
    void Render();

public:
    typedef std::vector<glm::vec3> PositionBuffer;
    typedef std::vector<glm::vec3> NormalBuffer;
    typedef std::vector<glm::vec2> Tex2DBuffer;
    //typedef std::vector<GLuint> IndexBuffer;
    typedef std::vector<std::uint32_t> IndexBuffer;

    struct Vertex
    {
        glm::vec3   m_Pos;
        glm::vec3   m_Normal;
        glm::vec2   m_Tex0;
        int         m_StartWeight;
        int         m_WeightCount;
    };
    typedef std::vector<Vertex> VertexList;

    struct Triangle
    {
        int             m_Indices[3];
    };
    typedef std::vector<Triangle> TriangleList;

    struct Weight
    {
        int             m_JointID;
        float           m_Bias;
        glm::vec3       m_Pos;
    };
    typedef std::vector<Weight> WeightList;

    struct Joint
    {
        std::string     m_Name;
        int             m_ParentID;
        glm::vec3       m_Pos;
        glm::quat       m_Orient;
        glm::mat4       m_JointMatrix; // added // the matrix is relative to parent matrix

    };
    typedef std::vector<Joint> JointList;

    struct Mesh // 
    {
        std::string     m_Shader;
        // This vertex list stores the vertices in the bind pose.
        VertexList      m_Verts;
        TriangleList    m_Tris;
        WeightList      m_Weights;

        // A texture ID for the material
        //GLuint          m_TexID;
        std::uint32_t          m_TexID;

        // These buffers are used for rendering the animated mesh
        PositionBuffer  m_PositionBuffer;   // Vertex position stream
        NormalBuffer    m_NormalBuffer;     // Vertex normals stream
        Tex2DBuffer     m_Tex2DBuffer;      // Texture coordinate set
        IndexBuffer     m_IndexBuffer;      // Vertex index buffer
    };
    typedef std::vector<Mesh> MeshList;

    // Prepare the mesh for rendering
    // Compute vertex positions and normals
    bool PrepareMesh(Mesh& mesh);
    bool PrepareMesh(Mesh& mesh, const MD5Animation::FrameSkeleton& skel);
    bool PrepareNormals(Mesh& mesh);

    // Render a single mesh of the model
    void RenderMesh(const Mesh& mesh);
    void RenderNormals(const Mesh& mesh);

    // Draw the skeleton of the mesh for debugging purposes.
    void RenderSkeleton(const JointList& joints);

    bool CheckAnimation(const MD5Animation& animation) const;
public:
    MeshList        m_Meshes;    // moved from private
    MD5Animation    m_Animation; // moved from private
    JointList       m_Joints;    // moved from private

private:
    int                 m_iMD5Version;
    int                 m_iNumJoints;
    int                 m_iNumMeshes;

    bool                m_bHasAnimation;

    glm::mat4x4         m_LocalToWorldMatrix;

};
