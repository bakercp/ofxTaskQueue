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


#include "ofApp.h"


void ofApp::setup()
{
    ofEnableAlphaBlending();

    ofSetFrameRate(120);

    queue.registerTaskEvents(this);

    for (int i = 0; i < 10000; ++i)
    {
        std::string name = "Test Task #" + ofToString(i);
        Poco::UUID uuid = queue.start(new CountingTask(name, 100));

        // Add the task to our map.
        TaskProgress task;
        task.name = name;
        task.uuid = uuid;
        tasks[uuid] = task;
    }
}


void ofApp::update()
{
    std::map<Poco::UUID, TaskProgress>::iterator iter = tasks.begin();

    unsigned long long now = ofGetElapsedTimeMillis();

    while (iter != tasks.end())
    {
        if (iter->second.progress < 0 && now > iter->second.autoClearTime)
        {
            tasks.erase(iter++);
        }
        else
        {
            ++iter;
        }
    }
}


void ofApp::draw()
{
    ofBackground(0);

    std::map<Poco::UUID, TaskProgress>::iterator iter = tasks.begin();

    int y = 0;
    int height = 20;

    while (iter != tasks.end())
    {
        TaskProgress& t = (*iter).second;

        t.height = height;
        t.width = ofGetWidth();
        t.x = 0;
        t.y = y;

        t.draw();

        y += (height + 5);

        ++iter;
    }
}


void ofApp::keyPressed(int key)
{
    if ('s' == key)
    {
        // Sort queued tasks.
    }
    else if ('c' == key)
    {
        queue.cancelAll();
    }
    else if (' ' == key)
    {
        std::map<Poco::UUID, TaskProgress>::iterator iter = tasks.begin();

        while (iter != tasks.end())
        {
            if (iter->second.progress < 0)
            {
                tasks.erase(iter++); // If it was started, then remove.
            }
            else
            {
                ++iter;
            }
        }
    }
}


void ofApp::onTaskStarted(const ofx::TaskStartedEventArgs& args)
{
    if (tasks.find(args.getTaskId()) != tasks.end())
    {
        tasks[args.getTaskId()].progress = 0.000001; // Just give it a nudge.
    }
    else
    {
        ofLogFatalError("ofApp::onTaskCancelled") << "Unknown UUID.";
    }
}


void ofApp::onTaskCancelled(const ofx::TaskCancelledEventArgs& args)
{
    if (tasks.find(args.getTaskId()) != tasks.end())
    {
        tasks[args.getTaskId()].progress = -1;
    }
    else
    {
        ofLogFatalError("ofApp::onTaskCancelled") << "Unknown UUID.";
    }
}


void ofApp::onTaskFinished(const ofx::TaskFinishedEventArgs& args)
{
    std::map<Poco::UUID, TaskProgress>::iterator iter = tasks.find(args.getTaskId());

    if (tasks.find(args.getTaskId()) != tasks.end())
    {
        if (iter->second.progress < 0)
        {
            // There was an error, so let it be here for just a few seconds.
            tasks[args.getTaskId()].autoClearTime = ofGetElapsedTimeMillis() + 2000;
        }
        else
        {
            tasks.erase(iter);
        }
    }
    else
    {
        ofLogFatalError("ofApp::onTaskFinished") << "Unknown UUID.";
    }
}


void ofApp::onTaskFailed(const ofx::TaskFailedEventArgs& args)
{
    if (tasks.find(args.getTaskId()) != tasks.end())
    {
        tasks[args.getTaskId()].progress = -1;
        tasks[args.getTaskId()].message = args.getException().displayText();
    }
    else
    {
        ofLogFatalError("ofApp::onTaskFailed") << "Unknown UUID.";
    }
}


void ofApp::onTaskProgress(const ofx::TaskProgressEventArgs& args)
{
    if (tasks.find(args.getTaskId()) != tasks.end())
    {
        tasks[args.getTaskId()].progress = args.getProgress();
    }
    else
    {
        ofLogFatalError("ofApp::onTaskProgress") << "Unknown UUID.";
    }
}


void ofApp::onTaskData(const ofx::TaskDataEventArgs<std::string>& args)
{
    if (tasks.find(args.getTaskId()) != tasks.end())
    {
        tasks[args.getTaskId()].message = args.getData();
    }
    else
    {
        ofLogFatalError("ofApp::onTaskData") << "Unknown UUID.";
    }
}
