//
// Copyright (c) 2014 Christopher Baker <https://christopherbaker.net>
//
// SPDX-License-Identifier:	MIT
//


#pragma once


#include "Poco/Task.h"
#include "Poco/Notification.h"
#include "Poco/Random.h"


class SimpleCountingTask: public Poco::Task
{
public:
    SimpleCountingTask(const std::string& name, float target);

    virtual ~SimpleCountingTask();
    
    void runTask();

private:
    float _targetNumber;
    float _currentNumber;

    Poco::Random rng;

};
