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


#include "ofx/BaseTaskQueue.h"


namespace ofx {


class TaskQueue: public BaseTaskQueue<Poco::UUID>
{
public:
    /// \brief Create a TaskQueue using the default ThreadPool.
    ///
    /// To modifiy the thread pool parameters, call
    TaskQueue(int maximumTasks = UNLIMITED_TASKS);

    /// \brief Create a TaskQueue using provided the default ThreadPool.
    /// \param threadPool The backing Poco::ThreadPool.
    TaskQueue(int maximumTasks, Poco::ThreadPool& threadPool);

    /// \brief Destroy the TaskQueue.
    virtual ~TaskQueue();

protected:
    /// \brief Generate a unique taskID to return when starting a task.
    /// \param tryCount An tryCount to limit the recusions.
    /// \returns An unused TaskHandle.
//    TaskHandle generateUniqueTaskID(std::size_t& tryCount) const;

};

} // namespace ofx
