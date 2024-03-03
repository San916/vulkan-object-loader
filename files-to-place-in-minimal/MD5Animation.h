#pragma once;

#include <string>
#include <vector>
#include <fstream>
using namespace std;

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

class MD5Animation
{
public:
    MD5Animation();
    virtual ~MD5Animation();

    // Load an animation from the animation file
    bool LoadAnimation(const std::string& filename);
    // Update this animation's joint set.
    void Update(float fDeltaTime);
    // Draw the animated skeleton
    void Render();

    // The JointInfo stores the information necessary to build the 
    // skeletons for each frame
    struct JointInfo
    {
        std::string m_Name;
        int         m_ParentID;
        int         m_Flags;
        int         m_StartIndex;
    };
    typedef std::vector<JointInfo> JointInfoList;

    struct Bound
    {
        glm::vec3   m_Min;
        glm::vec3   m_Max;
    };
    typedef std::vector<Bound> BoundList;

    struct BaseFrame
    {
        glm::vec3   m_Pos;
        glm::quat   m_Orient;
    };
    typedef std::vector<BaseFrame> BaseFrameList;

    struct FrameData
    {
        int m_iFrameID;
        std::vector<float> m_FrameData;
    };
    typedef std::vector<FrameData> FrameDataList;

    // A Skeleton joint is a joint of the skeleton per frame
    struct SkeletonJoint
    {
        SkeletonJoint()
            : m_Parent(-1)
            , m_Pos(0)
        {}

        SkeletonJoint(const BaseFrame& copy)
            : m_Pos(copy.m_Pos)
            , m_Orient(copy.m_Orient)
        {}

        int         m_Parent;
        glm::vec3   m_Pos;
        glm::quat   m_Orient;
    };
    typedef std::vector<SkeletonJoint> SkeletonJointList;

    // A frame skeleton stores the joints of the skeleton for a single frame.
    struct FrameSkeleton
    {
        SkeletonJointList   m_Joints;
    };
    typedef std::vector<FrameSkeleton> FrameSkeletonList;

    const FrameSkeleton& GetSkeleton() const
    {
        return m_AnimatedSkeleton;
    }

    int GetNumJoints() const
    {
        return m_iNumJoints;
    }

    const JointInfo& GetJointInfo(unsigned int index) const
    {
        assert(index < m_JointInfos.size());
        return m_JointInfos[index];
    }

    // moved from private
    int m_iNumFrames;

    FrameSkeletonList   m_Skeletons;    // All the skeletons for all the frames // moved from protected
    BaseFrameList       m_BaseFrames;   // moved from protected
    JointInfoList       m_JointInfos;   // moved from protected
    FrameDataList       m_Frames;       // moved from protected

protected:
    BoundList           m_Bounds;
    FrameSkeleton       m_AnimatedSkeleton;

    // Build the frame skeleton for a particular frame
    void BuildFrameSkeleton(FrameSkeletonList& skeletons, const JointInfoList& jointInfo, const BaseFrameList& baseFrames, const FrameData& frameData);
    void InterpolateSkeletons(FrameSkeleton& finalSkeleton, const FrameSkeleton& skeleton0, const FrameSkeleton& skeleton1, float fInterpolate);


private:
    int m_iMD5Version;
    int m_iNumJoints;
    int m_iFramRate;
    int m_iNumAnimatedComponents;

    float m_fAnimDuration;
    float m_fFrameDuration;
    float m_fAnimTime;
};
