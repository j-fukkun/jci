This is my tiny C compiler, *jcc*.

*jcc* compiles the subset of C into an assembly code through the following steps:
1. Lexical analysis: converts input strings into *tokens*.
2. Syntax analysis: builds abstract syntax trees (ASTs) from the tokens.
3. Generates intermediate representation (IR) code from the ASTs.
4. Optimizes the IR code.
5. Register allocation.
6. Generates x86-64 code from the IR code.

The steps 1, 2, 3, and 6 of the above ones are implemented by referring [9cc](https://github.com/rui314/9cc) and [chibicc](https://github.com/rui314/9cc) written by [@rui314](https://github.com/rui314).