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


#pragma once


#include "Poco/Exception.h"
#include "Poco/UUID.h"
#include "ofEvents.h"
#include "ofTypes.h"


namespace ofx {


/// \brief A base class for Task events.
class BaseTaskEventArgs: public ofEventArgs
{
public:
    /// \brief Create a BaseTaskEventArgs.
    /// \param taskId The unique task id for the referenced task.
    BaseTaskEventArgs(const Poco::UUID& taskId);

    /// \brief Destroy the BaseTaskEventArgs.
    virtual ~BaseTaskEventArgs();

    /// \brief Get the task id.
    /// \returns the task id.
    const Poco::UUID& getTaskId() const;

protected:
    /// \brief The unique task id for the referenced task.
    const Poco::UUID& _taskId;

};


typedef BaseTaskEventArgs TaskStartedEventArgs;
typedef BaseTaskEventArgs TaskCancelledEventArgs;
typedef BaseTaskEventArgs TaskFinishedEventArgs;


/// \brief Event arguments for a Task failure event.
class TaskFailedEventArgs: public BaseTaskEventArgs
{
public:
    /// \brief Create a TaskFailedEventArgs.
    /// \param taskId The unique task id for the referenced task.
    /// \param exception The exception that caused the Task failure.
    TaskFailedEventArgs(const Poco::UUID& taskId,
                        const Poco::Exception& exception);

    /// \brief Destroy the TaskFailedEventArgs.
    virtual ~TaskFailedEventArgs();

    /// \brief Get the exception.
    /// \returns the exception.
    const Poco::Exception& getException() const;

private:
    /// \brief The exception that caused the task failure.
    const Poco::Exception& _exception;

};


/// \brief Event arguments for a Task progress event.
class TaskProgressEventArgs: public BaseTaskEventArgs
{
public:
    /// \brief Create a TaskProgressEventArgs.
    /// \param taskId The unique task id for the referenced task.
    /// \param progress The current progress (0.0 - 1.0).
    TaskProgressEventArgs(const Poco::UUID& taskId, float progress);

    /// \brief Destroy the TaskProgressEventArgs.
    virtual ~TaskProgressEventArgs();

    /// \brief Get the current progress.
    /// \returns The current progress (0.0 - 1.0).
    float getProgress() const;

protected:
    /// \brief The Task's progress.
    float _progress;

};


/// \brief Event arguments for a Task failure event.
///
///
/// \tparam DataType The custom event data type.
template<typename DataType>
class TaskDataEventArgs: public BaseTaskEventArgs
{
public:
    /// \brief Create a TaskDataEventArgs.
    /// \param taskId The unique task id for the referenced task.
    /// \param data The custom event data.
    TaskDataEventArgs(const Poco::UUID& taskId,
                      const DataType& data):
        BaseTaskEventArgs(taskId),
        _data(data)
    {
    }

    /// \brief Destroy the TaskDataEventArgs.
    virtual ~TaskDataEventArgs()
    {
    }

    /// \brief Get the custom data type.
    /// \returns the custom data.
    const DataType& getData() const
    {
        return _data;
    }

protected:
    /// \brief A const reference to the custom data type sent with the event.
    const DataType& _data;

};


/// \brief A collection of TaskQueue events.
///
/// Clients can subscribe to these events through the TaskQeueue.
/// TaskQueueEvents are only called during the main thread, so the user can
/// be assured that their program data will be thread-safe during event
/// callbacks.  That said, users must take more care when defining and handling
/// custom data events.  Events are passed as const references in order to
/// prevent the user from modifying data in the source thread.
template<typename DataType>
class TaskQueueEvents
{
public:
    /// \brief Event called when the Task is started.
    ofEvent<const TaskStartedEventArgs> onTaskStarted;

    /// \brief Event called when the Task is cancelled.
    ofEvent<const TaskCancelledEventArgs> onTaskCancelled;

    /// \brief Event called when the Task is finished.
    ofEvent<const TaskFinishedEventArgs> onTaskFinished;

    /// \brief Event called when the Task failed.
    ofEvent<const TaskFailedEventArgs> onTaskFailed;

    /// \brief Event called when the Task reports its progress.
    ofEvent<const TaskProgressEventArgs> onTaskProgress;

    /// \brief Event called when the Task sends a custom data notification.
    ofEvent<const TaskDataEventArgs<DataType> > onTaskData;
    
};


} // namespace ofx
