;;; -*- Mode: lisp; Syntax: ANSI-Common-Lisp; Package: CLIM-LISP; Base: 10; Lowercase: Yes -*-
;; See the file LICENSE for the full license governing this code.

(in-package :clim-lisp)

;;; All of this is taken from the STREAM-DEFINITION-BY-USER proposal to
;;; the X3J13 committee, made by David Gray of TI on 22 March 1989.  No
;;; Lisp implementation yet supports this proposal, so we implement it
;;; here in this separate package.  This way we will be ready when some
;;; Lisp implementation adopts it (or something like it).



#-(or Cloe-Runtime ccl-2)
(progn

;;; We shadow this (and the other functions that already exist in the
;;; Lisp package) and fake the "genericness" of the operation in order
;;; to fit into the existing implementation-dependent stream mechanisms.

#-PCL
(defgeneric streamp (stream))

(defmethod STREAMP (stream)
  (lisp:streamp stream))

;;;

#+Genera
(defgeneric open-stream-p (stream))

#+Genera
(defmethod OPEN-STREAM-P (stream)
  (future-common-lisp:open-stream-p stream))

;;;

#-(or PCL Clozure)
(defgeneric input-stream-p (stream))

#-Clozure
(defmethod INPUT-STREAM-P (stream)
  (lisp:input-stream-p stream))

;;;

#-(or PCL Clozure)
(defgeneric output-stream-p (stream))

#-Clozure
(defmethod OUTPUT-STREAM-P (stream)
  (lisp:output-stream-p stream))

;;;

#-(or PCL Clozure)
(defgeneric stream-element-type (stream))

#-Clozure
(defmethod STREAM-ELEMENT-TYPE (stream)
  (lisp:stream-element-type stream))

;;;

#-(or PCL Clozure)
(defgeneric close (stream &key abort))

#-Clozure
(defmethod CLOSE (stream &key abort)
  (lisp:close stream :abort abort))

;;;

#-PCL
(defgeneric pathname (stream))

(defmethod PATHNAME (stream)
  (cl:pathname stream))

(deftype pathname () 'lisp:pathname)

;;;

#-PCL
(defgeneric truename (stream))

(defmethod TRUENAME (stream)
  (cl:truename stream))

) ;; #-(or Cloe-Runtime ccl-2)

#+Clozure
(progn
(defgeneric pathname (stream))
(defmethod pathname (stream)
  (cl:pathname stream))

(defgeneric truename (stream))
(defmethod truename (stream)
  (cl:truename stream))
) ;end #+Clozure

#-Cloe-Runtime
(progn

(defmacro write-forwarding-cl-output-stream-function (name args &key #+Genera message)
  (let* ((cl-name (find-symbol (symbol-name name) (find-package :lisp)))
	 (method-name (intern (lisp:format nil "~A-~A" 'stream (symbol-name name))))
	 (optional-args (or (member '&optional args) (member '&key args)))
	 (required-args (ldiff args optional-args))
	 (optional-parameters (mapcan #'(lambda (arg)
					  (cond ((member arg lambda-list-keywords) nil)
						((atom arg) (list arg))
						(t (list (car arg)))))
				      optional-args))
	 (pass-args (append required-args optional-parameters))
	 ;; optional-args are &optional in the method,
	 ;; even if &key in the Common Lisp function
	 (method-args (if (eq (first optional-args) '&key)
			  (append required-args '(&optional) (cdr optional-args))
			  args))
	 (pass-keys (if (eq (first optional-args) '&key)
			(mapcan #'(lambda (arg)
				    (unless (atom arg)
				      (setq arg (car arg)))
				    (list (intern (string arg) :keyword) arg))
				(cdr optional-args))
			optional-parameters)))
    (when (eq (first optional-args) '&optional)
      (pop optional-args))
    `(clim-utils:define-group ,name write-forwarding-cl-output-stream-function
       ;; Shadow the Common Lisp function with one that calls the generic function,
       ;; except in Genera and Cloe where the Common Lisp function will work
       #-(or Genera ccl-2)
       (eval-when (compile load eval) (proclaim '(inline ,name)))
       #-(or Genera ccl-2)
       (defun ,name (,@required-args &optional stream ,@optional-args)
	 (case stream
	   ((nil) (,method-name *standard-output* ,@pass-args))
	   ((t) (,method-name *terminal-io* ,@pass-args))
	   (otherwise (,method-name stream ,@pass-args))))

       ;; Define a default method for the generic function that calls back to the
       ;; system stream implementation.  Call back via a message if there is one,
       ;; otherwise via the Common Lisp function.
       ;; Uses T as a parameter specializer name as a standin for cl:stream,
       ;; which Genera doesn't support as a builtin class

       (defmethod ,method-name ((stream t) ,@method-args)
	 #+Genera ,(if message
		       `(scl:send stream ,message ,@pass-args)
		       `(,cl-name ,@required-args stream ,@pass-keys))
	 #-Genera (,cl-name ,@required-args stream ,@pass-keys)))))

(write-forwarding-cl-output-stream-function write-byte (integer)
					    #+Genera :message #+Genera :tyo)

(write-forwarding-cl-output-stream-function write-char (character)
					    #+Genera :message #+Genera :tyo)

(write-forwarding-cl-output-stream-function write-string (string &key (start 0) end)
					    #+Genera :message #+Genera :string-out)

(write-forwarding-cl-output-stream-function terpri ())

(write-forwarding-cl-output-stream-function fresh-line ()
					    #+Genera :message #+Genera :fresh-line)

(write-forwarding-cl-output-stream-function force-output ()
					    #+Genera :message #+Genera :force-output)

(write-forwarding-cl-output-stream-function finish-output ()
					    #+Genera :message #+Genera :finish)

(write-forwarding-cl-output-stream-function clear-output ()
					    #+Genera :message #+Genera :clear-output)


;;;

(eval-when (compile load eval)
(defun order-preserving-set-difference (set-one set-two)
  #-ccl-2 (set-difference set-one set-two)
  #+ccl-2 (nreverse (set-difference set-one set-two)))
)	;eval-when

(defmacro write-forwarding-cl-input-stream-function (name lambda-list
						     &key eof
							  #+Genera message
							  additional-arguments)
  (let* ((cl-name (find-symbol (symbol-name name) (find-package :lisp)))
	 (method-name (intern (lisp:format nil "~A-~A" 'stream (symbol-name name))))
	 (args (mapcar #'(lambda (var) (if (atom var) var (first var)))
		       (order-preserving-set-difference lambda-list lambda-list-keywords)))
	 (stream-args (remove 'stream args))
	 #-(or Genera ccl-2)
	 (call-method `(case stream
			 ((nil) (,method-name *standard-input* ,@stream-args))
			 ((t) (,method-name *terminal-io* ,@stream-args))
			 (otherwise (,method-name stream ,@stream-args)))))
    `(clim-utils:define-group ,name write-forwarding-cl-input-stream-function
       ;; Shadow the Common Lisp function with one that calls the generic function,
       ;; except in Genera or Cloe where the Common Lisp function will work
       #-(or Genera ccl-2)
       (eval-when (compile load eval) (proclaim '(inline ,name)))
       #-(or Genera ccl-2)
       ,(if eof
	    (let ((args `(eof-error-p eof-value ,@(and (not (eq eof :no-recursive))
						       '(recursive-p)))))
	      `(defun ,name (,@lambda-list ,@args)
		 (let ((result ,call-method))
		   (cond ((not (eq result :eof))
			  result)
			 (eof-error-p
			  (signal-stream-eof stream ,@(and (not (eq eof :no-recursive))
							   '(recursive-p))))
			 (t
			  eof-value)))))
	    `(defun ,name ,lambda-list
	       ,call-method))

       ;; Define a default method for the generic function that calls back to the
       ;; system stream implementation.  Call back via a message if there is one,
       ;; otherwise via the Common Lisp function.
       ;; Uses T as a parameter specializer name as a standin for cl:stream,
       ;; which Genera doesn't support as a builtin class

       (defmethod ,method-name ((stream t) ,@(remove 'stream lambda-list))
	 #+Genera ,(if message
		       `(scl:send stream ,message ,@stream-args)
		       `(,cl-name ,@additional-arguments ,@args ,@(and eof `(nil :eof))))
	 #-Genera (,cl-name ,@additional-arguments ,@args ,@(and eof `(nil :eof)))))))

(write-forwarding-cl-input-stream-function read-byte (&optional stream) :eof :no-recursive)

(write-forwarding-cl-input-stream-function read-char (&optional stream) :eof t)

(write-forwarding-cl-input-stream-function unread-char (character &optional stream)
					   #+Genera :message #+Genera :untyi)

(write-forwarding-cl-input-stream-function read-char-no-hang (&optional stream) :eof t)

(write-forwarding-cl-input-stream-function peek-char (&optional stream)
					   :eof t :additional-arguments (nil))

(write-forwarding-cl-input-stream-function listen (&optional stream)
					   #+Genera :message #+Genera :listen)

(write-forwarding-cl-input-stream-function read-line (&optional stream) :eof t
					   #+Genera :message #+Genera :line-in)

(write-forwarding-cl-input-stream-function clear-input (&optional stream)
					   #+Genera :message #+Genera :clear-input)

#-ccl-2
(defun signal-stream-eof (stream &optional recursive-p)
  (declare (ignore stream recursive-p))
  (error "EOF"))

;;; Make FORMAT do something useful on CLIM windows.  (At least CLIM:FORMAT, that is.)
;;; This isn't needed in Genera and Cloe, where the system FORMAT works on CLIM windows.

#-(or Genera ccl-2)
(defun format (stream format-control &rest format-args)
  (when (null stream)
    (return-from format
      (apply #'lisp:format nil format-control format-args)))
  (when (eq stream 't)
    (setq stream *standard-output*))
  (cond ((streamp stream)
	 ;; this isn't going to quite work for ~&,
	 ;; but it's better than nothing.
	 (write-string (apply #'lisp:format nil format-control format-args) stream)
	 nil)
	(t
	 (apply #'lisp:format stream format-control format-args))))

) ;; #-Cloe-Runtime



(defclass FUNDAMENTAL-STREAM (#+ccl-2 stream) ())

(defmethod STREAMP ((stream fundamental-stream)) t)

;;;

(defclass FUNDAMENTAL-INPUT-STREAM (fundamental-stream) ())

(defmethod INPUT-STREAM-P ((stream fundamental-input-stream)) t)

;;;

(defclass FUNDAMENTAL-OUTPUT-STREAM (fundamental-stream) ())

(defmethod OUTPUT-STREAM-P ((stream fundamental-output-stream)) t)

;;;

(defclass FUNDAMENTAL-CHARACTER-STREAM (fundamental-stream) ())

(defmethod STREAM-ELEMENT-TYPE ((stream fundamental-character-stream)) 'character)

;;;

(defclass FUNDAMENTAL-BINARY-STREAM (fundamental-stream) ())

(defclass FUNDAMENTAL-CHARACTER-INPUT-STREAM
	  (fundamental-input-stream fundamental-character-stream)
     ())

(defclass FUNDAMENTAL-CHARACTER-OUTPUT-STREAM
	  (fundamental-output-stream fundamental-character-stream)
     ())

(defclass FUNDAMENTAL-BINARY-INPUT-STREAM
	  (fundamental-input-stream fundamental-binary-stream)
     ())

(defclass FUNDAMENTAL-BINARY-OUTPUT-STREAM
	  (fundamental-output-stream fundamental-binary-stream)
     ())
