// wmx2obj - Push Final Fantasy 8 world map geometry to Wavefront OBJ format
// Copyright (C) 2015-2016 Aleksanteri Hirvonen

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "command_line_args.h"
#include "config.h"

static bool convert_to_obj(const struct config *cfg);
static bool handle_segments(const struct config *cfg, FILE *in_fp, FILE *out_fp);
static void handle_segment(long long *max_vtx_ptr, int pos_idx,
                           const unsigned char *buf, FILE *out_fp);
static void handle_block(long long *max_vtx_ptr, int pos_idx, long long x, long long z,
                         const unsigned char *buf, FILE *out_fp);
static void handle_polygon(long long curr_vtx_idx, long long *max_vtx_ptr,
                           const unsigned char *buf, FILE *out_fp);
static void handle_vertex(int bound, long long x, long long z,
                          const unsigned char *buf, FILE *out_fp);

int main(int argc, char **argv)
{
    bool err = false;
    struct config cfg = create_config();

    const enum action act = parse_args(argc, argv, &cfg);
    if (act == ACTION_HELP)
    {
        print_help(argv[0]);
        goto exit_program;
    }

    err = !is_config_valid(&cfg);
    if (err)
        goto exit_program;

    err = !convert_to_obj(&cfg);

exit_program:
    destroy_config(&cfg);

    if (err)
        fprintf(stderr, "Errors occurred during execution.\n");

    return err ? 1 : 0;
}

static bool convert_to_obj(const struct config *cfg)
{
    bool err = false;
    FILE *in_fp  = NULL;
    FILE *out_fp = NULL;

    err = (in_fp = fopen(cfg->input_path, "rb")) == NULL;
    if (err)
    {
        fprintf(stderr, "Error opening input file.\n");
        goto exit_conversion;
    }

    err = (out_fp = fopen(cfg->output_path, "w")) == NULL;
    if (err)
    {
        fprintf(stderr, "Error opening output file.\n");
        goto exit_conversion;
    }

    err = !handle_segments(cfg, in_fp, out_fp);

exit_conversion:
    if (in_fp  != NULL) fclose(in_fp);
    if (out_fp != NULL) fclose(out_fp);

    return !err;
}

static bool handle_segments(const struct config *cfg, FILE *in_fp, FILE *out_fp)
{
    static const long SEGMENT_BYTE_CNT = 0x9000;

    bool err = false;
    unsigned char *buf = NULL;
    long long max_vtx_idx = 1;

    err = fseek(in_fp, cfg->start_segment_idx * SEGMENT_BYTE_CNT, SEEK_SET) != 0;
    if (err)
        fprintf(stderr, "Error locating starting segment in file.\n");

    for (int i = cfg->start_segment_idx; i <= cfg->end_segment_idx; ++i)
    {
        err = (buf = malloc(SEGMENT_BYTE_CNT)) == NULL;
        if (err)
        {
            fprintf(stderr, "Error allocating memory for buffer.\n");
            break;
        }

        err = fread(buf, SEGMENT_BYTE_CNT, 1, in_fp) != 1;
        if (err)
        {
            fprintf(stderr, "Error reading file to buffer.\n");
            break;
        }

        handle_segment(&max_vtx_idx, i - cfg->start_segment_idx, buf, out_fp);

        free(buf);
        buf = NULL;
    }

    if (buf != NULL)
        free(buf);

    return !err;
}

