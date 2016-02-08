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


#pragma once


#include <list>
#include <map>
#include "Poco/NotificationQueue.h"
#include "Poco/NObserver.h"
#include "Poco/Task.h"
#include "Poco/TaskManager.h"
#include "Poco/ThreadPool.h"
#include "Poco/UUIDGenerator.h"
#include "Poco/Version.h"
#include "ofEvents.h"
#include "ofx/TaskQueueEvents.h"
#include "ofLog.h"


namespace ofx {


/// \brief TaskQueue executes and manages tasks in a multi-threaded environment.
///
/// The TaskQueue wraps a Poco::TaskManager, Poco::NotificationQueue and a
/// Poco::ThreadPool to simplify the process of multi-threaded task execution.
/// A "Task" extends Poco::Task and is best suited for relatively short-lived
/// tasks.  That said, it simplifies the execution of long-running tasks by
/// coalescing all thread callbacks into a notification queue that is
/// distributed in FIFO order during the update() method, thus placing it in
/// the main openFrameworks execution thread.
///
/// The TaskQueue makes effort to simplify the process of multi-threaded task
/// execution by placing all Task event notifications within the main operating
/// thread.  This prevents the user from worrying about mutexes, thread locking
/// and the associated complexities of accessing shared data from multiple
/// threads.  Additionally, by situating all event notifications in the main
/// thread, the TaskQueue can be used to complete tasks that require execution
/// in the primary execution thread (e.g. OpenGL calls, texture upload, etc).
///
/// When returning data in a custom event, the user is encouraged to design a
/// system wherein a final piece of process is delivered, ready to be quickly
/// consumed by the listener.  For example, if the user schedules web-tasks,
/// the socket stream should be consumed, parsed and packaged before delivery
/// in order keep the main thread moving quickly.  Passing "work" back to the
/// main thread defeats the purpose of a multi-threaded queue.
///
/// TaskQueue has events for standard task callbacks, starting, cancellation,
/// exceptions, progress and finishing.  Additionally the user can define a
/// custom "Data" callback that can be called at any time during thread
/// execution.  The user specifies the DataType class as a template parameter
/// when creating the TaskQueue.  This can be a base class.  The user is
/// responsible for handling special subclass behaviors.
///
/// Additionally, by overriding handleUserNotifications and adding additional
/// events that extend BaseTaskEventArgs (just as TaskDataEventArgs currently
/// does), users can handle an unlimited number of custom data types.
///
/// \tparam TaskHandle defines the custom data type to be sent with notifications.
template <typename TaskHandle>
class TaskQueue_
{
public:
    /// \brief A typedef for a task pointer.
	typedef Poco::AutoPtr<Poco::Task> TaskPtr;

    /// \brief A typedef for a task list.
	typedef std::list<TaskPtr> TaskList;

    /// \breif A typedef for task progress information.
    typedef std::map<TaskHandle, TaskProgressEventArgs_<TaskHandle>> ProgressMap;

    /// \brief Create a TaskQueue using the default ThreadPool.
    ///
    /// To modifiy the thread pool parameters, call
    TaskQueue_(int maximumTasks = UNLIMITED_TASKS);

    /// \brief Create a TaskQueue using provided the default ThreadPool.
    /// \param threadPool The backing Poco::ThreadPool.
    TaskQueue_(int maximumTasks, Poco::ThreadPool& threadPool);

    /// \brief Destroy the TaskQueue.
    virtual ~TaskQueue_();

    /// \brief Starts the given task in a thread obtained from the thread pool.
    ///
    /// The TaskManager immediately takes ownership of the Task object and
    /// releases and deletes it when it is finished.  It is recommended that
    /// users no longer interact directly with the task to preserve the task's
    /// data integrity.
    ///
    /// \param pTask a raw pointer to a task to be queued.
    /// \throws Poco::ExistsException if the taskID already exists.
    TaskHandle start(const TaskHandle& taskID, TaskPtr pTask);

    /// \brief Cancel a specific task by pointer.
    /// \param pTask The task pointer of the Task to cancel.
    void cancel(TaskPtr pTask);

