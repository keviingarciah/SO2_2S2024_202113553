#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>

#define SYS_get_memory_info 456
#define SYS_get_swap_info 457
#define SYS_get_page_faults 458
#define SYS_get_active_inactive_pages 459
#define SYS_get_top_memory_processes 460

void test_syscall(long syscall_number, char *description, size_t buffer_size) {
    char *buffer = malloc(buffer_size);
    if (!buffer) {
        perror("malloc");
        return;
    }

    if (syscall(syscall_number, buffer) < 0) {
        perror(description);
    } else {
        printf("%s:\n%s\n", description, buffer);
    }

    free(buffer);
}

int main() {
    test_syscall(SYS_get_memory_info, "Memory Info", 1024);
    test_syscall(SYS_get_swap_info, "Swap Info", 256);
    test_syscall(SYS_get_page_faults, "Page Faults Info", 256);
    test_syscall(SYS_get_active_inactive_pages, "Active and Inactive Pages Info", 256);
    test_syscall(SYS_get_top_memory_processes, "Top Memory Consuming Processes", 1024);

    return 0;
}
