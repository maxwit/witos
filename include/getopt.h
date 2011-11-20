#pragma once

extern char *optarg;
extern int optind;
extern int optopt;
extern int opterr;

void getopt_init(void);

int getopt(int argc, char *argv[], const char *opt_str);
