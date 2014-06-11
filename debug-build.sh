cc -DOPT_DEBUG -g `ls *.c | grep -v jsmain` debugmain/*.c -Demscripten_run_script=puts
