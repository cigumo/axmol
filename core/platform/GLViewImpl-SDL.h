/****************************************************************************
Copyright (c) 2010-2012 cocos2d-x.org
Copyright (c) 2013-2016 Chukong Technologies Inc.
Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.

https://axmolengine.github.io/

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/

#pragma once
#include "platform/GL.h"
#include "base/Ref.h"
#include "platform/Common.h"
#include "platform/GLView.h"
#include <SDL.h>
#include <SDL_syswm.h>

NS_AX_BEGIN

//class GLFWEventHandler;
class AX_DLL GLViewImpl : public GLView
{
    //friend class GLFWEventHandler;

public:
    static GLViewImpl* create(std::string_view viewName);
    static GLViewImpl* create(std::string_view viewName, bool resizable);
    static GLViewImpl* createWithRect(std::string_view viewName,
                                      const Rect& rect,
                                      float frameZoomFactor = 1.0f,
                                      bool resizable        = false);
    static GLViewImpl* createWithFullScreen(std::string_view viewName);
    static GLViewImpl* createWithFullScreen(std::string_view viewName,
                                            const SDL_DisplayMode& displayMode,
                                            int monitorIndex);

    /*
     *frameZoomFactor for frame. This method is for debugging big resolution (e.g.new ipad) app on desktop.
     */

    // void resize(int width, int height);

    float getFrameZoomFactor() const override;
    // void centerWindow();

    virtual void setViewPortInPoints(float x, float y, float w, float h) override;
    virtual void setScissorInPoints(float x, float y, float w, float h) override;
    virtual Rect getScissorRect() const override;

    bool windowShouldClose() override;
    void pollEvents() override;
    SDL_Window* getWindow() const { return _mainWindow; }
    

    bool isFullscreen() const;

    /* Sets primary monitor full screen with default w*h(refresh rate) */
    void setFullscreen();
    /* Sets primary monitor full screen with w*h(refresh rate) */
    void setFullscreen(int w, int h, int refreshRate);

    /* Sets monitor full screen with default w*h(refresh rate) */
    void setFullscreen(int monitorIndex);
    /// <summary>
    /// Sets monitor full screen with w*h(refresh rate)
    /// </summary>
    /// <param name="monitorIndex">the 0 based index of monitor</param>
    /// <param name="w">the width of hardware resolution in full screen, -1 use default value</param>
    /// <param name="h">the height of hardware resolution in full screen, -1 use default value</param>
    /// <param name="refreshRate">the display refresh rate, usually 60, -1 use default value</param>
    void setFullscreen(int monitorIndex, int w, int h, int refreshRate);

    /* for internal use */
    //void setFullscreen(int monitorIndex, int w, int h, int refreshRate);
    void setWindowed(int width, int height);

    int getMonitorCount() const;
    Vec2 getMonitorSize() const;

    /* override functions */
    virtual bool isOpenGLReady() override;
    virtual void end() override;
    virtual void swapBuffers() override;
    virtual void setFrameSize(float width, float height) override;
    virtual void setIMEKeyboardState(bool bOpen) override;

#if AX_ICON_SET_SUPPORT
    virtual void setIcon(std::string_view filename) const override;
    virtual void setIcon(const std::vector<std::string_view>& filelist) const override;
    virtual void setDefaultIcon() const override;
#endif /* AX_ICON_SET_SUPPORT */

    /*
     * Set zoom factor for frame. This method is for debugging big resolution (e.g.new ipad) app on desktop.
     */
    void setFrameZoomFactor(float zoomFactor) override;
    /**
     * Hide or Show the mouse cursor if there is one.
     */
    virtual void setCursorVisible(bool isVisible) override;
    /** Retina support is disabled by default
     *  @note This method is only available on Mac.
     */
    void enableRetina(bool enabled);
    /** Check whether retina display is enabled. */
    bool isRetinaEnabled() const { return _isRetinaEnabled; };

    /** Get retina factor */
    int getRetinaFactor() const override { return _retinaFactor; }

#if (AX_TARGET_PLATFORM == AX_PLATFORM_WIN32)
    HWND getWin32Window() override;
#endif /* (AX_TARGET_PLATFORM == AX_PLATFORM_WIN32) */

#if (AX_TARGET_PLATFORM == AX_PLATFORM_MAC)
    void* getCocoaWindow() override;
    void* getNSGLContext() override;  // stevetranby: added
#endif                                // #if (AX_TARGET_PLATFORM == AX_PLATFORM_MAC)

protected:
    GLViewImpl(bool initsdl = true);
    virtual ~GLViewImpl();

    bool initWithRect(std::string_view viewName, const Rect& rect, float frameZoomFactor, bool resizable);
    bool initWithFullScreen(std::string_view viewName);
    bool initWithFullscreen(std::string_view viewname, const SDL_DisplayMode& displayMode, int monitorIndex);
#if (AX_TARGET_PLATFORM != AX_PLATFORM_MAC)  // Windows, Linux: use glad to loadGL
    bool loadGL();
#endif
    /* update frame layout when enter/exit full screen mode */
    void updateWindowSize();

    void updateFrameSize();

    //// GLFW callbacks
    //void onGLFWError(int errorID, const char* errorDesc);
    //void onGLFWMouseCallBack(GLFWwindow* window, int button, int action, int modify);
    //void onGLFWMouseMoveCallBack(GLFWwindow* window, double x, double y);
    //void onGLFWMouseScrollCallback(GLFWwindow* window, double x, double y);
    //void onGLFWKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    //void onGLFWCharCallback(GLFWwindow* window, unsigned int character);
    //void onGLFWWindowPosCallback(GLFWwindow* windows, int x, int y);
    //void onGLFWWindowSizeCallback(GLFWwindow* window, int width, int height);
    //void onGLFWWindowIconifyCallback(GLFWwindow* window, int iconified);
    //void onGLFWWindowFocusCallback(GLFWwindow* window, int focused);

    bool _isTouchDevice = false;
    bool _captured;
    bool _isInRetinaMonitor;
    bool _isRetinaEnabled;
    int _retinaFactor;  // Should be 1 or 2

    float _frameZoomFactor;

    SDL_Window* _mainWindow;
    //GLFWmonitor* _monitor;
    int _monitorIndex;  // fullscreen monitor in use

    SDL_SysWMinfo _mainWindowInfo;

    SDL_GLContext _glContext;
#if defined(AX_USE_METAL)
    SDL_MetalView _metalView;
#endif
    
    bool _quit = false;
    
    //std::string _glfwError;

    // SDL2 events
    void onMouseButtonEvent(SDL_MouseButtonEvent &event, bool isDown);
    void onMouseMotionEvent(SDL_MouseMotionEvent &event);
    void onMouseWheelEvent(SDL_MouseWheelEvent &event);
    void onKeyboardEvent(SDL_KeyboardEvent &event, bool isDown);
    void onTextInputEvent(SDL_Event &sdlEvent);
    void onDisplayEvent(SDL_DisplayEvent &event);
    void onWindowEvent(SDL_WindowEvent &event);
    void onWindowSizeChanged(int width, int height);

    float _mouseX;
    float _mouseY;

    
public:
    // View will trigger an event when window is resized, gains or loses focus
    static const std::string EVENT_WINDOW_RESIZED;
    static const std::string EVENT_WINDOW_FOCUSED;
    static const std::string EVENT_WINDOW_UNFOCUSED;

private:
    AX_DISALLOW_COPY_AND_ASSIGN(GLViewImpl);

};

NS_AX_END  // end of namespace   cocos2d
