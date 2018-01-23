.. _stp-base-call:

Call
****

Delegate.h
==========

Delegates devirtualize pointer-to-member-function and provides interface similar to ``std::function`` (use ``MakeDelegate()`` instead of ``std::bind``).

Example::

  void SetHandler(Delegate<int(String)>);

  class Handler {
   public:
    virtual int OnCall(String str) = 0;
  };

  void Register(Handler* handler) {
    SetHandler(MakeDelegate(&Handler::OnCall, handler));
  }

Delegates cannot be initialized with lambdas, standalone functions or capture any values for arguments. A delegate holds a function pointer and ``this`` pointer only (possibly altered through devirtualization).

Delegates are much faster than std::function because they are not allocating any memory and have direct function pointer which is called like any standalone (static) function. And most importantly they are small (only two words) - cache friendly.

A ``Delegate`` class is internally nullable. A null delegate cannot be invoked. Check ``Delegate::IsNull()`` for null value.

.. todo:: Explain the differences between compilers/ABIs in virtual function handling.

ObserverList.h
==============

A container for a list of observers. Unlike a normal ``List<T>``, this container can be modified during iteration without invalidating the iterator.
So, it safely handles the case of an observer removing itself or other observers from the list while observers are being notified.

Example::

    class MyWidget {
     public:
      ...

      class Observer {
       public:
        virtual void OnFoo(MyWidget* w) = 0;
        virtual void OnBar(MyWidget* w, int x, int y) = 0;
      };

      void AddObserver(Observer* obs) {
        observer_list_.AddObserver(obs);
      }

      void RemoveObserver(Observer* obs) {
        observer_list_.RemoveObserver(obs);
      }

      void NotifyFoo() {
        FOR_EACH_OBSERVER(Observer, observer_list_, OnFoo(this));
      }

      void NotifyBar(int x, int y) {
        FOR_EACH_OBSERVER(Observer, observer_list_, OnBar(this, x, y));
      }

     private:
      ObserverList<Observer> observer_list_;
    };
