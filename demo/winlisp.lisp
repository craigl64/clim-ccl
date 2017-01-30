;;; -*- Mode: Lisp; Syntax: ANSI-Common-Lisp; Package: CLIM-DEMO; Base: 10; Lowercase: Yes -*-
;; See the file LICENSE for the full license governing this code.
;;

(in-package :clim-demo)

;;"Copyright (c) 1990, 1991, 1992 Symbolics, Inc.  All rights reserved."

(define-command-table mswin-file-commands
  :menu (("Exit" :command (com-wlisp-exit))))

(define-command-table mswin-edit-commands
  :menu (("Copy" :command (com-copy-object))
	 ("Paste" :command (com-paste))))

(define-command-table mswin-help-commands
  :menu (("About" :command (com-about))))

(define-command (com-wlisp-exit :name "Exit"
				:command-table mswin-file-commands
				:menu ("Exit" :documentation "Quit application"))
  ()
  #+ignore
  (format *terminal-io* "~%Quitting ~S" clim:*application-frame*)
  (clim:frame-exit clim:*application-frame*))

(define-command-table wlistener-file
  :inherit-from (mswin-file-commands)
  :inherit-menu t)

(define-command-table wlistener-edit)

(define-command-table wlistener-help)

#+ignore ;; doesn't work unless inside define-app-frame ?!?
(define-command-table wlistener
  :inherit-from (user-command-table
		  wlistener-file
		  wlistener-edit
		  wlistener-help
		  )
  :menu (("File"
	       :menu wlistener-file
	       :mnemonic #\F  
	       :documentation "File commands")
	 ("Edit"
	       :menu wlistener-edit
	       :mnemonic #\E
	       :documentation "Edit Commands")
	 ("Help"
	       :menu wlistener-help
	       :mnemonic #\H
	       :documentation "Help Commands")))

(define-command (com-copy-object
		  :name "Copy Object"
		  :command-table wlistener-edit
		  :menu
		  ("Copy" :documentation
			"Copy selected object to Clipboard"))
  ()
  (let ((obj (history-top-element clim-internals::*kill-ring*)))
    (acl-clim::lisp->clipboard obj)))

(define-command (com-paste
		  :name "Paste"
		  :command-table wlistener-edit
		  :menu
		  ("Paste" :documentation "Paste from Clipboard"))
  ()
  (let ((value (acl-clim::clipboard->lisp)))
    (push-history-element clim-internals::*kill-ring* value)))
 
(define-command (com-about :command-table wlistener-help
			   :menu
			   ("About Lisp Listener"
				   :documentation "About Lisp Listener"))
  ()
  (acl-clim::pop-up-about-climap-dialog *application-frame*))

(define-application-frame wlistener ()
			  ()
  (:command-table (wlistener
		   :inherit-from (user-command-table
				  wlistener-file
				  wlistener-edit
				  wlistener-help
				  )
		   :menu (("File"
			   :menu wlistener-file
			   :mnemonic #\F  
			   :documentation "File commands")
			  ("Edit"
			   :menu wlistener-edit
			   :mnemonic #\E
			   :documentation "Edit Commands")
			  ("Help"
			   :menu wlistener-help
			   :mnemonic #\H
			   :documentation "Help Commands"))))
  (:command-definer nil)
  (:menu-bar t)
  (:top-level (wlistener-top-level))
  (:panes
   (interactor :interactor 
	       #+allegro :excl-recording-p #+allegro t
	       :scroll-bars :both))
  (:layouts (default interactor)))

(defmethod frame-maintain-presentation-histories ((frame wlistener)) t)

