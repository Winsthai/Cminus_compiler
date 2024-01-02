# Scanner & Parser Using Flex and Bison + Semantic Analyzer

## To execute the provided executable on a file "test_file", run:
`./parser <test_file>` or `parser <test_file>`

This prints the program in the form of an abstract syntax tree which also contains additional semantic information (including scope and type) for nodes in the AST.

### If you want to generate the executable (and the other utility files included), follow the steps below

### *Note: Requires GNU Bison (version 3.8.2+) and flex (version 2.6.4+) to be installed*

### For generating the scanner, run:
`flex --header-file=scanner.h -o scanner.c scanner.l`

### For generating the parser, run:
`bison -dvo parser.c parser.y`

### To generate the executable, run:
`gcc scanner.c parser.c -o parser`
