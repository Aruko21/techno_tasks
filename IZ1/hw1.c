// Косенков Александр
// АПО-12
//Задача A-4. Задача о простых сомножителях
//Time limit:	14 s
//Memory limit:	64 M
//
//	Составить программу разложения положительного целого числа на простые сомножители и единицу.
//
//Программа считывает входные данные со стандартного ввода, и печатает результат в стандартный вывод.
//
//Верными входными данными считается только ровно одно положительное число, не превосходящее 2^63 - 1,
//возможно с предшествующими или последующими пробельными символами.
//
//Хотя единица не входит в каноническое разложение, но в ответе первым элементом всегда необходимо
//выводить единицу.
//Считать, что разложение самой единицы состоит только из единицы.
//
//Процедура разложения числа должна быть оформлена в виде отдельной функции, которой на вход подается целое число.
//Функция должна возвращать указатель на массив целых чисел, содержащий разложение введенного числа на
//простые сомножители.
//
// Последний элемент этого массива должен быть нулевым.
//
//Программа должна уметь обрабатывать ошибки - такие как неверные входные данные(не число, отрицательное число),
//ошибки выделения памяти и т.п.
//В случае возникновения ошибки нужно вывести сообщение "[error]" (без кавычек) и завершать выполнение программы.

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <stddef.h>
#include <errno.h>
#include <math.h>

#define BUF_SIZE 128
#define REALLOC_COEF 1.5

#define SUCCESS 0
#define BUF_MEMORY_ALLOC_ERR -5
#define BUF_MEMORY_REALLOC_ERR -6
#define INCORRECT_INPUT_ERR -7


typedef struct Buffer {
	size_t curr_size;		// total size (in bytes)
	size_t size_elem;		// size of single element
	void *data;
} Buffer;

Buffer * allocate_buffer(size_t element_size) {
	Buffer *user_buf = (Buffer *)malloc(sizeof(Buffer));
	if (!user_buf) {
		fprintf(stderr, "Allocating memory for struct failed\n");
		return NULL;
	}
	user_buf->data = malloc(BUF_SIZE);
	if (!user_buf->data) {
		fprintf(stderr, "Allocating memory for buffer failed\n");
		free(user_buf);
		return NULL;
	}
	user_buf->curr_size = BUF_SIZE;
	user_buf->size_elem = element_size;
	return user_buf;
}

void free_buffer(Buffer *user_buf) {
	free(user_buf->data);
	free(user_buf);
	return;
}

int reallocate_buffer(Buffer *user_buf) {
	void *new_buf = realloc(user_buf->data, (size_t)(user_buf->curr_size * REALLOC_COEF));
	if (!new_buf) {
		fprintf(stderr, "Reallocating memory for buffer failed\n");
		return BUF_MEMORY_REALLOC_ERR;
	}
	user_buf->data = new_buf;
	user_buf->curr_size *= REALLOC_COEF;
	return SUCCESS;
}

int cut_buf(Buffer *user_buf, ptrdiff_t count) {
	// Buffer compression to real data size
	if (count * user_buf->size_elem >= user_buf->curr_size) {
		fprintf(stderr, "The size of the real data must be less than the buffer size.\n");
		return BUF_MEMORY_ALLOC_ERR;
	}
	void *new_buf = realloc(user_buf->data, user_buf->size_elem * count);
	if (!new_buf) {
		fprintf(stderr, "Reallocating while cutting memory for buffer failed\n");
		return BUF_MEMORY_REALLOC_ERR;
	}
	user_buf->data = new_buf;
	user_buf->curr_size = user_buf->size_elem * count;
	return SUCCESS;
}

char * reading_from_stdin(Buffer *user_buf) {
	size_t chunk_size = user_buf->curr_size;	// Block of new data from stream
	char *ptr = (char *)user_buf->data;			// Current position on buffer, equailent to the
												// stream position
	char *end = NULL;
	while (!end && fgets(ptr, chunk_size, stdin)) {
		end = strchr(ptr, '\n');				// Check for ending of input
		if (!end) {								// Check for End Of File
			size_t length = strlen(ptr);
			if (length < (chunk_size - 1)) {
				end = ((char *)(user_buf->data)) + length;
			}
		}
		if (!end) {
			size_t prev_size = user_buf->curr_size - 1;		// excluding '\0'
			if (reallocate_buffer(user_buf) != SUCCESS) {
				return NULL;
			}
			ptr = user_buf->data + prev_size;				// Update the position in buffer
			chunk_size = user_buf->curr_size - prev_size;
		} else {
			*end = '\0';
			if (cut_buf(user_buf, end - (char *)user_buf->data) != SUCCESS) {
				return NULL;
			}
		}
	}
	return user_buf->data + user_buf->curr_size - 1;
}

