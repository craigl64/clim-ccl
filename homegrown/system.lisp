;;; -*- Mode: Lisp; Syntax: ANSI-Common-Lisp; Package: CL-USER; Base: 10 -*-

(in-package :CL-USER)


(clsm:define-system :clim-homegrown
        (:pretty-name "CLIM Homegrown"
         :default-pathname  "clim2:homegrown;"
         :journal-directory "clim2:patches;"
         :patchable t)
  (:module standalone (:clim-standalone) (:type :system))

  (:serial standalone
   "db-button"
   "db-label"
   "db-menu"
   "db-list"
   "db-slider"
   "db-text"
;   "scroll-pane"
   ))
