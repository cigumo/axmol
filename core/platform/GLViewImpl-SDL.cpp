/****************************************************************************
Copyright (c) 2010-2012 cocos2d-x.org
Copyright (c) 2013-2016 Chukong Technologies Inc.
Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
Copyright (c) 2020 C4games Ltd.
Copyright (c) 2021-2022 Bytedance Inc.
Copyright (c) 2023 Rusty Moyher. (https://rustymoyher.com)
Copyright (c) 2023 Ciro Mondueri (Kalio Ltda).

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

#include "platform/GLViewImpl-SDL.h"

#include <cmath>
#include <unordered_map>

#include "platform/Application.h"
#include "base/Director.h"
#include "base/Touch.h"
#include "base/EventDispatcher.h"
#include "base/EventKeyboard.h"
#include "base/EventMouse.h"
#include "base/IMEDispatcher.h"
#include "base/Utils.h"
#include "base/UTF8.h"
#include "2d/Camera.h"
#if AX_ICON_SET_SUPPORT
#    include "platform/Image.h"
#endif /* AX_ICON_SET_SUPPORT */

#include "renderer/Renderer.h"

#if defined(AX_USE_METAL)
#    include <Metal/Metal.h>
#    include "renderer/backend/metal/DeviceMTL.h"
#    include "renderer/backend/metal/UtilsMTL.h"
#else
#    include "renderer/backend/opengl/DeviceGL.h"
#    include "renderer/backend/opengl/DeviceInfoGL.h"
#    include "renderer/backend/opengl/MacrosGL.h"
#    include "renderer/backend/opengl/OpenGLState.h"
#endif  // #if (AX_TARGET_PLATFORM == AX_PLATFORM_MAC)

NS_AX_BEGIN

const std::string GLViewImpl::EVENT_WINDOW_RESIZED   = "glview_window_resized";
const std::string GLViewImpl::EVENT_WINDOW_FOCUSED   = "glview_window_focused";
const std::string GLViewImpl::EVENT_WINDOW_UNFOCUSED = "glview_window_unfocused";

////////////////////////////////////////////////////

struct keyCodeItem
{
    SDL_Keycode sdlKeyCode;
    EventKeyboard::KeyCode keyCode;
};

static std::unordered_map<SDL_Keycode, EventKeyboard::KeyCode> g_keyCodeMap;

