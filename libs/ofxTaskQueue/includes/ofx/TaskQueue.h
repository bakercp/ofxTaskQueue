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


#include <list>
#include <map>
#include "Poco/NotificationQueue.h"
#include "Poco/Observer.h"
#include "Poco/Task.h"
#include "Poco/TaskManager.h"
#include "Poco/UUID.h"
#include "ofEvents.h"
#include "ofx/TaskQueueEvents.h"
#include "Poco/UUIDGenerator.h"
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
/// \tparam DataType defines the custom data type to be sent with notifications.
template<typename TaskHandle>
class TaskQueue_
{
public:
    /// \brief A typedef for a task pointer.
	typedef Poco::AutoPtr<Poco::Task> TaskPtr;

    /// \brief A typedef for a task list.
	typedef std::list<TaskPtr> TaskList;

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
    /// releases and deletes it when it is finished.
    ///
    /// \param pTask a raw pointer to a task to be queued.
    TaskHandle start(const TaskHandle& taskID, Poco::Task* pTask);

    /// \brief Cancel a specific task by pointer.
    /// \param pTask The task pointer of the Task to cancel.
    void cancel(TaskPtr pTask);

    /// \brief Cancel a specific task by taskID.
    /// \param taskID The taskID of the Task to cancel.
    void cancel(const TaskHandle& taskID);

    /// \brief Request cancellation of all tasks, both queued and active.
    void cancelAll();

    bool inQueue(TaskPtr pTask) const;

    bool inQueue(const TaskHandle& taskID) const;


    /// \brief Get the name of a given task.
    /// \param taskID The id of the desired task.
    /// \returns A string with the task name.
    std::string getTaskName(const TaskHandle& taskID) const;

    /// \brief Waits for all active threads in the thread pool to complete.
    ///
    /// joinAll() will wait for ALL tasks in the TaskQueu's Poco::ThreadPool to
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
    template<class ListenerClass>
    void registerTaskProgressEvents(ListenerClass* listener);

    /// \brief Unregister event listeners.
    /// \tparam ListenerClass The class type with the required callback methods.
    /// \param listener a pointer to the listening class (usually "this").
    template<class ListenerClass>
    void unregisterTaskProgressEvents(ListenerClass* listener);

    /// \brief Event called when the Task is Queued.
    ofEvent<const TaskQueueEventArgs_<TaskHandle> > onTaskQueued;

    /// \brief Event called when the Task is started.
    ofEvent<const TaskQueueEventArgs_<TaskHandle> > onTaskStarted;

    /// \brief Event called when the Task is cancelled.
    ofEvent<const TaskQueueEventArgs_<TaskHandle> > onTaskCancelled;

    /// \brief Event called when the Task is finished.
    ofEvent<const TaskQueueEventArgs_<TaskHandle> > onTaskFinished;

    /// \brief Event called when the Task failed.
    ofEvent<const TaskFailedEventArgs_<TaskHandle> > onTaskFailed;

    /// \brief Event called when the Task reports its progress.
    ofEvent<const TaskProgressEventArgs_<TaskHandle> > onTaskProgress;

    /// \brief Event called when the Task sends an unhandled notification.
    ofEvent<const TaskCustomNotificationEventArgs_<TaskHandle> > onTaskCustomNotification;

    enum
    {
        /// \brief A value describing the maximum number of tasks.
        UNLIMITED_TASKS = -1
    };

protected:
    /// \brief Get a pointer to a Task given its taskID.
    /// \param taskID The the taskID search key.
    /// \return Return a pointer to the Task matching task or 0 if not found.
    TaskPtr getTaskPtr(const TaskHandle& taskID) const;

