Changes in `1.0.0 <http://matrix.org/git/olm/commit/?h=1.0.0>`_
===============================================================

This release includes a fix to a bug which had the potential to leak sensitive
data to the application: see
https://github.com/vector-im/vector-web/issues/1719. Users of pre-1.x.x
versions of the Olm library should upgrade. Our thanks to `Dmitry Luyciv
<https://github.com/dluciv>`_ for bringing our attention to the bug.

Other changes since 0.1.0:

 * *Experimental* implementation of the primitives for group sessions. This
   implementation has not yet been used in an application and developers are
   advised not to rely on its stability.

 * Replace custom build scripts with a Makefile.

 * Include the major version number in the soname of libolm.so (credit to
   Emmanuel Gil Peyrot).
