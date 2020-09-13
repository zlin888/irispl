(import utils "docs/test.utils.scm")
(import error)
(import object)

; (define _cons
;   (lambda (l1 l2)
;     (if (or (not (list? l1)) (not (list? l2)))
;       (exit error.cons-error)
;       (if (pair)) 
;         )))

(define display-list 
  (lambda (l) 
    (if (pair? l)
      (begin (display-list (cdr l)) (display (car l)) )
      (display (car l)))))

(define _list?
  (lambda (item)
    (if (list? item) #t #f)))

; (display (_cons (list 12) 3))
; (display (_cons 123 12))

; (begin (display 2) (display 1) )
; (display-list (list 3 4 5 6))
; (utils.show error.cons-error)

; ; (display (begin (display 12) 123))
; ; (display (begin (display 12) (display 18) 99 1299))
; ; (display (let ((_ (display 12)) (x 13)) (+ x 1)))
; (display (car (car (list 1))))

(class food (name)
  (super object.object)
  ((fetch (lambda (self) name)))) ;adsf

(class apple (value)
  (super (food "apple"))
  ( (fetch-value (lambda (self) value))
    (store (lambda (self new-value) (set! value new-value)))))

(define my-apple (apple 380))
(display (object.=> my-apple 'fetch))
(object.=> my-apple 'store 800)
((my-apple 'store) my-apple 400)
(display (object.=> my-apple 'fetch-value))
(display (object.=> my-apple 'fetch))

(display (begin (* x 2) (define x 12)))
(display (let ((x 12) (y 17)) (begin (+ x y) (* x y))))
(display (quasiquote (quote 12)))
(display (quote list 1 2 3))

(define funcc
  (lambda (z)
    (lambda (x y)
    (if (and (eq? x (car z)) (eq? y (car (cdr z))))
        (+ x y)
        'error))))

(define func
    (funcc (list 1 2)))

(display (cond ((> 2 9) #f) ((> 2 8) #t) (else 'jack)))
(display (type 12))
(display (type "jack"))

(define list-size-drive
  (lambda (l n)
    (if (pair? l)
        (list-size-drive (cdr l) (+ n 1))
        (+ n 1))))

(define list-size
  (lambda (l)
    (list-size-drive l 0)))

; (define type-lambda
;   (lambda (_type . args)
;     ()))

(define map
  (lambda (f l)
    (if (pair? l)
      (list (f (car l)) (map f (cdr l)))
      (f (car l)))))

(define double
  (lambda (x) (* x 2)))

(display (cons (list 1 2 3) (list 7 8 9)))
(display (map double (list 1 2 3)))

; (display (type-lambda (list 'NUMBER 'NUMBER 'NUMBER) 1 2))

; (define shit
;   (lambda (_type . args)
;     (display _type)))

; (shit (list 129 'NUMBER 'NUMBER))