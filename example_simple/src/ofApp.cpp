//
// Copyright (c) 2014 Christopher Baker <https://christopherbaker.net>
//
// SPDX-License-Identifier:	MIT
//


#include "ofApp.h"


void ofApp::setup()
{
    ofEnableAlphaBlending();
    ofSetFrameRate(60);

    for (int i = 0; i < 1000; ++i)
    {
        std::string name = "Counting Task #" + ofToString(i);
        queue.start(ofToString(i), new SimpleCountingTask(name, 100));
    }
}


void ofApp::draw()
{
    ofBackground(0);

    std::stringstream ss;

    ss << "Press 'c' to cancel all tasks." << std::endl;
    ss << "Press 'C' to cancel queued tasks." << std::endl;
    ss << "Press 'a' to add tasks." << std::endl;

    ofDrawBitmapStringHighlight(ss.str(), ofPoint(10, 14));

    ss.str("");
    ss << "Total Active Tasks: " << queue.getActiveCount() << std::endl;
    ss << "Total Queued Tasks: " << queue.getQueuedCount();

    ofDrawBitmapStringHighlight(ss.str(), ofPoint(ofGetWidth() / 2, 14));

    int height = 20;
    int y = height * 3;

    ofx::TaskQueue::ProgressMap progress = queue.getTaskProgress();
    ofx::TaskQueue::ProgressMap::const_iterator iter = progress.begin();

    while (iter != progress.end() && y < ofGetHeight())
    {
        const ofx::TaskProgressEventArgs& progressInfo = iter->second;

        float progress = progressInfo.progress();

        std::string taskId = progressInfo.taskId();

        std::string name = progressInfo.taskName();

        ofColor color;

        std::string statusString;

        switch (progressInfo.state())
        {
            case Poco::Task::TASK_IDLE:
                color = ofColor(127);
                statusString = "idle";
                break;
            case Poco::Task::TASK_STARTING:
                color = ofColor(255, 255, 0);
                statusString = "starting";
                break;
            case Poco::Task::TASK_RUNNING:
                color = ofColor(255, 127, 127);
                statusString = "running";
                break;
            case Poco::Task::TASK_CANCELLING:
                color = ofColor(255, 0, 0);
                statusString = "cancelling";
                break;
            case Poco::Task::TASK_FINISHED:
                color = ofColor(0, 255, 0);
                statusString = "finished";
                break;
        }

        std::stringstream ss;

        ss << taskId << ": " << statusString << " : " << name << std::endl;

        ofPushMatrix();
        ofTranslate(0, y);

        ofFill();
        ofSetColor(color, 127);
        ofDrawRectangle(0, 0, ofGetWidth() * progress, height - 2);

        ofNoFill();
        ofSetColor(color);
        ofDrawRectangle(0, 0, ofGetWidth() * progress, height - 2);

        ofFill();
        ofSetColor(255);
        ofDrawBitmapString(ss.str(), 4, 14);

        ofPopMatrix();

        y += height;

        ++iter;
    }
}


void ofApp::keyPressed(int key)
{
    if ('c' == key)
    {
        queue.cancelAll();
    }
    else if ('C' == key)
    {
        queue.cancelQueued();
    }
    else if ('a' == key)
    {
        queue.start(ofToString(ofRandom(1)), new SimpleCountingTask("User manually added!", 100));
    }
}

