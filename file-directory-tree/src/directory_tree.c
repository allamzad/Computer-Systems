#include "directory_tree.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

const mode_t MODE = 0777;
const size_t SPACES_PER_LEVEL = 4;

void init_node(node_t *node, char *name, node_type_t type) {
    if (name == NULL) {
        name = strdup("ROOT");
        assert(name != NULL);
    }
    node->name = name;
    node->type = type;
}

file_node_t *init_file_node(char *name, size_t size, uint8_t *contents) {
    file_node_t *node = malloc(sizeof(file_node_t));
    assert(node != NULL);
    init_node((node_t *) node, name, FILE_TYPE);
    node->size = size;
    node->contents = contents;
    return node;
}

directory_node_t *init_directory_node(char *name) {
    directory_node_t *node = malloc(sizeof(directory_node_t));
    assert(node != NULL);
    init_node((node_t *) node, name, DIRECTORY_TYPE);
    node->num_children = 0;
    node->children = NULL;
    return node;
}

void sorted_dir(directory_node_t *dnode) {
    for (size_t i = 0; i < dnode->num_children; i++) {
        char *min_string = dnode->children[i]->name;
        size_t min_idx = i;
        for (size_t j = i + 1; j < dnode->num_children; j++) {
            node_t *cur_node = (node_t *) dnode->children[j];
            char *str = cur_node->name;
            if (strcmp(str, min_string) < 0) {
                min_string = str;
                min_idx = j;
            }
        }

        node_t *temp_node = dnode->children[i];
        dnode->children[i] = dnode->children[min_idx];
        dnode->children[min_idx] = temp_node;
    }
}

void add_child_directory_tree(directory_node_t *dnode, node_t *child) {
    dnode->num_children = dnode->num_children + 1;
    dnode->children =
        (node_t **) realloc(dnode->children, (dnode->num_children) * sizeof(node_t *));
    assert(dnode->children != NULL);
    dnode->children[dnode->num_children - 1] = child;
    sorted_dir(dnode);
}

void print_directory_tree_helper(node_t *node, size_t level) {
    if (node->type == FILE_TYPE) {
        for (size_t i = 0; i < SPACES_PER_LEVEL * level; i++) {
            printf(" ");
        }
        printf("%s\n", node->name);
    }
    else {
        directory_node_t *dir_node = (directory_node_t *) node;
        for (size_t i = 0; i < SPACES_PER_LEVEL * level; i++) {
            printf(" ");
        }
        printf("%s\n", node->name);
        for (size_t i = 0; i < dir_node->num_children; i++) {
            print_directory_tree_helper(dir_node->children[i], level + 1);
        }
    }
}

void print_directory_tree(node_t *node) {
    print_directory_tree_helper(node, 0);
}

void create_directory_tree_helper(node_t *node, char *path) {
    char new_path[(strlen(path) + strlen(node->name) + 2) * sizeof(char)];
    strcpy(new_path, path);
    if (strlen(new_path) != 0) {
        strcat(new_path, "/");
    }
    strcat(new_path, node->name);
    if (node->type == FILE_TYPE) {
        file_node_t *file_node = (file_node_t *) node;
        FILE *file = fopen(new_path, "w");
        assert(file != NULL);
        size_t written = fwrite(file_node->contents, 1, file_node->size, file);
        assert(written == file_node->size);
        size_t closed = fclose(file);
        assert(closed == 0);
    }
    else {
        directory_node_t *directory_node = (directory_node_t *) node;
        assert(mkdir(new_path, MODE) == 0);
        for (size_t i = 0; i < directory_node->num_children; i++) {
            create_directory_tree_helper(directory_node->children[i], new_path);
        }
    }
}

void create_directory_tree(node_t *node) {
    char *path = "";
    create_directory_tree_helper(node, path);
}

void free_directory_tree(node_t *node) {
    if (node->type == FILE_TYPE) {
        file_node_t *fnode = (file_node_t *) node;
        free(fnode->contents);
    }
    else {
        assert(node->type == DIRECTORY_TYPE);
        directory_node_t *dnode = (directory_node_t *) node;
        for (size_t i = 0; i < dnode->num_children; i++) {
            free_directory_tree(dnode->children[i]);
        }
        free(dnode->children);
    }
    free(node->name);
    free(node);
}
