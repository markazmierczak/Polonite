Thread
******

Overview
========

This directory contains classes for thread management.

Threads
-------

The :class:`Thread` class is an owner of system level object which executes code in another thread. The :class:`Thread` object may refer to alive thread or not. The latter is denoted as `Dead-Thread`.

A new thread

**Thread Safety**

The Thread object cannot be accessed simultaneously from multiple threads. But serialized access (with mutual exclusion) from multiple threads is allowed, This enables scenario where the thread which spawned execution can delegate :func:`Thread::Join()` call to another thread.

Reference
=========

.. class:: Thread

.. function:: Thread::IsAlive()

   Returns whether thread started its work and has not been joined yet.
   This may return true even thread finished its work and is only waiting to be joined from other thread.

.. function:: void Thread::Detach()

.. function:: void Thread::Join()

   To call this function, the thread must be started already.

.. class:: ThisThread

.. function:: static void ThisThread::GetHandle()

   Returns the handle of current thread.

.. function:: static void ThisThread::Yield()

.. function:: static void ThisThread::SleepFor(TimeDelta duration)

.. function:: static void ThisThread::SleepUntil(TimeTicks end_time)

.. function:: static void ThisThread::Adopt()

   External threads, created outside of classes provided in this document need to be registered before used with certain functionality (:func:`ThisThread::AtExit` notably).

   It is OK to adopt single thread multiple times.

.. function:: static void ThisThread::AtExit(Function<void()> callback)
