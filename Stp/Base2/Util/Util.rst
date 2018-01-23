Utilities
*********

:class:`LazyInstance<T>`
========================

The :class:`LazyInstance` class manages a single instance of ``T``, which will be lazily created on the first time it's accessed.
This class is  useful for places you would normally use a function-level static, but you need to have guaranteed thread-safety.

The ``T`` constructor will only ever be called once, even if two threads are racing to create the object. :func:`LazyInstance<T>::Pointer` will always return the same, completely initialized instance.

When the instance is constructed it is registered with :class:`AtExitManager`. The destructor will be called on program exit.

:class:`LazyInstance` is completely thread safe, assuming that you create it safely. The class was designed to be POD initialized, so it shouldn't require a static constructor. It really only makes sense to declare a LazyInstance as a global variable using the ``LAZY_INSTANCE_INITIALIZER`` initializer.

LazyInstance also pre-allocates the space for ``T``, as to avoid allocating the instance on the heap. This may help with the performance of creating the instance, and reducing heap fragmentation. This requires that ``T`` be a complete type so we can determine the size.

.. todo:: DestroyAtExit and LeakAtExit

Example::

   static LazyInstance<MyClass>::DestroyAtExit my_instance = LAZY_INSTANCE_INITIALIZER;

   void SomeMethod() {
     my_instance->SomeMethod(); // MyClass::SomeMethod()

     MyClass* ptr = my_instance.Pointer();
     ptr->DoDoDo(); // MyClass::DoDoDo
   }

``operator->`` and ``operator*`` are provided for convenience.

To detect whether an instance is already created compare it with ``nullptr`` (directly, without calling :func:`LazyInstance<T>::Pointer`).

Many resource types are collected by operating system at process exit with ease. For such resources it is best to not destroy them explicitly. With :class:`LazyInstance<T>` one must declare the type of operation at exit:

* ``DestroyAtExit`` - destroys the object at process exit
* ``LeakAtExit`` - do not call destructor at process exit

``CallOnce``
============

:class:`CallOnce` is similar to :class:`LazyInstance<T>` but less armed. Its sole purpose is to ensure the function will be executed only once at call site::

   static CallOnce g_once = CALL_ONCE_INITIALIZER;
   static int g_total_memory;

   g_once([&]() {
     g_total_memory = GetSystemMemory();
   });

Reference
=========

.. class:: template<typename T> LazyInstance

.. function:: T* LazyInstance<T>::Pointer()