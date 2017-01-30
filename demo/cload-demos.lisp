;; -*- Mode: Lisp; Package: clim-user -*-
;; See the file LICENSE for the full license governing this code.
;;

(in-package :clim-user)

(defvar *demo-files*
    '(
      "test-suite"
      "packages"
      "demo-driver"
      "cad-demo"
      "thinkadot"
      "graphics-demos"
      "address-book"
      "listener"
      "navfun"
      "navdata"
      "puzzle"
      "plot"
      "color-editor"
      "graphics-editor"
      "bitmap-editor"
      "ico"
      "process-browser"
      "peek-frame"
      "demo-activity"
      "custom-records"
      "browser"))

(defun compile-and-load-demos (&key forcep
				    (directory
				     (make-pathname
				      :directory (pathname-directory
						  #+Allegro
                                                  #.(cl:truename excl::*source-pathname*
                                                  #-Allegro cl:*load-truename*)))))
  (mapcar #'(lambda (name)
	      (let ((name (merge-pathnames
			   directory
			   name)))
		(if forcep
		    (compile-file name)
		  #+Allegro (excl::compile-file-if-needed name)
                  #+Clozure
                  (let ((bpath (compile-file-pathname name))
                        (spath (make-pathname :type "lisp" :defaults name))
                        bdate sdate)
                    (setf bdate (when (probe-file bpath) (file-write-date bpath)))
                    (setf sdate (file-write-date spath))
                    (when (< bdate sdate) (compile-file name))))
		(load name)))
	  *demo-files*))
