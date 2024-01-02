#include <stdlib.h>
#include <stdio.h>

typedef enum {
    ADD_EXPR,
    SUB_EXPR,
    MULT_EXPR,
    DIV_EXPR,
    NUM_EXPR,
    CALL_EXPR,
    NAME_EXPR,
    ARG_EXPR,
    ASSIGN_EXPR,
    SUBSCRIPT_EXPR,
    LT_EXPR,
    GT_EXPR,
    LEQ_EXPR,
    GEQ_EXPR,
    EQ_EXPR,
    NEQ_EXPR
} expr_t;

typedef enum {
    STMT_DECL,
    STMT_EXPR,
    STMT_IF_ELSE,
    STMT_IF,
    STMT_WHILE,
    STMT_RETURN,
    STMT_EMPTY
} stmt_t;

typedef enum {
    TYPE_INTEGER,
    TYPE_VOID,
    TYPE_ARRAY,
    TYPE_FUNCTION
} type_t;

typedef enum {
    SYMBOL_LOCAL,
    SYMBOL_PARAM,
    SYMBOL_GLOBAL
} symbol_t;

// Symbol structure
struct symbol {
    symbol_t kind;
    struct type *type;
    char *name;
    struct symbol *next; // Points to another symbol in case of chaining in hash table
};

struct symbol *createSymbol(symbol_t kind, struct type *type, char *name) {
    struct symbol *s = malloc(sizeof(*s));
    s->kind = kind;
    s->type = type;
    s->name = name;
    return s;
};

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Implementing hash table ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

// Define the structure for the symbol table
struct symbolTable{
    int size;
    struct symbol** table;
};

// Function to create a new symbol table
struct symbolTable *createSymbolTable(int size) {
    struct symbolTable *symTable = malloc(sizeof(*symTable));
    symTable->size = size;

    // Allocate memory for the table
    symTable->table = (struct symbol**)malloc(sizeof(struct symbol*) * size);

    // Initialize each entry in the table to NULL
    for (int i = 0; i < size; i++) {
        symTable->table[i] = NULL;
    }

    return symTable;
}

// Hash function algorithm for strings (might not be fully optimized, but good enough for this implementation)
int hash(char* key, int size) {
    int hashValue = 0;

    // djb2 hash function algorithm
    while (*key != '\0') {
        hashValue = ((hashValue << 5) + hashValue) + *key++;
    }

    return hashValue % size;
}

// Function to insert a key (name) value (symbol structure) pair into the symbol table
void insert_to_st(struct symbolTable* symTable, char* key, struct symbol *value) {
    value->next = NULL;
    int index = hash(key, symTable->size);

    // Insert the entry into the hash table
    if (symTable->table[index] == NULL) {
        symTable->table[index] = value;
    } 
    else {
        // Handle collision by chaining (linked list)
        struct symbol *current = symTable->table[index];
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = value;
    }
}

// Function to look up a key in the symbol table
// Returns the symbol associated with that key; NULL if there was no key
struct symbol *lookup(struct symbolTable *symTable, char *name) {
    int index = hash(name, symTable->size);

    // Search for the key in the chain
    struct symbol *current = symTable->table[index];
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }

    // If key is not found, return NULL
    return NULL;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Hash table implementation end ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Implementing stack of symbol tables ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

// Define the structure for a symbol table in the stack
struct symbolTableNode {
    struct symbolTable *symTable;
    struct symbolTableNode *next; // Pointer to the next node in the stack
};

// Function to create new symbol table node in stack
struct symbolTableNode* createSymbolTableNode(struct symbolTable *symTable) {
    struct symbolTableNode* node = malloc(sizeof(*node));
    node->symTable = symTable;
    node->next = NULL;
    return node;
}

// Function to push a symbol table onto the stack
// symTable = symbol table to push onto top of stack
void scope_enter(struct symbolTableNode **stack, struct symbolTable *symTable) {
    struct symbolTableNode *node = createSymbolTableNode(symTable);
    node->next = *stack;
    *stack = node;
}

