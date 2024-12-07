#ifndef _SYSCALLS_USAC_H
#define _SYSCALLS_USAC_H

#include <linux/kernel.h>
#include <linux/syscalls.h>

asmlinkage long sys_my_encrypt(const char __user *input_file, const char __user>
asmlinkage long sys_my_decrypt(const char __user *input_file, const char __user>

#endif /* _SYSCALLS_USAC_H */