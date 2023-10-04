
#include "ControllerImpl-SDL.h"

NS_AX_BEGIN

std::map<SDL_JoystickID, SDL_GameController*>ControllerImpl::allGameControllers;
std::unordered_map<int, int> ControllerImpl::sdlButtonMap;
std::unordered_map<int, int> ControllerImpl::sdlAxisMap;

ControllerImpl::ControllerImpl()
{
    // SDL to Cocos button mapping
    ControllerImpl::sdlButtonMap[SDL_CONTROLLER_BUTTON_INVALID]       = Controller::Key::KEY_NONE;
    ControllerImpl::sdlButtonMap[SDL_CONTROLLER_BUTTON_A]             = Controller::Key::BUTTON_A;
    ControllerImpl::sdlButtonMap[SDL_CONTROLLER_BUTTON_B]             = Controller::Key::BUTTON_B;
    ControllerImpl::sdlButtonMap[SDL_CONTROLLER_BUTTON_X]             = Controller::Key::BUTTON_X;
    ControllerImpl::sdlButtonMap[SDL_CONTROLLER_BUTTON_Y]             = Controller::Key::BUTTON_Y;
    ControllerImpl::sdlButtonMap[SDL_CONTROLLER_BUTTON_BACK]          = Controller::Key::BUTTON_SELECT;
    ControllerImpl::sdlButtonMap[SDL_CONTROLLER_BUTTON_GUIDE]         = Controller::Key::BUTTON_PAUSE;
    ControllerImpl::sdlButtonMap[SDL_CONTROLLER_BUTTON_START]         = Controller::Key::BUTTON_START;
    ControllerImpl::sdlButtonMap[SDL_CONTROLLER_BUTTON_LEFTSTICK]     = Controller::Key::BUTTON_LEFT_THUMBSTICK;
    ControllerImpl::sdlButtonMap[SDL_CONTROLLER_BUTTON_RIGHTSTICK]    = Controller::Key::BUTTON_RIGHT_THUMBSTICK;
    ControllerImpl::sdlButtonMap[SDL_CONTROLLER_BUTTON_LEFTSHOULDER]  = Controller::Key::BUTTON_LEFT_SHOULDER;
    ControllerImpl::sdlButtonMap[SDL_CONTROLLER_BUTTON_RIGHTSHOULDER] = Controller::Key::BUTTON_RIGHT_SHOULDER;
    ControllerImpl::sdlButtonMap[SDL_CONTROLLER_BUTTON_DPAD_UP]       = Controller::Key::BUTTON_DPAD_UP;
    ControllerImpl::sdlButtonMap[SDL_CONTROLLER_BUTTON_DPAD_DOWN]     = Controller::Key::BUTTON_DPAD_DOWN;
    ControllerImpl::sdlButtonMap[SDL_CONTROLLER_BUTTON_DPAD_LEFT]     = Controller::Key::BUTTON_DPAD_LEFT;
    ControllerImpl::sdlButtonMap[SDL_CONTROLLER_BUTTON_DPAD_RIGHT]    = Controller::Key::BUTTON_DPAD_RIGHT;
    ControllerImpl::sdlButtonMap[SDL_CONTROLLER_BUTTON_MISC1]         = Controller::Key::KEY_NONE;
    ControllerImpl::sdlButtonMap[SDL_CONTROLLER_BUTTON_PADDLE1]       = Controller::Key::KEY_NONE; 
    ControllerImpl::sdlButtonMap[SDL_CONTROLLER_BUTTON_PADDLE2]       = Controller::Key::KEY_NONE;
    ControllerImpl::sdlButtonMap[SDL_CONTROLLER_BUTTON_PADDLE3]       = Controller::Key::KEY_NONE;
    ControllerImpl::sdlButtonMap[SDL_CONTROLLER_BUTTON_PADDLE4]       = Controller::Key::KEY_NONE;
    ControllerImpl::sdlButtonMap[SDL_CONTROLLER_BUTTON_TOUCHPAD]      = Controller::Key::KEY_NONE;

    // SDL to Cocos axis mapping
    ControllerImpl::sdlAxisMap[SDL_CONTROLLER_AXIS_INVALID]      = Controller::Key::KEY_NONE;
    ControllerImpl::sdlAxisMap[SDL_CONTROLLER_AXIS_LEFTX]        = Controller::Key::JOYSTICK_LEFT_X;
    ControllerImpl::sdlAxisMap[SDL_CONTROLLER_AXIS_LEFTY]        = Controller::Key::JOYSTICK_LEFT_Y;
    ControllerImpl::sdlAxisMap[SDL_CONTROLLER_AXIS_RIGHTX]       = Controller::Key::JOYSTICK_RIGHT_X;
    ControllerImpl::sdlAxisMap[SDL_CONTROLLER_AXIS_RIGHTY]       = Controller::Key::JOYSTICK_RIGHT_Y;
    ControllerImpl::sdlAxisMap[SDL_CONTROLLER_AXIS_TRIGGERLEFT]  = Controller::Key::AXIS_LEFT_TRIGGER;
    ControllerImpl::sdlAxisMap[SDL_CONTROLLER_AXIS_TRIGGERRIGHT] = Controller::Key::AXIS_RIGHT_TRIGGER;

    if (SDL_InitSubSystem(SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) < 0) {
        printf("SDL could not initialize joysticks! SDL_Error: %s\n", SDL_GetError());
    }
}

