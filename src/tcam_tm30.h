#ifndef TCAM_TM30_H
#define TCAM_TM30_H

#include "tcam.h"

typedef struct {
    uint8_t data[5U];
} tcam_tm30_cid_t;

typedef struct {
    char data[9U];
} tcam_tm30_sn_t;

typedef struct {
    char data[48U];
} tcam_tm30_model_number_t;

typedef struct {
    uint8_t major;
    uint8_t minor;
    uint8_t patch;
} tcam_tm30_fpga_firmware_version_t;

typedef struct {
    uint16_t year;
    uint8_t month;
    uint8_t day;
} tcam_tm30_fpga_firmware_build_time_t;

typedef struct {
    uint8_t major;
    uint8_t minor;
    uint8_t patch;
} tcam_tm30_firmware_version_t;

typedef struct {
    uint16_t year;
    uint8_t month;
    uint8_t day;
} tcam_tm30_firmware_build_time_t;

typedef struct {
    uint16_t year;
    uint8_t month;
    uint8_t day;
} tcam_tm30_camera_calibration_time_t;

#define TCAM_TM30_TIME_YEAR_MIN        (2004U)
#define TCAM_TM30_TIME_YEAR_MAX        (2037U)
#define TCAM_TM30_TIME_MONTH_MIN       (1U)
#define TCAM_TM30_TIME_MONTH_MAX       (12U)
#define TCAM_TM30_TIME_WEEK_MIN        (0U)
#define TCAM_TM30_TIME_WEEK_MAX        (6U)
#define TCAM_TM30_TIME_DAY_MIN         (1U)
#define TCAM_TM30_TIME_DAY_MAX         (31U)
#define TCAM_TM30_TIME_HOUR_MIN        (0U)
#define TCAM_TM30_TIME_HOUR_MAX        (23U)
#define TCAM_TM30_TIME_MINUTE_MIN      (0U)
#define TCAM_TM30_TIME_MINUTE_MAX      (59U)
#define TCAM_TM30_TIME_SECOND_MIN      (0U)
#define TCAM_TM30_TIME_SECOND_MAX      (59U)
#define TCAM_TM30_TIME_MILLISECOND_MIN (0U)
#define TCAM_TM30_TIME_MILLISECOND_MAX (999U)

typedef struct {
    uint16_t year;
    uint8_t month;
    uint8_t week;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint16_t millisecond;
} tcam_tm30_time_t;

typedef enum {
    TCAM_TM30_AUTO_SHUTTER_COMPENSATION_STATE_OFF = 0U,
    TCAM_TM30_AUTO_SHUTTER_COMPENSATION_STATE_TIMED,
    TCAM_TM30_AUTO_SHUTTER_COMPENSATION_STATE_TEMP_DELTA,
    TCAM_TM30_AUTO_SHUTTER_COMPENSATION_STATE_AUTO,
    TCAM_TM30_AUTO_SHUTTER_COMPENSATION_STATE_COUNT,
} tcam_tm30_auto_shutter_compensation_state_t;

#define TCAM_TM30_AUTO_SHUTTER_COMPENSATION_TIMED_INTERVAL_MIN_MIN (0U)
#define TCAM_TM30_AUTO_SHUTTER_COMPENSATION_TIMED_INTERVAL_MIN_MAX (65535U)

typedef enum {
    TCAM_TM30_CURSOR_DISPLAY_STATE_OFF = 0U,
    TCAM_TM30_CURSOR_DISPLAY_STATE_ON,
    TCAM_TM30_CURSOR_DISPLAY_STATE_COUNT,
} tcam_tm30_cursor_display_state_t;

typedef enum {
    TCAM_TM30_CURSOR_POSITION_CTRL_UP = 0U,
    TCAM_TM30_CURSOR_POSITION_CTRL_DOWN,
    TCAM_TM30_CURSOR_POSITION_CTRL_LEFT,
    TCAM_TM30_CURSOR_POSITION_CTRL_RIGHT,
    TCAM_TM30_CURSOR_POSITION_CTRL_CENTER,
    TCAM_TM30_CURSOR_POSITION_CTRL_COUNT,
} tcam_tm30_cursor_position_ctrl_t;

