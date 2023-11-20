#include "jvm.h"

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "heap.h"
#include "read_class.h"

/** The name of the method to invoke to run the class file */
const char MAIN_METHOD[] = "main";
/**
 * The "descriptor" string for main(). The descriptor encodes main()'s signature,
 * i.e. main() takes a String[] and returns void.
 * If you're interested, the descriptor string is explained at
 * https://docs.oracle.com/javase/specs/jvms/se12/html/jvms-4.html#jvms-4.3.2.
 */
const char MAIN_DESCRIPTOR[] = "([Ljava/lang/String;)V";

/**
 * Represents the return value of a Java method: either void or an int or a reference.
 * For simplification, we represent a reference as an index into a heap-allocated array.
 * (In a real JVM, methods could also return object references or other primitives.)
 */
typedef struct {
    /** Whether this returned value is an int */
    bool has_value;
    /** The returned value (only valid if `has_value` is true) */
    int32_t value;
} optional_value_t;

/**
 * Runs a method's instructions until the method returns.
 *
 * @param method the method to run
 * @param locals the array of local variables, including the method parameters.
 *   Except for parameters, the locals are uninitialized.
 * @param class the class file the method belongs to
 * @param heap an array of heap-allocated pointers, useful for references
 * @return an optional int containing the method's return value
 */
optional_value_t execute(method_t *method, int32_t *locals, class_file_t *class,
                         heap_t *heap) {
    int32_t stack[(int) method->code.max_stack];
    memset(stack, 0, sizeof(stack));
    size_t stack_p = 0;
    optional_value_t result = {.has_value = false};

    for (size_t pc = 0; pc < (size_t) method->code.code_length; pc++) {
        uint8_t cur_method = (uint8_t) method->code.code[pc];
        switch (cur_method) {
            case i_iadd ... i_ixor:
                add_to_xor(cur_method, stack, &stack_p);
                break;
            case i_bipush:
                bi_push(method, stack, &stack_p, &pc);
                break;
            case i_getstatic:
                get_static(&pc);
                break;
            case i_invokevirtual:
                invoke_virtual(stack, &stack_p, &pc);
                break;
            case i_iconst_m1 ... i_iconst_5:
                i_const(method, stack, &stack_p, &pc);
                break;
            case i_sipush:
                si_push(method, stack, &stack_p, &pc);
                break;
            case i_iload:
                i_load(method, locals, stack, &stack_p, &pc);
                break;
            case i_istore:
                i_store(method, locals, stack, &stack_p, &pc);
                break;
            case i_iinc:
                i_inc(method, locals, &pc);
                break;
            case i_iload_0 ... i_iload_3:
                i_iload_nums(method, locals, stack, &stack_p, &pc);
                break;
            case i_istore_0 ... i_istore_3:
                i_istore_nums(method, locals, stack, &stack_p, &pc);
                break;
            case i_ldc:
                ldc(method, class, stack, &stack_p, &pc);
                break;
            case i_ifeq ... i_if_icmple:
                jump(cur_method, method, stack, &stack_p, &pc);
                break;
            case i_goto:
                go_to(method, &pc);
                break;
            case i_invokestatic: {
                int b1 = method->code.code[pc + 1];
                int b2 = method->code.code[pc + 2];
                uint16_t index = (b1 << 8) | b2;
                method_t *new_method = find_method_from_index(index, class);
                assert(new_method != false);
                uint16_t num_params = get_number_of_parameters(new_method);
                int32_t new_locals[new_method->code.max_locals];
                memset(new_locals, 0, sizeof(new_locals));
                for (size_t idx = 0; idx < num_params; idx++) {
                    *(new_locals + idx) = (int32_t) stack[stack_p - (num_params - idx)];
                    stack[stack_p - (num_params - idx)] = 0;
                }
                stack_p -= num_params;
                optional_value_t new_result =
                    execute(new_method, new_locals, class, heap);
                if (new_result.has_value) {
                    stack[stack_p] = new_result.value;
                    stack_p += 1;
                }
                pc += 2;
                break;
            }
            case i_nop:
                break;
            case i_dup:
                dup(stack, &stack_p);
                break;
            case i_newarray:
                newarray(stack, &stack_p, heap, &pc);
                break;
            case i_arraylength:
                arraylength(stack, &stack_p, heap);
                break;
            case i_iastore:
                iastore(stack, &stack_p, heap);
                break;
            case i_iaload:
                iaload(stack, &stack_p, heap);
                break;
            case i_aload:
                aload(method, stack, &stack_p, &pc, locals);
                break;
            case i_astore:
                astore(method, stack, &stack_p, &pc, locals);
                break;
            case i_aload_0 ... i_aload_3:
                aload_nums(method, stack, &stack_p, &pc, locals);
                break;
            case i_astore_0 ... i_astore_3:
                astore_nums(method, stack, &stack_p, &pc, locals);
                break;
            case i_areturn:
            case i_ireturn: {
                result =
                    (optional_value_t){.value = stack[stack_p - 1], .has_value = true};
                return result;
                break;
            }
            case i_return: {
                return result;
                break;
            }
            default:
                fprintf(stderr, "Unhandled value! %d\n", cur_method);
                exit(1);
                break;
        }
    }
    return result;
}

