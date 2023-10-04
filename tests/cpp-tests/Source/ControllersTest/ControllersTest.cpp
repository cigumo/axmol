/****************************************************************************
 Copyright (c) 2023 Ciro Mondueri (Kalio Ltda.)

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

#if !defined(__EMSCRIPTEN__)

#include "platform/PlatformConfig.h"
#include "ControllersTest.h"
#include "stdio.h"
#include "stdlib.h"

USING_NS_AX;

std::map<int, std::string> ControllersTest::buttonNames;

ControllersTests::ControllersTests()
{
    ADD_TEST_CASE(ControllersTest);
}

ControllersTest::ControllersTest()
{
    ControllersTest::buttonNames[Controller::Key::BUTTON_A]               = "A";
    ControllersTest::buttonNames[Controller::Key::BUTTON_B]               = "B";
    ControllersTest::buttonNames[Controller::Key::BUTTON_X]               = "X";
    ControllersTest::buttonNames[Controller::Key::BUTTON_Y]               = "Y";
    ControllersTest::buttonNames[Controller::Key::BUTTON_PAUSE]           = "PA";
    ControllersTest::buttonNames[Controller::Key::BUTTON_SELECT]          = "SE";
    ControllersTest::buttonNames[Controller::Key::BUTTON_START]           = "ST";
    ControllersTest::buttonNames[Controller::Key::BUTTON_LEFT_THUMBSTICK] = "LT";
    ControllersTest::buttonNames[Controller::Key::BUTTON_RIGHT_THUMBSTICK]= "RT";
    ControllersTest::buttonNames[Controller::Key::BUTTON_LEFT_SHOULDER]   = "LS";
    ControllersTest::buttonNames[Controller::Key::BUTTON_RIGHT_SHOULDER]  = "RS";
    ControllersTest::buttonNames[Controller::Key::BUTTON_DPAD_UP]         = "DU";
    ControllersTest::buttonNames[Controller::Key::BUTTON_DPAD_DOWN]       = "DD";
    ControllersTest::buttonNames[Controller::Key::BUTTON_DPAD_LEFT]       = "DL";
    ControllersTest::buttonNames[Controller::Key::BUTTON_DPAD_RIGHT]      = "DR";

    auto center = VisibleRect::center();
    
    auto label = Label::createWithTTF("Controllers Test", "fonts/arial.ttf", 15);
    label->setPosition(center.x, center.y + 100);
    addChild(label, 0);
    
    // create a label to display the tip string
    _cCount = Label::createWithTTF("Controller count", "fonts/arial.ttf", 14);
    _cCount->setPosition(Vec2(center.x, center.y + 80));
    addChild(_cCount, 0);

    _rows = Node::create();
    addChild(_rows);

    // add those already connected
    int count = 0;
    for (auto c : Controller::getAllController()) {
        auto row = createRow(fmt("%i",c->getDeviceId()));
        row->setPosition(20, center.y - (count++)*15);
        row->setTag(c->getDeviceId());
        _rows->addChild(row);
    }
            
    Director::getInstance()->getScheduler()->scheduleUpdate(this, 0, false);

    _controllerListener = EventListenerController::create();
    _controllerListener->onKeyDown = AX_CALLBACK_3(ControllersTest::onButtonPressed, this);
    _controllerListener->onKeyUp = AX_CALLBACK_3(ControllersTest::onButtonReleased, this);
    _controllerListener->onConnected = AX_CALLBACK_2(ControllersTest::onConnected, this);
    _controllerListener->onDisconnected = AX_CALLBACK_2(ControllersTest::onDisconnected, this);
    _controllerListener->retain();
    
    EventDispatcher* dispatcher = Director::getInstance()->getEventDispatcher();
    dispatcher->addEventListenerWithSceneGraphPriority(_controllerListener, this);
    
    Controller::startDiscoveryController();
}

ControllersTest::~ControllersTest()
{
    Director::getInstance()->getScheduler()->unscheduleUpdate(this);

    EventDispatcher* dispatcher = Director::getInstance()->getEventDispatcher();
    dispatcher->removeEventListener(_controllerListener);
    _controllerListener->release();
    
    _rows->removeFromParentAndCleanup(true);
}

void ControllersTest::update(float dt)
{
    std::vector<Controller*> controllers = Controller::getAllController();

    _cCount->setString(fmt("Joystick count:%i", controllers.size()));

    std::string axis = "";

    for (auto controller : controllers) {        
        float lx = controller->getKeyStatus(ax::Controller::Key::JOYSTICK_LEFT_X).value;
        float ly = controller->getKeyStatus(ax::Controller::Key::JOYSTICK_LEFT_Y).value;
        float rx = controller->getKeyStatus(ax::Controller::Key::JOYSTICK_RIGHT_X).value;
        float ry = controller->getKeyStatus(ax::Controller::Key::JOYSTICK_RIGHT_Y).value;

        auto row = _rows->getChildByTag(controller->getDeviceId());
        if (row) {
            dynamic_cast<Label*>(row->getChildByName("JOYSTICK_LEFT"))->setString(fmt("%0.03f,%0.03f",lx,ly));
            dynamic_cast<Label*>(row->getChildByName("JOYSTICK_RIGHT"))->setString(fmt("%0.03f,%0.03f",rx,ry));
        }
    }    
}

void ControllersTest::onButtonPressed(ax::Controller* c, int keyCode, Event* event)
{
    setButtonLabel(c->getDeviceId(), keyCode, true);

}
void ControllersTest::onButtonReleased(ax::Controller* c, int keyCode, Event* event)
{
    setButtonLabel(c->getDeviceId(), keyCode, false);
}

void ControllersTest::onConnected(ax::Controller* c, Event* event)
{
    if (_rows->getChildByTag(c->getDeviceId()))
        return;
    auto center = VisibleRect::center();
    auto count = _rows->getChildren().size();
    auto row = createRow(fmt("%i",c->getDeviceId()));
    row->setPosition(20, center.y - count*15);
    row->setTag(c->getDeviceId());
    _rows->addChild(row);
}

void ControllersTest::onDisconnected(ax::Controller* c, Event* event)
{
    auto deviceId = c->getDeviceId();
    auto row = _rows->getChildByTag(deviceId);
    if (row) {
        _rows->removeChildByTag(deviceId);
    }
}

void ControllersTest::setButtonLabel(int deviceId, int keyCode, bool visible)
{
    if (buttonNames.contains(keyCode)) {
        auto row = _rows->getChildByTag(deviceId);
        if (row) {
            auto name = buttonNames[keyCode];
            auto l = dynamic_cast<Label*>(row->getChildByName(name));
            if (l)
                l->setOpacity(visible ? 255 : 50);
        }
    }
}

Node* ControllersTest::createRow(std::string name)
{
    auto out = Node::create();

    auto CL = [out](std::string name, std::string txt, int font_size, int x, int opacity=50) {
        auto l = Label::createWithTTF(txt, "fonts/arial.ttf", font_size);
        l->setPosition(x,0);
        l->setName(name);
        l->setOpacity(opacity);
        out->addChild(l);
        return l;
    };
    
    CL("id",              name,        12,   0, 255);
    CL("JOYSTICK_LEFT",   "",          10,  40, 255);
    CL("JOYSTICK_RIGHT",  "",          10, 100, 255);

    int x = 140;
    int j = 0;
    for (auto &[id,name] : buttonNames) {
        CL(name, name, 10, x+(j++)*20);
    }
    return out;
}

std::string ControllersTest::fmt(const char* format, ...)
{
    va_list args;
    va_start (args, format);
    size_t len = std::vsnprintf(NULL, 0, format, args);
    va_end (args);
    std::vector<char> vec(len + 1);
    va_start (args, format);
    std::vsnprintf(&vec[0], len + 1, format, args);
    va_end (args);
    return &vec[0];
    
}

#endif
