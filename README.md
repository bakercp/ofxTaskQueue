ofxTaskQueue
============

TaskQueue executes and manages tasks in a multi-threaded environment.

The TaskQueue wraps a Poco::TaskManager, Poco::NotificationQueue and a
Poco::ThreadPool to simplify the process of multi-threaded task execution.
A "Task" extends Poco::Task and is best suited for relatively short-lived
tasks.  That said, it simplifies the execution of long-running tasks by
coalescing all thread callbacks into a notification queue that is
distributed in FIFO order during the update() method, thus placing it in
the main openFrameworks execution thread.

The TaskQueue makes effort to simplify the process of multi-threaded task
execution by placing all Task event notifications within the main operating
thread.  This prevents the user from worrying about mutexes, thread locking
and the associated complexities of accessing shared data from multiple
threads.  Additionally, by situating all event notifications in the main
thread, the TaskQueue can be used to complete tasks that require execution
in the primary execution thread (e.g. OpenGL calls, texture upload, etc).

When returning data in a custom event, the user is encouraged to design a
system wherein a final piece of process is delivered, ready to be quickly
consumed by the listener.  For example, if the user schedules web-tasks,
the socket stream should be consumed, parsed and packaged before delivery
in order keep the main thread moving quickly.  Passing "work" back to the
main thread defeats the purpose of a multi-threaded queue.

TaskQueue has events for standard task callbacks, starting, cancellation,
exceptions, progress and finishing.  Additionally the user can define a
custom "Data" callback that can be called at any time during thread
execution.  The user specifies the DataType class as a template parameter
when creating the TaskQueue.  This can be a base class.  The user is
responsible for handling special subclass behaviors.

Additionally, by overriding handleUserNotifications and adding additional
events that extend BaseTaskEventArgs (just as TaskDataEventArgs currently
does), users can handle an unlimited number of custom data types.