#define TCAM_TM30_CURSOR_POSITION_CTRL_STEP_MIN (1U)
#define TCAM_TM30_CURSOR_POSITION_CTRL_STEP_MAX (15U)

typedef enum {
    TCAM_TM30_CURSOR_BAD_POINT_CTRL_ADD = 0U,
    TCAM_TM30_CURSOR_BAD_POINT_CTRL_DELETE,
    TCAM_TM30_CURSOR_BAD_POINT_CTRL_COUNT,
} tcam_tm30_cursor_bad_point_ctrl_t;

#define TCAM_TM30_IMAGE_BRIGHTNESS_PERCENT_MIN (0U)
#define TCAM_TM30_IMAGE_BRIGHTNESS_PERCENT_MAX (100U)

#define TCAM_TM30_IMAGE_CONTRAST_PERCENT_MIN (0U)
#define TCAM_TM30_IMAGE_CONTRAST_PERCENT_MAX (100U)

#define TCAM_TM30_IMAGE_DETAIL_ENHANCEMENT_PERCENT_MIN (0U)
#define TCAM_TM30_IMAGE_DETAIL_ENHANCEMENT_PERCENT_MAX (100U)

#define TCAM_TM30_IMAGE_SPATIAL_DENOISING_PERCENT_MIN (0U)
#define TCAM_TM30_IMAGE_SPATIAL_DENOISING_PERCENT_MAX (100U)

#define TCAM_TM30_IMAGE_TEMPORAL_DENOISING_PERCENT_MIN (0U)
#define TCAM_TM30_IMAGE_TEMPORAL_DENOISING_PERCENT_MAX (100U)

typedef enum {
    TCAM_TM30_IMAGE_COLOR_WHITE_HOT = 0U,
    TCAM_TM30_IMAGE_COLOR_BLACK_HOT,
    TCAM_TM30_IMAGE_COLOR_FUSION_1,
    TCAM_TM30_IMAGE_COLOR_RAINBOW,
    TCAM_TM30_IMAGE_COLOR_FUSION_2,
    TCAM_TM30_IMAGE_COLOR_IRON_RED_1,
    TCAM_TM30_IMAGE_COLOR_IRON_RED_2,
    TCAM_TM30_IMAGE_COLOR_DARK_BROWN,
    TCAM_TM30_IMAGE_COLOR_HUE_1,
    TCAM_TM30_IMAGE_COLOR_HUE_2,
    TCAM_TM30_IMAGE_COLOR_ICE_FIRE,
    TCAM_TM30_IMAGE_COLOR_RAIN,
    TCAM_TM30_IMAGE_COLOR_GREEN_HOT,
    TCAM_TM30_IMAGE_COLOR_RED_HOT,
    TCAM_TM30_IMAGE_COLOR_DARK_BLUE,
    TCAM_TM30_IMAGE_COLOR_COUNT,
} tcam_tm30_image_color_t;

typedef enum {
    TCAM_TM30_IMAGE_MIRROR_NONE = 0U,
    TCAM_TM30_IMAGE_MIRROR_CENTER,
    TCAM_TM30_IMAGE_MIRROR_LEFT_RIGHT,
    TCAM_TM30_IMAGE_MIRROR_UP_DOWN,
    TCAM_TM30_IMAGE_MIRROR_COUNT,
} tcam_tm30_image_mirror_t;

typedef enum {
    TCAM_TM30_IMAGE_ROTATE_DISABLE = 0U,
    TCAM_TM30_IMAGE_ROTATE_ENABLE,
    TCAM_TM30_IMAGE_ROTATE_COUNT,
} tcam_tm30_image_rotate_t;

