#include <stddef.h>

typedef enum {
    MALLOC,
    FREE
} trace_operation_type_t;

typedef struct {
    trace_operation_type_t type;
    size_t id;
    size_t size; // valid iff MALLOC
} trace_operation_t;
