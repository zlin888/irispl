(import utils "/Users/bytedance/CLionProjects/typed-scheme/docs/test.utils.scm")

(define x
    (lambda (y)
        (lambda (x)
            (+ x y))))

(display ((x 12) (+ 999 12)))

(display (begin (+ 12 13) (+ 13 18)))

(define l
    (cons 12 13 (lambda (x y) (+ x y))))

(display ((cdr (cdr l)) (car l) (car (cdr l))))

(define j
    (cond ((list? 12) 12)
           ((list? 999) 888)
           (else 88)))

(utils.show j)

(display (let 12))