typedef enum {
    TCAM_TM30_STREAM_FORMAT_MT = 0U,
    TCAM_TM30_STREAM_FORMAT_RAW,
    TCAM_TM30_STREAM_FORMAT_YUV,
    TCAM_TM30_STREAM_FORMAT_MT_YUV,
    TCAM_TM30_STREAM_FORMAT_RAW_YUV,
    TCAM_TM30_STREAM_FORMAT_YUV_IMAGE,
    TCAM_TM30_STREAM_FORMAT_COUNT,
} tcam_tm30_stream_format_t;

typedef enum {
    TCAM_TM30_TEMP_MEASUREMENT_STATE_DISABLE = 0U,
    TCAM_TM30_TEMP_MEASUREMENT_STATE_ENABLE,
    TCAM_TM30_TEMP_MEASUREMENT_STATE_COUNT,
} tcam_tm30_temp_measurement_state_t;

typedef enum {
    TCAM_TM30_TEMP_MEASUREMENT_UNIT_CELSIUS = 0U,
    TCAM_TM30_TEMP_MEASUREMENT_UNIT_FAHRENHEIT,
    TCAM_TM30_TEMP_MEASUREMENT_UNIT_KELVIN,
    TCAM_TM30_TEMP_MEASUREMENT_UNIT_COUNT,
} tcam_tm30_temp_measurement_unit_t;

typedef enum {
    TCAM_TM30_TEMP_MEASUREMENT_RANGE_N20_150 = 0U,
    TCAM_TM30_TEMP_MEASUREMENT_RANGE_100_550,
    TCAM_TM30_TEMP_MEASUREMENT_RANGE_AUTO,
    TCAM_TM30_TEMP_MEASUREMENT_RANGE_COUNT,
} tcam_tm30_temp_measurement_range_t;

#define TCAM_TM30_EMISSIVITY_PERCENT_MIN (1U)
#define TCAM_TM30_EMISSIVITY_PERCENT_MAX (100U)

#define TCAM_TM30_DISTANCE_DM_MIN (3U)
#define TCAM_TM30_DISTANCE_DM_MAX (30U)

typedef enum {
    TCAM_TM30_REFLECTED_TEMP_STATE_DISABLE = 0U,
    TCAM_TM30_REFLECTED_TEMP_STATE_ENABLE,
    TCAM_TM30_REFLECTED_TEMP_STATE_COUNT,
} tcam_tm30_reflected_temp_state_t;

#define TCAM_TM30_REFLECTED_TEMP_ADD100_MULT10_MIN (0U)
#define TCAM_TM30_REFLECTED_TEMP_ADD100_MULT10_MAX (11000U)

#define TCAM_TM30_CALIBRATION_PARAM_DISTANCE_DM_MIN                    (3U)
#define TCAM_TM30_CALIBRATION_PARAM_DISTANCE_DM_MAX                    (1000U)
#define TCAM_TM30_CALIBRATION_PARAM_ATMOSPHERIC_TEMP_ADD300_MULT10_MIN (270U)
#define TCAM_TM30_CALIBRATION_PARAM_ATMOSPHERIC_TEMP_ADD300_MULT10_MAX (13000U)
#define TCAM_TM30_CALIBRATION_PARAM_EMISSIVITY_PERCENT_MIN             (1U)
#define TCAM_TM30_CALIBRATION_PARAM_EMISSIVITY_PERCENT_MAX             (100U)
#define TCAM_TM30_CALIBRATION_PARAM_CALIBRATION_TEMP_ADD100_MULT10_MIN (600U)
#define TCAM_TM30_CALIBRATION_PARAM_CALIBRATION_TEMP_ADD100_MULT10_MAX (7500U)
#define TCAM_TM30_CALIBRATION_PARAM_XPOS_NOR_MIN                       (13U)
#define TCAM_TM30_CALIBRATION_PARAM_XPOS_NOR_MAX                       (993U)
#define TCAM_TM30_CALIBRATION_PARAM_YPOS_NOR_MIN                       (13U)
#define TCAM_TM30_CALIBRATION_PARAM_YPOS_NOR_MAX                       (993U)