static keyCodeItem g_keyCodeStructArray[] = {
    {SDLK_UNKNOWN,     EventKeyboard::KeyCode::KEY_NONE},
        
    {SDLK_RETURN,      EventKeyboard::KeyCode::KEY_RETURN},
    {SDLK_ESCAPE,      EventKeyboard::KeyCode::KEY_ESCAPE},
    {SDLK_BACKSPACE,   EventKeyboard::KeyCode::KEY_BACKSPACE},
    {SDLK_TAB,         EventKeyboard::KeyCode::KEY_TAB},
    {SDLK_SPACE,       EventKeyboard::KeyCode::KEY_SPACE},
    {SDLK_EXCLAIM,     EventKeyboard::KeyCode::KEY_EXCLAM},
    {SDLK_QUOTEDBL,    EventKeyboard::KeyCode::KEY_QUOTE}, // Maybe correct?
    {SDLK_HASH,        EventKeyboard::KeyCode::KEY_3},     // NOT CORRECT
    {SDLK_PERCENT,     EventKeyboard::KeyCode::KEY_PERCENT},
    {SDLK_DOLLAR,      EventKeyboard::KeyCode::KEY_DOLLAR},
    {SDLK_AMPERSAND,   EventKeyboard::KeyCode::KEY_AMPERSAND},
    {SDLK_QUOTE,       EventKeyboard::KeyCode::KEY_APOSTROPHE},
    {SDLK_LEFTPAREN,   EventKeyboard::KeyCode::KEY_LEFT_PARENTHESIS},
    {SDLK_RIGHTPAREN,  EventKeyboard::KeyCode::KEY_RIGHT_PARENTHESIS},
    {SDLK_ASTERISK,    EventKeyboard::KeyCode::KEY_ASTERISK},
    {SDLK_PLUS,        EventKeyboard::KeyCode::KEY_PLUS},
    {SDLK_COMMA,       EventKeyboard::KeyCode::KEY_COMMA},
    {SDLK_MINUS,       EventKeyboard::KeyCode::KEY_MINUS},
    {SDLK_PERIOD,      EventKeyboard::KeyCode::KEY_PERIOD},
    {SDLK_SLASH,       EventKeyboard::KeyCode::KEY_SLASH},
    {SDLK_0,           EventKeyboard::KeyCode::KEY_0},
    {SDLK_1,           EventKeyboard::KeyCode::KEY_1},
    {SDLK_2,           EventKeyboard::KeyCode::KEY_2},
    {SDLK_3,           EventKeyboard::KeyCode::KEY_3},
    {SDLK_4,           EventKeyboard::KeyCode::KEY_4},
    {SDLK_5,           EventKeyboard::KeyCode::KEY_5},
    {SDLK_6,           EventKeyboard::KeyCode::KEY_6},
    {SDLK_7,           EventKeyboard::KeyCode::KEY_7},
    {SDLK_8,           EventKeyboard::KeyCode::KEY_8},
    {SDLK_9,           EventKeyboard::KeyCode::KEY_9},

    {SDLK_COLON,       EventKeyboard::KeyCode::KEY_COLON},
    {SDLK_SEMICOLON,   EventKeyboard::KeyCode::KEY_SEMICOLON},
    {SDLK_LESS,        EventKeyboard::KeyCode::KEY_LESS_THAN},
    {SDLK_EQUALS,      EventKeyboard::KeyCode::KEY_EQUAL},
    {SDLK_GREATER,     EventKeyboard::KeyCode::KEY_GREATER_THAN},
    {SDLK_QUESTION,    EventKeyboard::KeyCode::KEY_QUESTION},
    {SDLK_AT,          EventKeyboard::KeyCode::KEY_AT},
    {SDLK_LEFTBRACKET, EventKeyboard::KeyCode::KEY_LEFT_BRACKET},
    {SDLK_BACKSLASH,   EventKeyboard::KeyCode::KEY_BACK_SLASH},
    {SDLK_RIGHTBRACKET,EventKeyboard::KeyCode::KEY_RIGHT_BRACKET},
    {SDLK_CARET,       EventKeyboard::KeyCode::KEY_6}, // NOT CORRECT
    {SDLK_UNDERSCORE,  EventKeyboard::KeyCode::KEY_UNDERSCORE},
    {SDLK_BACKQUOTE,   EventKeyboard::KeyCode::KEY_GRAVE},

    {SDLK_a,           EventKeyboard::KeyCode::KEY_A},
    {SDLK_b,           EventKeyboard::KeyCode::KEY_B},
    {SDLK_c,           EventKeyboard::KeyCode::KEY_C},
    {SDLK_d,           EventKeyboard::KeyCode::KEY_D},
    {SDLK_e,           EventKeyboard::KeyCode::KEY_E},
    {SDLK_f,           EventKeyboard::KeyCode::KEY_F},
    {SDLK_g,           EventKeyboard::KeyCode::KEY_G},
    {SDLK_h,           EventKeyboard::KeyCode::KEY_H},
    {SDLK_i,           EventKeyboard::KeyCode::KEY_I},
    {SDLK_j,           EventKeyboard::KeyCode::KEY_J},
    {SDLK_k,           EventKeyboard::KeyCode::KEY_K},
    {SDLK_l,           EventKeyboard::KeyCode::KEY_L},
    {SDLK_m,           EventKeyboard::KeyCode::KEY_M},
    {SDLK_n,           EventKeyboard::KeyCode::KEY_N},
    {SDLK_o,           EventKeyboard::KeyCode::KEY_O},
    {SDLK_p,           EventKeyboard::KeyCode::KEY_P},
    {SDLK_q,           EventKeyboard::KeyCode::KEY_Q},
    {SDLK_r,           EventKeyboard::KeyCode::KEY_R},
    {SDLK_s,           EventKeyboard::KeyCode::KEY_S},
    {SDLK_t,           EventKeyboard::KeyCode::KEY_T},
    {SDLK_u,           EventKeyboard::KeyCode::KEY_U},
    {SDLK_v,           EventKeyboard::KeyCode::KEY_V},
    {SDLK_w,           EventKeyboard::KeyCode::KEY_W},
    {SDLK_x,           EventKeyboard::KeyCode::KEY_X},
    {SDLK_y,           EventKeyboard::KeyCode::KEY_Y},
    {SDLK_z,           EventKeyboard::KeyCode::KEY_Z},

    {SDLK_CAPSLOCK,    EventKeyboard::KeyCode::KEY_CAPS_LOCK},

    {SDLK_F1,          EventKeyboard::KeyCode::KEY_F1},
    {SDLK_F2,          EventKeyboard::KeyCode::KEY_F2},
    {SDLK_F3,          EventKeyboard::KeyCode::KEY_F3},
    {SDLK_F4,          EventKeyboard::KeyCode::KEY_F4},
    {SDLK_F5,          EventKeyboard::KeyCode::KEY_F5},
    {SDLK_F6,          EventKeyboard::KeyCode::KEY_F6},
    {SDLK_F7,          EventKeyboard::KeyCode::KEY_F7},
    {SDLK_F8,          EventKeyboard::KeyCode::KEY_F8},
    {SDLK_F9,          EventKeyboard::KeyCode::KEY_F9},
    {SDLK_F10,         EventKeyboard::KeyCode::KEY_F10},
    {SDLK_F11,         EventKeyboard::KeyCode::KEY_F11},
    {SDLK_F12,         EventKeyboard::KeyCode::KEY_F12},

    {SDLK_PRINTSCREEN, EventKeyboard::KeyCode::KEY_PRINT},
    {SDLK_SCROLLLOCK,  EventKeyboard::KeyCode::KEY_SCROLL_LOCK},
    {SDLK_PAUSE,       EventKeyboard::KeyCode::KEY_PAUSE},
    {SDLK_INSERT,      EventKeyboard::KeyCode::KEY_INSERT},
    {SDLK_HOME,        EventKeyboard::KeyCode::KEY_HOME},
    {SDLK_PAGEUP,      EventKeyboard::KeyCode::KEY_PG_UP},
    {SDLK_DELETE,      EventKeyboard::KeyCode::KEY_DELETE},
    {SDLK_END,         EventKeyboard::KeyCode::KEY_END},
    {SDLK_PAGEDOWN,    EventKeyboard::KeyCode::KEY_PG_DOWN},
    {SDLK_RIGHT,       EventKeyboard::KeyCode::KEY_RIGHT_ARROW},
    {SDLK_LEFT,        EventKeyboard::KeyCode::KEY_LEFT_ARROW},
    {SDLK_DOWN,        EventKeyboard::KeyCode::KEY_DOWN_ARROW},
    {SDLK_UP,          EventKeyboard::KeyCode::KEY_UP_ARROW},

    {SDLK_NUMLOCKCLEAR,EventKeyboard::KeyCode::KEY_NUM_LOCK},
    {SDLK_KP_DIVIDE,   EventKeyboard::KeyCode::KEY_KP_DIVIDE},
    {SDLK_KP_MULTIPLY, EventKeyboard::KeyCode::KEY_KP_MULTIPLY},
    {SDLK_KP_MINUS,    EventKeyboard::KeyCode::KEY_KP_MINUS},
    {SDLK_KP_PLUS,     EventKeyboard::KeyCode::KEY_KP_PLUS},
    {SDLK_KP_ENTER,    EventKeyboard::KeyCode::KEY_KP_ENTER},
    {SDLK_KP_1,        EventKeyboard::KeyCode::KEY_KP_END},
    {SDLK_KP_2,        EventKeyboard::KeyCode::KEY_KP_DOWN},
    {SDLK_KP_3,        EventKeyboard::KeyCode::KEY_KP_PG_DOWN},
    {SDLK_KP_4,        EventKeyboard::KeyCode::KEY_KP_LEFT},
    {SDLK_KP_5,        EventKeyboard::KeyCode::KEY_KP_FIVE},
    {SDLK_KP_6,        EventKeyboard::KeyCode::KEY_KP_RIGHT},
    {SDLK_KP_7,        EventKeyboard::KeyCode::KEY_KP_HOME},
    {SDLK_KP_8,        EventKeyboard::KeyCode::KEY_KP_UP},
    {SDLK_KP_9,        EventKeyboard::KeyCode::KEY_KP_PG_UP},
    {SDLK_KP_0,        EventKeyboard::KeyCode::KEY_KP_INSERT},
    {SDLK_KP_PERIOD,   EventKeyboard::KeyCode::KEY_KP_DELETE},

    {SDLK_APPLICATION, EventKeyboard::KeyCode::KEY_MENU}, // Maybe correct on Mac?
    {SDLK_MENU,        EventKeyboard::KeyCode::KEY_MENU},

    {SDLK_LCTRL,       EventKeyboard::KeyCode::KEY_LEFT_CTRL},
    {SDLK_LSHIFT,      EventKeyboard::KeyCode::KEY_LEFT_SHIFT},
    {SDLK_LALT,        EventKeyboard::KeyCode::KEY_LEFT_ALT},
    {SDLK_LGUI,        EventKeyboard::KeyCode::KEY_MENU},
    {SDLK_RCTRL,       EventKeyboard::KeyCode::KEY_RIGHT_CTRL},
    {SDLK_RSHIFT,      EventKeyboard::KeyCode::KEY_RIGHT_SHIFT},
    {SDLK_RALT,        EventKeyboard::KeyCode::KEY_RIGHT_ALT},
    {SDLK_RGUI,        EventKeyboard::KeyCode::KEY_HYPER},
};
    
