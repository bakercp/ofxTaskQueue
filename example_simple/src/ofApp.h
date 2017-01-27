//
// Copyright (c) 2014 Christopher Baker <https://christopherbaker.net>
//
// SPDX-License-Identifier:	MIT
//


#pragma once


#include "ofMain.h"
#include "ofxTaskQueue.h"
#include "SimpleCountingTask.h"


class ofApp: public ofBaseApp
{
public:
    void setup();
    void draw();

    void keyPressed(int key);

    ofx::TaskQueue queue;

};
