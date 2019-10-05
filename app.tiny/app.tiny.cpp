#include "app.tiny.h"
#include <iostream>

#include <nlohmann/json.hpp>

#include <tiny_gltf.h>

#include <glm/glm.hpp>
#include <map>
#include <vector>

class SceneNode
{
protected:
    char const *_id;
    SceneNode *_parent;
    glm::mat4 _relativeMatrix;

public:
    SceneNode(char const *id);
    SceneNode(char const *id, SceneNode *parent);
    virtual ~SceneNode();

    void Parent(SceneNode *parent);
    SceneNode const *Parent() const;

    glm::mat4 WorldMatrix() const;
};

class DevicePanelControl : public SceneNode
{
public:
    DevicePanelControl(char const *id);
    DevicePanelControl(char const *id, SceneNode *parent);
    virtual ~DevicePanelControl();

    class DevicePanel const *ParentPanel() const;
};

class DevicePanelKnob : public DevicePanelControl
{
public:
    DevicePanelKnob(char const *id);
    DevicePanelKnob(char const *id, SceneNode *parent);
    virtual ~DevicePanelKnob();
};

class DevicePanelFader : public DevicePanelControl
{
public:
    DevicePanelFader(char const *id);
    DevicePanelFader(char const *id, SceneNode *parent);
    virtual ~DevicePanelFader();
};

class DevicePanelSwitch : public DevicePanelControl
{
public:
    DevicePanelSwitch(char const *id);
    DevicePanelSwitch(char const *id, SceneNode *parent);
    virtual ~DevicePanelSwitch();
};

class DevicePanelButton : public DevicePanelControl
{
public:
    DevicePanelButton(char const *id);
    DevicePanelButton(char const *id, SceneNode *parent);
    virtual ~DevicePanelButton();
};

class DevicePanelDisplay : public SceneNode
{
public:
    DevicePanelDisplay(char const *id);
    DevicePanelDisplay(char const *id, SceneNode *parent);
    virtual ~DevicePanelDisplay();

    class DevicePanel const *ParentPanel() const;
};

class DevicePanelLed : public DevicePanelDisplay
{
public:
    DevicePanelLed(char const *id);
    DevicePanelLed(char const *id, SceneNode *parent);
    virtual ~DevicePanelLed();
};

enum class ControlTypes
{
    Knob,
    Fader,
    Switch,
    Button,
};

class DevicePanelControlBuilder
{
    char const *_id;
    ControlTypes _type;
    DevicePanelControlBuilder(char const *id, ControlTypes type);

public:
    static DevicePanelControlBuilder NewKnob(char const *id);
    static DevicePanelControlBuilder NewFader(char const *id);
    static DevicePanelControlBuilder NewSwitch(char const *id);
    static DevicePanelControlBuilder NewButton(char const *id);
    DevicePanelControl *Build();
};

class DevicePanel : public SceneNode
{
public:
    DevicePanel(char const *id);
    DevicePanel(char const *id, SceneNode *parent);
    virtual ~DevicePanel();

    class Device const *ParentDevice() const;

    std::vector<DevicePanelControl *> _controls;
    std::vector<DevicePanelDisplay *> _displays;
};

class DevicePanelBuilder
{
    char const *_id;
    std::vector<DevicePanelControlBuilder> _devicePanelControlBuilders;
    DevicePanelBuilder(char const *id);

public:
    static DevicePanelBuilder New(char const *id);
    DevicePanel *Build();

public:
    DevicePanelBuilder &AddControl(DevicePanelControlBuilder builder);
};

class Device : public SceneNode
{
public:
    Device(char const *id);
    Device(char const *id, SceneNode *parent);
    virtual ~Device();

    class DeviceRack const *ParentRack() const;

    std::vector<DevicePanel *> _panels;
};

class DeviceBuilder
{
    char const *_id;
    std::vector<DevicePanelBuilder> _devicePanelBuilders;
    DeviceBuilder(char const *id);

public:
    static DeviceBuilder New(char const *id);
    Device *Build();

public:
    DeviceBuilder &AddPanel(DevicePanelBuilder builder);
};

