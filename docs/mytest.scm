(display (let (
        (var1 12) (var2 99))
        (+ var1 var2)))

(display (pair? (cdr (cdr (cons 11 12)))))

(lambda (x y . others) (+ x y))