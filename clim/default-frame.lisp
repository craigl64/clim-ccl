;;; -*- Mode: Lisp; Syntax: ANSI-Common-Lisp; Package: CLIM-INTERNALS; Base: 10; Lowercase: Yes -*-
;; See the file LICENSE for the full license governing this code.
;;

(in-package :clim-internals)

;;;"Copyright (c) 1992 Symbolics, Inc.  All rights reserved."


;;; The default application frame

(define-application-frame default-application () ()
  (:command-definer nil)
  (:menu-bar nil)
  (:top-level nil))

(defvar *default-application-frame*
        (make-application-frame 'default-application :frame-manager nil))

;; We keep a global binding of *APPLICATION-FRAME* because it can make
;; life easier for people debugging applications
(setq *application-frame* *default-application-frame*)


;;; The rest of the default frame manager

(defmethod frame-wrapper ((framem standard-frame-manager) 
                          (frame standard-application-frame) pane)
  (let ((menu-bar (slot-value frame 'menu-bar)))
    (with-look-and-feel-realization (framem frame)
      (outlining ()
        (if menu-bar
            (vertically ()
              (compute-menu-bar-pane frame menu-bar)
              pane)
            pane)))))

(defmethod frame-manager-notify-user
           ((framem standard-frame-manager) message-string 
            &key (style :inform)
                 (frame nil frame-p)
                 (associated-window
                   (if frame-p
                       (frame-top-level-sheet frame)
                       (graft framem)))
                 (title "Notify user")
                 documentation
                 (exit-boxes '(:exit :abort :help))
                 (name title)
                 text-style
                 &allow-other-keys)
  (declare (ignore style documentation name))
  (let ((stream associated-window))
    (accepting-values (stream :exit-boxes exit-boxes :label title
                              :own-window t)
      (with-text-style (stream text-style)
        (write-string message-string stream)))))

;;--- We can do better than this
(defmethod frame-manager-select-file 
           ((framem standard-frame-manager)
            &key (default nil default-p)
                 (frame nil frame-p)
                 (associated-window
                   (if frame-p
                       (frame-top-level-sheet frame)
                       (graft framem)))
                 (title "Select a file")
                 documentation
                 file-search-proc
                 directory-list-label
                 file-list-label
                 (exit-boxes '(:exit :abort :help))
                 (name title)
                 &allow-other-keys)
  (declare (ignore style documentation name
                   file-search-proc directory-list-label file-list-label))
  (let ((stream associated-window))
    (accepting-values (stream :exit-boxes exit-boxes :label title
                              :own-window t)
      (values
        (accept 'pathname :prompt "Enter a pathname"
                :stream stream
                :default default 
                :provide-default (not default-p))))))
