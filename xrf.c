#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

const unsigned int COMMANDS_PER_CHUNK = 5;

/* A struct for each node of the stack */
struct Stack {
    unsigned int val; /* The value held by this node */
    struct Stack *next; /* Pointer to the next node */
} *stack, *bottom;

/* A struct for keeping track of the read-in XRF code */
struct Code {
    char *commands; /* Array of all the commands */
    bool *visited; /* Array of whether each chunk has been visited yet */
    int len; /* How many commands there are */
} code;

/* Returns a pointer to a newly-malloc'd node with a value of val */
struct Stack *new_stack_node(int val) {
    struct Stack *node = malloc(sizeof(struct Stack));
    node->next = NULL;
    node->val = val;
    return node;
}

/* Frees the stack */
void free_stack() {
    struct Stack *node;
    while (stack != NULL) {
        node = stack;
        stack = stack->next;
        free(node);
    }
}

/* Pushes a new node onto the stack */
void push_stack(int val) {
    struct Stack *node = new_stack_node(val);
    node->next = stack;
    stack = node;
    /* If the stack was formerly empty, we need to update the pointer to the
       bottom to reflect that there's actually something in the stack */
    if (bottom == NULL)
        bottom = stack;
}

/* Pops the stack, and returns the popped value */
int pop_stack() {
    struct Stack *node;
    int val;

    if (stack == NULL) {
        fprintf(stderr, "Error! Unable to pop empty stack!\n");
        exit(1);
    }

    node = stack;
    val = stack->val;
    stack = stack->next;
    free(node);

    /* If the stack is empty, update the bottom pointer so it isn't pointing
       to anything */
    if (stack == NULL)
        bottom = stack;
    return val;
}

/* Swaps the top two elements of the stack */
void swap_stack() {
    int temp;

    if (stack == NULL || stack->next == NULL) {
        fprintf(stderr, "Error! Can't swap the top two elements on a%s"
                        " stack\n", stack ? " one-element" : "n empty");
        exit(1);
    }

    temp = stack->val;
    stack->val = stack->next->val;
    stack->next->val = temp;
}

/* Duplicates the top element of the stack */
void dup_stack() {
    if (stack == NULL) {
        fprintf(stderr, "Error! Nothing on the stack to be duplicated!\n");
        exit(1);
    }
    push_stack(stack->val);
}

/* Sends the top node of the stack to the bottom of the stack */
void send_top_to_bottom() {
    struct Stack *node;
    if (stack == NULL) {
        fprintf(stderr, "Error! Can't send nonexistent value to the bottom of"
                        " the stack!\n");
        exit(1);
    }
    if (stack->next == NULL)
        return;
    node = stack;
    stack = stack->next;
    bottom->next = node;
    bottom = bottom->next;
    bottom->next = NULL;
}

/* Randomizes the order of the stack */
void randomize_stack() {
    unsigned i, *vals, num_vals, num_allocated;
    struct Stack *node;

    vals = malloc(sizeof(int));
    num_allocated = 1;
    num_vals = 0;

    /* Goes through the stack and stores the values in an array */
    for (node = stack; node != NULL; node = node->next) {
        if (num_vals == num_allocated) {
            unsigned *new_vals;
            num_allocated <<= 1;
            new_vals = realloc(vals, num_allocated * sizeof(int));
            if (vals == NULL) {
                fprintf(stderr, "Error! Unable to allocate additional"
                                " space\n!");
                free(vals);
                exit(1);
            }
            vals = new_vals;
        }
        vals[num_vals++] = node->val;
    }

    /* Shuffles the array of values */
    for (i = num_vals - 1; i > 0; i--) {
        unsigned swap_index = rand() % (i + 1);
        int temp = vals[swap_index];
        vals[swap_index] = vals[i];
        vals[i] = temp;
    }

    /* Goes through the list, assigning to each node a shuffled value */
    for (i = 0, node = stack; node != NULL; node = node->next, i++) {
        node->val = vals[i];
    }

    free(vals);
}

/* Frees the stored XRF code */
void free_xrf_code() {
    free(code.commands);
    free(code.visited);
}

