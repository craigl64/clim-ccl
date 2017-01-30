;;; -*- Mode: Lisp; Syntax: ANSI-Common-Lisp; Package: CLIM-INTERNALS; Base: 10; Lowercase: Yes -*-
;; See the file LICENSE for the full license governing this code.
;;

(in-package :clim-internals)

#-ics
(eval-when (compile)
  (warn "~S contains fat strings but is being compiled with a non-ICS lisp"
	excl:*source-pathname*))

(excl:ics-target-case
(:+ics

#-ics
(cerror "Continue with incorrect fat strings"
	"~S contains fat strings but was compiled with a non-ICS lisp"
	excl:*source-pathname*)

(defclass japanese-input-editing-stream
    (standard-input-editing-stream)
  (;; romaji-kana state transition
   (kana-state :initform nil)
   ;; kana->kanji convertor
   (kanji-server :initform nil :initarg :kanji-server)))

(defun make-kana-state ()
  (make-array 10 :fill-pointer 0 :adjustable t))

(defvar *initial-kana-state*
    (make-kana-state))

(define-input-editor-gestures
    (:ie-kana-mode :\\ :control))

(assign-input-editor-key-bindings
   com-ie-kana-mode :ie-kana-mode)

(define-input-editor-command (com-ie-kana-mode) (stream input-buffer)
  (with-slots (command-mode command-state kana-state) stream
    (setf command-mode *kana-input-editor-command-aarray*
	  command-state *kana-input-editor-command-aarray*
	  kana-state *initial-kana-state*)
    (let* ((point (stream-insertion-pointer stream))
	   (end (fill-pointer input-buffer)))
      (cond ((= point end)
	     (incf (fill-pointer input-buffer) 2))
	    (t
	     (erase-input-buffer stream point)
	     (shift-buffer-portion input-buffer point (+ point 2))))
      (setf (subseq input-buffer point) "||"
	    (stream-insertion-pointer stream) (1+ point))
      (redraw-input-buffer stream point))))

(defmethod find-kana-start ((stream input-editing-stream-mixin))
  (with-slots (input-buffer) stream
    (let ((i (stream-insertion-pointer stream))
	  (input-buffer (input-editor-buffer stream)))
      (loop
	(when (zerop i)
	  (return nil))
	(decf i)
	(when (eql (aref input-buffer i) #\|)
	  (return i))))))

(defmethod find-kana-end ((stream input-editing-stream-mixin))
  (with-slots (input-buffer) stream
    (let* ((i (stream-insertion-pointer stream))
	   (input-buffer (input-editor-buffer stream))
	   (fill-pointer (fill-pointer input-buffer)))
      (loop
	(when (eq (aref input-buffer i) #\|)
	  (return i))
	(when (eql i fill-pointer)
	  (return nil))
	(incf i)))))

(defmethod kana-end-input ((stream input-editing-stream-mixin))
  (with-slots (command-mode command-state kana-state input-buffer)
      stream
    (setf command-mode *input-editor-command-aarray*
	  command-state *input-editor-command-aarray*)
    (let ((start (find-kana-start stream))
	  (end (find-kana-end stream)))
      (erase-input-buffer stream start)
      (shift-buffer-portion input-buffer (1+ end) end)
      (shift-buffer-portion input-buffer (1+ start) start)
      (setf (stream-insertion-pointer stream) (1- end))
      (redraw-input-buffer stream start)
      (immediate-rescan stream))))

(defmethod input-editor-kanji-server ((stream input-editing-stream-mixin))
  (with-slots (kanji-server) stream
    (or kanji-server
	(setq kanji-server (find-kanji-server)))))

(defmethod kana-begin-henkan ((stream input-editing-stream-mixin))
  (with-slots (command-mode command-state kana-state input-buffer)
      stream
    (let* ((start (find-kana-start stream))
	   (end (find-kana-end stream))
	   (yomi (coerce (subseq input-buffer (1+ start) end)
			 'string))
	   (kanji-server (input-editor-kanji-server stream))
	   (candidates (jie-begin-kanji-conversion kanji-server yomi))
	   (kanji (let ((bunsetu 0)
			(r ""))
		    (dolist (candidate candidates)
		      (declare (ignore candidate))
		      (setq r
			(concatenate 'string
			  r (jie-get-kanji kanji-server bunsetu 0))))
		    r)))
      (erase-input-buffer stream start)
      (shift-buffer-portion input-buffer
			    end
			    (+ end (- (length kanji) (length yomi))))
      (setf (subseq input-buffer (1+ start)) kanji)
      (setf (stream-insertion-pointer stream) (1+ start))
      (redraw-input-buffer stream start)
      (immediate-rescan stream))))

(defmethod kana-process-gesture ((stream input-editing-stream-mixin)
				 (gesture character) type)
  (with-slots (kana-state input-buffer) stream
    (when (eq gesture #\newline)
      (kana-end-input stream))
    (when (eq gesture #\space)
      (kana-begin-henkan stream))
    (let ((entry (cdr (find gesture kana-state :key #'car))))
      (if entry
	  (if (arrayp entry)
	      (progn
		(setf kana-state entry)
		(return-from kana-process-gesture
		  (values gesture type)))
	    (destructuring-bind (hiragana katakana depth new) entry
	      (declare (ignore katakana))
	      #+debug
	      (format excl:*initial-terminal-io* "~%~S" entry)
	      (let* ((point (- (stream-insertion-pointer stream) depth))
		     (end (fill-pointer input-buffer))
		     (new-string (concatenate 'string hiragana new))
		     (length (length new-string)))
		(cond ((= point end)
		       (incf (fill-pointer input-buffer) length))
		      (t
		       (erase-input-buffer stream point)
		       (shift-buffer-portion input-buffer
					     (+ point depth)
					     (+ point length))))
		(setf (subseq input-buffer point) new-string
		      (stream-insertion-pointer stream) (+ point length))
		(redraw-input-buffer stream point))
	      (setf kana-state *initial-kana-state*)
	      (dovector (c new)
  		 (setf kana-state (cdr (find c kana-state :key #'car))))
	      (return-from kana-process-gesture
		nil)))
	(progn
	  (setf kana-state *initial-kana-state*)
	  (return-from kana-process-gesture
	    nil))))))

;;;;; default romaji-kana conversion

(defun add-romaji-kana (romaji hiragana katakana &optional new-romaji)
  (let ((state *initial-kana-state*)
	(depth 0))
    (dorest (r (map 'list #'identity romaji))
      (destructuring-bind (c . rest) r
	(if rest
	    (let ((new-state (cdr (find c state :key #'car))))
	      (if new-state
		  (if (arrayp new-state)
		      (setf state new-state)
		    (error "Trying to add a non-terminal romaji-kana ~A"
			   romaji))
		(let ((new-state (make-kana-state)))
		  (vector-push-extend (cons c new-state) state)
		  (setf state new-state))))
	  (let ((entry (find c state :key #'car))
		(terminal (list hiragana katakana depth new-romaji)))
	    (if entry
		(if (arrayp (cdr entry))
		    (error "Trying to add a terminal romaji-kana ~A"
			   romaji)
		  (setf (cdr entry) terminal))
	      (vector-push-extend (cons c terminal) state)))))
      (incf depth))))

(dolist (a '("k" "s" "t" "h" "y" "r" "w" "g" "z" "d" "b"
		 "p" "c" "f" "j" "v"))
  (add-romaji-kana (format nil "~A~A" a a) "��" "��" a))

(add-romaji-kana "tch"  "��" "��" "ch")

(add-romaji-kana "n'" "��" "��")
(add-romaji-kana "N" "��" "��")

(dolist (a '("b" "m" "p"))
  (add-romaji-kana (format nil "m~A" a) "��" "��" a))

(dolist (a '("k" "s" "t" "c" "h" "f" "m" "r" "l"
	      "w" "g" "z" "j" "d" "b" "v" "p" "x" "n"))
  (add-romaji-kana (format nil "n~A" a) "��" "��" a))

(add-romaji-kana "a" "��" "��")
(add-romaji-kana "i" "��" "��")
(add-romaji-kana "u" "��" "��")
(add-romaji-kana "e" "��" "��")
(add-romaji-kana "o" "��" "��")
(add-romaji-kana "ka" "��" "��")
(add-romaji-kana "ki" "��" "��")
(add-romaji-kana "ku" "��" "��")
(add-romaji-kana "ke" "��" "��")
(add-romaji-kana "ko" "��" "��")
(add-romaji-kana "kya" "����" "����")
(add-romaji-kana "kyu" "����" "����")
(add-romaji-kana "kye" "����" "����")
(add-romaji-kana "kyo" "����" "����")
(add-romaji-kana "sa" "��" "��")
(add-romaji-kana "si" "��" "��")
(add-romaji-kana "su" "��" "��")
(add-romaji-kana "se" "��" "��")
(add-romaji-kana "so" "��" "��")
(add-romaji-kana "sya" "����" "����")
(add-romaji-kana "syu" "����" "����")
(add-romaji-kana "sye" "����" "����")
(add-romaji-kana "syo" "����" "����")
(add-romaji-kana "sha" "����" "����")
(add-romaji-kana "shi" "��" "��")
(add-romaji-kana "shu" "����" "����")
(add-romaji-kana "she" "����" "����")
(add-romaji-kana "sho" "����" "����")
(add-romaji-kana "ta" "��" "��")
(add-romaji-kana "ti" "��" "��")
(add-romaji-kana "tu" "��" "��")
(add-romaji-kana "te" "��" "��")
(add-romaji-kana "to" "��" "��")
(add-romaji-kana "tya" "����" "����")
(add-romaji-kana "tyi" "�Ƥ�" "�ƥ�")
(add-romaji-kana "tyu" "����" "����")
(add-romaji-kana "tye" "����" "����")
(add-romaji-kana "tyo" "����" "����")
(add-romaji-kana "tsu" "��" "��")
(add-romaji-kana "cha" "����" "����")
(add-romaji-kana "chi" "��" "��")
(add-romaji-kana "chu" "����" "����")
(add-romaji-kana "che" "����" "����")
(add-romaji-kana "cho" "����" "����")
(add-romaji-kana "na" "��" "��")
(add-romaji-kana "ni" "��" "��")
(add-romaji-kana "nu" "��" "��")
(add-romaji-kana "ne" "��" "��")
(add-romaji-kana "no" "��" "��")
(add-romaji-kana "nya" "�ˤ�" "�˥�")
(add-romaji-kana "nyu" "�ˤ�" "�˥�")
(add-romaji-kana "nye" "�ˤ�" "�˥�")
(add-romaji-kana "nyo" "�ˤ�" "�˥�")
(add-romaji-kana "ha" "��" "��")
(add-romaji-kana "hi" "��" "��")
(add-romaji-kana "hu" "��" "��")
(add-romaji-kana "he" "��" "��")
(add-romaji-kana "ho" "��" "��")
(add-romaji-kana "hya" "�Ҥ�" "�ҥ�")
(add-romaji-kana "hyu" "�Ҥ�" "�ҥ�")
(add-romaji-kana "hye" "�Ҥ�" "�ҥ�")
(add-romaji-kana "hyo" "�Ҥ�" "�ҥ�")
(add-romaji-kana "fa" "�դ�" "�ե�")
(add-romaji-kana "fi" "�դ�" "�ե�")
(add-romaji-kana "fu" "��" "��")
(add-romaji-kana "fe" "�դ�" "�ե�")
(add-romaji-kana "fo" "�դ�" "�ե�")
(add-romaji-kana "ma" "��" "��")
(add-romaji-kana "mi" "��" "��")
(add-romaji-kana "mu" "��" "��")
(add-romaji-kana "me" "��" "��")
(add-romaji-kana "mo" "��" "��")
(add-romaji-kana "mya" "�ߤ�" "�ߥ�")
(add-romaji-kana "myu" "�ߤ�" "�ߥ�")
(add-romaji-kana "mye" "�ߤ�" "�ߥ�")
(add-romaji-kana "myo" "�ߤ�" "�ߥ�")
(add-romaji-kana "ya" "��" "��")
(add-romaji-kana "yi" "��" "��")
(add-romaji-kana "yu" "��" "��")
(add-romaji-kana "ye" "����" "����")
(add-romaji-kana "yo" "��" "��")
(add-romaji-kana "ra" "��" "��")
(add-romaji-kana "ri" "��" "��")
(add-romaji-kana "ru" "��" "��")
(add-romaji-kana "re" "��" "��")
(add-romaji-kana "ro" "��" "��")
(add-romaji-kana "la" "��" "��")
(add-romaji-kana "li" "��" "��")
(add-romaji-kana "lu" "��" "��")
(add-romaji-kana "le" "��" "��")
(add-romaji-kana "lo" "��" "��")
(add-romaji-kana "rya" "���" "���")
(add-romaji-kana "ryu" "���" "���")
(add-romaji-kana "rye" "�ꤧ" "�ꥧ")
(add-romaji-kana "ryo" "���" "���")
(add-romaji-kana "lya" "���" "���")
(add-romaji-kana "lyu" "���" "���")
(add-romaji-kana "lye" "�ꤧ" "�ꥧ")
(add-romaji-kana "lyo" "���" "���")
(add-romaji-kana "wa" "��" "��")
(add-romaji-kana "wi" "��" "��")
(add-romaji-kana "wu" "��" "��")
(add-romaji-kana "we" "��" "��")
(add-romaji-kana "wo" "��" "��")
(add-romaji-kana "ga" "��" "��")
(add-romaji-kana "gi" "��" "��")
(add-romaji-kana "gu" "��" "��")
(add-romaji-kana "ge" "��" "��")
(add-romaji-kana "go" "��" "��")
(add-romaji-kana "gya" "����" "����")
(add-romaji-kana "gyu" "����" "����")
(add-romaji-kana "gye" "����" "����")
(add-romaji-kana "gyo" "����" "����")
(add-romaji-kana "za" "��" "��")
(add-romaji-kana "zi" "��" "��")
(add-romaji-kana "zu" "��" "��")
(add-romaji-kana "ze" "��" "��")
(add-romaji-kana "zo" "��" "��")
(add-romaji-kana "zya" "����" "����")
(add-romaji-kana "zyu" "����" "����")
(add-romaji-kana "zye" "����" "����")
(add-romaji-kana "zyo" "����" "����")
(add-romaji-kana "ja" "����" "����")
(add-romaji-kana "ji" "��" "��")
(add-romaji-kana "ju" "����" "����")
(add-romaji-kana "je" "����" "����")
(add-romaji-kana "jo" "����" "����")
(add-romaji-kana "jya" "����" "����")
(add-romaji-kana "jyu" "����" "����")
(add-romaji-kana "jye" "����" "����")
(add-romaji-kana "jyo" "����" "����")
(add-romaji-kana "da" "��" "��")
(add-romaji-kana "di" "��" "��")
(add-romaji-kana "du" "��" "��")
(add-romaji-kana "de" "��" "��")
(add-romaji-kana "do" "��" "��")
(add-romaji-kana "dya" "�¤�" "�¥�")
(add-romaji-kana "dyi" "�Ǥ�" "�ǥ�")
(add-romaji-kana "dyu" "�¤�" "�¥�")
(add-romaji-kana "dye" "�¤�" "�¥�")
(add-romaji-kana "dyo" "�¤�" "�¥�")
(add-romaji-kana "ba" "��" "��")
(add-romaji-kana "bi" "��" "��")
(add-romaji-kana "bu" "��" "��")
(add-romaji-kana "be" "��" "��")
(add-romaji-kana "bo" "��" "��")
(add-romaji-kana "va" "����" "����")
(add-romaji-kana "vi" "����" "����")
(add-romaji-kana "vu" "��" "��")
(add-romaji-kana "ve" "����" "����")
(add-romaji-kana "vo" "����" "����")
(add-romaji-kana "bya" "�Ӥ�" "�ӥ�")
(add-romaji-kana "byu" "�Ӥ�" "�ӥ�")
(add-romaji-kana "bye" "�Ӥ�" "�ӥ�")
(add-romaji-kana "byo" "�Ӥ�" "�ӥ�")
(add-romaji-kana "pa" "��" "��")
(add-romaji-kana "pi" "��" "��")
(add-romaji-kana "pu" "��" "��")
(add-romaji-kana "pe" "��" "��")
(add-romaji-kana "po" "��" "��")
(add-romaji-kana "pya" "�Ԥ�" "�ԥ�")
(add-romaji-kana "pyu" "�Ԥ�" "�ԥ�")
(add-romaji-kana "pye" "�Ԥ�" "�ԥ�")
(add-romaji-kana "pyo" "�Ԥ�" "�ԥ�")
(add-romaji-kana "kwa" "����" "����")
(add-romaji-kana "kwi" "����" "����")
(add-romaji-kana "kwu" "��" "��")
(add-romaji-kana "kwe" "����" "����")
(add-romaji-kana "kwo" "����" "����")
(add-romaji-kana "gwa" "����" "����")
(add-romaji-kana "gwi" "����" "����")
(add-romaji-kana "gwu" "��" "��")
(add-romaji-kana "gwe" "����" "����")
(add-romaji-kana "gwo" "����" "����")
(add-romaji-kana "tsa" "�Ĥ�" "�ĥ�")
(add-romaji-kana "tsi" "�Ĥ�" "�ĥ�")
(add-romaji-kana "tse" "�Ĥ�" "�ĥ�")
(add-romaji-kana "tso" "�Ĥ�" "�ĥ�")

(add-romaji-kana "xa" "��" "��")
(add-romaji-kana "xi" "��" "��")
(add-romaji-kana "xu" "��" "��")
(add-romaji-kana "xe" "��" "��")
(add-romaji-kana "xo" "��" "��")
(add-romaji-kana "xya" "��" "��")
(add-romaji-kana "xyu" "��" "��")
(add-romaji-kana "xyo" "��" "��")
(add-romaji-kana "xtu" "��" "��")
(add-romaji-kana "xtsu" "��" "��")
(add-romaji-kana "xwa" "��" "��")

(add-romaji-kana "xka" "��" "��")
(add-romaji-kana "xke" "��" "��")
(add-romaji-kana "xti" "�Ƥ�" "�ƥ�")
(add-romaji-kana "xdi" "�Ǥ�" "�ǥ�")
(add-romaji-kana "xdu" "�ɤ�" "�ɥ�")
(add-romaji-kana "xde" "�Ǥ�" "�ǥ�")
(add-romaji-kana "xdo" "�ɤ�" "�ɥ�")
(add-romaji-kana "xwi" "����" "����")
(add-romaji-kana "xwe" "����" "����")
(add-romaji-kana "xwo" "����" "����")

)) ;; ics-target-case