// Function to pop a symbol table from the stack
void scope_exit(struct symbolTableNode **stack) {
    // If stack is empty, return (shouldn't happen)
    if (*stack == NULL) {
        return;
    }

    struct symbolTableNode *top = *stack;
    struct symbolTable *symTable = top->symTable;
    *stack = top->next;
}

// Function that returns number of symbol tables currently in stack
int scope_level(struct symbolTableNode *stack) {
     int i = 0;
     struct symbolTableNode *currentSymTable = stack;
     while(currentSymTable != NULL) {
        currentSymTable = currentSymTable->next;
        i++;
     }
     return i;
}

/* Looks for whether symbol was defined in any scope
Return NULL if no symbol was found */
struct symbol *scope_lookup(struct symbolTableNode *stack, char *name) {
    struct symbolTableNode *currentSymTable = stack;

    while (currentSymTable != NULL) {
        struct symbol *lookup_sym = lookup(currentSymTable->symTable, name);
        if (lookup_sym != NULL) {
            return lookup_sym;
        }
        currentSymTable = currentSymTable->next;
    }

    // Return NULL if no symbol was found
    return NULL;
}

// Looks for whether symbol was defined in current scope
struct symbol *scope_lookup_current(struct symbolTableNode *stack, char *name) {
    return lookup(stack->symTable, name);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Implementing stack of symbol tables end ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/* Struct for declarations */
struct decl {
    struct type *type;
    char *name;
    struct stmt *code;
    struct decl *next;
    int size;
    // For semantic information
    symbol_t scope;
};

/* Function that allocates memory for declarations and its fields */
struct decl *createDecl(struct type *type, char *name, struct stmt *code, struct decl *next, int size) {
    struct decl *newDecl = malloc(sizeof(*newDecl));
    newDecl->type = type;
    newDecl->name = name;
    newDecl->code = code;
    newDecl->next = next;
    newDecl->size = size;

    return newDecl;
};

struct stmt {
    stmt_t kind;
    struct decl *decl;
    struct expr *expr;
    struct stmt *body;
    struct stmt *else_body;
    struct stmt *next;
};

struct stmt *createStmt(stmt_t kind, struct decl *decl, struct expr *expr, struct stmt *body, struct stmt *else_body, struct stmt *next){
    struct stmt *newStmt = malloc(sizeof(*newStmt));
    newStmt->kind = kind;
    newStmt->decl = decl;
    newStmt->expr = expr;
    newStmt->body = body;
    newStmt->else_body = else_body;
    newStmt->next = next;

    return newStmt;
}

struct expr {
    expr_t exprType;
    struct expr *left;
    struct expr *right;
    int value;
    char *name;
    // For semantic information
    symbol_t scope;
    struct type *type;
};

struct expr *createExpr(expr_t exprType, struct expr *left, struct expr *right, int value, char *name){
    struct expr *newExpr = malloc(sizeof(*newExpr));
    newExpr->exprType = exprType;
    newExpr->left = left;
    newExpr->right = right;
    newExpr->value = value;
    newExpr->name = name;

    return newExpr;
}

/* Struct for type specification   
   subtype field is used for array type or function return type
   params field is used for function parameters */
struct type {
    type_t kind;
    type_t subtype;
    struct param_list *params;
};

struct param_list {
    char *name;
    struct type *type;
    struct param_list *next;
};

/* Function that allocates memory for type specification and its fields */
struct type *createType(type_t kind, type_t subtype, struct param_list *params) {
    struct type *newType = malloc(sizeof(*newType));
    newType->kind = kind;
    newType->subtype = subtype;
    newType->params = params;

    return newType;
}

struct param_list *createParam_list(char *name, struct type *type, struct param_list *next) {
    struct param_list *newParam_list = malloc(sizeof(*newParam_list));
    newParam_list->name = name;
    newParam_list->type = type;
    newParam_list->next = next;

