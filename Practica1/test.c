#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <linux/kernel.h>

#define SYS_get_current_time 548 
#define SYS_get_uptime 549
#define SYS_get_last_5_kernel_logs 550

int main() {
    long current_time = syscall(SYS_get_current_time);
    long uptime = syscall(SYS_get_uptime);
    char buffer[1024];
    long log_size = syscall(SYS_get_last_5_kernel_logs, buffer, sizeof(buffer));

    if (current_time == -1 || uptime == -1 || log_size == -1) {
        perror("syscall");
        return 1;
    }

    printf("Current time: %ld seconds since the epoch\n", current_time);
    printf("Uptime: %ld seconds since last reboot\n", uptime);
    printf("Last 5 kernel logs:\n%s\n", buffer);

    return 0;
}