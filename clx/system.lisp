;;; -*- Mode: Lisp; Syntax: ANSI-Common-Lisp; Package: CL-USER; Base: 10 -*-

(in-package :CL-USER)

(clsm:define-system :clx-clim
      (:pretty-name "CLX CLIM"
       :default-pathname  "clim2:clx;"
       :journal-directory "clim2:patches;"
       :patchable t)
  (:module homegrown (:clim-homegrown) (:type :system))
  (:module xlib (:clim-xlib) (:type :system))
  (:module pkg ("pkgdcl") (:type :lisp-read-only))

  (:serial homegrown xlib pkg
   "clx-port"
   "clx-mirror"
   "clx-medium"
   "clx-pixmaps"
   "clx-frames"
   #+(or Genera Cloe-Runtime) "clx-prefill"
   ))
