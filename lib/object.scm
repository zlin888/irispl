(define selector-undefined-error 'selector-undefined-error)

(define object
  (lambda (selector) selector-undefined-error))

(define =>
  (lambda (instance method . args)
    (let ((_method (instance method)))
      (apply _method (cons instance args)))))