    /// \brief Cancel a specific task by taskID.
    /// \param taskID The taskID of the Task to cancel.
    void cancel(const TaskHandle& taskID);

    /// \brief Cancel all tasks that have not yet started.
    void cancelQueued();

    /// \brief Request cancellation of all tasks, both queued and active.
    void cancelAll();

    /// \returns true iff the provided task exists.
    bool exists(TaskPtr pTask) const;

    /// \returns true if the provided task handle exists.
    bool exists(const TaskHandle& taskID) const;

    /// \brief Waits for all active threads in the thread pool to complete.
    ///
    /// joinAll() will wait for ALL tasks in the TaskQueue's Poco::ThreadPool to
    /// complete. If the Poco::ThreadPool has threads created by other
    /// facilities, these threads must also complete before joinAll() can
    /// return.
    void joinAll();

    /// \brief Get the number of Tasks currently running.
    /// \returns the number of Tasks currently running.
    std::size_t getActiveCount() const;

    /// \brief Get the number of Tasks queued to run.
    /// \returns the number of Tasks queued to run.
    std::size_t getQueuedCount() const;

    /// \brief Get the number of tasks under the control of the TaskQueue.
    ///
    /// This is equivalent to getActiveCount() + getQueuedCount();
    ///
    /// \returns the number of tasks under the control of the TaskQueue.
    std::size_t getCount() const;

    /// \brief Get a map of task progress.
    /// \returns a Task Progress map.
    const ProgressMap& getTaskProgress() const;

    /// \brief Get the maximum number of tasks.
    ///
    /// The maximum number of active tasks can be limited by setting the
    /// maximum tasks variable.  Be aware that this number may also be limited
    /// by the number of threads available in the thread pool.  If using the
    /// default thread pool, users can increase the number of threads by calling
    ///
    ///    Poco::ThreadPool::defaultPool().addCapacity(...)
    ///
    /// Alternatively, if the user is using thier own thread pool, capacity
    /// can be adjusted by modifying their referenced thread pool directly.
    ///
    /// \returns the maximum number of tasks or UNLIMITED_TASKS.
    int getMaximumTasks() const;

    /// \brief Set the maximum number of tasks.
    ///
    /// The maximum number of active tasks can be limited by setting the
    /// maximum tasks variable.  Be aware that this number may also be limited
    /// by the number of threads available in the thread pool.  If using the
    /// default thread pool, users can increase the number of threads by calling
    ///
    ///    Poco::ThreadPool::defaultPool().addCapacity(...)
    ///
    /// Alternatively, if the user is using thier own thread pool, capacity
    /// can be adjusted by modifying their referenced thread pool directly.
    ///
    /// For unlimited tasks, pass UNLIMITED_TASKS
    ///
    /// \param maximumTasks The maximum number of active tasks.
    void setMaximumTasks(int maximumTasks);

    /// \brief Register event listeners.
    /// \tparam ListenerClass The class type with the required callback methods.
    /// \param listener a pointer to the listening class (usually "this").
    /// \param priority the listener priority.
    template <class ListenerClass>
    void registerTaskProgressEvents(ListenerClass* listener,
                                    int priority = OF_EVENT_ORDER_AFTER_APP);

    /// \brief Unregister event listeners.
    /// \tparam ListenerClass The class type with the required callback methods.
    /// \param listener a pointer to the listening class (usually "this").
    /// \param priority the listener priority.
    template <class ListenerClass>
    void unregisterTaskProgressEvents(ListenerClass* listener,
                                      int priority = OF_EVENT_ORDER_AFTER_APP);

    /// \brief Event called when the Task is Queued.
    ofEvent<const TaskQueueEventArgs_<TaskHandle>> onTaskQueued;

    /// \brief Event called when the Task is started.
    ofEvent<const TaskQueueEventArgs_<TaskHandle>> onTaskStarted;

    /// \brief Event called when the Task is cancelled.
    ofEvent<const TaskQueueEventArgs_<TaskHandle>> onTaskCancelled;

    /// \brief Event called when the Task is finished.
    ofEvent<const TaskQueueEventArgs_<TaskHandle>> onTaskFinished;

    /// \brief Event called when the Task failed.
    ofEvent<const TaskFailedEventArgs_<TaskHandle>> onTaskFailed;

