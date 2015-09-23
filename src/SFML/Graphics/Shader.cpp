////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2015 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

// Disable warning C4996: Calling std::copy_n() with raw pointers may be unsafe
#ifdef _MSC_VER
    #define _SCL_SECURE_NO_WARNINGS
#endif


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/GLCheck.hpp>
#include <SFML/Window/Context.hpp>
#include <SFML/System/InputStream.hpp>
#include <SFML/System/Mutex.hpp>
#include <SFML/System/Lock.hpp>
#include <SFML/System/Err.hpp>
#include <fstream>
#include <vector>
#include <algorithm>


#ifndef SFML_OPENGL_ES

#if defined(SFML_SYSTEM_MACOS) || defined(SFML_SYSTEM_IOS)

    #define castToGlHandle(x) reinterpret_cast<GLEXT_GLhandle>(static_cast<ptrdiff_t>(x))
    #define castFromGlHandle(x) static_cast<unsigned int>(reinterpret_cast<ptrdiff_t>(x))

#else

    #define castToGlHandle(x) (x)
    #define castFromGlHandle(x) (x)

#endif

namespace
{
    sf::Mutex mutex;

    GLint checkMaxTextureUnits()
    {
        GLint maxUnits = 0;
        glCheck(glGetIntegerv(GLEXT_GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxUnits));

        return maxUnits;
    }

    // Retrieve the maximum number of texture units available
    GLint getMaxTextureUnits()
    {
        // TODO: Remove this lock when it becomes unnecessary in C++11
        sf::Lock lock(mutex);

        static GLint maxUnits = checkMaxTextureUnits();

        return maxUnits;
    }

    // Read the contents of a file into an array of char
    bool getFileContents(const std::string& filename, std::vector<char>& buffer)
    {
        std::ifstream file(filename.c_str(), std::ios_base::binary);
        if (file)
        {
            file.seekg(0, std::ios_base::end);
            std::streamsize size = file.tellg();
            if (size > 0)
            {
                file.seekg(0, std::ios_base::beg);
                buffer.resize(static_cast<std::size_t>(size));
                file.read(&buffer[0], size);
            }
            buffer.push_back('\0');
            return true;
        }
        else
        {
            return false;
        }
    }

    // Read the contents of a stream into an array of char
    bool getStreamContents(sf::InputStream& stream, std::vector<char>& buffer)
    {
        bool success = true;
        sf::Int64 size = stream.getSize();
        if (size > 0)
        {
            buffer.resize(static_cast<std::size_t>(size));
            stream.seek(0);
            sf::Int64 read = stream.read(&buffer[0], size);
            success = (read == size);
        }
        buffer.push_back('\0');
        return success;
    }

    bool checkShadersAvailable()
    {
        // Create a temporary context in case the user checks
        // before a GlResource is created, thus initializing
        // the shared context
        if (!sf::Context::getActiveContext())
        {
            sf::Context context;

            // Make sure that extensions are initialized
            sf::priv::ensureExtensionsInit();

            bool available = GLEXT_multitexture         &&
                             GLEXT_shading_language_100 &&
                             GLEXT_shader_objects       &&
                             GLEXT_vertex_shader        &&
                             GLEXT_fragment_shader;

            return available;
        }

        // Make sure that extensions are initialized
        sf::priv::ensureExtensionsInit();

        bool available = GLEXT_multitexture         &&
                         GLEXT_shading_language_100 &&
                         GLEXT_shader_objects       &&
                         GLEXT_vertex_shader        &&
                         GLEXT_fragment_shader;

        return available;
    }


    // Functors to call a glUniform*() variant
    template <typename Arg0>
    struct UniformSetter1
    {
        typedef void (GL_FUNCPTR *FuncPtr)(GLint, Arg0);

        UniformSetter1(FuncPtr function, Arg0 v0) :
        function(function),
        v0(v0)
        {
        }

        void operator() (GLint location) const
        {
            function(location, v0);
        }

        FuncPtr function;
        Arg0    v0;
    };

