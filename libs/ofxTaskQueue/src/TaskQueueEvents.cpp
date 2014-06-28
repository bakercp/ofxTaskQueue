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


#include "ofx/TaskQueueEvents.h"


namespace ofx {


BaseTaskEventArgs::BaseTaskEventArgs(const Poco::UUID& taskId,
                                     const std::string& taskName,
                                     Poco::Task::TaskState state):
    _taskId(taskId),
    _taskName(taskName),
    _state(state)
{
}


BaseTaskEventArgs::~BaseTaskEventArgs()
{
}


const Poco::UUID& BaseTaskEventArgs::getTaskId() const
{
    return _taskId;
}


const std::string& BaseTaskEventArgs::getTaskName() const
{
    return _taskName;
}


Poco::Task::TaskState BaseTaskEventArgs::getState() const
{
    return _state;
}


TaskFailedEventArgs::TaskFailedEventArgs(const Poco::UUID& taskId,
                                         const std::string& taskName,
                                         const Poco::Exception& exception):
    BaseTaskEventArgs(taskId, taskName, Poco::Task::TASK_RUNNING),
    _exception(exception)
{
}


TaskFailedEventArgs::~TaskFailedEventArgs()
{
}


const Poco::Exception& TaskFailedEventArgs::getException() const
{
    return _exception;
}


TaskProgressEventArgs::TaskProgressEventArgs(const Poco::UUID& taskId,
                                             const std::string& taskName,
                                             float progress):
    BaseTaskEventArgs(taskId, taskName, Poco::Task::TASK_RUNNING),
    _progress(progress)
{
}


TaskProgressEventArgs::~TaskProgressEventArgs()
{
}


float TaskProgressEventArgs::getProgress() const
{
    return _progress;
}


TaskProgress::TaskProgress(const Poco::UUID& taskId,
                           const std::string& name,
                           Poco::Task::TaskState state,
                           float progress,
                           const std::string& errorMessage):
    _taskId(taskId),
    _name(name),
    _state(state),
    _progress(progress)
{
}


TaskProgress::~TaskProgress()
{
}


void TaskProgress::update(const Poco::Task& task)
{
    _name = task.name();
    _state = task.state();
}


void TaskProgress::update(const BaseTaskEventArgs& args)
{
    _taskId = args.getTaskId();
    _name = args.getTaskName();
    _state = args.getState();
}


void TaskProgress::update(const TaskProgressEventArgs& args)
{
    _taskId = args.getTaskId();
    _name = args.getTaskName();
    _state = args.getState();
    _progress = args.getProgress();
}


void TaskProgress::update(const TaskFailedEventArgs& args)
{
    _taskId = args.getTaskId();
    _name = args.getTaskName();
    _state = args.getState();
    _errorMessage = args.getException().displayText();
}


const Poco::UUID& TaskProgress::getTaskId() const
{
    return _taskId;
}


void TaskProgress::setTaskId(const Poco::UUID& taskId)
{
    _taskId = taskId;
}


const std::string& TaskProgress::getName() const
{
    return _name;
}


void TaskProgress::setName(const std::string& name)
{
    _name = name;
}


Poco::Task::TaskState TaskProgress::getState() const
{
    return _state;
}


void TaskProgress::setState(Poco::Task::TaskState state)
{
    _state = state;
}


float TaskProgress::getProgress() const
{
    return _progress;
}


void TaskProgress::setProgress(float progress)
{
    _progress = progress;
}


const std::string& TaskProgress::getErrorMessage() const
{
    return _errorMessage;
}


void TaskProgress::setErrorMessage(const std::string& errorMessage)
{
    _errorMessage = errorMessage;
}


} // namespace ofx
