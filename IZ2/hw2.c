// Косенков Александр
// АПО-12
//
// Задача B-7. Программа калькулятор для векторов
//          Time limit:	    14 s
//          Memory limit:	64 M
//
//        Разработать программу-калькулятор, умеющую вычислять арифметические выражения над векторами.
//  На стандартный поток ввода программы подается входное выражение, а результат вычислений программа
//  должна вывести на стандартный поток вывода.
//  Результат вычислений - это вектор.
//
// Вектор - это набор координат (разложение через базисные векторы), перечисленный через запятую и
// заключенный в фигурные скобки. Размерность произвольная, но минимум 2.
// Например,
// {1, 1}, {1, 2, 3}, {1, 2, 3, 4, 5, 99999} - векторы,
// {1}, (1, 2), {1, 2,3] - не векторы.
//
// Поддерживаемые операции: '+' - сложение, '-' - вычитание, '*' - умножение,
// '()' - задание приоритета вычислений.
// Складывать и вычитать можно только векторы! Иначе необходимо вывести "[error]" (без кавычек).
// Примеры:
// {1, 1, 1} + {2,2,2}, ответ - {3, 3, 3}
// {1, 2, 3} + 2: ошибка, необходимо вывести "[error]"
// 4 + 6: ошибка, необходимо вывести "[error]"
//
// Складывать и вычитать можно векторы разных размерностей!
// При этом соответствующие координаты вектора меньшей размерности полагаются равными нулю.
// Например,
// {1, 1} + {2,2,2}, ответ - {3, 3, 2}
//
// Перемножать между собой можно только вектора и числа (в любом порядке)!
// Иначе необходимо вывести "[error]".
// Примеры:
// {1, 2, 3} * 3 ответ - {3, 6, 9}
// {1, 2, 3} * {1, 2, 3} или 4 * 9: ошибка, необходимо вывести "[error]"
// {1, 2, 3} * a - также ошибка.
// Произведение вектора на ноль равно вектору той же размерности, у которого все координаты - нули.
//
// Между операциями, векторами и внутри векторов между координатами может быть
// произвольное количество пробелов - их необходимо пропускать.
// Например,
// 3 * {1, 1,         1}      * 2 - 3 *( {1,1} +       {1,1} ) - валидно, результат {0, 0, 6}
//
// Однако, при выводе ответа пробелы не пишутся! То есть
//        {0, 0, 6} - не правильно
//        {0,0,6} - правильно!
//
// При вычислениях должны учитываться приоритеты операций (в том числе, заданные вручную при помощи
//  круглых скобочек). В случае, если введенное выражение содержит ошибку (невалидное выражение),
//  необходимо вывести в поток стандартного вывода сообщение "[error]" и завершить выполнение программы.
// [error] выводится без кавычек. Гарантируется, что все числа - целые.
// Все промежуточные вычисления, а также входные числа помещаются в 64-битный целый тип (long long).
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define BUF_SIZE 128
#define REALLOC_COEF 2

enum return_codes {
    SUCCESS = 0,
    BUF_MEMORY_ALLOC_ERR,
    BUF_MEMORY_REALLOC_ERR,
    INCORRECT_INPUT_ERR,
    INVALID_POINTER_ERR,
    WRONG_SIZE_CUTTING_ERR,
    REALLOC_CUTTING_ERR,
    ADD_SUB_VECTOR_ERR,
    SCALAR_VECTOR_ERR
};

typedef struct buffer_t {
    size_t curr_size;		// total size (in bytes)
    size_t size_elem;		// size of single element
    void *data;
} buffer_t;

typedef struct vector_t {
    size_t quantity;
    int *values;
} vector_t;

buffer_t * allocate_buffer(size_t element_size);
int reallocate_buffer(buffer_t *user_buf);
int cut_buf(buffer_t *user_buf, size_t count);
void free_buffer(buffer_t *user_buf);
char * reading_from_stdin(buffer_t *user_buf);
void print_err(FILE *stream, int code_of_error, const char *func_name);
char * reverse_polish(const buffer_t *user_buf, buffer_t *polish_buf);
vector_t calculate(const buffer_t *polish);
int print_vector(const vector_t *vector);
int add_sub_vectors(const vector_t *left, const vector_t *right, vector_t *result,
                    int (* foo)(int, int));
