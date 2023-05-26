#ifndef MOPE_ILLUSTRATOR_H
#define MOPE_ILLUSTRATOR_H

/*
    To use, #define MOPE_ILLUSTRATOR_IMPL before the #include in exactly one of
    your source files.
*/

#include <chrono>
#include <thread>
#include <mutex>
#include <string>
#include <string_view>
#include <vector>
#include <cassert>
#include <unordered_map>
#include <set>
#include <cstring>
#include <bitset>

#include "mope_vec/mope_vec.hxx"

#include <GL/GL.h>
#pragma comment(lib, "opengl32.lib")

// We define OpenGL junk and load up extensions (this replaces a more rigorous
// extension loader--such as GLEW--for my limited purposes)
typedef intptr_t GLintptr;
typedef uintptr_t GLsizeiptr;
typedef char GLchar;

#define GL_ARRAY_BUFFER						0x8892
#define GL_ELEMENT_ARRAY_BUFFER				0x8893
#define GL_SHADER_STORAGE_BUFFER            0x90D2
#define GL_STREAM_DRAW						0x88E0
#define GL_STATIC_DRAW						0x88E4
#define GL_DYNAMIC_DRAW                     0x88E8
#define GL_FRAGMENT_SHADER					0x8B30
#define GL_VERTEX_SHADER					0x8B31
#define	GL_COMPILE_STATUS					0x8B81
#define	GL_LINK_STATUS						0x8B82
#define GL_TEXTURE_3D						0x806F
#define GL_TEXTURE0							0x84C0
#define GL_RG							    0x8227
#define GL_CLAMP_TO_EDGE                    0x812F

#define GL_PROCS \
    GL_PROC(void,	glGenBuffers,				GLsizei n, GLuint* buffers) \
    GL_PROC(void,	glBindBuffer,				GLenum target, GLuint buffer) \
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
    GL_PROC(void,	glBufferData,				GLenum target, GLsizeiptr size, const void* data, GLenum usage) \
    GL_PROC(void,	glBufferSubData,			GLenum target, GLintptr offset, GLsizeiptr size, const void* data) \
    GL_PROC(void,	glVertexAttribPointer,		GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer) \
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
    GL_PROC(void,   glBindBufferBase,           GLenum target, GLuint index, GLuint buffer)

