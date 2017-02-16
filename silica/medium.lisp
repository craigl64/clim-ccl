;;; -*- Mode: Lisp; Syntax: ANSI-Common-Lisp; Package: SILICA; Base: 10; Lowercase: Yes -*-
;; See the file LICENSE for the full license governing this code.
;;

(in-package :silica)

(define-condition unsupported-ink (error)
  ((ink :initarg :ink :reader unsupported-ink)
   (message :initarg :message :reader unsupported-ink-message))
  (:report (lambda (c s)
             (format s "Ink ~A is not supported by the backend: ~A."
                     (unsupported-ink c)
                     (unsupported-ink-message c)))))

;;;"Copyright (c) 1990, 1991, 1992 Symbolics, Inc.  All rights reserved.
;;; Portions copyright (c) 1991, 1992 Franz, Inc.  All rights reserved."

(defgeneric medium-drawable (medium))

(defmethod engraft-medium ((medium basic-medium) port sheet)
  (declare (ignore port sheet))
  nil)

;; Doing this in an :AROUND method ensures that when any :BEFORE methods 
;; run, the medium and the sheet are correctly associated
(defmethod engraft-medium :around ((medium basic-medium) port sheet)
  #-aclpc (declare (ignore port))
  (setf (medium-sheet medium) sheet)
  (call-next-method)
  nil)

(defmethod degraft-medium ((medium basic-medium) port sheet)
  (declare (ignore sheet port))
  nil)

(defmethod invoke-with-sheet-medium ((sheet basic-sheet) continuation)
  (declare (dynamic-extent continuation))
  (let ((medium (sheet-medium sheet)))
    (if medium
        (funcall continuation medium)
        (with-temporary-medium (medium sheet)
          (with-sheet-medium-bound (sheet medium)
            (funcall continuation medium))))))