void add_to_xor(uint8_t cur_method, int *stack, size_t *stack_p) {
    if (cur_method == i_ineg) {
        stack[*stack_p - 1] = -(int) stack[*stack_p - 1];
        return;
    }
    int a = (int) stack[*stack_p - 2];
    int b = (int) stack[*stack_p - 1];
    int result = 0;
    switch (cur_method) {
        case i_iadd:
            result = a + b;
            break;
        case i_isub:
            result = a - b;
            break;
        case i_imul:
            result = a * b;
            break;
        case i_idiv: {
            if (b != 0) {
                result = a / b;
            }
            break;
        }
        case i_irem: {
            if (b != 0) {
                result = a % b;
                break;
            }
        }
        case i_ineg:
            result = -a;
            break;
        case i_ishl:
            result = a << b;
            break;
        case i_ishr:
            result = a >> b;
            break;
        case i_iushr:
            result = ((unsigned) a) >> b;
            break;
        case i_iand:
            result = a & b;
            break;
        case i_ior:
            result = a | b;
            break;
        case i_ixor:
            result = a ^ b;
            break;
        default:
            break;
    }
    stack[*stack_p - 2] = result;
    stack[*stack_p - 1] = 0;
    *stack_p -= 1;
}

void bi_push(method_t *method, int *stack, size_t *stack_p, size_t *pc) {
    stack[*stack_p] = (int) (signed char) method->code.code[*pc + 1];
    *stack_p += 1;
    *pc += 1;
}

void invoke_virtual(int *stack, size_t *stack_p, size_t *pc) {
    stack[*stack_p] = 0;
    *stack_p -= 1;
    printf("%d\n", stack[*stack_p]);
    *pc += 2;
}

void get_static(size_t *pc) {
    *pc += 2;
}

void i_const(method_t *method, int *stack, size_t *stack_p, const size_t *pc) {
    stack[*stack_p] = (int) (method->code.code[*pc] - i_iconst_0);
    *stack_p += 1;
}

void si_push(method_t *method, int *stack, size_t *stack_p, size_t *pc) {
    uint8_t num1 = method->code.code[*pc + 1];
    uint8_t num2 = method->code.code[*pc + 2];
    stack[*stack_p] = (signed short) ((num1 << 8) | num2);
    *stack_p += 1;
    *pc += 2;
}

void i_load(method_t *method, const int32_t *locals, int *stack, size_t *stack_p,
            size_t *pc) {
    unsigned char i = method->code.code[*pc + 1];
    stack[*stack_p] = locals[i];
    *stack_p += 1;
    *pc += 1;
}

void i_store(method_t *method, int32_t *locals, int *stack, size_t *stack_p, size_t *pc) {
    unsigned char i = method->code.code[*pc + 1];
    int a = stack[*stack_p - 1];
    locals[i] = a;
    stack[*stack_p - 1] = 0;
    *stack_p -= 1;
    *pc += 1;
}

void i_inc(method_t *method, int32_t *locals, size_t *pc) {
    unsigned char i = method->code.code[*pc + 1];
    signed char b = method->code.code[*pc + 2];
    locals[i] += b;
    *pc += 2;
}

void i_iload_nums(method_t *method, const int32_t *locals, int *stack, size_t *stack_p,
                  const size_t *pc) {
    int i = (int) method->code.code[*pc] - (int) i_iload_0;
    stack[*stack_p] = locals[i];
    *stack_p += 1;
}

void i_istore_nums(method_t *method, int32_t *locals, int *stack, size_t *stack_p,
                   const size_t *pc) {
    int i = (int) method->code.code[*pc] - (int) i_istore_0;
    int a = stack[*stack_p - 1];
    locals[i] = a;
    stack[*stack_p - 1] = 0;
    *stack_p -= 1;
}

void ldc(method_t *method, class_file_t *class, int *stack, size_t *stack_p, size_t *pc) {
    unsigned char i = method->code.code[*pc + 1];
    CONSTANT_Integer_info num =
        *(CONSTANT_Integer_info *) class->constant_pool[i - 1].info;
    stack[*stack_p] = num.bytes;
    *stack_p += 1;
    *pc += 1;
}