    template <typename Arg0, typename Arg1>
    struct UniformSetter2
    {
        typedef void (GL_FUNCPTR *FuncPtr)(GLint, Arg0, Arg1);

        UniformSetter2(FuncPtr function, Arg0 v0, Arg1 v1) :
        function(function),
        v0(v0),
        v1(v1)
        {
        }

        void operator() (GLint location) const
        {
            function(location, v0, v1);
        }

        FuncPtr function;
        Arg0    v0;
        Arg1    v1;
    };

    template <typename Arg0, typename Arg1, typename Arg2>
    struct UniformSetter3
    {
        typedef void (GL_FUNCPTR *FuncPtr)(GLint, Arg0, Arg1, Arg2);

        UniformSetter3(FuncPtr function, Arg0 v0, Arg1 v1, Arg2 v2) :
        function(function),
        v0(v0),
        v1(v1),
        v2(v2)
        {
        }

        void operator() (GLint location) const
        {
            function(location, v0, v1, v2);
        }

        FuncPtr function;
        Arg0    v0;
        Arg1    v1;
        Arg2    v2;
    };

    template <typename Arg0, typename Arg1, typename Arg2, typename Arg3>
    struct UniformSetter4
    {
        typedef void (GL_FUNCPTR *FuncPtr)(GLint, Arg0, Arg1, Arg2, Arg3);

        UniformSetter4(FuncPtr function, Arg0 v0, Arg1 v1, Arg2 v2, Arg3 v3) :
        function(function),
        v0(v0),
        v1(v1),
        v2(v2),
        v3(v3)
        {
        }

        void operator() (GLint location) const
        {
            function(location, v0, v1, v2, v3);
        }

        FuncPtr function;
        Arg0    v0;
        Arg1    v1;
        Arg2    v2;
        Arg3    v3;
    };

    // Transforms an array of 2D vectors into a contiguous array of scalars
    template <typename T>
    std::vector<T> createContiguousArray(const sf::Vector2<T>* vectorArray, std::size_t length)
    {
        const std::size_t vectorSize = 2;

        std::vector<T> contiguous(vectorSize * length);
        for (std::size_t i = 0; i < length; ++i)
        {
            contiguous[vectorSize * i]     = vectorArray[i].x;
            contiguous[vectorSize * i + 1] = vectorArray[i].y;
        }

        return contiguous;
    }

    // Transforms an array of 3D vectors into a contiguous array of scalars
    template <typename T>
    std::vector<T> createContiguousArray(const sf::Vector3<T>* vectorArray, std::size_t length)
    {
        const std::size_t vectorSize = 3;

        std::vector<T> contiguous(vectorSize * length);
        for (std::size_t i = 0; i < length; ++i)
        {
            contiguous[vectorSize * i]     = vectorArray[i].x;
            contiguous[vectorSize * i + 1] = vectorArray[i].y;
            contiguous[vectorSize * i + 2] = vectorArray[i].z;
        }

        return contiguous;
    }

    // Transforms an array of 4D vectors into a contiguous array of scalars
    template <typename T>
    std::vector<T> createContiguousArray(const sf::priv::Vector4<T>* vectorArray, std::size_t length)
    {
        const std::size_t vectorSize = 4;

        std::vector<T> contiguous(vectorSize * length);
        for (std::size_t i = 0; i < length; ++i)
        {
            contiguous[vectorSize * i]     = vectorArray[i].x;
            contiguous[vectorSize * i + 1] = vectorArray[i].y;
            contiguous[vectorSize * i + 2] = vectorArray[i].z;
            contiguous[vectorSize * i + 3] = vectorArray[i].w;
        }

        return contiguous;
    }
}


