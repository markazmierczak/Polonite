.. _stp-contributing:

Contributing to the Polonite
****************************

Contribution Copyrights and Licensing
=====================================

Polonite contributors retain the copyright to their contribution and agree to irrevocably license the contribution under the :ref:`Polonite Contribution License <stp-contribution-license>` (MIT).

To report bugs or request features
==================================

Use GitHub issues to report bugs or request features.

To make a code contribution
===========================

Please be patient!

   * Your contribution will gladly be reviewed, but free time is an expensive resource.

   * Discuss your future contribution with us before coding it (preferably through GitHub issue). Let's avoid duplicate work.

Workflow
--------

#. Please read following documents before making a contribution:

   * :ref:`stp-cpp-code-style`
   * :ref:`stp-branch-policy`

#. **Fork** the repository

#. Create a **branch**

   * Use a well named topic branch for the changes.
   * Fork off the ``master`` branch. Avoid forking from repository work branches as they are frequently rebased.
   * Your branch should have a consistent logical scope. If the branch does several independent things (like adding a feature and fixing some unrelated repo scripts), use separate branches.

#. Add **commit**\ s

   * Commits create a transparent history of your work that others can follow to understand what you've done and why.

   * Each commit has an associated commit message, which is a description explaining why a particular change was made.

   * Furthermore, each commit is considered a separate unit of change. This lets you roll back changes if a bug is found, or if you decide to head in a different direction.

#. **Test** your changes as thoroughly as possible.

#. Add yourself to the end of the **author list** in :ref:`AUTHORS.rst <stp-authors>` if you're not already on the list. By doing this you **confirm** that:

   * You own the rights to the contribution, or have the legal right to license the contribution under Polonite :ref:`License <stp-license>` on behalf of the copyright owner(s).

   * You, or the copyright owner(s), agree to irrevocably license your contribution under Polonite :ref:`License <stp-license>`.

   * Please include an e-mail address, a link to your GitHub profile, or something similar to allow your contribution to be identified accurately.

#. Open a **pull request**. For now, the "base branch" should be ``master``, i.e. the pull requests are merged directly to the ``master`` branch. In the description:

   * Summarize the change and the motivation for the change.

   * If test case status changes (tests are broken / fixed, test cases themselves needed fixing, test cases were added, etc), mention that.

   * A pull request can be created before you think your changes are finished. It's OK to work on a feature in the pull request: this facilitates discussion in the pull request comments. By using ``@mention`` system you can ask for feedback from specific people.


.. seealso:: `Understanding the GitHub Flow <https://guides.github.com/introduction/flow/>`_