#if defined _WIN32

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#define getProcAddress wglGetProcAddress
#define GL_PROCS_EX GL_PROC(BOOL, wglSwapIntervalEXT, int interval)
#define GL_PROC(type, name, ...) \
    typedef type (WINAPI name##_t)(__VA_ARGS__); \
    extern name##_t* name;

// Support other platforms?
#endif

#if !defined getProcAddress or !defined GL_PROC
#error "Your platform is not supported!"
#elif defined GL_PROCS_EX
#define GL_PROC_LIST GL_PROCS GL_PROCS_EX
#else
#define GL_PROC_LIST GL_PROCS
#endif

GL_PROC_LIST
#undef GL_PROC
// whew!

/*============================================================================*\
|  Declarations                                                                |
\*============================================================================*/

namespace mope
{
    typedef vec4b Pixel;

    inline constexpr double pi = 3.14159265358979323846;
    inline constexpr float fPi = static_cast<float>(pi);
    inline constexpr double tau = 2.0 * pi;
    inline constexpr float fTau = static_cast<float>(tau);

    namespace settings
    {
        // If a shader storage buffer for a sprite fills up, how much bigger to make it
        inline constexpr float ssboResizeFactor = 2.f;
        static_assert(ssboResizeFactor >= 2.f);

        // Size information about the default fontsheet
        inline constexpr int glyphsPerRow = 16;
        inline constexpr int glyphRows = 6;
        inline constexpr vec2i pxGlyphSize = { 16, 32 };
        inline constexpr vec2i pxFontSheetSize = { pxGlyphSize.x() * glyphsPerRow, pxGlyphSize.y() * glyphRows };
        inline constexpr float glyphAspect = static_cast<float>(pxGlyphSize.x()) / pxGlyphSize.y();
    }

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
            const float nr,
            const float fr
        );
        mat4f perspective(
            const float fov,
            const float aspect,
            const float nr,
            const float fr
        );
    }

    /*========================================================================*\
    |  Shader                                                                  |
    \*========================================================================*/

    class Shader
    {
    public:
        GLuint Id();
        void Use();

        void Make(std::string_view srcVertex, std::string_view srcFragment);
        void Release();

        void SetUniform(const char* name, float value);
        void SetUniform(const char* name, int value);
        void SetUniform(const char* name, const vec2f& value);
        void SetUniform(const char* name, const mat2f& value, bool transpose = false);
        void SetUniform(const char* name, const mat3f& value, bool transpose = false);
        void SetUniform(const char* name, const mat4f& value, bool transpose = false);

    private:
        struct _Shader
        {
            ~_Shader();
            GLuint Id();
        private:
            GLuint id{ 0 };
        };

        std::shared_ptr<_Shader> m_shader = std::make_shared<_Shader>();

        GLuint compile(std::string_view srcShader, GLenum type);
        void checkErrors(GLuint object, bool isProgram = false);
    };

    /*========================================================================*\
    |  Texture                                                                 |
    \*========================================================================*/

    class Texture
    {
    public:
        virtual ~Texture() = default;

        virtual void Bind() = 0;

        GLuint Id();
        void Release();

    protected:
        struct _Texture
        {
            ~_Texture();
            GLuint Id();
        private:
            GLuint id{ 0 };
        };

        std::shared_ptr<_Texture> m_texture = std::make_shared<_Texture>();
    };

    class Texture2D : public Texture
    {
    public:
        ~Texture2D() = default;

        void Bind() override;
        void Make(std::string_view filename);
        void Make(GLsizei width, GLsizei height, Pixel* data);
        void Make(GLsizei width, GLsizei height, float* data);
        void Make(GLsizei width, GLsizei height, uint8_t* data);
        void Make(GLsizei width, GLsizei height, void* data, GLenum format, GLenum type);

        void UpdateData(Pixel* data, int x, int y, int width, int height);
    };

    /*========================================================================*\
    |  VAO, VBO, etc...                                                        |
    \*========================================================================*/

    class VAO
    {
    public:
        VAO() = default;
        ~VAO();
        VAO(const VAO&) = delete;
        VAO& operator=(const VAO&) = delete;
        VAO(VAO&&) noexcept;
        VAO& operator=(VAO&&) noexcept;

        void Generate();
        void Bind();
        void Release();

    private:
        GLuint m_id{ 0 };
    };

    class BufferObject
    {
    public:
        BufferObject() = default;
        ~BufferObject();
        BufferObject(const BufferObject&) = delete;
        BufferObject& operator=(const BufferObject&) = delete;
        BufferObject(BufferObject&&) noexcept;
        BufferObject& operator=(BufferObject&&) noexcept;

        void Generate();
        void Bind();
        void Release();
        void Allocate(size_t size, const void* data = nullptr);
        void Fill(size_t size, size_t offset, const void* data);

        int Id();

    private:
        GLuint m_id{ 0 };

        virtual GLenum target() = 0;
        virtual GLenum usage() = 0;
    };

    class VBO : public BufferObject
    {
    private:
        GLenum target() override;
        GLenum usage() override;
    };

    class EBO : public BufferObject
    {
    private:
        GLenum target() override;
        GLenum usage() override;
    };

    class SSBO : public BufferObject
    {
    private:
        GLenum target() override;
        GLenum usage() override;
    };

    /*========================================================================*\
    |  Camera                                                                  |
    \*========================================================================*/

    class Camera
    {
    public:
        Camera(Shader shader, std::string uniform);

        void MoveForward(const float delta);
        void Strafe(const float delta);
        void ChangeYaw(const float delta);
        void ChangePitch(const float delta);
        void Update();

    private:
        // The shader managed by this camera
        Shader m_shader;
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
    |  Sprite                                                                  |
    \*========================================================================*/

    class Instance
    {
    public:
        Instance(const vec3f& position, const vec3f& scale, float rotation);
        Instance(vec3f&& position, vec3f&& scale, float&& rotation);

        mat4f Model() const;
        vec3f Position() const;
        vec3f Scale() const;
        float Rotation() const;

        void MoveTo(vec3f position);
        void SetScale(vec3f factors);
        //void SetAngle(float);

        void Move(const vec3f& displacement);
        void ScaleBy(const vec3f& factors);
        //void Rotate(float);

    protected:
        vec3f m_position;
        vec3f m_scale;
        float m_rotation;
    };

    class Sprite
    {
    public:
        Sprite(
            Texture2D texture,
            Shader shader,
            vec2f leftBottom = { 0.f, 0.f },
            vec2f rightTop = { 1.f, 1.f }
        );

        void Render();
        void SetTextureCorners(vec2f leftBottom, vec2f rightTop);

    protected:
        vec2f m_leftBottom{ 0.f, 0.f };
        vec2f m_rightTop{ 1.f, 1.f };

        VAO m_vao{ };
        VBO m_vbo{ };
        EBO m_ebo{ };
        SSBO m_ssbo{ };

        Texture2D m_texture;
        Shader m_shader;

    private:
        virtual void render() = 0;
        virtual void prepare() = 0;

        bool m_prepared{ false };
    };

    class BasicSprite : public Sprite, public Instance
    {
    public:
        BasicSprite(
            Texture2D texture,
            Shader shader,
            vec2f leftBottom = { 0.f, 0.f },
            vec2f rightTop = { 1.f, 1.f },
            vec3f position = { 0.f, 0.f, -1.f },
            vec3f scale = { 1.f, 1.f, 1.f },
            float rotation = 0
        );

    private:
        void render() override;
        void prepare() override;
    };

    class InstancedSprite : public Sprite
    {
    public:
        using Sprite::Sprite;

        void AddInstance(std::shared_ptr<Instance> instance);
        std::shared_ptr<Instance> MakeInstance(
            vec3f position = vec3f{ 0.f, 0.f, -1.f },
            vec3f scale = vec3f{ 1.f, 1.f, 1.f },
            float rotation = 0
        );

    protected:
        void render() override;
        void prepare() override;

        std::vector<std::weak_ptr<Instance>> m_instances{ };
        size_t m_bufferSize{ 0 };
    };

    class AnimatedSprite : public BasicSprite
    {
    public:
        using BasicSprite::BasicSprite;

        void SetTexture(Texture2D tex, vec2f leftBottom, vec2f rightTop);
    };


    /*========================================================================*\
    |  Illustrator                                                             |
    \*========================================================================*/

    /*
    * Window is required to define the following API:
    */

    class BaseRenderer
    {
    public:
        // Display graphics
        virtual void showFrame() = 0;
    };

    class BaseWindow
    {
    public:
        // Retrieve an abstract object that represents control of a rendering context
        virtual std::unique_ptr<BaseRenderer> getRenderer() = 0;

        // Makes the given string the title of the window
        virtual void setTitle(std::string_view title) = 0;

        // get size of client area
        virtual int getWidth() = 0;
        virtual int getHeight() = 0;

        // get the mouse deltas since last call and reset
        virtual int retrieveXDelta() = 0;
        virtual int retrieveYDelta() = 0;

        // get a bitet representing currently pressed keys
        virtual std::bitset<256> getKeyStates() = 0;

        // return false to stop the app
        virtual bool running() = 0;

        // force window to start closing
        virtual void close() = 0;
    };

    class IllustratorCore
    {
    public:
        // start this puppy going
        void run();

    protected:
        IllustratorCore(std::unique_ptr<BaseWindow> window, std::string_view title);
        virtual ~IllustratorCore() = default;

        // Set OpengGl clear color using bytes in range [0-255] for RGBA
        void setClearColor(Pixel color);

        // Set OpengGl clear color using floats in range [0-1] for RGBA
        void setClearColor(vec4f color);

        // Set the projection matrix on the default shader
        void setProjection(const mat4f& projection_matrix);

        // Toggle whether to display the FPS in the title bar
        void toggleFps();

        // Get the mouse deltas from the last frame
        vec2i mouseDeltas();

        // Get the current dimensions of the screen client area
        vec2i clientDims();

        Shader defaultShader{ };

        // Internal representation of keystates
        std::bitset<256> m_pressed{ };
        std::bitset<256> m_released{ };
        std::bitset<256> m_held{ };

    private:
        // User should override this to do stuff to prepare for rendering.
        // Return false to abort starting the game.
        virtual bool gameStart();

        // User should override this to draw stuff or whatever.
        // Return false to end the rendering loop and begin closing.
        virtual bool gameUpdate(double deltaTime);

        // User should override this to release OpenGL resources before context is destroyed.
        // Generally not needed if RAII wrappers are used for resources.
        virtual void gameEnd();

        // The main driver of activity
        void coreLoop();

        // Updates during loop
        void updateSize(bool initial = false);
        void updateTitle();
        void updateInputs();

        // access to the window that we are working with
        std::unique_ptr<BaseWindow> m_window;

        // current usable width and height of window
        int m_width{ 0 };
        int m_height{ 0 };

        // once set, try to maintain this aspect ratio
        float m_initialAspect{ 0 };

        // x and y deltas of mouse on last frame
        int m_xDelta{ 0 };
        int m_yDelta{ 0 };

        // Timing/FPS mechanisms
        unsigned int m_frameCount{ 0 };
        double m_frameTime{ 0 };
        double m_fpsUpdateTimer{ 0 };
        bool m_showFps{ true };

        // title of the game/window, defined in constructor
        const std::string m_title;
    };

    /*
    *   KeyType must be statically castable to a uint8_t.
    */
    template <class KeyT>
    class Illustrator : public IllustratorCore
    {
    public:
        using IllustratorCore::IllustratorCore;

    protected:
        bool pressed(KeyT k)
        {
            return m_pressed[static_cast<uint8_t>(k)];
        }

        bool held(KeyT k)
        {
            return m_held[static_cast<uint8_t>(k)];
        }

        bool released(KeyT k)
        {
            return m_released[static_cast<uint8_t>(k)];
        }

    private:
        using IllustratorCore::m_pressed;
        using IllustratorCore::m_held;
        using IllustratorCore::m_released;
    };
}

