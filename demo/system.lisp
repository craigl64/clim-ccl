;;; -*- Mode: Lisp; Syntax: ANSI-Common-Lisp; Package: CL-USER; Base: 10 -*-

(in-package :CL-USER)


(clsm:define-system :franz-demos
        (:pretty-name "Franz CLIM Demos"
         :default-pathname  "franz:demo;"
         :journal-directory "franz:patches;"
         :patchable t)
  (:module clim (:clx-franz) (:type :system))
  (:module pkg ("packages") (:type :lisp-read-only))

  (:serial clim pkg
    "demo-driver"
    "address-book"
    "bitmap-editor"
    "browser"
    "cad-demo"
    "cload-demos"
    "color-editor"
    "custom-records"
    "default-frame-top-level"
    "demo-activity"
    "demo-last"
    "graphics-demos"
    "graphics-editor"
    "ico"
    #+Allegro "japanese-graphics-editor"
    "listener"
    "navdata"
    "navfun"
    "palette"
    "peek-frame"
    "plot"
    #+Allegro "process-browser"
    "puzzle"
    "thinkadot"
    ))