    return newParam_list;
}

/* Helper function for indentation when printing */
char* getTabs(int indents) {
    char* tabs = malloc(indents * 5);
    for (int i = 0; i < indents * 4; i++) {
        strcat(tabs, " ");
    }
    return tabs;
}

/* Helper function to get type of an array or return type of function when printing */
char* getSubtype(struct type *type) {
    switch (type->subtype) {
        case TYPE_INTEGER:
            return "int";
            break;
        case TYPE_VOID:
            return "void";
            break;
        default:
            break;
    }
}

void check_decl(struct decl *d, struct symbolTableNode *stack);
void check_stmt(struct stmt *s, struct symbolTableNode *stack, char *name);
struct type *check_expr(struct expr *e, struct symbolTableNode *stack, struct param_list *params);
void check_params(struct param_list *p, struct symbolTableNode *stack);

void check_decl(struct decl *d, struct symbolTableNode *stack) {
    if (!d) return;

    // Get current scope; insert declaration into stack
    symbol_t kind = scope_level(stack) > 1 ? SYMBOL_LOCAL : SYMBOL_GLOBAL;
    struct symbol *new_sym = createSymbol(kind, d->type, d->name);

    // Test that a function is not declared twice in the same scope
    struct symbol *scope_test = scope_lookup_current(stack, d->name);
    if (scope_test != NULL) {
        printf("Error: \"%s\" redefined in the same scope\n");
        exit(-1);
    }

    // Otherwise, insert into the symbol table
    insert_to_st(stack->symTable, d->name, new_sym);

    // Update scope for AST printing
    d->scope = kind;

    switch (d->type->kind) {
        case TYPE_INTEGER:
            break;

        case TYPE_ARRAY:
            if (d->type->subtype == TYPE_VOID) {
                printf("Error: array declaration cannot be of type void\n");
                exit(-1);
            }
            break;

        case TYPE_FUNCTION:
            // Create new symbol table
            struct symbolTable *newSt = createSymbolTable(100);
            
            // Call check stmt and params on the inner code with a lower scope, then return to previous scope afterwards
            scope_enter(&stack, newSt);
            check_decl(d->code->decl, stack);
            check_params(d->type->params, stack);
            check_stmt(d->code->body, stack, d->name);
            scope_exit(&stack);
            break;

        case TYPE_VOID:
            printf("Error: variable declaration cannot be of type void\n");
            exit(-1);
            break;
    }

    // If there is no more declarations and we are in global scope, the last function must be of form "void main(void)"
    if (d->next == NULL && scope_level(stack) == 1
        && (strcmp(d->name, "main") != 0 || d->type->kind != TYPE_FUNCTION || d->type->subtype != TYPE_VOID || d->type->params != NULL)) {
            printf("Error: last global declaration is not main function of form \"void main(void)\"\n");
            exit(-1);
    }

    check_decl(d->next, stack);
}

// Pass name of the function to see if return statement matches type of function declared
void check_stmt(struct stmt *s, struct symbolTableNode *stack, char *name) {
    if (!s) return;

    switch (s->kind) {
        // Compound statement
        case STMT_DECL:
            // Go into inner scope
            struct symbolTable *newSt = createSymbolTable(100);
            scope_enter(&stack, newSt);
            check_decl(s->decl, stack);
            check_stmt(s->body, stack, name);
            scope_exit(&stack);
            break;

        case STMT_EXPR:
            check_expr(s->expr, stack, NULL);
            break;

        case STMT_IF_ELSE:
            check_expr(s->expr, stack, NULL);
            check_stmt(s->body, stack, name);
            check_stmt(s->else_body, stack, name);
            break;

        case STMT_IF:
            check_expr(s->expr, stack, NULL);
            check_stmt(s->body, stack, name);
            break;

        case STMT_WHILE:
            check_expr(s->expr, stack, NULL);
            check_stmt(s->body, stack, name);
            break;

        // Must match output type of the function
        // void -> return;
        // int -> return expr; expr is type int
        case STMT_RETURN:
            struct symbol *func = scope_lookup(stack, name);
            type_t return_type = func->type->subtype;
            // Check that void functions are not returning anything
            if (return_type == TYPE_VOID && s->expr != NULL) {
                printf("Error: returning value in void function\n");
                exit(-1);
            }
            // Check that int functions are not returning nothing
            else if (return_type == TYPE_INTEGER && s->expr == NULL) {
                printf("Error: empty return statement in int function\n");
                exit(-1);
            }
            check_expr(s->expr, stack, NULL);
            break;

        case STMT_EMPTY:
            break;
    }

    check_stmt(s->next, stack, name);
}

struct type *check_expr(struct expr *e, struct symbolTableNode *stack, struct param_list *params) {
    if (!e) return 0;

