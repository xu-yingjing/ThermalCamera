#include "tcam_mini212.h"
#include <stddef.h>
#include <string.h>

#define CMD_HEAD_LEN      (2U)
#define CMD_SIZE_LEN      (1U)
#define CMD_DATA_MAX_LEN  (19U)
#define CMD_CHECKSUM_LEN  (1U)
#define CMD_TAIL_LEN      (1U)
#define CMD_LEN(data_len) (CMD_HEAD_LEN +     \
                           CMD_SIZE_LEN +     \
                           (data_len) +       \
                           CMD_CHECKSUM_LEN + \
                           CMD_TAIL_LEN)

#define CMD_HEAD_IDX(data_len)     (0U)
#define CMD_SIZE_IDX(data_len)     (CMD_HEAD_IDX((data_len)) + CMD_HEAD_LEN)
#define CMD_DATA_IDX(data_len)     (CMD_SIZE_IDX((data_len)) + CMD_SIZE_LEN)
#define CMD_CHECKSUM_IDX(data_len) (CMD_DATA_IDX((data_len)) + (data_len))
#define CMD_TAIL_IDX(data_len)     (CMD_CHECKSUM_IDX((data_len)) + CMD_CHECKSUM_LEN)

#define CMD_HEAD_0         (0x55U)
#define CMD_HEAD_1         (0xAAU)
#define CMD_SIZE(data_len) (data_len)
#define CMD_TAIL           (0xF0U)

#define CMD_MAX_LEN CMD_LEN(CMD_DATA_MAX_LEN)

#define CMD_FLUSH_IDLE_MS (2U)

#define CMD_SEND_TIMEOUT_MS  (20U)
#define CMD_RECV_TIMEOUT_MS  (200U)
#define CMD_FLUSH_TIMEOUT_MS (20U)

#define STREAM_FRAME_WIDTH     (512U)
#define STREAM_FRAME_HEIGHT    (193U)
#define STREAM_FRAME_LINE_SIZE (STREAM_FRAME_WIDTH * 2U)
#define STREAM_FRAME_SIZE      (STREAM_FRAME_LINE_SIZE * STREAM_FRAME_HEIGHT)

#define STREAM_FRAME_MT_OFFSET         (0U)
#define STREAM_FRAME_MT_WIDTH          (256U)
#define STREAM_FRAME_MT_HEIGHT         (192U)
#define STREAM_FRAME_MT_BYTES_PER_ITEM (2U)
#define STREAM_FRAME_MT_STRIDE_ITEM    (STREAM_FRAME_LINE_SIZE / STREAM_FRAME_MT_BYTES_PER_ITEM)

#define STREAM_FRAME_YUV_OFFSET         (STREAM_FRAME_MT_OFFSET + STREAM_FRAME_MT_WIDTH * STREAM_FRAME_MT_BYTES_PER_ITEM)
#define STREAM_FRAME_YUV_WIDTH          (256U)
#define STREAM_FRAME_YUV_HEIGHT         (192U)
#define STREAM_FRAME_YUV_BYTES_PER_ITEM (2U)
#define STREAM_FRAME_YUV_STRIDE_ITEM    (STREAM_FRAME_LINE_SIZE / STREAM_FRAME_YUV_BYTES_PER_ITEM)

#define STREAM_FRAME_PARAM_OFFSET (STREAM_FRAME_MT_OFFSET + STREAM_FRAME_LINE_SIZE * STREAM_FRAME_MT_HEIGHT)
#define STREAM_FRAME_PARAM_HEAD1  (0x55AAU)
#define STREAM_FRAME_PARAM_HEAD2  (0x0038U)
#define STREAM_FRAME_PARAM_TAIL   (0x6666U)

typedef struct {
    uint16_t head1;
    uint16_t head2;
    uint16_t humidity_percent;
    uint16_t distance_dm;
    uint16_t emissivity_percent;
    uint16_t reflected_temp_mult10;
    uint16_t reserved0[22U];
    uint16_t shutter_off;
    uint16_t reserved1[15U];
    uint16_t max_x;
    uint16_t max_y;
    uint16_t max_temp_mult10;
    uint16_t min_x;
    uint16_t min_y;
    uint16_t min_temp_mult10;
    uint16_t cur_x;
    uint16_t cur_y;
    uint16_t cur_temp_mult10;
    uint16_t avg_temp_mult10;
    uint16_t reserved2[5];
    uint16_t tail;
} stream_frame_param_t;

static tcam_status_t cmd_encode(const uint8_t *data, uint8_t data_len, uint8_t *cmd, uint32_t cmd_size, uint8_t *cmd_len)
{
    uint8_t i;

    if (data_len > CMD_DATA_MAX_LEN) {
        return TCAM_ERROR;
    }

    if (cmd_size < CMD_LEN(data_len)) {
        return TCAM_ERROR;
    }

    cmd[CMD_HEAD_IDX(data_len)] = CMD_HEAD_0;
    cmd[CMD_HEAD_IDX(data_len) + 1U] = CMD_HEAD_1;

    cmd[CMD_SIZE_IDX(data_len)] = CMD_SIZE(data_len);

    for (i = 0U; i < data_len; i++) {
        cmd[CMD_DATA_IDX(data_len) + i] = data[i];
    }

    cmd[CMD_CHECKSUM_IDX(data_len)] = 0U;
    for (i = CMD_SIZE_IDX(data_len); i < CMD_CHECKSUM_IDX(data_len); i++) {
        cmd[CMD_CHECKSUM_IDX(data_len)] ^= cmd[i];
    }

    cmd[CMD_TAIL_IDX(data_len)] = CMD_TAIL;

    *cmd_len = CMD_LEN(data_len);

    return TCAM_OK;
}

static tcam_status_t cmd_decode(const uint8_t *cmd, uint8_t cmd_len, const uint8_t **data, uint8_t *data_len)
{
    uint8_t size;
    uint8_t checksum;
    uint8_t i;

    if ((cmd_len < CMD_LEN(0U)) ||
        (cmd_len > CMD_MAX_LEN)) {
        return TCAM_ERROR;
    }

    if ((cmd[CMD_HEAD_IDX(0U)] != CMD_HEAD_0) ||
        (cmd[CMD_HEAD_IDX(0U) + 1U] != CMD_HEAD_1)) {
        return TCAM_ERROR;
    }

    size = cmd[CMD_SIZE_IDX(0U)];
    if (size > CMD_SIZE(CMD_DATA_MAX_LEN)) {
        return TCAM_ERROR;
    }

    if (cmd_len != CMD_LEN(size - CMD_SIZE(0U))) {
        return TCAM_ERROR;
    }

    if (cmd[CMD_TAIL_IDX(size - CMD_SIZE(0U))] != CMD_TAIL) {
        return TCAM_ERROR;
    }

    checksum = 0U;
    for (i = CMD_SIZE_IDX(size - CMD_SIZE(0U)); i < CMD_CHECKSUM_IDX(size - CMD_SIZE(0U)); i++) {
        checksum ^= cmd[i];
    }
    if (checksum != cmd[CMD_CHECKSUM_IDX(size - CMD_SIZE(0U))]) {
        return TCAM_ERROR;
    }

    *data = &cmd[CMD_DATA_IDX(size - CMD_SIZE(0U))];
    *data_len = size - CMD_SIZE(0U);

    return TCAM_OK;
}