#endif // MOPE_ILLUSTRATOR_H

#ifdef MOPE_ILLUSTRATOR_IMPL
#undef MOPE_ILLUSTRATOR_IMPL

/*============================================================================*\
|  Implementation                                                              |
\*============================================================================*/

#define GL_PROC(type, name, ...) static name##_t* name;
GL_PROC_LIST
#undef GL_PROC

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace mope::gl
{
    void BindProcs()
    {
#define GL_PROC(type, name, ...) name = (name##_t*)getProcAddress(#name);
        GL_PROC_LIST
#undef GL_PROC
    }

    void PrintErrors(const char* location)
    {
        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR) {
            fprintf(stdout, "Error arose at %s. Code: 0x%04x\n", location, err);
        }
    }

    constexpr mat4f translation(const vec3f& offsets)
    {
        mat4f res = mat4f::identity();
        for (size_t i = 0; i < 3; ++i) {
            res[3][i] = offsets[i];
        }
        return res;
    }

    constexpr mat4f scale(const vec3f& factors)
    {
        mat4f res = mat4f::identity();
        for (size_t i = 0; i < 3; ++i) {
            res[i][i] = factors[i];
        }
        return res;
    }

    constexpr mat4f ortho(
        const float left, const float right,
        const float bottom, const float top,
        const float nr, const float fr
    ) {
        const vec3f offsets{ -(right + left) / 2.f, -(top + bottom) / 2.f, -(fr + nr) / 2.f };
        const mat4f T = translation(offsets);

        const vec3f factors{ 2.f / (right - left), 2.f / (top - bottom), 2.f / (fr - nr) };
        const mat4f S = scale(factors);

        return S * T;
    }

    mat4f perspective(
        const float fov, const float aspect,
        const float nr, const float fr
    ) {
        float top = nr * tan(fov / 2.f);
        float right = aspect * top;

        return {
            nr / right, 0.f, 0.f, 0.f,
            0.f, nr / top, 0.f, 0.f,
            0.f, 0.f, (fr + nr) / (nr - fr), -1.f,
            0.f, 0.f, 2.f * fr * nr / (nr - fr), 0.f
        };
    }
}

