// =============================================================================
//
// Copyright (c) 2014 Christopher Baker <http://christopherbaker.net>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// =============================================================================


#include "ofApp.h"


void ofApp::setup()
{
    ofEnableAlphaBlending();

    ofSetFrameRate(60);

    for (int i = 0; i < 100; ++i)
    {
        std::string name = "Test Task #" + ofToString(i);
        queue.start(new CountingTask(name, 100));
    }
}


void ofApp::update()
{
}


void ofApp::draw()
{
    ofBackground(0);

    const ofx::TaskQueue::TaskProgressMap& tasks = queue.getTaskProgressMap();

    ofx::TaskQueue::TaskProgressMap::const_iterator iter = tasks.begin();

    int y = 0;
    int height = 20;

    while (iter != tasks.end())
    {
        const ofx::TaskProgress& task = iter->second;

        ofPushMatrix();
        ofTranslate(0, y);
        ofFill();

        if (task.getProgress() < 0) // Failed.
        {
            ofSetColor(255, 0, 0);
        }
        else if (task.getProgress() > 0)
        {
            ofSetColor(0, 255, 0, 50);
        }
        else
        {
            ofSetColor(255, 80);
        }

        ofRect(0, 0, ofGetWidth(), height);

        if (task.getProgress() > 0)
        {
            ofFill();
            ofSetColor(255, 255, 0, 75);
            ofRect(0, 0, task.getProgress() * ofGetWidth(), height);
        }

        ofSetColor(255);

        std::stringstream ss;

        ss << task.getName() << " ";
        ss << (task.getProgress() * 100);

        if (!task.getErrorMessage().empty())
        {
            ss << task.getErrorMessage();
        }

        ofDrawBitmapString(ss.str(), ofPoint(10, 14, 0));
        
        ofPopMatrix();

        y += (height + 5);
        ++iter;
    }
}


void ofApp::keyPressed(int key)
{
    if ('c' == key)
    {
        queue.cancelAll();
    }
}