typedef struct {
    uint16_t distance_dm;
    uint16_t atmospheric_temp_add300_mult10;
    uint16_t emissivity_percent;
    uint16_t calibration_temp_add100_mult10;
    uint16_t xpos_nor;
    uint16_t ypos_nor;
} tcam_tm30_calibration_param_t;

typedef enum {
    TCAM_TM30_CALIBRATION_STATE_DISABLE = 0U,
    TCAM_TM30_CALIBRATION_STATE_ENABLE,
    TCAM_TM30_CALIBRATION_STATE_COUNT,
} tcam_tm30_calibration_state_t;

typedef enum {
    TCAM_TM30_YUV_RESOLUTION_96X96 = 0U,
    TCAM_TM30_YUV_RESOLUTION_240X240,
    TCAM_TM30_YUV_RESOLUTION_COUNT,
} tcam_tm30_yuv_resolution_t;

#define TCAM_TM30_HUMIDITY_PERMILLE_MIN (0U)
#define TCAM_TM30_HUMIDITY_PERMILLE_MAX (1000U)

typedef enum {
    TCAM_TM30_ATMOSPHERIC_TEMP_STATE_DISABLE = 0U,
    TCAM_TM30_ATMOSPHERIC_TEMP_STATE_ENABLE,
    TCAM_TM30_ATMOSPHERIC_TEMP_STATE_COUNT,
} tcam_tm30_atmospheric_temp_state_t;

#define TCAM_TM30_ATMOSPHERIC_TEMP_ADD100_MULT10_MIN (0U)
#define TCAM_TM30_ATMOSPHERIC_TEMP_ADD100_MULT10_MAX (11000U)

typedef struct {
    uint16_t x;
    uint16_t y;
    int16_t temp_mult10;
} tcam_tm30_stream_frame_point_t;

typedef struct {
    const uint8_t *data;
    uint16_t width;
    uint16_t height;
    uint8_t bytes_per_item;
    uint32_t stride_item;
} tcam_tm30_stream_frame_matrix_t;

typedef struct {
    bool freezed;
    tcam_tm30_stream_frame_point_t max_point;
    tcam_tm30_stream_frame_point_t min_point;
    int16_t avg_temp_mult10;
    tcam_tm30_stream_frame_matrix_t mt_matrix;
    tcam_tm30_stream_frame_matrix_t yuv_matrix;
} tcam_tm30_stream_frame_info_t;

tcam_status_t tcam_tm30_probe(const tcam_t *tcam);

tcam_status_t tcam_tm30_get_cid(const tcam_t *tcam, tcam_tm30_cid_t *cid);

tcam_status_t tcam_tm30_get_sn(const tcam_t *tcam, tcam_tm30_sn_t *sn);

tcam_status_t tcam_tm30_get_model_number(const tcam_t *tcam, tcam_tm30_model_number_t *model_number);

tcam_status_t tcam_tm30_get_fpga_firmware_version(const tcam_t *tcam, tcam_tm30_fpga_firmware_version_t *fpga_firmware_version);

tcam_status_t tcam_tm30_get_fpga_firmware_build_time(const tcam_t *tcam, tcam_tm30_fpga_firmware_build_time_t *fpga_firmware_build_time);

tcam_status_t tcam_tm30_get_firmware_version(const tcam_t *tcam, tcam_tm30_firmware_version_t *firmware_version);

tcam_status_t tcam_tm30_get_firmware_build_time(const tcam_t *tcam, tcam_tm30_firmware_build_time_t *firmware_build_time);

tcam_status_t tcam_tm30_get_camera_calibration_time(const tcam_t *tcam, tcam_tm30_camera_calibration_time_t *camera_calibration_time);

