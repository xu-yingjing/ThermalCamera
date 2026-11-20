#include "tcam.h"
#include <stddef.h>

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
