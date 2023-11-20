#include "compile.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "ast.h"

const size_t A_ASCII = 65;
int if_local = 0;
int while_local = 0;
bool fold = false;

bool compile_ast(node_t *node) {
    if (node->type == NUM) {
        printf("\tmovq $%ld, %%rdi\n", ((num_node_t *) node)->value);
    }
    else if (node->type == PRINT) {
        node_t *expression = ((print_node_t *) node)->expr;
        if (expression->type == BINARY_OP) {
            compute_folds(&expression);
        }
        compile_ast(expression);
        printf("\tcall print_int\n");
    }
    else if (node->type == SEQUENCE) {
        sequence_node_t *sequence_info = ((sequence_node_t *) node);
        size_t sequence_len = sequence_info->statement_count;
        node_t **statements = sequence_info->statements;
        for (size_t i = 0; i < sequence_len; i++) {
            compile_ast(statements[i]);
        }
    }
    else if (node->type == BINARY_OP) {
        binary_node_t *binary = ((binary_node_t *) node);
        bool power = false;

        if (binary->right->type == NUM && binary->op == '*') {
            power = check_power_two(binary->left, binary->right);
        }
        if (!power) {
            compile_ast(binary->left);
            printf("\tpush %%rdi\n");
            compile_ast(binary->right);
            printf("\tpush %%rdi\n");

            printf("\tpop %%rcx\n");
            printf("\tpop %%rdi\n");

            if (binary->op == '<' || binary->op == '>' || binary->op == '=') {
                printf("\tcmp %%rcx, %%rdi\n");
            }
            else if (binary->op == '+') {
                printf("\taddq %%rcx, %%rdi\n");
            }
            else if (binary->op == '-') {
                printf("\tsubq %%rcx, %%rdi\n");
            }
            else if (binary->op == '*') {
                printf("\timulq %%rcx, %%rdi\n");
            }
            else if (binary->op == '/') {
                printf("\tmovq %%rdi, %%rax\n");
                printf("\tcqto\n");
                printf("\tidiv %%rcx\n");
                printf("\tmovq %%rax, %%rdi\n");
            }
        }
    }
    else if (node->type == VAR) {
        var_node_t *var_info = ((var_node_t *) node);
        printf("\tmovq -0x%x(%%rbp), %%rdi\n",
               (unsigned int) ((var_info->name - A_ASCII + 1) * sizeof(value_t)));
    }
    else if (node->type == LET) {
        let_node_t *let_info = ((let_node_t *) node);
        node_t *let_value = let_info->value;
        // if (let_value->type == BINARY_OP){
        //     compute_folds(&let_value);
        // }
        compile_ast(let_value);
        printf("\tmovq %%rdi, -0x%x(%%rbp)\n",
               (unsigned int) ((let_info->var - A_ASCII + 1) * sizeof(value_t)));
    }
    else if (node->type == IF) {
        if_local += 1;
        if_node_t *if_info = ((if_node_t *) node);
        node_t *if_condition = (node_t *) if_info->condition;
        if (if_condition->type == BINARY_OP) {
            compute_folds(&if_condition);
        }
        compile_ast(if_condition);

        if (if_info->condition->op == '<') {
            printf("\tjl .IF_%d\n", if_local);
            if (if_info->else_branch != NULL) {
                printf("\tjge .ELSE_%d\n", if_local);
            }
            else {
                printf("\tjge .END_IF_%d\n", if_local);
            }
        }
        else if (if_info->condition->op == '>') {
            printf("\tjg .IF_%d\n", if_local);
            if (if_info->else_branch != NULL) {
                printf("\tjle .ELSE_%d\n", if_local);
            }
            else {
                printf("\tjle .END_IF_%d\n", if_local);
            }
        }
        else if (if_info->condition->op == '=') {
            printf("\tje .IF_%d\n", if_local);
            if (if_info->else_branch != NULL) {
                printf("\tjne .ELSE_%d\n", if_local);
            }
            else {
                printf("\tjne .END_IF_%d\n", if_local);
            }
        }

        int cur_local = if_local;
        printf("\t.IF_%d:\n", if_local);
        compile_ast(if_info->if_branch);
        printf("\tjmp .END_IF_%d\n", cur_local);
        printf("\t.ELSE_%d:\n", cur_local);
        if (if_info->else_branch != NULL) {
            compile_ast(if_info->else_branch);
        }
        printf("\t.END_IF_%d:\n", cur_local);
    }
    else if (node->type == WHILE) {
        while_local += 1;
        while_node_t *while_info = ((while_node_t *) node);
        node_t *while_condition = (node_t *) while_info->condition;
        if (while_condition->type == BINARY_OP) {
            compute_folds(&while_condition);
        }
        printf("\t.START_WHILE_%d:\n", while_local);
        compile_ast(while_condition);

        if (while_info->condition->op == '<') {
            printf("\tjl .WHILE_%d\n", while_local);
            printf("\tjge .WHILE_END_%d\n", while_local);
        }
        else if (while_info->condition->op == '>') {
            printf("\tjg .WHILE_%d\n", while_local);
            printf("\tjle .WHILE_END_%d\n", while_local);
        }
        else if (while_info->condition->op == '=') {
            printf("\tje .WHILE_%d\n", while_local);
            printf("\tjne .WHILE_END_%d\n", while_local);
        }

        int cur_local = while_local;
        printf("\t.WHILE_%d:\n", cur_local);
        compile_ast(while_info->body);
        printf("\tjmp .START_WHILE_%d\n", cur_local);
        printf("\t.WHILE_END_%d:\n", cur_local);
    }
    return true;
}

void compute_folds(node_t **node) {
    if ((*node)->type == BINARY_OP) {
        node_t *left = ((binary_node_t *) (*node))->left;
        node_t *right = ((binary_node_t *) (*node))->right;
        char bi_op = ((binary_node_t *) (*node))->op;

        if (left->type == BINARY_OP) {
            compute_folds(&left);
        }

        if (right->type == BINARY_OP) {
            compute_folds(&right);
        }

        if (left->type == NUM && right->type == NUM &&
            (bi_op == '+' || bi_op == '-' || bi_op == '*' || bi_op == '/')) {
            value_t left_val = ((num_node_t *) left)->value;
            value_t right_val = ((num_node_t *) right)->value;

            if (bi_op == '+') {
                left_val += right_val;
            }
            else if (bi_op == '-') {
                left_val -= right_val;
            }
            else if (bi_op == '*') {
                left_val *= right_val;
            }
            else if (bi_op == '/') {
                left_val /= right_val;
            }
            ((num_node_t *) left)->value = left_val;
            *node = left;
            // free_ast(left);
            // free_ast(right);
        }
    }
}

bool check_power_two(node_t *left, node_t *right) {
    int shifted_right = 0;
    if ((((num_node_t *) right)->value > 1) &&
        ((((num_node_t *) right)->value) & (((num_node_t *) right)->value) - 1) == 0) {
        shifted_right = log2(((num_node_t *) right)->value);
        compile_ast(left);
        printf("\tshlq $%d, %%rdi\n", shifted_right);
        return true;
    }
    return false;
}
