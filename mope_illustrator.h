/* 
* To use, #define MOPE_ILLUSTRATOR_IMPL before the #include in exactly one of your source files.
* 
* Subclass the Illustrator, and provide a window name, width, and height to the constructor.
* Overload bool ThreadStart(), bool FrameUpdate(double timeDelta), and void ThreadEnd().
*/

#pragma once

#include <chrono>
#include <thread>
#include <string>
#include <string_view>
#include <vector>
#include <cassert>
#include <unordered_map>

#include "mope_vec/mope_vec.h"

#if defined(_WIN32)
/*
* This header is currently strongly coupled with mope_window.h, which is a window class utilizing
* WINAPI. The plan is to decouple these ideas by providing making Illustrator into a class template
* and providing a window interface to the Illustrator.
*/
#define MOPE_WINDOW_IMPL
#include "mope_window.h"
#define GL_EXTRA_PROCS \
    GL_PROC(BOOL, wglSwapIntervalEXT, int interval)
#else
// Support other platforms?
#define GL_EXTRA_PROCS
#endif

#include <GL/GL.h>
#pragma comment(lib, "opengl32.lib")

// cruft
#pragma push_macro("near")
#pragma push_macro("far")
#undef near
#undef far

// We define OpenGL junk and load up extensions (this replaces a more rigorous
// extension loader--such as GLEW--for my limited purposes)
typedef intptr_t GLintptr;
typedef uintptr_t GLsizeiptr;
typedef char GLchar;

#define GL_ARRAY_BUFFER						0x8892
#define GL_ELEMENT_ARRAY_BUFFER				0x8893
#define GL_STREAM_DRAW						0x88E0
#define GL_STATIC_DRAW						0x88E4
#define GL_FRAGMENT_SHADER					0x8B30
#define GL_VERTEX_SHADER					0x8B31
#define	GL_COMPILE_STATUS					0x8B81
#define	GL_LINK_STATUS						0x8B82
#define GL_TEXTURE_3D						0x806F
#define GL_TEXTURE0							0x84C0

#define GL_PROC_LIST \
    GL_PROC(void,	glGenBuffers,				GLsizei n, GLuint* buffers) \
    GL_PROC(void,	glBindBuffer,				GLenum target, GLuint buffer) \
    GL_PROC(GLboolean,	glIsBuffer,				GLuint buffer) \
    GL_PROC(void,	glDeleteBuffers,			GLsizei n, GLuint* buffers) \
    GL_PROC(void,	glGenVertexArrays,			GLsizei n, GLuint* arrays) \
    GL_PROC(void,	glBindVertexArray,			GLuint array) \
    GL_PROC(void,	glDeleteVertexArrays,		GLsizei n, GLuint* arrays) \
    GL_PROC(GLuint,	glCreateShader,				GLenum shaderType) \
    GL_PROC(void,	glShaderSource,				GLuint shader, GLsizei count, const GLchar** string, const GLint *length) \
    GL_PROC(void,	glCompileShader,			GLuint shader) \
    GL_PROC(void,	glAttachShader,				GLuint program, GLuint shader) \
    GL_PROC(void,	glDetachShader,				GLuint program, GLuint shader) \
    GL_PROC(void,	glDeleteShader,				GLuint shader) \
    GL_PROC(GLuint,	glCreateProgram,			void) \
    GL_PROC(void,	glUseProgram,				GLuint program) \
    GL_PROC(void,	glDeleteProgram,			GLuint program) \
    GL_PROC(void,	glLinkProgram,				GLuint program) \
    GL_PROC(void,	glBufferData,				GLenum target, GLsizeiptr size, GLvoid* data, GLenum usage) \
    GL_PROC(void,	glBufferSubData,			GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data) \
    GL_PROC(void,	glVertexAttribPointer,		GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid * pointer) \
    GL_PROC(void,	glEnableVertexAttribArray,	GLuint index) \
    GL_PROC(void,	glDrawArraysInstanced,		GLenum mode, GLint first, GLsizei count, GLsizei primcount) \
    GL_PROC(void,	glDrawElementsInstanced,	GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei primcount) \
    GL_PROC(void,	glGenerateMipmap,			GLenum target) \
    GL_PROC(void,	glGetProgramInfoLog,		GLuint program, GLsizei maxLength, GLsizei* length, GLchar* infolog) \
    GL_PROC(void,	glGetProgramiv,				GLuint program, GLenum pname, GLint* params) \
    GL_PROC(void,	glGetShaderInfoLog,			GLuint shader, GLsizei maxLength, GLsizei* length, GLchar* infolog) \
    GL_PROC(void,	glGetShaderiv,				GLuint shader, GLenum pname, GLint* params) \
    GL_PROC(GLint,	glGetUniformLocation,		GLuint program, const GLchar* name) \
    GL_PROC(void,	glVertexAttribDivisor,		GLuint index, GLuint divisor) \
    GL_PROC(void,	glActiveTexture,			GLenum target) \
    GL_PROC(void,	glUniform1f,				GLint location, GLfloat v0) \
    GL_PROC(void,	glUniform2f,				GLint location, GLfloat v0, GLfloat v1) \
    GL_PROC(void,	glUniform3f,				GLint location, GLfloat v0, GLfloat v1, GLfloat v2) \
    GL_PROC(void,	glUniform4f,				GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) \
    GL_PROC(void,	glUniform1i,				GLint location, GLint v0) \
    GL_PROC(void,	glUniform2i,				GLint location, GLint v0, GLint v1) \
    GL_PROC(void,	glUniform3i,				GLint location, GLint v0, GLint v1, GLint v2) \
    GL_PROC(void,	glUniform4i,				GLint location, GLint v0, GLint v1, GLint v2, GLint v3) \
    GL_PROC(void,	glUniform1ui,				GLint location, GLuint v0) \
    GL_PROC(void,	glUniform2ui,				GLint location, GLuint v0, GLuint v1) \
    GL_PROC(void,	glUniform3ui,				GLint location, GLuint v0, GLuint v1, GLuint v2) \
    GL_PROC(void,	glUniform4ui,				GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3) \
    GL_PROC(void,	glUniform1fv,				GLint location, GLsizei count, const GLfloat *value) \
    GL_PROC(void,	glUniform2fv,				GLint location, GLsizei count, const GLfloat* value) \
    GL_PROC(void,	glUniform3fv,				GLint location, GLsizei count, const GLfloat* value) \
    GL_PROC(void,	glUniform4fv,				GLint location, GLsizei count, const GLfloat* value) \
    GL_PROC(void,	glUniform1iv,				GLint location, GLsizei count, const GLint* value) \
    GL_PROC(void,	glUniform2iv,				GLint location, GLsizei count, const GLint* value) \
    GL_PROC(void,	glUniform3iv,				GLint location, GLsizei count, const GLint* value) \
    GL_PROC(void,	glUniform4iv,				GLint location, GLsizei count, const GLint* value) \
    GL_PROC(void,	glUniform1uiv,				GLint location, GLsizei count, const GLuint* value) \
    GL_PROC(void,	glUniform2uiv,				GLint location, GLsizei count, const GLuint* value) \
    GL_PROC(void,	glUniform3uiv,				GLint location, GLsizei count, const GLuint* value) \
    GL_PROC(void,	glUniform4uiv,				GLint location, GLsizei count, const GLuint* value) \
    GL_PROC(void,	glUniformMatrix2fv,			GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) \
    GL_PROC(void,	glUniformMatrix3fv,			GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) \
    GL_PROC(void,	glUniformMatrix4fv,			GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) \
    GL_PROC(void,	glUniformMatrix2x3fv,		GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) \
    GL_PROC(void,	glUniformMatrix3x2fv,		GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) \
    GL_PROC(void,	glUniformMatrix2x4fv,		GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) \
    GL_PROC(void,	glUniformMatrix4x2fv,		GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) \
    GL_PROC(void,	glUniformMatrix3x4fv,		GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) \
    GL_PROC(void,	glUniformMatrix4x3fv,		GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) \
    GL_EXTRA_PROCS