    struct type *result = malloc(sizeof(*result));

    struct type *lt = malloc(sizeof(*lt));
    lt = check_expr(e->left, stack, NULL);

    // Special cases: When a function is called, we need to pass its parameters down the AST to ensure that they are valid
    if (e->exprType == CALL_EXPR) {
        // Check if left child matches type function
        // If not, then Error: Calling something that isn’t a function
        if (lt->kind != TYPE_FUNCTION) {
            printf("Error: Calling something that isn't a function\n");
            exit(-1);
        }          

        // Need to get symbol of name of function being called 
        struct symbol *func = scope_lookup(stack, e->left->name);

        // Pass list of parameters to the function call arguments to be able to type check
        struct param_list *func_params = func->type->params;
        struct type *rt = malloc(sizeof(*rt));
        rt = check_expr(e->right, stack, func_params);

        // Return the return type of the function called
        type_t return_type = func->type->subtype;
        result = createType(return_type, 0, 0);
        e->type = result;
        e->scope = func->kind;
        return result;
    }
    else if (e->exprType == ARG_EXPR) {
        // If function was declared with no arguments, check if there are no children of the expression; if so, this is a valid call
        if (params == NULL && e->left == NULL && e->right == NULL) {return NULL;}

        // This occurs if there are more parameters in the function call then there are in the declaration
        if (params == NULL) {
            printf("Error: call of function does not match number of arguments from function declaration, too many arguments\n");
            exit(-1);
        }

        // This occurs only if there are too few arguments in the function call;
        // the AST has no more arguments, but there is still more parameters
        if (params->next != NULL && e->right == NULL) {
            printf("Error: call of function does not match number of arguments from function declaration, too few arguments\n");
            exit(-1);
        }

        // This occurs if an argument in the function call is not of the right type
        if (lt->kind != params->type->kind) {
            printf("Error: call of function does not match type of arguments from function declaration\n");
            exit(-1);
        }

        // Otherwise, continue checking the function call arguments
        struct param_list *func_params = params->next;
        struct type *rt = malloc(sizeof(*rt));
        rt = check_expr(e->right, stack, func_params);

        return NULL;
    }

    struct type *rt = malloc(sizeof(*rt));
    rt = check_expr(e->right, stack, NULL);

