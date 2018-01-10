//
// Copyright (c) 2014 Christopher Baker <https://christopherbaker.net>
//
// SPDX-License-Identifier:	MIT
//


#include "SimpleTaskProgress.h"
#include "ofGraphics.h"
#include "ofMath.h"


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
    taskId(args.taskId()),
    name(args.taskName()),
    state(args.state()),
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
    taskId = args.taskId();
    name = args.taskName();
    state = args.state();
}


void SimpleTaskProgress::update(const ofx::TaskProgressEventArgs& args)
{
    taskId = args.taskId();
    name = args.taskName();
    state = args.state();
    progress = args.progress();
}


void SimpleTaskProgress::update(const ofx::TaskFailedEventArgs& args)
{
    taskId = args.taskId();
    name = args.taskName();
    state = args.state();
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

