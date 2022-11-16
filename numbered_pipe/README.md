## Usage example
Note: remember to copy for command into `bin` or change PATH to correct directory. Otherwise, command will not be found.

```bash
> printenv PATH  # Output: bin:.

> setenv PATH bin

> ls  # Output: Makefile npshell npshell.cpp npshell.hpp README.md

> echo "2 more command" |2  # pipe output of `ls` to next next command

> echo "1 more command" |1  # pipe output of `ls` to next command

> cat  # Output: aa bb

