;;; -*- Mode: Lisp; Syntax: ANSI-Common-Lisp; Package: CLIM-INTERNALS; Base: 10 -*-
#-Genera
(in-package :CLIM-INTERNALS)

(defgeneric window-shift-visible-region (window
                                         old-left old-top old-right old-bottom
                                         new-left new-top new-right new-bottom))

(defgeneric viewport-region-changed (stream viewport))

(defgeneric native-gadget-range* (gadget))

(defgeneric frame-shell (frame))
(defgeneric (setf frame-shell) (value frame))

(defgeneric frame-panes (frame))
(defgeneric (setf frame-panes) (value frame))

(defgeneric frame-state (frame))
(defgeneric (setf frame-state) (value frame))

(defgeneric disable-frame (frame))
(defgeneric enable-frame (frame))
(defgeneric frame-foreground (frame))
(defgeneric frame-background (frame))
(defgeneric frame-text-style (frame))

(defgeneric frame-top-level-sheet (frame))
(defgeneric (setf frame-top-level-sheet) (sheet frame))

(defgeneric port-note-frame-adopted (port frame))

(defgeneric frame-manager (activity))
(defgeneric (setf frame-manager) (value activity))

(defgeneric generate-panes (framem frame))

(defgeneric frame-input-buffer (frame))
(defgeneric frame-user-specified-position-p (frame))
(defgeneric frame-user-specified-size-p (frame))

(defgeneric port-set-pane-text-style (port pane medium style))
(defgeneric port-set-pane-background (port pane medium ink))
(defgeneric port-set-pane-foreground (port pane medium ink))
(defgeneric clim-internals::port-query-pointer (port sheet))

(defgeneric pointer-cursor (pointer))
(defgeneric (setf pointer-cursor) (cursor pointer))
(defgeneric pointer-native-x-position (poiner))
(defgeneric (setf pointer-native-x-position) (value pointer))
(defgeneric pointer-native-y-position (pointer))
(defgeneric (setf pointer-native-y-position) (value pointer))
(defgeneric pointer-x-position (pointer))
(defgeneric (setf pointer-x-position) (value pointer))
(defgeneric pointer-y-position (pointer))
(defgeneric (setf pointer-y-position) (value pointer))

(defgeneric pointer-button-state (event))
(defgeneric (setf pointer-button-state) (value event))

(defgeneric pointer-sheet (pointer))
(defgeneric (setf pointer-sheet) (sheet pointer))

(defgeneric pointer-valid-p (pointer))
(defgeneric (setf pointer-valid-p) (value pointer))

(defgeneric frame-manager-display-pointer-documentation-string (framem frame stream string))

(defgeneric scroll-up-line-callback   (gadget client id &optional value))
(defgeneric scroll-down-line-callback (gadget client id &optional value))
(defgeneric scroll-up-page-callback   (gadget client id &optional value))
(defgeneric scroll-down-page-callback (gadget client id &optional value))
(defgeneric scroll-to-top-callback    (gadget client id &optional value))
(defgeneric scroll-to-bottom-callback (gadget client id &optional value))

(defgeneric output-recording-stream-p (stream))
(defgeneric clim-internals::stream-event-handler      (stream &key &allow-other-keys))

(defgeneric window-set-viewport-position (stream x y))
(defgeneric window-viewport-position (stream))

(defgeneric presentation-replace-input (stream object type view &key &allow-other-keys)
  ;; buffer-start rescan query-identifier for-context-type)
  )

(defgeneric wait-for-window-exposed (window))
