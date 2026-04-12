#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>

#include "Element.h"

namespace tui {
    struct GLContextConfig {
        int majorVersion = 3;
        int minorVersion = 3;
        
        enum class Profile {
            Core,
            Compatibility,
            ES
        } profile{Profile::Core};

        struct {
            int red{8};
            int green{8};
            int blue{8};
            int alpha{8};
            int depth{24};
            int stencil{8};
        } bufferSizes;

        bool doubleBuffer{true};
        bool debug{false};
        bool vSync{true};

        struct {
            bool enabled{false};
            int samples{4};
        } multiSample;

        void Apply() const;
    };

    class GLView : public Element {
    public:
        GLView(GLContextConfig config = GLContextConfig());
        ~GLView();

        virtual void OnDraw(Graphics& g) override final;
        virtual void OnRender() = 0;
    private:
        SDL_GLContext m_glContext;
        SDL_Window* m_dummyWindow;
        Image m_screen;

        GLuint m_fbo{0}, m_texture{0}, m_rbo{0};

        void ResizeFBO(int w, int h);
    };
}