namespace mope
{
    IllustratorCore::IllustratorCore(std::unique_ptr<BaseWindow> window, std::string_view title)
        : m_window{ std::move(window) }
        , m_title{ title }
    { }

    void IllustratorCore::run()
    {
        if (m_window->running())
        {
            coreLoop();
        }
    }

    void IllustratorCore::setClearColor(Pixel color)
    {
        constexpr float factor = 1.f / 255.f;
        setClearColor(factor * (vec4f)color);
    }

    void IllustratorCore::setClearColor(vec4f color)
    {
        glClearColor(color.x(), color.y(), color.z(), color.w());
    }

    void IllustratorCore::setProjection(const mat4f& projection_matrix)
    {
        defaultShader.SetUniform("u_Projection", projection_matrix);
    }

    void IllustratorCore::toggleFps()
    {
        m_showFps = !m_showFps;
        if (m_showFps) {
            updateTitle();
        }
        else {
            m_window->setTitle(m_title);
            m_frameCount = 0;
            m_fpsUpdateTimer = 0.0;
        }
    }

    void IllustratorCore::updateSize(bool initial)
    {
        // get current window size
        int new_width = m_window->getWidth();
        int new_height = m_window->getHeight();

        // make changes only if the window size has changed
        if (new_width != m_width || new_height != m_height) {
            m_width = new_width;
            m_height = new_height;

            // try to maintain original aspect ratio and keep the viewport centered
            float new_aspect = static_cast<float>(m_width) / m_height;
            if (initial)
            {
                m_initialAspect = new_aspect;
            }

            if (new_aspect > m_initialAspect)
            {
                int calc_width = static_cast<int>(m_height * m_initialAspect);
                int x = (m_width - calc_width) / 2;
                glViewport(x, 0, calc_width, m_height);
            }
            else if (new_aspect < m_initialAspect)
            {
                int calc_height = static_cast<int>(m_width / m_initialAspect);
                int y = (m_height - calc_height) / 2;
                glViewport(0, y, m_width, calc_height);
            }
            else
            {
                glViewport(0, 0, m_width, m_height);
            }
        }
    }

