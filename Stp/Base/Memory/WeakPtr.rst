.. Copyright 2017 Polonite Authors. All rights reserved.
   Copyright (c) 2012 The Chromium Authors. All rights reserved.
   Use of this source code is governed by a BSD-style license that can be
   found in the LICENSE file.

.. _stp-base-weak-ptr:

WeakPtr
=======

Weak pointers are pointers to an object that do not affect its lifetime, and which may be invalidated (i.e. reset to ``nullptr``) by the object, or its owner, at any time, most commonly when the object is about to be deleted.

The ``WeakPtr`` class holds a weak reference to ``T*``. This class is designed to be used like a normal pointer. You should always null-test an object of this class before using it or invoking a method that may result in the underlying object being destroyed::

  class Foo { ... };
  WeakPtr<Foo> foo;
  if (foo)
    foo->Method();

Weak pointers are useful when an object needs to be accessed safely by one or more objects other than its owner, and those callers can cope with the object vanishing and e.g. tasks posted to it being silently dropped. Reference-counting such an object would complicate the ownership graph and make it harder to reason about the object's lifetime::

  class Controller {
   public:
    Controller() : weak_factory_(this) {}
    void SpawnWorker() { Worker::StartNew(weak_factory_.getWeakPtr()); }
    void WorkComplete(const Result& result) { ... }
   private:
    // Member variables should appear before the WeakPtrFactory, to ensure
    // that any WeakPtrs to Controller are invalidated before its members
    // variable's destructors are executed, rendering them invalid.
    WeakPtrFactory<Controller> weak_factory_;
  };

  class Worker {
   public:
    static void StartNew(const WeakPtr<Controller>& controller) {
      Worker* worker = new Worker(controller);
      // Kick off asynchronous processing...
    }
   private:
    Worker(const WeakPtr<Controller>& controller)
        : controller_(controller) {}
    void DidCompleteAsynchronousProcessing(const Result& result) {
      if (controller_)
        controller_->WorkComplete(result);
    }
    WeakPtr<Controller> controller_;
  };

With this implementation a caller may use ``SpawnWorker()`` to dispatch multiple ``Worker``\ s and subsequently delete the Controller, without waiting for all Workers to have completed.

IMPORTANT: Thread-safety
------------------------

Weak pointers may be passed safely between threads, but must always be dereferenced and invalidated on the same thread; otherwise checking the pointer would be racey.

To ensure correct use, the first time a ``WeakPtr`` issued by a ``WeakPtrFactory`` is dereferenced, the factory and its ``WeakPtr``\ s become bound to the calling thread, and cannot be dereferenced or invalidated on any other threads. Bound ``WeakPtr``\ s can still be handed off to other threads, e.g. to use to post tasks back to object on the bound sequence.

If all ``WeakPtr`` objects are destroyed or invalidated then the factory is unbound from the thread. The ``WeakPtrFactory`` may then be destroyed, or new ``WeakPtr`` objects may be used, from a different sequence.

Thus, at least one ``WeakPtr`` object must exist and have been dereferenced on the correct thread to enforce that other ``WeakPtr`` objects will enforce they are used on the desired thread.

WeakPtrFactory
--------------

A class may be composed of a ``WeakPtrFactory`` and thereby control how it exposes weak pointers to itself. This is helpful if you only need weak pointers within the implementation of a class. This class is also  useful when working with primitive types. For example, you could have a
``WeakPtrFactory<bool>`` that is used to pass around a weak reference to a ``bool``.

SupportsWeakPtr
---------------

A class may extend from ``SupportsWeakPtr`` to let others take weak pointers to it. This avoids the class itself implementing boilerplate to dispense weak  pointers. However, since ``SupportsWeakPtr``\ 's destructor won't invalidate weak pointers to the class until after the derived class' members have been destroyed, its use can lead to subtle use-after-destroy issues.

asWeakPtr()
-----------

Helper function that uses type deduction to safely return a ``WeakPtr<Derived>`` when Derived doesn't directly extend ``SupportsWeakPtr<Derived>``, instead it extends a Base that extends ``SupportsWeakPtr<Base>`` ::

   class Base : public SupportsWeakPtr<Producer> {};
   class Derived : public Base {};

   Derived derived;
   WeakPtr<Derived> ptr = asWeakPtr(&derived);

Note that the following doesn't work (invalid type conversion) since ``Derived::asWeakPtr()`` is ``WeakPtr<Base> SupportsWeakPtr<Base>::asWeakPtr()``,  and there's no way to safely cast WeakPtr<Base> to WeakPtr<Derived> at the caller::

   WeakPtr<Derived> ptr = derived.asWeakPtr(); // Fails.
