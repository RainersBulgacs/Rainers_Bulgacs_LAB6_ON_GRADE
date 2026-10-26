// Name: Rainers   Lastname: Bulgacs   StudentID: 251RDC073
// (optional) https://github.com/<your-username>/calc-lab-home
// Compile with: gcc -O2 -Wall -Wextra -std=c17 -o calc calc.c -lm
// Resources used: To understand how it works: https://www.youtube.com/watch?v=anMacQo3IR8
// Structure in priority of calculating: https://github.com/WarpRules/recursive_descent_parser_tutorial/blob/main/recursive_descent_parser_tutorial.c
// https://stackoverflow.com/questions/58146228/infinite-recursion-for-parsing-arithmetic-expressions-using-a-recursive-descent
// https://www.geeksforgeeks.org/c/dynamic-memory-allocation-in-c-using-malloc-calloc-free-and-realloc/
// https://stackoverflow.com/questions/2029103/correct-way-to-read-a-text-file-into-a-buffer-in-c
// https://stackoverflow.com/questions/1271064/how-do-i-loop-through-all-files-in-a-folder-using-c
// https://www.geeksforgeeks.org/dsa/expression-evaluation/; https://www.w3schools.com/c/c_files.php
// https://stackoverflow.com/questions/14002954/c-how-to-read-an-entire-file-into-a-buffer
// https://stackoverflow.com/questions/11703082/parsing-math-expression-in-c
// Lecture 4, 5, 6 presentations, more detailed expalined in code.
// AI usage explanation: I tried to use it as my asistant, helped me to understand idea more, first steps to be more oriented what to do. After some promts I realized that is not such great helper.
// Conversation with AI (Chatgpt): https://chatgpt.com/share/6adebb35-6574-83eb-a642-6d4c4de12b73

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <dirent.h>
#include <sys/stat.h>

#define Name "Rainers"
#define Surname "Bulgacs"
#define ID "251RDC073"

static const char *expression;
static size_t expression_len;
static size_t pos;
static size_t line_end;
static size_t err_pos;

static char *out_data;
static size_t out_len;
static size_t out_cap;


void fail_at(size_t offset) { // shows where 1st error happaned.
    if (err_pos == 0) {
        err_pos = offset + 1;
    }
}

void skip_ws(void) { // skips spaces, tabs
    while (pos < line_end && (expression[pos] == ' ' || expression[pos] == '\t' || expression[pos] == '\r')) {
        pos++;
    }
}

char cur(void) { // checks current character in expression
    if (pos < line_end) {
        return expression[pos];
    }
    return '\n'; // returns erorr, shows end of line.
}

long long parse_expr(void);

// to count powers: https://www.geeksforgeeks.org/dsa/expression-evaluation/
long long int_pow(long long base, long long exponent) { //needed to recognize ** operator, which stands for power
    long long result = 1;
    while (exponent > 0) { // exponent gets devided in smallet values, continues until exponent = 0
        if (exponent % 2 == 1) {
            result = result * base;
        }
        exponent = exponent / 2;
        if (exponent > 0) {
            base = base * base;
        }
    }
    return result;
}
// brackets: https://www.geeksforgeeks.org/dsa/c-program-to-check-for-balanced-brackets-in-an-expression-well-formedness-using-stack/
// https://stackoverflow.com/questions/58146228/infinite-recursion-for-parsing-arithmetic-expressions-using-a-recursive-descent
// https://stackoverflow.com/questions/11703082/parsing-math-expression-in-c
long long parse_primary(void) { // checks all signs, brackets, checks for invalid chars
    skip_ws();
    char c = cur();

    if (c >= '0' && c <= '9') { // allowed characters
        long long value = 0;
        while (cur() >= '0' && cur() <= '9') {
            value = value * 10 + (cur() - '0'); // formula for multiple digit numbers, 10, 100 etc.
            pos++; 
        }
        return value;
    }

    if (c == '(') { // bracket checker
        pos++;
        long long value = parse_expr();
        if (err_pos != 0) {
            return 0;
        }
        skip_ws();
        if (cur() != ')') {
            fail_at(pos);
            return 0;
        }
        pos++;
        return value;
    }

    fail_at(pos);
    return 0;
}

