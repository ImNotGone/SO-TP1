SO-TP1
======
A pair of programs which display the use of Inter Process Communication (IPC)
 1. md5 - which distributes the load of calculating md5 hashes into multiple processes
 2. view - which through shared memory allows for the viewing of md5's output

Table of Contents
-----------------

 * [Requirements](#requirements)
 * [Build](#build)
 * [Usage](#usage)
 * [Static-code-analysis](#static-code-analysis)

Requirements
------------

This project requires the following to be built & run
 * [Docker][docker]
 * [Agodio-Image][agodio] or [ImNotGone-Image][imnotgone]

Agodio's image requires adittional steps to for example update pvs-studio (linked below)

Additional information on how to install docker & agodio's image [here][aquili-docker], to use ImNotGone's image just replace `agodio/itba-so:1.0` with `imnotgone/itba-so` everywhere you see it

To start the container and execute code inside it, run the following command:
```sh
docker run -v "${PWD}:/root" --privileged --rm -ti $SO_IMAGE
```
This will let you run commands on your image of choice nedded for this project to be built & run.

Where the /root folder will attach to your current directory & `$SO_IMAGE` is your image of choice (`agodio/itba-so:1.0` or `imnotgone/itba-so`).

(make sure to be in /root when trying to build using make)

Build
-----
To create both ./md5 and ./view the following command should be executed inside the containers
```sh
make all
```

Usage
-----
```sh
./md5 file1 file2 file3 ... | ./view
```

or

### Terminal 1 (md5 will print info for the view process to connect)
```sh
./md5 file1 file2 file3 ...
```
### Terminal 2 (this info should be passed as arguments, or stdin inputs)
 *     ./view <INFO1> <INFO2> ...
        
 *     ./view
       <INFO1><ENTER>
       <INFO2><ENTER>
       ...
(with less than 2 seconds of time between executions in term1 & term2)

To test the two terminal execution you may start up a container using the command show above, and then run this command to enter the same container in another terminal session:
```sh
docker exec -it $(docker ps -q) bash
```
The terminal session will be opened on your most recent running docker container

Static-code-analysis
--------------------
To run the static code analysis tools use the following commands
### pvs-studio (requires and update when using agodio's image, steps [here][aquili-pvs])
```sh
make pvs
```
### cpp-check
```sh
make cpp-check
```

[docker]: https://www.docker.com/
[agodio]: https://hub.docker.com/r/agodio/itba-so/
[imnotgone]: https://hub.docker.com/r/imnotgone/itba-so/
[aquili-docker]: https://github.com/alejoaquili/ITBA-72.11-SO/blob/main/docker/README.md/
[aquili-pvs]: https://github.com/alejoaquili/ITBA-72.11-SO/blob/main/static-code-analysis/pvs-studio.md/