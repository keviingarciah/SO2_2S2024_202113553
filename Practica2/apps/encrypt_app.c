#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>

#define SYS_MY_ENCRYPT 454

void print_usage(const char *prog_name) {
    fprintf(stderr, "Usage: %s -p <input_file_path> -o <output_file_path> -j <num_threads> -k <key_file_path>\n", prog_name);
}

int main(int argc, char *argv[]) {
    char *input_file = NULL;
    char *output_file = NULL;
    char *key_file = NULL;
    int num_threads = 0;
    int opt;

    while ((opt = getopt(argc, argv, "p:o:j:k:")) != -1) {
        switch (opt) {
            case 'p':
                input_file = optarg;
                break;
            case 'o':
                output_file = optarg;
                break;
            case 'j':
                num_threads = atoi(optarg);
                break;
            case 'k':
                key_file = optarg;
                break;
            default:
                print_usage(argv[0]);
                return EXIT_FAILURE;
        }
    }

    if (!input_file || !output_file || !key_file || num_threads <= 0) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    long ret = syscall(SYS_MY_ENCRYPT, input_file, output_file, key_file, num_threads);
    if (ret < 0) {
        perror("syscall");
        return EXIT_FAILURE;
    }

    printf("Encryption successful\n");
    return EXIT_SUCCESS;
}