class DeviceRack : public SceneNode
{
public:
    DeviceRack(char const *id);
    DeviceRack(char const *id, SceneNode *parent);
    virtual ~DeviceRack();

    std::vector<Device *> _devices;
};

class DeviceRackBuilder
{
    char const *_id;
    std::vector<DeviceBuilder> _deviceBuilders;
    DeviceRackBuilder(char const *id);

public:
    static DeviceRackBuilder New(char const *id);
    DeviceRack *Build();

public:
    DeviceRackBuilder &AddDevice(DeviceBuilder builder);
};

SceneNode *AppTiny::BuildScene()
{
    _sceneRoot = DeviceRackBuilder::New("ZynAddSubFx")
                     .AddDevice(DeviceBuilder::New("Mixer")
                                    .AddPanel(DevicePanelBuilder::New("Front")))
                     .Build();

    return _sceneRoot;
}

SceneNode::SceneNode(char const *id) : _id(id) {}

SceneNode::SceneNode(char const *id, SceneNode *parent) : _id(id), _parent(parent) {}

SceneNode::~SceneNode() {}

void SceneNode::Parent(SceneNode *parent) { _parent = parent; }

SceneNode const *SceneNode::Parent() const { return _parent; }

//

DevicePanelControlBuilder::DevicePanelControlBuilder(char const *id, ControlTypes type) : _id(id), _type(type) {}

DevicePanelControlBuilder DevicePanelControlBuilder::NewKnob(const char *id)
{
    DevicePanelControlBuilder builder(id, ControlTypes::Knob);

    return builder;
}

DevicePanelControlBuilder DevicePanelControlBuilder::NewFader(char const *id)
{
    DevicePanelControlBuilder builder(id, ControlTypes::Fader);

    return builder;
}

DevicePanelControlBuilder DevicePanelControlBuilder::NewSwitch(char const *id)
{
    DevicePanelControlBuilder builder(id, ControlTypes::Switch);

    return builder;
}

DevicePanelControlBuilder DevicePanelControlBuilder::NewButton(char const *id)
{
    DevicePanelControlBuilder builder(id, ControlTypes::Button);

    return builder;
}

DevicePanelControl::DevicePanelControl(char const *id) : SceneNode(id) {}

DevicePanelControl::DevicePanelControl(char const *id, SceneNode *parent) : SceneNode(id, parent) {}

DevicePanelControl::~DevicePanelControl() {}

DevicePanelKnob::DevicePanelKnob(char const *id) : DevicePanelControl(id) {}

DevicePanelKnob::DevicePanelKnob(char const *id, SceneNode *parent) : DevicePanelControl(id, parent) {}

DevicePanelKnob::~DevicePanelKnob() {}

DevicePanelFader::DevicePanelFader(char const *id) : DevicePanelControl(id) {}

DevicePanelFader::DevicePanelFader(char const *id, SceneNode *parent) : DevicePanelControl(id, parent) {}

DevicePanelFader::~DevicePanelFader() {}

DevicePanelSwitch::DevicePanelSwitch(char const *id) : DevicePanelControl(id) {}

DevicePanelSwitch::DevicePanelSwitch(char const *id, SceneNode *parent) : DevicePanelControl(id, parent) {}

DevicePanelSwitch::~DevicePanelSwitch() {}

DevicePanelButton::DevicePanelButton(char const *id) : DevicePanelControl(id) {}

DevicePanelButton::DevicePanelButton(char const *id, SceneNode *parent) : DevicePanelControl(id, parent) {}

DevicePanelButton::~DevicePanelButton() {}

DevicePanelControl *DevicePanelControlBuilder::Build()
{
    switch (_type)
    {
        case ControlTypes::Knob:
        {
            return new DevicePanelKnob(_id);
        }
        case ControlTypes::Fader:
        {
            return new DevicePanelFader(_id);
        }
        case ControlTypes::Button:
        {
            return new DevicePanelButton(_id);
        }
        case ControlTypes::Switch:
        {
            return new DevicePanelSwitch(_id);
        }
    }

    return nullptr;
}

//

DevicePanelBuilder::DevicePanelBuilder(char const *id) : _id(id) {}

