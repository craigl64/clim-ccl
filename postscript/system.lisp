;;; -*- Mode: Lisp; Syntax: ANSI-Common-Lisp; Package: CL-USER; Base: 10 -*-

(in-package :CL-USER)


(clsm:define-system :postscript-franz
        (:pretty-name "Franz PostScript CLIM"
         :default-pathname  "franz:postscript;"
         :journal-directory "franz:patches;"
         :patchable t)
  (:module clim (:clx-franz) (:type :system))

  (:serial clim
   "postscript-s"
;   "postscript-clim-stubs"
   "postscript-port"
   "postscript-medium"
   "read-afm"
   "laserwriter-metrics"
;   "climps"
   ))