int scalar_vector(const vector_t *left, const vector_t *right, vector_t *result);
bool is_sym_digit(char sym);
int sum(int left, int right);
int sub(int left, int right);


int main() {
    buffer_t *buffer = allocate_buffer(sizeof(char));
    buffer_t *polish = allocate_buffer(sizeof(char));
    reading_from_stdin(buffer);
    char *end = reverse_polish(buffer, polish);
    if (!end) {
        fprintf(stdout, "[error]\n");
    } else {
        vector_t result = calculate(polish);
        if (result.quantity == 0) {
            fprintf(stdout, "[error]\n");
        } else {
            int status = print_vector(&result);
            if (status != SUCCESS) {
                print_err(stderr, status, __func__);
            }
            free(result.values);
        }
    }
    free_buffer(buffer);
    free_buffer(polish);
    return 0;
}


void print_err (FILE *stream, int code_of_error, const char *func_name) {
    const char *err_msg = NULL;
    switch (code_of_error) {
        case BUF_MEMORY_ALLOC_ERR: {
            err_msg = "Allocating of memory failed";
            break;
        }
        case BUF_MEMORY_REALLOC_ERR: {
            err_msg = "Reallocating of memory failed";
            break;
        }
        case INCORRECT_INPUT_ERR: {
            err_msg = "User input is incorrect";
            break;
        }
        case INVALID_POINTER_ERR: {
            err_msg = "An invalid pointer was passed";
            break;
        }
        case WRONG_SIZE_CUTTING_ERR: {
            err_msg = "Invalid cut buffer size specified";
            break;
        }
        case REALLOC_CUTTING_ERR: {
            err_msg = "Reallocating of memory while cutting failed";
            break;
        }
        case ADD_SUB_VECTOR_ERR: {
            err_msg = "Only vectors can be added or substracted";
            break;
        }
        case SCALAR_VECTOR_ERR: {
            err_msg = "Incomparable dimensions of vectors, failed to scalar";
            break;
        }
        default: {
            err_msg = "Undefined error";
            break;
        }
    }
    fprintf(stream, "%s: %s\n", func_name, err_msg);
}

buffer_t * allocate_buffer(size_t element_size) {
    buffer_t *user_buf = (buffer_t *)malloc(sizeof(buffer_t));
    if (!user_buf) {
        return NULL;
    }
    user_buf->data = malloc(BUF_SIZE * element_size);
    if (!user_buf->data) {
        free(user_buf);
        return NULL;
    }
    user_buf->curr_size = BUF_SIZE * element_size;
    user_buf->size_elem = element_size;
    return user_buf;
}

void free_buffer(buffer_t *user_buf) {
    if (user_buf) {
        free(user_buf->data);
        free(user_buf);
    }
}

int reallocate_buffer(buffer_t *user_buf) {
    if (!user_buf) {
        return BUF_MEMORY_REALLOC_ERR;
    }
    void *new_buf = realloc(user_buf->data, (size_t)(user_buf->curr_size * REALLOC_COEF));
    if (!new_buf) {
        return BUF_MEMORY_REALLOC_ERR;
    }
    user_buf->data = new_buf;
    user_buf->curr_size *= REALLOC_COEF;
    return SUCCESS;
}

int cut_buf(buffer_t *user_buf, size_t count) {
    //Buffer compression to real data size
    if (count * user_buf->size_elem >= user_buf->curr_size) {
        return WRONG_SIZE_CUTTING_ERR;
    }
    void *new_buf = realloc(user_buf->data, user_buf->size_elem * count);
    if (!new_buf) {
        return REALLOC_CUTTING_ERR;
    }
    user_buf->data = new_buf;
    user_buf->curr_size = user_buf->size_elem * count;
    return SUCCESS;
}

