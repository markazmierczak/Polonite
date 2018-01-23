.. _stp-build-configuration:

Build Configuration
*******************

Understanding GN build flags
============================

Recall that in GN, you pick your own build directory. These should generally go in a subdirectory of ``Out``. You set build arguments on a build directory by typing:

.. code-block:: console

   $ gn args Out/MyBuild

This will bring up an editor. The format of the stuff in the args file is just GN code, so set variables according to normal GN syntax (true/false for booleans, double-quotes for string values, # for comments). When you close the editor, a build will be made in that directory. To change the arguments for an existing build, just re-run this command on the existing directory.

You can get a list of all available build arguments for a given build directory, with documentation, by typing:

.. code-block:: console

   $ gn args Out/MyBuild --list

To get documentation for a single flag (in this example, ``is_component_build``):

.. code-block:: console

   $ gn args Out/MyBuild --list=is_component_build

.. note:: "GN args" as used on this page are not the command line arguments passed to GN. They refer to the individual variables that are passed as part of the ``--args`` command line flag and/or written to the ``args.gn`` file.

Common build variants
=====================

Release build
-------------

The default GN build is debug. To do a release build:

.. code-block:: js

   is_debug = false

Component build
---------------

The component build links many parts of your application into separate shared libraries to avoid the long link step at the end. It is the default when compiling debug (non-iOS) builds and most developers use this mode for everyday builds and debugging. Startup is slower and some linker optimizations won't work, so don't do benchmarks in this mode. Some people like to turn it on for release builds to get both faster links and reasonable runtime performance.

.. code-block:: js

   is_component_build = true
