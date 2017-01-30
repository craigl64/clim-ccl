;;; -*- Mode: Lisp; Syntax: ANSI-Common-Lisp; Package: SILICA; Base: 10; Lowercase: Yes -*-
;; See the file LICENSE for the full license governing this code.
;;

(in-package :silica)

;;;"Copyright (c) 1991, 1992 Franz, Inc.  All rights reserved.
;;; Portions copyright (c) 1992 Symbolics, Inc.  All rights reserved."


(defgeneric process-next-event (port &key wait-function timeout))

(defgeneric distribute-event (port event))
(defgeneric dispatch-event (client event))
(defgeneric handle-event (client event))

(defmethod handle-event (sheet (event window-repaint-event))
  (repaint-sheet sheet (window-event-region event))
  (deallocate-event event))

(defmethod handle-event (sheet event)
  (declare (ignore sheet event))
  #+++ignore (warn "Ignoring event ~S on sheet ~S" sheet event)
  nil)

(defgeneric port-input-focus-selection (port))
(defgeneric port-keyboard-input-focus (port))

(defgeneric note-sheet-gain-focus (sheet))
(defgeneric note-sheet-lose-focus (sheet))

(defmethod note-sheet-gain-focus ((sheet basic-sheet)) nil)
(defmethod note-sheet-lose-focus ((sheet basic-sheet)) nil)

(defmethod (setf port-keyboard-input-focus) :before (focus (port basic-port))
  (let ((old-focus (port-keyboard-input-focus port)))
    (when (and old-focus
               (not (eq focus old-focus)))
      (note-sheet-lose-focus old-focus))
    (when focus
      (note-sheet-gain-focus focus))))

(defclass sheet-with-event-queue-mixin ()
    ((event-queue :initform nil :initarg :event-queue
                  :accessor sheet-event-queue)))

(defmethod note-sheet-grafted :after ((sheet sheet-with-event-queue-mixin))
  (unless (sheet-event-queue sheet)
    (setf (sheet-event-queue sheet)
          (or (do ((sheet (sheet-parent sheet) (sheet-parent sheet)))
                  ((null sheet))
                (when (and (typep sheet 'sheet-with-event-queue-mixin)
                           (sheet-event-queue sheet))
                  (return (sheet-event-queue sheet))))
              (make-queue)))))

(defgeneric queue-event (client event))
(defmethod queue-event ((sheet sheet-with-event-queue-mixin) event)
  (let ((queue (sheet-event-queue sheet)))
    (when queue
      (queue-put queue event))))

(defgeneric event-read (client))
(defmethod event-read ((sheet sheet-with-event-queue-mixin))
  (let ((queue (sheet-event-queue sheet)))
    (loop
      (process-wait "Event" #'(lambda ()
                                (not (queue-empty-p queue))))
      (let ((event (queue-get queue)))
        (when event (return event))))))


(defgeneric event-read-no-hang (client))
(defmethod event-read-no-hang ((sheet sheet-with-event-queue-mixin))
  (queue-get (sheet-event-queue sheet) nil))

(defgeneric event-peek (client &optional event-type))
(defmethod event-peek ((sheet sheet-with-event-queue-mixin) &optional event-type)
  (loop
    (let ((event (event-read sheet)))
      (when (or (null event-type)
                (typep event event-type))
        (event-unread sheet event)
        (return-from event-peek event)))))

;; Is there a lock here?

(defgeneric event-unread (sheet event))
(defmethod event-unread ((sheet sheet-with-event-queue-mixin) event)
  (queue-unget (sheet-event-queue sheet) event))

(defgeneric event-listen (client))
(defmethod event-listen ((sheet sheet-with-event-queue-mixin))
  (not (queue-empty-p (sheet-event-queue sheet))))



;;; Standard

(defclass standard-sheet-input-mixin (sheet-with-event-queue-mixin) ())

(defmethod dispatch-event ((sheet standard-sheet-input-mixin) event)
  (queue-event sheet event))

(defmethod dispatch-event ((sheet standard-sheet-input-mixin)
                           (event window-configuration-event))
  (handle-event sheet event))


;;; Immediate

(defclass immediate-sheet-input-mixin (sheet-with-event-queue-mixin) ())

(defmethod dispatch-event ((sheet immediate-sheet-input-mixin) event)
  (handle-event sheet event))


;;; Delegate

(defclass delegate-sheet-input-mixin ()
    ((delegate :accessor delegate-sheet-delegate)))