int digit_input(long unsigned *digit) {
	// Reading str from stdin and checking it for a digit
	Buffer * buffer = allocate_buffer(sizeof(char));
	if (!buffer) {
		return BUF_MEMORY_ALLOC_ERR;
	}

	char *end = reading_from_stdin(buffer);
	if (!end) {
		free_buffer(buffer);
		return INCORRECT_INPUT_ERR;
	}
	// Now we should checking input on correct
	int check = 0;						// The flag used for the flag below indicates
										// whether a number entry has occurred before
	int is_digit_before = 0;			// Flag for the state when there is a space after the digit
										// Needed to handle the situation with the input of several numbers
	for (char* tmp = buffer->data; tmp < end; ++tmp) {
		if (((*tmp != ' ') && (*tmp != '\t'))
			&& ((*tmp < '0') || (*tmp > '9'))) {
			// Wrong symbol
			free_buffer(buffer);
			return INCORRECT_INPUT_ERR;
		}
		if ((*tmp >= '0') && (*tmp <= '9')) {
			check = 1;
			if (is_digit_before) {
				// If several numbers were entered
				free_buffer(buffer);
				return INCORRECT_INPUT_ERR;
			}
		} else {
			if (check && ((*tmp == ' ') || (*tmp == '\t'))) {
				is_digit_before = 1;
			}
		}
	}
	if (!check) {
		// If the string consisted only of spaces
		free_buffer(buffer);
		return INCORRECT_INPUT_ERR;
	}
	// And now we can convert our string to digit
	*digit = strtol(buffer->data, &end, 10);
	if (errno == ERANGE) {
		// If the entered number exceeds 2^63 - 1
		free_buffer(buffer);
		return INCORRECT_INPUT_ERR;
	}
	if (*digit == 0) {
		free_buffer(buffer);
		return INCORRECT_INPUT_ERR;
	}
	free_buffer(buffer);
	return SUCCESS;
}

long unsigned * factorization(long unsigned digit) {
	// The main logic of program
	Buffer *values_arr = allocate_buffer(sizeof(long unsigned));
	if (!values_arr) {
		return NULL;
	}
	long unsigned *values = values_arr->data;
	long unsigned prime = 2;
	values[0] = 1;
	size_t index = 1;
	if (digit == 1) {
		if (cut_buf(values_arr, (ptrdiff_t)(index + 1)) != SUCCESS) {
			free_buffer(values_arr);
			return NULL;
		}
		values = values_arr->data;			// Update after cutting
		values[index] = 0;
		free(values_arr);
		return values;
	}
	while ((digit != prime) && (prime <= sqrt(digit))) {
		// sqrt(digit) - from the divisibility check algorithm
		if (digit % prime == 0) {
			if ((index + 2) * values_arr->size_elem >= values_arr->curr_size) {
				// index + 2: one incrementtion need for the last element and another one
				// for null-element
				if (reallocate_buffer(values_arr) != SUCCESS) {
					free_buffer(values_arr);
					return NULL;
				}
				values = values_arr->data;
			}
			values[index++] = prime;
			digit /= prime;
		} else {
			// We can added a test for a prime, but it's useless
			prime++;
		}
	}
	values[index++] = digit;
	values[index] = 0;
	if (cut_buf(values_arr, (ptrdiff_t)(index + 1)) != SUCCESS) {
		free_buffer(values_arr);
		return NULL;
	}
	values = values_arr->data;		// Update after cutting
	free(values_arr);				// We need to save the values address
	return values;
}

int main() {
	long unsigned digit = 0;
	if (digit_input(&digit) != SUCCESS) {
		fprintf(stdout, "[error]\n");
		return 0;
	}
	long unsigned *result = factorization(digit);
	if (!result) {
		fprintf(stdout, "[error]\n");
		return 0;
	} else {
		size_t index = 0;
		while(result[index] != 0) {
			printf("%lu ", result[index++]);
		}
		putchar('\n');
	}
	free(result);
	return 0;
}
