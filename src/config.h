#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

#define SEGMENT_CNT 835

struct config
{
    int start_segment_idx;
    int end_segment_idx;

    char *input_path;
    char *output_path;
};

struct config create_config(void);
void destroy_config(struct config *cfg);
bool is_config_valid(const struct config *cfg);

#endif // CONFIG_H