//////////////////////////////////////////////////////////////////////////
// implement GLViewImpl
//////////////////////////////////////////////////////////////////////////

GLViewImpl::GLViewImpl(bool initsdl)
    : _captured(false)
    , _isInRetinaMonitor(false)
    , _isRetinaEnabled(false)
    , _retinaFactor(1)
    , _frameZoomFactor(1.0f)
    , _mainWindow(nullptr)
    , _monitorIndex(-1)
    , _mouseX(0.0f)
    , _mouseY(0.0f)
{
    _viewName = "AXMOL20";
    g_keyCodeMap.clear();
    for (auto&& item : g_keyCodeStructArray)
    {
        g_keyCodeMap[item.sdlKeyCode] = item.keyCode;
    }

    if (initsdl) {
        if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0 ) {
            printf("SDL could not initialize Video! SDL_Error: %s\n", SDL_GetError());
        }
    }
    
}

GLViewImpl::~GLViewImpl()
{
    AXLOGINFO("deallocing GLViewImpl: %p", this);
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

#if (AX_TARGET_PLATFORM == AX_PLATFORM_WIN32)
HWND GLViewImpl::getWin32Window()
{
    //return glfwGetWin32Window(_mainWindow);
    return _mainWindowInfo.win.window;
}
#endif /* (AX_TARGET_PLATFORM == AX_PLATFORM_WIN32) */

#if (AX_TARGET_PLATFORM == AX_PLATFORM_MAC)
void* GLViewImpl::getCocoaWindow()
{
    //return (void*)glfwGetCocoaWindow(_mainWindow);
    return _mainWindowInfo.info.cocoa.window;
}
void* GLViewImpl::getNSGLContext()
{
    //return (void*)glfwGetNSGLContext(_mainWindow);
    return _glContext;
}
#endif  // #if (AX_TARGET_PLATFORM == AX_PLATFORM_MAC)

GLViewImpl* GLViewImpl::create(std::string_view viewName)
{
    return GLViewImpl::create(viewName, false);
}

GLViewImpl* GLViewImpl::create(std::string_view viewName, bool resizable)
{
    auto ret = new GLViewImpl;
    if (ret->initWithRect(viewName, ax::Rect(0, 0, 960, 640), 1.0f, resizable))
    {
        ret->autorelease();
        return ret;
    }
    AX_SAFE_DELETE(ret);
    return nullptr;
}

GLViewImpl* GLViewImpl::createWithRect(std::string_view viewName,
                                       const ax::Rect& rect,
                                       float frameZoomFactor,
                                       bool resizable)
{
    auto ret = new GLViewImpl;
    if (ret->initWithRect(viewName, rect, frameZoomFactor, resizable))
    {
        ret->autorelease();
        return ret;
    }
    AX_SAFE_DELETE(ret);
    return nullptr;
}

GLViewImpl* GLViewImpl::createWithFullScreen(std::string_view viewName)
{
    auto ret = new GLViewImpl();
    if (ret->initWithFullScreen(viewName))
    {
        ret->autorelease();
        return ret;
    }
    AX_SAFE_DELETE(ret);
    return nullptr;
}

GLViewImpl* GLViewImpl::createWithFullScreen(std::string_view viewName,
                                             const SDL_DisplayMode& displayMode,
                                             int monitorIndex)
{
    auto ret = new GLViewImpl();
    if (ret->initWithFullscreen(viewName, displayMode, monitorIndex))
    {
        ret->autorelease();
        return ret;
    }
    AX_SAFE_DELETE(ret);
    return nullptr;
}

bool GLViewImpl::initWithRect(std::string_view viewName, const ax::Rect& rect, float frameZoomFactor, bool resizable)
{
    setViewName(viewName);

    _frameZoomFactor = frameZoomFactor;

    Vec2 frameSize = rect.size;

#if AX_GLES_PROFILE
    //glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    //glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
    SDL_SetHint(SDL_HINT_OPENGL_ES_DRIVER, "1");
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_EGL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, AX_GLES_PROFILE / AX_GLES_PROFILE_DEN);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    
#elif defined(AX_USE_GL)
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // We don't want the old OpenGL
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

#elif defined(AX_USE_METAL)
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "metal");    
    
#endif

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, _glContextAttrs.redBits);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, _glContextAttrs.greenBits);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, _glContextAttrs.blueBits);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, _glContextAttrs.alphaBits);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, _glContextAttrs.depthBits);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, _glContextAttrs.stencilBits);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, _glContextAttrs.multisamplingCount);

    //glfwWindowHint(GLFW_VISIBLE, _glContextAttrs.visible);
    //glfwWindowHint(GLFW_DECORATED, _glContextAttrs.decorated);
    //glfwWindowHint(GLFW_RESIZABLE, resizable ? GL_TRUE : GL_FALSE);

    Uint32 flags = SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI;
    if (resizable)
        flags |= SDL_WINDOW_RESIZABLE;
    else
        flags |= SDL_WINDOW_FULLSCREEN;

#if defined(AX_USE_GL)
    flags |= SDL_WINDOW_OPENGL;
#elif defined(AX_USE_METAL)
    flags |= SDL_WINDOW_METAL;
#endif
    
    int neededWidth  = static_cast<int>(frameSize.width * _frameZoomFactor);
    int neededHeight = static_cast<int>(frameSize.height * _frameZoomFactor);

#if (AX_TARGET_PLATFORM == AX_PLATFORM_WIN32)
    // TODO
    //glfwxSetParent((HWND)_glContextAttrs.viewParent);
#endif

    _mainWindow = SDL_CreateWindow(std::string(viewName).c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, neededWidth, neededHeight, flags);
    if (!_mainWindow) {
        std::string message = "Can't create window";
        message.append("\nMore info: \n");
        message.append(SDL_GetError());
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        ccMessageBox(message.c_str(), "Error launch application");
        utils::killCurrentProcess();  // kill current process, don't cause crash when driver issue.
        return false;
    }

#if defined(AX_USE_GL)
    //TODO glfwSetWindowUserPointer(_mainWindow, backend::__gl);
    _glContext = SDL_GL_CreateContext(_mainWindow);
    if (_glContext == NULL) {
        printf("OpenGL context could not be created! SDL Error: %s\n", SDL_GetError());
        return false;
    }

#elif defined(AX_USE_METAL)
    int drW = 0, drH = 0;
    SDL_Metal_GetDrawableSize(_mainWindow, &drW, &drH);
    
    _metalView = SDL_Metal_CreateView(_mainWindow);
    if (_metalView == NULL) {
        printf("Metal view could not be creaed! SDL Error: %s\n", SDL_GetError());
        return false;
    }

    CGSize size;
    size.width  = static_cast<CGFloat>(drW);
    size.height = static_cast<CGFloat>(drH);

    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    if (!device) {
        AXLOG("Doesn't support metal.");
        return false;
    }        
    CAMetalLayer* layer = (CAMetalLayer*) SDL_Metal_GetLayer(_metalView);
    [layer setDevice:device];
    [layer setPixelFormat:MTLPixelFormatBGRA8Unorm];
    [layer setFramebufferOnly:YES];
    [layer setDrawableSize:size];
    layer.displaySyncEnabled = _glContextAttrs.vsync;
    backend::DeviceMTL::setCAMetalLayer(layer);
