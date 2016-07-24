#include "command_line_args.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"

static char *copy_string(const char *str);
static int parse_segment_idx(const char *arg);

enum action parse_args(int argc, char **argv, struct config *cfg)
{
    if (argc < 2)
    {
        fprintf(stderr, "Too few arguments given.\n");
        return ACTION_NONE;
    }

    const char *help_arg = argv[1];
    if (strcmp(help_arg, "-h") == 0)
        return ACTION_HELP;

    for (int i = 1; i < argc - 1; i += 2)
    {
        const char *curr_arg = argv[i];
        const char *next_arg = argv[i + 1];

        if (strcmp(curr_arg, "-f") == 0)
            cfg->input_path = copy_string(next_arg);
        else
        if (strcmp(curr_arg, "-o") == 0)
            cfg->output_path = copy_string(next_arg);
        else
        if (strcmp(curr_arg, "-s") == 0)
            cfg->start_segment_idx = parse_segment_idx(next_arg);
        else
        if (strcmp(curr_arg, "-e") == 0)
            cfg->end_segment_idx = parse_segment_idx(next_arg);
    }

    return ACTION_NONE;
}

static char *copy_string(const char *str)
{
    char *destination = malloc(strlen(str) + 1);
    if (destination == NULL)
    {
        fprintf(stderr, "Error allocating memory for string.\n");
        return NULL;
    }

    strcpy(destination, str);
    return destination;
}

static int parse_segment_idx(const char *arg)
{
    // Return arg - 1 for array indexing reasons.
    return strtol(arg, NULL, 10) - 1;
}

void print_help(const char *arg0)
{
    fprintf(stderr, "Usage: %s [OPTIONS]\n", arg0);
    fprintf(stderr, "  -h Display this help message and exit.\n");
    fprintf(stderr, "  -f INPUT_FILE  Path to the original wmx.obj.\n");
    fprintf(stderr, "  -o OUTPUT_FILE Destination path for the Wavefront OBJ model.\n");
    fprintf(stderr, "  -s [1, 835]    First segment to be converted. (Default = 1)\n");
    fprintf(stderr, "  -e [s, 835]    Last segment to be converted. (Default = 835)\n");
}
