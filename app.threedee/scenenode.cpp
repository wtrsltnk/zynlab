#include "scenenode.h"
#include "gl.utilities/gl.utilities.shaders.h"

static std::string vertexGlsl = "#version 150\n\
in vec3 vertex;\
in vec3 normal;\
in vec2 texcoord;\
\
uniform mat4 u_projection;\
uniform mat4 u_view;\
uniform mat4 u_model;\
\
out vec2 f_texcoord;\
\
void main()\
{\
    gl_Position = u_projection * u_view * u_model * vec4(vertex.xyz, 1.0);\
    f_texcoord = texcoord;\
}";

static std::string fragmentGlsl = "#version 150\n\
uniform sampler2D u_texture;\
\
in vec2 f_texcoord;\
\
out vec4 color;\
\
void main()\
{\
   color = texture(u_texture, f_texcoord);\
}";

Shader<glm::vec3, glm::vec3, glm::vec2> SceneNode::_shader;

SceneNode::SceneNode()
    : _parent(nullptr), _firstChild(nullptr), _lastChild(nullptr),
      _nextSibling(nullptr), _previousSibling(nullptr), _vertexBuffer(_shader)
{ }

SceneNode::~SceneNode() { }

void SceneNode::Setup()
{
    SceneNode::_shader.compile(vertexGlsl, fragmentGlsl);
}

void SceneNode::AddChild(SceneNode *child)
{
    if (child == nullptr) return;

    if (this->isChild(child)) return;

    if (child->_parent != nullptr) child->_parent->RemoveChild(child);

    if (this->_lastChild != nullptr) this->_lastChild->_nextSibling = child;
    child->_previousSibling = this->_lastChild;
    this->_lastChild = child;
    if (this->_firstChild == nullptr) this->_firstChild = child;
}

bool SceneNode::isChild(SceneNode* child) const
{
    auto n = this->_firstChild;

    while (n != nullptr)
    {
        if (n == child) return true;
        n = n->_nextSibling;
    }

    return false;
}

void SceneNode::RemoveChild(SceneNode *child)
{
    if (child == nullptr) return;

    if (!this->isChild(child)) return;

    if (child->_previousSibling != nullptr) child->_previousSibling->_nextSibling = child->_nextSibling;
    if (child->_nextSibling != nullptr) child->_nextSibling->_previousSibling = child->_previousSibling;

    if (this->_firstChild == child) this->_firstChild = child->_nextSibling;
    if (this->_lastChild == child) this->_lastChild = child->_previousSibling;

    child->_parent = child->_nextSibling = child->_previousSibling = nullptr;
}
