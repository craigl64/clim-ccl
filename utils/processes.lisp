;;; -*- Mode: Lisp; Syntax: ANSI-Common-Lisp; Package: CLIM-UTILS; Base: 10; Lowercase: Yes -*-
;; See the file LICENSE for the full license governing this code.
;;

(in-package :clim-utils)

;;;"Copyright (c) 1990, 1991, 1992 Symbolics, Inc.  All rights reserved.
;;; Portions copyright (c) 1988, 1989, 1990 International Lisp Associates.
;;; Portions copyright (c) 1992, 1993 Franz, Inc."


;;; Locks 

#+Allegro
(eval-when (compile load eval)
;;;  (require :mdproc)
  (require :process))

(defvar *multiprocessing-p* 
  #{
    (or allegro Genera Lucid Lispworks Minima Clozure) t
    otherwise nil
    }
    )

;;; This is to keep it quiet: On ACL it's safe to declare the
;;; predicate & args dynamic-extent on platforms with native threads
;;; *only*, which at present (6.0beta) is Windows platforms.
;;;
;;; the real definition of process-wait is in
;;; clim2:;aclpc;acl-clim.lisp.  That definition is almost certainly
;;; bogus because it misunderstands the whole way multithreading
;;; works: the definition above should be used instead.  But the
;;; Windows event-loop depends on this misunderstanding, and I don't
;;; want to change that.
;;;
#+(and allegro mswindows)
(excl:defun-proto process-wait (wait-reason predicate &rest args)
  (declare (dynamic-extent predicate args)))
  
;;-- I dont think we need this
;#+Allegro
;(unless (excl::scheduler-running-p)
;  (mp:start-scheduler))

(defmacro with-lock-held ((place &optional state) &body forms)
  #+(or allegro Xerox Genera (and ccl (not Clozure)) Minima)
  (declare (ignore state #+ccl place))
  #{
    allegro        `(mp:with-process-lock (,place) ,@forms)
    Lucid        `(lcl:with-process-lock (,place ,@(if state (cons state nil)))
                   ,@forms)
    lispworks        `(mp::with-lock (,place) ,@forms)
    Xerox        `(il:with.monitor ,place ,@forms)
    Cloe-Runtime `(progn ,@forms)
    aclpc       `(progn ,@forms)
    Genera        `(process:with-lock (,place) ,@forms)
    Minima        `(minima:with-lock (,place) ,@forms)
    CCL-2        `(progn ,@forms)
    Clozure     `(ccl:with-lock-grabbed (,place ,state) ,@forms)
    }
  )

(defun make-lock (&optional (lock-name "a CLIM lock"))
  #-(or Genera Minima allegro Clozure) (declare (ignore lock-name))
  #{
    allegro        (mp::make-process-lock :name lock-name)
    lispworks        (mp::make-lock)
    Lucid        nil
    CCL-2        nil
    Clozure      (ccl:make-lock lock-name)
    Xerox        (il:create.monitorlock)
    Cloe-Runtime nil
    aclpc       nil
    Genera        (process:make-lock lock-name)
    Minima        (minima:make-lock lock-name)
   }
  )

;;; A lock that CAN be relocked by the same process.
#-(or Genera Minima)
(defmacro with-simple-recursive-lock ((lock &optional (state "Unlock")) &body forms)
  `(flet ((foo () ,@forms))
     (declare (dynamic-extent #'foo))
     (invoke-with-simple-recursive-lock ,lock ,state #'foo)))

#-(or Genera Minima)
(defun invoke-with-simple-recursive-lock (place state continuation)
  (let ((store-value (current-process))
        (place-value (first place)))
    (if (and place-value (eql place-value store-value))
        (funcall continuation)
        (progn
          (unless (null place-value)
            (flet ((waiter ()
                     (null (first place))))
              #-allegro (declare (dynamic-extent #'waiter))
              (process-wait state #'waiter)))
          (unwind-protect
              (progn (rplaca place store-value)
                     (funcall continuation))
            (rplaca place nil))))))

(defmacro with-recursive-lock-held ((place &optional state) &body forms)
  #+(or Xerox Genera (and ccl (not Clozure)) Minima)
  (declare (ignore state #+ccl place))
  #{Genera `(process:with-lock (,place) ,@forms)
    Minima `(minima:with-lock (,place) ,@forms)
    CCL-2 `(progn ,@forms)
    Clozure `(ccl:with-lock-grabbed (,place ,state) ,@forms)
    otherwise `(with-simple-recursive-lock (,place ,state) ,@forms)
    }
  )

