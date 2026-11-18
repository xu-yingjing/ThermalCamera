#include "tcam_tm30.h"
#include <stddef.h>
#include <string.h>

#define CMD_HEAD_LEN        (1U)
#define CMD_SIZE_LEN        (1U)
#define CMD_DEVICE_ADDR_LEN (1U)
#define CMD_CLASS_LEN       (1U)
#define CMD_SUB_LEN         (1U)
#define CMD_FLAG_LEN        (1U)
#define CMD_DATA_MAX_LEN    (48U)
#define CMD_CHECKSUM_LEN    (1U)
#define CMD_TAIL_LEN        (1U)
#define CMD_LEN(data_len)   (CMD_HEAD_LEN +        \
                             CMD_SIZE_LEN +        \
                             CMD_DEVICE_ADDR_LEN + \
                             CMD_CLASS_LEN +       \
                             CMD_SUB_LEN +         \
                             CMD_FLAG_LEN +        \
                             (data_len) +          \
                             CMD_CHECKSUM_LEN +    \
                             CMD_TAIL_LEN)

#define CMD_HEAD_IDX(data_len)        (0U)
#define CMD_SIZE_IDX(data_len)        (CMD_HEAD_IDX((data_len)) + CMD_HEAD_LEN)
#define CMD_DEVICE_ADDR_IDX(data_len) (CMD_SIZE_IDX((data_len)) + CMD_SIZE_LEN)
#define CMD_CLASS_IDX(data_len)       (CMD_DEVICE_ADDR_IDX((data_len)) + CMD_DEVICE_ADDR_LEN)
#define CMD_SUB_IDX(data_len)         (CMD_CLASS_IDX((data_len)) + CMD_CLASS_LEN)
#define CMD_FLAG_IDX(data_len)        (CMD_SUB_IDX((data_len)) + CMD_SUB_LEN)
#define CMD_DATA_IDX(data_len)        (CMD_FLAG_IDX((data_len)) + CMD_FLAG_LEN)
#define CMD_CHECKSUM_IDX(data_len)    (CMD_DATA_IDX((data_len)) + (data_len))
#define CMD_TAIL_IDX(data_len)        (CMD_CHECKSUM_IDX((data_len)) + CMD_CHECKSUM_LEN)

#define CMD_HEAD           (0xF0U)
#define CMD_SIZE(data_len) (CMD_DEVICE_ADDR_LEN + CMD_CLASS_LEN + CMD_SUB_LEN + CMD_FLAG_LEN + (data_len))
#define CMD_DEVICE_ADDR    (0x36U)
#define CMD_FLAG_WRITE     (0x00U)
#define CMD_FLAG_READ      (0x01U)
#define CMD_FLAG_RESPONSE  (0x03U)
#define CMD_TAIL           (0xFFU)

#define CMD_MAX_LEN CMD_LEN(CMD_DATA_MAX_LEN)

#define CMD_FLUSH_IDLE_MS (2U)

#define CMD_SEND_TIMEOUT_MS  (20U)
#define CMD_RECV_TIMEOUT_MS  (200U)
#define CMD_FLUSH_TIMEOUT_MS (20U)

#define STREAM_FRAME_WIDTH     (240U)
#define STREAM_FRAME_HEIGHT    (364U)
#define STREAM_FRAME_LINE_SIZE (STREAM_FRAME_WIDTH * 2U)
#define STREAM_FRAME_SIZE      (STREAM_FRAME_LINE_SIZE * STREAM_FRAME_HEIGHT)

#define STREAM_FRAME_PARAM_OFFSET         (0U)
#define STREAM_FRAME_PARAM_WIDTH          (192U)
#define STREAM_FRAME_PARAM_HEIGHT         (28U)
#define STREAM_FRAME_PARAM_BYTES_PER_ITEM (1U)
#define STREAM_FRAME_PARAM_STRIDE_ITEM    (STREAM_FRAME_LINE_SIZE / STREAM_FRAME_PARAM_BYTES_PER_ITEM)

#define STREAM_FRAME_MT_OFFSET         (STREAM_FRAME_PARAM_OFFSET + STREAM_FRAME_LINE_SIZE * STREAM_FRAME_PARAM_HEIGHT)
#define STREAM_FRAME_MT_WIDTH          (96U)
#define STREAM_FRAME_MT_HEIGHT         (96U)
#define STREAM_FRAME_MT_BYTES_PER_ITEM (2U)
#define STREAM_FRAME_MT_STRIDE_ITEM    (STREAM_FRAME_LINE_SIZE / STREAM_FRAME_MT_BYTES_PER_ITEM)

#define STREAM_FRAME_YUV_OFFSET         (STREAM_FRAME_MT_OFFSET + STREAM_FRAME_LINE_SIZE * STREAM_FRAME_MT_HEIGHT)
#define STREAM_FRAME_YUV_WIDTH          (240U)
#define STREAM_FRAME_YUV_HEIGHT         (240U)
#define STREAM_FRAME_YUV_BYTES_PER_ITEM (2U)
#define STREAM_FRAME_YUV_STRIDE_ITEM    (STREAM_FRAME_LINE_SIZE / STREAM_FRAME_YUV_BYTES_PER_ITEM)

#define STREAM_FRAME_PARAM_HEAD_OFFSET     (0U)
#define STREAM_FRAME_PARAM_FREEZED_OFFSET  (32U)
#define STREAM_FRAME_PARAM_MIN_TEMP_OFFSET (144U)
#define STREAM_FRAME_PARAM_MAX_TEMP_OFFSET (148U)
#define STREAM_FRAME_PARAM_AVG_TEMP_OFFSET (152U)
#define STREAM_FRAME_PARAM_MAX_X_OFFSET    (156U)
#define STREAM_FRAME_PARAM_MAX_Y_OFFSET    (160U)
#define STREAM_FRAME_PARAM_MIN_X_OFFSET    (164U)
#define STREAM_FRAME_PARAM_MIN_Y_OFFSET    (168U)

#define STREAM_FRAME_PARAM_HEAD (0x70827789U)

typedef struct {
    uint32_t head;
    uint32_t freezed;
    uint32_t min_temp;
    uint32_t max_temp;
    uint32_t avg_temp;
    uint32_t max_x;
    uint32_t max_y;
    uint32_t min_x;
    uint32_t min_y;
} stream_frame_param_t;

static tcam_status_t cmd_encode(uint8_t class, uint8_t sub, uint8_t flag, const uint8_t *data, uint8_t data_len, uint8_t *cmd, uint32_t cmd_size, uint32_t *cmd_len)
{
    uint32_t i;

    if (((flag != CMD_FLAG_WRITE) &&
         (flag != CMD_FLAG_READ)) ||
        (data_len > CMD_DATA_MAX_LEN)) {
        return TCAM_ERROR;
    }

    if (cmd_size < CMD_LEN(data_len)) {
        return TCAM_ERROR;
    }

    cmd[CMD_HEAD_IDX(data_len)] = CMD_HEAD;

    cmd[CMD_SIZE_IDX(data_len)] = CMD_SIZE(data_len);

    cmd[CMD_DEVICE_ADDR_IDX(data_len)] = CMD_DEVICE_ADDR;

    cmd[CMD_CLASS_IDX(data_len)] = class;

    cmd[CMD_SUB_IDX(data_len)] = sub;

    cmd[CMD_FLAG_IDX(data_len)] = flag;

    for (i = 0U; i < data_len; i++) {
        cmd[CMD_DATA_IDX(data_len) + i] = data[i];
    }

    cmd[CMD_CHECKSUM_IDX(data_len)] = 0U;
    for (i = CMD_DEVICE_ADDR_IDX(data_len); i < CMD_CHECKSUM_IDX(data_len); i++) {
        cmd[CMD_CHECKSUM_IDX(data_len)] += cmd[i];
    }

    cmd[CMD_TAIL_IDX(data_len)] = CMD_TAIL;

    *cmd_len = CMD_LEN(data_len);

    return TCAM_OK;
}

static tcam_status_t cmd_decode(const uint8_t *cmd, uint32_t cmd_len, uint8_t *class, uint8_t *sub, uint8_t *flag, const uint8_t **data, uint8_t *data_len)
{
    uint8_t size;
    uint8_t checksum;
    uint8_t i;

    if ((cmd_len < CMD_LEN(0U)) ||
        (cmd_len > CMD_MAX_LEN)) {
        return TCAM_ERROR;
    }

    if (cmd[CMD_HEAD_IDX(0U)] != CMD_HEAD) {
        return TCAM_ERROR;
    }

    size = cmd[CMD_SIZE_IDX(0U)];
    if ((size < CMD_SIZE(0U)) ||
        (size > CMD_SIZE(CMD_DATA_MAX_LEN))) {
        return TCAM_ERROR;
    }

    if (cmd_len != CMD_LEN(size - CMD_SIZE(0U))) {
        return TCAM_ERROR;
    }

    if (cmd[CMD_DEVICE_ADDR_IDX(size - CMD_SIZE(0U))] != CMD_DEVICE_ADDR) {
        return TCAM_ERROR;
    }

    if (cmd[CMD_TAIL_IDX(size - CMD_SIZE(0U))] != CMD_TAIL) {
        return TCAM_ERROR;
    }

    checksum = 0U;
    for (i = CMD_DEVICE_ADDR_IDX(size - CMD_SIZE(0U)); i < CMD_CHECKSUM_IDX(size - CMD_SIZE(0U)); i++) {
        checksum += cmd[i];
    }
    if (checksum != cmd[CMD_CHECKSUM_IDX(size - CMD_SIZE(0U))]) {
        return TCAM_ERROR;
    }

    *class = cmd[CMD_CLASS_IDX(size - CMD_SIZE(0U))];
    *sub = cmd[CMD_SUB_IDX(size - CMD_SIZE(0U))];
    *flag = cmd[CMD_FLAG_IDX(size - CMD_SIZE(0U))];
    *data = &cmd[CMD_DATA_IDX(size - CMD_SIZE(0U))];
    *data_len = size - CMD_SIZE(0U);

    return TCAM_OK;
}

