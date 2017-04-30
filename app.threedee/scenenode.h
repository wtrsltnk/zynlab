#ifndef SCENENODE_H
#define SCENENODE_H

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include "gl.utilities/gl.utilities.vertexbuffers.h"

class SceneNode
{
    SceneNode* _parent;
    SceneNode* _firstChild;
    SceneNode* _lastChild;
    SceneNode* _nextSibling;
    SceneNode* _previousSibling;
public:
    SceneNode();
    virtual ~SceneNode();

    static void Setup();

    void AddChild(SceneNode* child);
    bool isChild(SceneNode* child) const;
    void RemoveChild(SceneNode* child);

    glm::vec3 _localPosition;
    glm::quat _localRotation;
    glm::vec3 _boundingBox[2]; // min == 0, max == 1
    static Shader<glm::vec3, glm::vec3, glm::vec2> _shader;
    VertexBuffer<glm::vec3, glm::vec3, glm::vec2> _vertexBuffer;
};

#endif // SCENENODE_H
