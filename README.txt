Github repo: https://github.com/AnargyrosArg/hy527

Most relevant code is in thread.c

Compilation with: make

It will compile all source files and produce object files. 
Also it will compile main.c and produce an executable ./a.out
main.c can be replaced with any example you would like to run.

Some implementation details:
- Each thread is given an 8KB stack
- Queues are implemented using linked lists
- Thread handles are the pointers returned by malloc -> automatically unique handle for every thread