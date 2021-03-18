# rt-matrix-mpi
Multiplicación paralela de matrices para Diseño de Sistemas en Tiempo Real

Para compilar debe definirse el valor de la constante `N` a través de la opción `-D` del compilador (por ejemplo, para compilar con un tamaño de matriz igual a 1024, debe ejecutarse `mpicc -D N=1024 mpi.c`)

Además, es posible realizar la multiplicación por bloques o de forma lineal, según se defina o no la constante `BLOCKS` a través de la opción `-D` del compilador (para multiplicar matrices de 2048x2048 de forma lineal, debe ejecutarse `mpicc -D N=2048 mpi.c`; para multiplicar por bloques, `mpicc -D N=2048 -D BLOCKS mpi.c`)
