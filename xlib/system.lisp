;;; -*- Mode: Lisp; Syntax: ANSI-Common-Lisp; Package: CL-USER; Base: 10 -*-

(in-package :CL-USER)

#+ANSI-CL (pushnew :clx-ansi-common-lisp *features*)

(clsm:define-system :franz-xlib
        (:pretty-name "Franz CLIM XLIB"
         :default-pathname  "franz:xlib;"
         :journal-directory "franz:patches;"
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
