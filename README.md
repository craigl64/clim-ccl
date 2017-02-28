# clim-ccl
Franz' CLIM code modified to comile in Clozure CL.

o All of the Makefile have been removed as they are not useful for
  Clozure CL.
o The Allegro FFI based XLIB directory has been replaced with a pure
  Common Lisp version of the XLIB library.
o The other Allegro FFI based backends are ignored and will eventually
  be removed.
o Only the CLX CLIM backend is used and it is using the Lisp based
  XLIB library.

o Eventually, we will add a Clozure CL FFI backend for GTK as the
preferred backend for Linux hosts.

o A Cocoa backend would be nice for Mac OSX hosts.

o A Win32 (or Win64) backend for Windows would also be nice.