static tcam_status_t cmd_recv_ack(const tcam_t *tcam)
{
    const uint8_t ack_data[1U] = {0x00U};
    uint8_t cmd[CMD_LEN(sizeof(ack_data))];
    const uint8_t *data;
    uint8_t data_len;

    if (tcam->ops->cmd.recv(cmd, sizeof(cmd), CMD_RECV_TIMEOUT_MS) != TCAM_OK) {
        return TCAM_ERROR;
    }

    if (cmd_decode(cmd, sizeof(cmd), &data, &data_len) != TCAM_OK) {
        return TCAM_ERROR;
    }

    if ((data_len != sizeof(ack_data)) ||
        (memcmp(data, ack_data, sizeof(ack_data)) != 0U)) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

static tcam_status_t cmd_read(const tcam_t *tcam, const uint8_t *send_data, uint8_t send_data_len, uint8_t *recv_data, uint8_t recv_data_len)
{
    uint8_t cmd[CMD_MAX_LEN];
    uint8_t cmd_len;
    const uint8_t *data;
    uint8_t data_len;

    if (cmd_encode(send_data, send_data_len, cmd, sizeof(cmd), &cmd_len) != TCAM_OK) {
        return TCAM_ERROR;
    }

    if (tcam->ops->cmd.flush_rx(CMD_FLUSH_IDLE_MS, CMD_FLUSH_TIMEOUT_MS) != TCAM_OK) {
        return TCAM_ERROR;
    }

    if (tcam->ops->cmd.send(cmd, cmd_len, CMD_SEND_TIMEOUT_MS) != TCAM_OK) {
        return TCAM_ERROR;
    }

    if (cmd_recv_ack(tcam) != TCAM_OK) {
        return TCAM_ERROR;
    }

    if (tcam->ops->cmd.recv(cmd, CMD_LEN(recv_data_len), CMD_RECV_TIMEOUT_MS) != TCAM_OK) {
        return TCAM_ERROR;
    }

    if (cmd_decode(cmd, CMD_LEN(recv_data_len), &data, &data_len) != TCAM_OK) {
        return TCAM_ERROR;
    }

    if (data_len != recv_data_len) {
        return TCAM_ERROR;
    }

    (void)memcpy(recv_data, data, recv_data_len);

    return TCAM_OK;
}

static tcam_status_t cmd_write(const tcam_t *tcam, const uint8_t *send_data, uint8_t send_data_len)
{
    uint8_t cmd[CMD_MAX_LEN];
    uint8_t cmd_len;

    if (cmd_encode(send_data, send_data_len, cmd, sizeof(cmd), &cmd_len) != TCAM_OK) {
        return TCAM_ERROR;
    }

    if (tcam->ops->cmd.flush_rx(CMD_FLUSH_IDLE_MS, CMD_FLUSH_TIMEOUT_MS) != TCAM_OK) {
        return TCAM_ERROR;
    }

    if (tcam->ops->cmd.send(cmd, cmd_len, CMD_SEND_TIMEOUT_MS) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return cmd_recv_ack(tcam);
}

static tcam_status_t tcam_mini212_object_check(const tcam_t *tcam)
{
    if ((tcam == NULL) ||
        (tcam->model != TCAM_MODEL_MINI212) ||
        (tcam->ops == NULL)) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_mini212_probe(const tcam_t *tcam)
{
    tcam_mini212_status_page_t status_page;

    if (tcam_mini212_object_check(tcam) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return tcam_mini212_get_status_page(tcam, &status_page);
}

tcam_status_t tcam_mini212_get_status_page(const tcam_t *tcam, tcam_mini212_status_page_t *status_page)
{
    const uint8_t send_data[7U] = {0x00U, 0x00U, 0x80U, 0x00U, 0x00U, 0x00U, 0x00U};
    uint8_t recv_data[19U];

    if ((tcam_mini212_object_check(tcam) != TCAM_OK) ||
        (status_page == NULL)) {
        return TCAM_ERROR;
    }

    if (cmd_read(tcam, send_data, sizeof(send_data), recv_data, sizeof(recv_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    if ((recv_data[0U] != 0x00U) ||
        (recv_data[1U] != 0x00U)) {
        return TCAM_ERROR;
    }

    status_page->pid = recv_data[2U];
    status_page->firmware_version.year = recv_data[4U];
    status_page->firmware_version.month = recv_data[5U];
    status_page->firmware_version.day = recv_data[6U];
    status_page->focal_temp_mult100 = (uint16_t)(((uint16_t)recv_data[7U] << 8) | (uint16_t)recv_data[8U]);
    status_page->machine_id = (uint32_t)(((uint32_t)recv_data[11U] << 24) | ((uint32_t)recv_data[12U] << 16) | ((uint32_t)recv_data[13U] << 8) | (uint32_t)recv_data[14U]);

    return TCAM_OK;
}

tcam_status_t tcam_mini212_get_digital_video_page(const tcam_t *tcam, tcam_mini212_digital_video_page_t *digital_video_page)
{
    const uint8_t send_data[7U] = {0x02U, 0x01U, 0x80U, 0x00U, 0x00U, 0x00U, 0x00U};
    uint8_t recv_data[19U];
    tcam_mini212_digital_video_external_sync_mode_t external_sync_mode;
    tcam_mini212_digital_video_digital_type_t digital_type;
    tcam_mini212_digital_video_cmos_content_t cmos_content;
    tcam_mini212_digital_video_cmos_interface_t cmos_interface;
    tcam_mini212_digital_video_frame_frequency_t frame_frequency;
    tcam_mini212_digital_video_clock_phase_t clock_phase;

    if ((tcam_mini212_object_check(tcam) != TCAM_OK) ||
        (digital_video_page == NULL)) {
        return TCAM_ERROR;
    }

    if (cmd_read(tcam, send_data, sizeof(send_data), recv_data, sizeof(recv_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    if ((recv_data[0U] != 0x02U) ||
        (recv_data[1U] != 0x01U)) {
        return TCAM_ERROR;
    }
    
    external_sync_mode = (tcam_mini212_digital_video_external_sync_mode_t)recv_data[2U];
    switch (external_sync_mode) {
        case TCAM_MINI212_DIGITAL_VIDEO_EXTERNAL_SYNC_MODE_DISABLE:
        case TCAM_MINI212_DIGITAL_VIDEO_EXTERNAL_SYNC_MODE_SLAVE:
        case TCAM_MINI212_DIGITAL_VIDEO_EXTERNAL_SYNC_MODE_MASTER:
            break;

        default:
            return TCAM_ERROR;
    }
    
    digital_type = (tcam_mini212_digital_video_digital_type_t)recv_data[3U];
    switch (digital_type) {
        case TCAM_MINI212_DIGITAL_VIDEO_DIGITAL_TYPE_DISABLE:
        case TCAM_MINI212_DIGITAL_VIDEO_DIGITAL_TYPE_USB20:
        case TCAM_MINI212_DIGITAL_VIDEO_DIGITAL_TYPE_CMOS:
        case TCAM_MINI212_DIGITAL_VIDEO_DIGITAL_TYPE_BT1120:
        case TCAM_MINI212_DIGITAL_VIDEO_DIGITAL_TYPE_BT656:
        case TCAM_MINI212_DIGITAL_VIDEO_DIGITAL_TYPE_USB20_UART:
        case TCAM_MINI212_DIGITAL_VIDEO_DIGITAL_TYPE_LCD:
        case TCAM_MINI212_DIGITAL_VIDEO_DIGITAL_TYPE_LVDS:
        case TCAM_MINI212_DIGITAL_VIDEO_DIGITAL_TYPE_LCD_DVP:
        case TCAM_MINI212_DIGITAL_VIDEO_DIGITAL_TYPE_UVC_CDC:
            break;

        default:
            return TCAM_ERROR;
    }
    
    cmos_content = (tcam_mini212_digital_video_cmos_content_t)recv_data[4U];
    switch (cmos_content) {
        case TCAM_MINI212_DIGITAL_VIDEO_CMOS_CONTENT_YUV422:
        case TCAM_MINI212_DIGITAL_VIDEO_CMOS_CONTENT_YUV422_PARAM:
        case TCAM_MINI212_DIGITAL_VIDEO_CMOS_CONTENT_Y16:
        case TCAM_MINI212_DIGITAL_VIDEO_CMOS_CONTENT_Y16_PARAM:
        case TCAM_MINI212_DIGITAL_VIDEO_CMOS_CONTENT_Y16_YUV422:
        case TCAM_MINI212_DIGITAL_VIDEO_CMOS_CONTENT_Y16_PARAM_YUV422:
        case TCAM_MINI212_DIGITAL_VIDEO_CMOS_CONTENT_TMP:
        case TCAM_MINI212_DIGITAL_VIDEO_CMOS_CONTENT_TMP_PARAM:
        case TCAM_MINI212_DIGITAL_VIDEO_CMOS_CONTENT_TMP_YUV422:
        case TCAM_MINI212_DIGITAL_VIDEO_CMOS_CONTENT_TMP_PARAM_YUV422:
            break;

        default:
            return TCAM_ERROR;
    }
    
    cmos_interface = (tcam_mini212_digital_video_cmos_interface_t)recv_data[5U];
    switch (cmos_interface) {
        case TCAM_MINI212_DIGITAL_VIDEO_CMOS_INTERFACE_CMOS16:
        case TCAM_MINI212_DIGITAL_VIDEO_CMOS_INTERFACE_CMOS8_MSB:
        case TCAM_MINI212_DIGITAL_VIDEO_CMOS_INTERFACE_CMOS8_LSB:
            break;

        default:
            return TCAM_ERROR;
    }
    
    frame_frequency = (tcam_mini212_digital_video_frame_frequency_t)recv_data[6U];
    switch (frame_frequency) {
        case TCAM_MINI212_DIGITAL_VIDEO_FRAME_FREQUENCY_30Hz:
        case TCAM_MINI212_DIGITAL_VIDEO_FRAME_FREQUENCY_25Hz:
        case TCAM_MINI212_DIGITAL_VIDEO_FRAME_FREQUENCY_9Hz:
        case TCAM_MINI212_DIGITAL_VIDEO_FRAME_FREQUENCY_50Hz:
            break;

        default:
            return TCAM_ERROR;
    }
    
    clock_phase = (tcam_mini212_digital_video_clock_phase_t)recv_data[8U];
    switch (clock_phase) {
        case TCAM_MINI212_DIGITAL_VIDEO_CLOCK_PHASE_POSEDGE:
        case TCAM_MINI212_DIGITAL_VIDEO_CLOCK_PHASE_NEGEDGE:
            break;

        default:
            return TCAM_ERROR;
    }

    digital_video_page->external_sync_mode = external_sync_mode;
    digital_video_page->digital_type = digital_type;
    digital_video_page->cmos_content = cmos_content;
    digital_video_page->cmos_interface = cmos_interface;
    digital_video_page->frame_frequency = frame_frequency;
    digital_video_page->clock_phase = clock_phase;

    return TCAM_OK;
}

tcam_status_t tcam_mini212_get_settings_page(const tcam_t *tcam, tcam_mini212_settings_page_t *settings_page)
{
    const uint8_t send_data[7U] = {0x01U, 0x00U, 0x80U, 0x00U, 0x00U, 0x00U, 0x00U};
    uint8_t recv_data[19U];
    tcam_mini212_settings_temp_rise_compensation_t temp_rise_compensation;

    if ((tcam_mini212_object_check(tcam) != TCAM_OK) ||
        (settings_page == NULL)) {
        return TCAM_ERROR;
    }

    if (cmd_read(tcam, send_data, sizeof(send_data), recv_data, sizeof(recv_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    if ((recv_data[0U] != 0x01U) ||
        (recv_data[1U] != 0x00U)) {
        return TCAM_ERROR;
    }
    
    temp_rise_compensation = (tcam_mini212_settings_temp_rise_compensation_t)recv_data[5U];
    switch (temp_rise_compensation) {
        case TCAM_MINI212_SETTINGS_TEMP_RISE_COMPENSATION_DISABLE:
        case TCAM_MINI212_SETTINGS_TEMP_RISE_COMPENSATION_ENABLE:
            break;

        default:
            return TCAM_ERROR;
    }

    settings_page->auto_compensation_time = recv_data[2U];
    settings_page->temp_rise_compensation = temp_rise_compensation;

    return TCAM_OK;
}

tcam_status_t tcam_mini212_get_focus_page(const tcam_t *tcam, tcam_mini212_focus_page_t *focus_page)
{
    const uint8_t send_data[7U] = {0x03U, 0x00U, 0x80U, 0x00U, 0x00U, 0x00U, 0x00U};
    uint8_t recv_data[19U];

    if ((tcam_mini212_object_check(tcam) != TCAM_OK) ||
        (focus_page == NULL)) {
        return TCAM_ERROR;
    }

    if (cmd_read(tcam, send_data, sizeof(send_data), recv_data, sizeof(recv_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    if ((recv_data[0U] != 0x03U) ||
        (recv_data[1U] != 0x00U)) {
        return TCAM_ERROR;
    }

    focus_page->lens_type = recv_data[2U];

    return TCAM_OK;
}

tcam_status_t tcam_mini212_get_custom_data(const tcam_t *tcam, tcam_mini212_custom_data_t *custom_data)
{
    const uint8_t send_data[7U] = {0xB0U, 0x01U, 0x80U, 0x00U, 0x00U, 0x00U, 0x00U};
    uint8_t recv_data[19U];

    if ((tcam_mini212_object_check(tcam) != TCAM_OK) ||
        (custom_data == NULL)) {
        return TCAM_ERROR;
    }

    if (cmd_read(tcam, send_data, sizeof(send_data), recv_data, sizeof(recv_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    if ((recv_data[0U] != 0xB0U) ||
        (recv_data[1U] != 0x01U)) {
        return TCAM_ERROR;
    }

    (void)memcpy(custom_data->data, &recv_data[2U], sizeof(custom_data->data));

    return TCAM_OK;
}

tcam_status_t tcam_mini212_do_shutter_compensation(const tcam_t *tcam)
{
    const uint8_t send_data[7U] = {0x02U, 0x01U, 0x08U, 0x00U, 0x00U, 0x00U, 0x01U};

    if (tcam_mini212_object_check(tcam) != TCAM_OK) {
        return TCAM_ERROR;
    }

    if (cmd_write(tcam, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_mini212_do_scene_compensation(const tcam_t *tcam)
{
    const uint8_t send_data[7U] = {0x02U, 0x01U, 0x07U, 0x00U, 0x00U, 0x00U, 0x01U};

    if (tcam_mini212_object_check(tcam) != TCAM_OK) {
        return TCAM_ERROR;
    }

    if (cmd_write(tcam, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_mini212_set_image_freeze(const tcam_t *tcam, tcam_mini212_image_freeze_t image_freeze)
{
    uint8_t send_data[7U] = {0x01U, 0x00U, 0x02U, 0x00U, 0x00U, 0x00U};
    const uint8_t send_data_idx6[TCAM_MINI212_IMAGE_FREEZE_COUNT] = {
        [TCAM_MINI212_IMAGE_FREEZE_ENABLE] = 0x01U,
        [TCAM_MINI212_IMAGE_FREEZE_DISABLE] = 0x00U,
    };

    if ((tcam_mini212_object_check(tcam) != TCAM_OK) ||
        ((uint32_t)image_freeze >= (uint32_t)TCAM_MINI212_IMAGE_FREEZE_COUNT)) {
        return TCAM_ERROR;
    }

    send_data[6U] = send_data_idx6[image_freeze];
    if (cmd_write(tcam, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_mini212_set_image_color(const tcam_t *tcam, tcam_mini212_image_color_t image_color)
{
    uint8_t send_data[7U] = {0x02U, 0x00U, 0x04U, 0x00U, 0x00U, 0x00U};
    const uint8_t send_data_idx6[TCAM_MINI212_IMAGE_COLOR_COUNT] = {
        [TCAM_MINI212_IMAGE_COLOR_WHITE_HOT] = 0x00U,
        [TCAM_MINI212_IMAGE_COLOR_BLACK_HOT] = 0x09U,
        [TCAM_MINI212_IMAGE_COLOR_RED_OUTLINE] = 0x08U,
        [TCAM_MINI212_IMAGE_COLOR_FULGURITE] = 0x01U,
        [TCAM_MINI212_IMAGE_COLOR_IRON_RED] = 0x02U,
        [TCAM_MINI212_IMAGE_COLOR_HOT_IRON] = 0x03U,
        [TCAM_MINI212_IMAGE_COLOR_MEDICAL] = 0x04U,
        [TCAM_MINI212_IMAGE_COLOR_ARCTIC] = 0x05U,
        [TCAM_MINI212_IMAGE_COLOR_RAINBOW_1] = 0x06U,
        [TCAM_MINI212_IMAGE_COLOR_RAINBOW_2] = 0x07U,
    };

    if ((tcam_mini212_object_check(tcam) != TCAM_OK) ||
        ((uint32_t)image_color >= (uint32_t)TCAM_MINI212_IMAGE_COLOR_COUNT)) {
        return TCAM_ERROR;
    }

    send_data[6U] = send_data_idx6[image_color];
    if (cmd_write(tcam, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_mini212_set_image_tone(const tcam_t *tcam, tcam_mini212_image_tone_t image_tone)
{
    uint8_t send_data[7U] = {0x02U, 0x02U, 0x19U, 0x00U, 0x00U, 0x00U};
    const uint8_t send_data_idx6[TCAM_MINI212_IMAGE_TONE_COUNT] = {
        [TCAM_MINI212_IMAGE_TONE_WARM] = 0x00U,
        [TCAM_MINI212_IMAGE_TONE_COOL] = 0x01U,
        [TCAM_MINI212_IMAGE_TONE_GREEN_HOT] = 0x02U,
    };

    if ((tcam_mini212_object_check(tcam) != TCAM_OK) ||
        ((uint32_t)image_tone >= (uint32_t)TCAM_MINI212_IMAGE_TONE_COUNT)) {
        return TCAM_ERROR;
    }

    send_data[6U] = send_data_idx6[image_tone];
    if (cmd_write(tcam, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_mini212_set_image_mode_preset(const tcam_t *tcam, tcam_mini212_image_mode_preset_t image_mode_preset)
{
    uint8_t send_data[7U] = {0x02U, 0x02U, 0x06U, 0x00U, 0x00U, 0x00U};
    const uint8_t send_data_idx6[TCAM_MINI212_IMAGE_MODE_PRESET_COUNT] = {
        [TCAM_MINI212_IMAGE_MODE_PRESET_SOFT] = 0x00U,
        [TCAM_MINI212_IMAGE_MODE_PRESET_STANDARD] = 0x01U,
        [TCAM_MINI212_IMAGE_MODE_PRESET_ENHANCED] = 0x02U,
        [TCAM_MINI212_IMAGE_MODE_PRESET_HIGHLIGHT] = 0x03U,
        [TCAM_MINI212_IMAGE_MODE_PRESET_CUSTOM] = 0x10U,
    };

    if ((tcam_mini212_object_check(tcam) != TCAM_OK) ||
        ((uint32_t)image_mode_preset >= (uint32_t)TCAM_MINI212_IMAGE_MODE_PRESET_COUNT)) {
        return TCAM_ERROR;
    }

    send_data[6U] = send_data_idx6[image_mode_preset];
    if (cmd_write(tcam, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_mini212_set_image_mode_spatial_denoising_level(const tcam_t *tcam, tcam_mini212_image_mode_spatial_denoising_level_t image_mode_spatial_denoising_level)
{
    uint8_t send_data[7U] = {0x02U, 0x02U, 0x1CU, 0x00U, 0x00U, 0x00U};
    const uint8_t send_data_idx6[TCAM_MINI212_IMAGE_MODE_SPATIAL_DENOISING_LEVEL_COUNT] = {
        [TCAM_MINI212_IMAGE_MODE_SPATIAL_DENOISING_LEVEL_1] = 0x01U,
        [TCAM_MINI212_IMAGE_MODE_SPATIAL_DENOISING_LEVEL_2] = 0x02U,
        [TCAM_MINI212_IMAGE_MODE_SPATIAL_DENOISING_LEVEL_3] = 0x03U,
        [TCAM_MINI212_IMAGE_MODE_SPATIAL_DENOISING_LEVEL_4] = 0x04U,
    };

    if ((tcam_mini212_object_check(tcam) != TCAM_OK) ||
        ((uint32_t)image_mode_spatial_denoising_level >= (uint32_t)TCAM_MINI212_IMAGE_MODE_SPATIAL_DENOISING_LEVEL_COUNT)) {
        return TCAM_ERROR;
    }

    send_data[6U] = send_data_idx6[image_mode_spatial_denoising_level];
    if (cmd_write(tcam, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_mini212_set_image_mode_temporal_denoising_level(const tcam_t *tcam, tcam_mini212_image_mode_temporal_denoising_level_t image_mode_temporal_denoising_level)
{
    uint8_t send_data[7U] = {0x02U, 0x02U, 0x21U, 0x00U, 0x00U, 0x00U};
    const uint8_t send_data_idx6[TCAM_MINI212_IMAGE_MODE_TEMPORAL_DENOISING_LEVEL_COUNT] = {
        [TCAM_MINI212_IMAGE_MODE_TEMPORAL_DENOISING_LEVEL_1] = 0x01U,
        [TCAM_MINI212_IMAGE_MODE_TEMPORAL_DENOISING_LEVEL_2] = 0x02U,
        [TCAM_MINI212_IMAGE_MODE_TEMPORAL_DENOISING_LEVEL_3] = 0x03U,
        [TCAM_MINI212_IMAGE_MODE_TEMPORAL_DENOISING_LEVEL_4] = 0x04U,
    };

    if ((tcam_mini212_object_check(tcam) != TCAM_OK) ||
        ((uint32_t)image_mode_temporal_denoising_level >= (uint32_t)TCAM_MINI212_IMAGE_MODE_TEMPORAL_DENOISING_LEVEL_COUNT)) {
        return TCAM_ERROR;
    }

    send_data[6U] = send_data_idx6[image_mode_temporal_denoising_level];
    if (cmd_write(tcam, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_mini212_set_image_mode_detail_enhancement_level(const tcam_t *tcam, tcam_mini212_image_mode_detail_enhancement_level_t image_mode_detail_enhancement_level)
{
    uint8_t send_data[7U] = {0x02U, 0x02U, 0x1DU, 0x00U, 0x00U, 0x00U};
    const uint8_t send_data_idx6[TCAM_MINI212_IMAGE_MODE_DETAIL_ENHANCEMENT_LEVEL_COUNT] = {
        [TCAM_MINI212_IMAGE_MODE_DETAIL_ENHANCEMENT_LEVEL_1] = 0x01U,
        [TCAM_MINI212_IMAGE_MODE_DETAIL_ENHANCEMENT_LEVEL_2] = 0x02U,
        [TCAM_MINI212_IMAGE_MODE_DETAIL_ENHANCEMENT_LEVEL_3] = 0x03U,
        [TCAM_MINI212_IMAGE_MODE_DETAIL_ENHANCEMENT_LEVEL_4] = 0x04U,
    };

    if ((tcam_mini212_object_check(tcam) != TCAM_OK) ||
        ((uint32_t)image_mode_detail_enhancement_level >= (uint32_t)TCAM_MINI212_IMAGE_MODE_DETAIL_ENHANCEMENT_LEVEL_COUNT)) {
        return TCAM_ERROR;
    }

    send_data[6U] = send_data_idx6[image_mode_detail_enhancement_level];
    if (cmd_write(tcam, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_mini212_set_image_mode_brightness_level(const tcam_t *tcam, tcam_mini212_image_mode_brightness_level_t image_mode_brightness_level)
{
    uint8_t send_data[7U] = {0x02U, 0x02U, 0x1EU, 0x00U, 0x00U, 0x00U};
    const uint8_t send_data_idx6[TCAM_MINI212_IMAGE_MODE_BRIGHTNESS_LEVEL_COUNT] = {
        [TCAM_MINI212_IMAGE_MODE_BRIGHTNESS_LEVEL_1] = 0x01U,
        [TCAM_MINI212_IMAGE_MODE_BRIGHTNESS_LEVEL_2] = 0x02U,
        [TCAM_MINI212_IMAGE_MODE_BRIGHTNESS_LEVEL_3] = 0x03U,
        [TCAM_MINI212_IMAGE_MODE_BRIGHTNESS_LEVEL_4] = 0x04U,
        [TCAM_MINI212_IMAGE_MODE_BRIGHTNESS_LEVEL_5] = 0x05U,
    };

    if ((tcam_mini212_object_check(tcam) != TCAM_OK) ||
        ((uint32_t)image_mode_brightness_level >= (uint32_t)TCAM_MINI212_IMAGE_MODE_BRIGHTNESS_LEVEL_COUNT)) {
        return TCAM_ERROR;
    }

    send_data[6U] = send_data_idx6[image_mode_brightness_level];
    if (cmd_write(tcam, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_mini212_set_image_mode_contrast_level(const tcam_t *tcam, tcam_mini212_image_mode_contrast_level_t image_mode_contrast_level)
{
    uint8_t send_data[7U] = {0x02U, 0x02U, 0x1FU, 0x00U, 0x00U, 0x00U};
    const uint8_t send_data_idx6[TCAM_MINI212_IMAGE_MODE_CONTRAST_LEVEL_COUNT] = {
        [TCAM_MINI212_IMAGE_MODE_CONTRAST_LEVEL_1] = 0x01U,
        [TCAM_MINI212_IMAGE_MODE_CONTRAST_LEVEL_2] = 0x02U,
        [TCAM_MINI212_IMAGE_MODE_CONTRAST_LEVEL_3] = 0x03U,
        [TCAM_MINI212_IMAGE_MODE_CONTRAST_LEVEL_4] = 0x04U,
        [TCAM_MINI212_IMAGE_MODE_CONTRAST_LEVEL_5] = 0x05U,
    };

    if ((tcam_mini212_object_check(tcam) != TCAM_OK) ||
        ((uint32_t)image_mode_contrast_level >= (uint32_t)TCAM_MINI212_IMAGE_MODE_CONTRAST_LEVEL_COUNT)) {
        return TCAM_ERROR;
    }

    send_data[6U] = send_data_idx6[image_mode_contrast_level];
    if (cmd_write(tcam, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_mini212_set_image_mirror(const tcam_t *tcam, tcam_mini212_image_mirror_t image_mirror)
{
    uint8_t send_data[7U] = {0x02U, 0x00U, 0x05U, 0x00U, 0x00U, 0x00U};
    const uint8_t send_data_idx6[TCAM_MINI212_IMAGE_MIRROR_COUNT] = {
        [TCAM_MINI212_IMAGE_MIRROR_NONE] = 0x00U,
        [TCAM_MINI212_IMAGE_MIRROR_Y] = 0x01U,
        [TCAM_MINI212_IMAGE_MIRROR_X] = 0x02U,
        [TCAM_MINI212_IMAGE_MIRROR_XY] = 0x03U,
    };

    if ((tcam_mini212_object_check(tcam) != TCAM_OK) ||
        ((uint32_t)image_mirror >= (uint32_t)TCAM_MINI212_IMAGE_MIRROR_COUNT)) {
        return TCAM_ERROR;
    }

    send_data[6U] = send_data_idx6[image_mirror];
    if (cmd_write(tcam, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_mini212_set_image_zoom_center_x(const tcam_t *tcam, uint16_t image_zoom_center_x)
{
    uint8_t send_data[7U] = {0x02U, 0x00U, 0x07U, 0x00U, 0x00U};

    if ((tcam_mini212_object_check(tcam) != TCAM_OK) ||
        (image_zoom_center_x < TCAM_MINI212_IMAGE_ZOOM_CENTER_X_MIN) ||
        (image_zoom_center_x > TCAM_MINI212_IMAGE_ZOOM_CENTER_X_MAX)) {
        return TCAM_ERROR;
    }

    send_data[5U] = (uint8_t)((image_zoom_center_x >> 8) & 0xFFU);
    send_data[6U] = (uint8_t)(image_zoom_center_x & 0xFFU);
    if (cmd_write(tcam, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_mini212_set_image_zoom_center_y(const tcam_t *tcam, uint16_t image_zoom_center_y)
{
    uint8_t send_data[7U] = {0x02U, 0x00U, 0x08U, 0x00U, 0x00U};

    if ((tcam_mini212_object_check(tcam) != TCAM_OK) ||
        (image_zoom_center_y < TCAM_MINI212_IMAGE_ZOOM_CENTER_Y_MIN) ||
        (image_zoom_center_y > TCAM_MINI212_IMAGE_ZOOM_CENTER_Y_MAX)) {
        return TCAM_ERROR;
    }

    send_data[5U] = (uint8_t)((image_zoom_center_y >> 8) & 0xFFU);
    send_data[6U] = (uint8_t)(image_zoom_center_y & 0xFFU);
    if (cmd_write(tcam, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_mini212_set_image_zoom_ratio(const tcam_t *tcam, tcam_mini212_image_zoom_ratio_t image_zoom_ratio)
{
    uint8_t send_data[7U] = {0x02U, 0x00U, 0x06U, 0x00U, 0x00U, 0x00U};
    const uint8_t send_data_idx6[TCAM_MINI212_IMAGE_ZOOM_RATIO_COUNT] = {
        [TCAM_MINI212_IMAGE_ZOOM_RATIO_X8] = 0x01U,
        [TCAM_MINI212_IMAGE_ZOOM_RATIO_X16] = 0x02U,
        [TCAM_MINI212_IMAGE_ZOOM_RATIO_X24] = 0x03U,
        [TCAM_MINI212_IMAGE_ZOOM_RATIO_X32] = 0x04U,
        [TCAM_MINI212_IMAGE_ZOOM_RATIO_X40] = 0x05U,
        [TCAM_MINI212_IMAGE_ZOOM_RATIO_X48] = 0x06U,
        [TCAM_MINI212_IMAGE_ZOOM_RATIO_X56] = 0x07U,
        [TCAM_MINI212_IMAGE_ZOOM_RATIO_X64] = 0x08U,
    };

    if ((tcam_mini212_object_check(tcam) != TCAM_OK) ||
        ((uint32_t)image_zoom_ratio >= (uint32_t)TCAM_MINI212_IMAGE_ZOOM_RATIO_COUNT)) {
        return TCAM_ERROR;
    }

    send_data[6U] = send_data_idx6[image_zoom_ratio];
    if (cmd_write(tcam, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_mini212_set_frame_frequency(const tcam_t *tcam, tcam_mini212_frame_frequency_t frame_frequency)
{
    uint8_t send_data[7U] = {0x02U, 0x01U, 0x05U, 0x00U, 0x00U, 0x00U};
    const uint8_t send_data_idx6[TCAM_MINI212_STREAM_FRAME_FREQUENCY_COUNT] = {
        [TCAM_MINI212_STREAM_FRAME_FREQUENCY_50HZ] = 0x03U,
        [TCAM_MINI212_STREAM_FRAME_FREQUENCY_30HZ] = 0x00U,
        [TCAM_MINI212_STREAM_FRAME_FREQUENCY_25HZ] = 0x01U,
        [TCAM_MINI212_STREAM_FRAME_FREQUENCY_9HZ] = 0x02U,
    };

    if ((tcam_mini212_object_check(tcam) != TCAM_OK) ||
        ((uint32_t)frame_frequency >= (uint32_t)TCAM_MINI212_STREAM_FRAME_FREQUENCY_COUNT)) {
        return TCAM_ERROR;
    }

    send_data[6U] = send_data_idx6[frame_frequency];
    if (cmd_write(tcam, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_mini212_set_digital_type(const tcam_t *tcam, tcam_mini212_digital_type_t digital_type)
{
    uint8_t send_data[7U] = {0x02U, 0x01U, 0x02U, 0x00U, 0x00U, 0x00U};
    const uint8_t send_data_idx6[TCAM_MINI212_DIGITAL_TYPE_COUNT] = {
        [TCAM_MINI212_DIGITAL_TYPE_DISABLE] = 0x00U,
        [TCAM_MINI212_DIGITAL_TYPE_USB20] = 0x01U,
        [TCAM_MINI212_DIGITAL_TYPE_CMOS] = 0x02U,
        [TCAM_MINI212_DIGITAL_TYPE_BT656] = 0x04U,
        [TCAM_MINI212_DIGITAL_TYPE_BT1120] = 0x03U,
        [TCAM_MINI212_DIGITAL_TYPE_LVDS] = 0x07U,
        [TCAM_MINI212_DIGITAL_TYPE_USB20_UART] = 0x05U,
        [TCAM_MINI212_DIGITAL_TYPE_LCD] = 0x06U,
        [TCAM_MINI212_DIGITAL_TYPE_LCD_DVP] = 0x08U,
        [TCAM_MINI212_DIGITAL_TYPE_UVC_CDC] = 0x09U,
    };

    if ((tcam_mini212_object_check(tcam) != TCAM_OK) ||
        ((uint32_t)digital_type >= (uint32_t)TCAM_MINI212_DIGITAL_TYPE_COUNT)) {
        return TCAM_ERROR;
    }

    send_data[6U] = send_data_idx6[digital_type];
    if (cmd_write(tcam, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_mini212_set_cmos_interface(const tcam_t *tcam, tcam_mini212_cmos_interface_t cmos_interface)
{
    uint8_t send_data[7U] = {0x02U, 0x01U, 0x04U, 0x00U, 0x00U, 0x00U};
    const uint8_t send_data_idx6[TCAM_MINI212_CMOS_INTERFACE_COUNT] = {
        [TCAM_MINI212_CMOS_INTERFACE_CMOS16] = 0x00U,
        [TCAM_MINI212_CMOS_INTERFACE_CMOS8_MSB] = 0x01U,
        [TCAM_MINI212_CMOS_INTERFACE_CMOS8_LSB] = 0x02U,
    };

    if ((tcam_mini212_object_check(tcam) != TCAM_OK) ||
        ((uint32_t)cmos_interface >= (uint32_t)TCAM_MINI212_CMOS_INTERFACE_COUNT)) {
        return TCAM_ERROR;
    }

    send_data[6U] = send_data_idx6[cmos_interface];
    if (cmd_write(tcam, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_mini212_set_cmos_content(const tcam_t *tcam, tcam_mini212_cmos_content_t cmos_content)
{
    uint8_t send_data[7U] = {0x02U, 0x01U, 0x03U, 0x00U, 0x00U, 0x00U};
    const uint8_t send_data_idx6[TCAM_MINI212_CMOS_CONTENT_COUNT] = {
        [TCAM_MINI212_CMOS_CONTENT_YUV422] = 0x00U,
        [TCAM_MINI212_CMOS_CONTENT_YUV422_PARAM] = 0x01U,
        [TCAM_MINI212_CMOS_CONTENT_Y16] = 0x02U,
        [TCAM_MINI212_CMOS_CONTENT_Y16_PARAM] = 0x03U,
        [TCAM_MINI212_CMOS_CONTENT_Y16_YUV422] = 0x04U,
        [TCAM_MINI212_CMOS_CONTENT_Y16_PARAM_YUV422] = 0x05U,
        [TCAM_MINI212_CMOS_CONTENT_TMP] = 0x08U,
        [TCAM_MINI212_CMOS_CONTENT_TMP_PARAM] = 0x09U,
        [TCAM_MINI212_CMOS_CONTENT_TMP_YUV422] = 0x0AU,
        [TCAM_MINI212_CMOS_CONTENT_TMP_PARAM_YUV422] = 0x0BU,
    };

    if ((tcam_mini212_object_check(tcam) != TCAM_OK) ||
        ((uint32_t)cmos_content >= (uint32_t)TCAM_MINI212_CMOS_CONTENT_COUNT)) {
        return TCAM_ERROR;
    }

    send_data[6U] = send_data_idx6[cmos_content];
    if (cmd_write(tcam, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_mini212_set_external_sync_mode(const tcam_t *tcam, tcam_mini212_external_sync_mode_t external_sync_mode)
{
    uint8_t send_data[7U] = {0x02U, 0x01U, 0x01U, 0x00U, 0x00U, 0x00U};
    const uint8_t send_data_idx6[TCAM_MINI212_EXTERNAL_SYNC_MODE_COUNT] = {
        [TCAM_MINI212_EXTERNAL_SYNC_MODE_DISABLE] = 0x00U,
        [TCAM_MINI212_EXTERNAL_SYNC_MODE_SLAVE] = 0x01U,
        [TCAM_MINI212_EXTERNAL_SYNC_MODE_MASTER] = 0x02U,
    };

    if ((tcam_mini212_object_check(tcam) != TCAM_OK) ||
        ((uint32_t)external_sync_mode >= (uint32_t)TCAM_MINI212_EXTERNAL_SYNC_MODE_COUNT)) {
        return TCAM_ERROR;
    }

    send_data[6U] = send_data_idx6[external_sync_mode];
    if (cmd_write(tcam, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_mini212_do_save_settings(const tcam_t *tcam)
{
    const uint8_t send_data[7U] = {0x01U, 0x00U, 0x04U, 0x00U, 0x00U, 0x00U, 0x01U};

    if (tcam_mini212_object_check(tcam) != TCAM_OK) {
        return TCAM_ERROR;
    }

    if (cmd_write(tcam, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_mini212_do_restore_factory_settings(const tcam_t *tcam)
{
    const uint8_t send_data[7U] = {0x01U, 0x00U, 0x05U, 0x00U, 0x00U, 0x00U, 0x01U};

    if (tcam_mini212_object_check(tcam) != TCAM_OK) {
        return TCAM_ERROR;
    }

    if (cmd_write(tcam, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_mini212_set_shutter_state(const tcam_t *tcam, tcam_mini212_shutter_state_t shutter_state)
{
    uint8_t send_data[7U] = {0xA0U, 0x02U, 0x08U, 0x00U, 0x00U, 0x00U};
    const uint8_t send_data_idx6[TCAM_MINI212_SHUTTER_STATE_COUNT] = {
        [TCAM_MINI212_SHUTTER_STATE_CLOSE] = 0x00U,
        [TCAM_MINI212_SHUTTER_STATE_OPEN] = 0x01U,
    };

    if ((tcam_mini212_object_check(tcam) != TCAM_OK) ||
        ((uint32_t)shutter_state >= (uint32_t)TCAM_MINI212_SHUTTER_STATE_COUNT)) {
        return TCAM_ERROR;
    }

    send_data[6U] = send_data_idx6[shutter_state];
    if (cmd_write(tcam, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_mini212_set_auto_shutter_compensation_state(const tcam_t *tcam, tcam_mini212_auto_shutter_compensation_state_t auto_shutter_compensation_state)
{
    uint8_t send_data[7U] = {0x01U, 0x00U, 0x07U, 0x00U, 0x00U, 0x00U};
    const uint8_t send_data_idx6[TCAM_MINI212_AUTO_SHUTTER_COMPENSATION_STATE_COUNT] = {
        [TCAM_MINI212_AUTO_SHUTTER_COMPENSATION_STATE_ON] = 0x01U,
        [TCAM_MINI212_AUTO_SHUTTER_COMPENSATION_STATE_OFF] = 0x00U,
    };

    if ((tcam_mini212_object_check(tcam) != TCAM_OK) ||
        ((uint32_t)auto_shutter_compensation_state >= (uint32_t)TCAM_MINI212_AUTO_SHUTTER_COMPENSATION_STATE_COUNT)) {
        return TCAM_ERROR;
    }

    send_data[6U] = send_data_idx6[auto_shutter_compensation_state];
    if (cmd_write(tcam, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_mini212_set_auto_shutter_compensation_interval(const tcam_t *tcam, uint8_t auto_shutter_compensation_interval_min)
{
    uint8_t send_data[7U] = {0x01U, 0x00U, 0x01U, 0x00U, 0x00U, 0x00U};

    if ((tcam_mini212_object_check(tcam) != TCAM_OK) ||
        (auto_shutter_compensation_interval_min < TCAM_MINI212_AUTO_SHUTTER_COMPENSATION_INTERVAL_MIN_MIN) ||
        (auto_shutter_compensation_interval_min > TCAM_MINI212_AUTO_SHUTTER_COMPENSATION_INTERVAL_MIN_MAX)) {
        return TCAM_ERROR;
    }

    send_data[6U] = auto_shutter_compensation_interval_min;
    if (cmd_write(tcam, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_mini212_set_bad_point_x(const tcam_t *tcam, uint16_t bad_point_x)
{
    uint8_t send_data[7U] = {0x03U, 0x01U, 0x02U, 0x00U, 0x00U};

    if ((tcam_mini212_object_check(tcam) != TCAM_OK) ||
        (bad_point_x < TCAM_MINI212_BAD_POINT_X_MIN) ||
        (bad_point_x > TCAM_MINI212_BAD_POINT_X_MAX)) {
        return TCAM_ERROR;
    }

    send_data[5U] = (uint8_t)((bad_point_x >> 8) & 0xFFU);
    send_data[6U] = (uint8_t)(bad_point_x & 0xFFU);
    if (cmd_write(tcam, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_mini212_set_bad_point_y(const tcam_t *tcam, uint16_t bad_point_y)
{
    uint8_t send_data[7U] = {0x03U, 0x01U, 0x03U, 0x00U, 0x00U};

    if ((tcam_mini212_object_check(tcam) != TCAM_OK) ||
        (bad_point_y < TCAM_MINI212_BAD_POINT_Y_MIN) ||
        (bad_point_y > TCAM_MINI212_BAD_POINT_Y_MAX)) {
        return TCAM_ERROR;
    }

    send_data[5U] = (uint8_t)((bad_point_y >> 8) & 0xFFU);
    send_data[6U] = (uint8_t)(bad_point_y & 0xFFU);
    if (cmd_write(tcam, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_mini212_do_bad_point_add(const tcam_t *tcam)
{
    const uint8_t send_data[7U] = {0x03U, 0x01U, 0x04U, 0x00U, 0x00U, 0x00U, 0x01U};

    if (tcam_mini212_object_check(tcam) != TCAM_OK) {
        return TCAM_ERROR;
    }

    if (cmd_write(tcam, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_mini212_do_bad_point_save(const tcam_t *tcam)
{
    const uint8_t send_data[7U] = {0x03U, 0x01U, 0x05U, 0x00U, 0x00U, 0x00U, 0x01U};

    if (tcam_mini212_object_check(tcam) != TCAM_OK) {
        return TCAM_ERROR;
    }

    if (cmd_write(tcam, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_mini212_set_range(const tcam_t *tcam, tcam_mini212_range_t range)
{
    uint8_t send_data[7U] = {0x04U, 0x00U, 0x09U, 0x00U, 0x00U, 0x00U};
    const uint8_t send_data_idx6[TCAM_MINI212_RANGE_COUNT] = {
        [TCAM_MINI212_RANGE_N20_150] = 0x00U,
        [TCAM_MINI212_RANGE_0_550] = 0x01U,
    };

    if ((tcam_mini212_object_check(tcam) != TCAM_OK) ||
        ((uint32_t)range >= (uint32_t)TCAM_MINI212_RANGE_COUNT)) {
        return TCAM_ERROR;
    }

    send_data[6U] = send_data_idx6[range];
    if (cmd_write(tcam, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_mini212_set_distance(const tcam_t *tcam, uint16_t distance_dm)
{
    uint8_t send_data[7U] = {0x04U, 0x00U, 0x01U, 0x00U, 0x00U};

    if ((tcam_mini212_object_check(tcam) != TCAM_OK) ||
        (distance_dm < TCAM_MINI212_DISTANCE_DM_MIN) ||
        (distance_dm > TCAM_MINI212_DISTANCE_DM_MAX)) {
        return TCAM_ERROR;
    }

    send_data[5U] = (uint8_t)((distance_dm >> 8) & 0xFFU);
    send_data[6U] = (uint8_t)(distance_dm & 0xFFU);
    if (cmd_write(tcam, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_mini212_set_emissivity(const tcam_t *tcam, uint8_t emissivity_percent)
{
    uint8_t send_data[7U] = {0x04U, 0x00U, 0x02U, 0x00U, 0x00U, 0x00U};

    if ((tcam_mini212_object_check(tcam) != TCAM_OK) ||
        (emissivity_percent < TCAM_MINI212_EMISSIVITY_PERCENT_MIN) ||
        (emissivity_percent > TCAM_MINI212_EMISSIVITY_PERCENT_MAX)) {
        return TCAM_ERROR;
    }

    send_data[6U] = emissivity_percent;
    if (cmd_write(tcam, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_mini212_set_humidity(const tcam_t *tcam, uint8_t humidity_percent)
{
    uint8_t send_data[7U] = {0x04U, 0x00U, 0x08U, 0x00U, 0x00U, 0x00U};

    if ((tcam_mini212_object_check(tcam) != TCAM_OK) ||
        (humidity_percent < TCAM_MINI212_HUMIDITY_PERCENT_MIN) ||
        (humidity_percent > TCAM_MINI212_HUMIDITY_PERCENT_MAX)) {
        return TCAM_ERROR;
    }

    send_data[6U] = humidity_percent;
    if (cmd_write(tcam, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_mini212_set_reflected_temp(const tcam_t *tcam, int16_t reflected_temp_mult10)
{
    uint8_t send_data[7U] = {0x04U, 0x00U, 0x07U, 0x00U, 0x00U};

    if ((tcam_mini212_object_check(tcam) != TCAM_OK) ||
        (reflected_temp_mult10 < TCAM_MINI212_REFLECTED_TEMP_MULT10_MIN) ||
        (reflected_temp_mult10 > TCAM_MINI212_REFLECTED_TEMP_MULT10_MAX)) {
        return TCAM_ERROR;
    }

    send_data[5U] = (uint8_t)(((uint16_t)reflected_temp_mult10 >> 8) & 0xFFU);
    send_data[6U] = (uint8_t)((uint16_t)reflected_temp_mult10 & 0xFFU);
    if (cmd_write(tcam, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_mini212_set_atmospheric_temp(const tcam_t *tcam, int16_t atmospheric_temp_mult100)
{
    uint8_t send_data[7U] = {0x04U, 0x00U, 0x18U, 0x00U, 0x00U};

    if ((tcam_mini212_object_check(tcam) != TCAM_OK) ||
        (atmospheric_temp_mult100 < TCAM_MINI212_ATMOSPHERIC_TEMP_MULT100_MIN) ||
        (atmospheric_temp_mult100 > TCAM_MINI212_ATMOSPHERIC_TEMP_MULT100_MAX)) {
        return TCAM_ERROR;
    }

    send_data[5U] = (uint8_t)(((uint16_t)atmospheric_temp_mult100 >> 8) & 0xFFU);
    send_data[6U] = (uint8_t)((uint16_t)atmospheric_temp_mult100 & 0xFFU);
    if (cmd_write(tcam, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_mini212_set_single_point_calibration_temp(const tcam_t *tcam, int16_t single_point_calibration_temp_mult10)
{
    uint8_t send_data[7U] = {0x04U, 0x01U, 0x08U, 0x00U, 0x00U};

    if ((tcam_mini212_object_check(tcam) != TCAM_OK) ||
        (single_point_calibration_temp_mult10 < TCAM_MINI212_SINGLE_POINT_CALIBRATION_TEMP_MULT10_MIN) ||
        (single_point_calibration_temp_mult10 > TCAM_MINI212_SINGLE_POINT_CALIBRATION_TEMP_MULT10_MAX)) {
        return TCAM_ERROR;
    }

    send_data[5U] = (uint8_t)(((uint16_t)single_point_calibration_temp_mult10 >> 8) & 0xFFU);
    send_data[6U] = (uint8_t)((uint16_t)single_point_calibration_temp_mult10 & 0xFFU);
    if (cmd_write(tcam, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_mini212_do_single_point_calibration_acquisition(const tcam_t *tcam)
{
    const uint8_t send_data[7U] = {0x04U, 0x01U, 0x04U, 0x00U, 0x00U, 0x00U, 0x01U};

    if (tcam_mini212_object_check(tcam) != TCAM_OK) {
        return TCAM_ERROR;
    }

    if (cmd_write(tcam, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_mini212_do_single_point_calibration(const tcam_t *tcam)
{
    const uint8_t send_data[7U] = {0x04U, 0x01U, 0x05U, 0x00U, 0x00U, 0x00U, 0x01U};

    if (tcam_mini212_object_check(tcam) != TCAM_OK) {
        return TCAM_ERROR;
    }

    if (cmd_write(tcam, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_mini212_set_two_point_calibration_low_temp(const tcam_t *tcam, int16_t two_point_calibration_low_temp_mult10)
{
    uint8_t send_data[7U] = {0x04U, 0x01U, 0x06U, 0x00U, 0x00U};

    if ((tcam_mini212_object_check(tcam) != TCAM_OK) ||
        (two_point_calibration_low_temp_mult10 < TCAM_MINI212_TWO_POINT_CALIBRATION_LOW_TEMP_MULT10_MIN) ||
        (two_point_calibration_low_temp_mult10 > TCAM_MINI212_TWO_POINT_CALIBRATION_LOW_TEMP_MULT10_MAX)) {
        return TCAM_ERROR;
    }

    send_data[5U] = (uint8_t)(((uint16_t)two_point_calibration_low_temp_mult10 >> 8) & 0xFFU);
    send_data[6U] = (uint8_t)((uint16_t)two_point_calibration_low_temp_mult10 & 0xFFU);
    if (cmd_write(tcam, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_mini212_do_two_point_calibration_low_acquisition(const tcam_t *tcam)
{
    const uint8_t send_data[7U] = {0x04U, 0x01U, 0x01U, 0x00U, 0x00U, 0x00U, 0x01U};

    if (tcam_mini212_object_check(tcam) != TCAM_OK) {
        return TCAM_ERROR;
    }

    if (cmd_write(tcam, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_mini212_set_two_point_calibration_high_temp(const tcam_t *tcam, int16_t two_point_calibration_high_temp_mult10)
{
    uint8_t send_data[7U] = {0x04U, 0x01U, 0x07U, 0x00U, 0x00U};

    if ((tcam_mini212_object_check(tcam) != TCAM_OK) ||
        (two_point_calibration_high_temp_mult10 < TCAM_MINI212_TWO_POINT_CALIBRATION_HIGH_TEMP_MULT10_MIN) ||
        (two_point_calibration_high_temp_mult10 > TCAM_MINI212_TWO_POINT_CALIBRATION_HIGH_TEMP_MULT10_MAX)) {
        return TCAM_ERROR;
    }

    send_data[5U] = (uint8_t)(((uint16_t)two_point_calibration_high_temp_mult10 >> 8) & 0xFFU);
    send_data[6U] = (uint8_t)((uint16_t)two_point_calibration_high_temp_mult10 & 0xFFU);
    if (cmd_write(tcam, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_mini212_do_two_point_calibration_high_acquisition(const tcam_t *tcam)
{
    const uint8_t send_data[7U] = {0x04U, 0x01U, 0x02U, 0x00U, 0x00U, 0x00U, 0x01U};

    if (tcam_mini212_object_check(tcam) != TCAM_OK) {
        return TCAM_ERROR;
    }

    if (cmd_write(tcam, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_mini212_do_two_point_calibration(const tcam_t *tcam)
{
    const uint8_t send_data[7U] = {0x04U, 0x01U, 0x03U, 0x00U, 0x00U, 0x00U, 0x01U};

    if (tcam_mini212_object_check(tcam) != TCAM_OK) {
        return TCAM_ERROR;
    }

    if (cmd_write(tcam, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

tcam_status_t tcam_mini212_set_custom_data(const tcam_t *tcam, const tcam_mini212_custom_data_t *custom_data)
{
    uint8_t send_data[18U] = {0xB0U, 0x01U, 0x01U};

    if ((tcam_mini212_object_check(tcam) != TCAM_OK) ||
        (custom_data == NULL)) {
        return TCAM_ERROR;
    }

    (void)memcpy(&send_data[3U], custom_data->data, sizeof(send_data) - 3U);
    if (cmd_write(tcam, send_data, sizeof(send_data)) != TCAM_OK) {
        return TCAM_ERROR;
    }

    return TCAM_OK;
}

uint32_t tcam_mini212_stream_frame_get_size(void)
{
    return STREAM_FRAME_SIZE;
}

tcam_status_t tcam_mini212_stream_frame_decode(const tcam_t *tcam, const void *frame, tcam_stream_frame_info_t *frame_info)
{
    stream_frame_param_t param;

    if ((tcam_mini212_object_check(tcam) != TCAM_OK) ||
        (frame == NULL) ||
        (frame_info == NULL)) {
        return TCAM_ERROR;
    }

    (void)memcpy(&param, (const uint8_t *)frame + STREAM_FRAME_PARAM_OFFSET, sizeof(param));

    if ((param.head1 != STREAM_FRAME_PARAM_HEAD1) ||
        (param.head2 != STREAM_FRAME_PARAM_HEAD2) ||
        (param.tail != STREAM_FRAME_PARAM_TAIL)) {
        return TCAM_ERROR;
    }

    frame_info->freezed = (param.shutter_off == 0U) ? false : true;
    frame_info->min_point.x = param.min_x;
    frame_info->min_point.y = param.min_y;
    frame_info->min_point.temp_mult10 = (int16_t)param.min_temp_mult10;
    frame_info->max_point.x = param.max_x;
    frame_info->max_point.y = param.max_y;
    frame_info->max_point.temp_mult10 = (int16_t)param.max_temp_mult10;
    frame_info->avg_temp_mult10 = (int16_t)param.avg_temp_mult10;
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
