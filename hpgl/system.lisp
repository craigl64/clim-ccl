;;; -*- Mode: Lisp; Syntax: ANSI-Common-Lisp; Package: cl-user; Base: 10 -*-

(in-package :CL-USER)


(clsm:define-system :hpgl-clim
        (:pretty-name "HPGL CLIM"
         :default-pathname  "clim2:hpgl;"
         :journal-directory "clim2:patches;"
         :patchable t)
  (:module pkg ("pkg") (:type :lisp-read-only))

  (:serial pkg
   "hpgl-port"
   "hpgl-medium"))