#define GL_PROC(type, name, ...) \
    typedef type (WINAPI name##_t)(__VA_ARGS__); \
    extern name##_t* name;
GL_PROC_LIST
#undef GL_PROC	


/*============================================================================*\
|  Declarations                                                                |
\*============================================================================*/

namespace mope
{
    typedef vec4b Pixel;

    constexpr double pi = 3.14159265358979323846;
    constexpr float fPi = (float)pi;
    constexpr double tau = 2.0 * pi;
    constexpr float fTau = (float)tau;

    // Size information about the default fontsheet
    constexpr int glyphsPerRow = 16;
    constexpr int glyphRows = 6;
    constexpr vec2i pxGlyphSize = { 16, 32 };
    constexpr vec2i pxFontSheetSize = { pxGlyphSize.x() * glyphsPerRow, pxGlyphSize.y() * glyphRows };
    constexpr float glyphAspect = static_cast<float>(pxGlyphSize.x()) / pxGlyphSize.y();

    namespace gl
    {
        void PrintErrors(const char* location);
        void BindProcs();

        // Transformation matrices
        constexpr mat4f translation(const vec3f& offsets);
        constexpr mat4f scale(const vec3f& factors);
        mat4f rotation(const float angle, const vec3f& axis, const vec3f& origin);
        // Projection matrices
        constexpr mat4f ortho(
            const float left,
            const float right,
            const float bottom,
            const float top,
            const float near,
            const float far
        );
        mat4f perspective(
            const float fov,
            const float aspect,
            const float near,
            const float far
        );
    }

    /*========================================================================*\
    |  Shader                                                                  |
    \*========================================================================*/

    class Shader
    {
    public:
        GLuint ID();
        void Use();

        void Make(std::string_view srcVertex, std::string_view srcFragment);
        void Release();

        void SetUniform(std::string name, float value);
        void SetUniform(std::string name, int value);
        void SetUniform(std::string name, vec2f value);
        void SetUniform(std::string name, mat4f value, bool transpose = false);

        static GLuint UsedProgram;
        static void UseProgram(GLuint program);

    private:
        struct _Shader
        {
            _Shader();
            ~_Shader();
            GLuint id{ 0 };
        };

        std::shared_ptr<_Shader> m_shader{};
        std::unordered_map<std::string, int> m_cachedUniforms{};

        GLuint compile(std::string_view srcShader, GLenum type);
        void checkErrors(GLuint object, bool isProgram = false);
        int ensureCached(std::string name);
    };

    /*========================================================================*\
    |  Texture                                                                 |
    \*========================================================================*/

    class Texture
    {
    public:
        virtual ~Texture() = default;

        GLuint ID();
        virtual void Bind() = 0;
        void Release();

        static std::unordered_map<GLenum, GLuint> BoundTexture;
        static void BindTexture(GLenum target, GLuint texture);

    protected:
        struct _Texture
        {
            _Texture();
            ~_Texture();
            GLuint id{ 0 };
        };

        std::shared_ptr<_Texture> m_texture{};
    };

    class Texture2D : public Texture
    {
    public:
        ~Texture2D() = default;

        void Bind() override;
        void Make(std::string_view filename);
        void Make(
            GLsizei width,
            GLsizei height,
            Pixel* data,
            GLenum format = GL_RGBA,
            GLenum type = GL_UNSIGNED_BYTE
        );
    };

    /*========================================================================*\
    |  VAO, VBO, etc...                                                        |
    \*========================================================================*/

    class VAO
    {
    public:
        VAO();
        ~VAO();
        VAO(const VAO&) = delete;
        VAO& operator=(const VAO&) = delete;
        VAO(VAO&&) noexcept;
        VAO& operator=(VAO&&) noexcept;

        void Bind();
        void Release();

    private:
        GLuint m_id{ 0 };
    };

    class BufferObject
    {
    public:
        BufferObject(GLenum target);
        ~BufferObject();
        BufferObject(const BufferObject&) = delete;
        BufferObject& operator=(const BufferObject&) = delete;
        BufferObject(BufferObject&&) noexcept;
        BufferObject& operator=(BufferObject&&) noexcept;

        void Bind();
        void Release();

    protected:
        GLuint m_id{ 0 };
        GLenum m_target{ 0 };
    };

    class VBO : public BufferObject
    {
    public:
        VBO();
    };

    class EBO : public BufferObject
    {
    public:
        EBO();
    };

    /*========================================================================*\
    |  Camera                                                                  |
    \*========================================================================*/

    class Camera
    {
    public:
        Camera(Shader& shader, std::string uniform);

        void MoveForward(const float delta);
        void Strafe(const float delta);
        void ChangeYaw(const float delta);
        void ChangePitch(const float delta);
        void Update();

    private:
        // The shader managed by this camera
        Shader& m_shader;
        // The name of the view matrix uniform
        std::string m_uniform;

        vec3f m_position{ 0.f, 0.f, 0.f };
        float m_yaw{ 0.f };
        float m_pitch{ 0.f };

        vec3f basisX{ 1.f, 0.f, 0.f };
        vec3f basisY{ 0.f, 1.f, 0.f };
        vec3f basisZ{ 0.f, 0.f, 1.f };
    };

    /*========================================================================*\
    |  GameObject                                                              |
    \*========================================================================*/

    class GameObject
    {
    public:
        virtual void Render() = 0;
        void Move(const vec3f& direction);
        // void Rotate(const float angle);
        // void Scale(const vec3f& factors);

    protected:
        template<typename GLtype>
        void generateQuad(vec2f size, vec2f texBottomLeft, vec2f texTopRight,
            vec<5, GLfloat>* o_vertices, GLtype* o_indices, size_t offset = 0);

        void remakeModel();

        VAO m_vao{};
        VBO m_vbo{};
        EBO m_ebo{};

        vec3f m_scale{ 1.f, 1.f, 1.f };
        vec3f m_position{ 0.f, 0.f, -1.f };
        mat4f m_model{};
    };

    class Sprite : public GameObject
    {
    public:
        Sprite(
            Texture2D texture,
            Shader shader,
            vec2f size,
            vec2f bottomLeft = { 0.f, 0.f },
            vec2f topRight = { 1.f, 1.f });

        void Render() override;

    private:
        Texture2D m_texture;
        Shader m_shader;
    };

    class TextSprite : public GameObject
    {
    public:
        TextSprite(Texture2D texture, Shader shader, std::string_view text, vec2f letterSize);

        void Render() override;

    private:
        vec2i charToTexCoords(char c);
        vec2f scaleToSheetSize(vec2i in);

        Texture2D m_texture;
        Shader m_shader;
        size_t m_len;
    };

    /*========================================================================*\
    |  Illustrator                                                             |
    \*========================================================================*/

    class Illustrator
    {
    public:
        Illustrator(std::string windowName, vec2i windowDims);

        // User should overload this to do stuff to prepare for rendering.
        // Return false to abort starting the game.
        virtual bool ThreadStart();

        // User should overload this to draw stuff or whatever.
        // Return false to end the rendering loop and begin closing.
        virtual bool FrameUpdate(double deltaTime);

        // User should overload this to release OpenGL resources before context is destroyed.
        virtual void ThreadEnd();

        void Run();
        bool Pressed(Key k);
        bool Held(Key k);
        bool Released(Key k);
        vec2i MouseDeltas();

        void ToggleFps();
        // Overload ambiguity is good because it requires that we are explicit about
        // whether we are using bytes or floats to describe colors
        void SetClearColor(Pixel color);
        void SetClearColor(vec4f color);

    protected:
        Shader spriteShader{};
        Texture2D fontSheet{};

        // Purely conceptual Camera wrapper
        Camera camera{ spriteShader, "u_View" };

    private:
        // Internal representation of keystates
        uint64_t m_pressed[2]{ };
        uint64_t m_released[2]{ };
        uint64_t m_held[2]{ };

        // x and y deltas of mouse on last frame
        int m_xDelta{ 0 };
        int m_yDelta{ 0 };

        // Timing/FPS mechanisms
        unsigned int m_frameCount{ 0 };
        double m_frameTime{ 0 };
        double m_fpsUpdateTimer{ 0 };
        bool m_showFps{ true };

        std::string m_name{ "" };
        int m_windowWidth{ 0 };
        int m_windowHeight{ 0 };
        float m_userAspect{ 1.f };

        Window m_window;

    private:
        // Set things up for the core thread that need an OpenGL context
        void setupGL();
        // Tear things down after the core thread that need an OpenGL context
        void tearDownGL();

        void coreThread();

        bool isKeyState(Key k, uint64_t* state);
        void updateTitleBar();
        void updateWindow();
        void updateInputs();
        void buildFontSheet();
        void buildShader();
    };
}


#ifdef MOPE_ILLUSTRATOR_IMPL
#undef MOPE_ILLUSTRATOR_IMPL

/*============================================================================*\
|  Implementation                                                              |
\*============================================================================*/

#define GL_PROC(type, name, ...) name##_t* name;
GL_PROC_LIST
#undef GL_PROC

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace mope::gl
{
    void BindProcs()
    {
    #define GL_PROC(type, name, ...) name = (name##_t*)wglGetProcAddress(#name);
        GL_PROC_LIST
    #undef GL_PROC
    #undef GL_PROC_LIST
    }

    void PrintErrors(const char* location)
    {
        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR)
        {
            fprintf(stdout, "Error arose at %s. Code: 0x%04x\n", location, err);
        }
    }

    constexpr mat4f translation(const vec3f& offsets)
    {
        mat4f res = mat4f::identity();
        for (size_t i = 0; i < 3; ++i)
            res[3][i] = offsets[i];
        return res;
    }

    constexpr mat4f scale(const vec3f& factors)
    {
        mat4f res = mat4f::identity();
        for (size_t i = 0; i < 3; ++i)
            res[i][i] = factors[i];
        return res;
    }

    mat4f rotation(const float angle, const vec3f& axis, const vec3f& origin)
    {
        const vec3f unitaxis = (vec3f)axis.unit();
        const float a = origin[0];
        const float b = origin[1];
        const float c = origin[2];
        const float u = unitaxis[0];
        const float v = unitaxis[1];
        const float w = unitaxis[2];
        const float u2 = u * u;
        const float v2 = v * v;
        const float w2 = w * w;
        const float uv = u * v;
        const float uw = u * w;
        const float vw = v * w;
        const float ct = cos(angle);
        const float st = sin(angle);

        return mat4f{
            u2 + (v2 + w2) * ct,	uv * (1 - ct) + w * st,	uw * (1 - ct) - v * st, 0.f,
            uv * (1 - ct) - w * st,	v2 + (u2 + w2) * ct,	vw * (1 - ct) + u * st, 0.f,
            uw * (1 - ct) + v * st,	vw * (1 - ct) - u * st,	w2 + (u2 + v2) * ct, 0.f,

            (a * (v2 + w2) - u * (b * v + c * w)) * (1 - ct) + (b * w - c * v) * st,
            (b * (u2 + w2) - v * (a * u + c * w)) * (1 - ct) + (c * u - a * w) * st,
            (c * (u2 + v2) - w * (a * u + b * v)) * (1 - ct) + (a * v - b * u) * st,
            1.f
        };
    }

    constexpr mat4f ortho(
        const float left, const float right,
        const float bottom, const float top,
        const float near, const float far
    ) {
        const vec3f offsets{ -(right + left) / 2, -(top + bottom) / 2, -(far + near) / 2 };
        const mat4f T = translation(offsets);

        const vec3f factors{ 2.f / (right - left), 2.f / (top - bottom), -2.f / (far - near) };
        const mat4f S = scale(factors);

        return S * T;
    }

    mat4f perspective(
        const float fov, const float aspect,
        const float near, const float far
    ) {
        float top = near * tan(fov / 2.f);
        float right = aspect * top;

        return {
            near / right, 0.f, 0.f, 0.f,
            0.f, near / top, 0.f, 0.f,
            0.f, 0.f, (far + near) / (near - far), -1.f,
            0.f, 0.f, 2.f * far * near / (near - far), 0.f
        };
    }
}