tcam_status_t tcam_tm30_do_reset(const tcam_t *tcam);

tcam_status_t tcam_tm30_get_time(const tcam_t *tcam, tcam_tm30_time_t *time);

tcam_status_t tcam_tm30_do_save_settings(const tcam_t *tcam);

tcam_status_t tcam_tm30_do_restore_factory_settings(const tcam_t *tcam);

tcam_status_t tcam_tm30_do_shutter_correction(const tcam_t *tcam);

tcam_status_t tcam_tm30_do_background_correction(const tcam_t *tcam);

tcam_status_t tcam_tm30_do_vignetting_correction(const tcam_t *tcam);

tcam_status_t tcam_tm30_set_auto_shutter_compensation(const tcam_t *tcam, tcam_tm30_auto_shutter_compensation_state_t auto_shutter_compensation_state);

tcam_status_t tcam_tm30_get_auto_shutter_compensation(const tcam_t *tcam, tcam_tm30_auto_shutter_compensation_state_t *auto_shutter_compensation_state);

tcam_status_t tcam_tm30_set_auto_shutter_compensation_timed_interval(const tcam_t *tcam, uint16_t auto_shutter_compensation_timed_interval_min);

tcam_status_t tcam_tm30_get_auto_shutter_compensation_timed_interval(const tcam_t *tcam, uint16_t *auto_shutter_compensation_timed_interval_min);

tcam_status_t tcam_tm30_set_cursor_display(const tcam_t *tcam, tcam_tm30_cursor_display_state_t cursor_display_state);

tcam_status_t tcam_tm30_set_cursor_position(const tcam_t *tcam, tcam_tm30_cursor_position_ctrl_t cursor_position_ctrl, uint8_t cursor_position_ctrl_step);

tcam_status_t tcam_tm30_set_cursor_bad_point(const tcam_t *tcam, tcam_tm30_cursor_bad_point_ctrl_t cursor_bad_point_ctrl);

tcam_status_t tcam_tm30_set_image_brightness(const tcam_t *tcam, uint8_t image_brightness_percent);

tcam_status_t tcam_tm30_get_image_brightness(const tcam_t *tcam, uint8_t *image_brightness_percent);

tcam_status_t tcam_tm30_set_image_contrast(const tcam_t *tcam, uint8_t image_contrast_percent);

tcam_status_t tcam_tm30_get_image_contrast(const tcam_t *tcam, uint8_t *image_contrast_percent);

tcam_status_t tcam_tm30_set_image_detail_enhancement(const tcam_t *tcam, uint8_t image_detail_enhancement_percent);

tcam_status_t tcam_tm30_get_image_detail_enhancement(const tcam_t *tcam, uint8_t *image_detail_enhancement_percent);

tcam_status_t tcam_tm30_set_image_spatial_denoising(const tcam_t *tcam, uint8_t image_spatial_denoising_percent);

tcam_status_t tcam_tm30_get_image_spatial_denoising(const tcam_t *tcam, uint8_t *image_spatial_denoising_percent);

tcam_status_t tcam_tm30_set_image_temporal_denoising(const tcam_t *tcam, uint8_t image_temporal_denoising_percent);

tcam_status_t tcam_tm30_get_image_temporal_denoising(const tcam_t *tcam, uint8_t *image_temporal_denoising_percent);

tcam_status_t tcam_tm30_set_image_color(const tcam_t *tcam, tcam_tm30_image_color_t image_color);

tcam_status_t tcam_tm30_get_image_color(const tcam_t *tcam, tcam_tm30_image_color_t *image_color);

tcam_status_t tcam_tm30_set_image_mirror(const tcam_t *tcam, tcam_tm30_image_mirror_t image_mirror);

tcam_status_t tcam_tm30_get_image_mirror(const tcam_t *tcam, tcam_tm30_image_mirror_t *image_mirror);

tcam_status_t tcam_tm30_set_image_rotate(const tcam_t *tcam, tcam_tm30_image_rotate_t image_rotate);