DevicePanelBuilder DevicePanelBuilder::New(char const *id)
{
    DevicePanelBuilder builder(id);

    return builder;
}

DevicePanel::DevicePanel(char const *id) : SceneNode(id) {}

DevicePanel::DevicePanel(char const *id, SceneNode *parent) : SceneNode(id, parent) {}

DevicePanel::~DevicePanel() {}

DevicePanel *DevicePanelBuilder::Build()
{
    auto panel = new DevicePanel(_id);

    for (auto builder : _devicePanelControlBuilders)
    {
        auto control = builder.Build();
        control->Parent(panel);
        panel->_controls.push_back(control);
    }

    return panel;
}

DevicePanelBuilder &DevicePanelBuilder::AddControl(DevicePanelControlBuilder builder)
{
    _devicePanelControlBuilders.push_back(builder);

    return *this;
}

//

DeviceBuilder::DeviceBuilder(char const *id) : _id(id) {}

DeviceBuilder DeviceBuilder::New(char const *id)
{
    DeviceBuilder builder(id);

    return builder;
}

Device::Device(char const *id) : SceneNode(id) {}

Device::Device(char const *id, SceneNode *parent) : SceneNode(id, parent) {}

Device::~Device() {}

Device *DeviceBuilder::Build()
{
    auto device = new Device(_id);

    for (auto builder : _devicePanelBuilders)
    {
        auto panel = builder.Build();
        panel->Parent(device);
        device->_panels.push_back(panel);
    }

    return device;
}

DeviceBuilder &DeviceBuilder::AddPanel(DevicePanelBuilder builder)
{
    _devicePanelBuilders.push_back(builder);

    return *this;
}

//

DeviceRackBuilder::DeviceRackBuilder(char const *id) : _id(id) {}

DeviceRackBuilder DeviceRackBuilder::New(char const *id)
{
    DeviceRackBuilder builder(id);

    return builder;
}

DeviceRack::DeviceRack(char const *id) : SceneNode(id) {}

DeviceRack::DeviceRack(char const *id, SceneNode *parent) : SceneNode(id, parent) {}

DeviceRack::~DeviceRack() {}

DeviceRack *DeviceRackBuilder::Build()
{
    auto rack = new DeviceRack(_id);

    for (auto builder : _deviceBuilders)
    {
        auto device = builder.Build();
        device->Parent(rack);
        rack->_devices.push_back(device);
    }

    return rack;
}

DeviceRackBuilder &DeviceRackBuilder::AddDevice(DeviceBuilder builder)
{
    _deviceBuilders.push_back(builder);

    return *this;
}

//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//

namespace gltf_code {
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

#define CheckGLErrors(desc)                                                         \
    {                                                                               \
        GLenum e = glGetError();                                                    \
        if (e != GL_NO_ERROR)                                                       \
        {                                                                           \
            printf("OpenGL error in \"%s\": %d (%d) %s:%d\n", desc, e, e, __FILE__, \
                   __LINE__);                                                       \
            exit(20);                                                               \
        }                                                                           \
    }
using namespace tinygltf;

typedef struct
{
    GLuint vb;
} GLBufferState;

typedef struct
{
    std::vector<GLuint> diffuseTex; // for each primitive in mesh
} GLMeshState;

typedef struct
{
    std::map<std::string, GLint> attribs;
    std::map<std::string, GLint> uniforms;
} GLProgramState;

typedef struct
{
    GLuint vb;    // vertex buffer
    size_t count; // byte count
} GLCurvesState;

std::map<int, GLBufferState> gBufferState;
std::map<std::string, GLMeshState> gMeshState;
std::map<int, GLCurvesState> gCurvesMesh;
GLProgramState gGLProgramState;

bool LoadShader(GLenum shaderType, // GL_VERTEX_SHADER or GL_FRAGMENT_SHADER(or
                                   // maybe GL_COMPUTE_SHADER)
                GLuint &shader, const char *shaderSourceFilename)
{
    GLint val = 0;

    // free old shader/program
    if (shader != 0)
    {
        glDeleteShader(shader);
    }

    std::vector<GLchar> srcbuf;
    FILE *fp = fopen(shaderSourceFilename, "rb");
    if (!fp)
    {
        fprintf(stderr, "failed to load shader: %s\n", shaderSourceFilename);
        return false;
    }
    fseek(fp, 0, SEEK_END);
    size_t len = ftell(fp);
    rewind(fp);
    srcbuf.resize(len + 1);
    len = fread(&srcbuf.at(0), 1, len, fp);
    srcbuf[len] = 0;
    fclose(fp);

    const GLchar *srcs[1];
    srcs[0] = &srcbuf.at(0);

    shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, srcs, NULL);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &val);
    if (val != GL_TRUE)
    {
        char log[4096];
        GLsizei msglen;
        glGetShaderInfoLog(shader, 4096, &msglen, log);
        printf("%s\n", log);
        // assert(val == GL_TRUE && "failed to compile shader");
        printf("ERR: Failed to load or compile shader [ %s ]\n",
               shaderSourceFilename);
        return false;
    }

    printf("Load shader [ %s ] OK\n", shaderSourceFilename);
    return true;
}

