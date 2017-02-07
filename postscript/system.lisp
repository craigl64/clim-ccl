;;; -*- Mode: Lisp; Syntax: ANSI-Common-Lisp; Package: CL-USER; Base: 10 -*-

(in-package :CL-USER)


(clsm:define-system :postscript-clim
        (:pretty-name "PostScript CLIM"
         :default-pathname  "clim2:postscript;"
         :journal-directory "clim2:patches;"
         :patchable t)
  (:module clim (:clx-clim) (:type :system))

  (:serial clim
   "postscript-s"
;   "postscript-clim-stubs"
   "postscript-port"
   "postscript-medium"
   "read-afm"
   "laserwriter-metrics"
;   "climps"
   ))

