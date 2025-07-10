#ifndef TEXTURE_H
#define TEXTURE_H

#include <glm/glm.hpp>
#include <string>
#include <array>

class RendererBase;
class Texture
{
public:
    enum class Type
    {
        TEXTURE_NOTYPE,
        TEXTURE_2D,
        TEXTURE_3D,
        TEXTURE_CUBE,
        QUANTITY
    };

    // the sides of a cube map
    enum class CubeFace
    {
        POS_X,
        NEG_X,
        POS_Y,
        NEG_Y,
        POS_Z,
        NEG_Z,
    };

    // pixel formats
    enum class Format
    {
        NOFORMAT,
        R8G8B8,
        R8G8B8A8,
        DXT1,
        DXT3,
        DXT5,
        DEPTH,
        QUANTITY
    };

    enum class Filter
    {
        NEAREST,
        LINEAR,
        NEAREST_MIPMAP_NEAREST,
        LINEAR_MIPMAP_NEAREST,
        NEAREST_MIPMAP_LINEAR,
        LINEAR_MIPMAP_LINEAR,
        QUANTITY
    };

    enum class Wrap
    {
        CLAMP,
        CLAMP_TO_BORDER,
        CLAMP_TO_EDGE,
        MIRRORED_REPEAT,
        REPEAT,
        QUANTITY
    };

    struct SamplerState
    {
        Filter    min            = Filter::NEAREST;
        Filter    max            = Filter::NEAREST;
        Wrap      s              = Wrap::REPEAT;
        Wrap      t              = Wrap::REPEAT;
        Wrap      r              = Wrap::REPEAT;
        glm::vec4 border_color   = {0.0f, 0.0f, 0.0f, 0.0f};
        float     max_anisotropy = 1.0f;
        bool      compare_mode   = false;
    };

    bool loadImageDataFromFile(std::string const & fname, RendererBase const & render);
    bool loadCubeMapFromFiles(std::array<char const *, 6> const & fnames, RendererBase const & render);

    // protected:
    bool         m_committed = false;
    bool         m_gen_mips  = true;
    Type         m_type      = Type::TEXTURE_NOTYPE;
    Format       m_format    = Format::NOFORMAT;
    SamplerState m_sampler   = {};
    uint32_t     m_width     = 0;
    uint32_t     m_height    = 0;
    uint32_t     m_depth     = 0;

    uint32_t m_render_id = 0;

    friend class RendererBase;
};

struct Light
{
    enum class LightType
    {
        Point,
        Directional,
        Spot
    };

    LightType m_type            = LightType::Directional;
    float     m_range           = 0.f;
    glm::vec4 m_diffuse         = glm::vec4(0.f);
    glm::vec4 m_specular        = glm::vec4(0.f);
    glm::vec4 m_ambient         = glm::vec4(0.f);
    glm::vec4 m_position        = glm::vec4(0.f);
    glm::vec3 m_spot_direction  = glm::vec3(0.f);
    float     m_spot_exponent   = 0.f;
    float     m_spot_cos_cutoff = 0.f;
    // bool      m_cast_shadows    = false;
};

struct TextureProjector
{
    bool  is_ortho      = false;
    bool  is_reflection = false;
    bool  is_cube_map   = false;
    float fovy          = 45.f;

    Texture const * projected_texture = nullptr;

    glm::mat4 modelview  = glm::mat4(1.f);
    glm::mat4 reflection = glm::mat4(1.f);

    glm::mat4 getTransformMatrix() const;
    glm::mat4 getProjectionMatrix() const;
    glm::mat4 getModelviewMatrix() const;

    static glm::mat4 GetReflectionMatrix(glm::vec4 const & plane);
    static glm::vec4 GetPlaneFromPoints(glm::vec3 const & p0, glm::vec3 const & p1, glm::vec3 const & p2);
};

struct CombineStage
{
    enum class CombineMode
    {
        ADD,   // [fragment color] Operation [texture color]
        MODULATE,
        DECAL,
        BLEND,
        REPLACE,
        COMBINE,   // custom combine function
        QUANTITY
    };

    enum class CombineFunctions
    {
        REPLACE,
        MODULATE,
        ADD,
        ADD_SIGNED,
        INTERPOLATE,
        SUBTRACT,
        DOT3_RGB,
        DOT3_RGBA,
        QUANTITY
    };

    enum class SrcType
    {
        TEXTURE,
        TEXTURE_STAGE,
        CONSTANT,
        PRIMARY_COLOR,
        PREVIOUS,
        QUANTITY
    };

    enum class OperandType
    {
        SRC_COLOR,
        ONE_MINUS_SRC_COLOR,
        SRC_ALPHA,
        ONE_MINUS_SRC_ALPHA,
        QUANTITY
    };

    CombineMode      mode                = CombineMode::MODULATE;
    CombineFunctions rgb_func            = CombineFunctions::REPLACE;
    CombineFunctions alpha_func          = CombineFunctions::REPLACE;
    int32_t          rgb_scale           = 0;
    int32_t          alpha_scale         = 0;
    SrcType          rgb_src0            = SrcType::PREVIOUS;
    uint32_t         rgb_stage0          = 0;
    SrcType          rgb_src1            = SrcType::PREVIOUS;
    uint32_t         rgb_stage1          = 0;
    SrcType          rgb_src2            = SrcType::PREVIOUS;
    uint32_t         rgb_stage2          = 0;
    SrcType          alpha_src0          = SrcType::PREVIOUS;
    uint32_t         alpha_stage0        = 0;
    SrcType          alpha_src1          = SrcType::PREVIOUS;
    uint32_t         alpha_stage1        = 0;
    SrcType          alpha_src2          = SrcType::PREVIOUS;
    uint32_t         alpha_stage2        = 0;
    glm::vec4        constant_color      = glm::vec4(0.f);
    OperandType      rgb_operand0        = OperandType::SRC_COLOR;
    OperandType      rgb_operand1        = OperandType::SRC_COLOR;
    OperandType      rgb_operand2        = OperandType::SRC_COLOR;
    OperandType      alpha_operand0      = OperandType::SRC_ALPHA;
    OperandType      alpha_operand1      = OperandType::SRC_ALPHA;
    OperandType      alpha_operand2      = OperandType::SRC_ALPHA;
    bool             const_color_enabled = false;
};

struct TextureSlot
{
    enum class TexCoordSource
    {
        TEX_COORD_GENERATED,
        TEX_COORD_BUFFER
    };

    enum class CubeMapGenMode
    {
        NORMAL,
        REFLECTION
    };

    TexCoordSource coord_source    = TexCoordSource::TEX_COORD_BUFFER;
    CubeMapGenMode cube_map_mode   = CubeMapGenMode::REFLECTION;
    std::uint32_t  tex_channel_num = 0;   // num of active channel in tex_coords pool
    CombineStage   combine_mode    = {};

    Texture const *          texture   = nullptr;
    TextureProjector const * projector = nullptr;
};

#endif