#endif

    if (!SDL_GetWindowWMInfo(_mainWindow, &_mainWindowInfo)) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Couldn't get window information: %s", SDL_GetError());
    }
    
    int realW = 0, realH = 0;
    SDL_GetWindowSize(_mainWindow, &realW, &realH);
    if (realW != neededWidth)
    {
        frameSize.width = realW / _frameZoomFactor;
    }
    if (realH != neededHeight)
    {
        frameSize.height = realH / _frameZoomFactor;
    }

    setFrameSize(frameSize.width, frameSize.height);

#if (AX_TARGET_PLATFORM != AX_PLATFORM_MAC)
    loadGL();

    // Init device after load GL
    backend::Device::getInstance();
#endif

#if defined(AX_USE_GL)
    if (SDL_GL_SetSwapInterval(_glContextAttrs.vsync ? 1 : 0) < 0) {
        printf("Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());
        return false;
    }
    
    // Will cause OpenGL error 0x0500 when use ANGLE-GLES on desktop
#    if !AX_GLES_PROFILE
    // Enable point size by default.
#        if defined(GL_VERSION_2_0)
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
#        else
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE_ARB);
#        endif
    if (_glContextAttrs.multisamplingCount > 0)
        glEnable(GL_MULTISAMPLE);
#    endif
    CHECK_GL_ERROR_DEBUG();

#endif

    return true;
}

bool GLViewImpl::initWithFullScreen(std::string_view viewName)
{
    // Create fullscreen window on primary monitor at its current video mode.
    SDL_DisplayMode displayMode;
    if (SDL_GetDesktopDisplayMode(0, &displayMode) < 0) {
        log("GLViewImpl-SDL: initWithFullScreen failed");
        return false;
    }    
    return initWithRect(viewName, ax::Rect(0, 0, displayMode.w, displayMode.h), 1.0f, false);
}

bool GLViewImpl::initWithFullscreen(std::string_view viewname, const SDL_DisplayMode& displayMode, int monitorIndex)
{
    // Create fullscreen on specified monitor at the specified video mode.
    if (monitorIndex < 0 || monitorIndex >= SDL_GetNumVideoDisplays()) {
        log("GLViewImpl-SDL: invalid monitor %i", monitorIndex);
        return false;
    }

    return initWithRect(viewname, ax::Rect(0, 0, displayMode.w, displayMode.h), 1.0f, false);
}

bool GLViewImpl::isOpenGLReady()
{
    return nullptr != _mainWindow;
}

void GLViewImpl::end()
{
    if (_glContext) {
        SDL_GL_DeleteContext(_glContext);
        _glContext = NULL;
    }

    if (_metalView) {
        SDL_Metal_DestroyView(_metalView);
        _metalView = NULL;
    }
    
    if (_mainWindow)
    {
        SDL_DestroyWindow(_mainWindow);
        _mainWindow = nullptr;
    }
    // Release self. Otherwise, GLViewImpl could not be freed.
    release();
}

void GLViewImpl::swapBuffers()
{
#if defined(AX_USE_GL)
    if (_mainWindow)
        SDL_GL_SwapWindow(_mainWindow);
#endif
}

bool GLViewImpl::windowShouldClose()
{
    return _quit;
}

void GLViewImpl::pollEvents()
{
    SDL_Event sdlEvent;
    
    while (SDL_PollEvent(&sdlEvent)) {
        switch (sdlEvent.type) {
            case SDL_QUIT:                      _quit = true; break;
                
            case SDL_APP_TERMINATING:           log("event SDL_APP_TERMINATING"); break;
            case SDL_APP_LOWMEMORY:             log("event SDL_APP_LOWMEMORY"); break;
            case SDL_APP_WILLENTERBACKGROUND:   log("event SDL_APP_WILLENTERBACKGROUND"); break;
            case SDL_APP_DIDENTERBACKGROUND:    log("event SDL_APP_DIDENTERBACKGROUND"); break;
            case SDL_APP_WILLENTERFOREGROUND:   log("event SDL_APP_WILLENTERFOREGROUND"); break;
            case SDL_APP_DIDENTERFOREGROUND:    log("event SDL_APP_DIDENTERFOREGROUND"); break;
                
            case SDL_DISPLAYEVENT:      onDisplayEvent(sdlEvent.display); break;
            case SDL_WINDOWEVENT:       onWindowEvent(sdlEvent.window); break;
            //case SDL_SYSWMEVENT:        log("event SDL_SYSWMEVENT"); break;
                
            case SDL_MOUSEMOTION:       onMouseMotionEvent(sdlEvent.motion); break;
            case SDL_MOUSEBUTTONDOWN:   onMouseButtonEvent(sdlEvent.button, true); break;
            case SDL_MOUSEBUTTONUP:     onMouseButtonEvent(sdlEvent.button, false); break;
            case SDL_MOUSEWHEEL:        onMouseWheelEvent(sdlEvent.wheel); break;
                
            case SDL_KEYDOWN:           onKeyboardEvent(sdlEvent.key, true); break;
            case SDL_KEYUP:             onKeyboardEvent(sdlEvent.key, false); break;
            case SDL_TEXTINPUT:         onTextInputEvent(sdlEvent); break;

            // TODO
            //case SDL_CONTROLLERAXISMOTION:      controllerAxisEventArray.push_back(sdlEvent.caxis); break;
            //case SDL_CONTROLLERBUTTONDOWN:      controllerButtonEventArray.push_back(sdlEvent.cbutton); break;
            //case SDL_CONTROLLERBUTTONUP:        controllerButtonEventArray.push_back(sdlEvent.cbutton); break;
            //case SDL_CONTROLLERDEVICEADDED:     controllerDeviceEventArray.push_back(sdlEvent.cdevice); break;
            //case SDL_CONTROLLERDEVICEREMOVED:   controllerDeviceEventArray.push_back(sdlEvent.cdevice); break;
            
            case SDL_RENDER_TARGETS_RESET:      log("event SDL_RENDER_TARGETS_RESET");break;
            case SDL_RENDER_DEVICE_RESET:       log("event SDL_RENDER_DEVICE_RESET"); break;
        }
    }    
}

void GLViewImpl::enableRetina(bool enabled)
{
#if (AX_TARGET_PLATFORM == AX_PLATFORM_MAC)
    _isRetinaEnabled = enabled;
    if (_isRetinaEnabled)
    {
        _retinaFactor = 1;
    }
    else
    {
        _retinaFactor = 2;
    }
    updateFrameSize();
#endif
}

