SO-TP1
======
A pair of programs which display the use of Inter Process Communication (IPC)
    1. ./md5 - which distributes the load of calculating md5 hashes into multiple processes
    2. ./view - which through shared memory allows for the viewing of ./md5's output

Table of Contents
-----------------

    * [Requirements](#requirements)
    * [Usage](#usage)

Requirements
------------

This project requires the following to be built & to run

    * [Make][make]
    * [md5sum][md5sum]

Usage
-----
```sh
    ./md5 <file1> <file2> <file3> ... | ./view
```

or

# Terminal 1
```sh
    ./md5 <file1> <file2> <file3> ...
```
# Terminal 2
```sh
    ./view
```
(with less than 2 seconds of time between executions in term1 & term2)

[make]: https://www.gnu.org/software/make/
[md5sum]: https://www.gnu.org/software/coreutils/