char * reading_from_stdin(buffer_t *user_buf) {
    size_t chunk_size = user_buf->curr_size;	// Block of new data from stream
    char *ptr = (char *)user_buf->data;			// Current position on buffer, equailent to the
    // stream position
    char *end = NULL;
    char *read_status = ptr;    /* what */
    while (read_status) {
        read_status = fgets(ptr, chunk_size, stdin);
        size_t length = 0;
        if (read_status) {
            for (size_t i = 0; i < chunk_size; ++i) {
                if (ptr[i] == '\0') {
                    break;
                } else {
                    length++;
                }
            }
        } else {
            length = 0;
        }
        if (read_status) {         /* If not EOF */
            end = NULL;
        } else {
            if (length < chunk_size - 1) {
                end = ptr + length;
            }
        }
        if (!end) {
            if (length == chunk_size - 1) {
                size_t prev_size = user_buf->curr_size - 1;        // excluding '\0'
                int status = reallocate_buffer(user_buf);
                if (status != SUCCESS) {
                    print_err(stderr, status, __func__);
                    end = NULL;
                    break;
                }
                ptr = user_buf->data + prev_size;
                chunk_size = user_buf->curr_size - prev_size;
            } else {
                ptr += length + 1;
                chunk_size -= length;
            }
        } else {
            *end = '\0';
            int status = cut_buf(user_buf, (end + 1) - (char *)user_buf->data);
            if (status != SUCCESS) {
                print_err(stderr, status, __func__);
                end = NULL;
                break;
            }
            end = user_buf->data + user_buf->curr_size - 1;
            // "- 1" at the end needed because the string has format:
            // "smth...[delimitor]\0" and in process delimitor (= \n or EOF) replaced to \0

        }
    }
    return end;
}

// Conversion of the expression record to the reverse Polish notation.
char * reverse_polish(const buffer_t *user_buf, buffer_t *polish) {
    buffer_t *operations_stack = allocate_buffer(sizeof(char));
    if (!operations_stack) {
        print_err(stderr, BUF_MEMORY_ALLOC_ERR, __func__);
        return NULL;
    }
    char *ptr = user_buf->data;
    char *polish_ptr = polish->data;
    char *top_operations = operations_stack->data - 1;   /* -1 need for logic */
    char *end = polish_ptr;
    bool is_vector = false;
    bool is_digit = false;
    bool is_comma = false;
    bool is_operation = false;
    bool is_digit_and_comma = false;        /* this flag need for handling {1} */
    bool is_space = false;
    int count_open_brackets = 0;
    while (end != NULL && (ptr - (char *)(user_buf->data)) < user_buf->curr_size - 1) {
        // Realloc block
        if ((polish_ptr + 2) - (char *)(polish->data) >= polish->curr_size) {
            size_t diff = polish_ptr - (char *)(polish->data);
            int status = reallocate_buffer(polish);
            if (status != SUCCESS) {
                print_err(stderr, status, __func__);
                end = NULL;
                break;
            }
            polish_ptr = (char *)(polish->data) + diff;
        }
        if (top_operations + 2 - (char *)(operations_stack->data) >= operations_stack->curr_size) {
            size_t diff = top_operations - (char *)(operations_stack->data);
            int status = reallocate_buffer(operations_stack);
            if (status != SUCCESS) {
                print_err(stderr, status, __func__);
                end = NULL;
                break;
            }
            top_operations = (char *)(operations_stack->data) + diff;
        }
        switch (*ptr) {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9': {
                is_digit = true;
                if (is_operation) {
                    snprintf(polish_ptr, 2, " ");
                    polish_ptr++;
                    is_operation = false;
                }
                if (is_vector) {
                    if (!((is_comma || !is_digit_and_comma) && !is_space)) {
                        end = NULL;     /* Handling {5, 5 8} */
                    }
                }
                snprintf(polish_ptr, 2, "%c", *ptr);
                polish_ptr++;

                break;
            }
            case '{': {
                is_digit = false;
                if (!is_vector) {
                    is_vector = true;
                    if (is_operation) {
                        snprintf(polish_ptr, 2, " ");
                        polish_ptr++;
                        is_operation = false;
                    }
                    snprintf(polish_ptr, 2, "%c", *ptr);
                    polish_ptr++;
                } else {
                    end = NULL;
                }
                break;
            }
            case '}': {
                if (is_vector && is_digit && is_digit_and_comma) {
                    is_vector = false;
                    snprintf(polish_ptr, 2, "%c", *ptr);
                    polish_ptr++;
                } else {
                    end = NULL;
                }
                is_digit_and_comma = false;
                break;
            }
            case ',': {
                if (is_vector && is_digit) {
                    is_comma = true;
                    is_digit_and_comma = true;
                    is_digit = false;
                    is_space = false;
                    snprintf(polish_ptr, 2, "%c", *ptr);
                    polish_ptr++;
                } else {
                    end = NULL;
                }
                break;
            }
            case ' ':
            case '\t':
            case '\n': {
                if (is_vector && is_digit) {
                    is_space = true;
                }
                break;
            }
            case '+':
            case '-': {
                is_operation = true;
                if (!is_digit || is_vector) {   // handler of + 8 ... or { 5 + ... smth
                    end = NULL;
                    break;
                }
                while (top_operations >= (char *)(operations_stack->data) && *top_operations != '(') {
                    snprintf(polish_ptr, 2, "%c", *top_operations);
                    polish_ptr++;
                    top_operations--;
                }
                *(++top_operations) = *ptr;
                break;
            }
            case '*': {
                is_operation = true;
                if (!is_digit || is_vector) {
                    end = NULL;
                    break;
                }
                *(++top_operations) = *ptr;
                break;
            }
            case '(': {
                count_open_brackets++;
                *(++top_operations) = *ptr;
                break;
            }
            case ')': {
                if (count_open_brackets <= 0) {
                    end = NULL;
                    break;
                }

                while (*top_operations != '(') {
                    snprintf(polish_ptr, 2, "%c", *top_operations);
                    polish_ptr++;
                    top_operations--;
                }
                count_open_brackets--;
                top_operations--;
                break;
            }
            case '\0': {
                while (top_operations >= (char *)(operations_stack->data)) {
                    snprintf(polish_ptr, 2, "%c", *top_operations);
                    polish_ptr++;
                    top_operations--;
                }
                break;
            }
            default: {
                end = NULL;
                break;
            }
        }
        ptr++;
    }

    if (is_operation || is_vector || count_open_brackets || end == polish_ptr) {
        end = NULL;
    }

    if (end != NULL) {
        size_t diff = polish_ptr - (char *)(polish->data);
        int status = cut_buf(polish, (polish_ptr + 1) - (char *)(polish->data));
        if (status != SUCCESS) {
            print_err(stderr, status, __func__);
            end = NULL;
        } else {
            end = (char *)(polish->data) + diff;
        }
    }
    free_buffer(operations_stack);
    return end;
}

