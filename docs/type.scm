; (import functools) 
; (display functools.x)
(import functools)

(display 1)
(display functools.map)

(define sum 
    (lambda (x y) (+ x y)))

; (define check-types
;   (lambda (_type-list object-list)
;     (eq? _type-list (functools.map type object-list))))

; (check-types (list 'NUMBER 'NUMBER) (list 12 13))