namespace sf
{
////////////////////////////////////////////////////////////
Shader::CurrentTextureType Shader::CurrentTexture;


////////////////////////////////////////////////////////////
Shader::Shader() :
m_shaderProgram (0),
m_currentTexture(-1),
m_textures      (),
m_uniforms      ()
{
}


////////////////////////////////////////////////////////////
Shader::~Shader()
{
    ensureGlContext();

    // Destroy effect program
    if (m_shaderProgram)
        glCheck(GLEXT_glDeleteObject(castToGlHandle(m_shaderProgram)));
}


////////////////////////////////////////////////////////////
bool Shader::loadFromFile(const std::string& filename, Type type)
{
    // Read the file
    std::vector<char> shader;
    if (!getFileContents(filename, shader))
    {
        err() << "Failed to open shader file \"" << filename << "\"" << std::endl;
        return false;
    }

    // Compile the shader program
    if (type == Vertex)
        return compile(&shader[0], NULL);
    else
        return compile(NULL, &shader[0]);
}


////////////////////////////////////////////////////////////
bool Shader::loadFromFile(const std::string& vertexShaderFilename, const std::string& fragmentShaderFilename)
{
    // Read the vertex shader file
    std::vector<char> vertexShader;
    if (!getFileContents(vertexShaderFilename, vertexShader))
    {
        err() << "Failed to open vertex shader file \"" << vertexShaderFilename << "\"" << std::endl;
        return false;
    }

    // Read the fragment shader file
    std::vector<char> fragmentShader;
    if (!getFileContents(fragmentShaderFilename, fragmentShader))
    {
        err() << "Failed to open fragment shader file \"" << fragmentShaderFilename << "\"" << std::endl;
        return false;
    }

    // Compile the shader program
    return compile(&vertexShader[0], &fragmentShader[0]);
}


////////////////////////////////////////////////////////////
bool Shader::loadFromMemory(const std::string& shader, Type type)
{
    // Compile the shader program
    if (type == Vertex)
        return compile(shader.c_str(), NULL);
    else
        return compile(NULL, shader.c_str());
}


////////////////////////////////////////////////////////////
bool Shader::loadFromMemory(const std::string& vertexShader, const std::string& fragmentShader)
{
    // Compile the shader program
    return compile(vertexShader.c_str(), fragmentShader.c_str());
}


////////////////////////////////////////////////////////////
bool Shader::loadFromStream(InputStream& stream, Type type)
{
    // Read the shader code from the stream
    std::vector<char> shader;
    if (!getStreamContents(stream, shader))
    {
        err() << "Failed to read shader from stream" << std::endl;
        return false;
    }

    // Compile the shader program
    if (type == Vertex)
        return compile(&shader[0], NULL);
    else
        return compile(NULL, &shader[0]);
}


////////////////////////////////////////////////////////////
bool Shader::loadFromStream(InputStream& vertexShaderStream, InputStream& fragmentShaderStream)
{
    // Read the vertex shader code from the stream
    std::vector<char> vertexShader;
    if (!getStreamContents(vertexShaderStream, vertexShader))
    {
        err() << "Failed to read vertex shader from stream" << std::endl;
        return false;
    }

    // Read the fragment shader code from the stream
    std::vector<char> fragmentShader;
    if (!getStreamContents(fragmentShaderStream, fragmentShader))
    {
        err() << "Failed to read fragment shader from stream" << std::endl;
        return false;
    }

    // Compile the shader program
    return compile(&vertexShader[0], &fragmentShader[0]);
}


////////////////////////////////////////////////////////////
void Shader::setUniformFloat(const std::string& name, float x)
{
    UniformSetter1<GLfloat> setter(GLEXT_glUniform1f, x);
    setUniformImpl(name, setter);
}


////////////////////////////////////////////////////////////
void Shader::setUniformVec2(const std::string& name, const Glsl::Vec2& v)
{
    UniformSetter2<GLfloat, GLfloat> setter(GLEXT_glUniform2f, v.x, v.y);
    setUniformImpl(name, setter);
}


////////////////////////////////////////////////////////////
void Shader::setUniformVec3(const std::string& name, const Glsl::Vec3& v)
{
    UniformSetter3<GLfloat, GLfloat, GLfloat> setter(GLEXT_glUniform3f, v.x, v.y, v.z);
    setUniformImpl(name, setter);
}


////////////////////////////////////////////////////////////
void Shader::setUniformVec4(const std::string& name, const Glsl::Vec4& v)
{
    UniformSetter4<GLfloat, GLfloat, GLfloat, GLfloat> setter(GLEXT_glUniform4f, v.x, v.y, v.z, v.w);
    setUniformImpl(name, setter);
}


////////////////////////////////////////////////////////////
void Shader::setUniformVec4(const std::string& name, const Color& color)
{
    setUniformVec4(name, Glsl::Vec4(color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f));
}


////////////////////////////////////////////////////////////
void Shader::setUniformInt(const std::string& name, int x)
{
    UniformSetter1<GLint> setter(GLEXT_glUniform1i, x);
    setUniformImpl(name, setter);
}


////////////////////////////////////////////////////////////
void Shader::setUniformIvec2(const std::string& name, const Glsl::Ivec2& v)
{
    UniformSetter2<GLint, GLint> setter(GLEXT_glUniform2i, v.x, v.y);
    setUniformImpl(name, setter);
}


////////////////////////////////////////////////////////////
void Shader::setUniformIvec3(const std::string& name, const Glsl::Ivec3& v)
{
    UniformSetter3<GLint, GLint, GLint> setter(GLEXT_glUniform3i, v.x, v.y, v.z);
    setUniformImpl(name, setter);
}


////////////////////////////////////////////////////////////
void Shader::setUniformIvec4(const std::string& name, const Glsl::Ivec4& v)
{
    UniformSetter4<GLint, GLint, GLint, GLint> setter(GLEXT_glUniform4i, v.x, v.y, v.z, v.w);
    setUniformImpl(name, setter);
}


////////////////////////////////////////////////////////////
void Shader::setUniformBool(const std::string& name, bool x)
{
    setUniformInt(name, static_cast<int>(x));
}


////////////////////////////////////////////////////////////
void Shader::setUniformBvec2(const std::string& name, const Glsl::Bvec2& v)
{
    setUniformIvec2(name, Glsl::Ivec2(v));
}


////////////////////////////////////////////////////////////
void Shader::setUniformBvec3(const std::string& name, const Glsl::Bvec3& v)
{
    setUniformIvec3(name, Glsl::Ivec3(v));
}


////////////////////////////////////////////////////////////
void Shader::setUniformBvec4(const std::string& name, const Glsl::Bvec4& v)
{
    setUniformIvec4(name, Glsl::Ivec4(v));
}


////////////////////////////////////////////////////////////
void Shader::setUniformMat3(const std::string& name, const float* pointer)
{
    UniformSetter3<GLsizei, GLboolean, const GLfloat*> setter(GLEXT_glUniformMatrix3fv, 1, GL_FALSE, pointer);
    setUniformImpl(name, setter);
}


////////////////////////////////////////////////////////////
void Shader::setUniformMat3(const std::string& name, const Glsl::Mat3& matrix)
{
    setUniformMat3(name, matrix.array);
}


////////////////////////////////////////////////////////////
void Shader::setUniformMat4(const std::string& name, const float* pointer)
{
    UniformSetter3<GLsizei, GLboolean, const GLfloat*> setter(GLEXT_glUniformMatrix4fv, 1, GL_FALSE, pointer);
    setUniformImpl(name, setter);
}


////////////////////////////////////////////////////////////
void Shader::setUniformMat4(const std::string& name, const Glsl::Mat4& matrix)
{
    setUniformMat4(name, matrix.array);
}


////////////////////////////////////////////////////////////
void Shader::setUniformMat4(const std::string& name, const Transform& transform)
{
    setUniformMat4(name, transform.getMatrix());
}


////////////////////////////////////////////////////////////
void Shader::setUniformFloatArray(const std::string& name, const float* valueArray, std::size_t length)
{
    UniformSetter2<GLsizei, const GLfloat*> setter(GLEXT_glUniform1fv, length, valueArray);
    setUniformImpl(name, setter);
}


////////////////////////////////////////////////////////////
void Shader::setUniformVec2Array(const std::string& name, const Glsl::Vec2* vectorArray, std::size_t length)
{
    std::vector<float> contiguous = createContiguousArray(vectorArray, length);
    UniformSetter2<GLsizei, const GLfloat*> setter(GLEXT_glUniform2fv, length, &contiguous[0]);
    setUniformImpl(name, setter);
}


////////////////////////////////////////////////////////////
void Shader::setUniformVec3Array(const std::string& name, const Glsl::Vec3* vectorArray, std::size_t length)
{
    std::vector<float> contiguous = createContiguousArray(vectorArray, length);
    UniformSetter2<GLsizei, const GLfloat*> setter(GLEXT_glUniform3fv, length, &contiguous[0]);
    setUniformImpl(name, setter);
}


////////////////////////////////////////////////////////////
void Shader::setUniformVec4Array(const std::string& name, const Glsl::Vec4* vectorArray, std::size_t length)
{
    std::vector<float> contiguous = createContiguousArray(vectorArray, length);
    UniformSetter2<GLsizei, const GLfloat*> setter(GLEXT_glUniform4fv, length, &contiguous[0]);
    setUniformImpl(name, setter);
}


////////////////////////////////////////////////////////////
void Shader::setUniformMat3Array(const std::string& name, const Glsl::Mat3* matrixArray, std::size_t length)
{
    const std::size_t matrixSize = 3 * 3;

    std::vector<float> contiguous(matrixSize * length);
    for (std::size_t i = 0; i < length; ++i)
        std::copy(matrixArray[i].array, matrixArray[i].array + matrixSize, &contiguous[matrixSize * i]);

    UniformSetter3<GLsizei, GLboolean, const GLfloat*> setter(GLEXT_glUniformMatrix3fv, length, GL_FALSE, &contiguous[0]);
    setUniformImpl(name, setter);
}


////////////////////////////////////////////////////////////
void Shader::setUniformMat4Array(const std::string& name, const Glsl::Mat4* matrixArray, std::size_t length)
{
    const std::size_t matrixSize = 4 * 4;

    std::vector<float> contiguous(matrixSize * length);
    for (std::size_t i = 0; i < length; ++i)
        std::copy(matrixArray[i].array, matrixArray[i].array + matrixSize, &contiguous[matrixSize * i]);

    UniformSetter3<GLsizei, GLboolean, const GLfloat*> setter(GLEXT_glUniformMatrix4fv, length, GL_FALSE, &contiguous[0]);
    setUniformImpl(name, setter);
}


////////////////////////////////////////////////////////////
void Shader::setUniformSampler2D(const std::string& name, const Texture& texture)
{
    if (m_shaderProgram)
    {
        ensureGlContext();

        // Find the location of the variable in the shader
        int location = getUniformLocation(name);
        if (location != -1)
        {
            // Store the location -> texture mapping
            TextureTable::iterator it = m_textures.find(location);
            if (it == m_textures.end())
            {
                // New entry, make sure there are enough texture units
                GLint maxUnits = getMaxTextureUnits();
                if (m_textures.size() + 1 >= static_cast<std::size_t>(maxUnits))
                {
                    err() << "Impossible to use texture \"" << name << "\" for shader: all available texture units are used" << std::endl;
                    return;
                }

                m_textures[location] = &texture;
            }
            else
            {
                // Location already used, just replace the texture
                it->second = &texture;
            }
        }
    }
}


////////////////////////////////////////////////////////////
void Shader::setUniformSampler2D(const std::string& name, CurrentTextureType)
{
    if (m_shaderProgram)
    {
        ensureGlContext();

        // Find the location of the variable in the shader
        m_currentTexture = getUniformLocation(name);
    }
}


////////////////////////////////////////////////////////////
void Shader::setParameter(const std::string& name, float x)
{
    setUniformFloat(name, x);
}


////////////////////////////////////////////////////////////
void Shader::setParameter(const std::string& name, float x, float y)
{
    setUniformVec2(name, Glsl::Vec2(x, y));
}


////////////////////////////////////////////////////////////
void Shader::setParameter(const std::string& name, float x, float y, float z)
{
    setUniformVec3(name, Glsl::Vec3(x, y, z));
}


////////////////////////////////////////////////////////////
void Shader::setParameter(const std::string& name, float x, float y, float z, float w)
{
    setUniformVec4(name, Glsl::Vec4(x, y, z, w));
}


////////////////////////////////////////////////////////////
void Shader::setParameter(const std::string& name, const Vector2f& v)
{
    setUniformVec2(name, v);
}


////////////////////////////////////////////////////////////
void Shader::setParameter(const std::string& name, const Vector3f& v)
{
    setUniformVec3(name, v);
}


////////////////////////////////////////////////////////////
void Shader::setParameter(const std::string& name, const Color& color)
{
    setUniformVec4(name, color);
}


////////////////////////////////////////////////////////////
void Shader::setParameter(const std::string& name, const Transform& transform)
{
    setUniformMat4(name, transform);
}


////////////////////////////////////////////////////////////
void Shader::setParameter(const std::string& name, const Texture& texture)
{
    setUniformSampler2D(name, texture);
}


////////////////////////////////////////////////////////////
void Shader::setParameter(const std::string& name, CurrentTextureType)
{
    setUniformSampler2D(name, CurrentTexture);
}


////////////////////////////////////////////////////////////
unsigned int Shader::getNativeHandle() const
{
    return m_shaderProgram;
}


////////////////////////////////////////////////////////////
void Shader::bind(const Shader* shader)
{
    ensureGlContext();

    // Make sure that we can use shaders
    if (!isAvailable())
    {
        err() << "Failed to bind or unbind shader: your system doesn't support shaders "
              << "(you should test Shader::isAvailable() before trying to use the Shader class)" << std::endl;
        return;
    }

    if (shader && shader->m_shaderProgram)
    {
        // Enable the program
        glCheck(GLEXT_glUseProgramObject(castToGlHandle(shader->m_shaderProgram)));

        // Bind the textures
        shader->bindTextures();

        // Bind the current texture
        if (shader->m_currentTexture != -1)
            glCheck(GLEXT_glUniform1i(shader->m_currentTexture, 0));
    }
    else
    {
        // Bind no shader
        glCheck(GLEXT_glUseProgramObject(0));
    }
}


////////////////////////////////////////////////////////////
bool Shader::isAvailable()
{
    // TODO: Remove this lock when it becomes unnecessary in C++11
    Lock lock(mutex);

    static bool available = checkShadersAvailable();

    return available;
}


////////////////////////////////////////////////////////////
bool Shader::compile(const char* vertexShaderCode, const char* fragmentShaderCode)
{
    ensureGlContext();

    // First make sure that we can use shaders
    if (!isAvailable())
    {
        err() << "Failed to create a shader: your system doesn't support shaders "
              << "(you should test Shader::isAvailable() before trying to use the Shader class)" << std::endl;
        return false;
    }

    // Destroy the shader if it was already created
    if (m_shaderProgram)
    {
        glCheck(GLEXT_glDeleteObject(castToGlHandle(m_shaderProgram)));
        m_shaderProgram = 0;
    }

    // Reset the internal state
    m_currentTexture = -1;
    m_textures.clear();
    m_uniforms.clear();

    // Create the program
    GLEXT_GLhandle shaderProgram;
    glCheck(shaderProgram = GLEXT_glCreateProgramObject());

    // Create the vertex shader if needed
    if (vertexShaderCode)
    {
        // Create and compile the shader
        GLEXT_GLhandle vertexShader;
        glCheck(vertexShader = GLEXT_glCreateShaderObject(GLEXT_GL_VERTEX_SHADER));
        glCheck(GLEXT_glShaderSource(vertexShader, 1, &vertexShaderCode, NULL));
        glCheck(GLEXT_glCompileShader(vertexShader));

        // Check the compile log
        GLint success;
        glCheck(GLEXT_glGetObjectParameteriv(vertexShader, GLEXT_GL_OBJECT_COMPILE_STATUS, &success));
        if (success == GL_FALSE)
        {
            char log[1024];
            glCheck(GLEXT_glGetInfoLog(vertexShader, sizeof(log), 0, log));
            err() << "Failed to compile vertex shader:" << std::endl
                  << log << std::endl;
            glCheck(GLEXT_glDeleteObject(vertexShader));
            glCheck(GLEXT_glDeleteObject(shaderProgram));
            return false;
        }

        // Attach the shader to the program, and delete it (not needed anymore)
        glCheck(GLEXT_glAttachObject(shaderProgram, vertexShader));
        glCheck(GLEXT_glDeleteObject(vertexShader));
    }

    // Create the fragment shader if needed
    if (fragmentShaderCode)
    {
        // Create and compile the shader
        GLEXT_GLhandle fragmentShader;
        glCheck(fragmentShader = GLEXT_glCreateShaderObject(GLEXT_GL_FRAGMENT_SHADER));
        glCheck(GLEXT_glShaderSource(fragmentShader, 1, &fragmentShaderCode, NULL));
        glCheck(GLEXT_glCompileShader(fragmentShader));

        // Check the compile log
        GLint success;
        glCheck(GLEXT_glGetObjectParameteriv(fragmentShader, GLEXT_GL_OBJECT_COMPILE_STATUS, &success));
        if (success == GL_FALSE)
        {
            char log[1024];
            glCheck(GLEXT_glGetInfoLog(fragmentShader, sizeof(log), 0, log));
            err() << "Failed to compile fragment shader:" << std::endl
                  << log << std::endl;
            glCheck(GLEXT_glDeleteObject(fragmentShader));
            glCheck(GLEXT_glDeleteObject(shaderProgram));
            return false;
        }

        // Attach the shader to the program, and delete it (not needed anymore)
        glCheck(GLEXT_glAttachObject(shaderProgram, fragmentShader));
        glCheck(GLEXT_glDeleteObject(fragmentShader));
    }

    // Link the program
    glCheck(GLEXT_glLinkProgram(shaderProgram));

    // Check the link log
    GLint success;
    glCheck(GLEXT_glGetObjectParameteriv(shaderProgram, GLEXT_GL_OBJECT_LINK_STATUS, &success));
    if (success == GL_FALSE)
    {
        char log[1024];
        glCheck(GLEXT_glGetInfoLog(shaderProgram, sizeof(log), 0, log));
        err() << "Failed to link shader:" << std::endl
              << log << std::endl;
        glCheck(GLEXT_glDeleteObject(shaderProgram));
        return false;
    }

    m_shaderProgram = castFromGlHandle(shaderProgram);

    // Force an OpenGL flush, so that the shader will appear updated
    // in all contexts immediately (solves problems in multi-threaded apps)
    glCheck(glFlush());

    return true;
}


////////////////////////////////////////////////////////////
void Shader::bindTextures() const
{
    TextureTable::const_iterator it = m_textures.begin();
    for (std::size_t i = 0; i < m_textures.size(); ++i)
    {
        GLint index = static_cast<GLsizei>(i + 1);
        glCheck(GLEXT_glUniform1i(it->first, index));
        glCheck(GLEXT_glActiveTexture(GLEXT_GL_TEXTURE0 + index));
        Texture::bind(it->second);
        ++it;
    }

    // Make sure that the texture unit which is left active is the number 0
    glCheck(GLEXT_glActiveTexture(GLEXT_GL_TEXTURE0));
}


////////////////////////////////////////////////////////////
int Shader::getUniformLocation(const std::string& name)
{
    // Check the cache
    UniformTable::const_iterator it = m_uniforms.find(name);
    if (it != m_uniforms.end())
    {
        // Already in cache, return it
        return it->second;
    }
    else
    {
        // Not in cache, request the location from OpenGL
        int location = GLEXT_glGetUniformLocation(castToGlHandle(m_shaderProgram), name.c_str());
        m_uniforms.insert(std::make_pair(name, location));

        if (location == -1)
            err() << "Parameter \"" << name << "\" not found in shader" << std::endl;

        return location;
    }
}

template <typename F>
void Shader::setUniformImpl(const std::string& name, const F& functor)
{
    if (m_shaderProgram)
    {
        ensureGlContext();

        // Enable program
        GLEXT_GLhandle program;
        glCheck(program = GLEXT_glGetHandle(GLEXT_GL_PROGRAM_OBJECT));
        glCheck(GLEXT_glUseProgramObject(castToGlHandle(m_shaderProgram)));

        // Get uniform location and assign it new values
        GLint location = getUniformLocation(name);
        if (location != -1)
            glCheck(functor(location));

        // Disable program
        glCheck(GLEXT_glUseProgramObject(program));
    }
}

} // namespace sf

