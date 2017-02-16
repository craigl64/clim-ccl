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
   "cl-streams"
   "defun-utilities"  "reader"  "processes"  "lisp-utilities"
   "clos-patches" "clos" "generics" "utilities"  "queue"  "timers"
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

;; Don't know how to fit these into the set of files for this directory
;; autoconstructor
;; #+CCL ccl-streams
;; #-CLIM-uses-Lisp-stream-classes cl-stream-classes
;; #-CLIM-uses-Lisp-stream-functions cl-stream-functions
;; cl-streams
;; #-ANSI-Conditions condpat
;; #+CCL coral-char-bits
;; ---- defpackage -- don't include
;; ---- defun -- don't include
;; #+Genera "genera-streams"
;; ---- generics-limited -- don't include
;; #+Allegro last
;; ---- lisp-package-fixups -- don't include
;; #+Lucid "lucid-before"
;; #+Lucid "lucid-stream-functions"
;; #+Minima "minima-stream-classes"