// Structure based from: https://github.com/WarpRules/recursive_descent_parser_tutorial/blob/main/recursive_descent_parser_tutorial.c
long long parse_power(void) { // for operation **, to make to do first
    long long base = parse_primary();
    if (err_pos != 0) {
        return 0;
    }

    skip_ws();
    if (cur() == '*' && pos + 1 < line_end && expression[pos + 1] == '*') {
        pos = pos + 2;
        skip_ws();
        size_t exponent_pos = pos;
        long long exponent = parse_power();
        if (err_pos != 0) {
            return 0;
        }
        if (exponent < 0) {
            fail_at(exponent_pos);
            return 0;
        }
        return int_pow(base, exponent);
    }
    return base;
}

// Structure based from: https://github.com/WarpRules/recursive_descent_parser_tutorial/blob/main/recursive_descent_parser_tutorial.c
long long parse_term(void) { // for operators multiple and devide (*,/), to calculate ir first if there is not any power
    long long value = parse_power(); // takes power first
    while (err_pos == 0) {
        skip_ws();
        char c = cur();
        if (c != '*' && c != '/') { // checks for * / in expression
            break;
        }
        pos++;
        skip_ws();
        size_t right_pos = pos;
        long long right = parse_power();
        if (err_pos != 0) { // for devision with 0
            return 0;
        }
        if (c == '*') {
            value = value * right;
        } else { // devision
            if (right == 0) { // x / 0
                fail_at(right_pos);
                return 0;
            }
            value = value / right;
        }
    }
    return value;
}

// Structure based from: https://github.com/WarpRules/recursive_descent_parser_tutorial/blob/main/recursive_descent_parser_tutorial.c
long long parse_expr(void) { // for + and -, to calculate it last, after powers and terms
    long long value = parse_term(); // checks (parse_power) power -> (parse_term) terms -> (parse_expr) simple operators
    while (err_pos == 0) {
        skip_ws();
        char c = cur();
        if (c != '+' && c != '-') {
            break;
        }
        pos++;
        long long right = parse_term();
        if (err_pos != 0) {
            return 0;
        }
        if (c == '+') {
            value = value + right;
        } else {
            value = value - right;
        }
    }
    return value;
}

void validate_expresion(void) { // checks all what is left, if there is for example extra bracket, it gives erorr, expr not valid
    if (err_pos != 0) {
        return; // if there is already mistake it skips
    }
    skip_ws();
    if (pos < line_end) { // mistake if something still left in expression
        fail_at(pos);
    }
}
// https://www.geeksforgeeks.org/c/dynamic-memory-allocation-in-c-using-malloc-calloc-free-and-realloc/
// Presentation from lecture 6 (sliede 9(malloc); slides 11-13(realloc))
int save_expresion(const char *text, size_t n) { // takes all results at memmory
    if (out_len + n + 1 > out_cap) { // if buffer out of memmory then buffer need to be larger
        size_t new_cap = out_cap;
        if (new_cap == 0) {
            new_cap = 1024;
        }
        while (new_cap < out_len + n + 1) {
            new_cap = new_cap * 2; // makes buffer larger twice
        }
        char *bigger = realloc(out_data, new_cap); //realocates results tempoarary
        if (bigger == NULL) {
            return -1;
        }
        out_data = bigger;
        out_cap = new_cap;
    }
    for (size_t k = 0; k < n; k++) {
        out_data[out_len + k] = text[k]; 
    }
    out_len = out_len + n;
    out_data[out_len] = '\0'; //copies symbols in end of buffer makes lenght bigger, and closes it.
    return 0;
}