#else // SFML_OPENGL_ES

// OpenGL ES 1 doesn't support GLSL shaders at all, we have to provide an empty implementation

namespace sf
{
////////////////////////////////////////////////////////////
Shader::CurrentTextureType Shader::CurrentTexture;


////////////////////////////////////////////////////////////
Shader::Shader() :
m_shaderProgram (0),
m_currentTexture(-1)
{
}


////////////////////////////////////////////////////////////
Shader::~Shader()
{
}


////////////////////////////////////////////////////////////
bool Shader::loadFromFile(const std::string& filename, Type type)
{
    return false;
}


////////////////////////////////////////////////////////////
bool Shader::loadFromFile(const std::string& vertexShaderFilename, const std::string& fragmentShaderFilename)
{
    return false;
}


////////////////////////////////////////////////////////////
bool Shader::loadFromMemory(const std::string& shader, Type type)
{
    return false;
}


////////////////////////////////////////////////////////////
bool Shader::loadFromMemory(const std::string& vertexShader, const std::string& fragmentShader)
{
    return false;
}


////////////////////////////////////////////////////////////
bool Shader::loadFromStream(InputStream& stream, Type type)
{
    return false;
}


////////////////////////////////////////////////////////////
bool Shader::loadFromStream(InputStream& vertexShaderStream, InputStream& fragmentShaderStream)
{
    return false;
}


////////////////////////////////////////////////////////////
void Shader::setParameter(const std::string& name, int x)
{
}


////////////////////////////////////////////////////////////
void Shader::setParameter(const std::string& name, int x, int y)
{
}


////////////////////////////////////////////////////////////
void Shader::setParameter(const std::string& name, int x, int y, int z)
{
}


////////////////////////////////////////////////////////////
void Shader::setParameter(const std::string& name, int x, int y, int z, int w)
{
}


////////////////////////////////////////////////////////////
void Shader::setParameter(const std::string& name, const Vector2f& v)
{
}


////////////////////////////////////////////////////////////
void Shader::setParameter(const std::string& name, const Vector3f& v)
{
}


////////////////////////////////////////////////////////////
void Shader::setParameter(const std::string& name, const Color& color)
{
}


////////////////////////////////////////////////////////////
void Shader::setParameter(const std::string& name, const Transform& transform)
{
}


////////////////////////////////////////////////////////////
void Shader::setParameter(const std::string& name, const Texture& texture)
{
}


////////////////////////////////////////////////////////////
void Shader::setParameter(const std::string& name, CurrentTextureType)
{
}


////////////////////////////////////////////////////////////
unsigned int Shader::getNativeHandle() const
{
    return 0;
}


////////////////////////////////////////////////////////////
void Shader::bind(const Shader* shader)
{
}


////////////////////////////////////////////////////////////
bool Shader::isAvailable()
{
    return false;
}


////////////////////////////////////////////////////////////
bool Shader::compile(const char* vertexShaderCode, const char* fragmentShaderCode)
{
    return false;
}


////////////////////////////////////////////////////////////
void Shader::bindTextures() const
{
}

} // namespace sf

#endif // SFML_OPENGL_ES
