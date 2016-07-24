#include "config.h"

#include <stdio.h>
#include <stdlib.h>

// 0 and SEGMENT_CNT - 1 used for array indexing reasons.
static const int SEGMENT_LB = 0;
static const int SEGMENT_UB = SEGMENT_CNT - 1;

struct config create_config(void)
{
    return (struct config)
    {
        // Default to conversion of the whole map.
        // Allow the user to leave out the segment arguments.
        .start_segment_idx = SEGMENT_LB,
        .end_segment_idx   = SEGMENT_UB,
        .input_path  = NULL,
        .output_path = NULL
    };
}

void destroy_config(struct config *cfg)
{
    if (cfg->input_path  != NULL) free(cfg->input_path);
    if (cfg->output_path != NULL) free(cfg->output_path);
}

bool is_config_valid(const struct config *cfg)
{
    const bool in_valid  = cfg->input_path  != NULL;
    const bool out_valid = cfg->output_path != NULL;
    const bool seg_range_valid = cfg->start_segment_idx >= SEGMENT_LB
                              && cfg->end_segment_idx   <= SEGMENT_UB
                              && cfg->start_segment_idx <= cfg->end_segment_idx;

    if (!in_valid)
        fprintf(stderr, "Erroneous input file path.\n");
    if (!out_valid)
        fprintf(stderr, "Erroneous output file path.\n");
    if (!seg_range_valid)
        fprintf(stderr, "Invalid segment range specified.\n");

    return in_valid && out_valid && seg_range_valid;
}
