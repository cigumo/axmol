
#ifndef __cocos2d_libs__ControllerImpl_SDL__
#define __cocos2d_libs__ControllerImpl_SDL__

#include "base/Controller.h"
#include <SDL.h>

NS_AX_BEGIN

class AX_DLL ControllerImpl
{
public:
    static ControllerImpl* getInstance();
    static void handleEvent(SDL_Event* event);
    void startDiscovery();
    void stopDiscovery();
    
private:
    ControllerImpl();
    ~ControllerImpl();

    static std::vector<Controller*>::iterator findController(SDL_JoystickID joyId);
    static void addController(int device_index); //SDL_JoystickID joyId, SDL_Joystick* joy);
    static void removeController(SDL_JoystickID joyId);
    
    static std::map<SDL_JoystickID, SDL_GameController*>allGameControllers;
    static std::unordered_map<int, int> sdlButtonMap;
    static std::unordered_map<int, int> sdlAxisMap;
};
    
NS_AX_END

#endif /* __cocos2d_libs__ControllerImpl_SDL__ */

