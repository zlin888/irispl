(define map
  (lambda (f l)
    (if (pair? l)
      (cons (f (car l)) (map f (cdr l)))
      (f (car l)))))
(display 1)