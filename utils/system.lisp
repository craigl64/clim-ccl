;;; -*- Mode: Lisp; Syntac: ANSI-Common-Lisp; Package: CL-USER; Base: 10 -*-

#-Genera
(in-package :CL-USER)

#+ANSI-CL (pushnew :ansi-90 *features*)

#+CLozure (pushnew :CLIM-Uses-Lisp-Stream-Classes   *features*)
#+Clozure (pushnew :CLIM-Uses-Lisp-Stream-Functions *features*)
;#+Clozure (pushnew :CLIM-Extends-CLOS               *features*)

(pushnew :clim     *features*)
(pushnew :clim-2   *features*)
(pushnew :clim-2.0 *features*)

(clsm:define-system :clim-utils
    (:pretty-name "CLIM Utils"
     :default-pathname  "clim2:utils;"
     :journal-directory "clim2:patches;"
     :patchable t)
  (:module pkgs ("packages") (:type :lisp-read-only))

  (:serial pkgs
   #+Allegro "excl-verification"
   "defun-utilities"  "reader"  "processes"  "lisp-utilities"
   "clos-patches"  "clos" "generics" "utilities"  "queue"  "timers"
   "protocols"  "clim-streams"
   #+Allegro "excl-streams"
   "clim-macros"
   #|"clim-defs"|#                          ; clim/clim-defs
   "transformations" "regions" "region-arithmetic" "extended-regions"
   #|"ptypes1"|#                            ; clim/ptypes1
   ;; postscript/pkgdcl
   "base-designs"
   "designs"
   ))