/* Reads a given XRF file */
void read_xrf_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    int cur_cmd;
    char c;

    if (file == NULL) {
        fprintf(stderr, "Error! Unable to open %s!\n", filename);
        exit(1);
    }

    code.commands = NULL;
    code.visited = NULL;
    cur_cmd = 0;
    code.len = 0;
    atexit(free_xrf_code);

    while ((c = fgetc(file)) != EOF) {
        if (isspace(c)) {
            continue;
        }
        else if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F')) {
            if (cur_cmd % COMMANDS_PER_CHUNK == 0) {
                char *new_commands;
                bool *new_visited;

                code.len += COMMANDS_PER_CHUNK;
                new_commands = realloc(code.commands, code.len);
                new_visited = realloc(code.visited, code.len / 5);
                if (new_commands == NULL || new_visited == NULL) {
                    fprintf(stderr, "Error! Unable to allocate additional "
                                    "space for the code!\n");
                    fclose(file);
                    free(new_commands);
                    free(new_visited);
                    exit(1);
                }
                code.commands = new_commands;
                code.visited = new_visited;
                code.visited[(code.len / COMMANDS_PER_CHUNK) - 1] = false;
            }
            code.commands[cur_cmd++] = c;
        }
        else {
            fprintf(stderr, "Error! Unknown character %c encountered!\n", c);
            fclose(file);
            exit(1);
        }
    }

    fclose(file);

    if (cur_cmd % COMMANDS_PER_CHUNK != 0) {
        fprintf(stderr, "Error! Inadequate code length!\n");
        exit(1);
    }
}

/* Executes a chunk of code */
void execute_chunk(const char *chunk, bool visited) {
    struct Stack *temp;
    unsigned i, temp_val;

    for (i = 0; i < COMMANDS_PER_CHUNK; i++) {
        switch (chunk[i]) {
            case '0':
                temp_val = getchar();
                if (temp_val == (unsigned) EOF)
                    push_stack(0);
                else
                    push_stack(temp_val);
                break;
            case '1':
                if (stack == NULL) {
                    fprintf(stderr, "Error! Cannot output nonexistent"
                                    " value!\n");
                    exit(1);
                }
                putchar(pop_stack());
                break;
            case '2':
                pop_stack();
                break;
            case '3':
                dup_stack();
                break;
            case '4':
                swap_stack();
                break;
            case '5':
                if (stack == NULL) {
                    fprintf(stderr, "Error! Cannot increment nonexistent "
                                    "value!\n");
                    exit(1);
                }
                stack->val += 1;
                break;
            case '6':
                if (stack == NULL) {
                    fprintf(stderr, "Error! Cannot decrement nonexistent "
                                    "value!\n");
                    exit(1);
                }
                if (stack->val > 0)
                    stack->val -= 1;
                break;
            case '7':
                if (stack == NULL || stack->next == NULL) {
                    fprintf(stderr, "Error! Cannot add the top values of a%s\n",
                            stack ? "one-value stack.": "n empty stack.");
                    exit(1);
                }
                stack->next->val += stack->val;
                temp = stack;
                stack = stack->next;
                free(temp);
                break;
            case '8':
                if (!visited) i++;
                break;
            case '9':
                send_top_to_bottom();
                break;
            case 'A':
                return;
            case 'B':
                exit(0);
            case 'C':
                if (visited) i++;
                break;
            case 'D':
                randomize_stack();
                break;
            case 'E':
                if (stack == NULL || stack->next == NULL) {
                    fprintf(stderr, "Error! Cannot get the difference of the"
                                    " top two values of a%s!\n",
                            stack ? " one-value stack": "n empty stack");
                    exit(1);
                }
                temp_val = pop_stack();
                if (temp_val <= stack->val)
                    stack->val -= temp_val;
                else
                    stack->val = temp_val - stack->val;
                break;
        }
    }
}

/* Executes the stored XRF code */
void execute_code() {
    unsigned cur_chunk = 0;

    stack = new_stack_node(0);
    bottom = stack;
    atexit(free_stack);

    while (true) {
        execute_chunk(code.commands + (cur_chunk * COMMANDS_PER_CHUNK),
                      code.visited[cur_chunk]);

        code.visited[cur_chunk] = true;
        if (stack == NULL) {
            fprintf(stderr, "Error! Can't have an empty stack upon reaching "
                            "the end of a chunk!\n");
            exit(1);
        }
        cur_chunk = stack->val;
        if (cur_chunk >= code.len / COMMANDS_PER_CHUNK) {
            fprintf(stderr, "Error! Cannot go to nonexistent chunk %u!\n",
                    stack->val);
            exit(1);
        }
    }
}

int main(int argc, char **argv) {
    const char *filename = NULL;
    int i;

    for (i = 1; i < argc; i++) {
        if (*argv[i] != '-') {
            if (filename != NULL) {
                fprintf(stderr, "Only one file at a time please!\n");
                exit(1);
            }
            filename = argv[i];
        }
    }

    if (filename == NULL) {
        fprintf(stderr, "Error! No filename given!\n");
        exit(1);
    }

    read_xrf_file(filename);

    srand(time(NULL));

    execute_code();

    return 0;
}