(defun make-recursive-lock (&optional (lock-name "a recursive CLIM lock"))
  #-(or Genera Minima) (declare (ignore lock-name))
  #{CCL-2 nil
    Clozure  (ccl::make-recursive-lock)
    Genera (process:make-lock lock-name :recursive T)
    Minima (minima:make-lock lock-name :recursive T)
    otherwise (cons nil nil)
   }
  )


;;; Atomic operations

(defmacro without-scheduling (&body forms)
  "Evaluate the forms w/o letting any other process run."
  #{
    allegro    `(excl:without-interrupts ,@forms) 
    lispworks  `(sys::without-scheduling ,@forms)
    Lucid      `(lcl:with-scheduling-inhibited ,@forms)
    Xerox      `(progn ,@forms)
    Cloe-Runtime `(progn ,@forms)
    aclpc      `(progn ,@forms)
    ;; should be process:with-no-other-processes if this is used as
    ;; a global locking mechanism
    Genera     `(scl:without-interrupts ,@forms)
    Minima     `(minima:with-no-other-processes ,@forms)
    CCL-2      `(ccl:without-interrupts ,@forms) ; slh
    Clozure    `(ccl:without-interrupts ,@forms)
   }
   )

;; Atomically increments a fixnum value
#+Genera
(defmacro atomic-incf (reference &optional (delta 1))
  (let ((location '#:location)
        (old-value '#:old)
        (new-value '#:new))
    `(loop with ,location = (scl:locf ,reference)
           for ,old-value = (scl:location-contents ,location)
           for ,new-value = (sys:%32-bit-plus ,old-value ,delta)
           do (when (scl:store-conditional ,location ,old-value ,new-value)
                (return ,new-value)))))

#-Genera
(defmacro atomic-incf (reference &optional (delta 1))
  (let ((value '#:value))
    (if (= delta 1)
        `(without-scheduling 
           (let ((,value ,reference))
             (if (eq ,value most-positive-fixnum)
                 (setf ,reference most-negative-fixnum)
	       (setf ,reference (the fixnum (1+ (the fixnum ,value)))))))
      #+ignore (warn "Implement ~S for the case when delta is not 1" 'atomic-incf)
      #-ignore ;; maybe?
      (if (< delta 0)
	  `(without-scheduling
	     (let ((,value ,reference))
	       (if (< ,delta (- ,value most-negative-fixnum))
		   (setf ,reference most-positive-fixnum)
		 (setf ,reference (the fixnum (+ (the fixnum ,delta) (the fixnum ,value)))))))
	`(without-scheduling
	   (let ((,value ,reference))
	     (if (> ,delta (- most-positive-fixnum ,value))
		 (setf ,reference most-negative-fixnum)
	       (setf ,reference (the fixnum (+ (the fixnum ,delta) (the fixnum ,value))))))))
      )))

;; Atomically decrements a fixnum value
#+Genera
(defmacro atomic-decf (reference &optional (delta 1))
  (let ((location '#:location)
        (old-value '#:old)
        (new-value '#:new))
    `(loop with ,location = (scl:locf ,reference)
           for ,old-value = (scl:location-contents ,location)
           for ,new-value = (sys:%32-bit-difference ,old-value ,delta)
           do (when (scl:store-conditional ,location ,old-value ,new-value)
                (return ,new-value)))))

#-Genera
(defmacro atomic-decf (reference &optional (delta 1))
  (let ((value '#:value))
    (if (= delta 1)
        `(without-scheduling 
           (let ((,value ,reference))
             (if (eq ,value most-negative-fixnum)
                 (setf ,reference most-positive-fixnum)
                 (setf ,reference (the fixnum (1- (the fixnum ,value)))))))
        (warn "Implement ~S for the case when delta is not 1" 'atomic-decf))))


;;; Processes

(defun make-process (function &key name)
  #+(and ccl (not Clozure)) (declare (ignore function  name))
  (when *multiprocessing-p*
    #{
    Clozure    (ccl:process-run-function name function)
    lispworks  (mp:process-run-function name nil function)
    Lucid      (lcl:make-process :function function :name name)
    allegro    (mp:process-run-function name function)
    Xerox      (il:add.process (funcall function) 'il:name name)
    Genera     (scl:process-run-function name function)
    Minima     (minima:make-process name :initial-function function)
    otherwise  (warn "No implementation of MAKE-PROCESS for this system.")
    }))

(eval-when (compile load eval) (proclaim '(inline processp)))
(defun processp (object)
  #{
  (and ccl (not Clozure))  (member object '(:user :event :interrupt))
  Clozure    (ccl::processp object)
  Lucid             (lcl:processp object)
  allegro    (mp::process-p object)
  lispworks  (mp::process-p object)
  ;; In 7.3 and after it is `(process:process-p ,object)
  Genera     (process:process-p object)
  Minima     (typep object 'minima-internals::basic-process)
  otherwise  (progn (warn "No implementation of PROCESSP for this system.")
                    nil)
  }
  )

(defun destroy-process (process)
  #+(and ccl (not Clozure)) (declare (ignore process))
  #{
  Lucid      (lcl:kill-process process)
  allegro    (mp:process-kill process)
  Clozure    (ccl:process-kill process)
  lispworks  (mp:process-kill process)
  Xerox             (il:del.process process)
  Genera     (scl:process-kill process)
  Minima     (minima:process-kill process)
  CCL-2             nil
  otherwise  (warn "No implementation of DESTROY-PROCESS for this system.")
  }
  )

#+CCL-2
(defvar *current-process* :user)

(eval-when (compile load eval) (proclaim '(inline current-process)))
(defun current-process ()
  #{
  Lucid      lcl:*current-process*
  allegro    mp:*current-process*
  lispworks  mp:*current-process*
  Xerox             (il:this.process)
  Genera     scl:*current-process*
  Minima     (minima:current-process)
  CCL-2             *current-process*
  Clozure        ccl:*current-process*
  Cloe-Runtime nil
  aclpc      nil
  }
  )

(eval-when (compile load eval) (proclaim '(inline all-processes)))
(defun all-processes ()
  #{
  Lucid      lcl:*all-processes*
  allegro    mp:*all-processes*
  lispworks  (mp::list-all-processes)
  Genera     sys:all-processes
  CCL-2             (adjoin *current-process* '(:user))
  Clozure    (ccl:all-processes)
  Cloe-Runtime nil
  aclpc      nil
  }
  )

(defun show-processes ()
  #{
       Lucid          (lcl::show-processes)
       Genera          (si:com-show-processes)
       otherwise  (all-processes)
  }
  )
  
(eval-when (compile load eval) (proclaim '(inline process-yield)))
(defun process-yield ()
  #{
  Lucid      (lcl:process-allow-schedule)
  allegro    (mp:process-allow-schedule)
  lispworks  (mp::process-allow-scheduling)
  Xerox             (il:block)
  Genera     (scl:process-allow-schedule)
  Minima     (sleep 1/10)
  CCL-2             (ccl:event-dispatch)
  Clozure       (ccl::yield)
  Cloe-Runtime nil
  aclpc      nil
  }
  )

#-mswindows
(defun process-wait (wait-reason predicate)
  #+(or Genera Minima) (declare (dynamic-extent predicate))
  "Cause the current process to go to sleep until the predicate returns TRUE."
  #{
  Lucid      (lcl:process-wait wait-reason predicate)
  allegro    (mp:process-wait wait-reason predicate)
  lispworks  (mp:process-wait wait-reason predicate)
  Xerox      (let ((il:*who-line-state* wait-reason))
               (loop
                 (il:block)
                 (when (and (funcall predicate))
                   (return))))
  CCL-2      (ccl::process-wait wait-reason predicate)
  Clozure    (ccl:process-wait wait-reason predicate)
  Cloe-Runtime nil
  aclpc      nil
  Genera     (scl:process-wait wait-reason predicate)
  Minima     (minima:process-wait wait-reason predicate)
  otherwise  (warn "No implementation of PROCESS-WAIT for this system.")
  }
  )

(defun process-wait-with-timeout (wait-reason timeout predicate)
  #+(or Genera Minima) (declare (dynamic-extent predicate))
  "Cause the current process to go to sleep until the predicate returns TRUE or
   timeout seconds have gone by." 
  (when (null timeout)
    ;; ensure genera semantics, timeout = NIL means indefinite timeout
    (return-from process-wait-with-timeout
      (process-wait wait-reason predicate)))
  #{
  allegro    (mp:process-wait-with-timeout wait-reason timeout predicate)
  lispworks  (mp:process-wait-with-timeout wait-reason timeout predicate)
  Lucid             (lcl:process-wait-with-timeout wait-reason timeout predicate)
  Genera     (sys:process-wait-with-timeout wait-reason (* timeout 60.) predicate)
  CCL-2             (ccl::process-wait-with-timeout wait-reason timeout predicate)
  Clozure    (ccl:process-wait-with-timeout wait-reason timeout predicate)
  otherwise  (warn "No implementation of PROCESS-WAIT-WITH-TIMEOUT for this system.")
  }
  )

(defun process-interrupt (process function)
  (declare #+CCL-2 (ignore process))
  #{
  Lucid     (lcl:interrupt-process process function)
  allegro   (mp:process-interrupt process function)
  lispworks (mp:process-interrupt process function)
  Genera    (scl:process-interrupt process function)
  CCL-2     (let ((*current-process* :interrupt)) (funcall function))
  Clozure   (ccl:process-interrupt process function)
  Minima    (minima:process-interrupt process function)
  otherwise (warn "No implementation of PROCESS-INTERRUPT for this system.")
  }
  )

(defun restart-process (process)
  #{
  Lucid (lcl::restart-process process)
  allegro (mp:process-reset process)
  Clozure (ccl:process-reset process)
  lispworks (mp:process-reset process)
  Genera (process:process-reset process)
  Minima (minima:process-reset process)
  otherwise (warn "No implementation of RESTART-PROCESS for this system.")
  }
  )

(defun enable-process (process)
  #{
  Lucid (lcl::activate-process process)
  allegro (mp:process-enable process)
  Clozure (ccl:process-enable process)
  lispworks (mp:process-enable process)
  Genera (process:process-enable process)
  Minima (minima:process-enable process)
  otherwise (warn "No implementation of ENABLE-PROCESS for this system.")
  }
  )

(defun disable-process (process)
  #{
  Lucid (lcl::deactivate-process process)
  allegro (mp:process-disable process)
  Clozure (ccl:process-reset process)
  lispworks (mp:process-disable process)
  Genera (process:process-disable process)
  Minima (minima:process-disable process)
  otherwise (warn "No implementation of DISABLE-PROCESS for this system.")
  }
  )

(defun process-name (process)
  #{
  Lucid (lcl::process-name process)
  allegro (mp:process-name process)
  Clozure  (ccl:process-name process)
  lispworks (mp:process-name process)
  Genera (process:process-name process)
  Minima (minima:process-name process)
  otherwise (warn "No implementation of PROCESS-NAME for this system.")
  }
  )

(defun process-state (process)
  #{
  Lucid (lcl::process-state process)
  allegro (cond ((mp:process-active-p process) "active")
                ((mp:process-runnable-p process) "runnable")
                (t "deactivated"))
  lispworks (cond ((mp:process-active-p process) "active")
                  ((mp:process-runnable-p process) "runnable")
                  (t "deactivated"))
  Genera (process:process-state process)
  Minima (minima:process-state process)
  Clozure (ccl:process-whostate process)
  otherwise (warn "No implementation of PROCESS-STATE for this system.")
  }
  )

(defun process-whostate (process)
  #{
  Lucid (lcl::process-whostate process)
  allegro (mp:process-whostate process)
  Clozure (ccl:process-whostate process)
  lispworks (mp:process-whostate process)
  Genera (process:process-whostate process)
  Minima (minima:process-whostate process)
  otherwise (warn "No implementation of PROCESS-WHOSTATE for this system.")
  }
  )