void GLViewImpl::setIMEKeyboardState(bool /*bOpen*/) {}

#if AX_ICON_SET_SUPPORT
void GLViewImpl::setIcon(std::string_view filename) const
{
    this->setIcon(std::vector<std::string_view>{filename});
}

void GLViewImpl::setIcon(const std::vector<std::string_view>& filelist) const
{
    if (filelist.empty())
        return;
    std::vector<Image*> icons;
    for (auto const& filename : filelist)
    {
        Image* icon = new Image();
        if (icon->initWithImageFile(filename))
        {
            icons.emplace_back(icon);
        }
        else
        {
            AX_SAFE_DELETE(icon);
        }
    }

    if (icons.empty())
        return;  // No valid images

    //size_t iconsCount = icons.size();
    //auto images       = new GLFWimage[iconsCount];
    //for (size_t i = 0; i < iconsCount; i++)
    //{
    //    auto& image  = images[i];
    //    auto& icon   = icons[i];
    //    image.width  = icon->getWidth();
    //    image.height = icon->getHeight();
    //    image.pixels = icon->getData();
    //};
    //GLFWwindow* window = this->getWindow();
    //glfwSetWindowIcon(window, iconsCount, images);

    // TODO: just using the first one now
    int pitch = icons[0]->getWidth() * icons[0]->getBitPerPixel();

    // TODO: this mask depends on endiannes in Image?
    uint32_t rmask = 0x000000FF;
    uint32_t gmask = 0x0000FF00;
    uint32_t bmask = 0x00FF0000;
    uint32_t amask = 0xFF000000;    
    
    SDL_Surface* image = SDL_CreateRGBSurfaceFrom(icons[0]->getData(),
                                                  icons[0]->getWidth(), icons[0]->getHeight(),
                                                  icons[0]->getBitPerPixel(),
                                                  pitch,
                                                  rmask,gmask,bmask,amask);
    SDL_SetWindowIcon(_mainWindow, image);
    SDL_FreeSurface(image);

    AX_SAFE_DELETE_ARRAY(images);
    for (auto&& icon : icons)
    {
        AX_SAFE_DELETE(icon);
    }
}

void GLViewImpl::setDefaultIcon() const
{
    //GLFWwindow* window = this->getWindow();
    //glfwSetWindowIcon(window, 0, nullptr);

    // TODO SDL_SetWindowIcon(_mainWindow, ??)
}
#endif /* AX_ICON_SET_SUPPORT */

void GLViewImpl::setCursorVisible(bool isVisible)
{
    if (_mainWindow == NULL)
        return;

    SDL_ShowCursor(isVisible ? SDL_ENABLE : SDL_DISABLE);
}

void GLViewImpl::setFrameZoomFactor(float zoomFactor)
{
    AXASSERT(zoomFactor > 0.0f, "zoomFactor must be larger than 0");

    if (std::abs(_frameZoomFactor - zoomFactor) < FLT_EPSILON)
    {
        return;
    }

    _frameZoomFactor = zoomFactor;
    updateFrameSize();
}

float GLViewImpl::getFrameZoomFactor() const
{
    return _frameZoomFactor;
}

bool GLViewImpl::isFullscreen() const
{
    return (_monitorIndex >= 0);
}

void GLViewImpl::setFullscreen()
{
    setFullscreen(-1, -1, -1);
}

void GLViewImpl::setFullscreen(int w, int h, int refreshRate)
{
    this->setFullscreen(0, w, h, refreshRate);
}

void GLViewImpl::setFullscreen(int monitorIndex)
{
    setFullscreen(monitorIndex, -1, -1, -1);
}

void GLViewImpl::setFullscreen(int monitorIndex, int w, int h, int refreshRate)
{
    SDL_DisplayMode displayMode;
    if (SDL_GetDesktopDisplayMode(_monitorIndex, &displayMode) < 0) {
        log("GLViewImpl-SDL: setFullscreen failed for monitor %i", monitorIndex);
        return false;
    }    
    if (w != -1)
        displayMode.w = w;
    if (h != -1)
        displayMode.h = h;
    if (refreshRate != -1)
        displayMode.refresh_rate = refreshRate;
    
    if (SDL_SetWindowDisplayMode(_mainWindow, &displayMode) != 0) {
        log("setFullscreen - SDL_SetWindowDisplayMode failed: %s.", SDL_GetError());
    }
    if (_monitorIndex < 0) {
        if (monitorIndex >= 0) {
            SDL_SetWindowPosition(_mainWindow, SDL_WINDOWPOS_CENTERED_DISPLAY(monitorIndex), SDL_WINDOWPOS_CENTERED_DISPLAY(monitorIndex));
        }
        if (SDL_SetWindowFullscreen(_mainWindow, SDL_WINDOW_FULLSCREEN) != 0) {
            log("GLViewImpl-SDL:setFullscreen failed - SDL_SetWindowFullscreen failed: %s.", SDL_GetError());
        }
        _monitorIndex = monitorIndex;
    }

    updateWindowSize();
}

void GLViewImpl::setWindowed(int width, int height)
{
    if (this->isFullscreen()) {
        SDL_SetWindowFullscreen(_mainWindow, 0);
        _monitorIndex = -1;
    }

    this->setFrameSize(width,height);

    int i = SDL_GetWindowDisplayIndex(_mainWindow);
    if (i >= 0) {
        SDL_SetWindowPosition(_mainWindow, SDL_WINDOWPOS_CENTERED_DISPLAY(i), SDL_WINDOWPOS_CENTERED_DISPLAY(i));
    }
    
    updateWindowSize();
}

void GLViewImpl::updateWindowSize()
{
    int w = 0, h = 0;
    SDL_GetWindowSize(_mainWindow, &w, &h);    
    int frameWidth  = w / _frameZoomFactor;
    int frameHeight = h / _frameZoomFactor;
    setFrameSize(frameWidth, frameHeight);
    updateDesignResolutionSize();
    Director::getInstance()->getEventDispatcher()->dispatchCustomEvent(GLViewImpl::EVENT_WINDOW_RESIZED, nullptr);
}

int GLViewImpl::getMonitorCount() const
{
    return SDL_GetNumVideoDisplays();
}

Vec2 GLViewImpl::getMonitorSize() const
{
    int displayIndex = SDL_GetWindowDisplayIndex(_mainWindow);
    if (displayIndex >= 0) {
        SDL_DisplayMode mode;
        if (SDL_GetCurrentDisplayMode(displayIndex, &mode) >= 0) {
            return Size(mode.w, mode.h);
        }
    }
    return Size::ZERO;
}