    /// \brief Event called when the Task reports its progress.
    ofEvent<const TaskProgressEventArgs_<TaskHandle>> onTaskProgress;

    /// \brief Event called when the Task sends an unhandled notification.
    ///
    /// To make best use of this event, subclasses should override
    /// handleTaskCustomNotification() and package the data for a custom event.
    ofEvent<const TaskCustomNotificationEventArgs_<TaskHandle>> onTaskCustomNotification;

    enum
    {
        /// \brief A value describing the maximum number of tasks.
        UNLIMITED_TASKS = -1
    };

protected:
    typedef Poco::AutoPtr<Poco::TaskNotification> TaskNotificationPtr;

    /// \brief Sort the queued tasks.
    ///
    /// Subclasses can implement a custom sorting scheme.
    /// No sorting is done by default.
    virtual void sortQueue()
    {
    }

    /// \brief Get a pointer to a Task given its taskID.
    /// \param taskID The the taskID search key.
    /// \return Return a pointer to the Task matching task or 0 if not found.
    /// \throws Poco::NotFoundException if the task cannot be found.
    TaskPtr getTaskPtr(const TaskHandle& taskID) const;

    /// \brief Get the taskID for a given Task.
    /// \param pTask The task search key.
    ///
    /// \warning If passed a raw task pointer (e.g. Poco::Task* p), the
    /// underlying Poco::AutoPtr will take ownership and decrement (and
    /// potentially delete) the wrapped object.  This can be avoided by passing
    /// a TaskPtr OR passing a duplicated (thus reference count incremented)
    /// ptr.
    ///
    /// \return Return the unique taskID for the matching task or a NULL UUID.
    /// \throws Poco::NotFoundException if the task cannot be found.
    TaskHandle getTaskId(const TaskPtr& pTask) const;

    /// \brief Handle all custom task notifications from the Notification queue.
    ///
    /// By default this method handles the custom DataType notifications.  If
    /// desired, the subclasses can override this method and add additional
    /// custom data event callbacks that extend BaseTaskEventArgs just as
    /// TaskDataEventArgs currently does.  The subclass must then host those
    /// additional events as a member variable.
    ///
    /// \param taskID the task UUID.
    /// \param pNotification a pointer to the notification.
    virtual void handleTaskCustomNotification(const TaskHandle& taskID,
                                              TaskNotificationPtr pNotification);

    /// \brief A typedef for a task list.
    typedef Poco::NObserver<TaskQueue_, Poco::TaskNotification> TaskQueueObserver;

    /// \brief A typedef for a ForwardTaskMap.
    typedef std::map<TaskHandle, TaskPtr> IDTaskMap;

    /// \brief A typedef for a ReverseTaskMap.
    typedef std::map<TaskPtr, TaskHandle> TaskIDMap;

    /// \brief Update method callback.
    /// \param args The args pass with the update event.
    void update(ofEventArgs& args);

    /// \brief Handle notifications from the Notification queue.
    /// \param pNotification a pointer to the notification.
    void handleNotification(Poco::Notification::Ptr pNotification);

    void onNotification(const TaskNotificationPtr& pNf);

    /// \brief The maximum number of simultaneous tasks.
    ///
    /// This number may also limited by the maximum size of the thread pool.
    int _maximumTasks;

    /// \brief A map of the taskID to the Task pointer.
    IDTaskMap _IDTaskMap;

    /// \brief A map of the Task pointer to the taskID.
    TaskIDMap _taskIDMap;

    /// \brief A list of tasks waiting to be submitted to the TaskManager.
    TaskList _queuedTasks;

    /// \brief A map of the taskID to the Task progress.
    ProgressMap _IDTaskProgressMap;

    /// \brief A NotificationQueue to distribute in the main thread.
    ///
    /// All notifications in the notification queue are handled during the
    /// update callback, allowing all events to be notified only from within
    /// the main thread.
    Poco::NotificationQueue _notifications;

