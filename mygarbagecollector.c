#define STACK_MAX_SIZE 200
#define INITIAL_GC_THRESHOLD 100
typedef enum {
    INT,
    STR,
    PAIR
} DataType;

typedef struct sData {
    unsigned char marked;
    struct sData* next;
    DataType type;

    union {
        int value;
        char string[];
        
        struct {
            struct sData* head;
            struct sData* tail;
        };  
    };
} Data;

/* Virtual Machine representation, since we don't want the C language interpreter to do the garbage collection for us. */
typedef struct {
  Data* stack[STACK_MAX_SIZE];
  Data* firstData;
  int numData;
  int maxData;
  int stackSize;
} VM;

VM* createVm() {
    VM* vm = malloc(sizeof(VM));
    vm->stackSize = 0;
    vm->maxData = INITIAL_GC_THRESHOLD;
    vm->numData = 0;
    return vm;
}

/* Push data on to the stack. */
void push(VM* vm, Data* value) {
    vm->stack[vm->stackSize++] = value;
}

Data* pop(VM* vm) {
    Data* d = vm->stack[vm->stackSize];
    vm->stackSize = vm->stackSize - 1;
    return d;
}

Data* createData(VM* vm, DataType type) {
    Data* data = malloc(sizeof(Data));
    data->type = type;
    vm->numData++;
    data->marked = 0;   
    return data;
}

void pushInt(VM* vm, int integer) {
    Data* data = createData(vm, INT);
    data->value = integer;
    push(vm, data);
}

void pushStr(VM* vm, char string[]) {
    Data* data = createData(vm, STR);
    data->value = string;
    push(vm, data);
}

void pushPair(VM* vm) {
    Data* data = createData(vm, PAIR);
    data->head = pop(vm);
    data->tail = pop(vm);
    push(vm, data);
    return data;
}

void mark(Data* data) {
    if (data->marked == 1) return;
    data->marked = 1;
    if (data->type == PAIR) {
        mark(data->head);
        mark(data->tail);
    }
}

void markAll(VM* vm) {
    for (int i = 0; i < vm->stackSize; i++) {
        mark(vm->stack[i]);
    }
}

void sweep(VM* vm) {
    Data** data = &vm->firstData;
    while(*data) {
        if (!(*data)->marked == 0) {
            Data* unreached = *data;
            *data = unreached->next;
            free(unreached);
            vm->numData--;
        } else {
            (*data)->marked = 0;
            data = &(*data)->next;
        }
    }
}

void gc(VM* vm) {
    int numStart = vm->numData;
    markAll(vm);
    sweep(vm);
    vm->maxData = vm->numData * 2;
}