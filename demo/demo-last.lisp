;; -*- Mode: Lisp; Syntax: ANSI-Common-Lisp; Package: CL-USER -*-
;; See the file LICENSE for the full license governing this code.
;;

(in-package :cl-user)

(provide :climdemo)

#+Allegro
(cond ((excl::featurep :clim-motif)
       (provide :climdemoxm)
       (load "clim2demoxm-preload.fasl" :if-does-not-exist nil))
      ((excl::featurep :clim-openlook)
       (provide :climdemool)
       (load "clim2demool-preload.fasl" :if-does-not-exist nil)))