    void IllustratorCore::updateTitle()
    {
        if (m_showFps) {
            m_fpsUpdateTimer += m_frameTime;
            m_frameCount++;
            if (m_fpsUpdateTimer > 1.0) {
                m_window->setTitle(m_title + " FPS: " + std::to_string(m_frameCount));
                m_fpsUpdateTimer -= 1.0;
                m_frameCount = 0;
            }
        }
    }

    void IllustratorCore::updateInputs()
    {
        m_xDelta = m_window->retrieveXDelta();
        m_yDelta = m_window->retrieveYDelta();

        auto new_states = m_window->getKeyStates();
        m_pressed = new_states & ~m_held;
        m_released = m_held & ~new_states;
        m_held = new_states;
    }

    vec2i IllustratorCore::mouseDeltas()
    {
        return { m_xDelta, m_yDelta };
    }

    vec2i IllustratorCore::clientDims()
    {
        return { m_width, m_height };
    }

    bool IllustratorCore::gameStart()
    {
        return true;
    }

    bool IllustratorCore::gameUpdate(double)
    {
        // If this hasn't been overloaded, close immediately
        return false;
    }

    void IllustratorCore::gameEnd()
    { }

    void IllustratorCore::coreLoop()
    {
        auto rc = m_window->getRenderer();

        updateSize(true);

        gl::BindProcs();
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#if defined _WIN32
        wglSwapIntervalEXT(0);
#else
        // Support other platforms?
#endif

        defaultShader.Make(
            "#version 430 core\n"
            "uniform mat4 u_Projection;"
            "uniform mat4 u_View;"
            "uniform sampler2D u_Texture;"
            "layout (location = 0) in vec2 i_Vertex;"
            "layout (location = 1) in vec2 i_TexCoord;"
            "layout (std430, binding = 0) buffer MatrixBlock { mat4 u_Models[]; };"
            "out vec2 io_TexCoord;"
            "void main() {"
            "mat4 mvp = u_Projection * u_View * u_Models[gl_InstanceID];"
            "gl_Position = mvp * vec4(i_Vertex, 0.0, 1.0);"
            "io_TexCoord = i_TexCoord; }",

            "#version 430 core\n"
            "uniform sampler2D u_Texture;"
            "in vec2 io_TexCoord;"
            "out vec4 o_Color;"
            "void main() {"
            "o_Color = texture(u_Texture, io_TexCoord); }");
        defaultShader.Use();
        defaultShader.SetUniform("u_Projection", mat4f::identity());
        defaultShader.SetUniform("u_View", mat4f::identity());

        // Give the app a chance to set things up
        if (gameStart()) {
            std::chrono::system_clock::time_point t1 = std::chrono::system_clock::now();
            std::chrono::system_clock::time_point t2;

            while (m_window->running())
            {
                // Find out how much time has passsed
                t2 = std::chrono::system_clock::now();
                std::chrono::duration<double> durElapsed = t2 - t1;
                m_frameTime = durElapsed.count();
                t1 = t2;

                // Do all our regular updates
                updateTitle();
                updateSize();
                updateInputs();

                // show the frame
                glClear(GL_COLOR_BUFFER_BIT);
                gameUpdate(m_frameTime) ? rc->showFrame() : m_window->close();
            }
        }

        // Give the app a chance to clean up
        gameEnd();
    }


    /*========================================================================*\
    |  Camera                                                                  |
    \*========================================================================*/

