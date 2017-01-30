;;; -*- Mode: Lisp; Syntax: ANSI-Common-Lisp; Package: CL-USER; Base: 10 -*-

(in-package :CL-USER)


(clsm:define-system :franz-homegrown
        (:pretty-name "Franz CLIM Homegrown"
         :default-pathname  "franz:homegrown;"
         :journal-directory "franz:patches;"
         :patchable t)
  (:module standalone (:franz-standalone) (:type :system))

  (:serial standalone
   "db-button"
   "db-label"
   "db-menu"
   "db-list"
   "db-slider"
   "db-text"
;   "scroll-pane"
   ))
