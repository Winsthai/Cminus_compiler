# Scanner & Parser Using Flex and Bison + Semantic Analyzer

### To execute the provided executable on a file "test_file", run:
`./parser <test_file>`

### The output of the program is similar to the output of the reference compiler, and thus can be interpreted in the same way. It also contains additional semantic information for nodes in the AST.

### If you want to generate the executable (and the other utility files), follow the steps below

### For generating the scanner, run:
`flex --header-file=scanner.h -o scanner.c scanner.l`

### For generating the parser, run:
`bison -dvo parser.c parser.y`

### To generate the executable, run:
`gcc scanner.c parser.c -o parser`