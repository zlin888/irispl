(import Utils      "/Users/bytedance/CLionProjects/typed-scheme/docs/test.utils.scm")
(import ManOrBoy   "/Users/bytedance/CLionProjects/typed-scheme/docs/test.MoB.scm")

;;;;;;;;;;;;;;;;;;;;;;;;;
;; AuroraScheme测试用例 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;

;; 简单的中缀表达式解析
;; 参见 The Little Schemer

(define k (quote + -))
(define j '*)

(define con
    (lambda (x y)
        (lambda (v)
            (if (eq? v 'car)
                x
                (if (eq? v 'cdr) y 77)))))

(define carr
    (lambda (l)
        (l 'car)))

(define cdrr
    (lambda (l)
        (l 'cdr)))

(define l (con 12 13))
(display (carr l))
(display (cdrr l))


