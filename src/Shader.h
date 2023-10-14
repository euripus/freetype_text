#ifndef SHADER_H
#define SHADER_H

#include <cstdint>
#include <string>

class Shader
{
public:
    Shader() {}
    virtual ~Shader() { DeInit(); }

    void Init(std::string vsfile, std::string fsfile);
    void DeInit();

    void Bind();
    void Unbind();

    uint32_t Id();

protected:
private:
    uint32_t shader_id;
    uint32_t shader_vp;
    uint32_t shader_fp;
};

#endif   // SHADER_H