void GLViewImpl::updateFrameSize()
{
    if (_screenSize.width <= 0 || _screenSize.height <= 0) return;
    
    int w = 0;
    int h = 0;
    SDL_GetWindowSize(_mainWindow, &w, &h);

    int frameBufferW = 0;
    int frameBufferH = 0;
    SDL_GetWindowSizeInPixels(_mainWindow, &frameBufferW, &frameBufferH);

    if (frameBufferW == 2 * w && frameBufferH == 2 * h) {
        if (_isRetinaEnabled) {
            _retinaFactor = 1;
        } else {
            _retinaFactor = 2;
        }
        SDL_SetWindowSize(_mainWindow, _screenSize.width / 2 * _retinaFactor * _frameZoomFactor,
                          _screenSize.height / 2 * _retinaFactor * _frameZoomFactor);
        _isInRetinaMonitor = true;
    } else {
        if (_isInRetinaMonitor) {
            _retinaFactor = 1;
        }
        SDL_SetWindowSize(_mainWindow, (int)(_screenSize.width * _retinaFactor * _frameZoomFactor),
                          (int)(_screenSize.height * _retinaFactor * _frameZoomFactor));        
        _isInRetinaMonitor = false;
    }
}

void GLViewImpl::setFrameSize(float width, float height)
{
    GLView::setFrameSize(width, height);
    updateFrameSize();
}

void GLViewImpl::setViewPortInPoints(float x, float y, float w, float h)
{
    Viewport vp;
    vp.x = (int)(x * _scaleX * _retinaFactor * _frameZoomFactor +
                 _viewPortRect.origin.x * _retinaFactor * _frameZoomFactor);
    vp.y = (int)(y * _scaleY * _retinaFactor * _frameZoomFactor +
                 _viewPortRect.origin.y * _retinaFactor * _frameZoomFactor);
    vp.w = (unsigned int)(w * _scaleX * _retinaFactor * _frameZoomFactor);
    vp.h = (unsigned int)(h * _scaleY * _retinaFactor * _frameZoomFactor);
    Camera::setDefaultViewport(vp);
}

void GLViewImpl::setScissorInPoints(float x, float y, float w, float h)
{
    auto x1       = (int)(x * _scaleX * _retinaFactor * _frameZoomFactor +
                    _viewPortRect.origin.x * _retinaFactor * _frameZoomFactor);
    auto y1       = (int)(y * _scaleY * _retinaFactor * _frameZoomFactor +
                    _viewPortRect.origin.y * _retinaFactor * _frameZoomFactor);
    auto width1   = (unsigned int)(w * _scaleX * _retinaFactor * _frameZoomFactor);
    auto height1  = (unsigned int)(h * _scaleY * _retinaFactor * _frameZoomFactor);
    auto renderer = Director::getInstance()->getRenderer();
    renderer->setScissorRect(x1, y1, width1, height1);
}

ax::Rect GLViewImpl::getScissorRect() const
{
    auto renderer = Director::getInstance()->getRenderer();
    auto& rect    = renderer->getScissorRect();

    float x = (rect.x - _viewPortRect.origin.x * _retinaFactor * _frameZoomFactor) /
              (_scaleX * _retinaFactor * _frameZoomFactor);
    float y = (rect.y - _viewPortRect.origin.y * _retinaFactor * _frameZoomFactor) /
              (_scaleY * _retinaFactor * _frameZoomFactor);
    float w = rect.width / (_scaleX * _retinaFactor * _frameZoomFactor);
    float h = rect.height / (_scaleY * _retinaFactor * _frameZoomFactor);
    return ax::Rect(x, y, w, h);
}

// SDL EVENTS

void GLViewImpl::onMouseButtonEvent(SDL_MouseButtonEvent &event, bool isDown)
{
    if (!_isTouchDevice)
    {
        if (event.button == SDL_BUTTON_LEFT)
        {
            if (event.state == SDL_PRESSED)
            {
                _captured = true;
                if (this->getViewPortRect().equals(ax::Rect::ZERO) ||
                    this->getViewPortRect().containsPoint(Vec2(_mouseX, _mouseY)))
                {
                    intptr_t id = 0;
                    this->handleTouchesBegin(1, &id, &_mouseX, &_mouseY);
                }
            }
            else if (event.state == SDL_RELEASED)
            {
                if (_captured)
                {
                    _captured   = false;
                    intptr_t id = 0;
                    this->handleTouchesEnd(1, &id, &_mouseX, &_mouseY);
                }
            }
        }
    }
    
    // Because OpenGL and cocos2d-x uses different Y axis, we need to convert the coordinate here
    float cursorX = (event.x - _viewPortRect.origin.x) / _scaleX;
    float cursorY = (_viewPortRect.origin.y + _viewPortRect.size.height - event.y) / _scaleY;
    
    EventMouse::MouseButton eventButton;
    if (event.button == SDL_BUTTON_LEFT) {
        eventButton = EventMouse::MouseButton::BUTTON_LEFT;
    } else if (event.button == SDL_BUTTON_RIGHT) {
        eventButton = EventMouse::MouseButton::BUTTON_RIGHT;
    } else if (event.button == SDL_BUTTON_MIDDLE) {
        eventButton = EventMouse::MouseButton::BUTTON_MIDDLE;
    } else if (event.button == SDL_BUTTON_X1) {
        eventButton = EventMouse::MouseButton::BUTTON_4;
    } else if (event.button == SDL_BUTTON_X2) {
        eventButton = EventMouse::MouseButton::BUTTON_5;
    } else {
        // Ignore buttons above 5
        //log("GLViewImpl::onMouseButtonEvent - unknown button %i", event.button);
        return;
    }
    
    if (event.state == SDL_PRESSED) {
        EventMouse eventMouse(EventMouse::MouseEventType::MOUSE_DOWN);
        eventMouse.setCursorPosition(cursorX, cursorY);
        eventMouse.setMouseButton(eventButton);
        Director::getInstance()->getEventDispatcher()->dispatchEvent(&eventMouse);
    } else if (event.state == SDL_RELEASED) {
        EventMouse eventMouse(EventMouse::MouseEventType::MOUSE_UP);
        eventMouse.setCursorPosition(cursorX, cursorY);
        eventMouse.setMouseButton(eventButton);
        Director::getInstance()->getEventDispatcher()->dispatchEvent(&eventMouse);
    }
}

