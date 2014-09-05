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


#include "ofx/TaskQueue.h"
#include "Poco/UUIDGenerator.h"
#include "ofLog.h"


namespace ofx {



TaskQueue::TaskQueue(int maximumTasks):
    _maximumTasks(maximumTasks),
    _taskManager(Poco::ThreadPool::defaultPool())
{
    // Add the ofEvent().update listener.
    ofAddListener(ofEvents().update,
                  this,
                  &TaskQueue::update,
                  OF_EVENT_ORDER_APP);

    // Add this class as a TaskManager notification observer.
    _taskManager.addObserver(TaskQueueObserver(*this, &TaskQueue::onNotification));
}


TaskQueue::TaskQueue(int maximumTasks,
                     Poco::ThreadPool& pool):
    _maximumTasks(maximumTasks),
    _taskManager(pool)
{
    // Add the ofEvent().update listener.
    ofAddListener(ofEvents().update, this, &TaskQueue::update, OF_EVENT_ORDER_APP);

    // Add this class as a TaskManager notification observer.
    _taskManager.addObserver(TaskQueueObserver(*this, &TaskQueue::onNotification));
}


TaskQueue::~TaskQueue()
{
    // Remove the ofEvent().update listener.
    ofRemoveListener(ofEvents().update, this, &TaskQueue::update, OF_EVENT_ORDER_APP);

    // Cancel all tasks currently running.
    _taskManager.cancelAll();

    // Wait for all tasks to complete.
    _taskManager.joinAll();

    // Remove this class as a TaskManager notification observer.
    _taskManager.removeObserver(TaskQueueObserver(*this, &TaskQueue::onNotification));
}

void TaskQueue::update(ofEventArgs& args)
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
            ofLogVerbose("TaskQueue::start") << "Task queued. Reason: " << exc.displayText();
            (*queuedTasksIter)->reset();
            break;
        }
        catch (const std::exception& exc)
        {
            ofLogVerbose("TaskQueue::start") << "Task queued. Reason: " << exc.what();
            (*queuedTasksIter)->reset();
            break;
        }
        catch (...)
        {
            ofLogVerbose("TaskQueue::start") << "Task queued. Reason: Unknown";
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


Poco::UUID TaskQueue::start(Poco::Task* pRawTask)
{
    // Take ownership immediately.
    Poco::AutoPtr<Poco::Task> pAutoTask(pRawTask);

    // Generate a unique task id.
    std::size_t tryCount = 0;
    Poco::UUID taskID = generateUniqueTaskID(tryCount);

    // Add the task to the forward taskID / task map.
    _IDTaskMap[taskID] = pAutoTask;

    // Add the task to the reverse task / taskID map.
    _taskIDMap[pAutoTask] = taskID;

    // Queue our task for an immediate start on the next update call.
    _queuedTasks.push_back(pAutoTask);

    TaskQueuedEventArgs args(taskID,
                             pAutoTask->name(),
                             pAutoTask->state());

    ofNotifyEvent(onTaskQueued, args, this);

    return taskID;
}


void TaskQueue::cancel(TaskPtr taskPtr)
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
        ofLogWarning("TaskQueue::cancel") << "TaskPtr is NULL, no task cancelled.";
    }
}


void TaskQueue::cancel(const Poco::UUID& taskID)
{
    TaskPtr taskPtr = getTaskPtr(taskID);

    if (!taskPtr.isNull())
    {
        cancel(taskPtr);
    }
    else
    {
        ofLogFatalError("TaskQueue::cancel") << "Unknown taskID: " << taskID.toString();
    }
}


void TaskQueue::cancelAll()
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


std::string TaskQueue::getTaskName(const Poco::UUID& taskID) const
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


void TaskQueue::onNotification(Poco::TaskNotification* pNf)
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


void TaskQueue::handleTaskCustomNotification(const Poco::UUID& taskID,
                                             Poco::AutoPtr<Poco::TaskNotification> pNotification)
{
    TaskCustomNotificationEventArgs args(taskID,
                                         pNotification->task()->name(),
                                         pNotification->task()->state(),
                                         pNotification->task()->progress(),
                                         pNotification);

    ofNotifyEvent(onTaskCustomNotification, args, this);
}


