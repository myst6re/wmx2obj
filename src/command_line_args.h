#ifndef COMMAND_LINE_ARGS_H
#define COMMAND_LINE_ARGS_H

struct config;

enum action
{
    ACTION_NONE,
    ACTION_HELP
};

enum action parse_args(int argc, char **argv, struct config *cfg);
void print_help(const char *arg0);

#endif // COMMAND_LINE_ARGS_H
