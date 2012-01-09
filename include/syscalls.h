#pragma once

#define __user

long sys_getcwd(char __user *buf, unsigned long size);

long sys_chdir(const char __user *filename);
