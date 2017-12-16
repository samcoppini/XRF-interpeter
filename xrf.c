#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

const unsigned int COMMANDS_PER_CHUNK = 5;

/* A struct for each node of the stack */
struct Stack {
    unsigned int val; /* The value held by this node */
    struct Stack *next, *prev; /* Pointer to the next and previous node */
} *top, *bottom;

int stack_size = 1;

/* A struct for keeping track of the read-in XRF code */
struct Code {
    char *commands; /* Array of all the commands */
    bool *visited; /* Array of whether each chunk has been visited yet */
    int len; /* How many commands there are */
} code;

/* Returns a pointer to a newly-malloc'd node with a value of val */
struct Stack *new_stack_node(int val) {
    struct Stack *node = malloc(sizeof(struct Stack));
    if (node == NULL) {
        fprintf(stderr, "Error! Unable to allocate additional stack space!\n");
        exit(1);
    }
    node->next = NULL;
    node->prev = NULL;
    node->val = val;
    return node;
}

/* Frees the stack */
void free_stack() {
    struct Stack *temp;
    while (bottom != NULL) {
        temp = bottom;
        bottom = bottom->prev;
        free(temp);
    }
}

/* Pushes a new node onto the stack */
void push_stack(int val) {
    if (top->prev != NULL) {
        /* If we've already allocated a node, we just make the
           already-allocated node the new top */
        top = top->prev;
        top->val = val;
    } else {
        struct Stack *node = new_stack_node(val);
        top->prev = node;
        node->next = top;
        top = node;
    }
    stack_size++;
}

/* Pops the stack, and returns the popped value */
int pop_stack() {
    if (stack_size == 0) {
        fprintf(stderr, "Error! Can't pop an empty stack!\n");
        exit(1);
    } else {
        int to_return = top->val;
        top = top->next;
        stack_size--;
        return to_return;
    }
}

/* Swaps the top two elements of the stack */
void swap_stack() {
    int temp;

    if (stack_size < 2) {
        fprintf(stderr, "Error! Can't swap the top two elements on a%s stack\n",
                        stack_size == 1 ? " one-element" : "n empty");
        exit(1);
    }

    temp = top->val;
    top->val = top->next->val;
    top->next->val = temp;
}

/* Duplicates the top element of the stack */
void dup_stack() {
    if (stack_size == 0) {
        fprintf(stderr, "Error! Nothing on the stack to be duplicated!\n");
        exit(1);
    }
    push_stack(top->val);
}

/* Sends the top node of the stack to the bottom of the stack */
void send_top_to_bottom() {
    if (stack_size == 0) {
        fprintf(stderr, "Error! Can't send nonexistent value to the bottom of"
                        " the stack!\n");
        exit(1);
    } else if (stack_size == 1) {
        return;
    } else {
        struct Stack *temp = top;
        top = top->next;
        top->prev = temp->prev;
        if (temp->prev != NULL) {
            temp->prev->next = top;
        }
        bottom->next = temp;
        temp->prev = bottom;
        bottom = temp;
        temp->next = NULL;
    }
}

/* Randomizes the order of the stack */
void randomize_stack() {
    unsigned i, *vals;
    struct Stack *node;

    if (stack_size == 0) {
        return;
    }

    vals = malloc(sizeof(int) * stack_size);
    if (vals == NULL) {
        fprintf(stderr, "Error! Unable to allocate additional"
                        " space\n!");
    }

    /* Goes through the stack and stores the values in an array */
    for (i = 0, node = top; node != NULL; node = node->next, i++) {
        vals[i] = node->val;
    }

    /* Shuffles the array of values */
    for (i = stack_size - 1; i > 0; i--) {
        unsigned swap_index = rand() % (i + 1);
        int temp = vals[swap_index];
        vals[swap_index] = vals[i];
        vals[i] = temp;
    }

    /* Goes through the list, assigning to each node a shuffled value */
    for (i = 0, node = top; node != NULL; node = node->next, i++) {
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
    int cur_cmd, c;

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
                if (stack_size == 0) {
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
                if (stack_size == 0) {
                    fprintf(stderr, "Error! Cannot increment nonexistent "
                                    "value!\n");
                    exit(1);
                }
                top->val += 1;
                break;
            case '6':
                if (stack_size == 0) {
                    fprintf(stderr, "Error! Cannot decrement nonexistent "
                                    "value!\n");
                    exit(1);
                }
                if (top->val > 0)
                    top->val -= 1;
                break;
            case '7':
                if (stack_size < 2) {
                    fprintf(stderr, "Error! Cannot add the top values of a%s\n",
                            stack_size ? " one-value stack.": "n empty stack.");
                    exit(1);
                }
                top->next->val += top->val;
                pop_stack();
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
                if (stack_size < 2) {
                    fprintf(stderr, "Error! Cannot get the difference of the"
                                    " top two values of a%s!\n",
                            stack_size ? " one-value stack": "n empty stack");
                    exit(1);
                }
                temp_val = pop_stack();
                if (temp_val <= top->val)
                    top->val -= temp_val;
                else
                    top->val = temp_val - top->val;
                break;
        }
    }
}

/* Executes the stored XRF code */
void execute_code() {
    unsigned cur_chunk = 0;

    top = new_stack_node(0);
    bottom = top;
    atexit(free_stack);

    while (true) {
        execute_chunk(code.commands + (cur_chunk * COMMANDS_PER_CHUNK),
                      code.visited[cur_chunk]);

        code.visited[cur_chunk] = true;
        if (stack_size == 0) {
            fprintf(stderr, "Error! Can't have an empty stack upon reaching "
                            "the end of a chunk!\n");
            exit(1);
        }
        cur_chunk = top->val;
        if (cur_chunk >= code.len / COMMANDS_PER_CHUNK) {
            fprintf(stderr, "Error! Cannot go to nonexistent chunk %u!\n",
                    top->val);
            exit(1);
        }
    }
}

int main(int argc, char **argv) {
	if (argc == 1) {
		fprintf(stderr, "Error! No filename given!");
		exit(1);
	}
    read_xrf_file(argv[1]);
    srand(time(NULL));
    execute_code();
    return 0;
}
