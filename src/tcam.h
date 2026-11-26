#ifndef TCAM_H
#define TCAM_H

#include <stdint.h>
#include <stdbool.h>

#define UTIL_U32_TO_FLOAT(v) ((union {uint32_t u32; float f;}){.u32 = (uint32_t)(v)}.f)

#define UTIL_MAP_U16_RANGE(v, src_min, src_max, dst_min, dst_max) ((uint16_t)((uint32_t)((v) - (src_min)) * ((dst_max) - (dst_min)) / ((src_max) - (src_min)) + (dst_min)))

#define UTIL_READ_U8_LE(buffer, offset)  ((uint8_t)((const uint8_t *)(buffer))[(offset)])
#define UTIL_READ_U16_LE(buffer, offset) ((uint16_t)((uint16_t)((const uint8_t *)(buffer))[(offset)]) | \
                                                    ((uint16_t)((const uint8_t *)(buffer))[(offset) + 1U] << 8U))
#define UTIL_READ_U32_LE(buffer, offset) ((uint32_t)((uint32_t)((const uint8_t *)(buffer))[(offset)]) |             \
                                                    ((uint32_t)((const uint8_t *)(buffer))[(offset) + 1U] << 8U) |  \
                                                    ((uint32_t)((const uint8_t *)(buffer))[(offset) + 2U] << 16U) | \
                                                    ((uint32_t)((const uint8_t *)(buffer))[(offset) + 3U] << 24U))

typedef enum {
    TCAM_OK = 0U,
    TCAM_ERROR,
} tcam_status_t;

typedef enum {
    TCAM_MODEL_UNKNOWN = 0U,
    TCAM_MODEL_TM30,
    TCAM_MODEL_MINI212,
    TCAM_MODEL_COUNT,
} tcam_model_t;

typedef struct {
    struct {
        tcam_status_t (*send)(const uint8_t *data, uint32_t len, uint32_t timeout_ms);
        tcam_status_t (*recv)(uint8_t *data, uint32_t len, uint32_t timeout_ms);
        tcam_status_t (*flush_rx)(uint32_t idle_ms, uint32_t timeout_ms);
    } cmd;
    struct {
        tcam_status_t (*configure)(uint32_t frame_size);
        tcam_status_t (*start)(void);
        tcam_status_t (*acquire_frame)(void **frame, uint32_t timeout_ms);
        tcam_status_t (*release_frame)(void);
    } stream;
} tcam_ops_t;

typedef struct {
    tcam_model_t model;
    const tcam_ops_t *ops;
} tcam_t;

typedef struct {
    uint16_t x;
    uint16_t y;
    int16_t temp_mult10;
} tcam_stream_frame_point_t;

typedef struct {
    const uint8_t *data;
    uint16_t width;
    uint16_t height;
    uint8_t bytes_per_item;
    uint32_t stride_item;
} tcam_stream_frame_matrix_t;

typedef struct {
    bool freezed;
    tcam_stream_frame_point_t max_point;
    tcam_stream_frame_point_t min_point;
    int16_t avg_temp_mult10;
    tcam_stream_frame_matrix_t mt_matrix;
    tcam_stream_frame_matrix_t yuv_matrix;
} tcam_stream_frame_info_t;

tcam_status_t tcam_init(tcam_t *tcam, tcam_model_t model, const tcam_ops_t *ops);

tcam_status_t tcam_device_probe(tcam_t *tcam);

tcam_status_t tcam_stream_start(const tcam_t *tcam);

tcam_status_t tcam_stream_frame_acquire(const tcam_t *tcam, void **frame, uint32_t timeout_ms);

tcam_status_t tcam_stream_frame_release(const tcam_t *tcam);

tcam_status_t tcam_stream_frame_decode(const tcam_t *tcam, const void *frame, tcam_stream_frame_info_t *frame_info);

#endif
