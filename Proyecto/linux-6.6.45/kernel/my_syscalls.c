#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/swap.h>
#include <linux/sort.h>
#include <linux/fs.h>
#include <linux/fdtable.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/syscalls_usac.h>

#define MEMINFO_BUFFER_SIZE 1024

// ---------------- PRACTICA 2 ---------------- 
// Structure to store data fragments
typedef struct {
    unsigned char *data;
    size_t data_size;
    unsigned char *key;
    size_t key_size;
    size_t start;
    size_t end;
} DataFragment;

// Structure to store task arguments
struct task_parameters {
    DataFragment fragment;
    struct completion done;
};

// ---------------- PROYECTO ----------------
struct process_mem_info {
    pid_t pid;
    unsigned long mem_usage;
    char comm[TASK_COMM_LEN];
};

// ---------------- PRACTICA 2 ---------------- 
// Function that performs the XOR operation on a data fragment
int xor_task(void *arg) {
    struct task_parameters *params = (struct task_parameters *)arg;
    DataFragment *fragment = &params->fragment;
    size_t i;

    printk(KERN_INFO "Thread started: start=%zu, end=%zu\n", fragment->start, fragment->end);

    for (i = fragment->start; i < fragment->end; i++) {
        fragment->data[i] ^= fragment->key[i % fragment->key_size];
    }

    printk(KERN_INFO "Thread completed: start=%zu, end=%zu\n", fragment->start, fragment->end);
    complete(&params->done); 
    return 0;
}