    /// \brief Get the taskID for a given Task.
    /// \param pTask The task search key.
    /// \return Return the unique taskID for the matching task or a NULL UUID.
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
                                              Poco::AutoPtr<Poco::TaskNotification> pNotification);

    /// \brief A typedef for a task list.
    typedef Poco::Observer<TaskQueue_, Poco::TaskNotification> TaskQueueObserver;

    /// \brief A typedef for a ForwardTaskMap.
    typedef std::map<TaskHandle, TaskPtr> IDTaskMap;

    /// \brief A typedef for a ReverseTaskMap.
    typedef std::map<TaskPtr, TaskHandle> TaskIDMap;

    /// \brief Update method callback.
    /// \param args The args pass with the update event.
    void update(ofEventArgs& args);

    /// \brief Attempt to submit a task to the task manager.
    /// \param pTask a pointer to the task to submit.
    /// \returns true iff the task was submitted successfully.
    bool startTask(TaskPtr pTask);

    /// \brief Handle notifications from the Notification queue.
    /// \param pNotification a pointer to the notification.
    void handleNotification(Poco::Notification::Ptr pNotification);

    ///  The underlying NotificationQueue will take ownership of the pointer.
    void onNotification(Poco::TaskNotification* pNf);

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

    /// \brief A NotificationQueue to distribute in the main thread.
    ///
    /// All notifications in the notification queue are handled during the
    /// update callback, allowing all events to be notified only from within
    /// the main thread.
    Poco::NotificationQueue _notifications;

    /// \brief The TaskManager is responsible for executing tasks in a thread.
    Poco::TaskManager _taskManager;

};


template<typename TaskHandle>
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


template<typename TaskHandle>
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


template<typename TaskHandle>
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


template<typename TaskHandle>
void TaskQueue_<TaskHandle>::update(ofEventArgs& args)
{
    // Try to start any queued tasks.
    TaskList::iterator queuedTasksIter = _queuedTasks.begin();

    while (queuedTasksIter != _queuedTasks.end())
    {
        try
        {
            if (_maximumTasks != UNLIMITED_TASKS &&
                _taskManager.count() >= _maximumTasks)
            {
                throw Poco::Exception("Maximum tasks exceeded.");
            }
            else
            {
                // We duplicate the task in order to share ownership and
                // preserve our own pointer references for taskID lookup etc.
                _taskManager.start((*queuedTasksIter).duplicate());
                _queuedTasks.erase(queuedTasksIter++); // If it was started, then remove.
            }
        }
        catch (const Poco::Exception& exc)
        {
            ofLogVerbose("TaskQueue_<TaskHandle>::start") << "Task queued. Reason: " << exc.displayText();
            (*queuedTasksIter)->reset();
            break;
        }
        catch (const std::exception& exc)
        {
            ofLogVerbose("TaskQueue_<TaskHandle>::start") << "Task queued. Reason: " << exc.what();
            (*queuedTasksIter)->reset();
            break;
        }
        catch (...)
        {
            ofLogVerbose("TaskQueue_<TaskHandle>::start") << "Task queued. Reason: Unknown";
            (*queuedTasksIter)->reset();
            break;
        }

        // Reset the state, progress and cancel status of the task, which was
        // modified when we attempted (and failed) to start the task with
        // _taskManager.start() in our try / catch block above.
    }

    // Process the notification queue.
    Poco::Notification::Ptr pNotification;

    do
    {
        // Immediately take ownership by assigning to AutoPtr.
        pNotification = _notifications.dequeueNotification();

        if (!pNotification.isNull())
        {
            handleNotification(pNotification);
        }
        else
        {
            break;
        }
    }
    while (!pNotification.isNull());

}


template<typename TaskHandle>
TaskHandle TaskQueue_<TaskHandle>::start(const TaskHandle& taskID, Poco::Task* pRawTask)
{
    // Take ownership immediately.
    Poco::AutoPtr<Poco::Task> pAutoTask(pRawTask);

    if (inQueue(taskID))
    {
        throw Poco::Exception("Already Queued");
    }

    // Add the task to the forward taskID / task map.
    _IDTaskMap[taskID] = pAutoTask;

    // Add the task to the reverse task / taskID map.
    _taskIDMap[pAutoTask] = taskID;

    // Queue our task for an immediate start on the next update call.
    _queuedTasks.push_back(pAutoTask);

    TaskQueueEventArgs_<TaskHandle> args(taskID,
                                         pAutoTask->name(),
                                         pAutoTask->state());

    ofNotifyEvent(onTaskQueued, args, this);

    return taskID;
}


template<typename TaskHandle>
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


template<typename TaskHandle>
void TaskQueue_<TaskHandle>::cancel(const TaskHandle& taskID)
{
    TaskPtr taskPtr = getTaskPtr(taskID);

    if (!taskPtr.isNull())
    {
        cancel(taskPtr);
    }
    else
    {
        ofLogFatalError("TaskQueue_<TaskHandle>::cancel") << "Unknown taskID: " << taskID.toString();
    }
}


