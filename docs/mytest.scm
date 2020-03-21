(import Utils      "/Users/bytedance/CLionProjects/typed-scheme/docs/test.utils.scm")
(import ManOrBoy   "/Users/bytedance/CLionProjects/typed-scheme/docs/test.MoB.scm")

;;;;;;;;;;;;;;;;;;;;;;;;;
;; AuroraScheme测试用例 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;

;; 简单的中缀表达式解析
;; 参见 The Little Schemer
(display "jack")
(Utils.show "jackk")
(display (= 2.12 2))
(display (- (+ 2.12 2) 12))

(define a (lambda (f) '(f + 12)))
(define numbered?
  (lambda (aexp)
    (cond ((atom? aexp) (number? aexp))
          ((atom? (car (cdr aexp))) (and (numbered? (car aexp)) (numbered? (car (cdr (cdr aexp))))))
          (else #f))))

(define value
  (lambda (aexp)
    (cond ((atom? aexp) aexp)
          ((eq? (car (cdr aexp)) '+)
           (+ (value (car aexp)) (value (car (cdr (cdr aexp))))))
          ((eq? (car (cdr aexp)) '-)
           (- (value (car aexp)) (value (car (cdr (cdr aexp))))))
          ((eq? (car (cdr aexp)) '*)
           (* (value (car aexp)) (value (car (cdr (cdr aexp))))))
          ((eq? (car (cdr aexp)) '/)
           (/ (value (car aexp)) (value (car (cdr (cdr aexp))))))
          (else (display "Unexpected operator")))))

;; (display (value '((1 / 3) - (1 / 4))))
(define f #f)
(define hello
  (lambda (num)
    (if (= num 12) 24 77)))

;; (define self (lambda (f) (+ f Utils.s)))
;; (define self (lambda (f) NoModule.s))
