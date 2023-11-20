#ifndef COMPILE_H
#define COMPILE_H

#include <stdbool.h>

#include "ast.h"

/**
 * Prints x86-64 assembly code that implements the given TeenyBASIC AST.
 *
 * @param node the statement to compile.
 *   The root node will be a SEQUENCE, PRINT, LET, IF, or WHILE.
 * @return true iff compilation succeeds
 */
bool compile_ast(node_t *node);
void compute_recursion(node_t *node);
bool check_power_two(node_t *left, node_t *right);
void compute_folds(node_t **node);
#endif /* COMPILE_H */
