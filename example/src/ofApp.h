//
// Copyright (c) 2014 Christopher Baker <https://christopherbaker.net>
//
// SPDX-License-Identifier:	MIT
//


#pragma once


#include "ofMain.h"
#include "ofxTaskQueue.h"
#include "SimpleCountingTask.h"
#include "SimpleTaskProgress.h"


class ofApp: public ofBaseApp
{
public:
    void setup();
    void update();
    void draw();
    void exit();

    void keyPressed(int key);

    void onTaskQueued(const ofx::TaskQueueEventArgs& args);
    void onTaskStarted(const ofx::TaskQueueEventArgs& args);
    void onTaskCancelled(const ofx::TaskQueueEventArgs& args);
    void onTaskFinished(const ofx::TaskQueueEventArgs& args);
    void onTaskFailed(const ofx::TaskFailedEventArgs& args);
    void onTaskProgress(const ofx::TaskProgressEventArgs& args);

    void onTaskCustomNotification(const ofx::TaskCustomNotificationEventArgs& args);

    ofx::TaskQueue queue;

    // Make a typedef for the map to make it shorter.
    typedef std::map<std::string, SimpleTaskProgress> TaskProgress;

    // We keep a simple task progress queue.
    TaskProgress taskProgress;

};
