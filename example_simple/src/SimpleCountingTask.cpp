//
// Copyright (c) 2014 Christopher Baker <https://christopherbaker.net>
//
// SPDX-License-Identifier:	MIT
//


#define NOMINMAX


#include "SimpleCountingTask.h"
#include "Poco/TaskNotification.h"
#include "ofUtils.h"


SimpleCountingTask::SimpleCountingTask(const std::string& name, float target):
    Poco::Task(name),
    _targetNumber(target),
    _currentNumber(0)
{
    rng.seed();
}


SimpleCountingTask::~SimpleCountingTask()
{
}


void SimpleCountingTask::runTask()
{
    while (_currentNumber < _targetNumber)
    {
        // Generate a random increment to add.
        _currentNumber = std::min(_currentNumber + ofRandom(1), _targetNumber);

        setProgress(_currentNumber / _targetNumber); // report progress

        // In our custom tasks, we must regularly see if this task is cancelled.
        if (isCancelled())
        {
            break;
        }

        // If cancelled, sleep will also return true, require us to break.
        if (sleep(10))
        {
            break;
        }

        // We occasionally post a data notification, using a string.
        // Our TaskQueue can receive and process the
        // corresponding Poco::TaskCustomNotification, e.g.
        //
        // Poco::TaskCustomNotification<std::string> or
        // Poco::TaskCustomNotification<int> or

        float r = rng.nextFloat();

        if (r > 0.999)
        {
            std::string txt = "Here's a random number: " + ofToString(r);
            postNotification(new Poco::TaskCustomNotification<std::string>(this, txt));
        }
        else if (r > 0.998)
        {
            // Send a task notification that is not handled by the onTaskData event.
            postNotification(new Poco::TaskCustomNotification<int>(this, _currentNumber));
        }
        else if (r > 0.997)
        {
            // We occasionally throw an exception to demonstrate error recovery.
            throw Poco::Exception("Random Exception " + ofToString(r));
        }
    }

    // Finished.

}