int calc_expresion(const char *text, size_t len) { // callculates each expresion, goes through evry line, stops when there is erorr 
    expression = text;
    expression_len = len;
    err_pos = 0;
    out_len = 0;
    size_t line_start = 0;
    char tmp[64];

    while (line_start < expression_len) {
        line_end = line_start;
        while (line_end < expression_len && expression[line_end] != '\n') {
            line_end++;
        } // works with 1st line
        pos = line_start;
        skip_ws();

        if (pos < line_end && expression[pos] != '#') { // check comments in next line
            long long value = parse_expr();
            validate_expresion();
            if (err_pos != 0) { 
                out_len = 0;
                int n = sprintf(tmp, "ERROR:%zu\n", err_pos);// if there is error, cancels result, and prints error
                return save_expresion(tmp, (size_t)n);
            }
            int n = sprintf(tmp, "%lld\n", value);
            if (save_expresion(tmp, (size_t)n) != 0) { // converts number in string, if there is no error
                return -1;
            }
        }
        line_start = line_end + 1;
    }
    return 0;
}

// https://stackoverflow.com/questions/2029103/correct-way-to-read-a-text-file-into-a-buffer-in-c
// https://stackoverflow.com/questions/14002954/c-how-to-read-an-entire-file-into-a-buffer
// https://www.geeksforgeeks.org/c/dynamic-memory-allocation-in-c-using-malloc-calloc-free-and-realloc/
// Presentation from lecture 5 (slides 21-24, (fread, perror, ferror))
// Presentation from Lecture 4 (slides 27-28, (pointer paramters))
// Presentation from lecture 6 (slides 9-12, (malloc, realloc, free))
char *load_expresion(const char *path, size_t *len_out) { // reads from input and takes in to memmory
    FILE *fp = fopen(path, "rb");
    if (fp == NULL) {
        perror(path);
        return NULL;
    }

    size_t cap = 65536;
    size_t len = 0;
    char *data = malloc(cap);
    if (data == NULL) {
        fclose(fp);
        return NULL;
    }

    while (true) {
        size_t got = fread(data + len, 1, cap - len, fp);
        len = len + got;
        if (len < cap) {
            break; // reads file until cap-len, if readed less than expected. program brakes 
        } 
        cap = cap * 2;
        char *bigger = realloc(data, cap);
        if (bigger == NULL) {
            free(data);
            fclose(fp);
            return NULL;
        }
        data = bigger; // dubles size if file is biger
    }

    if (ferror(fp) != 0) { // erorr check
        perror(path);
        free(data);
        fclose(fp);
        return NULL;
    }
    fclose(fp);
    *len_out = len;
    return data;
}
// Presentation from lecture 5(slide 24 (perror); slides 28-33 (opendir/clsedir, mkdirr, access modes 0775))
int ensure_dir(const char *path) { // creates forlder for output (e.g. -o "out")
    if (mkdir(path, 0755) == 0) { // makes map, 0755 stands for permisions.
        return 0;
    }
    DIR *d = opendir(path); // if map allready exsists, it opens it
    if (d != NULL) {
        closedir(d);
        return 0;
    }
    perror(path);
    return -1;
}

// Presentation from lecture 4 (slides 13-14 (strings end with '\0')); lecture 5(slide 27 (strcmp); slide 35 (strlen, strchr))
void base_name(const char *path, char *out, size_t size) { // makes file name for output, cuts folders, and .txt after inputs file name.
    size_t end = strlen(path);
    while (end > 1 && path[end - 1] == '/') {
        end--;
    }
    size_t start = end;
    while (start > 0 && path[start - 1] != '/') {
        start--;
    } // cuts form foldes and txt
    size_t m = 0;
    while (start + m < end && m + 1 < size) {
        out[m] = path[start + m];
        m++;
    }// copies name
    out[m] = '\0';
    if (m > 4 && strcmp(out + m - 4, ".txt") == 0) {
        out[m - 4] = '\0';
    }
}
// Presentation from lecture 5,(slides 4-6 fopen with "wb"; slides 19-20 (fwrite), slide 24 (perror))
// https://www.w3schools.com/c/c_files.php
int write_result(const char *out_path) { // write result in output filee
    FILE *fp = fopen(out_path, "wb");
    if (fp == NULL) {
        perror(out_path);
        return -1;
    }
    int result = 0;
    if (out_len > 0 && fwrite(out_data, 1, out_len, fp) != out_len) {
        perror(out_path);
        result = -1;
    } // checks how much is writen in files, if less that expected, returns erorr
    if (fclose(fp) != 0) { // writes data to disk
        perror(out_path);
        result = -1;
    }
    return result;
}

