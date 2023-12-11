#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>

const size_t OPERATION_COUNT = sizeof(OPERATIONS) / sizeof(*OPERATIONS);

typedef struct {
    uint8_t *payload;
    size_t size;
} allocation_t;

allocation_t allocations[ALLOCATION_IDS];

uint8_t rand_byte(void) {
    return rand();
}

void run_malloc(const trace_operation_t *operation) {
    // Perform the requested malloc
    assert(operation->type == MALLOC);
    size_t id = operation->id;
    size_t size = operation->size;
    uint8_t *payload = malloc(size);
    assert(payload != NULL);

    // Check that this allocation comes after all others
    for (size_t i = 0; i < id; i++) {
        allocation_t *allocation = &allocations[i];
        assert(allocation->payload + allocation->size < payload);
    }
    allocation_t *new_allocation = &allocations[id];
    assert(new_allocation->payload == NULL);
    new_allocation->payload = payload;
    new_allocation->size = size;

    #ifndef DISABLE_CONTENTS_CHECK
        // Fill the payload with (deterministic) random bytes
        srand(id);
        for (size_t i = 0; i < size; i++) {
            payload[i] = rand_byte();
        }
    #endif // #ifndef DISABLE_CONTENTS_CHECK
}

void run_free(const trace_operation_t *operation) {
    // Find the payload associated with the id
    assert(operation->type == FREE);
    size_t id = operation->id;
    allocation_t *allocation = &allocations[id];
    uint8_t *payload = allocation->payload;
    assert(payload != NULL);

    #ifndef DISABLE_CONTENTS_CHECK
        // Check that the payload has the correct (deterministic) random bytes
        size_t size = allocation->size;
        srand(id);
        for (size_t i = 0; i < size; i++) {
            assert(payload[i] == rand_byte());
        }
    #endif // #ifndef DISABLE_CONTENTS_CHECK

    // Perform the requested free
    free(payload);
}

int main() {
    for (size_t i = 0; i < OPERATION_COUNT; i++) {
        const trace_operation_t *operation = &OPERATIONS[i];
        if (operation->type == MALLOC) {
            run_malloc(operation);
        }
        else {
            run_free(operation);
        }
    }
}
