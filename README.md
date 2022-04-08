# Game-Trainer-Tutorials

A simple repo following some guides on creating internal and external game trainers.

## External Trainer

The external trainer folder contains the external trainer(s) from the Guided Hacking
youtube tutorials:

- Part 2: https://www.youtube.com/watch?v=wiX5LmdD5yk
- Part 3: https://www.youtube.com/watch?v=UMt1daXknes

It uses the [Assault Cube](https://assault.cubers.net/) game as a workbench.

### Part 2

The 'part 2' tutorial used Cheat Engine to find the location of the ammunition
value, and to trace back the pointers to find the offset(s) relative to the .exe.

These values were then used in the C++ project to start from the beginning of the
process's module and follow each pointer address with their relative offset to find
the ammo value.

Then it reads/writes this value as a proof-of-concept. The game will then reflect
the newly written amount of ammunition.

## Part 3

TODO

## Internal Trainer

Internal trainers involve injecting a .dll into the process to gain direct access
to the processes memory.

TODO