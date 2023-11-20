#include "recover.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "directory_tree.h"
#include "fat16.h"

const size_t MASTER_BOOT_RECORD_SIZE = 0x20B;

void follow(FILE *disk, directory_node_t *node, bios_parameter_block_t bpb) {
    directory_entry_t entry;
    size_t read = fread(&entry, sizeof(directory_entry_t), 1, disk);
    assert(read == 1);
    char *file_name = get_file_name(entry);

    while (file_name[0] != '\0') {
        if (!is_hidden(entry)) {
            if (is_directory(entry)) {
                follow_directory(disk, node, bpb, entry);
            }
            else {
                follow_file(disk, node, bpb, entry);
            }
        }
        size_t read = fread(&entry, sizeof(directory_entry_t), 1, disk);
        assert(read == 1);
        free(file_name);
        file_name = get_file_name(entry);
    }
    free(file_name);
}

void follow_directory(FILE *disk, directory_node_t *node, bios_parameter_block_t bpb,
                      directory_entry_t entry) {
    size_t prev_offset = ftell(disk);
    size_t offset = get_offset_from_cluster(entry.first_cluster, bpb);
    fseek(disk, offset, SEEK_SET);
    directory_node_t *dir_node = init_directory_node(get_file_name(entry));
    add_child_directory_tree(node, (node_t *) dir_node);
    follow(disk, dir_node, bpb);
    fseek(disk, prev_offset, SEEK_SET);
}

void follow_file(FILE *disk, directory_node_t *node, bios_parameter_block_t bpb,
                 directory_entry_t entry) {
    char *file_entry = malloc(entry.file_size);
    size_t prev_offset = ftell(disk);
    size_t offset = get_offset_from_cluster(entry.first_cluster, bpb);
    fseek(disk, offset, SEEK_SET);
    size_t read = fread(file_entry, entry.file_size, 1, disk);
    assert(read == 1);
    file_node_t *file_node =
        init_file_node(get_file_name(entry), entry.file_size, (uint8_t *) file_entry);
    add_child_directory_tree(node, (node_t *) file_node);
    fseek(disk, prev_offset, SEEK_SET);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "USAGE: %s <image filename>\n", argv[0]);
        return 1;
    }

    FILE *disk = fopen(argv[1], "r");
    if (disk == NULL) {
        fprintf(stderr, "No such image file: %s\n", argv[1]);
        return 1;
    }

    bios_parameter_block_t bpb;
    fseek(disk, MASTER_BOOT_RECORD_SIZE, SEEK_SET);
    size_t read = fread(&bpb, sizeof(bios_parameter_block_t), 1, disk);
    assert(read == 1);
    fseek(disk, get_root_directory_location(bpb), SEEK_SET);

    directory_node_t *root = init_directory_node(NULL);
    follow(disk, root, bpb);
    print_directory_tree((node_t *) root);
    create_directory_tree((node_t *) root);
    free_directory_tree((node_t *) root);

    int result = fclose(disk);
    assert(result == 0);
}
