;;; -*- Mode: Lisp; Syntax: ANSI-Common-Lisp; Package: CL-USER; Base: 10 -*-

(in-package :CL-USER)

(clsm:define-system :clim-standalone
      (:pretty-name "CLIM Standalone"
       :default-pathname  "clim2:clim;"
       :journal-directory "clim2:patches;"
       :patchable t)
  (:module silica (:clim-silica) (:type :system))

  (:serial silica
   "defresource"
   "temp-strings"
   "clim-defs" ;; mixed with CLIM Utils
   "ptypes1"   ;; mixed with CLIM Utils

   "recording-defs"  "gestures"  "defprotocol"  "stream-defprotocols"
   #|"defresource"|#  "stream-class-defs"  "interactive-defs"
   "cursor"  "view-defs"  "input-defs"  "input-protocol" "output-protocol"
   "formatted-output-defs"  "translators"  "ptypes2"  "command"
   "command-processor"  "recording-protocol"  "presentations"  "frames"
   "text-recording"  "graphics-recording"  "design-recording"
   "interactive-protocol"  "input-editor-commands"
;   "formatted-output-defs"
   "db-menu"

   "incremental-redisplay"  "r-tree"  "window-stream"  "pixmap-streams"

   "completer"
;   "translators"
   "histories"
;   "ptypes2"
   #+Allegro "excl-presentations"
   "standard-types"  "table-formatting"

   "graph-formatting"  "surround-output"  "text-formatting"  "tracking-pointer"
   "dragging-output"  "db-stream"
;   "command"
;   "command-processor"
   "gadget-output"  "accept"  "present"  "basic-translators"  "panes"
   "default-frame"  "activities"
;   "db-menu"
   "noting-progress"  "menus"  "accept-values"

   "drag-and-drop"  "item-list-manager"  "stream-trampolines"
   ))