// Presentation from lecture 5, (35-37 (sprintf)); lecture 6(slide 9; malloc)
int process_file(const char *in_path, const char *outdir) { // takes all preocesess ensure_dir, base_name and write_result, to make file
    size_t len = 0;
    char *text = load_expresion(in_path, &len); //loads expresion
    if (text == NULL) {
        return -1;
    }

    int rc = calc_expresion(text, len);
    free(text);
    if (rc != 0) { 
        free(out_data);
        out_data = NULL;
        out_cap = 0;
        return -1;
    }

    int result = 0;
    if (ensure_dir(outdir) != 0) { 
        result = -1;
    } else {
        char base[999];
        base_name(in_path, base, sizeof base);
        char *out_path = malloc(strlen(outdir) + strlen(base) + 64); // gives for path required space
        if (out_path == NULL) {
            result = -1;
        } else {
            sprintf(out_path, "%s/%s_%s_%s_%s.txt",outdir, base, Name, Surname, ID); // builds file name in this format

            if (write_result(out_path) != 0) { // writes result in file
                result = -1;
            }
            free(out_path);
        }
    }

    free(out_data);
    out_data = NULL;
    out_cap = 0;
    return result;
}

// https://stackoverflow.com/questions/1271064/how-do-i-loop-through-all-files-in-a-folder-using-c
// Presentation from lecture 5 (slides 28-30 (opendir, readdir, closedir); slides 35-37 (strrchr, sprintf for path))
// Presentation from lecture 6 (slide 16 (struct pointer access))
int process_directory(const char *dir, const char *outdir) { // for -d option, find all .txt files and executes each
    DIR *d = opendir(dir); // opens map
    if (d == NULL) {
        perror(dir);
        return 1;
    }

    int status = 0;
    struct dirent *entry;
    while ((entry = readdir(d)) != NULL) {
        char *name = (*entry).d_name;
        char *path = malloc(strlen(dir) + strlen(name) + 2);
        if (path == NULL) {
            status = 1;
            continue;
        }
        sprintf(path, "%s/%s", dir, name); // makes full path, foldername/filename
        if (process_file(path, outdir) != 0) {
            status = 1;
        }
        free(path);
    }
    closedir(d);
    return status;
}

void usage(const char *prog) { // says to user to how run progrram, if arguments have issues.
    fprintf(stderr, "Usage: %s [-d DIR | --dir DIR] [-o OUTDIR | --output-dir OUTDIR] [input.txt]\n", prog);
}
// Lecture 5 (slides 25-27(argc, argv))
int main(int argc, char *argv[]) { 
    const char *dir = NULL;
    const char *outdir = NULL;
    const char *input = NULL;

    for (int i = 1; i < argc; i++) { // argc - count of programs; argv[i] - given arguments
        const char *arg = argv[i];
        if (strcmp(arg, "-d") == 0 || strcmp(arg, "--dir") == 0) {
            if (i + 1 >= argc) {
                usage(argv[0]);
                return 1;
            }
            i++;
            dir = argv[i];
        } else if (strcmp(arg, "-o") == 0 || strcmp(arg, "--output-dir") == 0) {
            if (i + 1 >= argc) {
                usage(argv[0]);
                return 1;
            }
            i++;
            outdir = argv[i];
        } else if (arg[0] == '-' && arg[1] != '\0') {
            usage(argv[0]);
            return 1;
        } else if (input == NULL) {
            input = arg;
        }
    }

    char default_out[999];
    if (outdir == NULL) {
        char base[99];
        if (dir != NULL) {
            base_name(dir, base, sizeof base);
        } else {
            base_name(input, base, sizeof base);
        }
        sprintf(default_out, "%s_%s_%s", base, Name, ID);
        outdir = default_out;
    }

    if (input != NULL) {
        if (process_file(input, outdir) != 0) {
            return 1;
        }
        return 0;
    }
    return process_directory(dir, outdir);
}