(defmacro wl-condition-restart-loop ((conditions description . args) &body body)
  #---ignore (declare (ignore conditions))
  (let ((tag (clim-utils:gensymbol 'restart)))
    `(tagbody ,tag
       (restart-case
	   (progn ,@body)
	 (nil ()
	   #|| :test (lambda (condition)
		       (some #'(lambda (x) (typep condition x)) ',conditions)) ||#
	   :report (lambda (stream)
		     (format stream ,description ,@args))))
       (go ,tag))))

(defvar *wlistener-depth* -1)

(defvar *wlistener-frame*)
(defvar *wlistener-io*)

(defvar *wlisp-use-native-debugger* #+aclpc nil #+acl86win32 t)

(defvar *wlisp-prompt-arrow-1* 
	(make-pattern #2A((0 0 0 0 0 0 0 0 0 0 0 0)
			  (0 0 0 0 0 1 0 0 0 0 0 0)
			  (0 0 0 0 0 1 1 0 0 0 0 0)
			  (0 1 1 1 1 1 1 1 0 0 0 0)
			  (0 1 1 1 1 1 1 1 1 0 0 0)
			  (0 0 0 0 0 0 0 1 1 1 0 0)
			  (0 0 0 0 0 0 0 0 1 1 1 0)
			  (0 0 0 0 0 0 0 1 1 1 0 0)
			  (0 1 1 1 1 1 1 1 1 0 0 0)
			  (0 1 1 1 1 1 1 1 0 0 0 0)
			  (0 0 0 0 0 1 1 0 0 0 0 0)
			  (0 0 0 0 0 1 0 0 0 0 0 0))
		      (list +background-ink+ +foreground-ink+)))

(defvar *wlisp-prompt-arrow-2* 
	(make-pattern #2A((0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0)
			  (0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0)
			  (0 0 0 0 0 0 0 0 0 1 1 0 0 0 0 0)
			  (0 1 1 1 1 1 1 1 1 1 1 1 0 0 0 0)
			  (0 1 1 1 1 1 1 1 1 1 1 1 1 0 0 0)
			  (0 0 0 0 0 0 0 0 0 0 0 1 1 1 0 0)
			  (0 0 0 0 1 1 1 1 1 1 1 1 1 1 1 0)
			  (0 0 0 0 0 0 0 0 0 0 0 1 1 1 0 0)
			  (0 1 1 1 1 1 1 1 1 1 1 1 1 0 0 0)
			  (0 1 1 1 1 1 1 1 1 1 1 1 0 0 0 0)
			  (0 0 0 0 0 0 0 0 0 1 1 0 0 0 0 0)
			  (0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0))
		      (list +background-ink+ +foreground-ink+)))

(defun wlistener-top-level (frame)
  "Run a simple Lisp listener using the window provided."
  (enable-frame frame)
  (let* ((*wlistener-frame* frame)
	 (window (frame-standard-input frame))
	 (command-table (frame-command-table frame))
	 (presentation-type `((command-or-form :command-table ,command-table)
			      :auto-activate #+(or Genera Cloe-Runtime) t
					     #-(or Genera Cloe-Runtime) nil)))
    (with-input-focus (window)
      (let* ((*wlistener-io* window)
	     (*standard-input* *wlistener-io*)
	     (*standard-output* *wlistener-io*)
	     #+(or Minima allegro) (*error-output* *wlistener-io*)
	     (*query-io* *wlistener-io*)
	     #+Minima (*debug-io* *wlistener-io*)
	     (*package* *package*)
	     (*wlistener-depth* (1+ *wlistener-depth*))
	     (*** nil) (** nil) (* nil)
	     (/// nil) (// nil) (/ nil)
	     (+++ nil) (++ nil) (+ nil)
	     (- nil)
	     #+allegro (*tpl-current-stack-group* sys::*current-thread*)
	     #+allegrox (*top-top-frame-pointer*
			 (debug::newest-frame sys::*current-thread*
			  :visible-only-p nil))
	     #+allegro (*top-frame-pointer*
			 (or (db::find-interesting-frame *top-top-frame-pointer*)
			     *top-top-frame-pointer*))
	     #+allegro (*current-frame-pointer* *top-frame-pointer*))
	(terpri *wlistener-io*)
	(with-command-table-keystrokes (keystrokes command-table)
	  (wl-condition-restart-loop (#+Genera (sys:error sys:abort)
				   #-Genera (error)
				   "Restart CLIM lisp listener")
	    (wlistener-command-reader
	      frame *standard-input* command-table presentation-type
	      :keystrokes keystrokes
	      :listener-depth *wlistener-depth*
	      :prompt (case *wlistener-depth*
			(0 *wlisp-prompt-arrow-1*)
			(1 *wlisp-prompt-arrow-2*)
			(otherwise 
			  (concatenate 'string 
			    (make-string (1+ *wlistener-depth*) :initial-element #\=)
			    "> "))))))))))

(defun wlistener-command-reader (frame stream command-table presentation-type 
				     &key keystrokes listener-depth (prompt "=> "))
  (catch-abort-gestures ("Return to ~A command level ~D"
			 (frame-pretty-name frame) listener-depth)
    ;; Eat any abort gestures that might be hanging around.
    ;; We need to do this because COMMAND-OR-FORM is wierd.
    (let* ((abort-gestures *abort-gestures*)
	   (*abort-gestures* nil))
      ;;--- What test does this need?
      (when (member (stream-read-gesture stream :timeout 0 :peek-p t) abort-gestures)
	(stream-read-gesture stream :timeout 0)))
    (fresh-line stream)
    (if (stringp prompt)
	(write-string prompt stream)
	(multiple-value-bind (x y) (stream-cursor-position stream)
	  (draw-pattern* stream prompt x y)
	  (stream-increment-cursor-position stream (pattern-width prompt) nil)))
    (multiple-value-bind (command-or-form type numeric-arg)
	(block keystroke
	  (handler-bind ((accelerator-gesture
			   #'(lambda (c)
			       ;; The COMMAND-OR-FORM type is peeking for the
			       ;; first character, looking for a ":", so we
			       ;; have to manually discard the accelerator
			       (stream-read-gesture stream :timeout 0)
			       (return-from keystroke
				 (values
				   (accelerator-gesture-event c)
				   :keystroke
				   (accelerator-gesture-numeric-argument c)))))
			 (simple-parse-error
			   #'(lambda (c)
			       (let ((args (clim-internals::parse-error-format-arguments c)))
				 (when (and (= (length args) 1)
					    (equal (first args) ""))
				   ;; Hmm, user must have just hit <Return>
				   (return-from wlistener-command-reader)))
			       ;; Otherwise decline to handle this
			       nil))
			 (clim-internals::synchronous-command-event
			   #'(lambda (c)
			       (let* ((command (clim-internals::synchronous-command-event-command c))
				      (ptype `(command :command-table ,(frame-command-table frame)))
				      )
				 #+ignore
				 (push-history-element
				   (presentation-type-history ptype)
				   (clim-internals::make-presentation-history-element
				     :object command :type ptype))
				 (return-from
				   keystroke
				   (values command ptype)))))
			 )
	    (let ((*accelerator-gestures* keystrokes))
	      (accept presentation-type
		      :stream stream
		      :prompt nil :prompt-mode :raw
		      :additional-activation-gestures '(#+Genera #\End)))))
      (when (eq type :keystroke)
	(let ((command (lookup-keystroke-command-item command-or-form command-table 
						      :numeric-argument numeric-arg)))
	  (unless (clim-internals::keyboard-event-p command)
	    (when (partial-command-p command)
	      (setq command (funcall *partial-command-parser*
				     command command-table stream nil
				     :for-accelerator t)))
	    (setq command-or-form command
		  type 'command))))
      (cond ((eq type ':keystroke)
	     (beep))
	    ((eq (presentation-type-name type) 'command)
	     (terpri)
	     (let ((*debugger-hook* 
		     (unless *wlisp-use-native-debugger*
		       (and (zerop listener-depth) #'wlistener-debugger-hook))))
	       (apply (command-name command-or-form)
		      (command-arguments command-or-form)))
	     (terpri))
	    (t
	     (terpri)
	     (let ((values 
		     (multiple-value-list
		       (let ((*debugger-hook* 
			       (unless *wlisp-use-native-debugger*
				 (and (zerop listener-depth) #'wlistener-debugger-hook))))
			 (eval command-or-form)))))
	       (fresh-line)
	       (dolist (value values)
		 (present value 'expression :single-box :highlighting)
		 (terpri))
	       (setq - command-or-form)
	       (shiftf +++ ++ + -)
	       (when values
		 ;; Don't change this stuff if no returned values
		 (shiftf /// // / values)
		 (shiftf *** ** * (first values)))))))))

(defvar *wlisp-debugger-condition* nil)
(defvar *wlisp-debugger-restarts* nil)

(defun wlistener-debugger-hook (condition hook)
  (declare (ignore hook))
  (let* ((*application-frame* *wlistener-frame*)
	 #+Minima (*debug-io* (frame-query-io *application-frame*))
	 (*error-output* (frame-standard-output *application-frame*))
	 (*wlisp-debugger-condition* condition)
	 (*wlisp-debugger-restarts* (compute-restarts)))
    (describe-error *error-output*)
    (with-output-recording-options (*wlistener-io* :draw t :record t)
      (wlistener-top-level *application-frame*))))

(defvar *wlisp-enter-debugger* '#:wlisp-enter-debugger)
(defun wlisp-enter-debugger (stream)
  #-Genera stream
  #+Genera
  (clim-internals::with-debug-io-selected (stream)
    (cl:break "Debugger break for ~A" (frame-pretty-name (pane-frame stream)))))

(define-presentation-type wlisp-restart-name ())

(define-presentation-method presentation-typep (object (type wlisp-restart-name))
  (or (eql object *wlisp-enter-debugger*)
      (typep object 'restart)))

(define-presentation-method present (object (type wlisp-restart-name) stream (view textual-view)
				     &key)
  (if (eql object *wlisp-enter-debugger*)
      (princ "Enter the debugger" stream)
      (prin1 (restart-name object) stream)))

(define-command (com-wlisp-invoke-restart :name t :command-table wlistener)
    ((restart 'wlisp-restart-name :gesture :select))
  (if (eql restart *wlisp-enter-debugger*)
      (enter-debugger *standard-input*)
    (let (values)
      #+allegro
      (case (restart-name (find-restart restart))
	(excl::return-value
	 (setq values
	   (list (eval
		  (accept 'form :stream *standard-input*
			  :prompt "Enter value to return")))))
	(excl::try-a-different-function
	 (setq values
	   (list (eval
		  (accept 'form :stream *standard-input*
			  :prompt "enter expression which will evaluate to the function to call")))))
	(excl::try-a-different-function-setf
	 (setq values
	   (list (eval
		  (accept 'form :stream *standard-input*
			  :prompt "enter expression which will evaluate to the function to call"))))))
      (apply #'invoke-restart restart values))))

(define-command (com-wlisp-describe-error :name t :command-table wlistener) ()
  (wlisp-describe-error *error-output*))

(defun wlisp-describe-error (stream)
  (when *wlisp-debugger-condition*
    (with-output-as-presentation (stream *wlisp-debugger-condition* 'form
				  :single-box t)
      (format stream "~2&Error: ~A" *wlisp-debugger-condition*))
    (let ((restarts *wlisp-debugger-restarts*))
      (when restarts
	(let ((actions '(invoke-restart)))
	  (dolist (restart (reverse restarts))
	    (let ((action (member (restart-name restart)
				  '(abort continue muffle-warning store-value use-value))))
	      (when action
		(pushnew (first action) actions))))
	  (format stream "~&Use~?to resume~:[~; or abort~] execution:"
	    "~#[~; ~S~; ~S or ~S~:;~@{~#[~; or~] ~S~^,~}~] "
	    actions (member 'abort actions)))
	(fresh-line stream)
	(formatting-table (stream :x-spacing '(2 :character))
	  (dolist (restart restarts)
	    (with-output-as-presentation (stream restart 'wlisp-restart-name
					  :single-box t :allow-sensitive-inferiors nil)
	      (formatting-row (stream)
		(formatting-cell (stream) stream)
		(formatting-cell (stream)
		  (when (restart-name restart)
		    (format stream "~S:" (restart-name restart))))
		(formatting-cell (stream)
		  (format stream "~A" restart)))))
	  #+Genera
	  (with-output-as-presentation (stream *wlisp-enter-debugger* 'wlisp-restart-name
					:single-box t :allow-sensitive-inferiors nil)
	    (formatting-row (stream)
	      (formatting-cell (stream) stream)
	      (formatting-cell (stream) stream)
	      (formatting-cell (stream)
		(write-string "Enter the debugger" stream)))))))
    (let ((process (clim-sys:current-process)))
      (when process
	(format stream "~&In process ~A." process))))
  (force-output stream))

(define-command (com-wlisp-use-native-debugger :name t :command-table wlistener)
    ((boolean 'boolean
	      :prompt "yes or no"
	      :default (not *wlisp-use-native-debugger*)))
  (setq *wlisp-use-native-debugger* boolean))


;;; Lisp-y stuff

(defun wlisp-quotify-object-if-necessary (object)
  (if (or (consp object)
	  (and (symbolp object)
	       (not (keywordp object))
	       (not (eq object nil))
	       (not (eq object t))))
      (list 'quote object)
    object))

(define-presentation-translator describe-lisp-object
    (expression form wlistener
     :documentation
       ((object stream)
	(let ((*print-length* 3)
	      (*print-level* 3)
	      (*print-pretty* nil))
	  (present `(describe ,(wlisp-quotify-object-if-necessary object)) 'expression
		   :stream stream :view +pointer-documentation-view+)))
     :gesture :describe)
    (object)
  `(describe ,(wlisp-quotify-object-if-necessary object)))

(define-presentation-translator expression-identity
    (expression nil wlistener
     :tester
       ((object context-type)
	(if (and (eq (presentation-type-name context-type) 'sequence)
		 (or (listp object)
		     (vectorp object)))
	    (clim-utils:with-stack-list
	      (type 'sequence (wlisp-reasonable-presentation-type (elt object 0)))
	      (clim-internals::presentation-subtypep-1 type context-type))
	    (clim-internals::presentation-subtypep-1
	      (wlisp-reasonable-presentation-type object) context-type)))
     :tester-definitive t
     :documentation ((object stream)
		     (let ((*print-length* 3)
			   (*print-level* 3)
			   (*print-pretty* nil))
		       (present object 'expression 
				:stream stream :view +pointer-documentation-view+)))
     :gesture :select)
    (object)
  object)

(defun wlisp-reasonable-presentation-type (object)
  (let* ((class (class-of object))
	 (class-name (class-name class)))
    (when (presentation-type-specifier-p class-name)
      ;; Don't compute precedence list if we don't need it
      (return-from wlisp-reasonable-presentation-type class-name))
    (dolist (class (class-precedence-list class))
      (when (presentation-type-specifier-p (class-name class))
	(return-from wlisp-reasonable-presentation-type (class-name class))))
    nil))

(define-gesture-name :describe-presentation :pointer-button (:middle :super))
(define-presentation-translator describe-presentation
    (t form wlistener
     :documentation
       ((object presentation stream)
	(declare (ignore object))
	(let ((*print-length* 3)
	      (*print-level* 3)
	      (*print-pretty* nil))
	  (present `(describe ,(wlisp-quotify-object-if-necessary presentation)) 'expression
		   :stream stream :view +pointer-documentation-view+)))
     :gesture :describe-presentation)
    (object presentation)
  (declare (ignore object))
  `(describe ,(wlisp-quotify-object-if-necessary presentation)))

(define-command (com-wlisp-edit-function :name t
				   :command-table wlistener-edit
				   :menu ("Edit Function"
					:documentation "Edit a function."))
    ((function 'expression 
	       :provide-default t :prompt "function name"))
  (ed function))

(define-presentation-to-command-translator edit-function
    (expression com-wlisp-edit-function wlistener
     :tester ((object)
	      (functionp object))
     :gesture :edit)
    (object)
  (list object))


;;; Useful commands

(define-command (com-wlisp-clear-output-history :name t
						:command-table wlistener)
    ()
  (window-clear (frame-standard-output *application-frame*)))

#+Genera
(add-keystroke-to-command-table 
  'wlistener '(:l :control :meta) :command 'com-wlisp-clear-output-history)

#-Minima
(progn
(define-command (com-wlisp-copy-output-history :name t
					       :command-table wlistener)
    ((pathname 'pathname :prompt "file"))
  (with-open-file (stream pathname :direction :output)
    (copy-textual-output-history *standard-output* stream)))

(define-command (com-wlisp-show-homedir :name t :command-table wlistener) ()
  (wlisp-show-directory (make-pathname :defaults (user-homedir-pathname)
				 :name :wild
				 :type :wild
				 :version :newest)))

(define-command (com-wlisp-show-directory :name t
				    :command-table
				    wlistener-file
				    :menu t)
    ((directory '((pathname) :default-type :wild)
		:provide-default t :prompt "file"))
  (wlisp-show-directory directory))

(defun wlisp-show-directory (directory-pathname)
  (let ((stream *standard-output*)
	(pathnames #+Genera (rest (fs:directory-list directory-pathname))
		   #-Genera (directory directory-pathname)))
    (flet ((pathname-lessp (p1 p2)
	     (let ((name1 (pathname-name p1))
		   (name2 (pathname-name p2)))
	       (or (string-lessp name1 name2)
		   (and (string-equal name1 name2)
			(let ((type1 (pathname-type p1))
			      (type2 (pathname-type p2)))
			  (and type1 type2 (string-lessp type1 type2))))))))
      (setq pathnames (sort pathnames #'pathname-lessp 
			    :key #+Genera #'first #-Genera #'identity)))
    (fresh-line stream)
    (format stream "~A" (namestring directory-pathname))
    (fresh-line stream)
    (formatting-table (stream :x-spacing "   ")
      (dolist (pathname pathnames)
	(let (size creation-date author)
	  #-Genera
	  (with-open-file (file-stream pathname :direction :input) 
	    (setf size (file-length file-stream)
		  creation-date (file-write-date file-stream)
		  author (file-author file-stream)))
	  #+Genera
	  (setf size (getf (rest pathname) :length-in-bytes)
		creation-date (getf (rest pathname) :modification-date)
		author (getf (rest pathname) :author)
		pathname (first pathname))
	  (with-output-as-presentation (stream pathname 'pathname
					:single-box t)
	    (formatting-row (stream)
	      (formatting-cell (stream)
		(format stream "  ~A" (file-namestring pathname)))
	      (formatting-cell (stream :align-x :right)
		(format stream "~D" size))
	      (formatting-cell (stream :align-x :right)
		(when creation-date
		  (multiple-value-bind (secs minutes hours day month year)
		      (decode-universal-time creation-date)
		    (format stream "~D/~2,'0D/~D ~2,'0D:~2,'0D:~2,'0D"
		      month day year hours minutes secs))))
	      (formatting-cell (stream)
		(write-string author stream)))))))))

(define-command (com-wlisp-show-file :name t
			       :command-table
			       wlistener-file
			       :menu t)
    ((pathname 'pathname 
	       :provide-default t :prompt "file"
	       :gesture :select))
  (wlisp-show-file pathname *standard-output*))

;;; I can't believe CL doesn't have this
(defun wlisp-show-file (pathname stream)
  (clim-utils:with-temporary-string (line-buffer :length 100)
    (with-open-file (file pathname :if-does-not-exist nil)
      (when file
	(loop
	  (let ((ch (read-char file nil 'eof)))
	    (case ch
	      (eof
		(return-from wlisp-show-file))
	      (#\linefeed nil)
	      ((#\Newline #-Genera #\Return)
	       (write-string line-buffer stream)
	       (write-char #\Newline stream)
	       (setf (fill-pointer line-buffer) 0))
	      (otherwise
		(vector-push-extend ch line-buffer)))))))))

(define-command (com-wlisp-edit-file :name t
			       :command-table wlistener-file
			       :menu t)
    ((pathname 'pathname
	       :provide-default t :prompt "file"
	       :gesture :edit))
  (ed pathname))

(define-command (com-wlisp-delete-file
				:name t
				:command-table wlistener-file
				:menu t)
    ((pathnames '(sequence pathname) :prompt "files"))
  (map nil #'delete-file pathnames))

(define-presentation-to-command-translator delete-file
    (pathname com-wlisp-delete-file wlistener
     :gesture nil)
    (object)
  (list `(,object)))

#+Genera
(define-wlistener-command (com-expunge-directory :name t)
    ((directory 'pathname 
		:provide-default t :prompt "directory"))
  (fs:expunge-directory directory))

;;--- We can do better than this
(define-command (com-wlisp-copy-file :name t
			       :command-table wlistener-file
			       :menu t)
    ((from-file 'pathname 
		:provide-default t :prompt "from file")
     (to-file 'pathname :default from-file :prompt "to file"))
  (write-string "Would copy ")
  (present from-file 'pathname)
  (write-string " to ")
  (present to-file 'pathname)
  (write-string "."))

(define-command (com-wlisp-compile-file
				:name t
				:command-table wlistener-file
			       :menu t)
    ((pathnames '(sequence pathname)
		:provide-default t :prompt "files"))
  (map nil #'compile-file pathnames))

(define-presentation-to-command-translator compile-file
    (pathname com-wlisp-compile-file wlistener
     :gesture nil)
    (object)
  (list `(,object)))

(define-command (com-wlisp-load-file
				:name t
				:command-table wlistener-file
				:menu t)
    ((pathnames '(sequence pathname) 
		:provide-default t :prompt "file"))
  (map nil #'load pathnames))

(define-presentation-to-command-translator load-file
    (pathname com-wlisp-load-file wlistener
     :gesture nil)
    (object)
  (list `(,object)))

)	;#-Minima

#+ignore
(define-wlistener-command (com-demonstrate-clim :name "Demonstrate CLIM") ()
  (start-demo :port (port *application-frame*)))


;;; Just for demonstration...

(define-presentation-type wlisp-printer ())

(defparameter *wlisp-printer-names*
	      '(("The Next Thing" tnt)
		("Asahi Shimbun" asahi)
		("Santa Cruz Comic News" comic-news)
		("Le Figaro" figaro)
		("LautScribner" lautscribner)))
		
(define-presentation-method accept ((type wlisp-printer) stream (view textual-view) &key)
  (completing-from-suggestions (stream :partial-completers '(#\space))
    (dolist (printer *wlisp-printer-names*)
      (suggest (first printer) (second printer)))))

(define-presentation-method present (printer (type wlisp-printer) stream (view textual-view)
				     &key acceptably)
  (let ((name (or (first (find printer *wlisp-printer-names* :key #'second))
		  (string printer))))
    (write-token name stream :acceptably acceptably)))

(define-presentation-method presentation-typep (object (type wlisp-printer))
  (symbolp object))

#-Minima
(define-command (com-wlisp-hardcopy-file :name t :command-table wlistener)
    ((file 'pathname 
	   :provide-default t :prompt "file"
	   :gesture :describe)
     (printer 'wlisp-printer 
	      :prompt "printer"
	      :gesture :select)
     &key
     (orientation '(member normal sideways) :default 'normal
      :documentation "Orientation of the printed result")
     (query 'boolean :default nil :mentioned-default t
      :documentation "Ask whether the file should be printed")
     (reflect 'boolean :when (and file (equal (pathname-type file) "SPREADSHEET"))
      :default nil :mentioned-default t
      :documentation "Reflect the spreadsheet before printing it"))
  (format t "Would hardcopy ")
  (present file 'pathname)
  (format t " on ")
  (present printer 'wlisp-printer)
  (format t " in ~A orientation." orientation)
  (when query
    (format t "~%With querying."))
  (when reflect
    (format t "~%Reflected.")))

;;--- Just for demonstration...
(define-command (com-wlisp-show-some-commands :name t :command-table wlistener) ()
  (let ((ptype `(command :command-table user-command-table)))
    (formatting-table ()
      #-Minima
      (formatting-row ()
	(formatting-cell ()
	  (present `(com-wlisp-show-file ,(merge-pathnames "foo" (user-homedir-pathname)))
		   ptype)))
      #-Minima
      (formatting-row ()
	(formatting-cell ()
	  (present `(com-wlisp-show-directory ,(merge-pathnames "*" (user-homedir-pathname)))
		   ptype)))
      #-Minima
      (formatting-row ()
	(formatting-cell ()
	  (present `(com-wlisp-copy-file ,(merge-pathnames "source" (user-homedir-pathname))
				   ,(merge-pathnames "dest" (user-homedir-pathname)))
		   ptype)))
      #-Minima
      (formatting-row ()
	(formatting-cell ()
	  (present `(com-wlisp-hardcopy-file ,(merge-pathnames "quux" (user-homedir-pathname))
				       asahi)
		   ptype)))
      (formatting-row ()
	(formatting-cell ()
	  (present '(com-quit) ptype))))))




(define-demo "Win Listener" wlistener :width 600 :height 500)

#+Genera
(define-genera-application wlistener
			   :pretty-name "CLIM Lisp Listener"
			   :select-key #\� 
			   :width +fill+ :height +fill+)

