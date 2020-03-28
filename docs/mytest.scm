(import Utils      "/Users/bytedance/CLionProjects/typed-scheme/docs/test.utils.scm")
(import ManOrBoy   "/Users/bytedance/CLionProjects/typed-scheme/docs/test.MoB.scm")

(if (> 12 13) (display 8888) (0))
(display (- 12 8))
(cons '88 12 "string" (lambda (a b) (+ a b)))
(define list0 (cons '88 12 "string" (lambda (a b) (+ a b))))
(display (car list0))

