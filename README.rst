Polonite
********

.. toctree::
   :hidden:

   AUTHORS.rst
   CONTRIBUTING.rst
   LICENSE.rst
   SPONSORS.rst
   Docs/BranchPolicy.rst
   Docs/Building.rst
   Docs/ContributionLicense.rst
   Docs/CppCodeStyle.rst
   Docs/Roadmap.rst
   Docs/StdLibrary.rst

   Stp/Build/Docs/BuildConfiguration.rst
   Stp/Base/README.rst

Overview
========

Polonite is a set of modules for building cross-platform C++ applications with usability and efficiency in mind.

Right now Polonite comes with ``Base`` and ``Build`` modules only, but plans are to extend framework with other modules, like ``Ui``, ``Audio``, ``Net``, etc. See :ref:`stp-roadmap` for more.

Getting Started
===============

The best is to see what features are included in :ref:`stp-base`. Also see :ref:`stp-building` to know how to play with it.

Goals
=====

The most prominent goal of this library is to provide a single place for common code used in native applications otherwise repeatedly written as part of other C++ projects. In common sense we want for a C++ developer to not reinvent a wheel or to connect multiple mini libraries into a one. Moreover, whole library is written in **uniform style** with readability in mind.

Design and implement abstractions of platform-dependent objects like threads, debugging symbols, processor intrinsics and others. Develop code once, build and deploy for each platform, but at the same time do **not** stick to least common denominator when relevant.

Aside wide gamut of built-in classes Polonite comes with `GN <https://chromium.googlesource.com/chromium/src/+/master/tools/gn/README.md>`_ build system which simplifies build process on many platforms/compilers, different configurations (debug/sanitizers/feature set/...).

Permissive license, iterative development with fast pace, community-driven.

No PImpl model, ABI can change. Right now API is changing frequently. Once maturity is reached, API may change between releases if required.

Support
=======

Developing Polonite takes a lot of time and effort: adding new features, improving existing ones, resolving bugs. Of course all of this is made with the love to the project.

However, if you found the Polonite project useful or made some income of using it, please support as financially. There are number of possibilities:

* A monthly contribution via `Patreon <https://www.patreon.com/markazmierczak>`_.
* A single donation via `PayPal <https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=JFTXFG6TKD8DU>`_.

All of this help contributes towards future development.

Contributing
============

See :ref:`CONTRIBUTING.rst <stp-contributing>`.

Copyright and license
=====================

See :ref:`AUTHORS.rst <stp-authors>` and :ref:`LICENSE.rst <stp-license>`.

Standard C++ Library
====================

Among notable rivals of this library is the Standard C++ Library.

.. note:: ``Stp`` for main directory and namespace has been chosen on purpose to mimic ``std`` with *d* letter rotated. ``Stp`` is a short for *Source Tree of Polonite*.

There are several valid reasons why Polonite does not use the standard library, not limited to:

* There are several implementations out there, each with its own bugs and quirks. Each of them has different coverage of the standard. Moreover, the release of new version of standard is tied with new standard for the language. This results in slow development pace (on both sides) and difficulties for cross-platform coverage.

* Polonite provides concepts incompatible with standard library.

* Several equivalents differ in functionality and performance.

Some of the concepts may be back-ported to the Standard C++ Library in future.

Before making any opinion or judgement, please give it a try.

For more details on the topic see :ref:`stp-docs-std-library`.