void GLViewImpl::onMouseMotionEvent(SDL_MouseMotionEvent &event)
{
    _mouseX = (float)event.x;
    _mouseY = (float)event.y;

    if (_isInRetinaMonitor && _retinaFactor == 1) {
        _mouseX *= 2;
        _mouseY *= 2;
    }

    if (!_isTouchDevice)
    {
        if (_captured)
        {
            intptr_t id = 0;
            this->handleTouchesMove(1, &id, &_mouseX, &_mouseY);
        }
    }
    
    //Because OpenGL and cocos2d-x uses different Y axis, we need to convert the coordinate here
    float cursorX = (_mouseX - _viewPortRect.origin.x) / _scaleX;
    float cursorY = (_viewPortRect.origin.y + _viewPortRect.size.height - _mouseY) / _scaleY;
    
    // Hmm, EventMouse only supports one mouse button for move events
    // Just send the lowest mouse button for now
    EventMouse eventMouse(EventMouse::MouseEventType::MOUSE_MOVE);
    if (event.state & SDL_BUTTON_LMASK) {
        eventMouse.setMouseButton(EventMouse::MouseButton::BUTTON_LEFT);
    } else if (event.state & SDL_BUTTON_RMASK) {
        eventMouse.setMouseButton(EventMouse::MouseButton::BUTTON_RIGHT);
    } else if (event.state & SDL_BUTTON_MMASK) {
        eventMouse.setMouseButton(EventMouse::MouseButton::BUTTON_MIDDLE);
    } else if (event.state & SDL_BUTTON_X1MASK) {
        eventMouse.setMouseButton(EventMouse::MouseButton::BUTTON_4);
    } else if (event.state & SDL_BUTTON_X2MASK) {
        eventMouse.setMouseButton(EventMouse::MouseButton::BUTTON_5);
    }
    eventMouse.setCursorPosition(cursorX, cursorY);
    Director::getInstance()->getEventDispatcher()->dispatchEvent(&eventMouse);
}

void GLViewImpl::onMouseWheelEvent(SDL_MouseWheelEvent &event)
{
    EventMouse eventMouse(EventMouse::MouseEventType::MOUSE_SCROLL);
    // Because OpenGL and cocos2d-x uses different Y axis, we need to convert the coordinate here
    float cursorX = (_mouseX - _viewPortRect.origin.x) / _scaleX;
    float cursorY = (_viewPortRect.origin.y + _viewPortRect.size.height - _mouseY) / _scaleY;
    eventMouse.setScrollData(event.preciseX, -event.preciseY);
    eventMouse.setCursorPosition(cursorX, cursorY);
    Director::getInstance()->getEventDispatcher()->dispatchEvent(&eventMouse);
}

void GLViewImpl::onKeyboardEvent(SDL_KeyboardEvent &event, bool isDown)
{
    //if (event.repeat > 0) return;
    
    EventKeyboard eventKeyboard(g_keyCodeMap[event.keysym.sym], isDown);
    auto dispatcher = Director::getInstance()->getEventDispatcher();
    dispatcher->dispatchEvent(&eventKeyboard);

    if (event.type != SDL_KEYUP)
    {
        switch (g_keyCodeMap[event.keysym.sym])
        {
        case EventKeyboard::KeyCode::KEY_BACKSPACE:
            IMEDispatcher::sharedDispatcher()->dispatchDeleteBackward();
            break;
        case EventKeyboard::KeyCode::KEY_HOME:
        case EventKeyboard::KeyCode::KEY_KP_HOME:
        case EventKeyboard::KeyCode::KEY_DELETE:
        case EventKeyboard::KeyCode::KEY_KP_DELETE:
        case EventKeyboard::KeyCode::KEY_END:
        case EventKeyboard::KeyCode::KEY_LEFT_ARROW:
        case EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
        case EventKeyboard::KeyCode::KEY_ESCAPE:
            IMEDispatcher::sharedDispatcher()->dispatchControlKey(g_keyCodeMap[event.keysym.sym]);
            break;
        default:
            break;
        }
    }

}

void GLViewImpl::onTextInputEvent(SDL_Event &sdlEvent)
{
    IMEDispatcher::sharedDispatcher()->dispatchInsertText(sdlEvent.text.text, strlen(sdlEvent.text.text));
}

void GLViewImpl::onDisplayEvent(SDL_DisplayEvent &event)
{
    switch(event.event) {
        // TODO!
        //case SDL_DISPLAYEVENT_ORIENTATION:      Application::getInstance()->applicationMonitorOrientation(); break;
        //case SDL_DISPLAYEVENT_CONNECTED:        Application::getInstance()->applicationMonitorConnected(); break;
        //case SDL_DISPLAYEVENT_DISCONNECTED:     Application::getInstance()->applicationMonitorDisconnected(); break;
    }
}

void GLViewImpl::onWindowEvent(SDL_WindowEvent &event)
{
    switch(event.event) {
        case SDL_WINDOWEVENT_MOVED:         Director::getInstance()->setViewport(); break;
        //case SDL_WINDOWEVENT_RESIZED:     break;
        case SDL_WINDOWEVENT_SIZE_CHANGED:  onWindowSizeChanged(event.data1, event.data2); break;
        //case SDL_WINDOWEVENT_EXPOSED:       log("SDL_WINDOWEVENT_EXPOSED"); break;            
        case SDL_WINDOWEVENT_HIDDEN:        Application::getInstance()->applicationDidEnterBackground(); break;
        case SDL_WINDOWEVENT_SHOWN:         Application::getInstance()->applicationWillEnterForeground(); break;
        case SDL_WINDOWEVENT_MINIMIZED:     Application::getInstance()->applicationDidEnterBackground(); break;
        case SDL_WINDOWEVENT_MAXIMIZED:     log("SDL_WINDOWEVENT_MAXIMIZED"); break;
        case SDL_WINDOWEVENT_RESTORED:      Application::getInstance()->applicationWillEnterForeground(); break;
            
        case SDL_WINDOWEVENT_FOCUS_GAINED:
            Director::getInstance()->getEventDispatcher()->dispatchCustomEvent(GLViewImpl::EVENT_WINDOW_FOCUSED, nullptr);
            break;
        case SDL_WINDOWEVENT_FOCUS_LOST:
            Director::getInstance()->getEventDispatcher()->dispatchCustomEvent(GLViewImpl::EVENT_WINDOW_UNFOCUSED, nullptr);
            break;
            
        case SDL_WINDOWEVENT_DISPLAY_CHANGED:
            //TODO!!! Application::getInstance()->applicationWindowDisplayChanged();
            break;
    }
}

void GLViewImpl::onWindowSizeChanged(int width, int height)
{
    if (width && height && _resolutionPolicy != ResolutionPolicy::UNKNOWN) {

#if defined(AX_USE_METAL)
        int drW = 0, drH = 0;
        SDL_Metal_GetDrawableSize(_mainWindow, &drW, &drH);
        backend::UtilsMTL::resizeDefaultAttachmentTexture(drW,drH);    
#endif
        
        Size baseDesignSize = _designResolutionSize;
        ResolutionPolicy baseResolutionPolicy = _resolutionPolicy;

        setFrameSize(width, height);
        setDesignResolutionSize(baseDesignSize.width, baseDesignSize.height, baseResolutionPolicy);
        Director::getInstance()->setViewport();
        Director::getInstance()->getEventDispatcher()->dispatchCustomEvent(GLViewImpl::EVENT_WINDOW_RESIZED, nullptr);
    }    
}