tcam_status_t tcam_tm30_get_image_rotate(const tcam_t *tcam, tcam_tm30_image_rotate_t *image_rotate);

tcam_status_t tcam_tm30_set_stream_format(const tcam_t *tcam, tcam_tm30_stream_format_t stream_format);

tcam_status_t tcam_tm30_get_stream_format(const tcam_t *tcam, tcam_tm30_stream_format_t *stream_format);

tcam_status_t tcam_tm30_set_temp_measurement(const tcam_t *tcam, tcam_tm30_temp_measurement_state_t temp_measurement_state);

tcam_status_t tcam_tm30_get_temp_measurement(const tcam_t *tcam, tcam_tm30_temp_measurement_state_t *temp_measurement_state);

tcam_status_t tcam_tm30_set_temp_unit(const tcam_t *tcam, tcam_tm30_temp_measurement_unit_t temp_measurement_unit);

tcam_status_t tcam_tm30_get_temp_unit(const tcam_t *tcam, tcam_tm30_temp_measurement_unit_t *temp_measurement_unit);

tcam_status_t tcam_tm30_set_temp_measurement_range(const tcam_t *tcam, tcam_tm30_temp_measurement_range_t temp_measurement_range);

tcam_status_t tcam_tm30_get_temp_measurement_range(const tcam_t *tcam, tcam_tm30_temp_measurement_range_t *temp_measurement_range);

tcam_status_t tcam_tm30_set_emissivity(const tcam_t *tcam, uint8_t emissivity_percent);

tcam_status_t tcam_tm30_get_emissivity(const tcam_t *tcam, uint8_t *emissivity_percent);

tcam_status_t tcam_tm30_set_distance(const tcam_t *tcam, uint16_t distance_dm);

tcam_status_t tcam_tm30_get_distance(const tcam_t *tcam, uint16_t *distance_dm);

tcam_status_t tcam_tm30_set_reflected_temp(const tcam_t *tcam, tcam_tm30_reflected_temp_state_t reflected_temp_state, uint16_t reflected_temp_add100_mult10);

tcam_status_t tcam_tm30_get_reflected_temp(const tcam_t *tcam, tcam_tm30_reflected_temp_state_t *reflected_temp_state, uint16_t *reflected_temp_add100_mult10);

tcam_status_t tcam_tm30_set_calibration_param(const tcam_t *tcam, tcam_tm30_calibration_param_t *calibration_param);

tcam_status_t tcam_tm30_do_calibration(const tcam_t *tcam, tcam_tm30_calibration_state_t calibration_state);

tcam_status_t tcam_tm30_set_yuv_resolution(const tcam_t *tcam, tcam_tm30_yuv_resolution_t yuv_resolution);

tcam_status_t tcam_tm30_set_humidity(const tcam_t *tcam, uint16_t humidity_permille);

tcam_status_t tcam_tm30_get_humidity(const tcam_t *tcam, uint16_t *humidity_permille);

tcam_status_t tcam_tm30_set_atmospheric_temp(const tcam_t *tcam, tcam_tm30_atmospheric_temp_state_t atmospheric_temp_state, uint16_t atmospheric_temp_add100_mult10);

tcam_status_t tcam_tm30_get_atmospheric_temp(const tcam_t *tcam, tcam_tm30_atmospheric_temp_state_t *atmospheric_temp_state, uint16_t *atmospheric_temp_add100_mult10);

tcam_status_t tcam_tm30_stream_start(const tcam_t *tcam);

tcam_status_t tcam_tm30_stream_frame_acquire(const tcam_t *tcam, void **frame, uint32_t timeout_ms);

tcam_status_t tcam_tm30_stream_frame_release(const tcam_t *tcam);

tcam_status_t tcam_tm30_stream_frame_decode(const tcam_t *tcam, const void *frame, tcam_tm30_stream_frame_info_t *frame_info);

#endif