bool LinkShader(GLuint &prog, GLuint &vertShader, GLuint &fragShader)
{
    GLint val = 0;

    if (prog != 0)
    {
        glDeleteProgram(prog);
    }

    prog = glCreateProgram();

    glAttachShader(prog, vertShader);
    glAttachShader(prog, fragShader);
    glLinkProgram(prog);

    glGetProgramiv(prog, GL_LINK_STATUS, &val);
    assert(val == GL_TRUE && "failed to link shader");

    printf("Link shader OK\n");

    return true;
}

static size_t ComponentTypeByteSize(int type)
{
    switch (type)
    {
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
        case TINYGLTF_COMPONENT_TYPE_BYTE:
            return sizeof(char);
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
        case TINYGLTF_COMPONENT_TYPE_SHORT:
            return sizeof(short);
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
        case TINYGLTF_COMPONENT_TYPE_INT:
            return sizeof(int);
        case TINYGLTF_COMPONENT_TYPE_FLOAT:
            return sizeof(float);
        case TINYGLTF_COMPONENT_TYPE_DOUBLE:
            return sizeof(double);
        default:
            return 0;
    }
}

static void SetupMeshState(tinygltf::Model &model, GLuint progId)
{
    // Buffer
    {
        for (size_t i = 0; i < model.bufferViews.size(); i++)
        {
            const tinygltf::BufferView &bufferView = model.bufferViews[i];
            if (bufferView.target == 0)
            {
                std::cout << "WARN: bufferView.target is zero" << std::endl;
                continue; // Unsupported bufferView.
            }

            int sparse_accessor = -1;
            for (size_t a_i = 0; a_i < model.accessors.size(); ++a_i)
            {
                const auto &accessor = model.accessors[a_i];
                if (accessor.bufferView == i)
                {
                    std::cout << i << " is used by accessor " << a_i << std::endl;
                    if (accessor.sparse.isSparse)
                    {
                        std::cout
                            << "WARN: this bufferView has at least one sparse accessor to "
                               "it. We are going to load the data as patched by this "
                               "sparse accessor, not the original data"
                            << std::endl;
                        sparse_accessor = a_i;
                        break;
                    }
                }
            }

            const tinygltf::Buffer &buffer = model.buffers[static_cast<size_t>(bufferView.buffer)];
            GLBufferState state;
            glGenBuffers(1, &state.vb);
            glBindBuffer(static_cast<GLenum>(bufferView.target), state.vb);
            std::cout << "buffer.size= " << buffer.data.size()
                      << ", byteOffset = " << bufferView.byteOffset << std::endl;

            if (sparse_accessor < 0)
            {
                glBufferData(bufferView.target, bufferView.byteLength,
                             &buffer.data.at(0) + bufferView.byteOffset,
                             GL_STATIC_DRAW);
            }
            else
            {
                const auto accessor = model.accessors[sparse_accessor];
                // copy the buffer to a temporary one for sparse patching
                unsigned char *tmp_buffer = new unsigned char[bufferView.byteLength];
                memcpy(tmp_buffer, buffer.data.data() + bufferView.byteOffset,
                       bufferView.byteLength);

                const size_t size_of_object_in_buffer =
                    ComponentTypeByteSize(accessor.componentType);
                const size_t size_of_sparse_indices =
                    ComponentTypeByteSize(accessor.sparse.indices.componentType);

                const auto &indices_buffer_view =
                    model.bufferViews[accessor.sparse.indices.bufferView];
                const auto &indices_buffer = model.buffers[indices_buffer_view.buffer];

                const auto &values_buffer_view =
                    model.bufferViews[accessor.sparse.values.bufferView];
                const auto &values_buffer = model.buffers[values_buffer_view.buffer];

                for (size_t sparse_index = 0; sparse_index < accessor.sparse.count;
                     ++sparse_index)
                {
                    int index = 0;
                    // std::cout << "accessor.sparse.indices.componentType = " <<
                    // accessor.sparse.indices.componentType << std::endl;
                    switch (accessor.sparse.indices.componentType)
                    {
                        case TINYGLTF_COMPONENT_TYPE_BYTE:
                        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                            index = (int)*(
                                unsigned char *)(indices_buffer.data.data() +
                                                 indices_buffer_view.byteOffset +
                                                 accessor.sparse.indices.byteOffset +
                                                 (sparse_index * size_of_sparse_indices));
                            break;
                        case TINYGLTF_COMPONENT_TYPE_SHORT:
                        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                            index = (int)*(
                                unsigned short *)(indices_buffer.data.data() +
                                                  indices_buffer_view.byteOffset +
                                                  accessor.sparse.indices.byteOffset +
                                                  (sparse_index * size_of_sparse_indices));
                            break;
                        case TINYGLTF_COMPONENT_TYPE_INT:
                        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
                            index = (int)*(
                                unsigned int *)(indices_buffer.data.data() +
                                                indices_buffer_view.byteOffset +
                                                accessor.sparse.indices.byteOffset +
                                                (sparse_index * size_of_sparse_indices));
                            break;
                    }
                    std::cout << "updating sparse data at index  : " << index
                              << std::endl;
                    // index is now the target of the sparse index to patch in
                    const unsigned char *read_from =
                        values_buffer.data.data() +
                        (values_buffer_view.byteOffset +
                         accessor.sparse.values.byteOffset) +
                        (sparse_index * (size_of_object_in_buffer * accessor.type));

                    unsigned char *write_to =
                        tmp_buffer + index * (size_of_object_in_buffer * accessor.type);

                    memcpy(write_to, read_from, size_of_object_in_buffer * accessor.type);
                }

                glBufferData(bufferView.target, bufferView.byteLength, tmp_buffer,
                             GL_STATIC_DRAW);
                delete[] tmp_buffer;
            }
            glBindBuffer(static_cast<GLenum>(bufferView.target), 0);

            gBufferState[static_cast<int>(i)] = state;
        }
    }

    glUseProgram(progId);
    GLint vtloc = glGetAttribLocation(progId, "in_vertex");
    GLint nrmloc = glGetAttribLocation(progId, "in_normal");
    GLint uvloc = glGetAttribLocation(progId, "in_texcoord");

    // GLint diffuseTexLoc = glGetUniformLocation(progId, "diffuseTex");
    GLint isCurvesLoc = glGetUniformLocation(progId, "uIsCurves");

    gGLProgramState.attribs["POSITION"] = vtloc;
    gGLProgramState.attribs["NORMAL"] = nrmloc;
    gGLProgramState.attribs["TEXCOORD_0"] = uvloc;
    // gGLProgramState.uniforms["diffuseTex"] = diffuseTexLoc;
    gGLProgramState.uniforms["isCurvesLoc"] = isCurvesLoc;
};

