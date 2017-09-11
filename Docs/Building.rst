.. _stp-building:

Building
********

Get the code
============

Clone the GIT repository (`git-lfs <https://git-lfs.github.com/>`_ is required).

Note that ``Polonite`` repository is a super module and merges multiple other modules. Your own project can be composed of modules you are interested in only (to minimize build time, executable size, dependencies and other factors).

Install dependencies
====================

Windows
-------

Install Visual Studio **2017 version 15.3** at least. Use the Custom install option and select:

* Visual C++
* Universal Windows Apps Development Tools
* Windows 10 SDK (10.0.15063.0)

``dbghelp.dll`` is required for debug builds for symbolizer.

If the Windows SDK was installed via the Visual Studio installer, the Debugging Tools can be installed by going to: Control Panel → Programs → Programs and Features → Select the "Windows Software Development Kit vXXXXX" → Change → Change → Check "Debugging Tools For Windows" → Change.

Linux
-----

Right now C++14 compliant compiler is required and Python for build process.

.. code-block:: console

   $ sudo apt install g++ binutils python pkg-config

At least ``5.1`` version is required for G++ compiler.

Download a Clang build from Google storage for ASan, MSan and other sanitizers:

.. code-block:: console

   $ python Stp/Build/Tools/Clang/Update.py

Build the Polonite
==================

Setting up the build
--------------------

Polonite uses `Ninja <https://ninja-build.org/>`_ as its main build tool along with a tool called `GN <https://chromium.googlesource.com/chromium/src/+/lkcr/tools/gn/docs/quick_start.md>`_ to generate ``.ninja`` files. You can create any number of build directories with different configurations. To create a build directory, run:

* Windows
   .. code-block:: console

      $ gn gen --ide=vs --sln=Polonite --winsdk=10.0.15063.0 Out/Debug

* Others
   .. code-block:: console

      $ gn gen Out/Debug

This command will generate ``.ninja`` files and skeletal solution for Visual Studio.

* You only have to run this once for each new build directory, Ninja will update the build files as needed.
* You can replace ``Debug`` with another name, but it should be a subdirectory of ``Out``.
* For other build arguments, including release settings, see :ref:`stp-build-configuration`. The default will be a debug component build matching the current host operating system and CPU.
* For more info on GN, run ``gn help`` on the command line or read the `quick start guide <https://chromium.googlesource.com/chromium/src/+/master/tools/gn/docs/quick_start.md>`_.

Perform actual build
--------------------

Build ``BaseUnitTests`` with Ninja using the command:

.. code-block:: console

   $ ninja -C Out/Debug BaseUnitTests

Build all targets with:

.. code-block:: console

   $ ninja -C Out/Debug

Run Base unit tests:

.. code-block:: console

   $ ./Out/Debug/BaseUnitTests.exe --gtest_filter="FlatMapTest*"

You can find out more about GoogleTest at its `GitHub page <https://github.com/google/googletest>`_.