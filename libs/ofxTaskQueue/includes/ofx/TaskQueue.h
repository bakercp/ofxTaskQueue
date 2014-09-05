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
class TaskQueue
{
public:
    /// \brief A typedef for a task pointer.
	typedef Poco::AutoPtr<Poco::Task> TaskPtr;

    /// \brief A typedef for a task list.
	typedef std::list<TaskPtr> TaskList;

    /// \brief Create a TaskQueue using the default ThreadPool.
    ///
    /// To modifiy the thread pool parameters, call
    TaskQueue(int maximumTasks = UNLIMITED_TASKS);

    /// \brief Create a TaskQueue using provided the default ThreadPool.
    /// \param threadPool The backing Poco::ThreadPool.
    TaskQueue(int maximumTasks, Poco::ThreadPool& threadPool);

    /// \brief Destroy the TaskQueue.
    virtual ~TaskQueue();

    /// \brief Starts the given task in a thread obtained from the thread pool.
    ///
    /// The TaskManager immediately takes ownership of the Task object and
    /// releases and deletes it when it is finished.
    ///
    /// \param pTask a raw pointer to a task to be queued.
    Poco::UUID start(Poco::Task* pTask);

    /// \brief Cancel a specific task by pointer.
    /// \param pTask The task pointer of the Task to cancel.
    void cancel(TaskPtr pTask);

    /// \brief Cancel a specific task by taskID.
    /// \param taskID The taskID of the Task to cancel.
    void cancel(const Poco::UUID& taskID);

    /// \brief Request cancellation of all tasks, both queued and active.
    void cancelAll();

    /// \brief Get the name of a given task.
    /// \param taskID The id of the desired task.
    /// \returns A string with the task name.
    std::string getTaskName(const Poco::UUID& taskID) const;

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
    ofEvent<const TaskQueuedEventArgs> onTaskQueued;

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

    /// \brief Event called when the Task sends an unhandled notification.
    ofEvent<const TaskCustomNotificationEventArgs> onTaskCustomNotification;

    enum
    {
        /// \brief A value describing the maximum number of tasks.
        UNLIMITED_TASKS = -1
    };

protected:
    /// \brief Generate a unique taskID to return when starting a task.
    /// \param tryCount An tryCount to limit the recusions.
    /// \returns An unused Poco::UUID.
    Poco::UUID generateUniqueTaskID(std::size_t& tryCount) const;

    /// \brief Get a pointer to a Task given its taskID.
    /// \param taskID The the taskID search key.
    /// \return Return a pointer to the Task matching task or 0 if not found.
    TaskPtr getTaskPtr(const Poco::UUID& taskID) const;

    /// \brief Get the taskID for a given Task.
    /// \param pTask The task search key.
    /// \return Return the unique taskID for the matching task or a NULL UUID.
    Poco::UUID getTaskId(const TaskPtr& pTask) const;

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
    virtual void handleTaskCustomNotification(const Poco::UUID& taskID,
                                              Poco::AutoPtr<Poco::TaskNotification> pNotification);

private:
    /// \brief A typedef for a task list.
    typedef Poco::Observer<TaskQueue, Poco::TaskNotification> TaskQueueObserver;

    /// \brief A typedef for a ForwardTaskMap.
    typedef std::map<Poco::UUID, TaskPtr> IdTaskMap;

    /// \brief A typedef for a ReverseTaskMap.
    typedef std::map<TaskPtr, Poco::UUID> TaskIdMap;

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
    IdTaskMap _IDTaskMap;

    /// \brief A map of the Task pointer to the taskID.
    TaskIdMap _taskIDMap;

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


template<typename ListenerClass>
void TaskQueue::registerTaskProgressEvents(ListenerClass* listener)
{
    ofAddListener(onTaskQueued, listener, &ListenerClass::onTaskQueued);
    ofAddListener(onTaskStarted, listener, &ListenerClass::onTaskStarted);
    ofAddListener(onTaskCancelled, listener, &ListenerClass::onTaskCancelled);
    ofAddListener(onTaskFinished, listener, &ListenerClass::onTaskFinished);
    ofAddListener(onTaskFailed, listener, &ListenerClass::onTaskFailed);
    ofAddListener(onTaskProgress, listener, &ListenerClass::onTaskProgress);
}


template<typename ListenerClass>
void TaskQueue::unregisterTaskProgressEvents(ListenerClass* listener)
{
    ofRemoveListener(onTaskQueued, listener, &ListenerClass::onTaskQueued);
    ofRemoveListener(onTaskStarted, listener, &ListenerClass::onTaskStarted);
    ofRemoveListener(onTaskCancelled, listener, &ListenerClass::onTaskCancelled);
    ofRemoveListener(onTaskFinished, listener, &ListenerClass::onTaskFinished);
    ofRemoveListener(onTaskFailed, listener, &ListenerClass::onTaskFailed);
    ofRemoveListener(onTaskProgress, listener, &ListenerClass::onTaskProgress);
}


} // namespace ofx
