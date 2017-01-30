;;; -*- Mode: Lisp; Syntax: ANSI-Common-Lisp; Package :CLSM; Base: 10 -*-

(in-package :clsm)

(compile-system :clim-utils)
(compile-system :clim-silica)
(compile-system :clim-standalone)
(compile-system :clim-homegrown)
(compile-system :clim-xlib)
(compile-system :clx-clim)
(compile-system :clim-demos)
(format t "~&;; Starting CLIM Demos~%")
(clim-demo:start-demo)
(ccl:quit)