    switch (e->exprType) {
        case ADD_EXPR:
            if (lt->kind!=TYPE_INTEGER || rt->kind!=TYPE_INTEGER) {
                printf("Error: Operand mismatch for '+'\n");
                exit(-1);
            }
            result = createType(TYPE_INTEGER, 0, 0);
            break;
        
        case SUB_EXPR:
            if (lt->kind!=TYPE_INTEGER || rt->kind!=TYPE_INTEGER) {
                printf("Error: Operand mismatch for '-'\n");
                exit(-1);
            }
            result = createType(TYPE_INTEGER, 0, 0);
            break;
        
        case MULT_EXPR:
            if (lt->kind!=TYPE_INTEGER || rt->kind!=TYPE_INTEGER) {
                printf("Error: Operand mismatch for '*'\n");
                exit(-1);
            }
            result = createType(TYPE_INTEGER, 0, 0);
            break;

        case DIV_EXPR:
            if (lt->kind!=TYPE_INTEGER || rt->kind!=TYPE_INTEGER) {
                printf("Error: Operand mismatch for '/'\n");
                exit(-1);
            }
            result = createType(TYPE_INTEGER, 0, 0);
            break;

        case NUM_EXPR:
            result = createType(TYPE_INTEGER, 0, 0);
            break;

        case NAME_EXPR:
            // Check if e->name exists in symbol table
            // If not, then Error: Unknown name "e->name"
            // If yes, then result = `type specified in symbol table for e->name`
            struct symbol *temp = scope_lookup(stack, e->name);
            if (temp == NULL) {
                printf("Error: Unknown name \"%s\"\n", e->name);
                exit(-1);
            }
            e->scope = temp->kind;
            result = createType(temp->type->kind, 0, 0);
            break;

        case ASSIGN_EXPR:
            // If left and right do not have the same type, print error
            if (lt->kind != rt->kind) {
                printf("Error: Operand mismatch for '='\n");
                exit(-1);
            }
            break;

        case SUBSCRIPT_EXPR:
            if (lt->kind == TYPE_ARRAY) {
                // This error should already be detected by the parser, but included anyway
                if (rt->kind != TYPE_INTEGER) {
                    printf("Error: Index of array is not an integer\n");
                    exit(-1);
                }
                struct symbol *temp = scope_lookup(stack, e->left->name);
                e->scope = temp->kind;
                result = createType(TYPE_INTEGER, 0, 0);
            } 
            else {
                printf("Error: Indexing something that isn’t an array\n");
                exit(-1);
            }
            break;

        case LT_EXPR:
            if (lt->kind!=TYPE_INTEGER || rt->kind!=TYPE_INTEGER) {
                printf("Error: Operand mismatch for '<'\n");
                exit(-1);
            }
            result = createType(TYPE_INTEGER, 0, 0);
            break;

        case GT_EXPR:
            if (lt->kind!=TYPE_INTEGER || rt->kind!=TYPE_INTEGER) {
                printf("Error: Operand mismatch for '>'\n");
                exit(-1);
            }
            result = createType(TYPE_INTEGER, 0, 0);
            break;

        case LEQ_EXPR:
            if (lt->kind!=TYPE_INTEGER || rt->kind!=TYPE_INTEGER) {
                printf("Error: Operand mismatch for '<='\n");
                exit(-1);
            }
            result = createType(TYPE_INTEGER, 0, 0);
            break;

        case GEQ_EXPR:
            if (lt->kind!=TYPE_INTEGER || rt->kind!=TYPE_INTEGER) {
                printf("Error: Operand mismatch for '>='\n");
                exit(-1);
            }
            result = createType(TYPE_INTEGER, 0, 0);
            break;

        case EQ_EXPR:
            if (lt->kind!=TYPE_INTEGER || rt->kind!=TYPE_INTEGER) {
                printf("Error: Operand mismatch for '=='\n");
                exit(-1);
            }
            result = createType(TYPE_INTEGER, 0, 0);
            break;

        case NEQ_EXPR:
            if (lt->kind!=TYPE_INTEGER || rt->kind!=TYPE_INTEGER) {
                printf("Error: Operand mismatch for '|='\n");
                exit(-1);
            }
            result = createType(TYPE_INTEGER, 0, 0);
            break;
    }
    e->type = result;