void CheckErrors(std::string desc)
{
    GLenum e = glGetError();
    if (e != GL_NO_ERROR)
    {
        fprintf(stderr, "OpenGL error in \"%s\": %d (%d)\n", desc.c_str(), e, e);
        exit(20);
    }
}

static std::string GetFilePathExtension(const std::string &FileName)
{
    if (FileName.find_last_of(".") != std::string::npos)
        return FileName.substr(FileName.find_last_of(".") + 1);
    return "";
}

static void DrawMesh(tinygltf::Model &model, const tinygltf::Mesh &mesh)
{
    if (gGLProgramState.uniforms["isCurvesLoc"] >= 0)
    {
        glUniform1i(gGLProgramState.uniforms["isCurvesLoc"], 0);
    }

    for (size_t i = 0; i < mesh.primitives.size(); i++)
    {
        const tinygltf::Primitive &primitive = mesh.primitives[i];

        if (primitive.indices < 0) return;

        // Assume TEXTURE_2D target for the texture object.
        // glBindTexture(GL_TEXTURE_2D, gMeshState[mesh.name].diffuseTex[i]);

        std::map<std::string, int>::const_iterator it(primitive.attributes.begin());
        std::map<std::string, int>::const_iterator itEnd(
            primitive.attributes.end());

        for (; it != itEnd; it++)
        {
            assert(it->second >= 0);
            const tinygltf::Accessor &accessor = model.accessors[it->second];
            glBindBuffer(GL_ARRAY_BUFFER, gBufferState[accessor.bufferView].vb);
            CheckErrors("bind buffer");
            int size = 1;
            if (accessor.type == TINYGLTF_TYPE_SCALAR)
            {
                size = 1;
            }
            else if (accessor.type == TINYGLTF_TYPE_VEC2)
            {
                size = 2;
            }
            else if (accessor.type == TINYGLTF_TYPE_VEC3)
            {
                size = 3;
            }
            else if (accessor.type == TINYGLTF_TYPE_VEC4)
            {
                size = 4;
            }
            else
            {
                assert(0);
            }
            // it->first would be "POSITION", "NORMAL", "TEXCOORD_0", ...
            if ((it->first.compare("POSITION") == 0) ||
                (it->first.compare("NORMAL") == 0) ||
                (it->first.compare("TEXCOORD_0") == 0))
            {
                if (gGLProgramState.attribs[it->first] >= 0)
                {
                    // Compute byteStride from Accessor + BufferView combination.
                    int byteStride =
                        accessor.ByteStride(model.bufferViews[accessor.bufferView]);
                    assert(byteStride != -1);
                    glVertexAttribPointer(gGLProgramState.attribs[it->first], size,
                                          accessor.componentType,
                                          accessor.normalized ? GL_TRUE : GL_FALSE,
                                          byteStride, BUFFER_OFFSET(accessor.byteOffset));
                    CheckErrors("vertex attrib pointer");
                    glEnableVertexAttribArray(gGLProgramState.attribs[it->first]);
                    CheckErrors("enable vertex attrib array");
                }
            }
        }

        const tinygltf::Accessor &indexAccessor =
            model.accessors[primitive.indices];
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,
                     gBufferState[indexAccessor.bufferView].vb);
        CheckErrors("bind buffer");
        int mode = -1;
        if (primitive.mode == TINYGLTF_MODE_TRIANGLES)
        {
            mode = GL_TRIANGLES;
        }
        else if (primitive.mode == TINYGLTF_MODE_TRIANGLE_STRIP)
        {
            mode = GL_TRIANGLE_STRIP;
        }
        else if (primitive.mode == TINYGLTF_MODE_TRIANGLE_FAN)
        {
            mode = GL_TRIANGLE_FAN;
        }
        else if (primitive.mode == TINYGLTF_MODE_POINTS)
        {
            mode = GL_POINTS;
        }
        else if (primitive.mode == TINYGLTF_MODE_LINE)
        {
            mode = GL_LINES;
        }
        else if (primitive.mode == TINYGLTF_MODE_LINE_LOOP)
        {
            mode = GL_LINE_LOOP;
        }
        else
        {
            assert(0);
        }
        glDrawElements(mode, indexAccessor.count, indexAccessor.componentType, BUFFER_OFFSET(indexAccessor.byteOffset));
        CheckErrors("draw elements");

        {
            std::map<std::string, int>::const_iterator it(
                primitive.attributes.begin());
            std::map<std::string, int>::const_iterator itEnd(
                primitive.attributes.end());

            for (; it != itEnd; it++)
            {
                if ((it->first.compare("POSITION") == 0) ||
                    (it->first.compare("NORMAL") == 0) ||
                    (it->first.compare("TEXCOORD_0") == 0))
                {
                    if (gGLProgramState.attribs[it->first] >= 0)
                    {
                        glDisableVertexAttribArray(gGLProgramState.attribs[it->first]);
                    }
                }
            }
        }
    }
}