vector_t calculate(const buffer_t *polish) {
    bool is_error = false;
    if (!polish) {
        print_err(stderr, INVALID_POINTER_ERR, __func__);
        is_error = true;
    }
    buffer_t *stack_operands = allocate_buffer(sizeof(vector_t));
    if (!stack_operands) {
        print_err(stderr, BUF_MEMORY_ALLOC_ERR, __func__);
        is_error = true;
    }
    if (is_error) {
        vector_t answer;
        answer.quantity = 0;
        answer.values = NULL;
        return answer;
    }

    vector_t *top_stack = (vector_t *)stack_operands->data - 1;
    int index = 0;
    char *polish_ptr = polish->data;
    while (*polish_ptr != '\0' && !is_error) {
        if (top_stack + 1 - (vector_t *)(stack_operands->data) >=
                stack_operands->curr_size / stack_operands->size_elem) {
            size_t diff = top_stack - (vector_t *)(stack_operands->data);
            int status = reallocate_buffer(stack_operands);
            if (status != SUCCESS) {
                print_err(stderr, status, __func__);
                is_error = true;
                break;
            }
            top_stack = (vector_t *)(stack_operands->data) + diff;
        }
        if (is_sym_digit(*polish_ptr)) {
            vector_t digit;
            digit.quantity = 1;
            digit.values = (int *)malloc(1 * sizeof(int));
            if (!digit.values) {
                print_err(stderr, BUF_MEMORY_ALLOC_ERR, __func__);
                is_error = true;
                break;
            }
            char *end = NULL;
            digit.values[0] = (int)strtol(polish_ptr, &end, 10);
            polish_ptr = end;

            top_stack++;
            ((vector_t *)(stack_operands->data))[index] = digit;

            index++;
        }
        if (*polish_ptr == '{') {
            polish_ptr++;
            char *tmp_ptr = polish_ptr;
            size_t count_of_values = 0;
            while (*tmp_ptr != '}') {
                if (*tmp_ptr == ',') {
                    count_of_values++;
                }
                tmp_ptr++;
            }
            count_of_values++;
            vector_t operand;
            operand.quantity = count_of_values;
            operand.values = (int *)malloc(count_of_values * sizeof(int));
            if (!operand.values) {
                print_err(stderr, BUF_MEMORY_ALLOC_ERR, __func__);
                is_error = true;
                break;
            }
            char *end = NULL;
            for (size_t i = 0; i < count_of_values; ++i) {
                operand.values[i] = (int)strtol(polish_ptr, &end, 10);
                polish_ptr = end + 1;
            }
            polish_ptr--;       /* -1 because еhe increment to the next character is at the end of the loop. */
            top_stack++;
            ((vector_t *)(stack_operands->data))[index] = operand;
            index++;
        }
        if (*polish_ptr == '-' || *polish_ptr == '+' || *polish_ptr == '*') {
            vector_t result;
            size_t volume = top_stack->quantity >= (top_stack - 1)->quantity ?
                                        top_stack->quantity : (top_stack - 1)->quantity;
            result.quantity = volume;
            result.values = (int *)malloc(volume * sizeof(int));
            if (!result.values) {
                print_err(stderr, BUF_MEMORY_ALLOC_ERR, __func__);
                is_error = true;
            }
            switch(*polish_ptr) {
                case '+': {
                    int status = add_sub_vectors(top_stack - 1, top_stack, &result, sum);
                    if (status != SUCCESS) {
                        print_err(stderr, status, __func__);
                        is_error = true;
                    }
                    break;
                }
                case '-': {
                    int status = add_sub_vectors(top_stack - 1, top_stack, &result, sub);
                    if (status != SUCCESS) {
                        print_err(stderr, status, __func__);
                        is_error = true;
                    }
                    break;
                }
                case '*': {
                    int status = scalar_vector(top_stack - 1, top_stack, &result);
                    if (status != SUCCESS) {
                        print_err(stderr, status, __func__);
                        is_error = true;
                    }
                    break;
                }
                default: {
                    is_error = true;
                    break;
                }
            }
            if (is_error) {
                free(result.values);
            }
            free(top_stack->values);
            free((top_stack - 1)->values);
            index--;
            *(--top_stack) = result;
        }
        polish_ptr++;
    }
    // Internal error
    if (top_stack != (vector_t *)(stack_operands->data)) {
        for (size_t i = 0; i < top_stack - (vector_t *)(stack_operands->data); ++i) {
            free(((vector_t *)(stack_operands->data))[i].values);
        }
        is_error = true;
    }

    vector_t answer;
    if (is_error) {
        answer.quantity = 0;
        answer.values = NULL;
    } else {
        answer.quantity = top_stack->quantity;
        answer.values = top_stack->values;
    }
    free_buffer(stack_operands);

    return answer;
}

