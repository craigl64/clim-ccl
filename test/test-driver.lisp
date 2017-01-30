(in-package :clim-test)

(defvar *catch-errors-in-tests* t)
(defvar *invocation* nil)

(defclass basic-invocation ()
	  ((process :initform nil :accessor invocation-process)
	   (condition :initform nil :accessor invocation-condition)
	   (state :initform nil :accessor invocation-state)
	   (avv-frame :initform nil :accessor invocation-avv-frame)
	   ))


(defclass frame-invocation (basic-invocation)
  ((frame :initform nil :accessor invocation-frame)))


(defmethod get-invocation-pane ((inv frame-invocation) name)
  (if (panep name)
      name
    (get-frame-pane (invocation-frame inv) name)))

(defclass activity-invocation (basic-invocation)
  ((activity :initform nil :accessor invocation-activity)))

(defmethod get-invocation-pane ((inv activity-invocation) (name symbol))
  (get-frame-pane (invocation-frame inv) name))

(defmethod invocation-frame ((inv activity-invocation))
  (clim-internals::activity-active-frame (invocation-activity inv)))

(defmethod get-invocation-pane ((inv activity-invocation) (name cons))
  (destructuring-bind (frame-type pane-name) name
    (dolist (frame (frame-manager-frames (invocation-activity inv))
	      (error "Cannot find frame type ~S in activity"))
      (when (typep frame frame-type)
	(return (get-frame-pane frame pane-name))))))

(defun invocation-active-p (invocation)
  (not (eq (invocation-state invocation) :dead)))