template<typename TaskHandle>
void TaskQueue_<TaskHandle>::cancelAll()
{
    // Cancel all active tasks.
    _taskManager.cancelAll();

    // Try to start any queued tasks.
    Poco::TaskManager::TaskList::iterator iter = _queuedTasks.begin();

    while (iter != _queuedTasks.end())
    {
        // Then simulate a callbacks sent by the TaskManager.

        // First send a task cancelled notification.
        onNotification(new Poco::TaskCancelledNotification(*iter));

        /// Then send a task finished notification.
        onNotification(new Poco::TaskFinishedNotification(*iter));

        // Remove the unstarted task from the queue.
        _queuedTasks.erase(iter++);
    }
}


template<typename TaskHandle>
bool TaskQueue_<TaskHandle>::inQueue(TaskPtr pTask) const
{
    return _taskIDMap.find(pTask) != _taskIDMap.end();
}


template<typename TaskHandle>
bool TaskQueue_<TaskHandle>::inQueue(const TaskHandle& taskID) const
{
    return _IDTaskMap.find(taskID) != _IDTaskMap.end();
}


template<typename TaskHandle>
std::string TaskQueue_<TaskHandle>::getTaskName(const TaskHandle& taskID) const
{
    if (Poco::AutoPtr<Poco::Task> ptr = getTaskPtr(taskID))
    {
        return ptr->name();
    }
    else
    {
        // We already log a warning in getTaskPtr() in this case.
        return "";
    }
}


template<typename TaskHandle>
void TaskQueue_<TaskHandle>::onNotification(Poco::TaskNotification* pNf)
{
    // TODO: This is a hack because TaskManager::postNotification() breaks
    // AutoPtr in pre 1.4.4.  This is fixed in 1.4.4.
    // https://github.com/pocoproject/poco/blob/develop/Foundation/include/Poco/TaskManager.h#L104

#if POCO_VERSION > 0x01040300
    Poco::TaskNotification* p = pNf;
#else
    Poco::TaskNotification::Ptr p(pNf, true);
#endif

    _notifications.enqueueNotification(p);
}


template<typename TaskHandle>
void TaskQueue_<TaskHandle>::handleTaskCustomNotification(const TaskHandle& taskID,
                                                         Poco::AutoPtr<Poco::TaskNotification> pNotification)
{
    TaskCustomNotificationEventArgs_<TaskHandle> args(taskID,
                                                      pNotification->task()->name(),
                                                      pNotification->task()->state(),
                                                      pNotification->task()->progress(),
                                                      pNotification);

    ofNotifyEvent(onTaskCustomNotification, args, this);
}


