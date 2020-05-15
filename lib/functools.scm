(define map
  (lambda (f l)
    (if (pair? l)
      (cons (f (car l)) (map f (cdr l)))
      (f (car l)))))

(define reduce
  (lambda (f l init)
    (if (pair? l)
      (reduce f (cdr l) (f (car l) init))
      (f (car l) init))))
