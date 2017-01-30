;;; -*- Mode: Lisp; Syntax: ANSI-Common-Lisp; Package: CLIM-UTILS; Base: 10; Lowercase: Yes -*-
;; See the file LICENSE for the full license governing this code.
;;

(in-package :clim-utils)

;;;"Copyright (c) 1990, 1991, 1992 Symbolics, Inc.  All rights reserved.
;;; Portions copyright (c) 1988, 1989, 1990 International Lisp Associates."

;; STANDARD-ENCAPSULATING-STREAM needs to be defined early on, because many methods are
;; written on this class.

(define-protocol-class encapsulating-stream (fundamental-stream))

;; These aren't really "encapsulating" streams, they're "delegating streams."
;; What about the delegation ("multiple self", "xstream") problem?
;; See *ORIGINAL-STREAM* for some more information.
(defclass standard-encapsulating-stream
          (encapsulating-stream)
    ((stream :initarg :stream
             :reader encapsulating-stream-stream))
  #+allegro (:default-initargs :element-type 'character))

;; Return the encapsulated stream corresponding to STREAM.
;; If you change GENERATE-STREAM-PROTOCOL-TRAMPOLINES to maintain a more complex
;; mapping than a simple binding of *ORIGINAL-STREAM*, change this too.

(defmethod stream-encapsulates-stream-p
    ((encapsulator standard-encapsulating-stream) stream)
  (let ((encapsulated-stream (encapsulating-stream-stream encapsulator)))
    (or (eq encapsulated-stream stream)
        (stream-encapsulates-stream-p encapsulated-stream stream))))

(defmethod stream-encapsulates-stream-p ((stream1 stream) (stream2 stream))
  nil)

(defun encapsulating-stream (stream)
  (declare (special *original-stream*)) ;;-- fwd reference
  (if (and *original-stream*
           (stream-encapsulates-stream-p *original-stream* stream))
      *original-stream*
    stream))


;;; Shadow this for all Lisp implementations

#-Clozure
(defgeneric interactive-stream-p (stream))

#-Clozure
(defmethod interactive-stream-p ((stream t))
  #+Genera (future-common-lisp:interactive-stream-p stream)
  #+Lucid (lucid::interactive-input-stream-p stream)
  #+Cloe-Runtime nil			;--- this can't be right --Hornig
  #+aclpc t  ; +++pr just for now, we will need to define something
  #+acl86win32 t  ; +++pr just for now, we will need to define something
  #-(or aclpc acl86win32 Genera Lucid Cloe-Runtime) (cl:interactive-stream-p stream))

(defmethod interactive-stream-p ((stream standard-encapsulating-stream))
  (interactive-stream-p (encapsulating-stream-stream stream)))

;;; defined for spr24046
#+Allegro
(defmethod excl:stream-read-sequence ((encapsulator standard-encapsulating-stream)
				      seq &optional start end)
  (let ((encapsulated-stream (encapsulating-stream-stream encapsulator)))
    (cond ((not (typep encapsulated-stream 'fundamental-input-stream))
	   (error "Encapsulated stream is not of type fundamental-input-stream: ~A" encapsulated-stream))
	  (t 
	   (excl:stream-read-sequence encapsulated-stream seq start end)))))
#+Allegro
(defmethod excl:stream-write-sequence ((encapsulator standard-encapsulating-stream)
					string &optional start end)
  (let ((encapsulated-stream (encapsulating-stream-stream encapsulator)))
    (cond ((not (typep encapsulated-stream 'fundamental-output-stream))
	   (error "Encapsulated stream is not of type fundamental-output-stream: ~A" encapsulated-stream))
	  (t 
	   (excl:stream-write-sequence encapsulated-stream string start end)))))

