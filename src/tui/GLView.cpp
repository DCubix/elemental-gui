#include "GLView.h"

#include <iostream>
#include <string>

namespace tui {
    namespace gl {
        // FBO-related OpenGL functions
        static PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers = nullptr;
        static PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer = nullptr;
        static PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D = nullptr;
        static PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus = nullptr;
        static PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers = nullptr;
        static PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers = nullptr;
        static PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer = nullptr;
        static PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage = nullptr;
        static PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer = nullptr;
        static PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers = nullptr;

        // Debug cnotext related OpenGL functions
        static PFNGLDEBUGMESSAGECALLBACKPROC glDebugMessageCallback = nullptr;
        static PFNGLDEBUGMESSAGECONTROLPROC glDebugMessageControl = nullptr;
        
        static void LoadGLFunctions() {
            glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)SDL_GL_GetProcAddress("glGenFramebuffers");
            glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)SDL_GL_GetProcAddress("glBindFramebuffer");
            glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)SDL_GL_GetProcAddress("glFramebufferTexture2D");
            glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)SDL_GL_GetProcAddress("glCheckFramebufferStatus");
            glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)SDL_GL_GetProcAddress("glDeleteFramebuffers");
            glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC)SDL_GL_GetProcAddress("glGenRenderbuffers");
            glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC)SDL_GL_GetProcAddress("glBindRenderbuffer");
            glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC)SDL_GL_GetProcAddress("glRenderbufferStorage");
            glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)SDL_GL_GetProcAddress("glFramebufferRenderbuffer");
            glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC)SDL_GL_GetProcAddress("glDeleteRenderbuffers");

            glDebugMessageCallback = (PFNGLDEBUGMESSAGECALLBACKPROC)SDL_GL_GetProcAddress("glDebugMessageCallback");
            glDebugMessageControl = (PFNGLDEBUGMESSAGECONTROLPROC)SDL_GL_GetProcAddress("glDebugMessageControl");
        }
    }

    static void APIENTRY GLDebugMessageCallback(GLenum source, GLenum type, GLuint id,
                                     GLenum severity, GLsizei length,
                                     const GLchar* message, const void* userParam) {
        // Choose color based on severity
        if (severity == GL_DEBUG_SEVERITY_HIGH) {
            std::cout << utils::Format("[OpenGL Error] {}", message) << std::endl;
        }
        else if (severity == GL_DEBUG_SEVERITY_MEDIUM) {
            std::cout << utils::Format("[OpenGL Warning] {}", message) << std::endl;
        }
        else if (severity == GL_DEBUG_SEVERITY_LOW) {
            std::cout << utils::Format("[OpenGL Notice] {}", message) << std::endl;
        }
        else if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) {
            std::cout << utils::Format("[OpenGL Info] {}", message) << std::endl;
        }
        else {
            // Unknown severity, print normally
            std::cout << utils::Format("[OpenGL Debug] {}", message) << std::endl;
        }
    }

    GLView::GLView(GLContextConfig config)
    {
        GetLocalBounds().w = 100;
        GetLocalBounds().h = 100;

        config.Apply();

        std::string thisStr = std::to_string((uintptr_t)this);
        std::string title = "GLView@" + thisStr;
        m_dummyWindow = SDL_CreateWindow(
            title.c_str(),
            GetLocalBounds().w, GetLocalBounds().h,
            SDL_WINDOW_HIDDEN | SDL_WINDOW_OPENGL
        );
        m_glContext = SDL_GL_CreateContext(m_dummyWindow);

        gl::LoadGLFunctions();

        if (config.debug) {
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

            if (gl::glDebugMessageCallback && gl::glDebugMessageControl) {
                gl::glDebugMessageCallback(GLDebugMessageCallback, nullptr);
                gl::glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
                // Skip GL_DEBUG_SEVERITY_NOTIFICATION
                gl::glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
            }
            else {
                std::cerr << "Failed to set up OpenGL debug message callback" << std::endl;
            }
        }

        if (config.multiSample.enabled) {
            glEnable(GL_MULTISAMPLE);
        }

        m_screen = Image(GetLocalBounds().w, GetLocalBounds().h);

        // Setup FBO for offscreen rendering
        gl::glGenFramebuffers(1, &m_fbo);
        gl::glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        glGenTextures(1, &m_texture);
        glBindTexture(GL_TEXTURE_2D, m_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, GetLocalBounds().w, GetLocalBounds().h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        gl::glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0);
        gl::glGenRenderbuffers(1, &m_rbo);
        gl::glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
        gl::glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, GetLocalBounds().w, GetLocalBounds().h);
        gl::glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rbo);
        if (gl::glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "Failed to set up framebuffer for GLView" << std::endl;
        }
        gl::glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    GLView::~GLView()
    {
        if (m_glContext) {
            if (SDL_GL_GetCurrentContext() != m_glContext) {
                SDL_GL_MakeCurrent(m_dummyWindow, m_glContext);
            }
            gl::glDeleteFramebuffers(1, &m_fbo);
            glDeleteTextures(1, &m_texture);
            gl::glDeleteRenderbuffers(1, &m_rbo);
            SDL_GL_DestroyContext(m_glContext);
        }
        if (m_dummyWindow) {
            SDL_DestroyWindow(m_dummyWindow);
        }
    }

    void GLView::OnDraw(Graphics &g)
    {
        auto bounds = GetBounds();
        const int w = bounds.w;
        const int h = bounds.h;

        // Make the GL context current before drawing
        if (SDL_GL_GetCurrentContext() != m_glContext) {
            SDL_GL_MakeCurrent(m_dummyWindow, m_glContext);
        }

        // Mismatched screen size, resize the FBO and screen surface
        if (w != m_screen.GetWidth() || h != m_screen.GetHeight()) {
            m_screen.Resize(w, h);
            ResizeFBO(w, h);
        }

        // Render to the FBO
        gl::glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        glViewport(0, 0, w, h);
        OnRender();
        glFinish();

        // Read pixels from FBO
        std::vector<unsigned char> pixels(w * h * 4);
        glReadPixels(0, 0, w, h, GL_BGRA, GL_UNSIGNED_BYTE, pixels.data());
        gl::glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Flip the image vertically (OpenGL reads bottom-to-top)
        const int stride = w * 4;
        std::vector<unsigned char> row(stride);
        for (int y = 0; y < h / 2; y++) {
            unsigned char* top = pixels.data() + y * stride;
            unsigned char* bot = pixels.data() + (h - 1 - y) * stride;
            std::memcpy(row.data(), top, stride);
            std::memcpy(top, bot, stride);
            std::memcpy(bot, row.data(), stride);
        }

        // Update surface from pixel data and paint
        m_screen.SetPixels(pixels.data(), stride);

        g.DrawImage(&m_screen, bounds.x, bounds.y, bounds.w, bounds.h);
    }

    void GLView::ResizeFBO(int w, int h)
    {
        glBindTexture(GL_TEXTURE_2D, m_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        gl::glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
        gl::glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
    }

    void GLContextConfig::Apply() const
    {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, majorVersion);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, minorVersion);

        int profileMask = 0;
        switch (profile) {
            case Profile::Core:
                profileMask = SDL_GL_CONTEXT_PROFILE_CORE;
                break;
            case Profile::Compatibility:
                profileMask = SDL_GL_CONTEXT_PROFILE_COMPATIBILITY;
                break;
            case Profile::ES:
                profileMask = SDL_GL_CONTEXT_PROFILE_ES;
                break;
        }
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, profileMask);

        SDL_GL_SetAttribute(SDL_GL_RED_SIZE, bufferSizes.red);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, bufferSizes.green);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, bufferSizes.blue);
        SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, bufferSizes.alpha);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, bufferSizes.depth);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, bufferSizes.stencil);

        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, doubleBuffer ? 1 : 0);
        if (debug) {
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
        }

        if (multiSample.enabled) {
            SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
            SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, multiSample.samples);
        }
    }
}