static tcam_status_t cmd_transfer(const tcam_t *tcam, uint8_t class, uint8_t sub, uint8_t flag, const uint8_t *send_data, uint8_t send_data_len, uint8_t *recv_data, uint8_t recv_data_len)
{
    uint8_t cmd[CMD_MAX_LEN];
    uint32_t cmd_len;
    uint8_t recv_class;
    uint8_t recv_sub;
    uint8_t recv_flag;
    const uint8_t *data;
    uint8_t data_len;

    if (recv_data_len > CMD_DATA_MAX_LEN) {
        return TCAM_ERROR;
    }

    if (cmd_encode(class, sub, flag, send_data, send_data_len, cmd, sizeof(cmd), &cmd_len) != TCAM_OK) {
        return TCAM_ERROR;
    }

    if (tcam->ops->cmd.flush_rx(CMD_FLUSH_IDLE_MS, CMD_FLUSH_TIMEOUT_MS) != TCAM_OK) {
        return TCAM_ERROR;
    }

    if (tcam->ops->cmd.send(cmd, cmd_len, CMD_SEND_TIMEOUT_MS) != TCAM_OK) {
        return TCAM_ERROR;
    }

    if (tcam->ops->cmd.recv(cmd, CMD_LEN(recv_data_len), CMD_RECV_TIMEOUT_MS) != TCAM_OK) {
        return TCAM_ERROR;
    }

    if (cmd_decode(cmd, CMD_LEN(recv_data_len), &recv_class, &recv_sub, &recv_flag, &data, &data_len) != TCAM_OK) {
        return TCAM_ERROR;
    }

    if ((recv_class != class) ||
        (recv_sub != sub) ||
        (recv_flag != CMD_FLAG_RESPONSE) ||
        (data_len != recv_data_len)) {
        return TCAM_ERROR;
    }

    (void)memcpy(recv_data, data, recv_data_len);

    return TCAM_OK;
}

static tcam_status_t cmd_read(const tcam_t *tcam, uint8_t class, uint8_t sub, uint8_t *recv_data, uint8_t recv_data_len)
{
    const uint8_t send_data[1U] = {0x00U};

    return cmd_transfer(tcam, class, sub, CMD_FLAG_READ, send_data, sizeof(send_data), recv_data, recv_data_len);
}