static void handle_segment(long long *max_vtx_ptr, int pos_idx,
                           const unsigned char *buf, FILE *out_fp)
{
    static const int SEGMENT_DIMENSIONS = 8192;
    static const int SEGMENTS_PER_ROW   =   32;
    // There are 4 unknown bytes at the beginning of each segment header.
    static const int SEGMENT_HEADER_SKIP_BYTE_CNT =  4;
    static const int SEGMENT_BLOCK_CNT            = 16;
    static const int BLOCK_OFFSET_BYTE_CNT        =  4;

    const long long x = SEGMENT_DIMENSIONS * (pos_idx % (long long) SEGMENTS_PER_ROW);
    const long long z = SEGMENT_DIMENSIONS * (pos_idx / (long long) SEGMENTS_PER_ROW);

    int blk_offset_loc = SEGMENT_HEADER_SKIP_BYTE_CNT;

    for (int i = 0; i < SEGMENT_BLOCK_CNT; ++i)
    {
        const unsigned char b1 = buf[blk_offset_loc];
        const unsigned char b2 = buf[blk_offset_loc + 1];
        const unsigned char b3 = buf[blk_offset_loc + 2];
        const unsigned char b4 = buf[blk_offset_loc + 3];

        // UL suffix used for correct implicit promotions to take place.
        const unsigned long blk_offset = (b1 << 0UL) | (b2 << 8UL) | (b3 << 16UL) | (b4 << 24UL);

        handle_block(max_vtx_ptr, i, x, z, &buf[blk_offset], out_fp);

        blk_offset_loc += BLOCK_OFFSET_BYTE_CNT;
    }
}

static void handle_block(long long *max_vtx_ptr, int pos_idx, long long x, long long z,
                         const unsigned char *buf, FILE *out_fp)
{
    static const int BLOCK_DIMENSIONS = 2048;
    static const int BLOCKS_PER_ROW   =    4;
    static const int BLOCK_HEADER_BYTE_CNT =  4;
    static const int POLYGON_BYTE_CNT      = 16;
    static const int VERTEX_BYTE_CNT       =  8;

    x += BLOCK_DIMENSIONS * (pos_idx % (long long) BLOCKS_PER_ROW);
    z += BLOCK_DIMENSIONS * (pos_idx / (long long) BLOCKS_PER_ROW);

    const int pgn_cnt = (int) buf[0];
    const int vtx_cnt = (int) buf[1];

    const long long curr_vtx_idx = *max_vtx_ptr;

    int offset = BLOCK_HEADER_BYTE_CNT;

    for (int i = 0; i < pgn_cnt; ++i)
    {
        handle_polygon(curr_vtx_idx, max_vtx_ptr, &buf[offset], out_fp);
        offset += POLYGON_BYTE_CNT;
    }

    ++(*max_vtx_ptr);

    for (int i = 0; i < vtx_cnt; ++i)
    {
        handle_vertex(BLOCK_DIMENSIONS, x, z, &buf[offset], out_fp);
        offset += VERTEX_BYTE_CNT;
    }
}

static void handle_polygon(long long curr_vtx_idx, long long *max_vtx_ptr,
                           const unsigned char *buf, FILE *out_fp)
{
    static const int POLYGON_VERTEX_CNT = 3;

    fprintf(out_fp, "f");

    for (int i = 0; i < POLYGON_VERTEX_CNT; ++i)
    {
        const long long vtx_idx = curr_vtx_idx + (long long) buf[i];

        fprintf(out_fp, " %lld", vtx_idx);

        if (vtx_idx > *max_vtx_ptr)
            *max_vtx_ptr = vtx_idx;
    }

    fprintf(out_fp, "\n");
}

static void handle_vertex(int bound, long long x, long long z,
                          const unsigned char *buf, FILE *out_fp)
{
    const unsigned int bx = (buf[0] << 0U) | (buf[1] << 8U);
    const unsigned int by = (buf[2] << 0U) | (buf[3] << 8U);
    const unsigned int bz = (buf[4] << 0U) | (buf[5] << 8U);

    const unsigned int dx = bx > (unsigned int) bound ? ~(bx - 1) & 0xFFFF : bx;
    const unsigned int dy = by > (unsigned int) bound ? ~(by - 1) & 0xFFFF : by;
    const unsigned int dz = bz > (unsigned int) bound ? ~(bz - 1) & 0xFFFF : bz;

    fprintf(out_fp, "v");
    fprintf(out_fp, " %.3Lf", (x + (long long) dx) / 1000.0L);
    fprintf(out_fp, " %.3Lf",      (long long) dy  / 1000.0L);
    fprintf(out_fp, " %.3Lf", (z + (long long) dz) / 1000.0L);
    fprintf(out_fp, "\n");
}
