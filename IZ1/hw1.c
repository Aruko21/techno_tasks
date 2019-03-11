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
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <stdbool.h>
#include <math.h>

#define BUF_SIZE 128
#define REALLOC_COEF 2		// But the coefficient 1.5 may be more efficient in some situations


enum return_codes {
	SUCCESS = 0,
	BUF_MEMORY_ALLOC_ERR,
	BUF_MEMORY_REALLOC_ERR,
	INCORRECT_INPUT_ERR,
	INVALID_POINTER_ERR,
	WRONG_SIZE_CUTTING_ERR,
	REALLOC_CUTTING_ERR
};

typedef struct buffer_t {
	size_t curr_size;		// total size (in bytes)
	size_t size_elem;		// size of single element
	void *data;
} buffer_t;

buffer_t * allocate_buffer(size_t element_size);
int reallocate_buffer(buffer_t *user_buf);
int cut_buf(buffer_t *user_buf, size_t count);
void free_buffer(buffer_t *user_buf);
char * reading_from_stdin(buffer_t *user_buf);
int digit_input(int64_t *digit);
int64_t * factorization(int64_t digit);
bool is_digit_or_delimitor(const char symbol);
int print_array(int64_t *array);
void print_err(FILE *stream, int code_of_error, const char *func_name);


int main() {
	int64_t digit = 0;
	int status = digit_input(&digit);
	if (status != SUCCESS) {
		print_err(stderr, status, __func__);
		fprintf(stdout, "[error]\n");
		return 0;
	}
	int64_t *result = factorization(digit);
	if (!result) {
		fprintf(stdout, "[error]\n");
		return 0;
	} else {
		status = print_array(result);
		if (status != SUCCESS) {
			print_err(stderr, status, __func__);
		}
	}
	free(result);
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
		default: {
			err_msg = "Undefined error";
			break;
		}
	}
	fprintf(stderr, "%s: %s\n", func_name, err_msg);
	return;
}

buffer_t * allocate_buffer(size_t element_size) {
	buffer_t *user_buf = (buffer_t *)malloc(sizeof(buffer_t));
	if (!user_buf) {
		return NULL;
	}
	user_buf->data = malloc(BUF_SIZE);
	if (!user_buf->data) {
		free(user_buf);
		return NULL;
	}
	user_buf->curr_size = BUF_SIZE;
	user_buf->size_elem = element_size;
	return user_buf;
}

void free_buffer(buffer_t *user_buf) {
	if (user_buf) {
		free(user_buf->data);
		free(user_buf);
	}
	return;
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
	while (!end && fgets(ptr, chunk_size, stdin)) {
		end = strchr(ptr, '\n');				// Check for ending of input
		if (!end) {								// Check for End Of File
			size_t length = strlen(ptr);
			if (length < (chunk_size - 1)) {	// "- 1" means excluding '\0'
				end = ((char *)(user_buf->data)) + length;
			}
		}
		if (!end) {
			size_t prev_size = user_buf->curr_size - 1;		// excluding '\0'
			int status = reallocate_buffer(user_buf);
			if (status != SUCCESS) {
				print_err(stderr, status, __func__);
				end = NULL;
				break;
			}
			ptr = user_buf->data + prev_size;
			chunk_size = user_buf->curr_size - prev_size;
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

bool is_digit_or_delimitor(const char symbol) {
	if (((symbol != ' ') && (symbol != '\t'))
		&& ((symbol < '0') || (symbol > '9'))) {
		return false;
	} else {
		return true;
	}
}

int print_array(int64_t *array) {
	if (!array) {
		return INVALID_POINTER_ERR;
	}
	size_t index = 0;
	while(array[index] != 0) {
		printf("%lu ", array[index++]);
	}
	return SUCCESS;
}

int digit_input(int64_t *digit) {
	// Reading str from stdin and checking it for a digit
	buffer_t * buffer = allocate_buffer(sizeof(char));
	if (!buffer) {
		print_err(stderr, BUF_MEMORY_ALLOC_ERR, __func__);
		return BUF_MEMORY_ALLOC_ERR;
	}

	char *end = reading_from_stdin(buffer);
	if (!end) {
		free_buffer(buffer);
		return INCORRECT_INPUT_ERR;
	}
	// Now we should checking input on correct
	bool check = false;					// The flag used for the flag below indicates
										// whether a number entry has occurred before
	bool is_digit_before = false;		// Flag for the state when there is a space after the digit
										// Needed to handle the situation with the input of several numbers
	for (char* tmp = buffer->data; tmp < end; ++tmp) {
		if (!is_digit_or_delimitor(*tmp)){
			check = false;
			break;
		}
		if ((*tmp >= '0') && (*tmp <= '9')) {
			check = true;
			if (is_digit_before) {
				// If several numbers were entered
				check = false;
				break;
			}
		} else {
			if (check && ((*tmp == ' ') || (*tmp == '\t'))) {
				is_digit_before = true;
			}
		}
	}
	if (!check) {
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

int64_t * factorization(int64_t digit) {
	// The main logic of program
	buffer_t *values_arr = allocate_buffer(sizeof(int64_t));
	if (!values_arr) {
		print_err(stderr, BUF_MEMORY_ALLOC_ERR, __func__);
		return NULL;
	}
	int64_t *values = values_arr->data;
	int64_t prime = 2;
	values[0] = 1;
	size_t index = 1;
	if (digit == 1) {
		int status = cut_buf(values_arr, (size_t)(index + 1));
		if (status != SUCCESS) {
			print_err(stderr, status, __func__);
			free_buffer(values_arr);
			return NULL;
		}
		values = values_arr->data;			// Update after cutting
		values[index] = 0;
		free(values_arr);
		return values;
	}
	while ((digit != prime) && (prime <= sqrt(digit))) {
		// We can use the condition (prime <= (digit / 2)) - but it's less efficient
		if (digit % prime == 0) {
			if ((index + 2) * values_arr->size_elem >= values_arr->curr_size) {
				// index + 2: one incrementtion need for the last element and another one
				// for null-element
				int status = reallocate_buffer(values_arr);
				if (status != SUCCESS) {
					print_err(stderr, status, __func__);
					values = NULL;
					break;
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
	if (!values) {
		free_buffer(values_arr);
		return NULL;
	}
	values[index++] = digit;
	values[index] = 0;
	int status = cut_buf(values_arr, (size_t)(index + 1));
	if (status != SUCCESS) {
		print_err(stderr, status, __func__);
		free_buffer(values_arr);
		return NULL;
	}
	values = values_arr->data;		// Update after cutting
	free(values_arr);				// We need to save the values address
	return values;
}