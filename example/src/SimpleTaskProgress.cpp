// =============================================================================
//
// Copyright (c) 2014-2015 Christopher Baker <http://christopherbaker.net>
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


#include "SimpleTaskProgress.h"
#include "ofGraphics.h"


SimpleTaskProgress::SimpleTaskProgress():
    taskId(""),
    name(""),
    state(Poco::Task::TASK_IDLE),
    progress(0),
    errorMessage(""),
    data(""),
    fader(1)
{
}


SimpleTaskProgress::SimpleTaskProgress(const ofx::TaskQueueEventArgs& args):
    taskId(args.getTaskId()),
    name(args.getTaskName()),
    state(args.getState()),
    progress(0),
    errorMessage(""),
    data(""),
    fader(1)
{
}


SimpleTaskProgress::~SimpleTaskProgress()
{
}


void SimpleTaskProgress::update(const ofx::TaskQueueEventArgs& args)
{
    taskId = args.getTaskId();
    name = args.getTaskName();
    state = args.getState();
}


void SimpleTaskProgress::update(const ofx::TaskProgressEventArgs& args)
{
    taskId = args.getTaskId();
    name = args.getTaskName();
    state = args.getState();
    progress = args.getProgress();
}


void SimpleTaskProgress::update(const ofx::TaskFailedEventArgs& args)
{
    taskId = args.getTaskId();
    name = args.getTaskName();
    state = args.getState();
    errorMessage = args.getException().displayText();
}


void SimpleTaskProgress::draw(float x, float y, float width, float height)
{
    if (Poco::Task::TASK_FINISHED == state)
    {
        fader = ofClamp(fader - 0.05, 0, 1);
    }

    ofPushMatrix();
    ofTranslate(x, y);

    ofNoFill();
    ofSetColor(255, 80);
    ofDrawRectangle(0, 0, width, height);

    ofFill();

    if (!errorMessage.empty()) // Failed.
    {
        ofSetColor(255, 0, 0, 255 * fader);
    }
    else if (state == Poco::Task::TASK_FINISHED)
    {
        ofSetColor(0, 0, 255, 100 * fader);
    }
    else if (progress > 0)
    {
        ofSetColor(0, 255, 0, 50 * fader);
    }
    else
    {
        //
        ofSetColor(255, 80 * fader);
    }

    // Background rectangle.
    ofDrawRectangle(0, 0, width, height);

    if (progress > 0)
    {
        ofFill();
        ofSetColor(255, 255, 0, 75 * fader);
        ofDrawRectangle(0, 0, progress * width, height);
    }

    ofSetColor(255, 255 * fader);

    std::stringstream ss;

    ss << taskId << " Name: " << name << " " << (progress * 100);

    if (!data.empty())
    {
        ss << " Received : " << data;
    }
    else
    {
        ss << " Waiting for data ...";
    }


    if (!errorMessage.empty())
    {
        ss << " Error: " << errorMessage;
    }

    ofDrawBitmapString(ss.str(), glm::vec3(10, 14, 0));

    ofPopMatrix();

}

