;;; -*- Mode: Lisp; Syntax: ANSI-Common-Lisp; Package: CL-USER; Base: 10 -*-

(in-package :CL-USER)

(clsm:define-system :clx-franz
      (:pretty-name "Franz CLX CLIM"
       :default-pathname  "franz:clx;"
       :journal-directory "franz:patches;"
       :patchable t)
  (:module homegrown (:franz-homegrown) (:type :system))
  (:module xlib (:franz-xlib) (:type :system))
  (:module pkg ("pkgdcl") (:type :lisp-read-only))

  (:serial homegrown xlib pkg
   "clx-port"
   "clx-mirror"
   "clx-medium"
   "clx-pixmaps"
   "clx-frames"
   #+(or Genera Cloe-Runtime) "clx-prefill"
   ))