namespace mope
{
    Illustrator::Illustrator(std::string windowName, vec2i windowDims)
        : m_name(windowName)
        , m_window(m_name.c_str(), windowDims.x(), windowDims.y())
    {
        m_window.GetDimensions(&m_windowWidth, &m_windowHeight);
        m_userAspect = m_window.AspectRatio();
    }


    /*========================================================================*\
    |  Main Illustrator API                                                    |
    \*========================================================================*/

    void Illustrator::Run()
    {
        std::thread t{ &Illustrator::coreThread, this };
        m_window.MessageLoop();
        t.join();
    }
    
    bool Illustrator::Pressed(Key k) { return isKeyState(k, m_pressed); }
    bool Illustrator::Held(Key k) { return isKeyState(k, m_held); }
    bool Illustrator::Released(Key k) { return isKeyState(k, m_released); }
    vec2i Illustrator::MouseDeltas() { return vec2i{ m_xDelta, m_yDelta }; }

    bool Illustrator::isKeyState(Key k, uint64_t* state)
    {
        uint8_t keyId = (uint8_t)k;
        size_t idx = (keyId & 0x40) >> 6;
        keyId &= 0xbf;
        uint64_t bit = 1ui64 << keyId;
        return state[idx] & bit;
    }

    void Illustrator::ToggleFps()
    {
        m_showFps = !m_showFps;
        if (m_showFps) {
            int fps = (int)(m_frameCount / m_fpsUpdateTimer);
            m_frameCount = 0;
            m_fpsUpdateTimer = 0;
            std::string strFps = m_name + " FPS: " + std::to_string(fps);
            m_window.SetTitle(strFps.c_str());
        }
        else {
            m_window.SetTitle(m_name.c_str());
        }
    }

