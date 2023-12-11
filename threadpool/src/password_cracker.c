#define _GNU_SOURCE
#include <assert.h>
#include <crypt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dictionary_words.h"
#include "thread_pool.h"

const char HASH_START[] = "$6$";
const size_t SALT_LENGTH = 20;
const size_t HASH_LENGTH = 106;
const size_t NUM_THREADS = 16;

static size_t hash_count = 0;
static char **hashes = NULL;

typedef struct hashpass {
    const char *password;
    const char *hash;
} hashpass_t;

static inline bool hashes_match(void *aux) {
    hashpass_t *hashpass = (hashpass_t *) aux;
    const char *password = hashpass->password;
    const char *hash = hashpass->hash;
    char salt[SALT_LENGTH + 1];
    memcpy(salt, hash, sizeof(char[SALT_LENGTH]));
    salt[SALT_LENGTH] = '\0';
    struct crypt_data data;
    memset(&data, 0, sizeof(data));
    char *hashed = crypt_r(password, salt, &data);
    char *hashed_hash = &hashed[SALT_LENGTH];
    const char *hash_hash = &hash[SALT_LENGTH];
    bool hashes_match =
        memcmp(hashed_hash, hash_hash, sizeof(char[HASH_LENGTH - SALT_LENGTH])) == 0;
    if (hashes_match) {
        printf("%s\n", password);
    }
    return hashes_match;
}

char *form_word(int num_pos, char *num, const char *word, size_t word_length) {
    bool num_added = false;
    char *password = malloc(sizeof(char) * (word_length + 1));
    if (num_pos == -1) {
        strcat(password, num);
        num_added = true;
    }
    for (int i = 0; i < (int) word_length; i++) {
        if (num_pos < i && !num_added) {
            strcat(password, num);
            num_added = true;
        }
        char letter[2];
        letter[0] = word[i];
        letter[1] = '\0';
        char *dict_letter = letter;
        strcat(password, dict_letter);
    }
    if (!num_added) {
        strcat(password, num);
    }
    return password;
}

int main(void) {
    // Read in the hashes from the standard input
    char *line = NULL;
    size_t line_capacity = 0;
    // Stop when the end of the input or an empty line is reached
    while (getline(&line, &line_capacity, stdin) > 0 && line[0] != '\n') {
        // Check that the line looks like a hash
        size_t line_length = strlen(line);
        assert(line_length == HASH_LENGTH ||
               (line_length == HASH_LENGTH + 1 && line[HASH_LENGTH] == '\n'));
        assert(memcmp(line, HASH_START, sizeof(HASH_START) - sizeof(char)) == 0);

        // Extend the hashes array and add the hash to it
        hashes = realloc(hashes, sizeof(char * [hash_count + 1]));
        assert(hashes != NULL);
        char *hash = malloc(sizeof(char[HASH_LENGTH + 1]));
        assert(hash != NULL);
        memcpy(hash, line, sizeof(char[HASH_LENGTH]));
        hash[HASH_LENGTH] = '\0';
        hashes[hash_count++] = hash;
    }
    free(line);

    // TODO (student): Use your threadpool to recover the passwords from the hashes.
    //                 You may assume the provided dictionary.h includes all the
    //                 possible root words.

    char *password = "";
    thread_pool_t *pool = thread_pool_init(NUM_THREADS);
    for (size_t i = 0; i < NUM_DICTIONARY_WORDS; i++) {
        // for (size_t i = 0; i < 1; i++) {
        const char *dict_word = DICTIONARY[i];
        // const char *dict_word = "quests";
        size_t word_length = strlen(dict_word);
        for (size_t j = 48; j < 58; j++) {
            char ascii = (char) j;
            char number[2];
            number[0] = ascii;
            number[1] = '\0';
            char *num = number;
            for (int num_pos = -1; num_pos < (int) word_length; num_pos++) {
                password = form_word(num_pos, num, dict_word, word_length);
                for (size_t k = 0; k < hash_count; k++) {
                    hashpass_t *hashpass = malloc(sizeof(hashpass_t));
                    assert(hashpass != NULL);
                    hashpass->password = password;
                    hashpass->hash = hashes[k];
                    thread_pool_add_work(pool, (work_function_t) hashes_match, hashpass);
                }
            }
        }
    }
    thread_pool_finish(pool);
}
