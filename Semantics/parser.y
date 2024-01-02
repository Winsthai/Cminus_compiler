%{
    // C code
    #include<stdio.h>
    #include"scanner.h"
    #include"ast.h"
    struct decl *parser_result = 0;
    int yyerror(char *msg);
%}

%union {
    struct decl *decl;
    struct stmt *stmt;
    struct type *type;
    struct expr *expr;
    struct param_list *param_list;
    char *str;
    int num;
}

%type<decl> program decl_list decl var_decl func_decl local_decl

%type<stmt> compound_stmt stmt_list stmt expr_stmt selection_stmt iteration_stmt return_stmt

%type<type> type

%type<expr> expr var simple_expr rel_op add_expr add_op term mul_op factor call args arg_list 

%type<param_list> params param_list param

%type<str> ID

%type<num> NUM

%define parse.error verbose
%token ELSE IF INT RETURN VOID WHILE PLUS SUB MULT DIV LT LEQ GT GEQ EQ NEQ SET SEM_COL COMMA O_PAREN C_PAREN O_BRACK C_BRACK O_BRACE C_BRACE ID NUM

%expect 1

%%
// grammar rules

program: decl_list   {parser_result = $1;}

decl_list: decl decl_list  {$$ = $1; $1->next = $2;}
    | decl   {$$ = $1;}

decl: var_decl   {$$ = $1;}
    | func_decl  {$$ = $1;}

var_decl: type ID SEM_COL   {$$ = createDecl($1, $2, 0, 0, 0);}
    | type ID O_BRACK NUM C_BRACK SEM_COL   {$$ = createDecl(createType(TYPE_ARRAY, $1->kind, 0), $2, 0, 0, $4);}

type: INT   {$$ = createType(TYPE_INTEGER, 0, 0);}
    | VOID  {$$ = createType(TYPE_VOID, 0, 0);}

func_decl: type ID O_PAREN params C_PAREN compound_stmt {$$ = createDecl(createType(TYPE_FUNCTION, $1->kind, $4), $2, $6, 0, 0);}

params: param_list  {$$ = $1;}
    | VOID  {$$ = NULL;} 

param_list: param COMMA param_list    {$$ = $1; $1->next = $3;}
    | param {$$ = $1;}

param: type ID  {$$ = createParam_list($2, $1, 0);}
    | type ID O_BRACK C_BRACK   {$$ = createParam_list($2, createType(TYPE_ARRAY, $1->kind, 0), 0);}

compound_stmt: O_BRACE local_decl stmt_list C_BRACE {$$ = createStmt(STMT_DECL, $2, 0, $3, 0, 0);}

local_decl: var_decl local_decl {$$ = $1; $1->next = $2;}
    | %empty    {$$ = NULL;}

stmt_list: stmt stmt_list   {$$ = $1; $1->next = $2;}
    | %empty    {$$ = NULL;}

stmt: expr_stmt {$$ = $1;}
    | compound_stmt {$$ = $1;}
    | selection_stmt    {$$ = $1;}
    | iteration_stmt    {$$ = $1;}
    | return_stmt   {$$ = $1;}

expr_stmt: expr SEM_COL {$$ = createStmt(STMT_EXPR, 0, $1, 0, 0, 0);}
    | SEM_COL   {$$ = createStmt(STMT_EMPTY, 0, 0, 0, 0, 0);}

// Causes shift/reduce conflict; however, by default, Bison resolves this conflict by choosing to shift, which is correct for our grammar
selection_stmt: IF O_PAREN expr C_PAREN stmt    {$$ = createStmt(STMT_IF, 0, $3, $5, 0, 0);}
    | IF O_PAREN expr C_PAREN stmt ELSE stmt    {$$ = createStmt(STMT_IF_ELSE, 0, $3, $5, $7, 0);}

iteration_stmt: WHILE O_PAREN expr C_PAREN stmt {$$ = createStmt(STMT_WHILE, 0, $3, $5, 0, 0);}

return_stmt: RETURN SEM_COL {$$ = createStmt(STMT_RETURN, 0, 0, 0, 0, 0);}
    | RETURN expr SEM_COL   {$$ = createStmt(STMT_RETURN, 0, $2, 0, 0, 0);}

