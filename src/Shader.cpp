#include "Shader.h"
#include <GL/glew.h>
#include <fstream>
#include <iostream>
#include <string.h>

std::string TextFileRead(std::string fileName)
{
    std::ifstream fs;
    std::string   text;

    if(!fileName.empty())
    {
        fs.open(fileName.c_str(), std::ios_base::in);

        if(fs.is_open())
        {
            fs.seekg(0, std::ios::end);
            text.reserve(fs.tellg());
            fs.seekg(0, std::ios::beg);

            text.assign((std::istreambuf_iterator<char>(fs)), std::istreambuf_iterator<char>());

            fs.close();
        }
    }

    return text;
}

void ValidateShader(GLuint shader, std::string file)
{
    const unsigned int BUFFER_SIZE = 512;
    char               buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    GLsizei length = 0;

    glGetShaderInfoLog(shader, BUFFER_SIZE, &length, buffer);
    if(length > 0)
    {
        std::cout << "Shader " << shader << " (" << file << ") compile error: " << buffer << std::endl;
    }
}

void ValidateProgram(GLuint program)
{
    const unsigned int BUFFER_SIZE = 512;
    char               buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    GLsizei length = 0;

    memset(buffer, 0, BUFFER_SIZE);
    glGetProgramInfoLog(program, BUFFER_SIZE, &length, buffer);
    if(length > 0)
        std::cout << "Program " << program << " link error: " << buffer << std::endl;

    glValidateProgram(program);
    GLint status;
    glGetProgramiv(program, GL_VALIDATE_STATUS, &status);
    if(status == GL_FALSE)
        std::cout << "Error validating shader " << program << std::endl;
}

void Shader::Init(std::string vsfile, std::string fsfile)
{
    shader_vp = glCreateShader(GL_VERTEX_SHADER);
    shader_fp = glCreateShader(GL_FRAGMENT_SHADER);

    std::string vsText = TextFileRead(vsfile.c_str());
    std::string fsText = TextFileRead(fsfile.c_str());

    if(vsText.size() == 0 || fsText.size() == 0)
    {
        std::cerr << "Either vertex shader or fragment shader file not found." << std::endl;
        return;
    }

    const char * vert = vsText.c_str();
    const char * frag = fsText.c_str();
    glShaderSource(shader_vp, 1, (const GLchar **)&vert, 0);
    glShaderSource(shader_fp, 1, (const GLchar **)&frag, 0);

    glCompileShader(shader_vp);
    ValidateShader(shader_vp, vsfile);
    glCompileShader(shader_fp);
    ValidateShader(shader_fp, fsfile);

    shader_id = glCreateProgram();
    glAttachShader(shader_id, shader_fp);
    glAttachShader(shader_id, shader_vp);

    glBindAttribLocation(shader_id, 0, "position");
    glBindAttribLocation(shader_id, 1, "inNormal");
    glBindAttribLocation(shader_id, 2, "tex");

    glLinkProgram(shader_id);
    ValidateProgram(shader_id);
}

void Shader::DeInit()
{
    glDetachShader(shader_id, shader_fp);
    glDetachShader(shader_id, shader_vp);

    glDeleteShader(shader_fp);
    glDeleteShader(shader_vp);
    glDeleteProgram(shader_id);
}

void Shader::Bind()
{
    glUseProgram(shader_id);
}

void Shader::Unbind()
{
    glUseProgram(0);
}

uint32_t Shader::Id()
{
    return shader_id;
}
