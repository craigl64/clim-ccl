;;; -*- Mode: Lisp; Syntax: ANSI-Common-Lisp; Package: CL-USER; Base: 10 -*-

(in-package :CL-USER)

#+ANSI-CL (pushnew :clx-ansi-common-lisp *features*)

(clsm:define-system :clim-xlib
        (:pretty-name "CLIM XLIB"
         :default-pathname  "clim2:xlib;"
         :journal-directory "clim2:patches;"
         :patchable t)
  (:module pkg ("package") (:type :lisp-read-only))

  (:serial pkg
   "depdefs"  "clx"
   #-(or Clozure Allegro) "dependent"
   #+Clozure "dep-openmcl"
   #+Allegro "dep-allegro"
   "macros"  "bufmac"  "buffer"  "display"  "gcontext"  "input"  "requests"
   "fonts"  "graphics"  "text"  "attributes"  "translate"  "keysyms"
   "manager"  "image"  "resource"
   #+Allegro "excldep"
   "shape"  "xvidmode"  "xrender"  "glx"  "gl"  "dpms"))