template<typename TaskHandle>
void TaskQueue_<TaskHandle>::handleNotification(Poco::Notification::Ptr pNotification)
{
    Poco::AutoPtr<Poco::TaskNotification> pTaskNotification = 0;

    if (!(pTaskNotification = pNotification.cast<Poco::TaskNotification>()).isNull())
    {
        TaskHandle taskID;

        if (!(taskID = getTaskId(pTaskNotification->task())).isNull())
        {
            // Now determine what kind of task notification we have.
            Poco::AutoPtr<Poco::TaskStartedNotification> taskStarted = 0;
            Poco::AutoPtr<Poco::TaskCancelledNotification> taskCancelled = 0;
            Poco::AutoPtr<Poco::TaskFinishedNotification> taskFinished = 0;
            Poco::AutoPtr<Poco::TaskFailedNotification> taskFailed = 0;
            Poco::AutoPtr<Poco::TaskProgressNotification> taskProgress = 0;

            if (!(taskStarted = pTaskNotification.cast<Poco::TaskStartedNotification>()).isNull())
            {

            
                TaskProgressEventArgs_<TaskHandle> args(taskID,
                                                        pTaskNotification->task()->name(),
                                                        Poco::Task::TASK_STARTING,
                                                        pTaskNotification->task()->progress());

                ofNotifyEvent(onTaskStarted, args, this);
            }
            else if (!(taskCancelled = pTaskNotification.cast<Poco::TaskCancelledNotification>()).isNull())
            {
                // Here we force the
                TaskProgressEventArgs_<TaskHandle> args(taskID,
                                                        pTaskNotification->task()->name(),
                                                        Poco::Task::TASK_CANCELLING,
                                                        pTaskNotification->task()->progress());

                ofNotifyEvent(onTaskCancelled, args, this);
            }
            else if (!(taskFinished = pTaskNotification.cast<Poco::TaskFinishedNotification>()).isNull())
            {
                TaskProgressEventArgs_<TaskHandle> args(taskID,
                                                        pTaskNotification->task()->name(),
                                                        Poco::Task::TASK_FINISHED,
                                                        pTaskNotification->task()->progress());

                ofNotifyEvent(onTaskFinished, args, this);

                typename IDTaskMap::iterator iterForward = _IDTaskMap.find(taskID);

                if (iterForward != _IDTaskMap.end())
                {
                    typename TaskIDMap::iterator iterReverse = _taskIDMap.find(iterForward->second);

                    _IDTaskMap.erase(iterForward);

                    if (iterReverse != _taskIDMap.end())
                    {
                        _taskIDMap.erase(iterReverse);
                    }
                    else
                    {
                        ofLogFatalError("TaskQueue_<TaskHandle>::handleNotification") << "Unable to find reverseIter.";
                    }
                }
                else
                {
                    ofLogFatalError("TaskQueue_<TaskHandle>::handleNotification") << "Unable to find forwardIter.";
                }
            }
            else if (!(taskFailed = pTaskNotification.cast<Poco::TaskFailedNotification>()).isNull())
            {
                TaskFailedEventArgs args(taskID,
                                         pTaskNotification->task()->name(),
                                         pTaskNotification->task()->state(),
                                         taskFailed->reason());

                ofNotifyEvent(onTaskFailed, args, this);
            }
            else if (!(taskProgress = pTaskNotification.cast<Poco::TaskProgressNotification>()).isNull())
            {
                TaskProgressEventArgs args(taskID,
                                           pTaskNotification->task()->name(),
                                           pTaskNotification->task()->state(),
                                           taskProgress->progress());

                ofNotifyEvent(onTaskProgress, args, this);
            }
            else
            {
                handleTaskCustomNotification(taskID, pTaskNotification);
            }
        }
        else
        {
            ofLogFatalError("TaskQueue_<TaskHandle>::handleNotification") << "Missing TaskId.";
        }
    }
    else
    {
        ofLogFatalError("TaskQueue_<TaskHandle>::handleNotification") << "Unknown notification type: " << pNotification->name();
    }
}


template<typename TaskHandle>
bool TaskQueue_<TaskHandle>::startTask(TaskPtr pTask)
{
    try
    {
        // We duplicate the task in order to share ownership and
        // preserve our own pointer references for taskID lookup etc.
        _taskManager.start(pTask);
        return true;
    }
    catch (const Poco::Exception& exc)
    {
        ofLogVerbose("TaskQueue_<TaskHandle>::start") << "Task queued. Reason: " << exc.displayText();
    }
    catch (const std::exception& exc)
    {
        ofLogVerbose("TaskQueue_<TaskHandle>::start") << "Task queued. Reason: " << exc.what();
    }
    catch (...)
    {
        ofLogVerbose("TaskQueue_<TaskHandle>::start") << "Task queued. Reason: Unknown";
    }

    // Reset the state, progress and cancel status of the task, which was
    // modified when we attempted (and failed) to start the task with
    // _taskManager.start() in our try / catch block above.
    pTask->reset();
    
    // Return unsuccessful.  This will keep it in our task queue.
    return false;
}


template<typename TaskHandle>
TaskHandle TaskQueue_<TaskHandle>::getTaskId(const Poco::AutoPtr<Poco::Task>& pNf) const
{
    typename TaskIDMap::const_iterator iter = _taskIDMap.find(pNf);
    
    if (iter != _taskIDMap.end())
    {
        return iter->second;
    }
    else
    {
        throw Poco::Exception("Task does not exist.");
    }
}


template<typename TaskHandle>
void TaskQueue_<TaskHandle>::joinAll()
{
    _taskManager.joinAll();
}


template<typename TaskHandle>
std::size_t TaskQueue_<TaskHandle>::getActiveCount() const
{
    return _taskManager.count();
}


template<typename TaskHandle>
std::size_t TaskQueue_<TaskHandle>::getQueuedCount() const
{
    return _queuedTasks.size();
}


