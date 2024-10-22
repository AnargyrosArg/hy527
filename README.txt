Github repo: https://github.com/AnargyrosArg/hy527

Most relevant code is in thread.c

Compilation with: make

It will compile all examples and thread "library" files into object and executable files.
The executable for each example has the same name as the example:
e.g
    ./sort

Some implementation details:
- Each thread is given an 8KB stack
- Queues are implemented using linked lists
- Thread handles are the pointers returned by malloc -> automatically unique handle for every thread