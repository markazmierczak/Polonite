.. _stp-branch-policy:

Branch policy
*************

Master branch
=============

The ``master`` branch is used for **active development**.
While pull requests are tested before merging, master may be broken from time to time.
When development on a new major release starts, master will also get API incompatible changes without warning.
For these reasons you should generally not depend on the master branch for building your project; use a release tag or a release maintenance branch instead.

Stable branch
=============

Once Polonite becomes stable enough a new branch will be created: an **evergreen** ``stable`` branch.

Development branches
====================

Development branches are used to aid parallel development between team members, ease tracking of features, and to assist in quickly fixing live production problems.

The different types of branches we may use are:

* ``new`` - feature branches
* ``bug`` - bug branches

Naming
------

Each new development branch should follow given convention:

   ``account-name/devel-type/descriptive-name``

For example:

   ``markazmierczak/new/audio-output``

Use descriptive names with lowercase and dashes.

Feature branches
----------------

Feature branches are used when developing a new feature or enhancement which has the potential of a development lifespan longer than a single deployment. When starting development, the deployment in which this feature will be released may not be known. No matter when the feature branch will be finished, it will always be merged back into the master branch.

Bug branches
------------

Bug branches differ from feature branches only semantically. Bug branches will be created when there is a bug on the live site that should be fixed and merged into the next deployment. For that reason, a bug branch typically will not last longer than one deployment cycle. Additionally, bug branches are used to explicitly track the difference between bug development and feature development. No matter when the bug branch will be finished, it will always be merged back into master.
