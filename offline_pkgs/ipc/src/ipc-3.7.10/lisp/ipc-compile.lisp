;;;; -*- Mode: LISP; Syntax: ANSI-COMMON-LISP; Package: CL-USER -*-
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;
;;; PROJECT: IPC (Interprocess Communication) Package
;;;          New Millennium, DS1
;;; 
;;; MODULE: IPC - lisp
;;;
;;; FILE: ipc-compile.lisp
;;;
;;; ABSTRACT:
;;; 
;;; IPC - Lisp Compile File
;;;
;;; $Revision: 2.1.1.1 $
;;; $Date: 1999/11/23 19:07:37 $
;;; $Author: reids $
;;;
;;; REVISION HISTORY
;;;
;;;  $Log: ipc-compile.lisp,v $
;;;  Revision 2.1.1.1  1999/11/23 19:07:37  reids
;;;  Putting IPC Version 2.9.0 under local (CMU) CVS control.
;;;
;;;

(load (make-pathname :DIRECTORY (pathname-directory *load-truename*)
		     :NAME "ipc-system.lisp"))

(operate-on-file-set2 *IPC-FILES* :compile)