// Hierarchically draw nodes
static void DrawNode(tinygltf::Model &model, const tinygltf::Node &node)
{
    // Apply xform

    glPushMatrix();
    if (node.matrix.size() == 16)
    {
        // Use `matrix' attribute
        glMultMatrixd(node.matrix.data());
    }
    else
    {
        // Assume Trans x Rotate x Scale order
        if (node.scale.size() == 3)
        {
            glScaled(node.scale[0], node.scale[1], node.scale[2]);
        }

        if (node.rotation.size() == 4)
        {
            glRotated(node.rotation[0], node.rotation[1], node.rotation[2],
                      node.rotation[3]);
        }

        if (node.translation.size() == 3)
        {
            glTranslated(node.translation[0], node.translation[1],
                         node.translation[2]);
        }
    }

    if (node.mesh > -1)
    {
        assert(node.mesh < model.meshes.size());
        DrawMesh(model, model.meshes[node.mesh]);
    }

    // Draw child nodes.
    for (size_t i = 0; i < node.children.size(); i++)
    {
        assert(node.children[i] < model.nodes.size());
        DrawNode(model, model.nodes[node.children[i]]);
    }

    glPopMatrix();
}

static void DrawModel(tinygltf::Model &model)
{
    // If the glTF asset has at least one scene, and doesn't define a default one
    // just show the first one we can find
    assert(model.scenes.size() > 0);
    int scene_to_display = model.defaultScene > -1 ? model.defaultScene : 0;
    const tinygltf::Scene &scene = model.scenes[scene_to_display];
    for (size_t i = 0; i < scene.nodes.size(); i++)
    {
        DrawNode(model, model.nodes[scene.nodes[i]]);
    }
}
}

