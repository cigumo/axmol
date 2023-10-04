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

#ifndef _CONTROLLERS_TEST_H_
#define _CONTROLLERS_TEST_H_

#if !defined(__EMSCRIPTEN__)

#include "axmol.h"
#include "../BaseTest.h"

DEFINE_TEST_SUITE(ControllersTests);

class ControllersTest : public TestCase
{
public:
    CREATE_FUNC(ControllersTest);

    ControllersTest();
    ~ControllersTest();

    void update(float dt);
    void onButtonPressed(ax::Controller* c, int keyCode, ax::Event* event);
    void onButtonReleased(ax::Controller* c, int keyCode, ax::Event* event);
    void onConnected(ax::Controller* c, ax::Event* event);
    void onDisconnected(ax::Controller* c, ax::Event* event);

    void setButtonLabel(int deviceId, int keyCode, bool visible);
    Node* createRow(std::string name);
        
private:
    ax::EventListenerController* _controllerListener;
    ax::Label* _cCount;
    ax::Node* _rows;
    
    std::string fmt(const char* format, ...);

    static std::map<int, std::string> buttonNames;
};

#endif

#endif  // _CONTROLLERS_TEST_H_
