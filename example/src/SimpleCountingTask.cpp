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


#include "SimpleCountingTask.h"
#include "Poco/TaskNotification.h"
#include "ofUtils.h"


SimpleCountingTask::SimpleCountingTask(const std::string& name, float target):
    Poco::Task(name),
    _targetNumber(target),
    _currentNumber(0)
{
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
        // Our TaskQueue_<std::string> can receive and process the
        // corresponding Poco::TaskCustomNotification, e.g.
        //
        // Poco::TaskCustomNotification<std::string>
        if (ofRandom(0, 1) > 0.999)
        {
            std::string txt = "Here's a random number: " + ofToString(ofRandom(1000));

            postNotification(new Poco::TaskCustomNotification<std::string>(this, txt));
        }

        // We occasionally throw an exception to demonstrate error recovery.
        if (ofRandom(0, 1) > 0.999)
        {
            throw Poco::Exception("Random Exception");
        }
    }

    // Finished.

}
