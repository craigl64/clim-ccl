;;; -*- Mode: Lisp; Syntax: ANSI-Common-Lisp; Package: CL-USER; Base: 10 -*-

(in-package :CL-USER)


(clsm:define-system :franz-silica
        (:pretty-name "Franz CLIM Silica"
         :default-pathname  "franz:silica;"
         :journal-directory "franz:patches;"
         :patchable t)
  (:module utils (:franz-utils) (:type :system))

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
