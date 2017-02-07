;;; -*- Mode: Lisp; Syntax: ANSI-Common-Lisp; Package: CL-USER; Base: 10 -*-

(in-package :CL-USER)


(clsm:define-system :clim-silica
        (:pretty-name "CLIM Silica"
         :default-pathname  "clim2:silica;"
         :journal-directory "clim2:patches;"
         :patchable t)
  (:module utils (:clim-utils) (:type :system))

  (:serial utils
   "classes"
;   "generics"
   "text-style"
   "macros"
   "sheet"
   "mirror"
   "event"
   "port"
   "medium"
   "framem"
   "graphics"
   "pixmaps"
   "std-sheet"
   "layout"
   "db-layout"
   "db-box"
   "db-table"
   "gadgets"
   "db-border"
   "db-button"
   "db-slider"
   "db-label"
   "db-scroll"
   "scroll-pane"))