(defmethod initialize-instance :after ((inv frame-invocation) &key class initargs frame)
  (let ((process nil))
    (if frame
      (setq process (clim-internals::frame-top-level-process frame))
      (setq frame (apply #'make-application-frame class initargs)))
    (setf (invocation-frame inv) frame)
    (initialize-invocation inv frame process)))

(defmethod initialize-instance :after ((inv activity-invocation) &key class initargs)
  (let ((activity (apply #'make-instance class initargs)))
    (setf (invocation-activity inv) activity)
    (initialize-invocation inv activity)))

(defmethod destroy-invocation ((inv frame-invocation))
  (destroy-frame (invocation-frame inv)))

(defmethod destroy-invocation ((inv activity-invocation))
  (destroy-activity (invocation-activity inv)))


(defun initialize-invocation (inv frame &optional process)
  (labels ((do-it (frame)
	     (unwind-protect
		 (loop
		   (let ((again (catch 'try-again
				  (setf (invocation-state inv) :running)
				  (run-frame-top-level frame)
				  nil)))
		     (unless again (return nil))))
	       (setf (invocation-state inv) :dead)))
	   (run-frame-top-level-almost (frame)
	     ;; Maybe this should be a handler bind so there is the
	     ;; option of trying again as well as aborting the process
	     (if *catch-errors-in-tests*
		 (handler-case
		     (do-it frame)
		   (error (condition)
		     (handler-invocation-debugger-hook inv condition)))
	       (do-it frame))))
    (unless process
      (setq process (mp:process-run-function
		     (format nil "~A Test process for" frame)
		     #'run-frame-top-level-almost
		     frame)))
    (setf (invocation-process inv) process)
    (wait-for-clim-input-state inv)))


(defun handler-invocation-debugger-hook (invocation condition)
  (declare (ignore invocation))
  (format excl:*initial-terminal-io* "The following error occurred: ~A~%" condition))

(defvar *default-input-state-timeout* 300)

(defun wait-for-clim-input-state (invocation &optional (timeout *default-input-state-timeout*))
  (let ((process (invocation-process invocation)))
    (let ((port (port (invocation-frame invocation))))
      (when port #-acl86win32 (xm-silica::port-finish-output port)))
    (mp:process-allow-schedule)
    (flet ((input-state-p (process)
	     (or (not (mp:process-thread process))
		 (member  (mp::process-whostate process)
			  '("Returned value"
			    "Waiting for dialog"
			    "CLIM Input")
			  :test #'string-equal))))
      (if timeout
	  (let ((done nil))
	    (mp:process-wait-with-timeout
	     "Waiting for process to sleep"
	     timeout
	     #'(lambda ()
		 (setq done (input-state-p process))))
	    (unless done
	      (error "Timed out after ~D seconds" timeout)))
	(mp:process-wait
	 "Waiting for process to sleep"
	 #'(lambda ()
	     (input-state-p process)))))))

(defvar *death-timeout* 30)

(defun terminate-invocation (invocation exit-command)
  (with-slots (process) invocation
    (write-line "Terminating frame")
    (unless (progn
	      (mp::process-thread process))
      (error "Frame terminated abnormally"))
    (execute-one-command invocation exit-command)
    (unless (wait-for-death process)
      (error "Process did not terminate"))))

(defun wait-for-death (process)
  (let (done)
    (mp:process-wait-with-timeout
     "Waiting for death"
     *death-timeout*
     #'(lambda ()
	 (setq done
	   (not (mp::process-thread process)))))
    done))

(defvar *execute-one-command-hook* nil)

(defun execute-commands-in-invocation (invocation commands)
  (do ((commands commands (cdr commands)))
      ((or (not (invocation-active-p invocation))
	   (null commands)))
    (let ((command (car commands)))
      (flet ((do-it ()
	       (execute-one-command invocation command)))
	(if *execute-one-command-hook*
	    (funcall *execute-one-command-hook* invocation command #'do-it)
	  (do-it))))))

(defmacro define-test-step (name lambda-list &body body)
  `(defun ,name ,lambda-list
     (with-test-stepper-wrapper ()
       ,@body)))

(defmacro with-test-stepper-wrapper ((&key timeout) &body body)
  `(let ((invocation *invocation*))
     (multiple-value-prog1
	 (progn ,@body)
       (wait-for-clim-input-state invocation ,timeout))))

(defvar *command-sequence-table* (make-hash-table))

(defmacro with-invocation-sheet ((sheet) &body body)
  `(let ((,sheet (with-slots (process avv-frame) invocation
		  (frame-top-level-sheet (or avv-frame (invocation-frame invocation))))))
     ,@body))

(defmacro with-invocation-frame ((frame) &body body)
  `(let ((,frame (with-slots (process avv-frame) invocation
		  (or avv-frame (invocation-frame invocation)))))
    ,@body))


(define-test-step character-event (x)
  (with-invocation-sheet (sheet)
    (flet ((send-character (character)
	     (let ((x (clim-internals::port-canonicalize-gesture-spec (port sheet) character)))
	       (when x
		 (distribute-event (port sheet)
				   (make-instance 'key-press-event
						  :sheet sheet
						  :character character
						  :key-name (car x)
						  :modifier-state (cdr x)))))))
      ;;--- make sure the event gets to the sheet
      ;;--- since if the mouse is outside the event is discarded
      (setf (port-keyboard-input-focus (port sheet)) sheet)
      (etypecase x
	(character (send-character x))
	(string (map nil #'send-character x))
	(keyword (distribute-event (port sheet)
				   (multiple-value-bind (keysym modifiers)
				       (clim-internals::gesture-name-keysym-and-modifiers x)
				     (assert keysym () "Illegal gesture")
				     (make-instance 'key-press-event
						    :sheet sheet
						    :character nil
						    :key-name keysym
						    :modifier-state modifiers))))))))

(defun execute-a-command (command)
  (if (and (consp command) (consp (car command)))
      (destructuring-bind (command &key timeout) command
	(with-test-stepper-wrapper (:timeout timeout)
	  (with-invocation-frame (frame)
	    (execute-command-in-frame frame command))))
    (with-test-stepper-wrapper ()
      (with-invocation-frame (frame)
	(execute-command-in-frame frame command)))))

(defun execute-one-command (invocation command)
  (with-slots (process avv-frame) invocation
    (unless (characterp command)
      (format t "Executing command ~S~%" command))
    (etypecase command
      ((or character string keyword)
       (character-event command))
      (list
       (case (car command)
	 (:sleep
	  (sleep (cadr command)))
	 (:presentation-click
	  (apply #'click-on-presentation (cdr command)))
	 (:presentation-press
	  (apply #'click-on-presentation
		 (second command) (third command) :release nil (cdddr command)))
	 (:click
	  (apply #'click-on-window (cdr command)))
	 (:press
	  (apply #'press-on-window (cdr command)))
	 (:release
	  (apply #'release-on-window (cdr command)))
	 (:command
	  (apply #'test-a-command (cdr command)))
	 (:commands
	  (funcall (cadr command) invocation))
	 (:edit-avv
	  (destructuring-bind (pane prompt new-value)  (cdr command)
	    (change-query-value prompt new-value pane)))
	 (t
	  (execute-a-command command)))))))

(defmacro with-test-success-expected ((test-name) &body body)
  (let ((tname (gensym)))
    `(let ((,tname ,test-name))
       (handler-case (progn ,@body)
	 (error (c)
	   (note-test-failed ,tname c))
	 (:no-error (&rest ignore)
	   (declare (ignore ignore))
	   (note-test-succeeded ,tname))))))


(defun exercise-frame (test-name
						class
						initargs
						command-continuation
						exit-command
						&key (#+acl86win32 anerror #-acl86win32 error *catch-errors-in-tests*)
						(invocation-class 'frame-invocation))
  (flet ((doit ()
			   (let ((invocation (make-instance invocation-class :class class :initargs initargs)))
				 (let ((*invocation* invocation))
				   (unwind-protect
					 (etypecase command-continuation
					   (null (sleep 5))
					   (list
						 (execute-commands-in-invocation
						   invocation command-continuation))
					   (function (funcall command-continuation)))
					 (unwind-protect
					   (terminate-invocation invocation exit-command)
					   (mp::process-kill (invocation-process invocation))
					   (unless (wait-for-death (invocation-process invocation))
						 (warn "Process would not die when killed")))
					 (destroy-invocation invocation))))))
	(if #+acl86win32 anerror #-acl86win32 error
		(with-test-success-expected (test-name)
		  (doit))
		(doit))

	#+ignore
	(progn
	  (format t "Port mapping ~S~%" (silica::port-mirror->sheet-table (clim:find-port)))

	  #+verbose
	  (maphash #'(lambda  (x y)
				   (print y))
			   (silica::port-mirror->sheet-table (clim:find-port)))

	  (format t "Port framem ~S~%" (find-frame-manager :port (clim:find-port)))
	  (format t "Port framem frames ~S~%" (frame-manager-frames
											(find-frame-manager :port (clim:find-port))))

	  (format t " Address mapping ~S~%" tk::*address->object-mapping*)

	  #+verbose
	  (maphash #'(lambda  (x y) (print y))
			   tk::*address->object-mapping*))

	))



(defvar *test-successes* nil)
(defvar *test-failures* nil)

(defun note-test-failed (test reason)
  (warn "The following error occurred: ~A" reason)
  (when *test-failures*
    (push (cons test reason) (cdr *test-failures*))))

(defun note-test-succeeded (test)
  (when *test-successes*
    (push test (cdr *test-successes*))))

(defmacro with-test-reporting ((&rest options &key file) &body body)
  (declare (ignore file))
  `(invoke-with-test-reporting #'(lambda () ,@body) ,@options))

(defun invoke-with-test-reporting (continuation &rest options)
  (let ((*test-successes* (list nil))
	(*test-failures* (list nil)))
    (funcall continuation)
    (pop *test-successes*)
    (pop *test-failures*)
    (apply #'generate-test-report options)))

(defun generate-test-report (&key file)
  (format t "~4%")
  (when *test-failures*
    (format t "The following tests failed:~%")
    (dolist (x *test-failures*)
      (format t "~10t~A : ~A~%" (car x) (cdr x))))

  (when (probe-file file)
    (let (old-successes old-failures)
      (with-open-file (*standard-input* file)
	(let ((*package* (find-package :clim-user)))
	  (setq old-successes (read) old-failures (read))))

      (let ((first t))
	(dolist (x *test-failures*)
	  (when (member (car x) old-successes)
	    (when first
	      (format t "The following tests have for failed the first time:~%")
	      (setq first nil))
	    (format t "~10t~A : ~A~%" (car x) (cdr x)))))

      (let ((first t))
	(dolist (x *test-failures*)
	  (when (member (car x) old-failures)
	    (when first
	      (format t "The following tests have failed again:~%")
	      (setq first nil))
	    (format t "~10t~A : ~A~%" (car x) (cdr x)))))

      (let ((first t))
	(dolist (x *test-successes*)
	  (when (member x old-failures)
	    (when first
	      (format t "The following tests have succeeded finally:~%")
	      (setq first nil))
	    (format t "~10t~A~%" x))))))

  (when *test-successes*
    (format t "The following tests succeeded:~%")
    (dolist (x *test-successes*)
      (format t "~10t~A ~%" x)))


  (with-open-file (*standard-output* file
		   :direction :output :if-exists :supersede)
    (let ((*package* (find-package :clim-user)))
      (print *test-successes*)
      (print (mapcar #'car *test-failures*)))))

(defun generate-pretty-test-report (&key file)
  (if (not (probe-file file))
      (error "Report file ~A does not exist. tests did not run!!!" file)
    (multiple-value-bind (successes failures)
	(with-open-file (*standard-input* file)
	  (values (read) (read)))
      (let ((expected-failures nil))
	(let ((unexpected-failures
	       (set-difference failures expected-failures))
	      (unexpected-successes
	       (intersection successes expected-failures)))
	  ;; use *error-output* so it goes to stderr and is seen even
	  ;; though make-dist redirects stdout to a file
	  (format *error-output*
		  "~D CLIM 2.0 tests succeeded~%~D CLIM 2.0 tests failed~%"
		  (length successes) (length failures))
	  (when unexpected-failures
	    (format *error-output*
		    "~D CLIM 2.0 tests failed unexpectedly:~{~20t~A~%~}~%"
		    (length unexpected-failures)
		    unexpected-failures))

	  (when unexpected-successes
	    (format *error-output*
		    "~D tests succeeded unexpectedly: ~{~20t~A~%~}~%"
		    (length unexpected-successes)
		    unexpected-successes)))))))



(defun walk-over-presentations (function output-record)
  (labels ((find-object-1 (record x-offset y-offset)
	     (with-bounding-rectangle* (left top right bottom) record
	       (incf left x-offset)
	       (incf right x-offset)
	       (incf top y-offset)
	       (incf bottom y-offset)
	       (when (typep record 'presentation)
		 (funcall function record left top right bottom))
	       (multiple-value-bind (xoff yoff) (output-record-start-cursor-position record)
		 (map-over-output-records
		  #'find-object-1 record
		  (- x-offset) (- y-offset)
		  (+ x-offset xoff) (+ y-offset yoff))))))
    (find-object-1 output-record 0 0)))

(setq *random-state* (make-random-state t))


(defun warp-the-pointer (sheet x y)
  (multiple-value-setq (x y) (transform-position (sheet-device-transformation sheet) x y))
  #-acl86win32 (tk-silica::port-set-pointer-position-1 (port sheet) sheet x y))

(define-condition cannot-find-presentation-error (simple-error) ())

(define-test-step click-on-presentation (pane-name
					 presentation-type
					 &key gesture
					 (press t)
					 (release t)
					 (modifier 0)
					 (button +pointer-left-button+)
					 (x-offset 0)
					 (y-offset 0)
					 (test #'identity))
  (with-slots (process) invocation
    (when gesture
      (multiple-value-setq (button modifier)
	(clim-internals::gesture-name-button-and-modifiers gesture))
      (assert button () "Not a valid gesture"))
    (let ((pane (get-invocation-pane invocation pane-name))
	  (presentations nil)
	  (expanded-presentation-type
	   (expand-presentation-type-abbreviation presentation-type)))
      (flet ((doit (record left top right bottom)
	       (when (and (presentation-subtypep
			   (presentation-type record)
			   expanded-presentation-type)
			  (funcall test record))
		 (warp-the-pointer pane (+ left x-offset) (+ top y-offset))
		 ;; At this point it would be nice to specify that a
		 ;; modifier key was pressed.
		 (wait-for-clim-input-state invocation)
		 (when (eq (clim-internals::stream-highlighted-presentation
			    pane) record)
		   (push (list record left top right bottom)
			 presentations)))))
	;;-- There is a race condition where moving the pointer
	;;-- generates an exit event which unhighlights the presentation
	;;-- so we loose
	(dotimes (i 2)
	  (wait-for-clim-input-state invocation)
	  (walk-over-presentations #'doit (stream-output-history pane))
	  (when presentations (return nil))
	  (sleep 1)))
      #+ignore
      (format excl:*initial-terminal-io* "~d presentations~%" (length presentations))
      (unless presentations
	(error 'cannot-find-presentation-error :format-control
	       "Did not find presentations to click on!"))
      (when presentations
	(let* ((len (length presentations))
	       (i (random len)))
	  (destructuring-bind
	      (record left top right bottom) (nth i presentations)
	    (declare (ignore right bottom))
	    #+ignore
	    (format excl:*initial-terminal-io*
		    "selecting ~d of ~d = ~s @ ~d,~d~%"
		    i len record left top)
	    ;;-- Exit event problem
	    (dotimes (i 2)
	      (warp-the-pointer pane (+ left x-offset)
				(+ top y-offset))
	      (wait-for-clim-input-state invocation)
	      (when (clim-internals::stream-highlighted-presentation pane)
		(return))
	      (warp-the-pointer pane (+ left 1 x-offset) (+ top 1 y-offset))
	      (sleep 1))
	    (assert (clim-internals::stream-highlighted-presentation pane))
	    ;;-- this is bypassing the distribution mechanism
	    ;;-- Perhaps we should have a send-event that interfaces to
	    ;;-- the Xlib code. But can we send fill in all the detail
	    ;;-- fields of the event
	    ;;-- In order to get the modifiers to work I think we need
	    ;;-- to send Keypress/release events
	    (multiple-value-bind
		(x y) (untransform-position
		       (sheet-device-transformation pane) (+ left
							     x-offset) (+ top y-offset))
	      (let ((ma (sheet-mirrored-ancestor pane))
		    (port (port pane)))
		(when press
		  (distribute-event port
				    (make-instance 'pointer-button-press-event
						   :sheet ma
						   :pointer (port-pointer port)
						   :button button
						   :native-x x
						   :native-y y
						   :x :?? :y :??
						   :modifier-state
						   modifier)))
		(when release
		  (distribute-event port
				    (make-instance 'pointer-button-release-event
						   :sheet ma
						   :pointer (port-pointer port)
						   :button button
						   :native-x x
						   :native-y y
						   :x :?? :y :??
						   :modifier-state
						   modifier)))))
	    (values record presentations)))))))


(define-test-step click-on-window (pane-name left top &rest args)
  (apply #'button-event-on-window invocation pane-name left top
	 :up t :down t args))

(define-test-step press-on-window (pane-name left top &rest args)
  (apply #'button-event-on-window invocation pane-name left top
	 :up nil :down t args))

(define-test-step release-on-window (pane-name left top &rest args)
  (apply #'button-event-on-window invocation pane-name left top
	 :up t :down nil args))

(defun button-event-on-window (invocation pane-name left top
			       &key gesture (modifier 0) (button +pointer-left-button+)
				    up down)
  (with-slots (process) invocation
    (when gesture
      (multiple-value-setq (button modifier)
	(clim-internals::gesture-name-button-and-modifiers gesture))
      (assert button () "Not a valid gesture"))
    (let ((pane (get-invocation-pane invocation pane-name)))
      (stream-set-pointer-position pane left top)
      (wait-for-clim-input-state invocation)
      (multiple-value-bind
	  (x y) (untransform-position (sheet-device-transformation pane) left top)
	(let ((ma (sheet-mirrored-ancestor pane))
	      (port (port pane)))
	  (when down
	    (distribute-event port
			      (make-instance 'pointer-button-press-event
					     :sheet ma
					     :pointer (port-pointer port)
					     :button button
					     :native-x x
					     :native-y y
					     ;;-- normally the distributor
					     ;;-- fills these in
					     :x :?? :y :??
					     :modifier-state modifier)))
	  (when up
	    (distribute-event port
			      (make-instance 'pointer-button-release-event
					     :sheet ma
					     :pointer (port-pointer port)
					     :button button
					     :native-x x
					     :native-y y
					     ;;-- normally the distributor
					     ;;-- fills these in
					     :x :?? :y :??
					     :modifier-state modifier))))))))



(defun simulate-accept (invocation pane-name presentation-type &key
								  stream
								  default provide-default
								  x-offset y-offset &allow-other-keys)
  ;; We a choice.
  ;; 1. Send the characters or
  ;; 2. Click on a presentation
  (declare (ignore provide-default default stream invocation))
  (assert (click-on-presentation pane-name
				 presentation-type
				 :x-offset x-offset
				 :y-offset y-offset)
      () "Clicking failed"))

(defun send-it (invocation x &key (delim t))
  (execute-one-command invocation (string x))
  (when delim
    (execute-one-command invocation " ")))

(define-test-step test-a-command (pane-name command &key colon-prefix
							 (x-offset 0)
							 (y-offset 0))
  (with-slots (process) invocation
    (let ((stream (get-invocation-pane invocation pane-name)))
      (flet ((accept-function (stream presentation-type &rest args)
	       (apply #'simulate-accept invocation pane-name
		      presentation-type :stream stream :x-offset x-offset :y-offset y-offset args))
	     (send-it-function (x)
	       (send-it invocation x)))
	(when colon-prefix
	  (send-it invocation ":" :delim nil))
	(fill-in-partial-command-1
	 (if (atom command) command (car command))
	 (frame-command-table (invocation-frame invocation))
	 stream
	 (substitute *unsupplied-argument-marker* :unsupplied  (if (atom command) (list command) command))
	 #'accept-function
	 #'send-it-function)
	;;--- Need to send a newline now
	;;--- It looks like some clim code tries to eat following
	;;--- delimiters if they are there. Which code is this?
	(sleep 2)
	(send-it invocation #\newline :delim nil)
	))))



(defvar *frame-tests* nil)

(defmacro define-frame-test (name (class &rest initargs) commands exit-command)
  `(progn
     (pushnew ',name *frame-tests*)
     (defun ,name ()
       (exercise-frame ',name ',class ',initargs
		       ',commands
		       ',exit-command))))

(defmacro define-activity-test (name (class &rest initargs) commands exit-command)
  `(progn
     (pushnew ',name *frame-tests*)
     (defun ,name ()
       (exercise-frame ',name ',class ',initargs
		       ',commands
		       ',exit-command
		       :invocation-class 'activity-invocation))))

(defmacro define-command-sequence (name &rest commands)
  `(defun ,name (invocation)
     (execute-commands-in-invocation invocation ',commands)))

(defun do-frame-tests (&optional (errorp *catch-errors-in-tests*))
  (dolist (test *frame-tests*)
    (format t "Doing test ~A~%" test)
    (if errorp
	(handler-case (funcall test)
	  (error (c)
	    (format t "~&The following error occurred in ~S: ~A~%" test
		    c)))
      (funcall test))))


;;; Training stuff

(defun train-clim-2 (&optional (n 2))
  (dotimes (i n)
    (format t "Test ~D out of ~D~%" (1+ i) n)
    (test-it)))

(defun benchmark-clim (&optional filename)
  (test-it (or filename
	       (multiple-value-bind (second minute hour date month year)
		   (get-decoded-time)
		 (declare (ignore second minute hour))
		 (format nil "notes/times/~D.~D.~D.n" month date year)))))

(defun test-it (&optional (pathname #-acl86win32 "/dev/null" #+acl86win32 "null") (errorp *catch-errors-in-tests*))
  (exercise-frame 'test-it
		  'clim-user::clim-tests
		  '(:width 600 :height 400 :left 0 :top 0)
		  `(((clim-user::run-benchmarks
		      :pathname ,pathname) :timeout 1800))
		  `(clim-user::exit-clim-tests)
		  #+acl86win32 :anerror #-acl86win32 :error errorp))

(defun fill-in-partial-command-1 (command-name command-table stream
				  partial-command accept-function send-it)
  (let ((*original-stream* nil)
	(copy-partial-command (if (atom partial-command)
				  (list partial-command)
				partial-command)))
    (labels ((arg-parser (stream presentation-type &rest args)
	       (declare (dynamic-extent args))
	       ;; This code is to handle the case where a partial command has been
	       ;; passed in.  PARSE-NORMAL-ARG needs to be called with a :DEFAULT of
	       ;; the appropriate element of the partial command structure.
	       (let* ((default (if copy-partial-command
				   (pop copy-partial-command)
				 *unsupplied-argument-marker*)))
		 (with-presentation-type-decoded (type-name parameters) presentation-type
		   (when (eq type-name 'command-name)
		     (send-it (string-downcase (command-line-name-for-command
						command-name
						command-table)))
		     (return-from arg-parser (values command-name presentation-type)))
		   (cond ((not (clim-internals::unsupplied-argument-p default))
			  (cond ((eq type-name 'keyword-argument-name)
				 default)
				(t (handler-case (apply #'parse-normal-arg
							stream presentation-type
							:default default args)
				     ;;-- The condition should be no presentations
				     (error (c)
				       (declare (ignore c))
				       default)))))
			 ((eq type-name 'keyword-argument-name)
			  (let ((name (intern (symbol-name (caar parameters)) clim-internals::*keyword-package*)))
			    (send-it name)
			    name))
			 (t
			  (apply #'parse-normal-arg
				 stream presentation-type
				 :provide-default nil args))))))
	     (send-it (it)
	       (funcall send-it it))
	     (parse-normal-arg (stream arg-type &rest options)
	       (declare (dynamic-extent options))
	       (with-delimiter-gestures (*command-argument-delimiters*)
		 (apply accept-function stream arg-type options)))
	     (separate-args (stream args-to-go)
	       (declare (ignore stream args-to-go))
	       #+ignore
	       (when (only-keyword-args-remain args-to-go)
		 (throw 'stop-reading-command-arguments nil))))
      (declare (dynamic-extent #'arg-parser #'parse-normal-arg #'separate-args))
      (clim-internals::invoke-command-parser-and-collect
       command-table #'arg-parser #'separate-args stream))))

;;; Need
;;; 1. Access to avv fields
;;; 2. Send mouse events clicks that are independent of presentations
;;; (eg tracking-pointer stuff).

;;; Testing avv

(defun get-avv-frame (invocation)
  (with-slots (process) invocation
    (let ((avv-frame nil))
      (loop
	(setf avv-frame nil)
	(mp::process-interrupt process #'(lambda () (setq avv-frame (list *application-frame*))))
	(mp:process-wait "Waiting for frame" #'(lambda () avv-frame))
	(when (typep  (car avv-frame) 'clim-internals::accept-values)
	  (return-from get-avv-frame (car avv-frame)))
	(sleep 1)))))

#+ignore
(defun find-avv-query (avv-stream prompt)
  (let ((table (slot-value (slot-value avv-stream 'clim-internals::avv-record) 'clim-internals::query-table)))
    (maphash #'(lambda (query-id query)
		 (when (if (consp query-id) ;;--yuck
			   (and
			    (eq (car query-id) :query-identifier)
			    (consp (cdr query-id))
			    (equal (cadr query-id) prompt))
			 (eq query-id prompt))
		   (return-from find-avv-query query)))
	     table)
    (maphash #'(lambda (x y)
		 (print x)
		 (print y))
	     table)
    (error "Could not find query ~S" prompt)))

(defun find-avv-query (avv-stream prompt)
  (maphash #'(lambda (query-id query)
	       (when (if (consp query-id) ;;--yuck
			 (and
			  (eq (car query-id) :query-identifier)
			  (consp (cdr query-id))
			  (equal (cadr query-id) prompt))
		       (eq query-id prompt))
		 (return-from find-avv-query query)))
	   (slot-value (slot-value avv-stream 'clim-internals::avv-record) 'clim-internals::query-table)))


(define-test-step change-query-value (prompt new-value &optional pane-name)
  (with-slots (process avv-frame) invocation
    (let ((query (find-avv-query (if pane-name
				     (car (gethash (get-invocation-pane invocation pane-name)
						   (clim-internals::frame-pane-to-avv-stream-table
						    (invocation-frame invocation))))
				   (slot-value avv-frame 'stream))
				 prompt)))
      (assert query () "Could not find the query")
      (execute-one-command invocation `(clim-internals::com-change-query ,query ,new-value)))))

(defun invoke-accept-values-button (label pane)
  (map-over-sheets #'(lambda (sheet)
		       (when (and (typep sheet 'push-button)
				  (equal label (gadget-label sheet)))
			 (return-from invoke-accept-values-button
			   (activate-push-button sheet))))
		   (get-frame-pane
		    (invocation-frame *invocation*)
		    pane))
  (error "Cannot find button ~A" label))

(defun activate-push-button (sheet)
  (distribute-event
   (port sheet)
   (silica::allocate-event 'silica:activate-gadget-event
			   :gadget sheet))
  (wait-for-clim-input-state *invocation*))

(defun change-gadget-value (sheet value)
  (distribute-event
   (port sheet)
   (silica::allocate-event 'silica::value-changed-gadget-event
			   :value value
			   :gadget sheet))
  (wait-for-clim-input-state *invocation*))


;; How to invoke command-buttons
;; It would be nice if we could change gadget values directly.
;; Lets say we want to run the bitmap-editor-add-color frame, change
;; some values and then exit from the frame
;; How can we wait for a frame to be created. Grab hold of it and then
;; run some commands on it.

(eval-when (compile load eval)
  (require :prof))

(defun do-frame-test-with-profiling (test &key (type :time) prefix)
  (flet ((profiling-hook (invocation command continuation)
	   ;;-- it would be nice to restrict it to the invocation process
	   (declare (ignore invocation))
	   (if (or (atom command)
		   (equal command '(clim-user::exit-clim-tests)))
	       (funcall continuation)
	     (progn
	       (unwind-protect
		   (progn
		     (prof::start-profiler :type type :verbose nil)
		     (funcall continuation))
		 (profiler:stop-profiler))
	       (with-open-file (*standard-output* (format nil "~A/~A.~A-tree.lisp" prefix (car command) type)
				:direction :output :if-exists :supersede)
		 (prof:show-call-graph))
	       (with-open-file (*standard-output* (format nil "~A/~A.~A-flat.lisp" prefix (car command) type)
				:direction :output :if-exists :supersede)
		 (prof:show-flat-profile))))))
    (let ((*execute-one-command-hook* #'profiling-hook))
      (funcall test))))

(defun wait-for-frame (&key (type t)
			      (framem (find-frame-manager))
			      timeout
			    (state :enabled)
			    (errorp t))
  (let ((frame nil))
    (flet ((predicate ()
	     (setq frame (find-frame :type type :framem framem :state state))))
      (if timeout
	  (mp::process-wait-with-timeout "Waiting for frame" timeout #'predicate)
	(mp:process-wait "Waiting for frame" #'predicate))
      (unless (or frame (not errorp))
	(error "Cannot find frame of type ~S" type))
      frame)))


(defun find-frame (&key (type t)
			(framem (find-frame-manager))
			(state :enabled))
  (find-if #'(lambda (frame)
	       (and (typep frame type)
		    (if (listp state)
			(member (frame-state frame) state)
		      (eq (frame-state frame) state))))
	   (frame-manager-frames framem)))

(defun find-notify-user (frame)
  (getf (mp:process-property-list
	 (clim-internals::frame-top-level-process frame))
	'notify-user))

(defun find-menu (frame)
  (getf (mp:process-property-list (clim-internals::frame-top-level-process frame)) 'menu-choose))


(defun abort-menu (frame)
  (mp::process-interrupt (clim-internals::frame-top-level-process frame)
			 #'(lambda () (throw 'menu-choose nil)))
  (mp::process-allow-schedule))

(defun select-menu-item (frame value)
  (mp::process-interrupt (clim-internals::frame-top-level-process frame)
			 #'(lambda () (throw 'menu-choose value)))
  (mp::process-allow-schedule))

(defun notify-user-ok (frame)
  (mp::process-interrupt (clim-internals::frame-top-level-process frame)
			 #'(lambda () (throw 'notify-user t)))
  (mp::process-allow-schedule))

(defun find-user-interface-components (frame specifications &key (errorp t))
  (let* ((n (length specifications))
	 (result (make-list n)))
    (map-over-sheets #'(lambda (sheet)
			 (dotimes (i n)
			   (when (and (not (nth i result))
				      (sheet-matches-specification-p
				       sheet (nth i specifications)))
			     (setf (nth i result) sheet))))
		     (frame-top-level-sheet frame))
    (when (and errorp (notevery #'identity result))
      (error "Cannot find ui components ~A"
	     (mapcan #'(lambda (spec res)
			 (unless res (list spec)))
		     specifications
		     result)))
    (values-list result)))

(defun sheet-matches-specification-p (sheet specification)
  (if (functionp specification)
      (funcall specification sheet)
    (typep sheet specification)))


(defun wait-for-frame-to-exit (frame &key timeout)
  (let ((done nil))
    (flet ((predicate ()
	     (setq done
	       (not (eq (frame-state frame) :enabled)))))
      (if timeout
	  (mp::process-wait-with-timeout "Waiting for disable" timeout #'predicate)
	(mp::process-wait "Waiting for disable" #'predicate)))
    (assert done () "Frame did not un enable")))

(defmacro with-frame-invocation ((frame) &body body)
  `(invoke-with-frame-invocation ,frame #'(lambda () ,@body)))

(defun invoke-with-frame-invocation (frame continuation)
  (let ((*invocation* (make-instance 'frame-invocation
				     :frame frame)))
    (funcall continuation)))

(define-test-step press-push-button (button)
  #-acl86win32 (xm-silica::queue-active-event nil nil button))

(defmacro with-waiting ((&key timeout) &body clauses)
  (let ((i 0)
	(result (gensym))
	(which (gensym))
	(time (gensym)))
    `(let ((,result nil)
	   (,time ,timeout)
	   ,which)
       (flet ((predicate ()
		(or ,@(mapcar #'(lambda (clause)
				  `(when (car (setq ,result (multiple-value-list ,(car clause))))
				    (setq ,which ,(incf i))))
			      clauses))))
	 (if ,time
	     (mp::process-wait-with-timeout "Waiting" ,time #'predicate)
	   (mp::process-wait "Waiting" #'predicate)))
       ,(do ((clauses clauses (cdr clauses))
	    (i 0)
	    res)
	   ((null clauses)
	    `(case ,which ,@(reverse res)))
	  (let ((clause (car clauses)))
	 (push `(,(incf i)
		     (destructuring-bind ,(second clause) ,result
			,@(cddr clause)))
	       res))))))


;;; This should be at the end:
;;; make the training selective.

(locally (declare (special si::*clos-preload-packages*))
  (setq si::*clos-preload-packages*
    (mapcar #'find-package
	    '(:clim :clim-utils :clim-internals :silica :tk #-acl86win32 :xm-silica))))

;; This stops warnings happening asynchronously and causing confusion.

;;The warnings go to the initial terminal io, anyway.  With the following
;;uncommented, the full tests can't complete in 100mb of swap space.
;;(setq excl:*global-gc-behavior* nil)