(defmethod dispatch-event ((sheet delegate-sheet-input-mixin) event)
  (dispatch-event (delegate-sheet-delegate sheet) event))

(defclass sheet-mute-input-mixin () ())

(defmethod queue-event ((sheet sheet-mute-input-mixin) (event event))
  nil)


;;; Repaint protocol

(defgeneric queue-repaint (sheet repaint-event))
(defmethod queue-repaint (sheet repaint-event)
  (queue-event sheet repaint-event))

(defgeneric repaint-sheet (sheet region))

(defmethod repaint-sheet ((sheet basic-sheet) region)
  (handle-repaint sheet (or region +everywhere+)))        ;--- kludge for null region

(defmethod repaint-sheet :around ((sheet basic-sheet) region)
  (declare (ignore region))
  (when (port sheet)
    (call-next-method)))

(defmethod repaint-sheet :after ((sheet sheet-parent-mixin) region)
  (when (null region)                                        ;--- kludge
    (setq region +everywhere+))
  (flet ((repaint-sheet-1 (child)
           (unless (sheet-direct-mirror child)
             (repaint-sheet
               child (untransform-region (sheet-transformation child) region)))))
    (declare (dynamic-extent #'repaint-sheet-1))
    (map-over-sheets-overlapping-region #'repaint-sheet-1 sheet region)))

(defgeneric handle-repaint (sheet region))


(defclass standard-repainting-mixin () ())

(defmethod queue-repaint ((sheet standard-repainting-mixin) repaint-event)
  (queue-event sheet repaint-event))

(defclass immediate-repainting-mixin () ())

(defmethod queue-repaint ((sheet immediate-repainting-mixin) repaint-event)
  (repaint-sheet sheet (window-event-region repaint-event)))

(defclass sheet-mute-repainting-mixin () ())

(defmethod queue-repaint ((sheet sheet-mute-repainting-mixin) repaint-event)
  (queue-event sheet repaint-event))

(defmethod handle-repaint ((sheet sheet-mute-repainting-mixin) region)
  (declare (ignore region))
  nil)



;;; Crossing events

;; We have fun here to generate enter/exit events for unmirrored sheets
;; Three cases:
;; 1. Mouse motion event:
;;    [Last mirrored sheet, position] --> [new mirrored sheet, position]
;; 2. Enter: [last-mirrored sheet, position] -> [new sheet]
;; 3. Exit: [last-mirrored sheet, position] -> [new sheet]
;;
;; Note that exit events on the parent are generated when the pointer goes
;; from a parent to a child, and that enter events on the parent are generated
;; when the pointer goes back from the child to the parent again.  If these
;; events are not interesting, the user should ignore all pointer enter and
;; exit events that have a kind of :INFERIOR.
;;
;; We boil this down to mouse moved to [new mirrored sheet, position]
;;  We have a current stack of sheets 0...n
;;  Find m such that 0...m - 1 contain mouse
;;  m...n require exit events
;; We can then go deep down inside m sheet generating enter events
;;
;; The 0th element of the trace is the top level mirrored sheet.
;;
;; Cases:
;; 1. It is the mirrored sheet.
;; 2. Ancestor: We have gone deeper.  Perhaps it still counts as exiting.
;;    We should just exit from the innermost sheet that encloses the new mirror.
;;    We should enter/exit intermediate ancestors.
;;    Ancestor-x,y,z-->Sheet
;; 3. Child: We have gone shallower.  Exit from all of the sheets.
;;    We should probably exit from intermediate descendents.
;;    Sheet- x,y,z --> Child
;; 4. Share a common ancestor: exit from all of the sheets.

;; This is the version for ports that support deep mirroring.
;; There is a lot of unnecessary hair in this code because a sheet's
;; region is not always correct in some implementations!
(defun generate-deeply-mirrored-crossing-events (port event)
  (declare (optimize (speed 3)))
  (let* ((mirrored-sheet (event-sheet event))
         (toplevel-sheet mirrored-sheet)
         (x (pointer-event-native-x event))
         (y (pointer-event-native-y event))
         (modifiers (event-modifier-state event))
         (pointer (pointer-event-pointer event))
         (ancestors-of-mirrored-sheet
           (do ((sheets nil)
                (sheet (sheet-parent mirrored-sheet) (sheet-parent sheet)))
               ((graftp sheet) sheets)
             (push sheet sheets))))
    (macrolet ((generate-enter-event (sheet kind)
                 `(let ((sheet ,sheet))
                    (when (eq (port-input-focus-selection port) :sheet-under-pointer)
                      (setf (port-keyboard-input-focus port) sheet))
                    (dispatch-event
                      sheet
                      (allocate-event 'pointer-enter-event
                        :sheet sheet
                        :native-x x :native-y y
                        :x x :y y
                        :kind ,kind
                        :modifier-state modifiers
                        :pointer pointer))))
               (generate-exit-event (sheet kind)
                 `(let ((sheet ,sheet))
                    (when (and (eq (port-input-focus-selection port)
                                   :sheet-under-pointer)
                               (eq (port-keyboard-input-focus port) sheet))
                      (setf (port-keyboard-input-focus port) nil))
                    (dispatch-event
                      sheet
                      (allocate-event 'pointer-exit-event
                        :sheet sheet
                        :native-x x :native-y y
                        :x x :y y
                        :kind ,kind
                        :modifier-state modifiers
                        :pointer pointer)))))

      ;; If we got an event for some mirrored child, go back to the
      ;; top level sheet.
      (unless (graftp (sheet-parent mirrored-sheet))
        (let ((new-x x)
              (new-y y))
          (dolist (sheet (cons mirrored-sheet (reverse ancestors-of-mirrored-sheet)))
            (when (graftp (sheet-parent sheet))
              (setq x new-x
                    y new-y
                    toplevel-sheet sheet)
              (return))
            (multiple-value-setq (new-x new-y)
              (map-sheet-position-to-parent sheet new-x new-y)))))

      (let* ((v (port-trace-thing port))
             (exitted-a-window
               (and (typep event 'pointer-exit-event)
                    (not (eq (pointer-boundary-event-kind event) :inferior))))
             (entered-from-child
               (and (typep event 'pointer-enter-event)
                    (eq (pointer-boundary-event-kind event) :inferior)))
             (exitted-top-level-window
               (and exitted-a-window
                    (eq toplevel-sheet mirrored-sheet))))
        (declare (type vector v))
        (when (or (not exitted-a-window)
                  exitted-top-level-window)
          ;; Pop up the stack of sheets
          (unless (zerop (fill-pointer v))
            (let ((m (if (and (not exitted-top-level-window)
                              (eq (aref v 0) toplevel-sheet))
                         (let ((new-x x)
                               (new-y y)
                               (mirrored-sheet-in-trace
                                 (position mirrored-sheet v :test #'eq))
                               sheet)
                           (if (and mirrored-sheet-in-trace
                                    (> mirrored-sheet-in-trace 1))
                               (do ((i 1 (1+ i)))
                                   ((> i mirrored-sheet-in-trace))
                                 (multiple-value-setq (new-x new-y)
                                   (map-sheet-position-to-child
                                     (aref v i) new-x new-y))))
                           (do ((i (if mirrored-sheet-in-trace
                                       (1+ mirrored-sheet-in-trace) 0) (1+ i)))
                               ((>= i (fill-pointer v)) (fill-pointer v))
                             (setq sheet (aref v i))
                             (unless (zerop i)
                               (multiple-value-setq (new-x new-y)
                                 (map-sheet-position-to-child sheet new-x new-y)))
                             (when (or (and (not mirrored-sheet-in-trace)
                                            (not (member sheet ancestors-of-mirrored-sheet)))
                                       (not (sheet-enabled-p sheet))
                                       (not (region-contains-position-p
                                              (sheet-region sheet) new-x new-y)))
                               (return i))))
                         0)))
              (do ((i (1- (fill-pointer v)) (1- i)))
                  ((< i m))
                (generate-exit-event (aref v i) :ancestor)
                (unless (zerop i)
                  (generate-enter-event (aref v (1- i)) :inferior)))
              (setf (fill-pointer v) m))))
        (when (and (not exitted-a-window)
                   (not entered-from-child)
                   (region-contains-position-p (sheet-region toplevel-sheet) x y))
          ;; If its empty initialize it with the toplevel sheet.
          (when (zerop (fill-pointer v))
            (vector-push-extend toplevel-sheet v)
            (generate-enter-event toplevel-sheet :ancestor))
          ;; Now add all sheets between the last sheet on the trace
          ;; and the mirrored sheet.
          (unless (find mirrored-sheet v :test #'eq)
            (let ((last-sheet (aref v (1- (fill-pointer v))))
                  (sheets nil))
              (do ((sheet mirrored-sheet (sheet-parent sheet)))
                  ((or (eq sheet last-sheet)
                       (when (graftp sheet)
                         (setq sheets nil)
                         t)))
                (push sheet sheets))
              (dolist (sheet sheets)
                (generate-exit-event (aref v (1- (fill-pointer v))) :inferior)
                (generate-enter-event sheet :ancestor)
                (vector-push-extend sheet v))))
          ;; We have to get the sheets into the correct coordinate space
          (loop for i from 1 below (fill-pointer v)
                do (multiple-value-setq (x y)
                     (map-sheet-position-to-child (aref v i) x y)))
          ;; Finally add progeny of mirrored-sheet.
          (let ((new-x x)
                (new-y y)
                (sheet (aref v (1- (fill-pointer v))))
                child)
            (loop
              (unless (typep sheet 'sheet-parent-mixin)
                (return nil))
              (setq child (child-containing-position sheet new-x new-y))
              (unless child
                (return nil))
              (generate-exit-event sheet :inferior)
              (generate-enter-event child :ancestor)
              (multiple-value-setq (new-x new-y)
                (map-sheet-position-to-child child new-x new-y))
              (setq sheet child)
              (vector-push-extend child v))))))))

;; This is the version for ports that do not have deep mirroring.
(defun generate-crossing-events (port event)
  (declare (optimize (speed 3)))
  (let* ((mirrored-sheet (event-sheet event))
         (toplevel-sheet mirrored-sheet)
         (x (pointer-event-native-x event))
         (y (pointer-event-native-y event))
         (modifiers (event-modifier-state event))
         (pointer (pointer-event-pointer event)))
    (macrolet ((generate-enter-event (sheet kind)
                 `(let ((sheet ,sheet))
                    (when (eq (port-input-focus-selection port) :sheet-under-pointer)
                      (setf (port-keyboard-input-focus port) sheet))
                    (dispatch-event
                      sheet
                      (allocate-event 'pointer-enter-event
                        :sheet sheet
                        :native-x x :native-y y
                        :x x :y y
                        :kind ,kind
                        :modifier-state modifiers
                        :pointer pointer))))
               (generate-exit-event (sheet kind)
                 `(let ((sheet ,sheet))
                    (when (and (eq (port-input-focus-selection port)
                                   :sheet-under-pointer)
                               (eq (port-keyboard-input-focus port) sheet))
                      (setf (port-keyboard-input-focus port) nil))
                    (dispatch-event
                      sheet
                      (allocate-event 'pointer-exit-event
                        :sheet sheet
                        :native-x x :native-y y
                        :x x :y y
                        :kind ,kind
                        :modifier-state modifiers
                        :pointer pointer)))))
      ;; If we got an event for some mirrored child, go back to the
      ;; top level sheet.
      (unless (graftp (sheet-parent mirrored-sheet))
        (do* ((sheet mirrored-sheet)
              (parent (sheet-parent sheet) (sheet-parent sheet))
              (new-x x)
              (new-y y))
             ((graftp parent)
              (setq x new-x
                    y new-y
                    toplevel-sheet sheet))
          (multiple-value-setq (new-x new-y)
            (map-sheet-position-to-parent sheet new-x new-y))
          (setq sheet parent)))
      (let* ((v (port-trace-thing port))
             (exitted-a-window
               (and (typep event 'pointer-exit-event)
                    (not (eq (pointer-boundary-event-kind event) :inferior))))
             (entered-from-child
               (and (typep event 'pointer-enter-event)
                    (eq (pointer-boundary-event-kind event) :inferior)))
             (exitted-top-level-window
               (and exitted-a-window
                    (eq toplevel-sheet mirrored-sheet))))
        (declare (type vector v))
        (when (or (not exitted-a-window)
                  exitted-top-level-window)
          ;; Pop up the stack of sheets
          (unless (zerop (fill-pointer v))
            (let ((m (if (and (not exitted-top-level-window)
                              (eq (aref v 0) toplevel-sheet))
                         (let ((new-x x)
                               (new-y y)
                               (pos (position mirrored-sheet v)))
                           (dotimes (i (fill-pointer v) (fill-pointer v))
                             (unless (zerop i)
                               (multiple-value-setq (new-x new-y)
                                 (map-sheet-position-to-child (aref v i) new-x new-y)))
                             (unless (or (and pos (<= i pos))
                                         (and (sheet-enabled-p (aref v i))
                                              (region-contains-position-p
                                                (sheet-region (aref v i)) new-x new-y)))
                               (return i))))
                         0)))
              (do ((i (1- (fill-pointer v)) (1- i)))
                  ((< i m))
                (generate-exit-event (aref v i) :ancestor)
                (unless (zerop i)
                  (generate-enter-event (aref v (1- i)) :inferior)))
              (setf (fill-pointer v) m))))
        (when (and (not exitted-a-window)
                   (not entered-from-child)
                   (region-contains-position-p (sheet-region toplevel-sheet) x y))
          ;; If its empty initialize it with the toplevel sheet.
          (when (zerop (fill-pointer v))
            (vector-push-extend toplevel-sheet v)
            (generate-enter-event toplevel-sheet :ancestor))
          ;; Now add all sheets between the last sheet on the trace
          ;; and the mirrored sheet.
          (unless (find mirrored-sheet v)
            (let ((last-sheet (aref v (1- (fill-pointer v))))
                  (sheets nil))
              (do ((sheet mirrored-sheet (sheet-parent sheet)))
                  ((or (graftp sheet) (eq sheet last-sheet)))
                (push sheet sheets))
              (dolist (sheet sheets)
                (generate-exit-event (aref v (1- (fill-pointer v))) :inferior)
                (generate-enter-event sheet :ancestor)
                (vector-push-extend sheet v))))
          ;; We have to get the sheets into the correct coordinate space
          (loop for i from 1 below (fill-pointer v)
                do (multiple-value-setq (x y)
                     (map-sheet-position-to-child (aref v i) x y)))
          ;; Finally add progeny of mirrored-sheet.
          (let ((new-x x)
                (new-y y)
                (sheet (aref v (1- (fill-pointer v))))
                child)
            (loop
              (unless (typep sheet 'sheet-parent-mixin)
                (return nil))
              (setq child (child-containing-position sheet new-x new-y))
              (unless child
                (return nil))
              (generate-exit-event sheet :inferior)
              (generate-enter-event child :ancestor)
              (multiple-value-setq (new-x new-y)
                (map-sheet-position-to-child child new-x new-y))
              (setq sheet child)
              (vector-push-extend child v))))))))


;;; Event distribution

(defmethod distribute-event ((port basic-port) event)
  (distribute-event-1 port event))

(defmethod distribute-event ((port null) (event event))
  #+++ignore (warn "Trying to distribute event ~S for a null port" event))

;;; Methods which maintain the state of the modifiers in the port and
;;; the buttons in the pointer.

(defmethod distribute-event :before ((port basic-port) (event device-event))
  (setf (port-modifier-state port) (event-modifier-state event)))

(defmethod distribute-event :before ((port basic-port) (event pointer-button-press-event))
  (let ((pointer (pointer-event-pointer event)))
    (when (pointer-event-button event)
      (setf (pointer-button-state pointer)
            (logior (pointer-button-state pointer)
                    (pointer-event-button event))))))

(defmethod distribute-event :before ((port basic-port) (event pointer-button-release-event))
  (let ((pointer (pointer-event-pointer event)))
    (when (pointer-event-button event)
      (setf (pointer-button-state pointer)
            (logandc2 (pointer-button-state pointer) (pointer-event-button event))))))

;;;

(defgeneric distribute-event-1 (port event))

(defmethod distribute-event-1 ((port basic-port) (event event))
  (dispatch-event (event-sheet event) event))

(defmethod distribute-event-1 ((port basic-port) (event keyboard-event))
  (let ((sheet (port-keyboard-input-focus port)))
    ;;---is it ok to setf an event's sheet??
    (when sheet
      (setf (slot-value event 'sheet) sheet)
      (dispatch-event sheet event))))

(defmethod distribute-event-1 ((port basic-port) (event focus-in-event))
  (setf (port-keyboard-input-focus port) (event-sheet event)))

(defmethod distribute-event-1 ((port basic-port) (event focus-out-event))
  (setf (port-keyboard-input-focus port) nil))

(defmethod distribute-event-1 ((port basic-port) (event window-event))
  (dispatch-event
    (window-event-mirrored-sheet event)
    event))

(defmethod distribute-event-1 ((port basic-port) (event pointer-event))
  (declare (optimize (speed 3)))
  (let ((sheet (let ((v (port-trace-thing port)))
                 (and (not (zerop (fill-pointer v)))
                      (aref v (1- (fill-pointer v)))))))
    (dispatch-pointer-event-to-sheet port event sheet)))

(defmethod distribute-event-1 :before ((port basic-port) (event pointer-event))
  ;; Generate all the correct enter/exit events.
  (if (port-deep-mirroring port)
      (generate-deeply-mirrored-crossing-events port event)
    (generate-crossing-events port event)))

(defmethod distribute-event-1 :around ((port basic-port) (event pointer-event))
  (let ((sheet (port-grabbing-sheet port)))
    (if sheet
        (let ((event-sheet (event-sheet event))
              (grab-sheet (sheet-mirrored-ancestor sheet)))
          ;; If the grabbing sheet is different from the sheet the
          ;; event was reported on it is necessary to transfer the
          ;; co-ordinates. (cim 10/14/94 - see clim2bug688)
          (unless (eq event-sheet grab-sheet)
            (let ((x (pointer-event-native-x event))
                  (y (pointer-event-native-y event)))
              (loop
                (unless event-sheet
                  (return))
                (multiple-value-setq (x y)
                  (transform-position (sheet-transformation event-sheet) x y))
                (setq event-sheet (sheet-parent event-sheet)))
              (loop
                (unless grab-sheet
                  (return))
                (multiple-value-setq (x y)
                  (untransform-position (sheet-transformation grab-sheet) x y))
                (setq grab-sheet (sheet-parent grab-sheet)))
              (with-slots (native-x native-y) event
                (setf native-x x native-y y))))
          (dispatch-pointer-event-to-sheet port event sheet))
      (call-next-method))))

(defmethod dispatch-pointer-event-to-sheet ((port basic-port) (event event) sheet)
  (let ((event-type (class-name (class-of event)))
        (x (pointer-event-native-x event))
        (y (pointer-event-native-y event))
        (modifiers (event-modifier-state event))
        (pointer (pointer-event-pointer event)))
    ;; Dispatch event to the innermost sheet
    (when (and sheet (port sheet))
      (multiple-value-bind (tx ty)
          (untransform-position (sheet-device-transformation sheet) x y)
        (typecase event
          (pointer-button-event
           (dispatch-event
            sheet
            (allocate-event event-type
                            :sheet sheet
                            :native-x x :native-y y
                            :x tx :y ty
                            :modifier-state modifiers
                            :button (pointer-event-button event)
                            :pointer pointer)))
          (pointer-motion-event
           (dispatch-event
            sheet
            (allocate-event event-type
                            :sheet sheet
                            :native-x x :native-y y
                            :x tx :y ty
                            :modifier-state modifiers
                            :pointer pointer)))
          ;; Pointer exit and enter events are handled by GENERATE-CROSSING-EVENTS
          ))))
  (deallocate-event event))

(defmethod dispatch-pointer-event-to-sheet :before
           ((port basic-port) (event pointer-button-press-event) sheet)
  (let ((button (pointer-event-button event)))
    (when (and (eq (port-input-focus-selection port) :click-to-select)
               (eq button +pointer-left-button+))
      (setf (port-keyboard-input-focus port) sheet))))

(defmethod dispatch-event :before ((sheet t) (event pointer-motion-event))
  (update-pointer event))

(defmethod dispatch-event :before ((sheet t) (event pointer-button-event))
  (update-pointer event))

(defmethod dispatch-event :before ((sheet t) (event pointer-enter-event))
  ;;-- We should just wait for mouse movements
  #+ignore (update-pointer event))

(defmethod dispatch-event :before ((sheet t) (event pointer-exit-event))
  ;;-- This should mark the pointer state as invalid
  (with-slots (pointer) event
    (setf (clim-internals::pointer-valid-p pointer) nil)))

;; pointer-exit-event
;; These events dont have the correct native-x,y coordinates
;; but perhaps all we need to do is for the pointer-exit-event to mark
;; things as invalid.

(defun update-pointer (event)
  (with-slots (x y native-x native-y pointer sheet) event
    ;; Update the pointer object
    ;;--- It might make a lot more sense for methods on
    ;;--- dispatch-event to update the pointer
    ;;--- This would enable exit events to do the right thing
    ;;--- Hence if we exit then we set the sheet to NIL??
    (setf (clim-internals::pointer-valid-p pointer) t
          (pointer-sheet pointer) sheet
          (pointer-x-position pointer) x
          (pointer-y-position pointer) y
          (pointer-native-x-position pointer) native-x
          (pointer-native-y-position pointer) native-y)
    (setf (pointer-cursor pointer)
      (or (sheet-pointer-cursor sheet) :default))))

;;; Local event processing

(defun process-event-locally (sheet event)
  (declare (ignore sheet))
  (handle-event (event-sheet event) event))

(defun local-event-loop (sheet)
  (loop
    (process-event-locally sheet (event-read sheet))))

(defmethod port-event-wait (port waiter
			    &key (wait-reason 
				  #+Genera si:*whostate-awaiting-user-input*
				  #-Genera "CLIM Input")
				 timeout)
  (cond (*multiprocessing-p*
         (assert (and port (port-alive-p port)) () "The port is not alive")
         (process-wait-with-timeout wait-reason timeout waiter)
         (values))
        (t
	 ;; Single process, so run the event processing loop right here
         (process-next-event port
                             :timeout timeout
                             :wait-function waiter
                             :state wait-reason))))


;;; Event resources

(defvar *resourced-events* nil)

(defgeneric initialize-event (event &key &allow-other-keys))

(defmacro define-event-resource (event-class nevents)
  ;; Gosh, this macro is bad style, and violates the MOP.
  ;; Piling warts upon warts, we have to force finalization at
  ;; macroexpand time... - smh 18may93
  (#+allegro clos:finalize-inheritance 
   #+aclpc acl:finalize-inheritance
   #+Clozure ccl:finalize-inheritance
   #-(or allegro aclpc Clozure) cl:finalize-inheritance
   (find-class event-class)) ;smh 18may93
  (let* ((slots #+aclpc  (acl:class-slots (find-class event-class))
                #+Clozure (ccl:class-slots (find-class event-class))
                #+(and MCL CCL-2) (ccl:class-slots (find-class event-class))
                #-(or (and MCL CCL-2) aclpc Clozure)
                (clos:class-slots (find-class event-class)))
         (slot-names #+aclpc (mapcar #'acl:slot-definition-name slots)
                     #+Clozure (mapcar #'ccl:slot-definition-name slots)
                     #+(and MCL CCL-2) (mapcar #'ccl:slot-definition-name slots)
                     #-(or (and MCL CCL-2) aclpc Clozure)
                     (mapcar #'clos:slot-definition-name slots))
         (resource-name (fintern "*~A-~A*" event-class 'resource)))
    `(progn
       (defvar ,resource-name nil)
       (defmethod initialize-event ((event ,event-class) &key ,@slot-names)
         ,@(mapcar #'(lambda (slot)
                       (if (eq slot 'timestamp)
                           `(setf (slot-value event ',slot)
                              (or ,slot (atomic-incf *event-timestamp*)))
                         `(setf (slot-value event ',slot) ,slot)))
                   slot-names))
       (let ((old (assoc ',event-class *resourced-events*)))
         (unless old
           (setq *resourced-events*
             (append *resourced-events*
                     (list (list ',event-class ',resource-name
                                 ;; Allocates, misses, creates, deallocates
                                 #+meter-events ,@(list 0 0 0 0)))))))
       ;; When an event is in the event resource, we use the timestamp
       ;; to point to the next free event
       (let ((previous-event (make-instance ',event-class)))
         (setq ,resource-name previous-event)
         (repeat ,(1- nevents)
                 (let ((event (make-instance ',event-class
                                             :timestamp nil)))
                   (setf (slot-value previous-event 'timestamp) event)
                   (setq previous-event event)))))))

(define-event-resource pointer-motion-event 20)
(define-event-resource pointer-enter-event 20)
(define-event-resource pointer-exit-event 20)
(define-event-resource pointer-button-press-event 10)
(define-event-resource pointer-button-release-event 10)
(define-event-resource key-press-event 10)
(define-event-resource key-release-event 10)
(define-event-resource window-configuration-event 10)
(define-event-resource window-repaint-event 10)

(defparameter *use-event-resources* nil)

;; Note that this assumes that the initarg for a slot has the same
;; symbol name as the slot itself
#+Genera (zwei:defindentation (allocate-event 1 1))
(defun allocate-event (event-class &rest initargs)
  (declare (dynamic-extent initargs))
  (let* ((entry (assoc event-class *resourced-events*))
         (resource (second entry))
         event)
    ;; Update the freelist, or allocate a new event if necessary
    #+meter-events (when resource (incf (nth 2 entry)))
    (cond ((and *use-event-resources* resource)
           (without-scheduling
             (setq event (symbol-value resource))
             (when event
               (setf (symbol-value resource) (event-timestamp event))))
           ;; INITIALIZE-EVENT is slower than an optimized MAKE-INSTANCE
           ;; when no slots have to be initialized, but usually we have
           ;; to fill in most of the slots, in which case it is about the
           ;; same speed.
           (cond (event
                  (apply #'initialize-event event initargs)
                  event)
                 (t
                  #+meter-events (incf (nth 3 entry))
                  (apply #'make-instance event-class initargs))))
          (t
           #+meter-events (when resource (incf (nth 4 entry)))
           (apply #'make-instance event-class initargs)))))

(defun deallocate-event (event)
  (when *use-event-resources*
    (unless (typep (event-timestamp event) 'fixnum)
      (cerror "Proceed anyway"
              "The event ~S has already been deallocated" event)
      (return-from deallocate-event nil))
    (let* ((entry (assoc (class-name (class-of event)) *resourced-events*))
           (resource (second entry)))
      (when resource
        (without-scheduling
          (setf (slot-value event 'timestamp) (symbol-value resource))
          (setf (symbol-value resource) event))
        #+meter-events (incf (nth 5 entry)))))
  nil)

#+meter-events
(defun describe-event-resources (&optional reset)
  (format t "~&Name~32TAlloc    Miss     New   Freed")
  (dolist (entry *resourced-events*)
    (destructuring-bind (name first allocates misses new deallocates) entry
      (declare (ignore first))
      (format t "~&~A~30T~7D ~7D ~7D ~7D" name allocates misses new deallocates)
      (when reset
        (setf (nth 2 entry) 0
              (nth 3 entry) 0
              (nth 4 entry) 0
              (nth 5 entry) 0)))))

;; It's generally better to copy the event when passing it up to the API
;; level, because that ensures that our event resource remains relatively
;; localized in virtual memory.
#+Symbolics
(defun-inline copy-event (event)
  (clos-internals::%allocate-instance-copy event))

#+CCL-2
(defun copy-event (event)
  (without-scheduling
    (ccl::copy-uvector
      (ccl::%maybe-forwarded-instance event))))

#-(or Symbolics (and MCL CCL-2))
(defun copy-event (event)
  (let* ((class (class-of event))
         (copy (allocate-instance class)))
    (dolist (slot (#+aclpc acl:class-slots 
                   #+Clozure ccl:class-slots
                   #-(or aclpc Clozure) clos:class-slots class))
      (let ((name (#+aclpc acl:slot-definition-name 
                   #+Clozure ccl:slot-definition-name
                   #-(or aclpc Clozure) clos:slot-definition-name slot))
            (allocation (#+aclpc acl:slot-definition-allocation
                         #+Clozure ccl:slot-definition-allocation
                         #-(or aclpc Clozure) clos:slot-definition-allocation slot)))
        (when (and (eql allocation :instance)
                   (slot-boundp event name))
          (setf (slot-value copy name) (slot-value event name)))))
    copy))


#||
;; Debugging help

(defmethod print-object ((event pointer-boundary-event) stream)
  (print-unreadable-object (event stream :type t :identity nil)
    (let ((comma nil))
      (when (slot-boundp event 'sheet)
        (let ((sheet (event-sheet event)))
          (format stream "~:[~;mirrored ~]sheet ~s"
            (sheet-direct-mirror sheet) sheet))
        (setq comma t))
      (when (slot-boundp event 'kind)
        (if comma
            (write-string ", " stream))
        (format stream "kind ~s" (pointer-boundary-event-kind event))))))

(defmethod print-object ((event pointer-motion-event) stream)
  (print-unreadable-object (event stream :type t :identity nil)
    (let ((comma nil))
      (when (slot-boundp event 'sheet)
        (let ((sheet (event-sheet event)))
          (format stream "~:[~;mirrored ~]sheet ~s"
            (sheet-direct-mirror sheet) sheet))
        (setq comma t))
      (when (slot-boundp event 'x)
        (if comma
            (write-string ", " stream))
        (format stream "x: ~d, y: ~d"
          (pointer-event-x event) (pointer-event-y event))
        (setq comma t))
      (when (slot-boundp event 'native-x)
        (if comma
            (write-string ", " stream))
        (format stream "nx: ~d, ny: ~d"
          (pointer-event-native-x event) (pointer-event-native-y event))))))

||#

