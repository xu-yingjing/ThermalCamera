#include "tcam.h"
#include <stddef.h>
#include "tcam_tm30.h"
#include "tcam_mini212.h"

static tcam_status_t tcam_object_check(const tcam_t *tcam)
{
    if ((tcam == NULL) ||
        (tcam->model == TCAM_MODEL_UNKNOWN) ||
        ((uint32_t)tcam->model >= (uint32_t)TCAM_MODEL_COUNT) ||
        (tcam->ops == NULL)) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_init(tcam_t *tcam,
                        tcam_model_t model,
                        const tcam_ops_t *ops)
{
    if ((tcam == NULL) ||
        (model == TCAM_MODEL_UNKNOWN) ||
        ((uint32_t)model >= (uint32_t)TCAM_MODEL_COUNT) ||
        (ops == NULL) ||
        (ops->cmd.send == NULL) ||
        (ops->cmd.recv == NULL) ||
        (ops->cmd.flush_rx == NULL) ||
        (ops->stream.configure == NULL) ||
        (ops->stream.start == NULL) ||
        (ops->stream.acquire_frame == NULL) ||
        (ops->stream.release_frame == NULL)) {
        return TCAM_ERROR;
    }

    tcam->model = model;
    tcam->ops = ops;

    return TCAM_OK;
}

tcam_status_t tcam_device_probe(tcam_t *tcam)
{
    tcam_status_t ret;

    if (tcam_object_check(tcam) != TCAM_OK) {
        return TCAM_ERROR;
    }

    switch (tcam->model) {
        case TCAM_MODEL_TM30: ret = tcam_tm30_probe(tcam); break;
        case TCAM_MODEL_MINI212: ret = tcam_mini212_probe(tcam); break;
        default: ret = TCAM_ERROR; break;
    }

    if (ret != TCAM_OK) {
        return ret;
    }

    return TCAM_OK;
}

tcam_status_t tcam_stream_start(const tcam_t *tcam)
{
    uint32_t frame_size;

    if (tcam_object_check(tcam) != TCAM_OK) {
        return TCAM_ERROR;
    }

    switch (tcam->model) {
        case TCAM_MODEL_TM30: frame_size = tcam_tm30_stream_frame_get_size(); break;
        case TCAM_MODEL_MINI212: frame_size = tcam_mini212_stream_frame_get_size(); break;
        default: return TCAM_ERROR;
    }

    if (tcam->ops->stream.configure(frame_size) != TCAM_OK) {
        return TCAM_ERROR;
    }

    if (tcam->ops->stream.start() != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_stream_frame_acquire(const tcam_t *tcam, void **frame, uint32_t timeout_ms)
{
    if ((tcam_object_check(tcam) != TCAM_OK) ||
        (frame == NULL)) {
        return TCAM_ERROR;
    }

    if (tcam->ops->stream.acquire_frame(frame, timeout_ms) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_stream_frame_release(const tcam_t *tcam)
{
    if (tcam_object_check(tcam) != TCAM_OK) {
        return TCAM_ERROR;
    }

    if (tcam->ops->stream.release_frame() != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_stream_frame_decode(const tcam_t *tcam, const void *frame, tcam_stream_frame_info_t *frame_info)
{
    tcam_status_t ret;

    if (tcam_object_check(tcam) != TCAM_OK) {
        return TCAM_ERROR;
    }
    
    switch (tcam->model) {
        case TCAM_MODEL_TM30: ret = tcam_tm30_stream_frame_decode(tcam, frame, frame_info); break;
        case TCAM_MODEL_MINI212: ret = tcam_mini212_stream_frame_decode(tcam, frame, frame_info); break;
        default: ret = TCAM_ERROR; break;
    }

    if (ret != TCAM_OK) {
        return ret;
    }

    return TCAM_OK;
}