ControllerImpl::~ControllerImpl()
{
}

ControllerImpl* ControllerImpl::getInstance()
{
    static ControllerImpl instance;
    return &instance;
}

std::vector<Controller*>::iterator ControllerImpl::findController(SDL_JoystickID joyId)
{
    auto iter = std::find_if(Controller::s_allController.begin(), Controller::s_allController.end(), [&](Controller* controller) {
        return joyId == static_cast<SDL_JoystickID>(controller->_deviceId);
    });
    return iter;
}
    
void ControllerImpl::startDiscovery()
{
    SDL_JoystickEventState(SDL_ENABLE);
    SDL_GameControllerEventState(SDL_ENABLE);    
}

void ControllerImpl::stopDiscovery()
{
    SDL_JoystickEventState(SDL_DISABLE);
    SDL_GameControllerEventState(SDL_DISABLE);

    for (auto&& controller : Controller::s_allController) {        
        delete controller;
    }
    Controller::s_allController.clear();

    for (const auto& [joyId,gc] : allGameControllers) {
        SDL_GameControllerClose(gc);
    }
    allGameControllers.clear();
}

void ControllerImpl::addController(int device_index)// SDL_JoystickID joyId, SDL_Joystick* joy)
{
    SDL_JoystickID joyId = SDL_JoystickGetDeviceInstanceID(device_index);

    // check if already exists
    auto iter = findController(joyId);
    if (iter != Controller::s_allController.end())
        return;

    // add to open sdl game controllers map
    SDL_GameController* gc = SDL_GameControllerOpen(device_index);
    allGameControllers[joyId] = gc;

    // add to Controller list
    auto controller         = new ax::Controller();
    controller->_deviceId   = static_cast<int>(joyId);
    controller->_deviceName = std::string_view(SDL_GameControllerName(gc));
    Controller::s_allController.emplace_back(controller);

    controller->onConnected();
}

void ControllerImpl::removeController(SDL_JoystickID joyId)
{
    auto iter = findController(joyId);
    if (iter == Controller::s_allController.end()) {
        AXLOGERROR("ControllerImpl::removeController : could not remove controller %i", (int)joyId);            
        return;
    }    

    // remove from controller list
    (*iter)->onDisconnected();
    Controller::s_allController.erase(iter);

    // close the sdl controller
    if (allGameControllers.contains(joyId)) {
        auto gc = allGameControllers[joyId];
        SDL_GameControllerClose(gc);
        allGameControllers.erase(joyId);
    }
}

void ControllerImpl::handleEvent(SDL_Event* event)
{
    switch (event->type) {
        case SDL_CONTROLLERDEVICEADDED:
            // which: the device index
            addController(event->cdevice.which);
            break;

        case SDL_CONTROLLERDEVICEREMOVED:
            // which: the instance id
            removeController(event->cdevice.which);
            break;

        case SDL_CONTROLLERDEVICEREMAPPED:
            AXLOG("TODO SDL_CONTROLLERDEVICEREMAPPED");
            break;
            
        case SDL_CONTROLLERBUTTONDOWN:
        case SDL_CONTROLLERBUTTONUP:
        {
            auto iter = findController(event->cbutton.which);
            if (iter == Controller::s_allController.end()) {
                AXLOG("ControllerImpl::handleEvent : event from unknown controller %i. Ignoring", event->cbutton.which);
            }
            int keyCode = ControllerImpl::sdlButtonMap[event->cbutton.button];
            (*iter)->onButtonEvent(keyCode, event->cbutton.state == SDL_PRESSED, 0, false);
        }
            break;

        case SDL_CONTROLLERAXISMOTION:
        {
            auto iter = findController(event->caxis.which);
            if (iter == Controller::s_allController.end()) {
                AXLOG("ControllerImpl::handleEvent : event from unknown controller %i. Ignoring", event->caxis.which);
            }
            int axisCode = ControllerImpl::sdlAxisMap[event->caxis.axis];
            (*iter)->onAxisEvent(axisCode, (event->caxis.value / 32767.0f), true);
        }
            break;

        default:
            break;
    }        
        
}
    
NS_AX_END
