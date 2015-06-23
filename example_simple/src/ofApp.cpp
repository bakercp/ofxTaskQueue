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

    for (int i = 0; i < 1000; ++i)
    {
        std::string name = "Counting Task #" + ofToString(i);
        queue.start(new SimpleCountingTask(name, 100));
    }
}


void ofApp::draw()
{
    ofBackground(0);

    std::stringstream ss;

    ss << "Press 'c' to cancel all tasks." << std::endl;
    ss << "Press 'C' to cancel queued tasks." << std::endl;
    ss << "Press 'a' to add tasks." << std::endl;

    ofDrawBitmapStringHighlight(ss.str(), ofPoint(10, 14));

    ss.str("");
    ss << "Total Active Tasks: " << queue.getActiveCount() << std::endl;
    ss << "Total Queued Tasks: " << queue.getQueuedCount();

    ofDrawBitmapStringHighlight(ss.str(), ofPoint(ofGetWidth() / 2, 14));

    int height = 20;
    int y = height * 3;

    ofx::TaskQueue::ProgressMap progress = queue.getTaskProgress();
    ofx::TaskQueue::ProgressMap::const_iterator iter = progress.begin();

    while (iter != progress.end() && y < ofGetHeight())
    {
        const ofx::TaskProgressEventArgs& progressInfo = iter->second;

        float progress = progressInfo.getProgress();

        std::string taskId = progressInfo.getTaskId();

        std::string name = progressInfo.getTaskName();

        ofColor color;

        std::string statusString;

        switch (progressInfo.getState())
        {
            case Poco::Task::TASK_IDLE:
                color = ofColor(127);
                statusString = "idle";
                break;
            case Poco::Task::TASK_STARTING:
                color = ofColor(255, 255, 0);
                statusString = "starting";
                break;
            case Poco::Task::TASK_RUNNING:
                color = ofColor(255, 127, 127);
                statusString = "running";
                break;
            case Poco::Task::TASK_CANCELLING:
                color = ofColor(255, 0, 0);
                statusString = "cancelling";
                break;
            case Poco::Task::TASK_FINISHED:
                color = ofColor(0, 255, 0);
                statusString = "finished";
                break;
        }

        std::stringstream ss;

        ss << taskId << ": " << statusString << " : " << name << std::endl;

        ofPushMatrix();
        ofTranslate(0, y);

        ofFill();
        ofSetColor(color, 127);
        ofRect(0, 0, ofGetWidth() * progress, height - 2);

        ofNoFill();
        ofSetColor(color);
        ofRect(0, 0, ofGetWidth() * progress, height - 2);

        ofFill();
        ofSetColor(255);
        ofDrawBitmapString(ss.str(), 4, 14);

        ofPopMatrix();

        y += height;

        ++iter;
    }
}


void ofApp::keyPressed(int key)
{
    if ('c' == key)
    {
        queue.cancelAll();
    }
    else if ('C' == key)
    {
        queue.cancelQueued();
    }
    else if ('a' == key)
    {
        queue.start(new SimpleCountingTask("User manually added!", 100));
    }
}