#if (AX_TARGET_PLATFORM != AX_PLATFORM_MAC)
static bool loadFboExtensions()
{
    const char* gl_extensions = (const char*)glGetString(GL_EXTENSIONS);
    // If the current opengl driver doesn't have framebuffers methods, check if an extension exists
    if (glGenFramebuffers == nullptr)
    {
        log("OpenGL: glGenFramebuffers is nullptr, try to detect an extension");
        if (strstr(gl_extensions, "ARB_framebuffer_object")) {
            log("OpenGL: ARB_framebuffer_object is supported");
            glIsRenderbuffer = (PFNGLISRENDERBUFFERPROC)SDL_GL_GetProcAddress("glIsRenderbuffer");
            glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC)SDL_GL_GetProcAddress("glBindRenderbuffer");
            glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC)SDL_GL_GetProcAddress("glDeleteRenderbuffers");
            glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC)SDL_GL_GetProcAddress("glGenRenderbuffers");
            glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC)SDL_GL_GetProcAddress("glRenderbufferStorage");
            glGetRenderbufferParameteriv = (PFNGLGETRENDERBUFFERPARAMETERIVPROC)SDL_GL_GetProcAddress("glGetRenderbufferParameteriv");
            glIsFramebuffer = (PFNGLISFRAMEBUFFERPROC)SDL_GL_GetProcAddress("glIsFramebuffer");
            glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)SDL_GL_GetProcAddress("glBindFramebuffer");
            glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)SDL_GL_GetProcAddress("glDeleteFramebuffers");
            glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)SDL_GL_GetProcAddress("glGenFramebuffers");
            glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)SDL_GL_GetProcAddress("glCheckFramebufferStatus");
            glFramebufferTexture1D = (PFNGLFRAMEBUFFERTEXTURE1DPROC)SDL_GL_GetProcAddress("glFramebufferTexture1D");
            glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)SDL_GL_GetProcAddress("glFramebufferTexture2D");
            glFramebufferTexture3D = (PFNGLFRAMEBUFFERTEXTURE3DPROC)SDL_GL_GetProcAddress("glFramebufferTexture3D");
            glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)SDL_GL_GetProcAddress("glFramebufferRenderbuffer");
            glGetFramebufferAttachmentParameteriv = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC)SDL_GL_GetProcAddress("glGetFramebufferAttachmentParameteriv");
            glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC)SDL_GL_GetProcAddress("glGenerateMipmap");
        } else if (strstr(gl_extensions, "EXT_framebuffer_object")) {
            log("OpenGL: EXT_framebuffer_object is supported");
            glIsRenderbuffer = (PFNGLISRENDERBUFFERPROC)SDL_GL_GetProcAddress("glIsRenderbufferEXT");
            glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC)SDL_GL_GetProcAddress("glBindRenderbufferEXT");
            glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC)SDL_GL_GetProcAddress("glDeleteRenderbuffersEXT");
            glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC)SDL_GL_GetProcAddress("glGenRenderbuffersEXT");
            glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC)SDL_GL_GetProcAddress("glRenderbufferStorageEXT");
            glGetRenderbufferParameteriv = (PFNGLGETRENDERBUFFERPARAMETERIVPROC)SDL_GL_GetProcAddress("glGetRenderbufferParameterivEXT");
            glIsFramebuffer = (PFNGLISFRAMEBUFFERPROC)SDL_GL_GetProcAddress("glIsFramebufferEXT");
            glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)SDL_GL_GetProcAddress("glBindFramebufferEXT");
            glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)SDL_GL_GetProcAddress("glDeleteFramebuffersEXT");
            glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)SDL_GL_GetProcAddress("glGenFramebuffersEXT");
            glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)SDL_GL_GetProcAddress("glCheckFramebufferStatusEXT");
            glFramebufferTexture1D = (PFNGLFRAMEBUFFERTEXTURE1DPROC)SDL_GL_GetProcAddress("glFramebufferTexture1DEXT");
            glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)SDL_GL_GetProcAddress("glFramebufferTexture2DEXT");
            glFramebufferTexture3D = (PFNGLFRAMEBUFFERTEXTURE3DPROC)SDL_GL_GetProcAddress("glFramebufferTexture3DEXT");
            glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)SDL_GL_GetProcAddress("glFramebufferRenderbufferEXT");
            glGetFramebufferAttachmentParameteriv = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC)SDL_GL_GetProcAddress("glGetFramebufferAttachmentParameterivEXT");
            glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC)SDL_GL_GetProcAddress("glGenerateMipmapEXT");
        } else if (strstr(gl_extensions, "GL_ANGLE_framebuffer_blit")) {
            log("OpenGL: GL_ANGLE_framebuffer_object is supported");
            glIsRenderbuffer = (PFNGLISRENDERBUFFERPROC)SDL_GL_GetProcAddress("glIsRenderbufferOES");
            glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC)SDL_GL_GetProcAddress("glBindRenderbufferOES");
            glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC)SDL_GL_GetProcAddress("glDeleteRenderbuffersOES");
            glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC)SDL_GL_GetProcAddress("glGenRenderbuffersOES");
            glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC)SDL_GL_GetProcAddress("glRenderbufferStorageOES");
            //glGetRenderbufferParameteriv = (PFNGLGETRENDERBUFFERPARAMETERIVPROC)SDL_GL_GetProcAddress("glGetRenderbufferParameterivOES");
            glIsFramebuffer = (PFNGLISFRAMEBUFFERPROC)SDL_GL_GetProcAddress("glIsFramebufferOES");
            glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)SDL_GL_GetProcAddress("glBindFramebufferOES");
            glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)SDL_GL_GetProcAddress("glDeleteFramebuffersOES");
            glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)SDL_GL_GetProcAddress("glGenFramebuffersOES");
            glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)SDL_GL_GetProcAddress("glCheckFramebufferStatusOES");
            glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)SDL_GL_GetProcAddress("glFramebufferRenderbufferOES");
            glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)SDL_GL_GetProcAddress("glFramebufferTexture2DOES");
            glGetFramebufferAttachmentParameteriv = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC)SDL_GL_GetProcAddress("glGetFramebufferAttachmentParameterivOES");
            glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC)SDL_GL_GetProcAddress("glGenerateMipmapOES");
        } else {
            log("OpenGL: No framebuffers extension is supported");
            log("OpenGL: Any call to Fbo will crash!");
            return false;
        }
    }
    return true;
}

// helper
bool GLViewImpl::loadGL()
{
#    if (AX_TARGET_PLATFORM != AX_PLATFORM_MAC)

    // glad: load all OpenGL function pointers
    // ---------------------------------------
#        if !AX_GLES_PROFILE
    if (!gladLoadGL(glfwGetProcAddress))
    {
        log("glad: Failed to Load GL");
        return false;
    }
#        else
    if (!gladLoadGLES2(glfwGetProcAddress))
    {
        log("glad: Failed to Load GLES2");
        return false;
    }
#        endif

    loadFboExtensions();
#    endif  // (AX_TARGET_PLATFORM != AX_PLATFORM_MAC)

    return true;
}

#endif

NS_AX_END  // end of namespace ax;