void jump(uint8_t cur_method, method_t *method, int *stack, size_t *stack_p, size_t *pc) {
    uint8_t b1 = method->code.code[*pc + 1];
    uint8_t b2 = method->code.code[*pc + 2];

    int a = stack[*stack_p - 1];
    int b = 0;
    if (cur_method >= i_if_icmpeq) {
        a = stack[*stack_p - 2];
        b = stack[*stack_p - 1];
        stack[*stack_p - 2] = 0;
        stack[*stack_p - 1] = 0;
        *stack_p -= 2;
    }
    else {
        stack[*stack_p - 1] = 0;
        *stack_p -= 1;
    }
    int16_t offset = (b1 << 8) | b2;
    bool add_offset = false;

    switch (cur_method) {
        case i_ifeq:
            add_offset = (a == 0);
            break;
        case i_ifne:
            add_offset = (a != 0);
            break;
        case i_iflt:
            add_offset = (a < 0);
            break;
        case i_ifge:
            add_offset = (a >= 0);
            break;
        case i_ifgt:
            add_offset = (a > 0);
            break;
        case i_ifle:
            add_offset = (a <= 0);
            break;
        case i_if_icmpeq:
            add_offset = (a == b);
            break;
        case i_if_icmpne:
            add_offset = (a != b);
            break;
        case i_if_icmplt:
            add_offset = (a < b);
            break;
        case i_if_icmpge:
            add_offset = (a >= b);
            break;
        case i_if_icmpgt:
            add_offset = (a > b);
            break;
        case i_if_icmple:
            add_offset = (a <= b);
            break;
        default:
            assert(false);
            break;
    }
    if (add_offset) {
        *pc += offset - 1;
    }
    else {
        *pc += 2;
    }
}

void go_to(method_t *method, size_t *pc) {
    uint8_t b1 = method->code.code[*pc + 1];
    uint8_t b2 = method->code.code[*pc + 2];
    int16_t offset = (b1 << 8) | b2;
    *pc += offset - 1;
}

void dup(int *stack, size_t *stack_p) {
    stack[*stack_p] = stack[*stack_p - 1];
    *stack_p += 1;
}

void newarray(int *stack, const size_t *stack_p, heap_t *heap, size_t *pc) {
    int32_t *arr = malloc(sizeof(int32_t) * (stack[*stack_p - 1] + 1));
    arr[0] = stack[*stack_p - 1];
    for (int i = 1; i < stack[*stack_p - 1] + 1; i++) {
        arr[i] = 0;
    }
    int32_t ref = heap_add(heap, arr);
    stack[*stack_p - 1] = ref;
    *pc += 1;
}

void arraylength(int *stack, const size_t *stack_p, heap_t *heap) {
    int32_t *arr = heap_get(heap, stack[*stack_p - 1]);
    int count = arr[0];
    stack[*stack_p - 1] = count;
}

void iastore(int *stack, size_t *stack_p, heap_t *heap) {
    int32_t *arr = heap_get(heap, stack[*stack_p - 3]);
    arr[stack[*stack_p - 2] + 1] = stack[*stack_p - 1];
    stack[*stack_p - 1] = 0;
    stack[*stack_p - 2] = 0;
    stack[*stack_p - 3] = 0;
    *stack_p -= 3;
}

void iaload(int *stack, size_t *stack_p, heap_t *heap) {
    int32_t *arr = heap_get(heap, stack[*stack_p - 2]);
    int value = arr[stack[*stack_p - 1] + 1];
    stack[*stack_p - 1] = 0;
    stack[*stack_p - 2] = 0;
    *stack_p -= 2;
    stack[*stack_p] = value;
    *stack_p += 1;
}

void aload(method_t *method, int *stack, size_t *stack_p, size_t *pc,
           const int32_t *locals) {
    uint8_t idx = method->code.code[*pc + 1];
    stack[*stack_p] = locals[idx];
    *stack_p += 1;
    *pc += 1;
}

void astore(method_t *method, int *stack, size_t *stack_p, size_t *pc, int32_t *locals) {
    uint8_t idx = method->code.code[*pc + 1];
    locals[idx] = stack[*stack_p - 1];
    stack[*stack_p - 1] = 0;
    *stack_p -= 1;
    *pc += 1;
}

void astore_nums(method_t *method, int *stack, size_t *stack_p, const size_t *pc,
                 int32_t *locals) {
    uint8_t idx = method->code.code[*pc] - i_astore_0;
    locals[idx] = stack[*stack_p - 1];
    stack[*stack_p - 1] = 0;
    *stack_p -= 1;
}

void aload_nums(method_t *method, int *stack, size_t *stack_p, const size_t *pc,
                const int32_t *locals) {
    uint8_t idx = method->code.code[*pc] - i_aload_0;
    stack[*stack_p] = locals[idx];
    *stack_p += 1;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "USAGE: %s <class file>\n", argv[0]);
        return 1;
    }

    // Open the class file for reading
    FILE *class_file = fopen(argv[1], "r");
    assert(class_file != NULL && "Failed to open file");

    // Parse the class file
    class_file_t *class = get_class(class_file);
    int error = fclose(class_file);
    assert(error == 0 && "Failed to close file");

    // The heap array is initially allocated to hold zero elements.
    heap_t *heap = heap_init();

    // Execute the main method
    method_t *main_method = find_method(MAIN_METHOD, MAIN_DESCRIPTOR, class);
    assert(main_method != NULL && "Missing main() method");
    /* In a real JVM, locals[0] would contain a reference to String[] args.
     * But since TeenyJVM doesn't support Objects, we leave it uninitialized. */
    int32_t locals[main_method->code.max_locals];
    // Initialize all local variables to 0
    memset(locals, 0, sizeof(locals));
    optional_value_t result = execute(main_method, locals, class, heap);
    assert(!result.has_value && "main() should return void");

    // Free the internal data structures
    free_class(class);

    // Free the heap
    heap_free(heap);
}
