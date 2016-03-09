// =============================================================================
//
// Copyright (c) 2014-2016 Christopher Baker <http://christopherbaker.net>
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
#include "Poco/TaskNotification.h"
#include "Poco/Task.h"
#include "ofEvents.h"
#include "ofTypes.h"


namespace ofx {


/// \brief A base class for Task events.
/// \tparam The data type for the task handle.
template <typename TaskHandle>
class TaskQueueEventArgs_: public ofEventArgs
{
public:
    TaskQueueEventArgs_():
        _taskID(TaskHandle()),
        _taskName(""),
        _state(Poco::Task::TASK_IDLE)
    {
    }

    /// \brief Create a BaseTaskEventArgs.
    /// \param taskID The unique task id for the referenced Task.
    /// \param taskName The name of the referenced Task.
    /// \param state The state of the referenced task.
    TaskQueueEventArgs_(const TaskHandle& taskID,
                        const std::string& taskName,
                        Poco::Task::TaskState state):
        _taskID(taskID),
        _taskName(taskName),
        _state(state)
    {
    }

    /// \brief Destroy the BaseTaskEventArgs.
    virtual ~TaskQueueEventArgs_()
    {
    }

    /// \brief Get the task id.
    /// \returns the task id.
    const TaskHandle& getTaskId() const
    {
        return _taskID;
    }

    /// \brief Get the task name.
    /// \returns the task name.
    const std::string& getTaskName() const
    {
        return _taskName;
    }

    /// \brief Get the State of the task.
    /// \returns the State of the task.
    Poco::Task::TaskState getState() const
    {
        return _state;
    }

protected:
    /// \brief The unique task id for the referenced task.
    TaskHandle _taskID;

    /// \brief The name of the given task.
    std::string _taskName;

    /// \brief The Poco::Task::TaskState of the task.
    Poco::Task::TaskState _state;

};


/// \brief Event arguments for a Task failure event.
template <typename TaskHandle>
class TaskFailedEventArgs_: public TaskQueueEventArgs_<TaskHandle>
{
public:
    /// \brief Create a TaskFailedEventArgs.
    /// \param taskId The unique task id for the referenced Task.
    /// \param taskName The name of the referenced Task;
    /// \param exception The exception that caused the Task failure.
    TaskFailedEventArgs_(const TaskHandle& taskId,
                         const std::string& taskName,
                         Poco::Task::TaskState state,
                         const Poco::Exception& exception):
        TaskQueueEventArgs_<TaskHandle>(taskId, taskName, state),
        _exception(exception)
    {
    }

    /// \brief Destroy the TaskFailedEventArgs.
    virtual ~TaskFailedEventArgs_()
    {
    }

    /// \brief Get the exception.
    /// \returns the exception.
    const Poco::Exception& getException() const
    {
        return _exception;
    }

private:
    /// \brief The exception that caused the task failure.
    const Poco::Exception& _exception;

};


/// \brief Event arguments for a Task progress event.
template <typename TaskHandle>
class TaskProgressEventArgs_: public TaskQueueEventArgs_<TaskHandle>
{
public:
    TaskProgressEventArgs_():
        TaskQueueEventArgs_<TaskHandle>(),
        _progress(0)
    {
    }

    /// \brief Create a TaskProgressEventArgs.
    /// \param taskId The unique task id for the referenced task.
    /// \param taskName The name of the referenced Task;
    /// \param progress The current progress (0.0 - 1.0).
    TaskProgressEventArgs_(const TaskHandle& taskId,
                           const std::string& taskName,
                           Poco::Task::TaskState state,
                           float progress):
        TaskQueueEventArgs_<TaskHandle>(taskId, taskName, state),
        _progress(progress)
    {
    }


    /// \brief Destroy the TaskProgressEventArgs.
    virtual ~TaskProgressEventArgs_()
    {
    }

    /// \brief Get the current progress.
    /// \returns The current progress (0.0 - 1.0).
    float getProgress() const
    {
        return _progress;
    }

protected:
    /// \brief The Task's progress.
    float _progress;

};


/// \brief Event arguments for a Task failure event.
///
/// \tparam TaskHandle The task handle type.
/// \tparam DataType The custom event data type.
template <typename TaskHandle, typename DataType>
class TaskDataEventArgs_: public TaskProgressEventArgs_<TaskHandle>
{
public:
    /// \brief Create a TaskDataEventArgs.
    /// \param taskId The unique task id for the referenced task.
    /// \param data The custom event data.
    TaskDataEventArgs_(const TaskHandle& taskId,
                       const std::string& taskName,
                       Poco::Task::TaskState state,
                       float progress,
                       const DataType& data):
        TaskProgressEventArgs_<TaskHandle>(taskId, taskName, state, progress),
        _data(data)
    {
    }

    /// \brief Destroy the TaskDataEventArgs.
    virtual ~TaskDataEventArgs_()
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


template <typename TaskHandle>
class TaskCustomNotificationEventArgs_: public TaskProgressEventArgs_<TaskHandle>
{
public:
    /// \brief Create a TaskCustomNotificationEventArgs.
    /// \param taskId The unique task id for the referenced task.
    TaskCustomNotificationEventArgs_(const TaskHandle& taskId,
                                     const std::string& taskName,
                                     Poco::Task::TaskState state,
                                     float progress,
                                     Poco::TaskNotification::Ptr pNotification):
        TaskProgressEventArgs_<TaskHandle>(taskId,
                                           taskName,
                                           state,
                                           progress),
        _pNotification(pNotification)
    {
    }

    /// \brief Destroy the TaskCustomNotificationEventArgs_.
    virtual ~TaskCustomNotificationEventArgs_()
    {
    }

    /// \brief Get the custom task notification.
    /// \returns the custom task notification.
    Poco::Notification::Ptr getNotification() const
    {
        return _pNotification;
    }

    /// \brief A shortcut for extracting a copy of the data.
    ///
    /// To avoid copying the data, a custom data event must be generated.
    template <typename DataType>
    bool extract(DataType& data) const
    {
        Poco::AutoPtr<Poco::TaskCustomNotification<DataType>> taskCustomNotification = nullptr;

        if (!(taskCustomNotification = _pNotification.cast<Poco::TaskCustomNotification<DataType>>()).isNull())
        {
            data = taskCustomNotification->custom();
            return true;
        }
        else
        {
            return false;
        }
    }

protected:
    Poco::TaskNotification::Ptr _pNotification;
    
};


typedef TaskQueueEventArgs_<std::string> TaskQueueEventArgs;
typedef TaskProgressEventArgs_<std::string> TaskProgressEventArgs;
typedef TaskFailedEventArgs_<std::string> TaskFailedEventArgs;
typedef TaskCustomNotificationEventArgs_<std::string> TaskCustomNotificationEventArgs;


} // namespace ofx