// Function that processes the input file and performs the XOR operation
int process_file(const char *input_path, const char *output_path, const char *key_path, int num_threads) {
    struct file *input_file, *output_file, *key_file;
    loff_t input_offset = 0, output_offset = 0, key_offset = 0;
    unsigned char *key_buffer, *data_buffer;
    size_t input_file_size, key_file_size;
    struct task_parameters *task_params;
    struct task_struct **threads;
    DataFragment *fragments;
    size_t fragment_size, remainder;
    int i, ret = 0;

    printk(KERN_INFO "Opening files: input=%s, output=%s, key=%s\n", input_path, output_path, key_path);

    input_file = filp_open(input_path, O_RDONLY, 0);
    output_file = filp_open(output_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    key_file = filp_open(key_path, O_RDONLY, 0);

    if (IS_ERR(input_file)) {
        ret = PTR_ERR(input_file);
        printk(KERN_ERR "Failed to open input file: %d\n", ret);
        goto out;
    }
    if (IS_ERR(output_file)) {
        ret = PTR_ERR(output_file);
        printk(KERN_ERR "Failed to open output file: %d\n", ret);
        goto close_input_file;
    }
    if (IS_ERR(key_file)) {
        ret = PTR_ERR(key_file);
        printk(KERN_ERR "Failed to open key file: %d\n", ret);
        goto close_output_file;
    }

    key_file_size = i_size_read(file_inode(key_file));
    if (key_file_size <= 0) {
        ret = -EINVAL;
        printk(KERN_ERR "Invalid key size: %zu\n", key_file_size);
        goto close_key_file;
    }

    key_buffer = kmalloc(key_file_size, GFP_KERNEL);
    if (!key_buffer) {
        ret = -ENOMEM;
        printk(KERN_ERR "Failed to allocate memory for key\n");
        goto close_key_file;
    }

    ret = kernel_read(key_file, key_buffer, key_file_size, &key_offset);
    if (ret < 0) {
        printk(KERN_ERR "Failed to read key: %d\n", ret);
        goto free_key_buffer;
    }

    input_file_size = i_size_read(file_inode(input_file));
    if (input_file_size <= 0) {
        ret = -EINVAL;
        printk(KERN_ERR "Invalid input file size: %zu\n", input_file_size);
        goto free_key_buffer;
    }

    data_buffer = kmalloc(input_file_size, GFP_KERNEL);
    if (!data_buffer) {
        ret = -ENOMEM;
        printk(KERN_ERR "Failed to allocate memory for data\n");
        goto free_key_buffer;
    }

    ret = kernel_read(input_file, data_buffer, input_file_size, &input_offset);
    if (ret < 0) {
        printk(KERN_ERR "Failed to read input file: %d\n", ret);
        goto free_data_buffer;
    }

    // Allocate memory for threads, task parameters, and fragments
    threads = kmalloc(sizeof(struct task_struct *) * num_threads, GFP_KERNEL);
    task_params = kmalloc(sizeof(struct task_parameters) * num_threads, GFP_KERNEL);
    fragments = kmalloc(sizeof(DataFragment) * num_threads, GFP_KERNEL);

    if (!threads || !task_params || !fragments) {
        ret = -ENOMEM;
        printk(KERN_ERR "Failed to allocate memory for threads, task parameters, or fragments.\n");
        goto free_data_buffer;
    }

    fragment_size = input_file_size / num_threads;
    remainder = input_file_size % num_threads;

    for (i = 0; i < num_threads; i++) {
        fragments[i].data = data_buffer;
        fragments[i].data_size = input_file_size;
        fragments[i].key = key_buffer;
        fragments[i].key_size = key_file_size;
        fragments[i].start = i * fragment_size;
        fragments[i].end = (i == num_threads - 1) ? (i + 1) * fragment_size + remainder : (i + 1) * fragment_size;

        printk(KERN_INFO "Creating thread %d: start=%zu, end=%zu\n", i, fragments[i].start, fragments[i].end);

        task_params[i].fragment = fragments[i];
        init_completion(&task_params[i].done);

        threads[i] = kthread_run(xor_task, &task_params[i], "xor_thread_%d", i);
        if (IS_ERR(threads[i])) {
            ret = PTR_ERR(threads[i]);
            printk(KERN_ERR "Failed to create thread %d: %d\n", i, ret);
            goto free_all;
        }
    }

    // Wait for all threads to complete
    for (i = 0; i < num_threads; i++) {
        printk(KERN_INFO "Waiting for thread %d to complete\n", i);
        wait_for_completion(&task_params[i].done);
    }

    ret = kernel_write(output_file, data_buffer, input_file_size, &output_offset);
    if (ret < 0) {
        printk(KERN_ERR "Failed to write output file: %d\n", ret);
    } else {
        printk(KERN_INFO "Output file written successfully\n");
    }

free_all:
    kfree(threads);
    kfree(task_params);
    kfree(fragments);

free_data_buffer:
    kfree(data_buffer);

free_key_buffer:
    kfree(key_buffer);

close_key_file:
    filp_close(key_file, NULL);

close_output_file:
    filp_close(output_file, NULL);

close_input_file:
    filp_close(input_file, NULL);

out:
    return ret;
}

// Definition of the my_encrypt syscall
SYSCALL_DEFINE4(my_encrypt, const char __user *, input_path, const char __user *, output_path, const char __user *, key_path, int, num_threads) {
    char *k_input_path, *k_output_path, *k_key_path;
    int ret;

    printk(KERN_INFO "Entering my_encrypt syscall\n");

    k_input_path = strndup_user(input_path, PATH_MAX);
    k_output_path = strndup_user(output_path, PATH_MAX);
    k_key_path = strndup_user(key_path, PATH_MAX);

    printk(KERN_INFO "Input file path: %s\n", k_input_path);
    printk(KERN_INFO "Output file path: %s\n", k_output_path);
    printk(KERN_INFO "Key file path: %s\n", k_key_path);

    if (IS_ERR(k_input_path) || IS_ERR(k_output_path) || IS_ERR(k_key_path)) {
        ret = -EFAULT;
        printk(KERN_ERR "Failed to copy paths from user space\n");
        goto out_free;
    }

    ret = process_file(k_input_path, k_output_path, k_key_path, num_threads);

out_free:
    kfree(k_input_path);
    kfree(k_output_path);
    kfree(k_key_path);
    return ret;
}

// Definition of the my_decrypt syscall
SYSCALL_DEFINE4(my_decrypt, const char __user *, input_path, const char __user *, output_path, const char __user *, key_path, int, num_threads) {
    char *k_input_path, *k_output_path, *k_key_path;
    int ret;

    printk(KERN_INFO "Entering my_decrypt syscall\n");

    k_input_path = strndup_user(input_path, PATH_MAX);
    k_output_path = strndup_user(output_path, PATH_MAX);
    k_key_path = strndup_user(key_path, PATH_MAX);

    printk(KERN_INFO "Input file path: %s\n", k_input_path);
    printk(KERN_INFO "Output file path: %s\n", k_output_path);
    printk(KERN_INFO "Key file path: %s\n", k_key_path);

    if (IS_ERR(k_input_path) || IS_ERR(k_output_path) || IS_ERR(k_key_path)) {
        ret = -EFAULT;
        printk(KERN_ERR "Failed to copy paths from user space\n");
        goto out_free;
    }

    ret = process_file(k_input_path, k_output_path, k_key_path, num_threads);

out_free:
    kfree(k_input_path);
    kfree(k_output_path);
    kfree(k_key_path);
    return ret;
}

// ---------------- PROYECTO ----------------
SYSCALL_DEFINE1(get_memory_info, char __user *, buffer) {
    struct sysinfo si;
    char *kbuffer;
    int ret = 0;

    kbuffer = kmalloc(256, GFP_KERNEL);
    if (!kbuffer) return -ENOMEM;

    si_meminfo(&si);  // Obtener información de memoria

    unsigned long free_mem = si.freeram * si.mem_unit;
    unsigned long used_mem = (si.totalram - si.freeram) * si.mem_unit;
    unsigned long cached_mem = global_node_page_state(NR_FILE_PAGES) * PAGE_SIZE;

    snprintf(kbuffer, 256, "Free Memory: %lu\nUsed Memory: %lu\nCached Memory: %lu\n",
             free_mem / 1024, used_mem / 1024, cached_mem / 1024);

    // Copiar los datos al espacio de usuario
    if (copy_to_user(buffer, kbuffer, 256)) ret = -EFAULT;

    kfree(kbuffer);
    return ret;
}

SYSCALL_DEFINE1(get_swap_info, char __user *, buffer) {
    char *kbuffer;
    struct file *file;
    loff_t pos = 0;
    char *meminfo_buffer;
    unsigned long total_swap = 0, free_swap = 0;
    int bytes_read, ret = 0;

    kbuffer = kmalloc(256, GFP_KERNEL);
    if (!kbuffer) return -ENOMEM;

    meminfo_buffer = kmalloc(MEMINFO_BUFFER_SIZE, GFP_KERNEL);
    if (!meminfo_buffer) {
        kfree(kbuffer);
        return -ENOMEM;
    }

    file = filp_open("/proc/meminfo", O_RDONLY, 0);
    if (IS_ERR(file)) {
        kfree(kbuffer);
        kfree(meminfo_buffer);
        return PTR_ERR(file);
    }

    bytes_read = kernel_read(file, meminfo_buffer, MEMINFO_BUFFER_SIZE, &pos);
    if (bytes_read < 0) {
        kfree(kbuffer);
        kfree(meminfo_buffer);
        filp_close(file, NULL);
        return bytes_read;
    }

    sscanf(strstr(meminfo_buffer, "SwapTotal:"), "SwapTotal: %lu", &total_swap);
    sscanf(strstr(meminfo_buffer, "SwapFree:"), "SwapFree: %lu", &free_swap);

    filp_close(file, NULL);
    kfree(meminfo_buffer);

    snprintf(kbuffer, 256, "Total Swap: %lu\nUsed Swap: %lu\nFree Swap: %lu\n",
             total_swap, total_swap - free_swap, free_swap);

    // Copiar los datos al espacio de usuario
    if (copy_to_user(buffer, kbuffer, 256)) ret = -EFAULT;

    kfree(kbuffer);
    return ret;
}

SYSCALL_DEFINE1(get_page_faults, char __user *, buffer) {
    char *kbuffer;
    int ret = 0;
    unsigned long minor_faults = 0;
    unsigned long major_faults = 0;
    struct task_struct *task;

    kbuffer = kmalloc(256, GFP_KERNEL);
    if (!kbuffer) return -ENOMEM;

    // Recorre todos los procesos para sumar los fallos de página menores y mayores
    for_each_process(task) {
        if (task->mm) {
            minor_faults += task->min_flt;
            major_faults += task->maj_flt;
        }
    }

    snprintf(kbuffer, 256, "Minor Page Faults: %lu\nMajor Page Faults: %lu\n",
             minor_faults, major_faults);

    // Copiar los datos al espacio de usuario
    if (copy_to_user(buffer, kbuffer, 256)) ret = -EFAULT;

    kfree(kbuffer);
    return ret;
}

SYSCALL_DEFINE1(get_active_inactive_pages, char __user *, buffer) {
    char *kbuffer;
    int ret = 0;
    unsigned long active = 0;
    unsigned long inactive = 0;

    kbuffer = kmalloc(256, GFP_KERNEL);
    if (!kbuffer) return -ENOMEM;

    // Obtener la información de páginas activas e inactivas
    active = global_node_page_state(NR_ACTIVE_ANON) + global_node_page_state(NR_ACTIVE_FILE);
    inactive = global_node_page_state(NR_INACTIVE_ANON) + global_node_page_state(NR_INACTIVE_FILE);

    snprintf(kbuffer, 256, "Active Pages: %lu\nInactive Pages: %lu\n",
             active * (PAGE_SIZE / 1024), inactive * (PAGE_SIZE / 1024));

    // Copiar los datos al espacio de usuario
    if (copy_to_user(buffer, kbuffer, 256)) ret = -EFAULT;

    kfree(kbuffer);
    return ret;
}

SYSCALL_DEFINE1(get_top_memory_processes, char __user *, buffer) {
    struct task_struct *task;
    struct process_mem_info top_processes[5];
    char *kbuffer;
    int i, ret = 0;

    memset(top_processes, 0, sizeof(top_processes));

    kbuffer = kmalloc(1024, GFP_KERNEL);
    if (!kbuffer) return -ENOMEM;

    rcu_read_lock();
    for_each_process(task) {
        if (task->mm) {
            unsigned long mem = get_mm_rss(task->mm) << PAGE_SHIFT;

            for (i = 0; i < 5; i++) {
                if (mem > top_processes[i].mem_usage) {
                    if (i < 4) {
                        memmove(&top_processes[i + 1], &top_processes[i], (4 - i) * sizeof(struct process_mem_info));
                    }
                    top_processes[i].pid = task->pid;
                    strncpy(top_processes[i].comm, task->comm, sizeof(top_processes[i].comm) - 1);
                    top_processes[i].comm[sizeof(top_processes[i].comm) - 1] = '\0';
                    top_processes[i].mem_usage = mem;
                    break;
                }
            }
        }
    }
    rcu_read_unlock();

    snprintf(kbuffer, 1024, "Top 5 Memory Consuming Processes:\n");
    for (i = 0; i < 5; i++) {
        if (top_processes[i].pid != 0) {
            snprintf(kbuffer + strlen(kbuffer), 1024 - strlen(kbuffer), "PID: %d, Memory: %lu, Command: %s\n",
                     top_processes[i].pid, top_processes[i].mem_usage >> 10, top_processes[i].comm);
        }
    }

    if (copy_to_user(buffer, kbuffer, 1024)) ret = -EFAULT;

    kfree(kbuffer);
    return ret;
}