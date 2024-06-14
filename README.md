# ISO Helper Project

The goal of this project is to create a UNIX program capable of reading the content of an .iso file under the 9660 standard

## Commandes

- `help` : help: display command help

- `info` : display volume info

- `ls` : display the content of a directory

- `cat <file>` : display file content

- `cd <dir>` : change current directory

- `pwd` : print current path

- `get <file>`: copy file to local directory

- `quit` : exit program

## Utilisation

- `42sh$ echo info | ./isohelper <path_to_iso>`
- `42sh$ echo ls | ./isohelper <path_to_iso>`
- `./isohelper <path_to_iso>` -> to use `cd`