void TaskQueue::handleNotification(Poco::Notification::Ptr pNotification)
{
    Poco::AutoPtr<Poco::TaskNotification> pTaskNotification = 0;

    if (!(pTaskNotification = pNotification.cast<Poco::TaskNotification>()).isNull())
    {
        Poco::UUID taskID;

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
                TaskStartedEventArgs args(taskID,
                                          pTaskNotification->task()->name(),
                                          Poco::Task::TASK_STARTING,
                                          pTaskNotification->task()->progress());

                ofNotifyEvent(onTaskStarted, args, this);
            }
            else if (!(taskCancelled = pTaskNotification.cast<Poco::TaskCancelledNotification>()).isNull())
            {
                // Here we force the
                TaskCancelledEventArgs args(taskID,
                                            pTaskNotification->task()->name(),
                                            Poco::Task::TASK_CANCELLING,
                                            pTaskNotification->task()->progress());

                ofNotifyEvent(onTaskCancelled, args, this);
            }
            else if (!(taskFinished = pTaskNotification.cast<Poco::TaskFinishedNotification>()).isNull())
            {
                TaskFinishedEventArgs args(taskID,
                                           pTaskNotification->task()->name(),
                                           Poco::Task::TASK_FINISHED,
                                           pTaskNotification->task()->progress());

                ofNotifyEvent(onTaskFinished, args, this);

                IdTaskMap::iterator iterForward = _IDTaskMap.find(taskID);

                if (iterForward != _IDTaskMap.end())
                {
                    TaskIdMap::iterator iterReverse = _taskIDMap.find(iterForward->second);

                    _IDTaskMap.erase(iterForward);

                    if (iterReverse != _taskIDMap.end())
                    {
                        _taskIDMap.erase(iterReverse);
                    }
                    else
                    {
                        ofLogFatalError("TaskQueue::handleNotification") << "Unable to find reverseIter.";
                    }
                }
                else
                {
                    ofLogFatalError("TaskQueue::handleNotification") << "Unable to find forwardIter.";
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
            ofLogFatalError("TaskQueue::handleNotification") << "Missing TaskId.";
        }
    }
    else
    {
        ofLogFatalError("TaskQueue::handleNotification") << "Unknown notification type: " << pNotification->name();
    }
}

bool TaskQueue::startTask(TaskPtr pTask)
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
        ofLogVerbose("TaskQueue::start") << "Task queued. Reason: " << exc.displayText();
    }
    catch (const std::exception& exc)
    {
        ofLogVerbose("TaskQueue::start") << "Task queued. Reason: " << exc.what();
    }
    catch (...)
    {
        ofLogVerbose("TaskQueue::start") << "Task queued. Reason: Unknown";
    }

    // Reset the state, progress and cancel status of the task, which was
    // modified when we attempted (and failed) to start the task with
    // _taskManager.start() in our try / catch block above.
    pTask->reset();
    
    // Return unsuccessful.  This will keep it in our task queue.
    return false;
}


Poco::UUID TaskQueue::getTaskId(const Poco::AutoPtr<Poco::Task>& pNf) const
{
    TaskIdMap::const_iterator iter = _taskIDMap.find(pNf);
    
    if (iter != _taskIDMap.end())
    {
        return iter->second;
    }
    else
    {
        return Poco::UUID::null();
    }
}


void TaskQueue::joinAll()
{
    _taskManager.joinAll();
}


std::size_t TaskQueue::getActiveCount() const
{
    return _taskManager.count();
}


std::size_t TaskQueue::getQueuedCount() const
{
    return _queuedTasks.size();
}


std::size_t TaskQueue::getCount() const
{
    return _taskManager.count() + _queuedTasks.size();
}


int TaskQueue::getMaximumTasks() const
{
    return _maximumTasks;
}


void TaskQueue::setMaximumTasks(int maximumTasks)
{
    _maximumTasks = maximumTasks;
}


Poco::UUID TaskQueue::generateUniqueTaskID(std::size_t& tryCount) const
{
    ++tryCount;
    
    Poco::UUID uuid = Poco::UUIDGenerator::defaultGenerator().createOne();
    
    if (_IDTaskMap.find(uuid) != _IDTaskMap.end())
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


Poco::AutoPtr<Poco::Task> TaskQueue::getTaskPtr(const Poco::UUID& taskID) const
{
    IdTaskMap::const_iterator iter = _IDTaskMap.find(taskID);
    
    if (iter != _IDTaskMap.end())
    {
        return iter->second;
    }
    else
    {
        ofLogWarning("TaskQueue::getTaskPtr") << "No task with id: " << taskID.toString();
        return 0;
    }
}

    
} // namespace ofx