    void Illustrator::SetClearColor(vec4f color)
    {
        glClearColor(color.x(), color.y(), color.z(), color.w());
    }

    void Illustrator::SetClearColor(Pixel color)
    {
        constexpr float factor = 1.f / 255.f;
        SetClearColor(factor * (vec4f)color);
    }

    bool Illustrator::ThreadStart()
    {
        return true;
    }

    bool Illustrator::FrameUpdate(double deltaTime)
    {
        // If this hasn't been overloaded, close immediately
        return false;
    }

    void Illustrator::ThreadEnd()
    {}


    /*========================================================================*\
    |  Setup and Tear Down                                                     |
    \*========================================================================*/

    void Illustrator::setupGL()
    {
        m_window.GetRenderingContext();

        gl::BindProcs();

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_LINE_SMOOTH);

#if defined(_WIN32)
        wglSwapIntervalEXT(0);
#else
        // Support other platforms?
#endif

        buildFontSheet();
        buildShader();
        camera.Update();
    }

    void Illustrator::tearDownGL()
    {
        fontSheet.Release();
        spriteShader.Release();
    }

    void Illustrator::buildShader()
    {
        const char* vertexSrc =
            "#version 330 core\n"
            "uniform mat4 u_Projection;\n"
            "uniform mat4 u_View;\n"
            "uniform mat4 u_Model;\n"
            "uniform sampler2D u_Texture;\n"
            "layout (location = 0) in vec3 i_Vertex;\n"
            "layout (location = 1) in vec2 i_TexCoord;\n"
            "out vec2 io_TexCoord;\n"
            "void main() {\n"
            "mat4 mvp = u_Projection * u_View * u_Model;\n"
            "gl_Position = mvp * vec4(i_Vertex, 1.0);\n"
            "io_TexCoord = i_TexCoord;\n"
            "}";

        const char* fragmentSrc =
            "#version 330 core\n"
            "uniform sampler2D u_Texture;\n"
            "in vec2 io_TexCoord;\n"
            "out vec4 o_Color;\n"
            "void main() {\n"
            "o_Color = texture(u_Texture, io_TexCoord);\n"
            "}";

        spriteShader.Make(vertexSrc, fragmentSrc);
        spriteShader.Use();
        spriteShader.SetUniform("u_Projection",
            gl::perspective(fPi / 4.f, m_window.AspectRatio(), 0.1f, 100.f)
        );
    }

    void Illustrator::buildFontSheet()
    {
        const char* dat =
            "OOOOOOOOOJQO,R7ROO9SO$QO,R7ROO9TCT*Q+TKR7ROO;SAU*Q+UJR7ROO<R@S-Q.SIR7ROO<R@R.Q/RIR#S+S#R(R-V-U(S#R*"
            "R+R%R%S'S(R+Z*R.Q/RIY)Y(R,X+V'U!R)T*R%R&S%S)R+Z*R.Q/RIS%R'R%S(R,P&S)S*S$S)T*R%R'S#S*R+S1R.Q/RIS%R'R"
            "%S(R3R)R+R%S)T*S#S(W*T+S0R.Q/RIR'R%R'R(R3R)R+R&R(V(T#T(U+T,R0R.Q/RIR'R%R'R(R2S)R+R&R(R!R(R!P#P!R(U*"
            "V+S/R.Q/RIR'R%R'R(R/V)R+R&R(R!R(R!S!R)S+R!R,S-R/Q0R.S7R'R%R'R(R-W*R+R&R'S!S'R!S!R*Q,R!R-S)T0Q1T$Q%V"
            "5R'R%R'R(R,V,R+R&R'R$R&R$Q$R(S*R$R-R)T0Q1T$W$Q5R'R%R'R(R,S/R+R&R'R$R&R$Q$R'U)R$R-S+R/Q0R)S<S%R'R%S("
            "S+R0R+R&R&R&R%R)R&W'S$S-S+R.Q/RIS%R'R%S(T%P%S%P*R+R&R&R&R%R)R&R#R'R&R.S*R.Q/RIY)Y(R!U&W&[&R&R&R&R$R"
            "+R$S#S&R&R'Z*R.Q/RIR#S+S#R(R#S(U'[&R&R%R(R#R+R#S%S$R(R&Z*R.Q/ROO0ROO4R.Q/ROO0ROO4R.Q/ROO0ROO4R.Q/RO"
            "O0ROO4S-Q.SOOOOGU*Q+UOOOOIT*Q+TOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO2UITOOOO5WHVOOOO3P&RKSOOOO"
            "9RKROOOO9RKROO.T#R%R#S,T+S#R)U+R-S#R%R&R&]*R)R&S*U$R#R#R%R&R)U9V!R%Y)W)Y'Y)R,U!R%R&R&]*R)R%S*V$R#R#"
            "R%R&R(W7S$T%S%R'S%Q'R$T&S&Q)R+S$S%R&R+R/R)R%R*S(R#R#R%R&R'R%R6R&S%S%R'R.R%S&R1R+R%S%R&R+R/R)R$S*R)R"
            "#R#R%R&R'R%R6R'R%R'R%R.R'R%R2R*R'R%R&R+R/R)S!S+R)R#R#R%R&R&R'R5R'R%R'R%R.R'R%R2R*R'R%R&R+R/R)W,R)R#"
            "R#R%R&R&R'R5S&R%R'R%R.R'R%](R*R'R%R&R+R/R)W,R)R#R#R%R&R&R'R6Z%R'R%R.R'R%](R*R'R%R&R+R/R)V-R)R#R#R%R"
            "&R&R'R8X%R'R%R.R'R%R(R(R*R'R%R&R+R/R)V-R)R#R#R%R&R&R'R>R%R'R%R.R'R%R(R(R*R'R%R&R+R/R)R!S,R)R#R#R%R&"
            "R&R'R>R%S%R'R-S%S&R&R)R+R%S%R&R+R/R)R#S+R)R#R#R%R&R'R%R7P'R&S%R'S%Q'R$T&S$S)R+S$S%S$S+R/R)R$S*R)R#R"
            "#R%S$S'R%R7Y&Y)W)Y'X&['Y%R!U(V+V)R%S)R)V!S&R!U)W:U(R#S,T+S#R)U'[(S#R%R#S)V+V)R&S(R)R!R!S&R#S+UHRGR9"
            "R:ROR1ROLRGR9R:ROR1RO2Q9RGR9R:ROR1RO1Q:RGR9S9R3R/R)R1RO0Q;RGR:V5R3R/R)R,WO/R;RGR;U5R3R/R)R,WO.ROOOO"
            "OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO6`OOOOO+`OOOOOO'QOOO1U9UO2SOOO0U0R'UO1ROOO2R2R+RHR2T(R(S%V.R+U-S*"
            "R&Q%S(S(R(])R2R+RHR0X&R(R$Y-R*W,S*R%R&R(R)R(])R1R,RHR/S$R&R'S$R%S,R)R%R+S*R%S%S&R*R(S2R1R,RHR/R&R%R"
            "'R%P)R+R(R'R)U(S%S&S$S*R)S1R0R-RHR/R&R%R&S.R+R(R'R)U(S%S'R$R+R*R1R0R-RHR.R(R$R&R/R+R(R'R)U(T#T'S!S+"
            "R*S0R/R.RHR.R(R$R%S/R+R(R'R)U(T#T(R!R,R+R0R/R.RHR.R(R$R$S/S+R(R'R(R#R'T#T(U-R+S/R.R/RHX(R(R$X.T,R(R"
            "'R(R#R'T#P!R)T-R,S.R.R/RHZ&R(R$Y)X,R(R'R(R#R&R#S#R)R-T,R.R.R/RHR&R&R(R$R&R'W.R(R'R'S#S%R#S#R)R-T,S-"
            "R-R0RHR'R%R(R$R'R%U1R(R'R'R%R%R#S#R(T+V,S,R-R0RHR'R%R(R$R'R%S3R(R'R'R%R%R#S#R'V*R!R-R,R,R1R(R)R3R'R"
            "%R(R$R'R%R4R(R'R'R%R%R$Q$R'R!R)S!S,S+R,R1R)R'R4R'R&R&R%R'R%R4R(R'R&R'R$R)R&S!S(R$R-R+R+R2R*R%R5R'R&"
            "R&R%R'R%R(P,R(R'R&R'R$R)R%S$S&R&R,S*R+R2R+R#R6R&R(R$R&R&R'S%Q,R(R'R&R'R#R+R$R&R%S&S,S)R*R3R,U7Y)X&Y"
            ")X&_!R'R%S'S!R+R#S&S$R(R#])R*R3R-S8X,T(X+V'_!R'R%R)R!R+R!S(S!S(S!])U&R1U-SOOOO5U9UOOOOOOOOOOOOOOOOO"
            "OOOOOOOOOOOOOOOOOOOOOOOOOOOO8VOOOOO2XOOOOO1S&POOOOO0ROOOOO8Q-R*R!X-U&V*[%R3U&R'R'Z&U)R(S#[%R(R$R&S)"
            "T)Q.S(S!Z)X%X([%R1Y$R'R'Z$Y'R(R$[%R(R$R&S'X'Q&S!Q#R(R#R&R(S%Q%R%R'R.R0R&R$R'R+R(Q&R'R'S$R.R(R$R%T&S"
            "$R&Q%X#R(R#R'R&R(P%R&R&R.R/R'R$R'R+R(P(R&R&S%R.R(R$R%T&R&R%Q%Q%R#S&S#R'R&R-R&R&R.R/R'R$R'R+R0R&R&R&"
            "R.R(R$R%T&R&R%Q$Q'Q$Z$R'R%R.R'R%R.R.R(R$R'R+R0R&R%S&R.R(R$R$Q!R%R(R$Q$Q'Q$Z$R'R%R.R'R%R.R.R(R$R'R+R"
            "0R&R$S'R.R(R$R$Q!R%R(R$Q$Q'Q$R&R$R'Q&R.R'R%R.R.R(R$R'R+R0R&R$R(R.R#R#R$R$Q!R%R(R$Q$Q'Q%R$R%R&R&R.R'"
            "R%R.R.R%U$R'R+R0R&S!S(R.R#R#R$R#Q#R%R(R$Q%Q%R%R$R%X(R.R'R%Z&Z&R%U$[+R0R&W)R.R#R#R$R#Q#R%R(R$Q%X%R$R"
            "%X(R.R'R%Z&Z&R.[+R0R&V*R.R#P!P#R$R#Q#R%R(R%Q&S!Q&R!R&R%R'R.R'R%R.R.R.R'R+R0R&R!R*R.R!Q!Q!R$R!Q$R%R("
            "R%Q+Q&R!R&R&R&R.R'R%R.R.R.R'R+R0R&R#R)R.R!Q!Q!R$R!Q$R%R(R%R*Q&R!R&R&R&R.R'R%R.R.R.R'R+R0R&R$R(R.R!P"
            "$P!R$R!Q$R%R(R&R(Q(T'R&R'R-R&R&R.R/R-R'R+R0R&R%R'R.T$T$T%R&R&R(R&R(T'R&R'S'P%R&R&R.R/R(P%R'R+R0R&R&"
            "R&R.T$T$T%R&R&R)X)T'R%R)S%Q%R%R'R.R0S%Q%R'R+R0R&R'R%R.T$T$T%R'R$R,U*T'Y*X%X([%[(X%R'R'Z'W&R(R$R.S&S"
            "$S&R'X<R(W.U&V*[%[*U&R'R'Z'W&R)R#R.S&S$S&R)TOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO"
            "OOOOBQOOOOO9ROOOOO9QOAS*Z%['U/R)V-T*R/U*U-R.RO0R,W(Z%[%Y-R(Y)W*R,Y'W,R.RO0R+S#S+R)R.P'S,R(P'S'R%R)R"
            ",R%R'P%S+R.R5P4P5R+R%R+R*S4S+R0R'R&R(S*R'R-R*R.R2S4S?R%R+R+S4R+R1R&Q'R)R*R'R-RLU4U<R'R*R,S3R$].R%R'"
            "R)R*R'R-SIT']'T.R*R'R*R-S2R$].R%R'R)S)R'R.RFU)])U+R*R'R*R.S0S$Q&R1R%R'R*R)R'R(S#RDTDT)R*R!R#R*R/S.S"
            "&Q%R1R%S&R*R*R%R'V!RDRHR)R*R!R#R*R0S)U(Q%R0R&S%R,R*W(R%S)R.R)TDT)S)R!R#R*R1S(V(Q$R)P&S&R!V,R+U(R&S)"
            "R.R+U)])U,R)R'R*R1S,S(Q#R)X'R#S.S(R%R&R'R)R.R.T']'T/S(R'R*R2R.R'Q#R)W(R5R'R'R%R'R)R.R0U4U2S'R'R*R2R"
            ".R(Q!R)R-R5R'R'R%R'RMS4S5S'R%R+R2R.R(Q!R)R.R4S&R'R%R'QO#P4P9R'R%R+R)P(S.R)T)R.R5R&R'R%R&ROIP'R'S#S'"
            "Q#R)R%S'P'R+S)R/S%P.R'R%R'R%ROIQ%S(W(V)Y(X,S)Y)W&[&Y(WOKW+S,T+U,U.R)Y*U'[(U+TONTOOOOOOOOOOOOOOOOOOO"
            "OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOEPOOOOO:PO3Q+QOQOOFPO2Q-QNRJROGPO2Q-QOQKRCR9Q$R+V/S(U#S8Q/QNR>R*RCR"
            "9R$Q*Y,U&W!R9Q/Q?Q.R>R+RBR:Q$Q*P%P!R+R#R$S%T8R/R>Q.R>R+RO0Q$Q/P#R*Q%Q#S&S9R/R>Q.R>R,RO/Q%Q.P#R*Q%Q#"
            "R&U7R1Q>QO.RO,_*P#R*R#R#R%S!Q7R1R=QO/R@R7_*P!S$Q&U$R%R$Q6R1R=Q=V:R@R;Q$Q-U&R%S%R$S$Q6R1R-P*^7V;R?R;"
            "Q%Q*V)R,Q#S%Q6R1R-P*^O*R?R<Q$Q)T.R*V&Q6R1R(Q$P$Q+QO0R?R<Q$Q(S!P0R)T>R1R(R#P#R+QO1R>R8_$R#P*S%R(S>R1"
            "R*V-QO1R>R,Q$Q&_$R#P)U&Q'R1Q-R1Q-R/QO2R=R,Q$Q+Q$Q'R#P(R#R,R2Q.R/R-R/QO2R=R,Q$Q+Q$Q'S!P$P$Q%Q,R2Q.R/"
            "R+V-QO3R<R,Q$Q+Q$R'X$Q%Q,R2Q/Q/Q*R#P#RO?R<R,Q$Q+Q%Q)U%R#R,S$P-Q/Q/Q*Q$P$QO@R;R,Q$Q,Q$Q+P)U.V-Q0Q-Q0"
            "POER;R,Q$Q=P*S0T.Q0Q-Q0POFRO=PO3Q+QOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO&";

        Pixel* pixels = new Pixel[pxFontSheetSize.x() * pxFontSheetSize.y()];
        Pixel* ptr = pixels;

        for (size_t i = 0; i < 4917; ++i)
        {
            char c = dat[i];
            char p = 0xff * (c > 'O');
            size_t n = (size_t)(c - '!' - (p & 46) + (c == '!') - (c >= '\\'));
            memset((void*)ptr, p, 4 * n);
            ptr += n;
        }

        fontSheet.Make(pxFontSheetSize.x(), pxFontSheetSize.y(), pixels);
        delete[] pixels;
    }


    /*========================================================================*\
    |  Core thread                                                             |
    \*========================================================================*/

    void Illustrator::coreThread()
    {
        setupGL();

        // Give the app a chance to set things up
        if (ThreadStart())
        {
            std::chrono::system_clock::time_point t1 = std::chrono::system_clock::now();
            std::chrono::system_clock::time_point t2;

            // Loop until the user requests close by returning false
            while (!m_window.WantsToClose())
            {
                // Find out how much time has passsed
                t2 = std::chrono::system_clock::now();
                std::chrono::duration<double> durElapsed = t2 - t1;
                m_frameTime = durElapsed.count();
                t1 = t2;

                // Some routine maintenance
                updateTitleBar();
                updateWindow();
                updateInputs();

                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                if (!FrameUpdate(m_frameTime)) break;
                m_window.Swap();
            }
        }

        // Give the app a chance to release OpenGL resources
        ThreadEnd();

        tearDownGL();
        m_window.Destroy();
    }

    void Illustrator::updateInputs()
    {
        m_xDelta = m_window.GetXDelta();
        m_yDelta = m_window.GetYDelta();

        std::array<uint64_t, 2> newstates = m_window.GetKeyStates();

        for (size_t idx = 0; idx < 2; idx++)
        {
            m_pressed[idx] = newstates[idx] & (~m_held[idx]);
            m_released[idx] = m_held[idx] & (~newstates[idx]);
            m_held[idx] = newstates[idx];
        }
    }

    void Illustrator::updateTitleBar()
    {
        m_fpsUpdateTimer += m_frameTime;
        m_frameCount++;
        if (m_fpsUpdateTimer >= 1.0)
        {
            if (m_showFps) {
                std::string strFps = m_name + " FPS: " + std::to_string(m_frameCount);
                m_window.SetTitle(strFps.c_str());
            }
            m_fpsUpdateTimer -= 1;
            m_frameCount = 0;
        }
    }

    void Illustrator::updateWindow()
    {
        // get current window size
        int newWidth, newHeight;
        m_window.GetDimensions(&newWidth, &newHeight);

        // make changes only if the window size has changed
        if (newWidth != m_windowWidth || newHeight != m_windowHeight) {

            m_windowWidth = newWidth;
            m_windowHeight = newHeight;

            // try to maintain original aspect ratio and keep the viewport centered
            float newAspect = m_window.AspectRatio();
            if (newAspect > m_userAspect) {
                int calcWidth = (int)(m_windowHeight * m_userAspect);
                int x = (m_windowWidth - calcWidth) / 2;
                glViewport(x, 0, calcWidth, m_windowHeight);
            }
            else {
                int calcHeight = (int)((float)m_windowWidth / m_userAspect);
                int y = (m_windowHeight - calcHeight) / 2;
                glViewport(0, y, m_windowWidth, calcHeight);
            }
        }
    }


    /*========================================================================*\
    |  Camera                                                                  |
    \*========================================================================*/

    Camera::Camera(Shader& shader, std::string uniform)
        : m_shader(shader), m_uniform(uniform)
    {}

    void Camera::MoveForward(float delta)
    {
        m_position += delta * basisZ;
    }

    void Camera::Strafe(float delta)
    {
        m_position += delta * basisX;
    }

    void Camera::ChangeYaw(float delta)
    {
        m_yaw -= delta;

        // fmod
        while (m_yaw >= fTau) { m_yaw -= fTau; };
        while (m_yaw < 0.f) { m_yaw += fTau; };
    }

    void Camera::ChangePitch(float delta)
    {
        m_pitch += delta;

        constexpr float upperbound = 0.90f * fPi;
        constexpr float lowerbound = -upperbound;
        // clamp
        if (m_pitch > upperbound) { m_pitch = upperbound; };
        if (m_pitch < lowerbound) { m_pitch = lowerbound; };
    }

    void Camera::Update()
    {
        // Update the transformation basis vectors
        basisZ = (vec3f{ sin(m_yaw) * cos(m_pitch), -sin(m_pitch), cos(m_yaw) * cos(m_pitch) }.funit());
        basisX = (vec3f{ 0.f, 1.f, 0.f }.cross(basisZ).funit());
        basisY = basisZ.cross(basisX);

        // Build the rotation matrix (inverse)
        mat4f changeOfBase = {
            basisX.x(), basisY.x(), basisZ.x(), 0.f,
            basisX.y(), basisY.y(), basisZ.y(), 0.f,
            basisX.z(), basisY.z(), basisZ.z(), 0.f,
            0.f, 0.f, 0.f, 1.f
        };

        // Build the translation matrix (inverse)
        mat4f translation = gl::translation(-m_position);

        // Upload to the shader
        m_shader.SetUniform(m_uniform.c_str(), changeOfBase * translation);
    }


    /*========================================================================*\
    |  Shader                                                                  |
    \*========================================================================*/

    GLuint Shader::UsedProgram = 0;

    void Shader::Make(std::string_view srcVertex, std::string_view srcFragment)
    {
        m_shader = std::make_shared<_Shader>();

        GLuint shader_vert = compile(srcVertex, GL_VERTEX_SHADER);
        checkErrors(shader_vert);

        GLuint shader_frag = compile(srcFragment, GL_FRAGMENT_SHADER);
        checkErrors(shader_frag);

        GLuint id = ID();
        glAttachShader(id, shader_vert);
        glAttachShader(id, shader_frag);
        glLinkProgram(id);
        checkErrors(id, true);

        glDetachShader(id, shader_vert);
        glDetachShader(id, shader_frag);
        glDeleteShader(shader_vert);
        glDeleteShader(shader_frag);
    }

    void Shader::Release()
    {
        m_shader.reset();
    }

    void Shader::UseProgram(GLuint program)
    {
        if (UsedProgram != program)
        {
            UsedProgram = program;
            glUseProgram(program);
        }
    }

    GLuint Shader::ID()
    {
        return m_shader->id;
    }

    void Shader::Use()
    {
        UseProgram(ID());
    }

    void Shader::SetUniform(std::string name, float value)
    {
        int loc = ensureCached(name);
        glUniform1f(loc, value);
    }

    void Shader::SetUniform(std::string name, int value)
    {
        int loc = ensureCached(name);
        glUniform1i(loc, value);
    }

    void Shader::SetUniform(std::string name, vec2f value)
    {
        int loc = ensureCached(name);
        glUniform2fv(loc, 1, value.elements);
    }

    void Shader::SetUniform(std::string name, mat4f value, bool transpose)
    {
        int loc = ensureCached(name);
        glUniformMatrix4fv(loc, 1, transpose, value.unpack().data());
    }

    GLuint Shader::compile(std::string_view srcShader, GLenum type)
    {
        const char* ptr = srcShader.data();

        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &ptr, 0);
        glCompileShader(shader);

        return shader;
    }

    void Shader::checkErrors(GLuint object, bool isProgram)
    {
        int success;
        char log[512];
        if (!isProgram) {
            glGetShaderiv(object, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(object, 512, NULL, log);
                std::cout << "Error arose at shader compilation:\n" << log << std::endl;
            }
        }
        else {
            glGetProgramiv(object, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(object, 512, NULL, log);
                std::cout << "Error arose at program linking:\n" << log << std::endl;
            }
        }
    }

    int Shader::ensureCached(std::string name)
    {
        int loc = m_cachedUniforms[name];
        if (loc) return loc;

        loc = glGetUniformLocation(ID(), name.data());
        m_cachedUniforms[name] = loc;
        return loc;
    }

    Shader::_Shader::_Shader()
    {
        id = glCreateProgram();
    }

    Shader::_Shader::~_Shader()
    {
        glDeleteProgram(id);
    }


    /*========================================================================*\
    |  Texture                                                                 |
    \*========================================================================*/

    std::unordered_map<GLenum, GLuint> Texture::BoundTexture = {
        {GL_TEXTURE_1D, 0},
        {GL_TEXTURE_2D, 0},
        {GL_TEXTURE_3D, 0}
    };

    void Texture::BindTexture(GLenum target, GLuint texture)
    {
        if (BoundTexture[target] != texture)
        {
            BoundTexture[target] = texture;
            glBindTexture(target, texture);
        }
    }

    GLuint Texture::ID()
    {
        return m_texture->id;
    }

    void Texture::Release()
    {
        m_texture.reset();
    }

    Texture::_Texture::_Texture()
    {
        glGenTextures(1, &id);
    }

    Texture::_Texture::~_Texture()
    {
        glDeleteTextures(1, &id);
    }

    void Texture2D::Bind()
    {
        BindTexture(GL_TEXTURE_2D, ID());
    }

    void Texture2D::Make(std::string_view filename)
    {
        // This only has to be done once but it just sets a variable
        stbi_set_flip_vertically_on_load(true);

        int width, height, nChannels;
        uint8_t* data = stbi_load(filename.data(), &width, &height, &nChannels, 0);

        int fmt =
            nChannels == 3 ? GL_RGB :
            nChannels == 4 ? GL_RGBA : 0;
        if (!fmt) throw std::exception("Problem loading image.");

        Make(width, height, (Pixel*)data, fmt);

        stbi_image_free(data);
    }

    void Texture2D::Make(GLsizei width, GLsizei height, Pixel* data, GLenum format, GLenum type)
    {
        m_texture = std::make_shared<_Texture>();
        Bind();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, format, type, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    }


    /*========================================================================*\
    |  VAO, VBO, etc...                                                        |
    \*========================================================================*/

    VAO::VAO()
    {
        glGenVertexArrays(1, &m_id);
    }

    VAO::~VAO()
    {
        Release();
    }

    VAO::VAO(VAO&& other) noexcept
        : m_id{ other.m_id }
    {
        other.m_id = 0;
    }

    VAO& VAO::operator=(VAO&& other) noexcept
    {
        if (this != &other)
        {
            Release();
            std::swap(m_id, other.m_id);
        }
        return *this;
    }

    void VAO::Bind()
    {
        glBindVertexArray(m_id);
    }

    void VAO::Release()
    {
        glDeleteVertexArrays(1, &m_id);
        m_id = 0;
    }

    BufferObject::BufferObject(GLenum target)
        : m_target{ target }
    {
        glGenBuffers(1, &m_id);
    }

    BufferObject::~BufferObject()
    {
        Release();
    }

    BufferObject::BufferObject(BufferObject&& other) noexcept
        : m_id{ other.m_id }
        , m_target{ other.m_target }
    {
        other.m_id = 0;
    }

    BufferObject& BufferObject::operator=(BufferObject&& other) noexcept
    {
        if (this != &other)
        {
            Release();
            m_target = other.m_target;
            std::swap(m_id, other.m_id);
        }
        return *this;
    }

    void BufferObject::Bind()
    {
        glBindBuffer(m_target, m_id);
    }

    void BufferObject::Release()
    {
        glDeleteBuffers(1, &m_id);
        m_id = 0;
    }

    VBO::VBO()
        : BufferObject(GL_ARRAY_BUFFER)
    {}

    EBO::EBO()
        : BufferObject(GL_ELEMENT_ARRAY_BUFFER)
    {}


    /*========================================================================*\
    |  GameObject                                                              |
    \*========================================================================*/

    void GameObject::Move(const vec3f& direction)
    {
        m_position += direction;
        remakeModel();
    }

    void GameObject::remakeModel()
    {
        mat4f translation = gl::translation(m_position);
        mat4f scale = gl::scale(m_scale);
        m_model = scale * translation;
    }

    template <typename GLtype>
    void GameObject::generateQuad(vec2f size, vec2f texBottomLeft, vec2f texTopRight,
        vec<5, GLfloat>* o_vertices, GLtype* o_indices, size_t offset)
    {
        float xPos = size.x() / 2.f;
        float yPos = size.y() / 2.f;
        float left = texBottomLeft.x();
        float bottom = texBottomLeft.y();
        float right = texTopRight.x();
        float top = texTopRight.y();

        o_vertices[4 * offset + 0] = { -xPos, yPos, 0.f, left, top };
        o_vertices[4 * offset + 1] = { -xPos, -yPos, 0.f, left, bottom };
        o_vertices[4 * offset + 2] = { xPos, -yPos, 0.f, right, bottom };
        o_vertices[4 * offset + 3] = { xPos, yPos, 0.f, right, top };

        o_indices[6 * offset + 0] = (GLtype)(4 * offset + 0);
        o_indices[6 * offset + 1] = (GLtype)(4 * offset + 1);
        o_indices[6 * offset + 2] = (GLtype)(4 * offset + 2);
        o_indices[6 * offset + 3] = (GLtype)(4 * offset + 0);
        o_indices[6 * offset + 4] = (GLtype)(4 * offset + 2);
        o_indices[6 * offset + 5] = (GLtype)(4 * offset + 3);
    }

    Sprite::Sprite(Texture2D texture, Shader shader, vec2f size, vec2f bottomLeft, vec2f topRight)
        : m_texture{ texture }, m_shader{ shader }
    {
        remakeModel();

        vec<5, GLfloat> vertices[4]{};
        GLubyte indices[6]{};
        generateQuad(size, bottomLeft, topRight, vertices, indices);

        m_vao.Bind();
        m_vbo.Bind();

        // This low-level GL code should probably be abstracted away

        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(4) * 5 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GL_FLOAT), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GL_FLOAT)));
        glEnableVertexAttribArray(1);

        m_ebo.Bind();
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLubyte), indices, GL_STATIC_DRAW);
    }

    void Sprite::Render()
    {
        m_texture.Bind();
        m_shader.Use();

        m_shader.SetUniform("u_Model", m_model);

        m_vao.Bind();
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0);
    }

    TextSprite::TextSprite(Texture2D texture, Shader shader, std::string_view text, vec2f letterSize)
        : GameObject(), m_texture{ texture }, m_shader{ shader }, m_len{ text.length() }
    {
        remakeModel();

        vec<5, GLfloat>* vertices = new vec<5, GLfloat>[4 * m_len]{};
        GLushort* indices = new GLushort[6 * m_len]{};

        constexpr vec2i trOffset{ pxGlyphSize.x(), 0 };
        constexpr vec2i blOffset{ 0, -pxGlyphSize.y() };

        for (size_t i = 0; i < m_len; ++i)
        {
            vec2i texCoords = charToTexCoords(text[i]);
            vec2f bottomLeft = scaleToSheetSize(texCoords + blOffset);
            vec2f topRight = scaleToSheetSize(texCoords + trOffset);

            generateQuad(letterSize, bottomLeft, topRight, vertices, indices, i);

            for (size_t j = 0; j < 4; ++j)
            { 
                // Move each subsequent letter over so they don't overlap
                vertices[4 * i + j] += (float)i * vec<5, GLfloat>{ letterSize.x(), 0.f, 0.f, 0.f, 0.f };
            }
        }

        m_vao.Bind();
        m_vbo.Bind();

        // This low-level GL code should probably be abstracted away

        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(m_len)* 4 * 5 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
        glEnableVertexAttribArray(1);

        m_ebo.Bind();
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(m_len) * 6 * sizeof(GLushort), indices, GL_STATIC_DRAW);

        delete[] vertices;
        delete[] indices;
    }

    vec2f TextSprite::scaleToSheetSize(vec2i in)
    {
        vec2f out = (vec2f)in;
        out.x() /= pxFontSheetSize.x();
        out.y() /= pxFontSheetSize.y();
        return out;
    }

    vec2i TextSprite::charToTexCoords(char c)
    {
        int ref = c - 32;
        int x = pxGlyphSize.x() * (ref % glyphsPerRow);
        int y = pxFontSheetSize.y() - pxGlyphSize.y() * (ref / glyphsPerRow);
        return { x, y };
    }

    void TextSprite::Render()
    {
        m_texture.Bind();
        m_shader.Use();

        m_shader.SetUniform("u_Model", m_model);

        m_vao.Bind();
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_len) * 6, GL_UNSIGNED_SHORT, 0);
    }
}
#endif //MOPE_ILLUSTRATOR_IMPL
#pragma pop_macro("near")
#pragma pop_macro("far")