static tcam_status_t cmd_write(const tcam_t *tcam, uint8_t class, uint8_t sub, const uint8_t *send_data, uint8_t send_data_len)
{
    const uint8_t ack_data[1U] = {0x01U};
    uint8_t recv_data[sizeof(ack_data)];

    if (cmd_transfer(tcam, class, sub, CMD_FLAG_WRITE, send_data, send_data_len, recv_data, sizeof(recv_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    if (memcmp(recv_data, ack_data, sizeof(ack_data)) != 0U) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

static tcam_status_t tcam_tm30_object_check(const tcam_t *tcam)
{
    if ((tcam == NULL) ||
        (tcam->model != TCAM_MODEL_TM30) ||
        (tcam->ops == NULL)) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_tm30_probe(const tcam_t *tcam)
{
    tcam_tm30_cid_t cid;

    if (tcam_tm30_object_check(tcam) != TCAM_OK) {
        return TCAM_ERROR;
    }

    if (tcam_tm30_get_cid(tcam, &cid) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_tm30_get_cid(const tcam_t *tcam, tcam_tm30_cid_t *cid)
{
    const uint8_t class = 0x74U;
    const uint8_t sub = 0x02U;
    uint8_t recv_data[5U];

    if ((tcam_tm30_object_check(tcam) != TCAM_OK) ||
        (cid == NULL)) {
        return TCAM_ERROR;
    }

    if (cmd_read(tcam, class, sub, recv_data, sizeof(recv_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    (void)memcpy(cid->data, &recv_data[0U], sizeof(cid->data));

    return TCAM_OK;
}

tcam_status_t tcam_tm30_get_sn(const tcam_t *tcam, tcam_tm30_sn_t *sn)
{
    const uint8_t class = 0x74U;
    const uint8_t sub = 0x11U;
    uint8_t recv_data[9U];

    if ((tcam_tm30_object_check(tcam) != TCAM_OK) ||
        (sn == NULL)) {
        return TCAM_ERROR;
    }

    if (cmd_read(tcam, class, sub, recv_data, sizeof(recv_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    (void)memcpy(sn->data, &recv_data[0U], sizeof(sn->data));

    return TCAM_OK;
}

tcam_status_t tcam_tm30_get_model_number(const tcam_t *tcam, tcam_tm30_model_number_t *model_number)
{
    const uint8_t class = 0x74U;
    const uint8_t sub = 0x20U;
    uint8_t recv_data[48U];

    if ((tcam_tm30_object_check(tcam) != TCAM_OK) ||
        (model_number == NULL)) {
        return TCAM_ERROR;
    }

    if (cmd_read(tcam, class, sub, recv_data, sizeof(recv_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    (void)memcpy(model_number->data, &recv_data[0U], sizeof(model_number->data));

    return TCAM_OK;
}

tcam_status_t tcam_tm30_get_fpga_firmware_version(const tcam_t *tcam, tcam_tm30_fpga_firmware_version_t *fpga_firmware_version)
{
    const uint8_t class = 0x74U;
    const uint8_t sub = 0x03U;
    uint8_t recv_data[3U];

    if ((tcam_tm30_object_check(tcam) != TCAM_OK) ||
        (fpga_firmware_version == NULL)) {
        return TCAM_ERROR;
    }

    if (cmd_read(tcam, class, sub, recv_data, sizeof(recv_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    fpga_firmware_version->major = (uint8_t)((((uint8_t)(recv_data[0U] >> 4U)) * 10U) + (recv_data[0U] & 0x0FU));
    fpga_firmware_version->minor = (uint8_t)((((uint8_t)(recv_data[1U] >> 4U)) * 10U) + (recv_data[1U] & 0x0FU));
    fpga_firmware_version->patch = (uint8_t)((((uint8_t)(recv_data[2U] >> 4U)) * 10U) + (recv_data[2U] & 0x0FU));

    return TCAM_OK;
}

tcam_status_t tcam_tm30_get_fpga_firmware_build_time(const tcam_t *tcam, tcam_tm30_fpga_firmware_build_time_t *fpga_firmware_build_time)
{
    const uint8_t class = 0x74U;
    const uint8_t sub = 0x04U;
    uint8_t recv_data[4U];

    if ((tcam_tm30_object_check(tcam) != TCAM_OK) ||
        (fpga_firmware_build_time == NULL)) {
        return TCAM_ERROR;
    }

    if (cmd_read(tcam, class, sub, recv_data, sizeof(recv_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    fpga_firmware_build_time->year = (uint16_t)((((uint8_t)(recv_data[0U] >> 4U)) * 10U) + (recv_data[0U] & 0x0FU)) * 100U + (uint16_t)((((uint8_t)(recv_data[1U] >> 4U)) * 10U) + (recv_data[1U] & 0x0FU));
    fpga_firmware_build_time->month = (uint8_t)((((uint8_t)(recv_data[2U] >> 4U)) * 10U) + (recv_data[2U] & 0x0FU));
    fpga_firmware_build_time->day = (uint8_t)((((uint8_t)(recv_data[3U] >> 4U)) * 10U) + (recv_data[3U] & 0x0FU));

    return TCAM_OK;
}

tcam_status_t tcam_tm30_get_firmware_version(const tcam_t *tcam, tcam_tm30_firmware_version_t *firmware_version)
{
    const uint8_t class = 0x74U;
    const uint8_t sub = 0x05U;
    uint8_t recv_data[3U];

    if ((tcam_tm30_object_check(tcam) != TCAM_OK) ||
        (firmware_version == NULL)) {
        return TCAM_ERROR;
    }

    if (cmd_read(tcam, class, sub, recv_data, sizeof(recv_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    firmware_version->major = (uint8_t)((((uint8_t)(recv_data[0U] >> 4U)) * 10U) + (recv_data[0U] & 0x0FU));
    firmware_version->minor = (uint8_t)((((uint8_t)(recv_data[1U] >> 4U)) * 10U) + (recv_data[1U] & 0x0FU));
    firmware_version->patch = (uint8_t)((((uint8_t)(recv_data[2U] >> 4U)) * 10U) + (recv_data[2U] & 0x0FU));

    return TCAM_OK;
}

tcam_status_t tcam_tm30_get_firmware_build_time(const tcam_t *tcam, tcam_tm30_firmware_build_time_t *firmware_build_time)
{
    const uint8_t class = 0x74U;
    const uint8_t sub = 0x06U;
    uint8_t recv_data[4U];

    if ((tcam_tm30_object_check(tcam) != TCAM_OK) ||
        (firmware_build_time == NULL)) {
        return TCAM_ERROR;
    }

    if (cmd_read(tcam, class, sub, recv_data, sizeof(recv_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    firmware_build_time->year = (uint16_t)((((uint8_t)(recv_data[0U] >> 4U)) * 10U) + (recv_data[0U] & 0x0FU)) * 100U + (uint16_t)((((uint8_t)(recv_data[1U] >> 4U)) * 10U) + (recv_data[1U] & 0x0FU));
    firmware_build_time->month = (uint8_t)((((uint8_t)(recv_data[2U] >> 4U)) * 10U) + (recv_data[2U] & 0x0FU));
    firmware_build_time->day = (uint8_t)((((uint8_t)(recv_data[3U] >> 4U)) * 10U) + (recv_data[3U] & 0x0FU));

    return TCAM_OK;
}

tcam_status_t tcam_tm30_get_camera_calibration_time(const tcam_t *tcam, tcam_tm30_camera_calibration_time_t *camera_calibration_time)
{
    const uint8_t class = 0x74U;
    const uint8_t sub = 0x0BU;
    uint8_t recv_data[4U];

    if ((tcam_tm30_object_check(tcam) != TCAM_OK) ||
        (camera_calibration_time == NULL)) {
        return TCAM_ERROR;
    }

    if (cmd_read(tcam, class, sub, recv_data, sizeof(recv_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    camera_calibration_time->year = (uint16_t)((((uint8_t)(recv_data[0U] >> 4U)) * 10U) + (recv_data[0U] & 0x0FU)) * 100U + (uint16_t)((((uint8_t)(recv_data[1U] >> 4U)) * 10U) + (recv_data[1U] & 0x0FU));
    camera_calibration_time->month = (uint8_t)((((uint8_t)(recv_data[2U] >> 4U)) * 10U) + (recv_data[2U] & 0x0FU));
    camera_calibration_time->day = (uint8_t)((((uint8_t)(recv_data[3U] >> 4U)) * 10U) + (recv_data[3U] & 0x0FU));

    return TCAM_OK;
}

tcam_status_t tcam_tm30_do_reset(const tcam_t *tcam)
{
    const uint8_t class = 0x7DU;
    const uint8_t sub = 0x17U;
    const uint8_t send_data[1U] = {0x00U};

    if (tcam_tm30_object_check(tcam) != TCAM_OK) {
        return TCAM_ERROR;
    }

    if (cmd_write(tcam, class, sub, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_tm30_get_time(const tcam_t *tcam, tcam_tm30_time_t *time)
{
    const uint8_t class = 0x7DU;
    const uint8_t sub = 0x15U;
    uint8_t recv_data[10U];

    if ((tcam_tm30_object_check(tcam) != TCAM_OK) ||
        (time == NULL)) {
        return TCAM_ERROR;
    }

    if (cmd_read(tcam, class, sub, recv_data, sizeof(recv_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    time->year = (uint16_t)(((uint16_t)recv_data[8U] << 8) | (uint16_t)recv_data[9U]);
    time->month = recv_data[7U];
    time->week = recv_data[6U];
    time->day = recv_data[5U];
    time->hour = recv_data[4U];
    time->minute = recv_data[3U];
    time->second = recv_data[2U];
    time->millisecond = (uint16_t)(((uint16_t)recv_data[0U] << 8) | (uint16_t)recv_data[1U]);

    return TCAM_OK;
}

tcam_status_t tcam_tm30_do_save_settings(const tcam_t *tcam)
{
    const uint8_t class = 0x74U;
    const uint8_t sub = 0x10U;
    const uint8_t send_data[1U] = {0x00U};

    if (tcam_tm30_object_check(tcam) != TCAM_OK) {
        return TCAM_ERROR;
    }

    if (cmd_write(tcam, class, sub, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_tm30_do_restore_factory_settings(const tcam_t *tcam)
{
    const uint8_t class = 0x74U;
    const uint8_t sub = 0x0FU;
    const uint8_t send_data[1U] = {0x00U};

    if (tcam_tm30_object_check(tcam) != TCAM_OK) {
        return TCAM_ERROR;
    }

    if (cmd_write(tcam, class, sub, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_tm30_do_shutter_correction(const tcam_t *tcam)
{
    const uint8_t class = 0x7CU;
    const uint8_t sub = 0x02U;
    const uint8_t send_data[1U] = {0x00U};

    if (tcam_tm30_object_check(tcam) != TCAM_OK) {
        return TCAM_ERROR;
    }

    if (cmd_write(tcam, class, sub, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_tm30_do_background_correction(const tcam_t *tcam)
{
    const uint8_t class = 0x7CU;
    const uint8_t sub = 0x03U;
    const uint8_t send_data[1U] = {0x00U};

    if (tcam_tm30_object_check(tcam) != TCAM_OK) {
        return TCAM_ERROR;
    }

    if (cmd_write(tcam, class, sub, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_tm30_do_vignetting_correction(const tcam_t *tcam)
{
    const uint8_t class = 0x7CU;
    const uint8_t sub = 0x0CU;
    const uint8_t send_data[1U] = {0x02U};

    if (tcam_tm30_object_check(tcam) != TCAM_OK) {
        return TCAM_ERROR;
    }

    if (cmd_write(tcam, class, sub, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_tm30_set_auto_shutter_compensation(const tcam_t *tcam, tcam_tm30_auto_shutter_compensation_state_t auto_shutter_compensation_state)
{
    const uint8_t class = 0x7CU;
    const uint8_t sub = 0x04U;
    uint8_t send_data[1U];
    const uint8_t send_data_idx0[TCAM_TM30_AUTO_SHUTTER_COMPENSATION_STATE_COUNT] = {
        [TCAM_TM30_AUTO_SHUTTER_COMPENSATION_STATE_OFF] = 0x00U,
        [TCAM_TM30_AUTO_SHUTTER_COMPENSATION_STATE_TIMED] = 0x01U,
        [TCAM_TM30_AUTO_SHUTTER_COMPENSATION_STATE_TEMP_DELTA] = 0x02U,
        [TCAM_TM30_AUTO_SHUTTER_COMPENSATION_STATE_AUTO] = 0x03U,
    };

    if ((tcam_tm30_object_check(tcam) != TCAM_OK) ||
        ((uint32_t)auto_shutter_compensation_state >= (uint32_t)TCAM_TM30_AUTO_SHUTTER_COMPENSATION_STATE_COUNT)) {
        return TCAM_ERROR;
    }

    send_data[0U] = send_data_idx0[auto_shutter_compensation_state];
    if (cmd_write(tcam, class, sub, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_tm30_get_auto_shutter_compensation(const tcam_t *tcam, tcam_tm30_auto_shutter_compensation_state_t *auto_shutter_compensation_state)
{
    const uint8_t class = 0x7CU;
    const uint8_t sub = 0x04U;
    uint8_t recv_data[1U];

    if ((tcam_tm30_object_check(tcam) != TCAM_OK) ||
        (auto_shutter_compensation_state == NULL)) {
        return TCAM_ERROR;
    }

    if (cmd_read(tcam, class, sub, recv_data, sizeof(recv_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    switch (recv_data[0U]) {
        case 0x00U: *auto_shutter_compensation_state = TCAM_TM30_AUTO_SHUTTER_COMPENSATION_STATE_OFF; break;
        case 0x01U: *auto_shutter_compensation_state = TCAM_TM30_AUTO_SHUTTER_COMPENSATION_STATE_TIMED; break;
        case 0x02U: *auto_shutter_compensation_state = TCAM_TM30_AUTO_SHUTTER_COMPENSATION_STATE_TEMP_DELTA; break;
        case 0x03U: *auto_shutter_compensation_state = TCAM_TM30_AUTO_SHUTTER_COMPENSATION_STATE_AUTO; break;
        default: return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_tm30_set_auto_shutter_compensation_timed_interval(const tcam_t *tcam, uint16_t auto_shutter_compensation_timed_interval_min)
{
    const uint8_t class = 0x7CU;
    const uint8_t sub = 0x05U;
    uint8_t send_data[2U];

    if ((tcam_tm30_object_check(tcam) != TCAM_OK) ||
        (auto_shutter_compensation_timed_interval_min < TCAM_TM30_AUTO_SHUTTER_COMPENSATION_TIMED_INTERVAL_MIN_MIN) ||
        (auto_shutter_compensation_timed_interval_min > TCAM_TM30_AUTO_SHUTTER_COMPENSATION_TIMED_INTERVAL_MIN_MAX)) {
        return TCAM_ERROR;
    }

    send_data[0] = (uint8_t)((auto_shutter_compensation_timed_interval_min >> 8) & 0xFFU);
    send_data[1] = (uint8_t)(auto_shutter_compensation_timed_interval_min & 0xFFU);
    if (cmd_write(tcam, class, sub, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_tm30_get_auto_shutter_compensation_timed_interval(const tcam_t *tcam, uint16_t *auto_shutter_compensation_timed_interval_min)
{
    const uint8_t class = 0x7CU;
    const uint8_t sub = 0x05U;
    uint8_t recv_data[2U];

    if ((tcam_tm30_object_check(tcam) != TCAM_OK) ||
        (auto_shutter_compensation_timed_interval_min == NULL)) {
        return TCAM_ERROR;
    }

    if (cmd_read(tcam, class, sub, recv_data, sizeof(recv_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    if (((uint16_t)(((uint16_t)recv_data[0U] << 8) | (uint16_t)recv_data[1U]) < TCAM_TM30_AUTO_SHUTTER_COMPENSATION_TIMED_INTERVAL_MIN_MIN) ||
        ((uint16_t)(((uint16_t)recv_data[0U] << 8) | (uint16_t)recv_data[1U]) > TCAM_TM30_AUTO_SHUTTER_COMPENSATION_TIMED_INTERVAL_MIN_MAX)) {
        return TCAM_ERROR;
    }

    *auto_shutter_compensation_timed_interval_min = (uint16_t)(((uint16_t)recv_data[0U] << 8) | (uint16_t)recv_data[1U]);

    return TCAM_OK;
}

tcam_status_t tcam_tm30_set_cursor_display(const tcam_t *tcam, tcam_tm30_cursor_display_state_t cursor_display_state)
{
    const uint8_t class = 0x78U;
    const uint8_t sub = 0x1AU;
    uint8_t send_data[1U];
    const uint8_t send_data_idx0[TCAM_TM30_CURSOR_DISPLAY_STATE_COUNT] = {
        [TCAM_TM30_CURSOR_DISPLAY_STATE_OFF] = 0x00U,
        [TCAM_TM30_CURSOR_DISPLAY_STATE_ON] = 0x0FU,
    };

    if ((tcam_tm30_object_check(tcam) != TCAM_OK) ||
        ((uint32_t)cursor_display_state >= (uint32_t)TCAM_TM30_CURSOR_DISPLAY_STATE_COUNT)) {
        return TCAM_ERROR;
    }

    send_data[0U] = send_data_idx0[cursor_display_state];
    if (cmd_write(tcam, class, sub, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_tm30_set_cursor_position(const tcam_t *tcam, tcam_tm30_cursor_position_ctrl_t cursor_position_ctrl, uint8_t cursor_position_ctrl_step)
{
    const uint8_t class = 0x78U;
    const uint8_t sub = 0x1AU;
    uint8_t send_data[1U];
    const uint8_t send_data_idx0[TCAM_TM30_CURSOR_POSITION_CTRL_COUNT] = {
        [TCAM_TM30_CURSOR_POSITION_CTRL_UP] = 0x20U,
        [TCAM_TM30_CURSOR_POSITION_CTRL_DOWN] = 0x30U,
        [TCAM_TM30_CURSOR_POSITION_CTRL_LEFT] = 0x40U,
        [TCAM_TM30_CURSOR_POSITION_CTRL_RIGHT] = 0x50U,
        [TCAM_TM30_CURSOR_POSITION_CTRL_CENTER] = 0x06U,
    };

    if ((tcam_tm30_object_check(tcam) != TCAM_OK) ||
        ((uint32_t)cursor_position_ctrl >= (uint32_t)TCAM_TM30_CURSOR_POSITION_CTRL_COUNT) ||
        ((cursor_position_ctrl != TCAM_TM30_CURSOR_POSITION_CTRL_CENTER) &&
         ((cursor_position_ctrl_step < TCAM_TM30_CURSOR_POSITION_CTRL_STEP_MIN) ||
          (cursor_position_ctrl_step > TCAM_TM30_CURSOR_POSITION_CTRL_STEP_MAX)))) {
        return TCAM_ERROR;
    }

    if (cursor_position_ctrl != TCAM_TM30_CURSOR_POSITION_CTRL_CENTER) {
        send_data[0U] = cursor_position_ctrl_step;
    } else {
        send_data[0U] = 0U;
    }

    send_data[0U] |= send_data_idx0[cursor_position_ctrl];
    if (cmd_write(tcam, class, sub, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_tm30_set_cursor_bad_point(const tcam_t *tcam, tcam_tm30_cursor_bad_point_ctrl_t cursor_bad_point_ctrl)
{
    const uint8_t class = 0x78U;
    const uint8_t sub = 0x1AU;
    uint8_t send_data[1U];
    const uint8_t send_data_idx0[TCAM_TM30_CURSOR_BAD_POINT_CTRL_COUNT] = {
        [TCAM_TM30_CURSOR_BAD_POINT_CTRL_ADD] = 0x0DU,
        [TCAM_TM30_CURSOR_BAD_POINT_CTRL_DELETE] = 0x0EU,
    };

    if ((tcam_tm30_object_check(tcam) != TCAM_OK) ||
        ((uint32_t)cursor_bad_point_ctrl >= (uint32_t)TCAM_TM30_CURSOR_BAD_POINT_CTRL_COUNT)) {
        return TCAM_ERROR;
    }

    send_data[0U] = send_data_idx0[cursor_bad_point_ctrl];
    if (cmd_write(tcam, class, sub, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_tm30_set_image_brightness(const tcam_t *tcam, uint8_t image_brightness_percent)
{
    const uint8_t class = 0x78U;
    const uint8_t sub = 0x02U;
    uint8_t send_data[1U];

    if ((tcam_tm30_object_check(tcam) != TCAM_OK) ||
        (image_brightness_percent < TCAM_TM30_IMAGE_BRIGHTNESS_PERCENT_MIN) ||
        (image_brightness_percent > TCAM_TM30_IMAGE_BRIGHTNESS_PERCENT_MAX)) {
        return TCAM_ERROR;
    }

    send_data[0] = image_brightness_percent;
    if (cmd_write(tcam, class, sub, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_tm30_get_image_brightness(const tcam_t *tcam, uint8_t *image_brightness_percent)
{
    const uint8_t class = 0x78U;
    const uint8_t sub = 0x02U;
    uint8_t recv_data[1U];

    if ((tcam_tm30_object_check(tcam) != TCAM_OK) ||
        (image_brightness_percent == NULL)) {
        return TCAM_ERROR;
    }

    if (cmd_read(tcam, class, sub, recv_data, sizeof(recv_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    if ((recv_data[0U] < TCAM_TM30_IMAGE_BRIGHTNESS_PERCENT_MIN) ||
        (recv_data[0U] > TCAM_TM30_IMAGE_BRIGHTNESS_PERCENT_MAX)) {
        return TCAM_ERROR;
    }

    *image_brightness_percent = recv_data[0U];

    return TCAM_OK;
}

tcam_status_t tcam_tm30_set_image_contrast(const tcam_t *tcam, uint8_t image_contrast_percent)
{
    const uint8_t class = 0x78U;
    const uint8_t sub = 0x03U;
    uint8_t send_data[1U];

    if ((tcam_tm30_object_check(tcam) != TCAM_OK) ||
        (image_contrast_percent < TCAM_TM30_IMAGE_CONTRAST_PERCENT_MIN) ||
        (image_contrast_percent > TCAM_TM30_IMAGE_CONTRAST_PERCENT_MAX)) {
        return TCAM_ERROR;
    }

    send_data[0] = image_contrast_percent;
    if (cmd_write(tcam, class, sub, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_tm30_get_image_contrast(const tcam_t *tcam, uint8_t *image_contrast_percent)
{
    const uint8_t class = 0x78U;
    const uint8_t sub = 0x03U;
    uint8_t recv_data[1U];

    if ((tcam_tm30_object_check(tcam) != TCAM_OK) ||
        (image_contrast_percent == NULL)) {
        return TCAM_ERROR;
    }

    if (cmd_read(tcam, class, sub, recv_data, sizeof(recv_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    if ((recv_data[0U] < TCAM_TM30_IMAGE_CONTRAST_PERCENT_MIN) ||
        (recv_data[0U] > TCAM_TM30_IMAGE_CONTRAST_PERCENT_MAX)) {
        return TCAM_ERROR;
    }

    *image_contrast_percent = recv_data[0U];

    return TCAM_OK;
}

tcam_status_t tcam_tm30_set_image_detail_enhancement(const tcam_t *tcam, uint8_t image_detail_enhancement_percent)
{
    const uint8_t class = 0x78U;
    const uint8_t sub = 0x10U;
    uint8_t send_data[1U];

    if ((tcam_tm30_object_check(tcam) != TCAM_OK) ||
        (image_detail_enhancement_percent < TCAM_TM30_IMAGE_DETAIL_ENHANCEMENT_PERCENT_MIN) ||
        (image_detail_enhancement_percent > TCAM_TM30_IMAGE_DETAIL_ENHANCEMENT_PERCENT_MAX)) {
        return TCAM_ERROR;
    }

    send_data[0] = image_detail_enhancement_percent;
    if (cmd_write(tcam, class, sub, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_tm30_get_image_detail_enhancement(const tcam_t *tcam, uint8_t *image_detail_enhancement_percent)
{
    const uint8_t class = 0x78U;
    const uint8_t sub = 0x10U;
    uint8_t recv_data[1U];

    if ((tcam_tm30_object_check(tcam) != TCAM_OK) ||
        (image_detail_enhancement_percent == NULL)) {
        return TCAM_ERROR;
    }

    if (cmd_read(tcam, class, sub, recv_data, sizeof(recv_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    if ((recv_data[0U] < TCAM_TM30_IMAGE_DETAIL_ENHANCEMENT_PERCENT_MIN) ||
        (recv_data[0U] > TCAM_TM30_IMAGE_DETAIL_ENHANCEMENT_PERCENT_MAX)) {
        return TCAM_ERROR;
    }

    *image_detail_enhancement_percent = recv_data[0U];

    return TCAM_OK;
}

tcam_status_t tcam_tm30_set_image_spatial_denoising(const tcam_t *tcam, uint8_t image_spatial_denoising_percent)
{
    const uint8_t class = 0x78U;
    const uint8_t sub = 0x15U;
    uint8_t send_data[1U];

    if ((tcam_tm30_object_check(tcam) != TCAM_OK) ||
        (image_spatial_denoising_percent < TCAM_TM30_IMAGE_SPATIAL_DENOISING_PERCENT_MIN) ||
        (image_spatial_denoising_percent > TCAM_TM30_IMAGE_SPATIAL_DENOISING_PERCENT_MAX)) {
        return TCAM_ERROR;
    }

    send_data[0] = image_spatial_denoising_percent;
    if (cmd_write(tcam, class, sub, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_tm30_get_image_spatial_denoising(const tcam_t *tcam, uint8_t *image_spatial_denoising_percent)
{
    const uint8_t class = 0x78U;
    const uint8_t sub = 0x15U;
    uint8_t recv_data[1U];

    if ((tcam_tm30_object_check(tcam) != TCAM_OK) ||
        (image_spatial_denoising_percent == NULL)) {
        return TCAM_ERROR;
    }

    if (cmd_read(tcam, class, sub, recv_data, sizeof(recv_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    if ((recv_data[0U] < TCAM_TM30_IMAGE_SPATIAL_DENOISING_PERCENT_MIN) ||
        (recv_data[0U] > TCAM_TM30_IMAGE_SPATIAL_DENOISING_PERCENT_MAX)) {
        return TCAM_ERROR;
    }

    *image_spatial_denoising_percent = recv_data[0U];

    return TCAM_OK;
}

tcam_status_t tcam_tm30_set_image_temporal_denoising(const tcam_t *tcam, uint8_t image_temporal_denoising_percent)
{
    const uint8_t class = 0x78U;
    const uint8_t sub = 0x16U;
    uint8_t send_data[1U];

    if ((tcam_tm30_object_check(tcam) != TCAM_OK) ||
        (image_temporal_denoising_percent < TCAM_TM30_IMAGE_TEMPORAL_DENOISING_PERCENT_MIN) ||
        (image_temporal_denoising_percent > TCAM_TM30_IMAGE_TEMPORAL_DENOISING_PERCENT_MAX)) {
        return TCAM_ERROR;
    }

    send_data[0] = image_temporal_denoising_percent;
    if (cmd_write(tcam, class, sub, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_tm30_get_image_temporal_denoising(const tcam_t *tcam, uint8_t *image_temporal_denoising_percent)
{
    const uint8_t class = 0x78U;
    const uint8_t sub = 0x16U;
    uint8_t recv_data[1U];

    if ((tcam_tm30_object_check(tcam) != TCAM_OK) ||
        (image_temporal_denoising_percent == NULL)) {
        return TCAM_ERROR;
    }

    if (cmd_read(tcam, class, sub, recv_data, sizeof(recv_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    if ((recv_data[0U] < TCAM_TM30_IMAGE_TEMPORAL_DENOISING_PERCENT_MIN) ||
        (recv_data[0U] > TCAM_TM30_IMAGE_TEMPORAL_DENOISING_PERCENT_MAX)) {
        return TCAM_ERROR;
    }

    *image_temporal_denoising_percent = recv_data[0U];

    return TCAM_OK;
}

tcam_status_t tcam_tm30_set_image_color(const tcam_t *tcam, tcam_tm30_image_color_t image_color)
{
    const uint8_t class = 0x78U;
    const uint8_t sub = 0x20U;
    uint8_t send_data[1U];
    const uint8_t send_data_idx0[TCAM_TM30_IMAGE_COLOR_COUNT] = {
        [TCAM_TM30_IMAGE_COLOR_WHITE_HOT] = 0x00U,
        [TCAM_TM30_IMAGE_COLOR_BLACK_HOT] = 0x01U,
        [TCAM_TM30_IMAGE_COLOR_FUSION_1] = 0x02U,
        [TCAM_TM30_IMAGE_COLOR_RAINBOW] = 0x03U,
        [TCAM_TM30_IMAGE_COLOR_FUSION_2] = 0x04U,
        [TCAM_TM30_IMAGE_COLOR_IRON_RED_1] = 0x05U,
        [TCAM_TM30_IMAGE_COLOR_IRON_RED_2] = 0x06U,
        [TCAM_TM30_IMAGE_COLOR_DARK_BROWN] = 0x07U,
        [TCAM_TM30_IMAGE_COLOR_HUE_1] = 0x08U,
        [TCAM_TM30_IMAGE_COLOR_HUE_2] = 0x09U,
        [TCAM_TM30_IMAGE_COLOR_ICE_FIRE] = 0x0AU,
        [TCAM_TM30_IMAGE_COLOR_RAIN] = 0x0BU,
        [TCAM_TM30_IMAGE_COLOR_GREEN_HOT] = 0x0CU,
        [TCAM_TM30_IMAGE_COLOR_RED_HOT] = 0x0DU,
        [TCAM_TM30_IMAGE_COLOR_DARK_BLUE] = 0x0EU,
    };

    if ((tcam_tm30_object_check(tcam) != TCAM_OK) ||
        ((uint32_t)image_color >= (uint32_t)TCAM_TM30_IMAGE_COLOR_COUNT)) {
        return TCAM_ERROR;
    }

    send_data[0U] = send_data_idx0[image_color];
    if (cmd_write(tcam, class, sub, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_tm30_get_image_color(const tcam_t *tcam, tcam_tm30_image_color_t *image_color)
{
    const uint8_t class = 0x78U;
    const uint8_t sub = 0x20U;
    uint8_t recv_data[1U];

    if ((tcam_tm30_object_check(tcam) != TCAM_OK) ||
        (image_color == NULL)) {
        return TCAM_ERROR;
    }

    if (cmd_read(tcam, class, sub, recv_data, sizeof(recv_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    switch (recv_data[0U]) {
        case 0x00U: *image_color = TCAM_TM30_IMAGE_COLOR_WHITE_HOT; break;
        case 0x01U: *image_color = TCAM_TM30_IMAGE_COLOR_BLACK_HOT; break;
        case 0x02U: *image_color = TCAM_TM30_IMAGE_COLOR_FUSION_1; break;
        case 0x03U: *image_color = TCAM_TM30_IMAGE_COLOR_RAINBOW; break;
        case 0x04U: *image_color = TCAM_TM30_IMAGE_COLOR_FUSION_2; break;
        case 0x05U: *image_color = TCAM_TM30_IMAGE_COLOR_IRON_RED_1; break;
        case 0x06U: *image_color = TCAM_TM30_IMAGE_COLOR_IRON_RED_2; break;
        case 0x07U: *image_color = TCAM_TM30_IMAGE_COLOR_DARK_BROWN; break;
        case 0x08U: *image_color = TCAM_TM30_IMAGE_COLOR_HUE_1; break;
        case 0x09U: *image_color = TCAM_TM30_IMAGE_COLOR_HUE_2; break;
        case 0x0AU: *image_color = TCAM_TM30_IMAGE_COLOR_ICE_FIRE; break;
        case 0x0BU: *image_color = TCAM_TM30_IMAGE_COLOR_RAIN; break;
        case 0x0CU: *image_color = TCAM_TM30_IMAGE_COLOR_GREEN_HOT; break;
        case 0x0DU: *image_color = TCAM_TM30_IMAGE_COLOR_RED_HOT; break;
        case 0x0EU: *image_color = TCAM_TM30_IMAGE_COLOR_DARK_BLUE; break;
        default: return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_tm30_set_image_mirror(const tcam_t *tcam, tcam_tm30_image_mirror_t image_mirror)
{
    const uint8_t class = 0x70U;
    const uint8_t sub = 0x11U;
    uint8_t send_data[1U];
    const uint8_t send_data_idx0[TCAM_TM30_IMAGE_MIRROR_COUNT] = {
        [TCAM_TM30_IMAGE_MIRROR_NONE] = 0x00U,
        [TCAM_TM30_IMAGE_MIRROR_CENTER] = 0x01U,
        [TCAM_TM30_IMAGE_MIRROR_LEFT_RIGHT] = 0x02U,
        [TCAM_TM30_IMAGE_MIRROR_UP_DOWN] = 0x03U,
    };

    if ((tcam_tm30_object_check(tcam) != TCAM_OK) ||
        ((uint32_t)image_mirror >= (uint32_t)TCAM_TM30_IMAGE_MIRROR_COUNT)) {
        return TCAM_ERROR;
    }

    send_data[0U] = send_data_idx0[image_mirror];
    if (cmd_write(tcam, class, sub, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_tm30_get_image_mirror(const tcam_t *tcam, tcam_tm30_image_mirror_t *image_mirror)
{
    const uint8_t class = 0x70U;
    const uint8_t sub = 0x11U;
    uint8_t recv_data[1U];

    if ((tcam_tm30_object_check(tcam) != TCAM_OK) ||
        (image_mirror == NULL)) {
        return TCAM_ERROR;
    }

    if (cmd_read(tcam, class, sub, recv_data, sizeof(recv_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    switch (recv_data[0U]) {
        case 0x00U: *image_mirror = TCAM_TM30_IMAGE_MIRROR_NONE; break;
        case 0x01U: *image_mirror = TCAM_TM30_IMAGE_MIRROR_CENTER; break;
        case 0x02U: *image_mirror = TCAM_TM30_IMAGE_MIRROR_LEFT_RIGHT; break;
        case 0x03U: *image_mirror = TCAM_TM30_IMAGE_MIRROR_UP_DOWN; break;
        default: return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_tm30_set_image_rotate(const tcam_t *tcam, tcam_tm30_image_rotate_t image_rotate)
{
    const uint8_t class = 0x7DU;
    const uint8_t sub = 0x18U;
    uint8_t send_data[1U];
    const uint8_t send_data_idx0[TCAM_TM30_IMAGE_ROTATE_COUNT] = {
        [TCAM_TM30_IMAGE_ROTATE_DISABLE] = 0x00U,
        [TCAM_TM30_IMAGE_ROTATE_ENABLE] = 0x01U,
    };

    if ((tcam_tm30_object_check(tcam) != TCAM_OK) ||
        ((uint32_t)image_rotate >= (uint32_t)TCAM_TM30_IMAGE_ROTATE_COUNT)) {
        return TCAM_ERROR;
    }

    send_data[0U] = send_data_idx0[image_rotate];
    if (cmd_write(tcam, class, sub, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_tm30_get_image_rotate(const tcam_t *tcam, tcam_tm30_image_rotate_t *image_rotate)
{
    const uint8_t class = 0x7DU;
    const uint8_t sub = 0x18U;
    uint8_t recv_data[1U];

    if ((tcam_tm30_object_check(tcam) != TCAM_OK) ||
        (image_rotate == NULL)) {
        return TCAM_ERROR;
    }

    if (cmd_read(tcam, class, sub, recv_data, sizeof(recv_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    switch (recv_data[0U]) {
        case 0x00U: *image_rotate = TCAM_TM30_IMAGE_ROTATE_DISABLE; break;
        case 0x01U: *image_rotate = TCAM_TM30_IMAGE_ROTATE_ENABLE; break;
        default: return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_tm30_set_stream_format(const tcam_t *tcam, tcam_tm30_stream_format_t stream_format)
{
    const uint8_t class = 0x7DU;
    const uint8_t sub = 0x16U;
    uint8_t send_data[1U];
    const uint8_t send_data_idx0[TCAM_TM30_STREAM_FORMAT_COUNT] = {
        [TCAM_TM30_STREAM_FORMAT_MT] = 0x02U,
        [TCAM_TM30_STREAM_FORMAT_RAW] = 0x03U,
        [TCAM_TM30_STREAM_FORMAT_YUV] = 0x06U,
        [TCAM_TM30_STREAM_FORMAT_MT_YUV] = 0x08U,
        [TCAM_TM30_STREAM_FORMAT_RAW_YUV] = 0x09U,
        [TCAM_TM30_STREAM_FORMAT_YUV_IMAGE] = 0x0AU,
    };

    if ((tcam_tm30_object_check(tcam) != TCAM_OK) ||
        ((uint32_t)stream_format >= (uint32_t)TCAM_TM30_STREAM_FORMAT_COUNT)) {
        return TCAM_ERROR;
    }

    send_data[0U] = send_data_idx0[stream_format];
    if (cmd_write(tcam, class, sub, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_tm30_get_stream_format(const tcam_t *tcam, tcam_tm30_stream_format_t *stream_format)
{
    const uint8_t class = 0x7DU;
    const uint8_t sub = 0x16U;
    uint8_t recv_data[1U];

    if ((tcam_tm30_object_check(tcam) != TCAM_OK) ||
        (stream_format == NULL)) {
        return TCAM_ERROR;
    }

    if (cmd_read(tcam, class, sub, recv_data, sizeof(recv_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    switch (recv_data[0U]) {
        case 0x02U: *stream_format = TCAM_TM30_STREAM_FORMAT_MT; break;
        case 0x03U: *stream_format = TCAM_TM30_STREAM_FORMAT_RAW; break;
        case 0x06U: *stream_format = TCAM_TM30_STREAM_FORMAT_YUV; break;
        case 0x08U: *stream_format = TCAM_TM30_STREAM_FORMAT_MT_YUV; break;
        case 0x09U: *stream_format = TCAM_TM30_STREAM_FORMAT_RAW_YUV; break;
        case 0x0AU: *stream_format = TCAM_TM30_STREAM_FORMAT_YUV_IMAGE; break;
        default: return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_tm30_set_temp_measurement(const tcam_t *tcam, tcam_tm30_temp_measurement_state_t temp_measurement_state)
{
    const uint8_t class = 0x7DU;
    const uint8_t sub = 0x01U;
    uint8_t send_data[1U];
    const uint8_t send_data_idx0[TCAM_TM30_TEMP_MEASUREMENT_STATE_COUNT] = {
        [TCAM_TM30_TEMP_MEASUREMENT_STATE_DISABLE] = 0x00U,
        [TCAM_TM30_TEMP_MEASUREMENT_STATE_ENABLE] = 0x01U,
    };

    if ((tcam_tm30_object_check(tcam) != TCAM_OK) ||
        ((uint32_t)temp_measurement_state >= (uint32_t)TCAM_TM30_TEMP_MEASUREMENT_STATE_COUNT)) {
        return TCAM_ERROR;
    }

    send_data[0U] = send_data_idx0[temp_measurement_state];
    if (cmd_write(tcam, class, sub, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_tm30_get_temp_measurement(const tcam_t *tcam, tcam_tm30_temp_measurement_state_t *temp_measurement_state)
{
    const uint8_t class = 0x7DU;
    const uint8_t sub = 0x01U;
    uint8_t recv_data[1U];

    if ((tcam_tm30_object_check(tcam) != TCAM_OK) ||
        (temp_measurement_state == NULL)) {
        return TCAM_ERROR;
    }

    if (cmd_read(tcam, class, sub, recv_data, sizeof(recv_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    switch (recv_data[0U]) {
        case 0x00U: *temp_measurement_state = TCAM_TM30_TEMP_MEASUREMENT_STATE_DISABLE; break;
        case 0x01U: *temp_measurement_state = TCAM_TM30_TEMP_MEASUREMENT_STATE_ENABLE; break;
        default: return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_tm30_set_temp_unit(const tcam_t *tcam, tcam_tm30_temp_measurement_unit_t temp_measurement_unit)
{
    const uint8_t class = 0x7DU;
    const uint8_t sub = 0x04U;
    uint8_t send_data[1U];
    const uint8_t send_data_idx0[TCAM_TM30_TEMP_MEASUREMENT_UNIT_COUNT] = {
        [TCAM_TM30_TEMP_MEASUREMENT_UNIT_CELSIUS] = 0x01U,
        [TCAM_TM30_TEMP_MEASUREMENT_UNIT_FAHRENHEIT] = 0x02U,
        [TCAM_TM30_TEMP_MEASUREMENT_UNIT_KELVIN] = 0x03U,
    };

    if ((tcam_tm30_object_check(tcam) != TCAM_OK) ||
        ((uint32_t)temp_measurement_unit >= (uint32_t)TCAM_TM30_TEMP_MEASUREMENT_UNIT_COUNT)) {
        return TCAM_ERROR;
    }

    send_data[0U] = send_data_idx0[temp_measurement_unit];
    if (cmd_write(tcam, class, sub, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_tm30_get_temp_unit(const tcam_t *tcam, tcam_tm30_temp_measurement_unit_t *temp_measurement_unit)
{
    const uint8_t class = 0x7DU;
    const uint8_t sub = 0x04U;
    uint8_t recv_data[1U];

    if ((tcam_tm30_object_check(tcam) != TCAM_OK) ||
        (temp_measurement_unit == NULL)) {
        return TCAM_ERROR;
    }

    if (cmd_read(tcam, class, sub, recv_data, sizeof(recv_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    switch (recv_data[0U]) {
        case 0x01U: *temp_measurement_unit = TCAM_TM30_TEMP_MEASUREMENT_UNIT_CELSIUS; break;
        case 0x02U: *temp_measurement_unit = TCAM_TM30_TEMP_MEASUREMENT_UNIT_FAHRENHEIT; break;
        case 0x03U: *temp_measurement_unit = TCAM_TM30_TEMP_MEASUREMENT_UNIT_KELVIN; break;
        default: return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_tm30_set_temp_measurement_range(const tcam_t *tcam, tcam_tm30_temp_measurement_range_t temp_measurement_range)
{
    const uint8_t class = 0x7DU;
    const uint8_t sub = 0x05U;
    uint8_t send_data[1U];
    const uint8_t send_data_idx0[TCAM_TM30_TEMP_MEASUREMENT_RANGE_COUNT] = {
        [TCAM_TM30_TEMP_MEASUREMENT_RANGE_N20_150] = 0x02U,
        [TCAM_TM30_TEMP_MEASUREMENT_RANGE_100_550] = 0x03U,
        [TCAM_TM30_TEMP_MEASUREMENT_RANGE_AUTO] = 0x04U,
    };

    if ((tcam_tm30_object_check(tcam) != TCAM_OK) ||
        ((uint32_t)temp_measurement_range >= (uint32_t)TCAM_TM30_TEMP_MEASUREMENT_RANGE_COUNT)) {
        return TCAM_ERROR;
    }

    send_data[0U] = send_data_idx0[temp_measurement_range];
    if (cmd_write(tcam, class, sub, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_tm30_get_temp_measurement_range(const tcam_t *tcam, tcam_tm30_temp_measurement_range_t *temp_measurement_range)
{
    const uint8_t class = 0x7DU;
    const uint8_t sub = 0x05U;
    uint8_t recv_data[2U];

    if ((tcam_tm30_object_check(tcam) != TCAM_OK) ||
        (temp_measurement_range == NULL)) {
        return TCAM_ERROR;
    }

    if (cmd_read(tcam, class, sub, recv_data, sizeof(recv_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    switch (recv_data[0U]) {
        case 0x00U:
            switch (recv_data[1U]) {
                case 0x02: *temp_measurement_range = TCAM_TM30_TEMP_MEASUREMENT_RANGE_N20_150; break;
                case 0x03: *temp_measurement_range = TCAM_TM30_TEMP_MEASUREMENT_RANGE_100_550; break;
                default: return TCAM_ERROR;
            }
            break;
        case 0x01U: *temp_measurement_range = TCAM_TM30_TEMP_MEASUREMENT_RANGE_AUTO; break;
        default: return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_tm30_set_emissivity(const tcam_t *tcam, uint8_t emissivity_percent)
{
    const uint8_t class = 0x7DU;
    const uint8_t sub = 0x08U;
    uint8_t send_data[1U];

    if ((tcam_tm30_object_check(tcam) != TCAM_OK) ||
        (emissivity_percent < TCAM_TM30_EMISSIVITY_PERCENT_MIN) ||
        (emissivity_percent > TCAM_TM30_EMISSIVITY_PERCENT_MAX)) {
        return TCAM_ERROR;
    }

    send_data[0U] = emissivity_percent;
    if (cmd_write(tcam, class, sub, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_tm30_get_emissivity(const tcam_t *tcam, uint8_t *emissivity_percent)
{
    const uint8_t class = 0x7DU;
    const uint8_t sub = 0x08U;
    uint8_t recv_data[1U];

    if ((tcam_tm30_object_check(tcam) != TCAM_OK) ||
        (emissivity_percent == NULL)) {
        return TCAM_ERROR;
    }

    if (cmd_read(tcam, class, sub, recv_data, sizeof(recv_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    if ((recv_data[0U] < TCAM_TM30_EMISSIVITY_PERCENT_MIN) ||
        (recv_data[0U] > TCAM_TM30_EMISSIVITY_PERCENT_MAX)) {
        return TCAM_ERROR;
    }

    *emissivity_percent = recv_data[0U];

    return TCAM_OK;
}

tcam_status_t tcam_tm30_set_distance(const tcam_t *tcam, uint16_t distance_dm)
{
    const uint8_t class = 0x7DU;
    const uint8_t sub = 0x09U;
    uint8_t send_data[3U];

    if ((tcam_tm30_object_check(tcam) != TCAM_OK) ||
        (distance_dm < TCAM_TM30_DISTANCE_DM_MIN) ||
        (distance_dm > TCAM_TM30_DISTANCE_DM_MAX)) {
        return TCAM_ERROR;
    }

    send_data[0U] = 0x02U;
    send_data[1U] = (uint8_t)(((distance_dm * 10U) >> 8) & 0xFFU);
    send_data[2U] = (uint8_t)((distance_dm * 10U) & 0xFFU);
    if (cmd_write(tcam, class, sub, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_tm30_get_distance(const tcam_t *tcam, uint16_t *distance_dm)
{
    const uint8_t class = 0x7DU;
    const uint8_t sub = 0x09U;
    uint8_t recv_data[3U];

    if ((tcam_tm30_object_check(tcam) != TCAM_OK) ||
        (distance_dm == NULL)) {
        return TCAM_ERROR;
    }

    if (cmd_read(tcam, class, sub, recv_data, sizeof(recv_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    if ((recv_data[0U] != 0x02U) ||
        ((uint16_t)(((uint16_t)recv_data[1U] << 8) | (uint16_t)recv_data[2U]) < (TCAM_TM30_DISTANCE_DM_MIN * 10U)) ||
        ((uint16_t)(((uint16_t)recv_data[1U] << 8) | (uint16_t)recv_data[2U]) > (TCAM_TM30_DISTANCE_DM_MAX * 10U))) {
        return TCAM_ERROR;
    }

    *distance_dm = (uint16_t)(((uint16_t)recv_data[1U] << 8) | (uint16_t)recv_data[2U]) / 10U;

    return TCAM_OK;
}

tcam_status_t tcam_tm30_set_reflected_temp(const tcam_t *tcam, tcam_tm30_reflected_temp_state_t reflected_temp_state, uint16_t reflected_temp_add100_mult10)
{
    const uint8_t class = 0x7DU;
    const uint8_t sub = 0x0AU;
    uint8_t send_data[3U];
    const uint8_t send_data_idx0[TCAM_TM30_REFLECTED_TEMP_STATE_COUNT] = {
        [TCAM_TM30_REFLECTED_TEMP_STATE_DISABLE] = 0x00U,
        [TCAM_TM30_REFLECTED_TEMP_STATE_ENABLE] = 0x01U,
    };

    if ((tcam_tm30_object_check(tcam) != TCAM_OK) ||
        ((uint32_t)reflected_temp_state >= (uint32_t)TCAM_TM30_REFLECTED_TEMP_STATE_COUNT) ||
        (reflected_temp_add100_mult10 < TCAM_TM30_REFLECTED_TEMP_ADD100_MULT10_MIN) ||
        (reflected_temp_add100_mult10 > TCAM_TM30_REFLECTED_TEMP_ADD100_MULT10_MAX)) {
        return TCAM_ERROR;
    }

    send_data[0U] = send_data_idx0[reflected_temp_state];
    send_data[1U] = (uint8_t)((reflected_temp_add100_mult10 >> 8) & 0xFFU);
    send_data[2U] = (uint8_t)(reflected_temp_add100_mult10 & 0xFFU);
    if (cmd_write(tcam, class, sub, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_tm30_get_reflected_temp(const tcam_t *tcam, tcam_tm30_reflected_temp_state_t *reflected_temp_state, uint16_t *reflected_temp_add100_mult10)
{
    const uint8_t class = 0x7DU;
    const uint8_t sub = 0x0AU;
    uint8_t recv_data[3U];

    if ((tcam_tm30_object_check(tcam) != TCAM_OK) ||
        (reflected_temp_state == NULL) ||
        (reflected_temp_add100_mult10 == NULL)) {
        return TCAM_ERROR;
    }

    if (cmd_read(tcam, class, sub, recv_data, sizeof(recv_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    if (((uint16_t)(((uint16_t)recv_data[1U] << 8) | (uint16_t)recv_data[2U]) < TCAM_TM30_REFLECTED_TEMP_ADD100_MULT10_MIN) ||
        ((uint16_t)(((uint16_t)recv_data[1U] << 8) | (uint16_t)recv_data[2U]) > TCAM_TM30_REFLECTED_TEMP_ADD100_MULT10_MAX)) {
        return TCAM_ERROR;
    }
    *reflected_temp_add100_mult10 = (uint16_t)(((uint16_t)recv_data[1U] << 8) | (uint16_t)recv_data[2U]);

    switch (recv_data[0U]) {
        case 0x00U: *reflected_temp_state = TCAM_TM30_REFLECTED_TEMP_STATE_DISABLE; break;
        case 0x01U: *reflected_temp_state = TCAM_TM30_REFLECTED_TEMP_STATE_ENABLE; break;
        default: return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_tm30_set_calibration_param(const tcam_t *tcam, tcam_tm30_calibration_param_t *calibration_param)
{
    const uint8_t class = 0x7DU;
    const uint8_t sub = 0x11U;
    uint8_t send_data[13U];

    if ((tcam_tm30_object_check(tcam) != TCAM_OK) ||
        (calibration_param == NULL) ||
        (calibration_param->distance_dm < TCAM_TM30_CALIBRATION_PARAM_DISTANCE_DM_MIN) ||
        (calibration_param->distance_dm > TCAM_TM30_CALIBRATION_PARAM_DISTANCE_DM_MAX) ||
        (calibration_param->atmospheric_temp_add300_mult10 < TCAM_TM30_CALIBRATION_PARAM_ATMOSPHERIC_TEMP_ADD300_MULT10_MIN) ||
        (calibration_param->atmospheric_temp_add300_mult10 > TCAM_TM30_CALIBRATION_PARAM_ATMOSPHERIC_TEMP_ADD300_MULT10_MAX) ||
        (calibration_param->emissivity_percent < TCAM_TM30_CALIBRATION_PARAM_EMISSIVITY_PERCENT_MIN) ||
        (calibration_param->emissivity_percent > TCAM_TM30_CALIBRATION_PARAM_EMISSIVITY_PERCENT_MAX) ||
        (calibration_param->calibration_temp_add100_mult10 < TCAM_TM30_CALIBRATION_PARAM_CALIBRATION_TEMP_ADD100_MULT10_MIN) ||
        (calibration_param->calibration_temp_add100_mult10 > TCAM_TM30_CALIBRATION_PARAM_CALIBRATION_TEMP_ADD100_MULT10_MAX) ||
        (calibration_param->xpos_nor < TCAM_TM30_CALIBRATION_PARAM_XPOS_NOR_MIN) ||
        (calibration_param->xpos_nor > TCAM_TM30_CALIBRATION_PARAM_XPOS_NOR_MAX) ||
        (calibration_param->ypos_nor < TCAM_TM30_CALIBRATION_PARAM_YPOS_NOR_MIN) ||
        (calibration_param->ypos_nor > TCAM_TM30_CALIBRATION_PARAM_YPOS_NOR_MAX)) {
        return TCAM_ERROR;
    }

    send_data[0U] = (uint8_t)(((calibration_param->distance_dm * 10U) >> 8) & 0xFFU);
    send_data[1U] = (uint8_t)((calibration_param->distance_dm * 10U) & 0xFFU);
    send_data[2U] = (uint8_t)((calibration_param->atmospheric_temp_add300_mult10 >> 8) & 0xFFU);
    send_data[3U] = (uint8_t)(calibration_param->atmospheric_temp_add300_mult10 & 0xFFU);
    send_data[4U] = (uint8_t)((calibration_param->emissivity_percent >> 8) & 0xFFU);
    send_data[5U] = (uint8_t)(calibration_param->emissivity_percent & 0xFFU);
    send_data[6U] = 0x01U;
    send_data[7U] = (uint8_t)((calibration_param->calibration_temp_add100_mult10 >> 8) & 0xFFU);
    send_data[8U] = (uint8_t)(calibration_param->calibration_temp_add100_mult10 & 0xFFU);
    send_data[9U] = (uint8_t)((calibration_param->xpos_nor >> 8) & 0xFFU);
    send_data[10U] = (uint8_t)(calibration_param->xpos_nor & 0xFFU);
    send_data[11U] = (uint8_t)((calibration_param->ypos_nor >> 8) & 0xFFU);
    send_data[12U] = (uint8_t)(calibration_param->ypos_nor & 0xFFU);
    if (cmd_write(tcam, class, sub, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_tm30_do_calibration(const tcam_t *tcam, tcam_tm30_calibration_state_t calibration_state)
{
    const uint8_t class = 0x7DU;
    const uint8_t sub = 0x12U;
    uint8_t send_data[1U];
    const uint8_t send_data_idx0[TCAM_TM30_CALIBRATION_STATE_COUNT] = {
        [TCAM_TM30_CALIBRATION_STATE_DISABLE] = 0x00U,
        [TCAM_TM30_CALIBRATION_STATE_ENABLE] = 0x01U,
    };

    if ((tcam_tm30_object_check(tcam) != TCAM_OK) ||
        ((uint32_t)calibration_state >= (uint32_t)TCAM_TM30_CALIBRATION_STATE_COUNT)) {
        return TCAM_ERROR;
    }

    send_data[0U] = send_data_idx0[calibration_state];
    if (cmd_write(tcam, class, sub, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_tm30_set_yuv_resolution(const tcam_t *tcam, tcam_tm30_yuv_resolution_t yuv_resolution)
{
    const uint8_t class = 0x70U;
    const uint8_t sub = 0x4AU;
    uint8_t send_data[6U];
    const uint8_t send_data_idx2_idx5[TCAM_TM30_YUV_RESOLUTION_COUNT] = {
        [TCAM_TM30_YUV_RESOLUTION_96X96] = 0x60U,
        [TCAM_TM30_YUV_RESOLUTION_240X240] = 0xF0U,
    };

    if ((tcam_tm30_object_check(tcam) != TCAM_OK) ||
        ((uint32_t)yuv_resolution >= (uint32_t)TCAM_TM30_YUV_RESOLUTION_COUNT)) {
        return TCAM_ERROR;
    }

    send_data[0U] = 0x00U;
    send_data[1U] = 0x00U;
    send_data[2U] = send_data_idx2_idx5[yuv_resolution];
    send_data[3U] = 0x00U;
    send_data[4U] = 0x00U;
    send_data[5U] = send_data_idx2_idx5[yuv_resolution];
    if (cmd_write(tcam, class, sub, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_tm30_set_humidity(const tcam_t *tcam, uint16_t humidity_permille)
{
    const uint8_t class = 0x7DU;
    const uint8_t sub = 0x1AU;
    uint8_t send_data[2U];

    if ((tcam_tm30_object_check(tcam) != TCAM_OK) ||
        (humidity_permille < TCAM_TM30_HUMIDITY_PERMILLE_MIN) ||
        (humidity_permille > TCAM_TM30_HUMIDITY_PERMILLE_MAX)) {
        return TCAM_ERROR;
    }

    send_data[0U] = (uint8_t)((humidity_permille >> 8) & 0xFFU);
    send_data[1U] = (uint8_t)(humidity_permille & 0xFFU);
    if (cmd_write(tcam, class, sub, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_tm30_get_humidity(const tcam_t *tcam, uint16_t *humidity_permille)
{
    const uint8_t class = 0x7DU;
    const uint8_t sub = 0x1AU;
    uint8_t recv_data[2U];

    if ((tcam_tm30_object_check(tcam) != TCAM_OK) ||
        (humidity_permille == NULL)) {
        return TCAM_ERROR;
    }

    if (cmd_read(tcam, class, sub, recv_data, sizeof(recv_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    if (((uint16_t)(((uint16_t)recv_data[0U] << 8) | (uint16_t)recv_data[1U]) < TCAM_TM30_HUMIDITY_PERMILLE_MIN) ||
        ((uint16_t)(((uint16_t)recv_data[0U] << 8) | (uint16_t)recv_data[1U]) > TCAM_TM30_HUMIDITY_PERMILLE_MAX)) {
        return TCAM_ERROR;
    }

    *humidity_permille = (uint16_t)(((uint16_t)recv_data[0U] << 8) | (uint16_t)recv_data[1U]);

    return TCAM_OK;
}

tcam_status_t tcam_tm30_set_atmospheric_temp(const tcam_t *tcam, tcam_tm30_atmospheric_temp_state_t atmospheric_temp_state, uint16_t atmospheric_temp_add100_mult10)
{
    const uint8_t class = 0x7DU;
    const uint8_t sub = 0x1BU;
    uint8_t send_data[4U];
    const uint8_t send_data_idx0[TCAM_TM30_ATMOSPHERIC_TEMP_STATE_COUNT] = {
        [TCAM_TM30_ATMOSPHERIC_TEMP_STATE_DISABLE] = 0x00U,
        [TCAM_TM30_ATMOSPHERIC_TEMP_STATE_ENABLE] = 0x01U,
    };

    if ((tcam_tm30_object_check(tcam) != TCAM_OK) ||
        ((uint32_t)atmospheric_temp_state >= (uint32_t)TCAM_TM30_ATMOSPHERIC_TEMP_STATE_COUNT) ||
        (atmospheric_temp_add100_mult10 < TCAM_TM30_ATMOSPHERIC_TEMP_ADD100_MULT10_MIN) ||
        (atmospheric_temp_add100_mult10 > TCAM_TM30_ATMOSPHERIC_TEMP_ADD100_MULT10_MAX)) {
        return TCAM_ERROR;
    }

    send_data[0U] = send_data_idx0[atmospheric_temp_state];
    send_data[1U] = send_data[0U];
    send_data[2U] = (uint8_t)((atmospheric_temp_add100_mult10 >> 8) & 0xFFU);
    send_data[3U] = (uint8_t)(atmospheric_temp_add100_mult10 & 0xFFU);
    if (cmd_write(tcam, class, sub, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_tm30_get_atmospheric_temp(const tcam_t *tcam, tcam_tm30_atmospheric_temp_state_t *atmospheric_temp_state, uint16_t *atmospheric_temp_add100_mult10)
{
    const uint8_t class = 0x7DU;
    const uint8_t sub = 0x1BU;
    uint8_t recv_data[4U];

    if ((tcam_tm30_object_check(tcam) != TCAM_OK) ||
        (atmospheric_temp_state == NULL) ||
        (atmospheric_temp_add100_mult10 == NULL)) {
        return TCAM_ERROR;
    }

    if (cmd_read(tcam, class, sub, recv_data, sizeof(recv_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    if (((uint16_t)(((uint16_t)recv_data[2U] << 8) | (uint16_t)recv_data[3U]) < TCAM_TM30_ATMOSPHERIC_TEMP_ADD100_MULT10_MIN) ||
        ((uint16_t)(((uint16_t)recv_data[2U] << 8) | (uint16_t)recv_data[3U]) > TCAM_TM30_ATMOSPHERIC_TEMP_ADD100_MULT10_MAX)) {
        return TCAM_ERROR;
    }
    *atmospheric_temp_add100_mult10 = (uint16_t)(((uint16_t)recv_data[2U] << 8) | (uint16_t)recv_data[3U]);

    switch (recv_data[0U]) {
        case 0x00U: *atmospheric_temp_state = TCAM_TM30_ATMOSPHERIC_TEMP_STATE_DISABLE; break;
        case 0x01U: *atmospheric_temp_state = TCAM_TM30_ATMOSPHERIC_TEMP_STATE_ENABLE; break;
        default: return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_tm30_stream_start(const tcam_t *tcam)
{
    if (tcam_tm30_object_check(tcam) != TCAM_OK) {
        return TCAM_ERROR;
    }

    if (tcam->ops->stream.configure(STREAM_FRAME_SIZE) != TCAM_OK) {
        return TCAM_ERROR;
    }

    if (tcam->ops->stream.start() != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_tm30_stream_frame_acquire(const tcam_t *tcam, void **frame, uint32_t timeout_ms)
{
    if ((tcam_tm30_object_check(tcam) != TCAM_OK) ||
        (frame == NULL)) {
        return TCAM_ERROR;
    }

    if (tcam->ops->stream.acquire_frame(frame, timeout_ms) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_tm30_stream_frame_release(const tcam_t *tcam)
{
    if (tcam_tm30_object_check(tcam) != TCAM_OK) {
        return TCAM_ERROR;
    }

    if (tcam->ops->stream.release_frame() != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_tm30_stream_frame_decode(const tcam_t *tcam, const void *frame, tcam_tm30_stream_frame_info_t *frame_info)
{
    const void *frame_param;
    stream_frame_param_t param;

    if ((tcam_tm30_object_check(tcam) != TCAM_OK) ||
        (frame == NULL) ||
        (frame_info == NULL)) {
        return TCAM_ERROR;
    }

    frame_param = (const void *)((const uint8_t *)frame + STREAM_FRAME_PARAM_OFFSET);

    param.head = UTIL_READ_U32_LE(frame_param, STREAM_FRAME_PARAM_HEAD_OFFSET);
    if (param.head != STREAM_FRAME_PARAM_HEAD) {
        return TCAM_ERROR;
    }

    param.freezed = UTIL_READ_U32_LE(frame_param, STREAM_FRAME_PARAM_FREEZED_OFFSET);
    param.min_temp = UTIL_READ_U32_LE(frame_param, STREAM_FRAME_PARAM_MIN_TEMP_OFFSET);
    param.max_temp = UTIL_READ_U32_LE(frame_param, STREAM_FRAME_PARAM_MAX_TEMP_OFFSET);
    param.avg_temp = UTIL_READ_U32_LE(frame_param, STREAM_FRAME_PARAM_AVG_TEMP_OFFSET);
    param.max_x = UTIL_READ_U32_LE(frame_param, STREAM_FRAME_PARAM_MAX_X_OFFSET);
    param.max_y = UTIL_READ_U32_LE(frame_param, STREAM_FRAME_PARAM_MAX_Y_OFFSET);
    param.min_x = UTIL_READ_U32_LE(frame_param, STREAM_FRAME_PARAM_MIN_X_OFFSET);
    param.min_y = UTIL_READ_U32_LE(frame_param, STREAM_FRAME_PARAM_MIN_Y_OFFSET);

    frame_info->freezed = (param.freezed == 0U) ? false : true;
    frame_info->max_point.x = UTIL_MAP_U16_RANGE(param.max_x, 0U, 1000U, 0U, 95U);
    frame_info->max_point.y = UTIL_MAP_U16_RANGE(param.max_y, 0U, 1000U, 0U, 95U);
    frame_info->max_point.temp_mult10 = (int16_t)(UTIL_U32_TO_FLOAT(param.max_temp) * 10.0f);
    frame_info->min_point.x = UTIL_MAP_U16_RANGE(param.min_x, 0U, 1000U, 0U, 95U);
    frame_info->min_point.y = UTIL_MAP_U16_RANGE(param.min_y, 0U, 1000U, 0U, 95U);
    frame_info->min_point.temp_mult10 = (int16_t)(UTIL_U32_TO_FLOAT(param.min_temp) * 10.0f);
    frame_info->avg_temp_mult10 = (int16_t)(UTIL_U32_TO_FLOAT(param.avg_temp) * 10.0f);
    frame_info->mt_matrix.data = (const uint8_t *)frame + STREAM_FRAME_MT_OFFSET;
    frame_info->mt_matrix.width = STREAM_FRAME_MT_WIDTH;
    frame_info->mt_matrix.height = STREAM_FRAME_MT_HEIGHT;
    frame_info->mt_matrix.bytes_per_item = STREAM_FRAME_MT_BYTES_PER_ITEM;
    frame_info->mt_matrix.stride_item = STREAM_FRAME_MT_STRIDE_ITEM;
    frame_info->yuv_matrix.data = (const uint8_t *)frame + STREAM_FRAME_YUV_OFFSET;
    frame_info->yuv_matrix.width = STREAM_FRAME_YUV_WIDTH;
    frame_info->yuv_matrix.height = STREAM_FRAME_YUV_HEIGHT;
    frame_info->yuv_matrix.bytes_per_item = STREAM_FRAME_YUV_BYTES_PER_ITEM;
    frame_info->yuv_matrix.stride_item = STREAM_FRAME_YUV_STRIDE_ITEM;

    return TCAM_OK;
}
