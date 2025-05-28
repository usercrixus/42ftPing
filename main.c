// main.c
#include "srcs/ping.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void print_usage(const char *prog)
{
    printf("Usage: %s [-v] [-?] <destination>\n", prog);
    printf("  -v    Verbose output\n");
    printf("  -?    Show this help message\n");
}

int main(int argc, char *argv[])
{
    int opt;
    int verbose = 0;
    while ((opt = getopt(argc, argv, "v?")) != -1)
    {
        switch (opt)
        {
        case 'v':
            verbose = 1;
            break;
        case '?':
            print_usage(argv[0]);
            return (0);
        }
    }
    if (optind + 1 != argc)
    {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    const char *dest = argv[optind];
    return ping(dest, verbose);
}
