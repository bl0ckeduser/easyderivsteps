cc -DOPT_DEBUG -DDESC_GEN -g `ls *.c | grep -v jsmain` debugmain/*.c -Demscripten_run_script=puts