    /// \brief The TaskManager is responsible for executing tasks in a thread.
    Poco::TaskManager _taskManager;

};


template <typename TaskHandle>
TaskQueue_<TaskHandle>::TaskQueue_(int maximumTasks):
    _maximumTasks(maximumTasks),
    _taskManager(Poco::ThreadPool::defaultPool())
{
    // Add the ofEvent().update listener.
    ofAddListener(ofEvents().update,
                  this,
                  &TaskQueue_<TaskHandle>::update,
                  OF_EVENT_ORDER_APP);

    // Add this class as a TaskManager notification observer.
    _taskManager.addObserver(TaskQueueObserver(*this, &TaskQueue_<TaskHandle>::onNotification));
}


template <typename TaskHandle>
TaskQueue_<TaskHandle>::TaskQueue_(int maximumTasks,
                                   Poco::ThreadPool& pool):
    _maximumTasks(maximumTasks),
    _taskManager(pool)
{
    // Add the ofEvent().update listener.
    ofAddListener(ofEvents().update, this, &TaskQueue_<TaskHandle>::update, OF_EVENT_ORDER_APP);

    // Add this class as a TaskManager notification observer.
    _taskManager.addObserver(TaskQueueObserver(*this, &TaskQueue_<TaskHandle>::onNotification));
}


template <typename TaskHandle>
TaskQueue_<TaskHandle>::~TaskQueue_()
{
    // Remove the ofEvent().update listener.
    ofRemoveListener(ofEvents().update, this, &TaskQueue_<TaskHandle>::update, OF_EVENT_ORDER_APP);

    // Cancel all tasks currently running.
    _taskManager.cancelAll();

    // Wait for all tasks to complete.
    _taskManager.joinAll();

    // Remove this class as a TaskManager notification observer.
    _taskManager.removeObserver(TaskQueueObserver(*this, &TaskQueue_<TaskHandle>::onNotification));
}


template <typename TaskHandle>
void TaskQueue_<TaskHandle>::update(ofEventArgs& args)
{
    // Sort the queue.
    sortQueue();

    // Try to start any queued tasks.
    TaskList::iterator queuedTasksIter = _queuedTasks.begin();

    while (queuedTasksIter != _queuedTasks.end() && (UNLIMITED_TASKS == _maximumTasks || _taskManager.count() < _maximumTasks))
    {
        try
        {
            // We duplicate the task in order to share ownership and
            // preserve our own pointer references for taskID lookup etc.
            // Poco::TaskManager expects a raw pointer and wraps it in a
            // Poco::AutoPtr, but since we already wrapped it in an
            // Poco::AutoPtr, we need to duplicate it so the task manager
            // doesn't delete it before we are finished with it.
            _taskManager.start((*queuedTasksIter).duplicate());

            // If it was started without an exception, then remove it from the queue.
            _queuedTasks.erase(queuedTasksIter++);
        }
        catch (...)
        {
            ofLogVerbose("TaskQueue_<TaskHandle>::start") << "Task start failure.";
            // Reset the state, progress and cancel status of the task, which was
            // modified when we attempted (and failed) to start the task with
            // _taskManager.start() in our try / catch block above.
            (*queuedTasksIter)->reset();

            // Break and attempt to queue them up next update.
            break;
        }
    }


    while (!_notifications.empty())
    {
        // Take ownership of the notification pointer.
        Poco::Notification::Ptr pNotification(_notifications.dequeueNotification());

        // Handle the notification
        //
        // This notification should have a refernce to the
        // task pointer until after the handle notification.
        handleNotification(pNotification);
    }
}


template <typename TaskHandle>
TaskHandle TaskQueue_<TaskHandle>::start(const TaskHandle& taskID, TaskPtr pAutoTask)
{
//    // Take ownership immediately.
//    // Reference count is not incremented.
//    // Reference count should still just be 1.
//    TaskPtr pAutoTask(pRawTask);

    if (exists(taskID))
    {
        throw Poco::ExistsException("TaskID already exists");
    }

    // Add the task to the forward taskID / task map.
    _IDTaskMap[taskID] = pAutoTask;

    // Add the task to the reverse task / taskID map.
    _taskIDMap[pAutoTask] = taskID;

    // Queue our task for an immediate start on the next update call.
    _queuedTasks.push_back(pAutoTask);

    // Update the progress
    _IDTaskProgressMap[taskID] = TaskProgressEventArgs_<TaskHandle>(taskID,
                                                                    pAutoTask->name(),
                                                                    pAutoTask->state(),
                                                                    pAutoTask->progress());

    TaskQueueEventArgs_<TaskHandle> args(taskID,
                                         pAutoTask->name(),
                                         pAutoTask->state());

    ofNotifyEvent(onTaskQueued, args, this);

    return taskID;
}


template <typename TaskHandle>
void TaskQueue_<TaskHandle>::cancel(TaskPtr taskPtr)
{
    if (!taskPtr.isNull())
    {
        // Send the cancel message to the task, even if it hasn't started.
        taskPtr->cancel();

        // Find the task if it is just queued.
        TaskList::iterator iter = std::find(_queuedTasks.begin(),
                                            _queuedTasks.end(),
                                            taskPtr);

        if (iter != _queuedTasks.end())
        {
            // Then simulate a callbacks sent by the TaskManager.

            // First send a task cancelled notification.
            onNotification(new Poco::TaskCancelledNotification(*iter));

            /// Then send a task finished notification.
            onNotification(new Poco::TaskFinishedNotification(*iter));

            // Remove the unstarted task from the queue.
            _queuedTasks.erase(iter);
        }
    }
    else
    {
        ofLogWarning("TaskQueue_<TaskHandle>::cancel") << "TaskPtr is NULL, no task cancelled.";
    }
}


template <typename TaskHandle>
void TaskQueue_<TaskHandle>::cancel(const TaskHandle& taskID)
{
    TaskPtr taskPtr = getTaskPtr(taskID);

    if (!taskPtr.isNull())
    {
        cancel(taskPtr);
    }
    else
    {
        ofLogFatalError("TaskQueue_<TaskHandle>::cancel") << "Unknown taskID: " << taskID;
    }
}


template <typename TaskHandle>
void TaskQueue_<TaskHandle>::cancelQueued()
{
    // Try to start any queued tasks.
    Poco::TaskManager::TaskList::iterator iter = _queuedTasks.begin();

    while (iter != _queuedTasks.end())
    {
        // Then simulate a callbacks sent by the TaskManager.

        // First send a task cancelled notification.
        onNotification(new Poco::TaskCancelledNotification(*iter));

        // Then send a task finished notification.
        // This notification will clear the maps map references.
        onNotification(new Poco::TaskFinishedNotification(*iter));

        // Remove the unstarted task from the queue.
        _queuedTasks.erase(iter++);
    }
}


template <typename TaskHandle>
void TaskQueue_<TaskHandle>::cancelAll()
{
    // Cancel all active tasks.
    _taskManager.cancelAll();

    cancelQueued();
}


template <typename TaskHandle>
bool TaskQueue_<TaskHandle>::exists(TaskPtr pTask) const
{
    return _taskIDMap.find(pTask) != _taskIDMap.end();
}


template <typename TaskHandle>
bool TaskQueue_<TaskHandle>::exists(const TaskHandle& taskID) const
{
    return _IDTaskMap.find(taskID) != _IDTaskMap.end();
}


template <typename TaskHandle>
void TaskQueue_<TaskHandle>::onNotification(const TaskNotificationPtr& pNf)
{
    // .enqueueNotification takes ownership of the pointer, so it is not
    // required to duplicated the pNf.
    _notifications.enqueueNotification(pNf);
}


template <typename TaskHandle>
void TaskQueue_<TaskHandle>::handleTaskCustomNotification(const TaskHandle& taskID,
                                                          TaskNotificationPtr pNotification)
{
    TaskCustomNotificationEventArgs_<TaskHandle> args(taskID,
                                                      pNotification->task()->name(),
                                                      pNotification->task()->state(),
                                                      pNotification->task()->progress(),
                                                      pNotification);

    _IDTaskProgressMap[taskID] = args;

    ofNotifyEvent(onTaskCustomNotification, args, this);
}


template <typename TaskHandle>
void TaskQueue_<TaskHandle>::handleNotification(Poco::Notification::Ptr pNotification)
{
    TaskNotificationPtr pTaskNotification = pNotification.cast<Poco::TaskNotification>();

    if (!pTaskNotification.isNull())
    {
        try
        {
            // We must wrap this task pointer so that it is not freed.
            // We treat it as a shared pointer, which duplicates it.
            TaskPtr pTask(pTaskNotification->task(), true);

            TaskHandle taskID = getTaskId(pTask);

            // Now determine what kind of task notification we have.
            Poco::AutoPtr<Poco::TaskStartedNotification> taskStarted = 0;
            Poco::AutoPtr<Poco::TaskCancelledNotification> taskCancelled = 0;
            Poco::AutoPtr<Poco::TaskFinishedNotification> taskFinished = 0;
            Poco::AutoPtr<Poco::TaskFailedNotification> taskFailed = 0;
            Poco::AutoPtr<Poco::TaskProgressNotification> taskProgress = 0;

            if (!(taskStarted = pTaskNotification.cast<Poco::TaskStartedNotification>()).isNull())
            {

                TaskProgressEventArgs_<TaskHandle> args(taskID,
                                                        pTask->name(),
                                                        Poco::Task::TASK_STARTING,
                                                        pTask->progress());

                _IDTaskProgressMap[taskID] = args;
                ofNotifyEvent(onTaskStarted, args, this);
            }
            else if (!(taskCancelled = pTaskNotification.cast<Poco::TaskCancelledNotification>()).isNull())
            {
                // Here we force the
                TaskProgressEventArgs_<TaskHandle> args(taskID,
                                                        pTask->name(),
                                                        Poco::Task::TASK_CANCELLING,
                                                        pTask->progress());

                _IDTaskProgressMap[taskID] = args;
                ofNotifyEvent(onTaskCancelled, args, this);
            }
            else if (!(taskFinished = pTaskNotification.cast<Poco::TaskFinishedNotification>()).isNull())
            {
                TaskProgressEventArgs_<TaskHandle> args(taskID,
                                                        pTask->name(),
                                                        Poco::Task::TASK_FINISHED,
                                                        pTask->progress());

                _IDTaskProgressMap[taskID] = args;
                ofNotifyEvent(onTaskFinished, args, this);
                _IDTaskProgressMap.erase(taskID);
                _IDTaskMap.erase(taskID);
                _taskIDMap.erase(pTask); // Must erase via pTask!

            }
            else if (!(taskFailed = pTaskNotification.cast<Poco::TaskFailedNotification>()).isNull())
            {
                TaskFailedEventArgs_<TaskHandle> args(taskID,
                                                      pTaskNotification->task()->name(),
                                                      pTaskNotification->task()->state(),
                                                      taskFailed->reason());

                float progress = _IDTaskProgressMap[taskID].getProgress();

                _IDTaskProgressMap[taskID] = TaskProgressEventArgs_<TaskHandle>(taskID,
                                                                                pTask->name(),
                                                                                pTask->state(),
                                                                                progress);

                ofNotifyEvent(onTaskFailed, args, this);

            }
            else if (!(taskProgress = pTaskNotification.cast<Poco::TaskProgressNotification>()).isNull())
            {
                TaskProgressEventArgs_<TaskHandle> args(taskID,
                                                        pTaskNotification->task()->name(),
                                                        pTaskNotification->task()->state(),
                                                        taskProgress->progress());
                _IDTaskProgressMap[taskID] = args;
                ofNotifyEvent(onTaskProgress, args, this);
            }
            else
            {
                handleTaskCustomNotification(taskID, pTaskNotification);
            }
        }
        catch (const Poco::NotFoundException& exc)
        {
            ofLogFatalError("TaskQueue_<TaskHandle>::handleNotification") << "Missing TaskId.";
        }
    }
    else
    {
        ofLogFatalError("TaskQueue_<TaskHandle>::handleNotification") << "Unknown notification type: " << pNotification->name();
    }
}


template <typename TaskHandle>
TaskHandle TaskQueue_<TaskHandle>::getTaskId(const TaskPtr& pNf) const
{
    typename TaskIDMap::const_iterator iter = _taskIDMap.find(pNf);
    
    if (iter != _taskIDMap.end())
    {
        return iter->second;
    }
    else
    {
        throw Poco::NotFoundException("TaskID Not found.");
    }
}


template <typename TaskHandle>
void TaskQueue_<TaskHandle>::joinAll()
{
    _taskManager.joinAll();
}


template <typename TaskHandle>
std::size_t TaskQueue_<TaskHandle>::getActiveCount() const
{
    return _taskManager.count();
}


template <typename TaskHandle>
std::size_t TaskQueue_<TaskHandle>::getQueuedCount() const
{
    return _queuedTasks.size();
}


template <typename TaskHandle>
std::size_t TaskQueue_<TaskHandle>::getCount() const
{
    return _taskManager.count() + _queuedTasks.size();
}


template <typename TaskHandle>
const typename TaskQueue_<TaskHandle>::ProgressMap& TaskQueue_<TaskHandle>::getTaskProgress() const
{
    return _IDTaskProgressMap;
}


template <typename TaskHandle>
int TaskQueue_<TaskHandle>::getMaximumTasks() const
{
    return _maximumTasks;
}


template <typename TaskHandle>
void TaskQueue_<TaskHandle>::setMaximumTasks(int maximumTasks)
{
    _maximumTasks = maximumTasks;
}


template <typename TaskHandle>
typename TaskQueue_<TaskHandle>::TaskPtr TaskQueue_<TaskHandle>::getTaskPtr(const TaskHandle& taskID) const
{
    typename IDTaskMap::const_iterator iter = _IDTaskMap.find(taskID);
    
    if (iter != _IDTaskMap.end())
    {
        return iter->second;
    }
    else
    {
        throw Poco::NotFoundException("TaskID Not found.");
    }
}


template <typename TaskHandle>
template <typename ListenerClass>
void TaskQueue_<TaskHandle>::registerTaskProgressEvents(ListenerClass* listener, int priority)
{
    ofAddListener(onTaskQueued, listener, &ListenerClass::onTaskQueued, priority);
    ofAddListener(onTaskStarted, listener, &ListenerClass::onTaskStarted, priority);
    ofAddListener(onTaskCancelled, listener, &ListenerClass::onTaskCancelled, priority);
    ofAddListener(onTaskFinished, listener, &ListenerClass::onTaskFinished, priority);
    ofAddListener(onTaskFailed, listener, &ListenerClass::onTaskFailed, priority);
    ofAddListener(onTaskProgress, listener, &ListenerClass::onTaskProgress, priority);
}


template <typename TaskHandle>
template <typename ListenerClass>
void TaskQueue_<TaskHandle>::unregisterTaskProgressEvents(ListenerClass* listener, int priority)
{
    ofRemoveListener(onTaskQueued, listener, &ListenerClass::onTaskQueued, priority);
    ofRemoveListener(onTaskStarted, listener, &ListenerClass::onTaskStarted, priority);
    ofRemoveListener(onTaskCancelled, listener, &ListenerClass::onTaskCancelled, priority);
    ofRemoveListener(onTaskFinished, listener, &ListenerClass::onTaskFinished, priority);
    ofRemoveListener(onTaskFailed, listener, &ListenerClass::onTaskFailed, priority);
    ofRemoveListener(onTaskProgress, listener, &ListenerClass::onTaskProgress, priority);
}


class TaskQueue: public TaskQueue_<std::string>
{
public:
    /// \brief Create a TaskQueue using the default ThreadPool.
    ///
    /// To modifiy the thread pool parameters, call
    TaskQueue(int maximumTasks = TaskQueue_<std::string>::UNLIMITED_TASKS):
        TaskQueue_<std::string>(maximumTasks)
    {
    }

    /// \brief Create a TaskQueue using provided the default ThreadPool.
    /// \param threadPool The backing Poco::ThreadPool.
    TaskQueue(int maximumTasks, Poco::ThreadPool& threadPool):
        TaskQueue_<std::string>(maximumTasks, threadPool)
    {
    }

    /// \brief Destroy the TaskQueue.
    virtual ~TaskQueue()
    {
    }

    std::string start(Poco::Task* pRawTask)
    {
        return TaskQueue_<std::string>::start(Poco::UUIDGenerator::defaultGenerator().createOne().toString(), pRawTask);
    }

};


} // namespace ofx
