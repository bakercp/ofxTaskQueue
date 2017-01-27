ofxTaskQueue
============

## Description

TaskQueue executes and manages tasks in a multi-threaded environment.

## Features

The `TaskQueue` wraps a `Poco::TaskManager`, `Poco::NotificationQueue` and a `Poco::ThreadPool` to simplify the process of multi-threaded task execution. A "Task" extends `Poco::Task` and is best suited for relatively short-lived
tasks. That said, it simplifies the execution of long-running tasks by coalescing all thread callbacks into a notification queue that is distributed in FIFO order during the `update()` method, thus placing it in the main openFrameworks execution thread.

The `TaskQueue` makes effort to simplify the process of multi-threaded task execution by placing all Task event notifications within the main operating thread. This prevents the user from worrying about mutexes, thread locking and the associated complexities of accessing shared data from multiple threads. Additionally, by situating all event notifications in the main thread, the `TaskQueue` can be used to complete tasks that require execution in the primary execution thread (e.g. OpenGL calls, texture upload, etc).

When returning data in a custom event, the user is encouraged to design a system wherein a final piece of process is delivered, ready to be quickly consumed by the listener. For example, if the user schedules web-tasks, the socket stream should be consumed, parsed and packaged before delivery in order keep the main thread moving quickly. Passing "work" back to the main thread defeats the purpose of a multi-threaded queue.

`TaskQueue` has events for standard task callbacks, starting, cancellation, exceptions, progress and finishing. Additionally the user can define a custom "Data" callback that can be called at any time during thread execution. The user specifies the `DataType` class as a template parameter when creating the `TaskQueue`. This can be a base class. The user is responsible for handling special subclass behaviors.

Additionally, by overriding `handleUserNotifications` and adding additional events that extend BaseTaskEventArgs (just as TaskDataEventArgs currently does), users can handle an unlimited number of custom data types.

## Getting Started

To get started, generate the example project files using the openFrameworks [Project Generator](http://openframeworks.cc/learning/01_basics/how_to_add_addon_to_project/).

## Documentation

API documentation can be found here.

## Build Status

Linux, macOS [![Build Status](https://travis-ci.org/bakercp/ofxTaskQueue.svg?branch=master)](https://travis-ci.org/bakercp/ofxTaskQueue)

Visual Studio, MSYS [![Build status](https://ci.appveyor.com/api/projects/status/m85qm425sp177t5p/branch/master?svg=true)](https://ci.appveyor.com/project/bakercp/ofxtaskqueue/branch/master)

## Compatibility

The `stable` branch of this repository is meant to be compatible with the openFrameworks [stable branch](https://github.com/openframeworks/openFrameworks/tree/stable), which corresponds to the latest official openFrameworks release.

The `master` branch of this repository is meant to be compatible with the openFrameworks [master branch](https://github.com/openframeworks/openFrameworks/tree/master).

Some past openFrameworks releases are supported via tagged versions, but only `stable` and `master` branches are actively supported.

## Versioning

This project uses Semantic Versioning, although strict adherence will only come into effect at version 1.0.0.

## Licensing

See `LICENSE.md`.

## Contributing

Pull Requests are always welcome, so if you make any improvements please feel free to float them back upstream :)

1. Fork this repository.
2. Create your feature branch (`git checkout -b my-new-feature`).
3. Commit your changes (`git commit -am 'Add some feature'`).
4. Push to the branch (`git push origin my-new-feature`).
5. Create new Pull Request.
