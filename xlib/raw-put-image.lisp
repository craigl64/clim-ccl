;;; -*- Mode:Lisp; Package:XLIB; Syntax:COMMON-LISP; Base:10; Lowercase:T -*-

(in-package :XLIB)


(defun put-image (drawable gcontext image &key
                  (src-x 0) (src-y 0)
                  (x (required-arg x))
                  (y (required-arg y))
                  width height
                  bitmap-p)
  (declare (type drawable drawable)
           (type gcontext gcontext)
           (type image image)
           (type int16 x y)
           (type int16 src-x src-y)
           (type (or null card16) width height)
           (type generalized-boolean bitmap-p))
  (block put-image
    (let* ((format
            (let ((#:g743 image))
              (declare (ignorable #:g743))
              (if (typep #:g743 'image-x)
                  (progn
                    nil
                    (image-x-format (the image-x image)))
                (if (typep #:g743 'image-xy)
                    (progn nil :xy-pixmap)
                  (if (typep #:g743 'image-z)
                      (progn nil :z-pixmap)
                    (the t
                      (progn
                        (sb-kernel:case-failure
                         'etypecase #:g743
                         '(image-x image-xy
                           image-z)))))))))
           (src-x
            (if (image-x-p image)
                (the array-index
                  (values
                   (+ (the array-index src-x)
                      (the array-index
                        (image-x-left-pad
                         (the image-x image))))))
              src-x))
           (image-width (image-width image))
           (image-height (image-height image))
           (width
            (min
             (let ((#:g744 width))
               (if #:g744 #:g744 (the t image-width)))
             (the array-index
               (values
                (- (the array-index image-width)
                   (the array-index src-x))))))
           (height
            (min
             (let ((#:g745 height))
               (if #:g745
                   #:g745
                 (the t image-height)))
             (the array-index
               (values
                (- (the array-index image-height)
                   (the array-index src-y))))))
           (depth (image-depth image))
           (display (drawable-display drawable))
           (bitmap-format (display-bitmap-format display))
           (unit (bitmap-format-unit bitmap-format))
           (byte-lsb-first-p
            (display-image-lsb-first-p display))
           (bit-lsb-first-p
            (bitmap-format-lsb-first-p bitmap-format)))
      (declare
       (type (member :bitmap :xy-pixmap :z-pixmap) format)
       (type fixnum src-x image-width image-height width
             height)
       (type image-depth depth)
       (type display display)
       (type bitmap-format bitmap-format)
       (type (member 8 16 32) unit)
       (type generalized-boolean byte-lsb-first-p
             bit-lsb-first-p))
      (if (if bitmap-p
              (the t (not (= (the array-index depth) 1)))
            nil)
          (progn (error "Bitmaps must have depth 1"))
        nil)
      (if (<= 0 src-x
              (the array-index
                (values
                 (1-
                  (the array-index
                    (image-width image))))))
          nil
        (progn (error "src-x not inside image")))
      (if (<= 0 src-y
              (the array-index
                (values
                 (1-
                  (the array-index
                    (image-height image))))))
          nil
        (progn (error "src-y not inside image")))
      (if (if (> (the array-index width) 0)
              (the t (> (the array-index height) 0))
            nil)
          (progn
            (multiple-value-bind (pad bits-per-pixel)
                (let ((#:g746 format))
                  (declare (ignorable #:g746))
                  (if (let ((#:g747 (eql #:g746 ':bitmap)))
                        (if #:g747
                            #:g747
                          (the t (eql #:g746 ':xy-pixmap))))
                      (progn
                        nil
                        (values
                         (bitmap-format-pad bitmap-format) 1))
                    (if (eql #:g746 ':z-pixmap)
                        (progn
                          nil
                          (if (= depth 1)
                              (values (bitmap-format-pad bitmap-format)
                                      1)
                            (let ((pixmap-format
                                   (find depth (display-pixmap-formats display)
                                         :key #'pixmap-format-depth)))
                              (declare (type (or null pixmap-format) pixmap-format))
                              (if (null pixmap-format)
                                  (error
                                   "The depth of the image ~s does not match any server pixmap format."
                                   image))
                              (if (not
                                   (=
                                    (let ((#:g748 image))
                                      (declare (ignorable #:g748))
                                      (if (typep #:g748 'image-z)
                                          (progn (image-z-bits-per-pixel image))
                                        (if (typep #:g748 'image-x)
                                            (progn (image-x-bits-per-pixel image))
                                          (the t
                                            (progn
                                              (sb-kernel:case-failure 'etypecase
                                                                      #:g748 '(image-z image-x)))))))
                                    (pixmap-format-bits-per-pixel pixmap-format)))
                                  (error "The bits-per-pixel of the image ~s does not match any server pixmap format."
                                         image))
                              (values (pixmap-format-scanline-pad pixmap-format)
                                      (pixmap-format-bits-per-pixel pixmap-format)))))
                      (the t
                        (progn
                          (sb-kernel:case-failure 'ecase #:g746
                                                  '(:bitmap :xy-pixmap :z-pixmap)))))))
              (declare (type (member 8 16 32) pad)
                       (type (member 1 4 8 16 24 32) bits-per-pixel))
              (let* ((left-pad
                      (if (let ((#:g749 (eq format :xy-pixmap)))
                            (if #:g749 #:g749 (the t (= depth 1))))
                          (the array-index
                            (values
                             (mod (the array-index src-x)
                                  (the array-index
                                    (the array-index
                                      (values (min (the array-index pad)
                                                   +image-pad+)))))))
                        0))
                     (left-padded-src-x
                      (the array-index
                        (values (- (the array-index src-x)
                                   (the array-index left-pad)))))
                     (left-padded-width
                      (the array-index
                        (values (+ (the array-index width)
                                   (the array-index left-pad)))))
                     (bits-per-line
                      (the array-index
                        (values (* (the array-index left-padded-width)
                                   (the array-index bits-per-pixel)))))
                     (padded-bits-per-line
                      (the array-index
                        (values
                         (*
                          (the array-index
                            (the array-index
                              (values
                               (ceiling (the array-index bits-per-line)
                                        (the array-index pad)))))
                          (the array-index pad)))))
                     (padded-bytes-per-line
                      (the array-index
                        (values
                         (ceiling (the array-index padded-bits-per-line)
                                  8))))
                     (request-bytes-per-line
                      (let ((#:g750 format))
                        (declare (ignorable #:g750))
                        (if (let ((#:g751 (eql #:g750 ':bitmap)))
                              (if #:g751 #:g751 (the t (eql #:g750 ':xy-pixmap))))
                            (progn
                              nil
                              (the array-index
                                (values (* (the array-index padded-bytes-per-line)
                                           (the array-index depth)))))
                          (if (eql #:g750 ':z-pixmap)
                              (progn padded-bytes-per-line)
                            (the t
                              (progn
                                (sb-kernel:case-failure 'ecase #:g750
                                                        '(:bitmap :xy-pixmap :z-pixmap))))))))
                     (max-bytes-per-request
                      (the array-index
                        (values
                         (*
                          (the array-index
                            (the array-index
                              (values
                               (- (the array-index (display-max-request-length display))
                                  6))))
                          4))))
                     (max-request-height
                      (floor max-bytes-per-request request-bytes-per-line)))
                (declare (type card8 left-pad)
                         (type int16 left-padded-src-x)
                         (type card16 left-padded-width)
                         (type array-index bits-per-line
                               padded-bits-per-line
                               padded-bytes-per-line
                               request-bytes-per-line
                               max-bytes-per-request
                               max-request-height))
                (if (zerop (the array-index max-request-height))
                    (progn (error "can't even fit one image scanline in a request"))
                  nil)
                (buffer-ensure-size display padded-bytes-per-line)
                (block nil
                  (let* ((request-src-y src-y)
                         (request-y y)
                         (height-remaining height)
                         (request-height
                          (the array-index
                            (values (min (the array-index height-remaining)
                                         (the array-index max-request-height))))))
                    (declare
                     (type array-index request-src-y request-height)
                     (fixnum height-remaining))
                    (tagbody
                     (go :g753)
                     :g752
                     (tagbody
                      (let* ((request-bytes
                              (the array-index
                                (values (* (the array-index request-bytes-per-line)
                                           (the array-index request-height)))))
                             (request-words
                              (the array-index
                                (values (ceiling (the array-index request-bytes)
                                                 4))))
                             (request-length
                              (the array-index
                                (values (+ (the array-index request-words)
                                           6)))))
                        (declare (type array-index request-bytes)
                                 (type card16 request-words request-length))
                        (let ((.display. display))
                          (declare (type display .display.))
                          (macrolet ((with-buffer ((buffer &key timeout) &body body)
                                       `(progn
                                         (progn
                                           ,buffer
                                           ,@(if timeout (the t `(,timeout))
                                               nil)
                                           nil)
                                         ,@body)))
                            (let ()
                              (declare (optimize (speed 3) (safety 0) (debug 1)))
                              (if (buffer-dead .display.)
                                  (progn
                                    (x-error 'closed-display :display .display.))
                                nil)
                              (flet ((sb-thread::with-recursive-lock-thunk ()
                                       (force-gcontext-changes-internal gcontext)
                                       (multiple-value-prog1
                                        (progn
                                          (let ((%buffer .display.))
                                            (declare (type display %buffer))
                                            (declare (optimize (speed 3) (safety 0) (debug 1)))
                                            (if (>=
                                                 (the array-index
                                                   (the array-index
                                                     (values (+ (the array-index (buffer-boffset %buffer))
                                                                +requestsize+))))
                                                 (the array-index (buffer-size %buffer)))
                                                (progn (buffer-flush %buffer))
                                              nil)
                                            (let* ((buffer-boffset
                                                    (the array-index (buffer-boffset %buffer)))
                                                   (buffer-bbuf (buffer-obuf8 %buffer)))
                                              (declare (type array-index buffer-boffset))
                                              (declare (type buffer-bytes buffer-bbuf))
                                              buffer-boffset
                                              buffer-bbuf
                                              (let* ((#:.display.764 .display.))
                                                (multiple-value-bind (#:new763)
                                                    buffer-boffset
                                                  (funcall #'(setf buffer-last-request)
                                                           #:new763 #:.display.764)))
                                              (aset-card8 (the card8 +x-putimage+)
                                                          buffer-bbuf (the array-index
                                                                        (values (+ (the array-index buffer-boffset)
                                                                                   0))))
                                              (let ((.value.
                                                     (if (let ((#:g765 (eq format :bitmap)))
                                                           (if #:g765 #:g765
                                                             (the t bitmap-p)))
                                                         (progn :bitmap)
                                                       (if (plusp left-pad)
                                                           (progn :xy-pixmap)
                                                         (the t (progn format))))))
                                                (macrolet ((write-card32 (index value)
                                                             index value))
                                                  (aset-card8
                                                   (the card8
                                                     (aset-card29
                                                      (the card29
                                                        (position .value.
                                                                  (the simple-vector
                                                                    '#(:bitmap :xy-pixmap :z-pixmap))
                                                                  :test #'eq))
                                                      buffer-bbuf
                                                      (the array-index
                                                        (values
                                                         (+ (the array-index buffer-boffset)
                                                            4)))))
                                                   buffer-bbuf
                                                   (the array-index
                                                     (values
                                                      (+ (the array-index buffer-boffset)
                                                         1))))))
                                              (aset-card29
                                               (the card29
                                                 (drawable-id drawable))
                                               buffer-bbuf
                                               (the array-index
                                                 (values
                                                  (+ (the array-index buffer-boffset)
                                                     4))))
                                              (aset-card29
                                               (the card29 (gcontext-id gcontext))
                                               buffer-bbuf
                                               (the array-index
                                                 (values
                                                  (+ (the array-index buffer-boffset)
                                                     8))))
                                              (aset-card16
                                               (the card16 width)
                                               buffer-bbuf
                                               (the array-index
                                                 (values
                                                  (+ (the array-index buffer-boffset)
                                                     12))))
                                              (aset-card16
                                               (the card16 request-height)
                                               buffer-bbuf
                                               (the array-index
                                                 (values
                                                  (+ (the array-index buffer-boffset)
                                                     14))))
                                              (aset-int16 (the int16 x) buffer-bbuf
                                                          (the array-index
                                                            (values
                                                             (+ (the array-index buffer-boffset)
                                                                16))))
                                              (aset-int16 (the int16 request-y) buffer-bbuf
                                                          (the array-index
                                                            (values
                                                             (+ (the array-index buffer-boffset)
                                                                18))))
                                              (aset-card8 (the card8 left-pad) buffer-bbuf
                                                          (the array-index
                                                            (values
                                                             (+ (the array-index buffer-boffset)
                                                                20))))
                                              (aset-card8 (the card8 depth) buffer-bbuf
                                                          (the array-index
                                                            (values
                                                             (+ (the array-index buffer-boffset)
                                                                21))))
                                              (let ((.value.
                                                     (aset-card16
                                                      (the card16 request-length)
                                                      buffer-bbuf
                                                      (the array-index
                                                        (values
                                                         (+ (the array-index buffer-boffset)
                                                            2))))))
                                                .value.)
                                              (let ((.value.
                                                     (let* ((#:display769 display))
                                                       (multiple-value-bind (#:new768)
                                                           (let ((.boffset.
                                                                  (the array-index
                                                                    (values
                                                                     (+ (the array-index
                                                                          buffer-boffset)
                                                                        24)))))
                                                             (declare
                                                              (type array-index .boffset.))
                                                             (setq buffer-boffset .boffset.))
                                                         (funcall #'(setf buffer-boffset)
                                                                  #:new768 #:display769)))))
                                                .value.)
                                              (let ((.value.
                                                     (let ((#:g770 image))
                                                       (declare (ignorable #:g770))
                                                       (if (typep #:g770 'image-x)
                                                           (progn
                                                             (let ((#:g771
                                                                    (image-x-format
                                                                     (the image-x image))))
                                                               (declare (ignorable #:g771))
                                                               (if (let ((#:g772
                                                                          (eql #:g771 ':bitmap)))
                                                                     (if #:g772 #:g772
                                                                       (the t (eql #:g771 ':xy-pixmap))))
                                                                   (progn
                                                                     (write-xy-format-image-x
                                                                      display
                                                                      image
                                                                      left-padded-src-x
                                                                      request-src-y
                                                                      left-padded-width
                                                                      request-height
                                                                      padded-bytes-per-line
                                                                      unit
                                                                      byte-lsb-first-p
                                                                      bit-lsb-first-p))
                                                                 (if (eql #:g771 ':z-pixmap)
                                                                     (progn
                                                                       (write-z-format-image-x
                                                                        display
                                                                        image
                                                                        left-padded-src-x
                                                                        request-src-y
                                                                        left-padded-width
                                                                        request-height
                                                                        padded-bytes-per-line
                                                                        unit
                                                                        byte-lsb-first-p
                                                                        bit-lsb-first-p))
                                                                   (the t
                                                                     (progn
                                                                       (sb-kernel:case-failure 'ecase #:g771
                                                                                               '(:bitmap :xy-pixmap :z-pixmap))))))))
                                                         (if (typep #:g770 'image-xy)
                                                             (progn
                                                               (write-image-xy
                                                                display
                                                                image
                                                                left-padded-src-x
                                                                request-src-y
                                                                left-padded-width
                                                                request-height
                                                                padded-bytes-per-line
                                                                unit
                                                                byte-lsb-first-p
                                                                bit-lsb-first-p))
                                                           (if (typep #:g770 'image-z)
                                                               (progn
                                                                 (write-image-z
                                                                  display
                                                                  image
                                                                  left-padded-src-x
                                                                  request-src-y
                                                                  left-padded-width
                                                                  request-height
                                                                  padded-bytes-per-line
                                                                  unit
                                                                  byte-lsb-first-p
                                                                  bit-lsb-first-p))
                                                             (the t
                                                               (progn
                                                                 (sb-kernel:case-failure 'etypecase #:g770
                                                                                         '(image-x image-xy image-z))))))))))
                                                .value.)
                                              (let ((.value.
                                                     (buffer-pad-request display
                                                                         (the array-index
                                                                           (values
                                                                            (- (the array-index
                                                                                 (the array-index
                                                                                   (values (* (the array-index
                                                                                                request-words)
                                                                                              4))))
                                                                               (the array-index request-bytes)))))))
                                                .value.)
                                              (buffer-new-request-number .display.))))
                                        (display-invoke-after-function .display.))))
                                (declare
                                 (sb-int:truly-dynamic-extent (function sb-thread::with-recursive-lock-thunk)))
                                (sb-thread::call-with-recursive-lock
                                 #'sb-thread::with-recursive-lock-thunk
                                 (buffer-lock .display.) t
                                 nil)))))))
                     (progn
                       (pi-format "~&;;   request-src-y=~D  request-y=~D  height-remaining=~D  request-height=~D~%"
                                  request-src-y request-y height-remaining request-height)
                       (setq request-src-y
                             (the array-index
                               (values (+ (the array-index request-src-y)
                                          (the array-index request-height)))))
                       (setq request-y
                             (the array-index
                               (values (+ (the array-index request-y)
                                          (the array-index request-height)))))
                       (setq height-remaining
                             (the fixnum (- height-remaining request-height)))
                       (setq request-height
                             (the array-index
                               (values (min (the array-index height-remaining)
                                            (the array-index max-request-height))))))
                     #:g753
                     (if (<= height-remaining 0) nil
                       (progn (go #:g752)))
                     (return-from nil (progn))))))))
        nil)))
  )