    Camera::Camera(Shader shader, std::string uniform)
        : m_shader{ shader }
        , m_uniform{ uniform }
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
        while (m_yaw >= fTau) {
            m_yaw -= fTau;
        };
        while (m_yaw < 0.f) {
            m_yaw += fTau;
        };
    }

    void Camera::ChangePitch(float delta)
    {
        m_pitch += delta;

        constexpr float upperbound = 0.90f * fPi;
        constexpr float lowerbound = -upperbound;

        // clamp
        if (m_pitch > upperbound) {
            m_pitch = upperbound;
        }
        else if (m_pitch < lowerbound) {
            m_pitch = lowerbound;
        }
    }

    void Camera::Update()
    {
        // Update the transformation basis vectors
        basisZ = (vec3f{ sin(m_yaw) * cos(m_pitch), -sin(m_pitch), cos(m_yaw) * cos(m_pitch) }.unitf());
        basisX = (vec3f{ 0.f, 1.f, 0.f }.cross(basisZ).unitf());
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

    void Shader::Make(std::string_view srcVertex, std::string_view srcFragment)
    {
        GLuint shader_vert = compile(srcVertex, GL_VERTEX_SHADER);
        checkErrors(shader_vert);

        GLuint shader_frag = compile(srcFragment, GL_FRAGMENT_SHADER);
        checkErrors(shader_frag);

        GLuint id = Id();
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

    GLuint Shader::Id()
    {
        return m_shader->Id();
    }

    void Shader::Use()
    {
        glUseProgram(Id());
    }

    void Shader::SetUniform(const char* name, float value)
    {
        int loc = glGetUniformLocation(Id(), name);
        glUniform1f(loc, value);
    }

    void Shader::SetUniform(const char* name, int value)
    {
        int loc = glGetUniformLocation(Id(), name);
        glUniform1i(loc, value);
    }

    void Shader::SetUniform(const char* name, const vec2f& value)
    {
        int loc = glGetUniformLocation(Id(), name);
        glUniform2fv(loc, 1, value.elements);
    }

    void Shader::SetUniform(const char* name, const mat2f& value, bool transpose)
    {
        int loc = glGetUniformLocation(Id(), name);
        glUniformMatrix2fv(loc, 1, transpose, &value[0][0]);
    }

    void Shader::SetUniform(const char* name, const mat3f& value, bool transpose)
    {
        int loc = glGetUniformLocation(Id(), name);
        glUniformMatrix3fv(loc, 1, transpose, &value[0][0]);
    }

    void Shader::SetUniform(const char* name, const mat4f& value, bool transpose)
    {
        int loc = glGetUniformLocation(Id(), name);
        glUniformMatrix4fv(loc, 1, transpose, &value[0][0]);
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
            if (!success) {
                glGetShaderInfoLog(object, 512, NULL, log);
                std::cerr << "Error arose at shader compilation:\n" << log << std::endl;
            }
        }
        else {
            glGetProgramiv(object, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(object, 512, NULL, log);
                std::cerr << "Error arose at program linking:\n" << log << std::endl;
            }
        }
    }

    GLuint Shader::_Shader::Id()
    {
        if (!id) {
            id = glCreateProgram();
        }
        return id;
    }

    Shader::_Shader::~_Shader()
    {
        glDeleteProgram(id);
    }


    /*========================================================================*\
    |  Texture                                                                 |
    \*========================================================================*/

    GLuint Texture::Id()
    {
        return m_texture->Id();
    }

    void Texture::Release()
    {
        m_texture.reset();
    }

    GLuint Texture::_Texture::Id()
    {
        if (!id) {
            glGenTextures(1, &id);
        }
        return id;
    }

    Texture::_Texture::~_Texture()
    {
        glDeleteTextures(1, &id);
    }

    void Texture2D::Bind()
    {
        glBindTexture(GL_TEXTURE_2D, Id());
    }

    void Texture2D::Make(std::string_view filename)
    {
        // This only has to be done once but it just sets a variable
        stbi_set_flip_vertically_on_load(true);

        int width, height, nChannels;
        uint8_t* data = stbi_load(filename.data(), &width, &height, &nChannels, 0);

        int fmt =
            nChannels == 1 ? GL_RED :
            nChannels == 2 ? GL_RG :
            nChannels == 3 ? GL_RGB :
            nChannels == 4 ? GL_RGBA :
            (throw std::exception("Problem loading image."), 0);

        Make(width, height, (void*)data, fmt, GL_UNSIGNED_BYTE);

        stbi_image_free(data);
    }

    void Texture2D::Make(GLsizei width, GLsizei height, Pixel* data)
    {
        Make(width, height, (void*)data, GL_RGBA, GL_UNSIGNED_BYTE);
    }

    void Texture2D::Make(GLsizei width, GLsizei height, float* data)
    {
        Make(width, height, (void*)data, GL_RED, GL_FLOAT);
    }

    void Texture2D::Make(GLsizei width, GLsizei height, uint8_t* data)
    {
        Make(width, height, (void*)data, GL_RED, GL_UNSIGNED_BYTE);
    }

    void Texture2D::Make(GLsizei width, GLsizei height, void* data, GLenum format, GLenum type)
    {
        Bind();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, format, type, data);
        //glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }

    void Texture2D::UpdateData(Pixel* data, int x, int y, int width, int height)
    {
        Bind();
        glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, (void*)data);
    }


    /*========================================================================*\
    |  VAO, VBO, etc...                                                        |
    \*========================================================================*/

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
        if (this != &other) {
            Release();
            std::swap(m_id, other.m_id);
        }
        return *this;
    }

    void VAO::Generate()
    {
        glGenVertexArrays(1, &m_id);
    }

    void VAO::Bind()
    {
        if (!m_id) {
            Generate();
        }
        glBindVertexArray(m_id);
    }

    void VAO::Release()
    {
        glDeleteVertexArrays(1, &m_id);
        m_id = 0;
    }

    BufferObject::~BufferObject()
    {
        Release();
    }

    BufferObject::BufferObject(BufferObject&& other) noexcept
        : m_id{ other.m_id }
    {
        other.m_id = 0;
    }

    BufferObject& BufferObject::operator=(BufferObject&& other) noexcept
    {
        if (this != &other) {
            Release();
            std::swap(m_id, other.m_id);
        }
        return *this;
    }

    void BufferObject::Generate()
    {
        glGenBuffers(1, &m_id);
    }

    void BufferObject::Bind()
    {
        if (!m_id) {
            Generate();
        }
        glBindBuffer(target(), m_id);
    }

    void BufferObject::Release()
    {
        glDeleteBuffers(1, &m_id);
        m_id = 0;
    }

    void BufferObject::Allocate(size_t size, const void* data)
    {
        Bind();
        glBufferData(target(), size, data, usage());
    }

    void BufferObject::Fill(size_t size, size_t offset, const void* data)
    {
        Bind();
        glBufferSubData(target(), offset, size, data);
    }

    int BufferObject::Id() { return m_id; }

    GLenum VBO::target() { return GL_ARRAY_BUFFER; }
    GLenum VBO::usage() { return GL_STATIC_DRAW; }
    GLenum EBO::target() { return GL_ELEMENT_ARRAY_BUFFER; }
    GLenum EBO::usage() { return GL_STATIC_DRAW; }
    GLenum SSBO::target() { return GL_SHADER_STORAGE_BUFFER; }
    GLenum SSBO::usage() { return GL_DYNAMIC_DRAW; }


    /*========================================================================*\
    |  Sprite                                                                  |
    \*========================================================================*/

    Instance::Instance(vec3f&& position, vec3f&& scale, float&& rotation)
        : m_position{ std::move(position) }
        , m_scale{ std::move(scale) }
        , m_rotation{ std::move(rotation) }
    { }

    Instance::Instance(const vec3f& position, const vec3f& scale, float rotation)
        : Instance(vec3f{ position }, vec3f{ scale }, std::move(rotation))
    { }

    vec3f Instance::Position() const { return m_position; }
    vec3f Instance::Scale() const { return m_scale; }
    float Instance::Rotation() const { return m_rotation; }

    void Instance::MoveTo(vec3f position)
    {
        m_position = std::move(position);
    }

    void Instance::Move(const vec3f& direction)
    {
        m_position += direction;
    }

    void Instance::SetScale(vec3f factors)
    {
        m_scale = std::move(factors);
    }

    void Instance::ScaleBy(const vec3f& factors)
    {
        m_scale.scaleby(factors);
    }

    mat4f Instance::Model() const
    {
        mat4f translation = gl::translation(m_position);
        mat4f scale = gl::scale(m_scale);
        return translation * scale;
    }

    Sprite::Sprite(Texture2D texture, Shader shader, vec2f leftBottom, vec2f rightTop)
        : m_texture{ texture }
        , m_shader{ shader }
        , m_leftBottom{ leftBottom }
        , m_rightTop{ rightTop }
    { }

    void Sprite::Render()
    {
        if (!m_prepared) {
            prepare();
            m_prepared = true;
        }

        render();
    }

    void Sprite::SetTextureCorners(vec2f leftBottom, vec2f rightTop)
    {
        m_leftBottom = leftBottom;
        m_rightTop = rightTop;

        float left = m_leftBottom.x(),
            bottom = m_leftBottom.y(),
            right = m_rightTop.x(),
            top = m_rightTop.y();

        const GLfloat vertices[] = {
            -0.5f,  -0.5f,  left,   bottom,
            0.5f,   -0.5f,  right,  bottom,
            -0.5f,  0.5f,   left,   top,
            0.5f,   0.5f,   right,  top
        };

        m_vbo.Fill(sizeof(vertices), 0, vertices);
    }

    BasicSprite::BasicSprite(
        Texture2D texture,
        Shader shader,
        vec2f leftBottom,
        vec2f rightTop,
        vec3f position,
        vec3f scale,
        float rotation
    ) : Sprite{ texture, shader, leftBottom, rightTop }
        , Instance{ std::move(position), std::move(scale), std::move(rotation) }
    { }

    void BasicSprite::prepare()
    {
        float left = m_leftBottom.x(),
            bottom = m_leftBottom.y(),
            right = m_rightTop.x(),
            top = m_rightTop.y();
        const GLfloat vertices[] = {
            -0.5f,  -0.5f,  left,   bottom,
            0.5f,   -0.5f,  right,  bottom,
            -0.5f,  0.5f,   left,   top,
            0.5f,   0.5f,   right,  top
        };
        const GLubyte indices[4] = { 0, 1, 2, 3 };

        m_vao.Bind();
        m_vbo.Allocate(sizeof(vertices), vertices);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat[4]), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat[4]), (void*)(sizeof(GLfloat[2])));

        m_ebo.Allocate(sizeof(indices), indices);
        m_ssbo.Allocate(sizeof(mat4f));
    }

    void BasicSprite::render()
    {
        m_texture.Bind();
        m_shader.Use();
        m_vao.Bind();

        mat4f model = Model();
        m_ssbo.Fill(sizeof(model), 0, &model);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_ssbo.Id());
        glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, (void*)0);
    }

    void InstancedSprite::AddInstance(std::shared_ptr<Instance> instance)
    {
        m_instances.push_back(instance);
    }

    std::shared_ptr<Instance> InstancedSprite::MakeInstance(vec3f position, vec3f scale, float rotation)
    {
        std::shared_ptr<Instance> instance = std::make_shared<Instance>(position, scale, rotation);
        AddInstance(instance);
        return instance;
    }

    void InstancedSprite::prepare()
    {
        float left = m_leftBottom.x(),
            bottom = m_leftBottom.y(),
            right = m_rightTop.x(),
            top = m_rightTop.y();
        const GLfloat vertices[] = {
            -0.5f,  -0.5f,  left,   bottom,
            0.5f,   -0.5f,  right,  bottom,
            -0.5f,  0.5f,   left,   top,
            0.5f,   0.5f,   right,  top
        };
        const GLubyte indices[4] = { 0, 1, 2, 3 };

        m_vao.Bind();
        m_vbo.Allocate(sizeof(vertices), vertices);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat[4]), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat[4]), (void*)(sizeof(GLfloat[2])));

        m_ebo.Allocate(sizeof(indices), indices);
    }

    void InstancedSprite::render()
    {
        m_texture.Bind();
        m_shader.Use();
        m_vao.Bind();

        std::vector<mat4f> sub_data;

        // copy all model matrices into sub_data, at the same time removing expired instances
        auto prev_end{ m_instances.end() };
        auto new_end{ prev_end };
        for (auto iter = m_instances.begin(); iter != prev_end; ++iter)
        {
            if (auto p_instance = iter->lock())
            {
                sub_data.push_back(p_instance->Model());
                if (new_end != prev_end) {
                    *new_end++ = std::move(*iter);
                }
            }
            else if (new_end == prev_end) {
                new_end = iter;
            }
        }
        m_instances.erase(new_end, prev_end);

        // resize the shader storage buffer if necessary
        if (sub_data.size() > m_bufferSize)
        {
            do {
                m_bufferSize = m_bufferSize ? static_cast<size_t>(m_bufferSize * settings::ssboResizeFactor) : 1;
            } while (sub_data.size() > m_bufferSize);
            m_ssbo.Allocate(sizeof(mat4f) * m_bufferSize);
        }

        // fill the shader storage buffer and draw
        m_ssbo.Fill(sizeof(mat4f) * sub_data.size(), 0, sub_data.data());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_ssbo.Id());
        glDrawElementsInstanced(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, (void*)0, m_instances.size());
    }

    void AnimatedSprite::SetTexture(Texture2D tex, vec2f leftBottom, vec2f rightTop)
    {
        m_texture = tex;
        SetTextureCorners(leftBottom, rightTop);
    }
}

#endif //MOPE_ILLUSTRATOR_IMPL