;; Special-case the one we know is going to work all the time
(defmethod invoke-with-sheet-medium ((sheet permanent-medium-sheet-output-mixin) continuation)
  (declare (dynamic-extent continuation))
  (let ((medium (slot-value sheet 'medium)))
    (if medium
        (funcall continuation medium)
        ;; Some gadgets won't have a medium while they are being created.
        ;; Go get one now so that foreground/background can be decoded, etc.
        (call-next-method))))

(defmethod invoke-with-sheet-medium ((sheet shared-medium-sheet-output-mixin) continuation)
  (declare (dynamic-extent continuation))
  (let ((medium (sheet-medium sheet)))
    (if medium
        ;; Note that we do not go through ENGRAFT-MEDIUM or anything
        ;; like that, since the sheets are assumed to be closely enough
        ;; related that ENGRAFT-MEDIUM shouldn't do anything useful.
        (letf-globally (((sheet-medium sheet) medium)
                        ((medium-sheet medium) sheet))
          ;; Be sure any clipping regions are decached
          (setf (medium-clipping-region medium) +everywhere+)
          (funcall continuation medium))
        (call-next-method))))

;;--- Use DEFOPERATION
(defmethod invoke-with-sheet-medium ((stream standard-encapsulating-stream) continuation)
  (declare (dynamic-extent continuation))
  (invoke-with-sheet-medium (encapsulating-stream-stream stream) continuation))

;; Note that we do not degraft the medium.  This is because we don't
;; want to release any of the medium's resource (GCs, etc).  If you want
;; the medium degrafted, just do it yourself.
(defmethod invoke-with-sheet-medium-bound (sheet medium continuation)
  (declare (dynamic-extent continuation))
  (cond ((sheet-medium sheet)
         (funcall continuation))
        (medium
         (letf-globally (((sheet-medium sheet) medium))
           (engraft-medium medium (port sheet) sheet)
           (funcall continuation)))
        (t
         (flet ((call-continuation (medium)
                  (declare (ignore medium))
                  (funcall continuation)))
           (declare (dynamic-extent #'call-continuation))
           (invoke-with-sheet-medium
             sheet #'call-continuation)))))

(defgeneric make-medium (port sheet))

(defgeneric medium-force-output (medium))
(defgeneric medium-finish-output (medium))
(defgeneric medium-beep (medium))

(defgeneric port-glyph-for-character (port character style &optional our-font))


(defvar *default-pane-foreground* +black+)
(defvar *default-pane-background* +white+)

;; removed out hard-wired white background hack for nt (cim 10/9/96)

(defclass sheet-with-resources-mixin ()
  ((foreground :initarg :foreground :accessor pane-foreground)
   (background :initarg :background :accessor pane-background)
   (text-style :initarg :text-style :accessor pane-text-style)
   (initargs :initform nil :reader sheet-with-resources-initargs))
  (:default-initargs :foreground nil :background nil
                     :text-style nil))
  
(defmethod initialize-instance :after 
           ((sheet sheet-with-resources-mixin) &rest args)
  (with-slots (initargs) sheet
    (setf initargs args)))


(defmethod get-sheet-resources ((port basic-port)
                                (sheet sheet-with-resources-mixin))
  nil)

(defmethod get-parent-initarg ((sheet basic-sheet) resource)
  (loop
    (when (null sheet)
      (return nil))
    (setq sheet (sheet-parent sheet))
    (when (typep sheet 'sheet-with-resources-mixin)
      (let ((r (getf (sheet-with-resources-initargs sheet) resource)))
        (when r (return r))))))

(defmethod note-sheet-grafted :before ((sheet sheet-with-resources-mixin))
  (let* ((port (port sheet))
         (palette (port-default-palette port))
         (initargs (sheet-with-resources-initargs sheet))
         (resources (get-sheet-resources port sheet)))
    (macrolet ((ensure-color (color)
                 (let ((c '#:c))
                   `(let ((,c ,color))
                      (etypecase ,c
                        (color ,c)
                        (string (find-named-color ,c palette))
                        (integer (make-device-color palette ,c))))))
               (get-resource (resource default)
                 `(or (getf initargs ,resource)
                      (get-parent-initarg sheet ,resource)
                      (getf resources ,resource)
                      ,default)))
      (with-slots (foreground background text-style) sheet
        (setf foreground (ensure-color
                          (get-resource :foreground *default-pane-foreground*))
              background (ensure-color
                          (get-resource :background *default-pane-background*))
              text-style (get-resource :text-style *default-text-style*))))))

(defmethod (setf pane-foreground) :after (ink (pane sheet-with-resources-mixin))
  (let ((m (sheet-direct-mirror pane)))
    (when m
      (port-set-pane-foreground (port pane) pane m ink))))

(defmethod (setf pane-background) :after (ink (pane sheet-with-resources-mixin))
  (let ((m (sheet-direct-mirror pane)))
    (when m
      (port-set-pane-background (port pane) pane m ink))))

(defmethod (setf pane-text-style) :after (text-style (pane sheet-with-resources-mixin))
  (let ((m (sheet-direct-mirror pane)))
    (when m
      (port-set-pane-text-style (port pane) pane m text-style))))

(defmethod engraft-medium :after
           ((medium basic-medium) (port t) (sheet sheet-with-resources-mixin))
  ;; We set the slots directly in order to avoid running any per-port
  ;; :AFTER methods (or whatever).  That work should be done by similar
  ;; per-port methods on ENGRAFT-MEDIUM.
  (with-slots (foreground background
               text-style default-text-style merged-text-style-valid)
      medium
    (setf foreground (pane-foreground sheet)
           background (pane-background sheet)
          default-text-style (parse-text-style (pane-text-style sheet))
          text-style nil
          merged-text-style-valid nil)))


(defclass pane-repaint-background-mixin () ())

(defmethod handle-repaint ((pane pane-repaint-background-mixin) region)
  (let ((clear (region-intersection
                 region
                 (or (pane-viewport-region pane)
                     (sheet-region pane)))))
    (unless (eq clear +nowhere+)
      (with-sheet-medium (medium pane)
        (with-bounding-rectangle* (left top right bottom) clear
          (medium-clear-area medium left top right bottom))))))


;;; Line styles

(define-protocol-class line-style ())

(defclass standard-line-style (line-style)
    ((unit :type (member :normal :point)
           :initform :normal :initarg :unit
           :reader line-style-unit)
     (thickness :type real
                :initform 1 :initarg :thickness
                :reader line-style-thickness)
     (joint-shape :type (member :miter :bevel :round :none)
                  :initform :miter :initarg :joint-shape
                  :reader line-style-joint-shape)
     (cap-shape :type (member :butt :square :round :no-end-point)
                :initform :butt :initarg :cap-shape
                :reader line-style-cap-shape)
     (dashes :initform nil :initarg :dashes
             :reader line-style-dashes)
     #+++ignore (initial-dash-phase :initform 0 :initarg :initial-dash-phase
                                    :reader line-style-initial-dash-phase)))

(defmethod print-object ((line-style standard-line-style) stream)
  (print-unreadable-object (line-style stream :type t :identity t)
    (with-slots (unit thickness joint-shape cap-shape dashes) line-style
      (format stream "Units ~(~A~), thickness ~D, joint ~(~A~), cap ~(~A~), dashes ~S"
              unit thickness joint-shape cap-shape dashes))))

(defvar +default-line-style+ (make-instance 'standard-line-style))
(defvar +dashed-line-styles+
        (make-array 5 :initial-contents
                        (list (make-instance 'standard-line-style :thickness 0 :dashes t)
                              (make-instance 'standard-line-style :thickness 1 :dashes t)
                              (make-instance 'standard-line-style :thickness 2 :dashes t)
                              (make-instance 'standard-line-style :thickness 3 :dashes t)
                              (make-instance 'standard-line-style :thickness 4 :dashes t))))
(defvar +undashed-line-styles+
        (make-array 5 :initial-contents
                        (list (make-instance 'standard-line-style :thickness 0 :dashes nil)
                              (make-instance 'standard-line-style :thickness 1 :dashes nil)
                              (make-instance 'standard-line-style :thickness 2 :dashes nil)
                              (make-instance 'standard-line-style :thickness 3 :dashes nil)
                              (make-instance 'standard-line-style :thickness 4 :dashes nil))))

(defun-inline make-line-style-1 (unit thickness dashes joint-shape cap-shape)
  #+Genera (declare lt:(side-effects simple reducible))
  (if (and (eq unit :normal)
           (eq joint-shape :miter)
           (eq cap-shape :butt)
           (integerp thickness) (<= 0 thickness 4)
           (or (eq dashes t) (eq dashes nil)))
      ;; Cache the common case when only :DASHES and :THICKNESS are provided
      (svref (if dashes +dashed-line-styles+ +undashed-line-styles+) thickness)
      (make-instance 'standard-line-style
        :unit unit :thickness thickness :dashes dashes
        :joint-shape joint-shape :cap-shape cap-shape)))

(defun make-line-style (&key (unit :normal) (thickness 1) (dashes nil)
                             (joint-shape :miter) (cap-shape :butt))
  #+Genera (declare lt:(side-effects simple reducible))
  (make-line-style-1 unit thickness dashes joint-shape cap-shape))

(defvar +highlighting-line-style+ (make-line-style :thickness 1))

(defmethod make-load-form ((line-style standard-line-style) #-aclpc &optional #-aclpc environment)
  #-aclpc (declare (ignore environment))
  (with-slots (unit thickness joint-shape cap-shape dashes) line-style
    `(make-line-style ,@(unless (eq unit :normal) `(:unit ,unit))
                      ,@(unless (= thickness 1) `(:thickness ,thickness))
                      ,@(unless (eq joint-shape :miter) `(:joint-shape ,joint-shape))
                      ,@(unless (eq cap-shape :butt) `(:cap-shape ,cap-shape))
                      ,@(unless (eq dashes nil) `(:dashes ,dashes)))))


(defmethod invoke-with-drawing-options ((sheet basic-sheet) continuation &rest options)
  (declare (dynamic-extent options))
  (with-sheet-medium (medium sheet)
    (apply #'invoke-with-drawing-options medium continuation options)))

;; For string streams, sigh
(defmethod invoke-with-drawing-options ((stream t) continuation &rest options)
  (declare (ignore options))
  (funcall continuation))

;;--- CLIM 1.0 had this stuff that frobbed the clipping region.  Is it right?
#+++ignore
(defmethod medium-clipping-region ((medium basic-medium))
  (with-slots (transformation transformed-clipping-region) medium
    (untransform-region transformation transformed-clipping-region)))

#+++ignore
(defmethod (setf medium-clipping-region) (clipping-region (medium basic-medium))
  (with-slots (transformation transformed-clipping-region) medium
    (setf transformed-clipping-region (transform-region transformation clipping-region)))
  clipping-region)

(defmacro with-medium-clipping-region ((medium region) &body body)
  `(flet ((with-medium-clipping-region-body (,medium) ,@body))
     (declare (dynamic-extent #'with-medium-clipping-region-body))
     (invoke-with-medium-clipping-region
       ,medium #'with-medium-clipping-region-body ,region)))

(defmethod invoke-with-medium-clipping-region
           ((medium basic-medium) continuation region)
  (let ((saved-region (medium-clipping-region medium)))
    (unwind-protect
        (progn
          (setf (medium-clipping-region medium) region)
          (funcall continuation medium))
      (setf (medium-clipping-region medium) saved-region))))

(defmethod invalidate-cached-regions ((medium basic-medium)) nil)
(defmethod invalidate-cached-transformations ((medium basic-medium)) nil)

;; NOTE: if you change the keyword arguments accepted by this method, you
;; also have to change the list of keywords in *ALL-DRAWING-OPTIONS*
(defmethod invoke-with-drawing-options
           ((medium basic-medium) continuation
            &key ink clipping-region transformation
                 line-style line-unit line-thickness (line-dashes nil dashes-p)
                 line-joint-shape line-cap-shape
                 (text-style nil text-style-p) (text-family nil text-family-p)
                 (text-face nil text-face-p) (text-size nil text-size-p))
  (with-accessors ((transformed-clipping-region medium-clipping-region)) medium
    (with-slots ((medium-ink ink)
                 (medium-transformation transformation)
                 (medium-line-style line-style)) medium
      (let* ((saved-ink medium-ink)
             (saved-transformation medium-transformation)
             (saved-clipping-region transformed-clipping-region)
             (saved-line-style medium-line-style))
        (unwind-protect
            (progn
              (when ink
                (setf medium-ink ink))
              (when transformation
                (setf medium-transformation
                      (compose-transformations saved-transformation transformation)))
              (when clipping-region
		(let ((tr (if (eq clipping-region +everywhere+)
			      +everywhere+
			    (transform-region medium-transformation clipping-region))))
		  (setf transformed-clipping-region
		    (if (eq tr +everywhere+)
			saved-clipping-region
		      (region-intersection saved-clipping-region tr)))))
              (cond ((or line-unit line-thickness line-joint-shape line-cap-shape dashes-p)
                     (when (null line-style)
                       (setf line-style saved-line-style))
                     (setf medium-line-style
                           (make-line-style-1
                             (or line-unit (line-style-unit line-style))
                             (or line-thickness (line-style-thickness line-style))
                             (if dashes-p line-dashes (line-style-dashes line-style))
                             (or line-joint-shape (line-style-joint-shape line-style))
                             (or line-cap-shape (line-style-cap-shape line-style)))))
                    (line-style
                     (setf medium-line-style line-style)))
              (when (or text-family-p text-face-p text-size-p)
                (if text-style-p
                    (setq text-style (with-stack-list (style text-family text-face text-size)
                                       (merge-text-styles style text-style)))
                    (setq text-style (make-text-style text-family text-face text-size)
                          text-style-p t)))
              (if text-style-p
                  (flet ((call-continuation (stream)
                           (declare (ignore stream))
                           (funcall continuation)))
                    (declare (dynamic-extent #'call-continuation))
                    (invoke-with-text-style medium #'call-continuation text-style medium))
                  (funcall continuation)))
          (setf medium-line-style saved-line-style)
          (setf transformed-clipping-region saved-clipping-region)
          (setf medium-transformation saved-transformation)
          (setf medium-ink saved-ink))))))


(defmethod allocate-medium (port sheet)
  (or (pop (port-medium-cache port))
      (make-medium port sheet)))

(defmethod deallocate-medium (port medium)
  (setf (medium-sheet medium) nil)
  (push medium (port-medium-cache port)))


;; Make sheets do the medium protocol

(defprotocol medium-protocol ()
  (:roles medium))

(defrole medium ()
  ((foreground :accessor medium-foreground)
   (background :accessor medium-background)
   (ink :accessor medium-ink)

   (line-style :accessor medium-line-style)
   (clipping-region :accessor medium-clipping-region)
   (transformation :accessor medium-transformation)
   (+y-upward-p :initform nil :accessor medium-+y-upward-p)

   (text-style :accessor medium-text-style)
   (default-text-style :accessor medium-default-text-style)
   (merged-text-style-valid :accessor medium-merged-text-style-valid)
   (merged-text-style :accessor medium-merged-text-style)))


(defmethod (setf medium-default-text-style) :before (new (medium basic-medium))
  (declare (ignore new))
  (setf (medium-merged-text-style-valid medium) nil))

(defmethod (setf medium-text-style) :before (new (medium basic-medium))
  (declare (ignore new))
  (setf (medium-merged-text-style-valid medium) nil))

(defmethod medium-merged-text-style ((medium basic-medium))
  (with-slots (text-style default-text-style
               merged-text-style merged-text-style-valid) medium
    (if merged-text-style-valid
        merged-text-style
        (prog1
          (setf merged-text-style (merge-text-styles text-style default-text-style))
          (setf merged-text-style-valid t)))))


(defmacro with-text-style ((medium style) &body body)
  (default-output-stream medium with-text-style)
  `(flet ((with-text-style-body (,medium) ,@body))
     (declare (dynamic-extent #'with-text-style-body))
     (invoke-with-text-style
       ,medium #'with-text-style-body ,style ,medium)))

(defmacro with-text-family ((medium family) &body body)
  `(with-text-style (,medium (make-text-style ,family nil nil)) ,@body))

(defmacro with-text-face ((medium face) &body body)
  `(with-text-style (,medium (make-text-style nil ,face nil)) ,@body))

(defmacro with-text-size ((medium size) &body body)
  `(with-text-style (,medium (make-text-style nil nil ,size)) ,@body))

(defoperation invoke-with-text-style medium-protocol
  ((medium medium) continuation style original-stream))

(defmethod invoke-with-text-style ((medium basic-medium)
                                   continuation style original-stream)
  (if (or (null style) (eq style *null-text-style*))
      (funcall continuation original-stream)
      (letf-globally (((medium-merged-text-style-valid medium) nil)
                      ((slot-value medium 'merged-text-style)
                       (slot-value medium 'merged-text-style))
                      ((medium-text-style medium)
                       (merge-text-styles style (medium-text-style medium))))
        (funcall continuation original-stream))))

(defmethod invoke-with-text-style ((stream standard-encapsulating-stream)
                                   continuation style original-stream)
  (invoke-with-text-style (encapsulating-stream-stream stream)
                          continuation style original-stream))

;; Default method for string streams
(defmethod invoke-with-text-style ((stream t) continuation style original-stream)
  (declare (ignore style))
  (funcall continuation original-stream))


(defmethod graft ((medium basic-medium))
  (graft (medium-sheet medium)))

(defoperation text-style-height medium-protocol
  (text-style (medium medium))
  (:no-defgeneric t))

(defoperation text-style-width medium-protocol
  (text-style (medium medium))
  (:no-defgeneric t))

(defoperation text-style-ascent medium-protocol
  (text-style (medium medium))
  (:no-defgeneric t))

(defoperation text-style-descent medium-protocol
  (text-style (medium medium))
  (:no-defgeneric t))

(defoperation text-style-fixed-width-p medium-protocol
  (text-style (medium medium))
  (:no-defgeneric t))

(defoperation text-size medium-protocol
  ((medium medium) string &key text-style start end)
  (declare (values largest-x total-height last-x last-y baseline))
  (:no-defgeneric t))


;; Generate the sheet->medium trampolines now
(generate-trampolines medium-protocol medium standard-sheet-output-mixin
                      `(sheet-medium ,standard-sheet-output-mixin))

