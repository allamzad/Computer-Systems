#ifndef RECOVER_H
#define RECOVER_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "directory_tree.h"
#include "fat16.h"
#include "recover.h"

/**
 * Read the directory entries, determines if it's a file or directory and constructs the
 * nodes of the directory tree accordingly until a directory entry that starts with '\0'
 * is encountered.
 *
 * @param disk the file where the directory entries are being read from
 * @param node the current parent folder of the directory tree
 * @param bpb the BIOS parameter block
 *
 */
void follow(FILE *disk, directory_node_t *node, bios_parameter_block_t bpb);

/**
 * Goes to the first cluster offset for the directory and adds the file or folder to the
 * parent node. Then returns to the previous offset.
 *
 * @param disk the file where the directory entries are being read from
 * @param node the current parent folder of the directory tree
 * @param bpb the BIOS parameter block
 * @param entry the directory entry we're currently on in the disk
 *
 */
void follow_directory(FILE *disk, directory_node_t *node, bios_parameter_block_t bpb,
                      directory_entry_t entry);

/**
 * Goes to the first cluster offset for the file, reads in the file's contents, creates
 * the file and adds it to the parent node, then returns to the previous offset.
 *
 * @param disk the file where the directory entries are being read from
 * @param node the current parent folder of the directory tree
 * @param bpb the BIOS parameter block
 * @param entry the directory entry we're currently on in the disk
 *
 */
void follow_file(FILE *disk, directory_node_t *node, bios_parameter_block_t bpb,
                 directory_entry_t entry);

#endif /* RECOVER_H */