    return result;
}

void check_params(struct param_list *p, struct symbolTableNode *stack) {
    if (!p) return;

    // Creating symbol to insert parameter into stack
    symbol_t kind = SYMBOL_PARAM;
    struct symbol *new_sym = createSymbol(kind, p->type, p->name);

    // Test that a variable is not declared twice in the same scope
    struct symbol *scope_test = scope_lookup_current(stack, p->name);
    if (scope_test != NULL) {
        printf("Error: \"%s\" redefined in the same scope\n");
        exit(-1);
    }

    // Otherwise, insert into the symbol table
    insert_to_st(stack->symTable, p->name, new_sym);

    check_params(p->next, stack);
}

char* getType(struct type *t) {
    switch (t->kind) {
        case TYPE_INTEGER:
            return "int";
            break;
        case TYPE_VOID:
            return "void";
            break;
        case TYPE_ARRAY:
            return "array of ints";
            break;
        case TYPE_FUNCTION:
            return "func";
            break;
    }
}

char* getScope(symbol_t s) {
    switch(s) {
        case SYMBOL_GLOBAL:
            return "global";
            break;
        case SYMBOL_LOCAL:
            return "local";
            break;
        case SYMBOL_PARAM:
            return "param";
            break;
    }
}

void printStmt(struct stmt *stmt, int indents);
void printDecl(struct decl *decl, int indents);
void printParams(struct param_list *param_list, int indents);
void printExpr(struct expr *expr, int indents);

void printStmt(struct stmt *stmt, int indents) {
    if (!stmt) {
        return;
    }

    char* tabs = getTabs(indents);

    switch(stmt->kind) {
        case STMT_DECL:
            /* Printing code block: Declarations then statements */
            printf("%sblock\n", tabs);
            printf("%s    declarations\n", tabs);
            printDecl(stmt->decl, (indents + 2));

            printf("%s    statements\n", tabs);
            printStmt(stmt->body, (indents + 2));
            break;
            
        case STMT_EXPR:
            printf("%sexprstmt\n", tabs);
            printExpr(stmt->expr, (indents + 1));
            break;

        case STMT_IF_ELSE:
            printf("%sif_else\n", tabs);
            printExpr(stmt->expr, (indents + 1));
            printStmt(stmt->body, (indents + 1));
            printStmt(stmt->else_body, (indents + 1));
            if (stmt->next && stmt->next->kind == STMT_DECL) {
                printStmt(stmt->next, indents + 2);
                stmt->next = NULL;
            }
            break;
        
        case STMT_IF:
            printf("%sif\n", tabs);
            printExpr(stmt->expr, (indents + 1));
            printStmt(stmt->body, (indents + 1));
            if (stmt->next && stmt->next->kind == STMT_DECL) {
                printStmt(stmt->next, indents + 2);
                stmt->next = NULL;
            }
            break;

        case STMT_WHILE:
            printf("%swhile\n", tabs);
            printExpr(stmt->expr, (indents + 1));
            printStmt(stmt->body, (indents + 1));
            if (stmt->next && stmt->next->kind == STMT_DECL) {
                printStmt(stmt->next, indents + 2);
                stmt->next = NULL;
            }
            break;

        case STMT_RETURN:
            printf("%sreturn\n", tabs);
            printExpr(stmt->expr, (indents + 1));
            break;

        case STMT_EMPTY:
            printf("%semptystmt\n", tabs);
            break;
    }

    if (stmt->next) {
        printStmt(stmt->next, indents);
    }
}

void printDecl(struct decl *decl, int indents) {
    if (!decl) {
        return;
    }

    char* tabs = getTabs(indents);

    switch (decl->type->kind) {
        case TYPE_INTEGER:
            printf("%svar_decl (scope [%s])\n", tabs, getScope(decl->scope));
            printf("%s    type [int]\n", tabs);
            printf("%s    new_id [%s]\n", tabs, decl->name);
            break;

        case TYPE_VOID:
            printf("%svar_decl (scope [%s])\n", tabs, getScope(decl->scope));
            printf("%s    type [void]\n", tabs);
            printf("%s    new_id [%s]\n", tabs, decl->name);
            break;

        case TYPE_ARRAY:
            printf("%sarray_decl (scope [%s])\n", tabs, getScope(decl->scope));
            printf("%s    type [%s]\n", tabs, getSubtype(decl->type));
            printf("%s    new_id [%s]\n", tabs, decl->name);
            printf("%s    int [%d]\n", tabs, decl->size);
            break;

        case TYPE_FUNCTION:
            printf("%sfunc (scope [%s])\n", tabs, getScope(decl->scope));
            printf("%s    type [%s]\n", tabs, getSubtype(decl->type));
            printf("%s    new_id [%s]\n", tabs, decl->name);

            /* Printing parameters */
            printf("%s    parameters\n", tabs);
            printParams(decl->type->params, (indents + 2));

            /* Printing inner code */
            /* Printing code block: Declarations then statements */
            printf("%s    block\n", tabs);
            printf("%s        declarations\n", tabs);
            printDecl(decl->code->decl, (indents + 3));

            printf("%s        statements\n", tabs);
            printStmt(decl->code->body, (indents + 3));
            break;
    }

    if (decl->next) {
        printDecl(decl->next, indents);
    }
}

void printParams(struct param_list *param_list, int indents) {
    if (!param_list) {
        return;
    }

    char* tabs = getTabs(indents);

    switch (param_list->type->kind) {
        case TYPE_INTEGER:
            printf("%svar_param\n", tabs);
            printf("%s\ttype [int]\n", tabs);
            printf("%s\tnew_id [%s]\n", tabs, param_list->name);
            break;

        case TYPE_VOID:
            printf("%svar_param\n", tabs);
            printf("%s\ttype [void]\n", tabs);
            printf("%s\tnew_id [%s]\n", tabs, param_list->name);
            break;

        case TYPE_ARRAY:
            printf("%sarray_param\n", tabs);
            printf("%s\ttype [%s]\n", tabs, getSubtype(param_list->type));
            printf("%s\tnew_id [%s]\n", tabs, param_list->name);
            break;
    }

    if (param_list->next) {
        printParams(param_list->next, indents);
    }
}

void printExpr(struct expr *expr, int indents){
    if(!expr){
        return;
    }

    char* tabs = getTabs(indents);

    switch (expr->exprType) {
        case ADD_EXPR:
            printf("%s+ (type [%s])\n", tabs, getType(expr->type));
            break;

        case SUB_EXPR:
            printf("%s- (type [%s])\n", tabs, getType(expr->type));
            break;

        case DIV_EXPR:
            printf("%s/ (type [%s])\n", tabs, getType(expr->type));
            break;

        case MULT_EXPR:
            printf("%s* (type [%s])\n", tabs, getType(expr->type));
            break;

        case NUM_EXPR:
            printf("%sint [%d] (type [int])\n", tabs, expr->value);
            break;

        case CALL_EXPR:
            printf("%sfunccall (type [%s])\n", tabs, getType(expr->type));
            printExpr(expr->left, (indents + 1));
            printf("%s    arguments\n", tabs);
            if (expr->right) {
                printExpr(expr->right, (indents + 1));
            }
            expr->left = NULL;
            expr->right = NULL;
            break;
        
        case NAME_EXPR:
            printf("%svar [%s] (scope [%s], type [%s])\n", tabs, expr->name, getScope(expr->scope), getType(expr->type));
            break;

        case ARG_EXPR:
            printExpr(expr->left, (indents + 1));
            if(expr->right){
                printExpr(expr->right, indents);
            }
            expr->left = NULL;
            expr->right = NULL;
            break;

        case ASSIGN_EXPR:
            printf("%s= (type [%s])\n", tabs, getType(expr->type));
            break;

        case SUBSCRIPT_EXPR:
            printf("%sarray_var [%s] (scope [%s], type [int])\n", tabs, expr->left->name, getScope(expr->scope));
            expr->left = NULL; // Don't print left child since we already did above
            break;

        case LT_EXPR:
            printf("%s< (type [%s])\n", tabs, getType(expr->type));
            break;

        case GT_EXPR:
            printf("%s> (type [%s])\n", tabs, getType(expr->type));
            break;

        case LEQ_EXPR:
            printf("%s<= (type [%s])\n", tabs, getType(expr->type));
            break;

        case GEQ_EXPR:
            printf("%s>= (type [%s])\n", tabs, getType(expr->type));
            break;

        case EQ_EXPR:
            printf("%s== (type [%s])\n", tabs, getType(expr->type));
            break;

        case NEQ_EXPR:
            printf("%s|= (type [%s])\n", tabs, getType(expr->type));
            break;
    }

    if(expr->left){

        printExpr(expr->left, indents + 1);
    }
    if(expr->right){

        printExpr(expr->right, indents + 1);
    }
}