expr: var SET expr  {$$ = createExpr(ASSIGN_EXPR, $1, $3, 0, 0);}
    | simple_expr   {$$ = $1;}

var: ID {$$ = createExpr(NAME_EXPR, 0, 0, 0, $1);}
    | ID O_BRACK expr C_BRACK   {$$ = createExpr(SUBSCRIPT_EXPR, createExpr(NAME_EXPR, 0, 0, 0, $1), $3, 0, 0);}

simple_expr: add_expr rel_op add_expr   {$$ = createExpr($2->exprType, $1, $3, 0, 0);}
    | add_expr  {$$ = $1;}

rel_op: LEQ {$$ = createExpr(LEQ_EXPR, 0, 0, 0, 0);}
    | LT    {$$ = createExpr(LT_EXPR, 0, 0, 0, 0);}
    | GT    {$$ = createExpr(GT_EXPR, 0, 0, 0, 0);}
    | GEQ   {$$ = createExpr(GEQ_EXPR, 0, 0, 0, 0);}
    | EQ    {$$ = createExpr(EQ_EXPR, 0, 0, 0, 0);}
    | NEQ   {$$ = createExpr(NEQ_EXPR, 0, 0, 0, 0);}

add_expr: add_expr add_op term  {$$ = createExpr($2->exprType, $1, $3, 0, 0);}
    | term  {$$ = $1;}

add_op: PLUS    {$$ = createExpr(ADD_EXPR, 0, 0, 0, 0);}
    | SUB   {$$ = createExpr(SUB_EXPR, 0, 0, 0, 0);}

term: term mul_op factor    {$$ = createExpr($2->exprType, $1, $3, 0, 0);}
    | factor    {$$ = $1;}

mul_op: MULT    {$$ = createExpr(MULT_EXPR, 0, 0, 0, 0);}
    | DIV   {$$ = createExpr(DIV_EXPR, 0, 0, 0, 0);}

factor: O_PAREN expr C_PAREN    {$$ = $2;}
    | var   {$$ = $1;}
    | call  {$$ = $1;}
    | NUM   {$$ = createExpr(NUM_EXPR, 0, 0, $1, 0);}

call: ID O_PAREN args C_PAREN   {$$ = createExpr(CALL_EXPR, createExpr(NAME_EXPR, 0, 0, 0, $1), $3, 0, 0);}

args: arg_list  {$$ = $1;}
    | %empty    {$$ = createExpr(ARG_EXPR, 0, 0, 0, 0);}

arg_list: expr COMMA arg_list   {$$ = createExpr(ARG_EXPR, $1, $3, 0, 0);}
    | expr  {$$ = createExpr(ARG_EXPR, $1, 0, 0, 0);}

%%

// C code 

int main(int argc, char **argv){
    if(argc > 1){
        yyin = fopen(argv[1], "r");
        if(!yyin){
            printf("error reading the file: %s\n", argv[1]);
            return 1;
        }
    } else {
        printf("usage: ./parser <filename>");
        return 1;
    }

    int result = yyparse();

    if(result == 0){
        // Initialize symbol table with global input and output function definitions
        struct symbolTable *symTableInit = createSymbolTable(100);
        struct symbol *inputInit = createSymbol(SYMBOL_GLOBAL, createType(TYPE_FUNCTION, TYPE_INTEGER, NULL), "input");
        struct symbol *outputInit = createSymbol(SYMBOL_GLOBAL, 
                                    createType(TYPE_FUNCTION, TYPE_VOID, createParam_list("y", createType(TYPE_INTEGER, 0, 0), NULL)), 
                                    "output");
        insert_to_st(symTableInit, "input", inputInit);
        insert_to_st(symTableInit, "output", outputInit);

        // Initialize the stack
        struct symbolTableNode *stack = createSymbolTableNode(symTableInit);
        check_decl(parser_result, stack);

        printf("program\n");
        printDecl(parser_result, 1);
        printf("\nparsing successful\n");
    } else {
        printf("\nparsing failed\n");
    }

    return result;

}

int yyerror(char *msg){
    printf("error: parsing failed at or near line %d: %s\n", yylineno, msg);
    return 1;
}