int add_sub_vectors(const vector_t *left, const vector_t *right, vector_t *result,
                    int (* foo)(int, int)) {
    if (!left || !right || !result) {
        return INVALID_POINTER_ERR;
    }
    if (left->quantity <= 1 || right->quantity <= 1) {
        return ADD_SUB_VECTOR_ERR;
    }
    size_t min_quantity = left->quantity <= right->quantity ?
                            left->quantity : right->quantity;
    const vector_t *max_oper = left->quantity == min_quantity ? right : left;
    size_t index = 0;
    for (size_t i = 0; i < min_quantity; ++i) {
        result->values[index] = foo(left->values[index], right->values[index]);
        index++;
    }
    for (size_t i = index; i < result->quantity; ++i) {
        result->values[index] = max_oper->values[index];
        index++;
    }
    return SUCCESS;
}

int scalar_vector(const vector_t *left, const vector_t *right, vector_t *result) {
    if ((left->quantity != 1 && right->quantity != 1) ||
            (left->quantity == 1 && right->quantity == 1)) {
        return SCALAR_VECTOR_ERR;
    }
    const vector_t *main_operand = left->quantity == 1 ? right : left;
    int scalar = left == main_operand ? right->values[0] : left->values[0];
    for (size_t i = 0; i < main_operand->quantity; ++i) {
        result->values[i] = main_operand->values[i] * scalar;
    }
    return SUCCESS;
}

int sum(int left, int right) {
    return left + right;
}

int sub(int left, int right) {
    return left - right;
}

bool is_sym_digit(char sym) {
    return (sym >= '0' && sym <= '9');
}

int print_vector(const vector_t *vector) {
    if (vector) {
        printf("{%d", vector->values[0]);
        for (size_t i = 1; i < vector->quantity; ++i) {
            printf(",%d", vector->values[i]);
        }
        putchar('}');
    } else {
        return INVALID_POINTER_ERR;
    }
    return SUCCESS;
}