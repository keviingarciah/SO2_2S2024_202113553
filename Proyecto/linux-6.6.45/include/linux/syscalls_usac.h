#ifndef _SYSCALLS_USAC_H
#define _SYSCALLS_USAC_H

#include <linux/kernel.h>
#include <linux/syscalls.h>

asmlinkage long sys_my_encrypt(const char __user *input_file, const char __user *output_file, int num_threads, const char __user *key_file);
asmlinkage long sys_my_decrypt(const char __user *input_file, const char __user *output_file, int num_threads, const char __user *key_file);

asmlinkage long sys_get_memory_info(char __user *buffer);
asmlinkage long sys_get_swap_info(char __user *buffer);
asmlinkage long sys_get_page_faults(char __user *buffer);
asmlinkage long sys_get_active_inactive_pages(char __user *buffer);
asmlinkage long sys_get_top_memory_processes(char __user *buffer);

#endif /* _SYSCALLS_USAC_H */