template<typename TaskHandle>
std::size_t TaskQueue_<TaskHandle>::getCount() const
{
    return _taskManager.count() + _queuedTasks.size();
}


template<typename TaskHandle>
int TaskQueue_<TaskHandle>::getMaximumTasks() const
{
    return _maximumTasks;
}


template<typename TaskHandle>
void TaskQueue_<TaskHandle>::setMaximumTasks(int maximumTasks)
{
    _maximumTasks = maximumTasks;
}


template<typename TaskHandle>
Poco::AutoPtr<Poco::Task> TaskQueue_<TaskHandle>::getTaskPtr(const TaskHandle& taskID) const
{
    typename IDTaskMap::const_iterator iter = _IDTaskMap.find(taskID);
    
    if (iter != _IDTaskMap.end())
    {
        return iter->second;
    }
    else
    {
        ofLogWarning("TaskQueue_<TaskHandle>::getTaskPtr") << "No task with id: " << taskID.toString();
        return 0;
    }
}


template<typename TaskHandle>
template<typename ListenerClass>
void TaskQueue_<TaskHandle>::registerTaskProgressEvents(ListenerClass* listener)
{
    ofAddListener(onTaskQueued, listener, &ListenerClass::onTaskQueued);
    ofAddListener(onTaskStarted, listener, &ListenerClass::onTaskStarted);
    ofAddListener(onTaskCancelled, listener, &ListenerClass::onTaskCancelled);
    ofAddListener(onTaskFinished, listener, &ListenerClass::onTaskFinished);
    ofAddListener(onTaskFailed, listener, &ListenerClass::onTaskFailed);
    ofAddListener(onTaskProgress, listener, &ListenerClass::onTaskProgress);
}


template<typename TaskHandle>
template<typename ListenerClass>
void TaskQueue_<TaskHandle>::unregisterTaskProgressEvents(ListenerClass* listener)
{
    ofRemoveListener(onTaskQueued, listener, &ListenerClass::onTaskQueued);
    ofRemoveListener(onTaskStarted, listener, &ListenerClass::onTaskStarted);
    ofRemoveListener(onTaskCancelled, listener, &ListenerClass::onTaskCancelled);
    ofRemoveListener(onTaskFinished, listener, &ListenerClass::onTaskFinished);
    ofRemoveListener(onTaskFailed, listener, &ListenerClass::onTaskFailed);
    ofRemoveListener(onTaskProgress, listener, &ListenerClass::onTaskProgress);
}


class TaskQueue: public TaskQueue_<Poco::UUID>
{
public:
    /// \brief Create a TaskQueue using the default ThreadPool.
    ///
    /// To modifiy the thread pool parameters, call
    TaskQueue(int maximumTasks = UNLIMITED_TASKS): TaskQueue_<Poco::UUID>(maximumTasks)
    {
    }

    /// \brief Create a TaskQueue using provided the default ThreadPool.
    /// \param threadPool The backing Poco::ThreadPool.
    TaskQueue(int maximumTasks, Poco::ThreadPool& threadPool):
        TaskQueue_<Poco::UUID>(maximumTasks, threadPool)
    {
    }

    /// \brief Destroy the TaskQueue.
    virtual ~TaskQueue()
    {
    }

    Poco::UUID start(Poco::Task* pRawTask)
    {
        // Generate a unique task id.
        std::size_t tryCount = 0;
        Poco::UUID taskID = generateUniqueTaskID(tryCount);

        return TaskQueue_<Poco::UUID>::start(taskID, pRawTask);
    }

protected:
    /// \brief Generate a unique taskID to return when starting a task.
    /// \param tryCount An tryCount to limit the recusions.
    /// \returns An unused TaskHandle.
    Poco::UUID generateUniqueTaskID(std::size_t& tryCount) const
    {
        ++tryCount;

        Poco::UUID uuid = Poco::UUIDGenerator::defaultGenerator().createOne();

        if (inQueue(uuid))
        {
            if (tryCount > 1)
            {
                ofLogFatalError("TaskQueue::generateUniqueTaskId") << "Duplicate UUID generated.";
            }

            return generateUniqueTaskID(tryCount);
        }
        else
        {
            return uuid;
        }
    }
    
};


} // namespace ofx
