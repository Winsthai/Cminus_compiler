## A compiler for the programming language C-, a subset of the language C. 
### Details regarding the syntax and semantics of the language can be seen [here](https://github.com/Winsthai/Cminus_compiler/blob/main/C-%20definition.pdf).

## Stages

- [x] Part 1: Scanner
  - Utilizes flex (Fast Lexical Analyzer Generator) to tokenise input code to feed to the parser
  - Also checks for unknown characters and basic syntax errors such as unclosed comments
- [x] Part 2: Parser
  - Utilizes GNU Bison to parse the tokens received from the scanner
  - Ensures that the input code conforms to the grammar of the language
  - Checks for any syntax errors
  - Also creates and prints an abstract syntax tree; a data structure which represents the structure of the code in the form of a tree.
- [x] Part 3: Semantic analyzer
  - Traverses the abstract syntax tree and checks for any semantic errors
  - Performs **type checking**; such as ensuring that a function is called with arguments of the right type
  - Performs **name resolution**:
    - Utilizes a hand-made data structure which consists of a stack of symbol tables (hash tables)
    - While traversing the abstract syntax tree, variables are added to this data structure to monitor their scope and type in order to prevent semantic errors
    - For instance, this ensures that variables can be redeclared within different scopes and that a variable is only used in ways that is compatible with the type it was declared with
- [ ] Part 4: Code generation



Special thanks to the book "*Introduction to Compilers and Language design*" by Douglas Thain, as this was a resource which helped greatly in the creation of this project.


