//
// Copyright (c) 2014 Christopher Baker <https://christopherbaker.net>
//
// SPDX-License-Identifier:	MIT
//


#pragma once


#include <string>
#include "Poco/Task.h"
#include "ofx/TaskQueueEvents.h"


class SimpleTaskProgress
{
public:
    SimpleTaskProgress();
    SimpleTaskProgress(const ofx::TaskQueueEventArgs& args);

    virtual ~SimpleTaskProgress();

    void update(const ofx::TaskQueueEventArgs& args);
    void update(const ofx::TaskProgressEventArgs& args);
    void update(const ofx::TaskFailedEventArgs& args);

    void draw(float x, float y, float width, float height);

    std::string taskId;
    std::string name;
    Poco::Task::TaskState state;
    float progress;
    std::string errorMessage;
    std::string data;

    float fader;

};