using namespace gltf_code;

static Model model;

AppTiny::AppTiny()
{}

bool AppTiny::OnInit()
{/*
    auto root = BuildScene();

    TinyGLTF loader;
    std::string err;
    std::string warn;

    bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, "assets/Duck.gltf");

    if (!warn.empty())
    {
        printf("Warn: %s\n", warn.c_str());
    }

    if (!err.empty())
    {
        printf("Err: %s\n", err.c_str());
    }

    if (!ret)
    {
        printf("Failed to parse glTF\n");
        return false;
    }

    GLuint vertId = 0, fragId = 0, progId = 0;

    const char *shader_frag_filename = "assets/shader.frag";
    const char *shader_vert_filename = "assets/shader.vert";

    if (false == LoadShader(GL_VERTEX_SHADER, vertId, shader_vert_filename))
    {
        return false;
    }
    CheckErrors("load vert shader");

    if (false == LoadShader(GL_FRAGMENT_SHADER, fragId, shader_frag_filename))
    {
        return false;
    }
    CheckErrors("load frag shader");

    if (false == LinkShader(progId, vertId, fragId))
    {
        return false;
    }

    CheckErrors("link");

    {
        // At least `in_vertex` should be used in the shader.
        GLint vtxLoc = glGetAttribLocation(progId, "in_vertex");
        if (vtxLoc < 0)
        {
            printf("vertex loc not found.\n");
            exit(-1);
        }
    }

    glUseProgram(progId);
    CheckErrors("useProgram");

    SetupMeshState(model, progId);*/

    glClearColor(0, 0.5f, 1, 1);

    return true;
}

void AppTiny::OnTick(double /*timeInMs*/)
{
}

void AppTiny::OnRender()
{
//    DrawModel(model);
}

void AppTiny::OnDestroy()
{
}

void AppTiny::OnKeyAction(int /*key*/, int /*scancode*/, int /*action*/, int /*mods*/)
{
}

void AppTiny::OnResize(int width, int height)
{
    glViewport(0, 0, width, height);
}
