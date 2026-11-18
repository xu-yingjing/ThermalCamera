#ifndef TCAM_MINI212_H
#define TCAM_MINI212_H

#include "tcam.h"

typedef struct {
    uint8_t pid;
    struct {
        uint8_t year;
        uint8_t month;
        uint8_t day;
    } firmware_version;
    uint16_t focal_temp_mult100;
    uint32_t machine_id;
} tcam_mini212_status_page_t;

typedef enum {
    TCAM_MINI212_DIGITAL_VIDEO_EXTERNAL_SYNC_MODE_DISABLE = 0x00U,
    TCAM_MINI212_DIGITAL_VIDEO_EXTERNAL_SYNC_MODE_SLAVE = 0x01U,
    TCAM_MINI212_DIGITAL_VIDEO_EXTERNAL_SYNC_MODE_MASTER = 0x02U,
} tcam_mini212_digital_video_external_sync_mode_t;

typedef enum {
    TCAM_MINI212_DIGITAL_VIDEO_DIGITAL_TYPE_DISABLE = 0x00U,
    TCAM_MINI212_DIGITAL_VIDEO_DIGITAL_TYPE_USB20 = 0x01U,
    TCAM_MINI212_DIGITAL_VIDEO_DIGITAL_TYPE_CMOS = 0x02U,
    TCAM_MINI212_DIGITAL_VIDEO_DIGITAL_TYPE_BT1120 = 0x03U,
    TCAM_MINI212_DIGITAL_VIDEO_DIGITAL_TYPE_BT656 = 0x04U,
    TCAM_MINI212_DIGITAL_VIDEO_DIGITAL_TYPE_USB20_UART = 0x05U,
    TCAM_MINI212_DIGITAL_VIDEO_DIGITAL_TYPE_LCD = 0x06U,
    TCAM_MINI212_DIGITAL_VIDEO_DIGITAL_TYPE_LVDS = 0x07U,
    TCAM_MINI212_DIGITAL_VIDEO_DIGITAL_TYPE_LCD_DVP = 0x08U,
    TCAM_MINI212_DIGITAL_VIDEO_DIGITAL_TYPE_UVC_CDC = 0x09U,
} tcam_mini212_digital_video_digital_type_t;

typedef enum {
    TCAM_MINI212_DIGITAL_VIDEO_CMOS_CONTENT_YUV422 = 0x00U,
    TCAM_MINI212_DIGITAL_VIDEO_CMOS_CONTENT_YUV422_PARAM = 0x01U,
    TCAM_MINI212_DIGITAL_VIDEO_CMOS_CONTENT_Y16 = 0x02U,
    TCAM_MINI212_DIGITAL_VIDEO_CMOS_CONTENT_Y16_PARAM = 0x03U,
    TCAM_MINI212_DIGITAL_VIDEO_CMOS_CONTENT_Y16_YUV422 = 0x04U,
    TCAM_MINI212_DIGITAL_VIDEO_CMOS_CONTENT_Y16_PARAM_YUV422 = 0x05U,
    TCAM_MINI212_DIGITAL_VIDEO_CMOS_CONTENT_TMP = 0x08U,
    TCAM_MINI212_DIGITAL_VIDEO_CMOS_CONTENT_TMP_PARAM = 0x09U,
    TCAM_MINI212_DIGITAL_VIDEO_CMOS_CONTENT_TMP_YUV422 = 0x0AU,
    TCAM_MINI212_DIGITAL_VIDEO_CMOS_CONTENT_TMP_PARAM_YUV422 = 0x0BU,
} tcam_mini212_digital_video_cmos_content_t;

typedef enum {
    TCAM_MINI212_DIGITAL_VIDEO_CMOS_INTERFACE_CMOS16 = 0x00U,
    TCAM_MINI212_DIGITAL_VIDEO_CMOS_INTERFACE_CMOS8_MSB = 0x01U,
    TCAM_MINI212_DIGITAL_VIDEO_CMOS_INTERFACE_CMOS8_LSB = 0x02U,
} tcam_mini212_digital_video_cmos_interface_t;

typedef enum {
    TCAM_MINI212_DIGITAL_VIDEO_FRAME_FREQUENCY_30Hz = 0x00U,
    TCAM_MINI212_DIGITAL_VIDEO_FRAME_FREQUENCY_25Hz = 0x01U,
    TCAM_MINI212_DIGITAL_VIDEO_FRAME_FREQUENCY_9Hz = 0x02U,
    TCAM_MINI212_DIGITAL_VIDEO_FRAME_FREQUENCY_50Hz = 0x03U,
} tcam_mini212_digital_video_frame_frequency_t;

typedef enum {
    TCAM_MINI212_DIGITAL_VIDEO_CLOCK_PHASE_POSEDGE = 0x00U,
    TCAM_MINI212_DIGITAL_VIDEO_CLOCK_PHASE_NEGEDGE = 0x01U,
} tcam_mini212_digital_video_clock_phase_t;

typedef struct {
    tcam_mini212_digital_video_external_sync_mode_t external_sync_mode;
    tcam_mini212_digital_video_digital_type_t digital_type;
    tcam_mini212_digital_video_cmos_content_t cmos_content;
    tcam_mini212_digital_video_cmos_interface_t cmos_interface;
    tcam_mini212_digital_video_frame_frequency_t frame_frequency;
    tcam_mini212_digital_video_clock_phase_t clock_phase;
} tcam_mini212_digital_video_page_t;

typedef enum {
    TCAM_MINI212_SETTINGS_TEMP_RISE_COMPENSATION_DISABLE = 0x00U,
    TCAM_MINI212_SETTINGS_TEMP_RISE_COMPENSATION_ENABLE = 0x01U,
} tcam_mini212_settings_temp_rise_compensation_t;

typedef struct {
    uint8_t auto_compensation_time;
    tcam_mini212_settings_temp_rise_compensation_t temp_rise_compensation;
} tcam_mini212_settings_page_t;

typedef struct {
    uint8_t lens_type;
} tcam_mini212_focus_page_t;

typedef struct {
    uint8_t data[15U];
} tcam_mini212_custom_data_t;

typedef enum {
    TCAM_MINI212_IMAGE_FREEZE_ENABLE = 0U,
    TCAM_MINI212_IMAGE_FREEZE_DISABLE,
    TCAM_MINI212_IMAGE_FREEZE_COUNT,
} tcam_mini212_image_freeze_t;

typedef enum {
    TCAM_MINI212_IMAGE_COLOR_WHITE_HOT = 0U,
    TCAM_MINI212_IMAGE_COLOR_BLACK_HOT,
    TCAM_MINI212_IMAGE_COLOR_RED_OUTLINE,
    TCAM_MINI212_IMAGE_COLOR_FULGURITE,
    TCAM_MINI212_IMAGE_COLOR_IRON_RED,
    TCAM_MINI212_IMAGE_COLOR_HOT_IRON,
    TCAM_MINI212_IMAGE_COLOR_MEDICAL,
    TCAM_MINI212_IMAGE_COLOR_ARCTIC,
    TCAM_MINI212_IMAGE_COLOR_RAINBOW_1,
    TCAM_MINI212_IMAGE_COLOR_RAINBOW_2,
    TCAM_MINI212_IMAGE_COLOR_COUNT,
} tcam_mini212_image_color_t;

typedef enum {
    TCAM_MINI212_IMAGE_TONE_WARM = 0U,
    TCAM_MINI212_IMAGE_TONE_COOL,
    TCAM_MINI212_IMAGE_TONE_GREEN_HOT,
    TCAM_MINI212_IMAGE_TONE_COUNT,
} tcam_mini212_image_tone_t;

typedef enum {
    TCAM_MINI212_IMAGE_MODE_PRESET_SOFT = 0U,
    TCAM_MINI212_IMAGE_MODE_PRESET_STANDARD,
    TCAM_MINI212_IMAGE_MODE_PRESET_ENHANCED,
    TCAM_MINI212_IMAGE_MODE_PRESET_HIGHLIGHT,
    TCAM_MINI212_IMAGE_MODE_PRESET_CUSTOM,
    TCAM_MINI212_IMAGE_MODE_PRESET_COUNT,
} tcam_mini212_image_mode_preset_t;

typedef enum {
    TCAM_MINI212_IMAGE_MODE_SPATIAL_DENOISING_LEVEL_1 = 0U,
    TCAM_MINI212_IMAGE_MODE_SPATIAL_DENOISING_LEVEL_2,
    TCAM_MINI212_IMAGE_MODE_SPATIAL_DENOISING_LEVEL_3,
    TCAM_MINI212_IMAGE_MODE_SPATIAL_DENOISING_LEVEL_4,
    TCAM_MINI212_IMAGE_MODE_SPATIAL_DENOISING_LEVEL_COUNT,
} tcam_mini212_image_mode_spatial_denoising_level_t;

typedef enum {
    TCAM_MINI212_IMAGE_MODE_TEMPORAL_DENOISING_LEVEL_1 = 0U,
    TCAM_MINI212_IMAGE_MODE_TEMPORAL_DENOISING_LEVEL_2,
    TCAM_MINI212_IMAGE_MODE_TEMPORAL_DENOISING_LEVEL_3,
    TCAM_MINI212_IMAGE_MODE_TEMPORAL_DENOISING_LEVEL_4,
    TCAM_MINI212_IMAGE_MODE_TEMPORAL_DENOISING_LEVEL_COUNT,
} tcam_mini212_image_mode_temporal_denoising_level_t;

typedef enum {
    TCAM_MINI212_IMAGE_MODE_DETAIL_ENHANCEMENT_LEVEL_1 = 0U,
    TCAM_MINI212_IMAGE_MODE_DETAIL_ENHANCEMENT_LEVEL_2,
    TCAM_MINI212_IMAGE_MODE_DETAIL_ENHANCEMENT_LEVEL_3,
    TCAM_MINI212_IMAGE_MODE_DETAIL_ENHANCEMENT_LEVEL_4,
    TCAM_MINI212_IMAGE_MODE_DETAIL_ENHANCEMENT_LEVEL_COUNT,
} tcam_mini212_image_mode_detail_enhancement_level_t;

typedef enum {
    TCAM_MINI212_IMAGE_MODE_BRIGHTNESS_LEVEL_1 = 0U,
    TCAM_MINI212_IMAGE_MODE_BRIGHTNESS_LEVEL_2,
    TCAM_MINI212_IMAGE_MODE_BRIGHTNESS_LEVEL_3,
    TCAM_MINI212_IMAGE_MODE_BRIGHTNESS_LEVEL_4,
    TCAM_MINI212_IMAGE_MODE_BRIGHTNESS_LEVEL_5,
    TCAM_MINI212_IMAGE_MODE_BRIGHTNESS_LEVEL_COUNT,
} tcam_mini212_image_mode_brightness_level_t;

typedef enum {
    TCAM_MINI212_IMAGE_MODE_CONTRAST_LEVEL_1 = 0U,
    TCAM_MINI212_IMAGE_MODE_CONTRAST_LEVEL_2,
    TCAM_MINI212_IMAGE_MODE_CONTRAST_LEVEL_3,
    TCAM_MINI212_IMAGE_MODE_CONTRAST_LEVEL_4,
    TCAM_MINI212_IMAGE_MODE_CONTRAST_LEVEL_5,
    TCAM_MINI212_IMAGE_MODE_CONTRAST_LEVEL_COUNT,
} tcam_mini212_image_mode_contrast_level_t;

typedef enum {
    TCAM_MINI212_IMAGE_MIRROR_NONE = 0U,
    TCAM_MINI212_IMAGE_MIRROR_Y,
    TCAM_MINI212_IMAGE_MIRROR_X,
    TCAM_MINI212_IMAGE_MIRROR_XY,
    TCAM_MINI212_IMAGE_MIRROR_COUNT,
} tcam_mini212_image_mirror_t;

#define TCAM_MINI212_IMAGE_ZOOM_CENTER_X_MIN (0U)
#define TCAM_MINI212_IMAGE_ZOOM_CENTER_X_MAX (256U - 1U)

#define TCAM_MINI212_IMAGE_ZOOM_CENTER_Y_MIN (0U)
#define TCAM_MINI212_IMAGE_ZOOM_CENTER_Y_MAX (192U - 1U)

typedef enum {
    TCAM_MINI212_IMAGE_ZOOM_RATIO_X8 = 0U,
    TCAM_MINI212_IMAGE_ZOOM_RATIO_X16,
    TCAM_MINI212_IMAGE_ZOOM_RATIO_X24,
    TCAM_MINI212_IMAGE_ZOOM_RATIO_X32,
    TCAM_MINI212_IMAGE_ZOOM_RATIO_X40,
    TCAM_MINI212_IMAGE_ZOOM_RATIO_X48,
    TCAM_MINI212_IMAGE_ZOOM_RATIO_X56,
    TCAM_MINI212_IMAGE_ZOOM_RATIO_X64,
    TCAM_MINI212_IMAGE_ZOOM_RATIO_COUNT,
} tcam_mini212_image_zoom_ratio_t;

typedef enum {
    TCAM_MINI212_STREAM_FRAME_FREQUENCY_50HZ = 0U,
    TCAM_MINI212_STREAM_FRAME_FREQUENCY_30HZ,
    TCAM_MINI212_STREAM_FRAME_FREQUENCY_25HZ,
    TCAM_MINI212_STREAM_FRAME_FREQUENCY_9HZ,
    TCAM_MINI212_STREAM_FRAME_FREQUENCY_COUNT,
} tcam_mini212_frame_frequency_t;

typedef enum {
    TCAM_MINI212_DIGITAL_TYPE_DISABLE = 0U,
    TCAM_MINI212_DIGITAL_TYPE_USB20,
    TCAM_MINI212_DIGITAL_TYPE_CMOS,
    TCAM_MINI212_DIGITAL_TYPE_BT656,
    TCAM_MINI212_DIGITAL_TYPE_BT1120,
    TCAM_MINI212_DIGITAL_TYPE_LVDS,
    TCAM_MINI212_DIGITAL_TYPE_USB20_UART,
    TCAM_MINI212_DIGITAL_TYPE_LCD,
    TCAM_MINI212_DIGITAL_TYPE_LCD_DVP,
    TCAM_MINI212_DIGITAL_TYPE_UVC_CDC,
    TCAM_MINI212_DIGITAL_TYPE_COUNT,
} tcam_mini212_digital_type_t;

typedef enum {
    TCAM_MINI212_CMOS_INTERFACE_CMOS16 = 0U,
    TCAM_MINI212_CMOS_INTERFACE_CMOS8_MSB,
    TCAM_MINI212_CMOS_INTERFACE_CMOS8_LSB,
    TCAM_MINI212_CMOS_INTERFACE_COUNT,
} tcam_mini212_cmos_interface_t;

typedef enum {
    TCAM_MINI212_CMOS_CONTENT_YUV422 = 0U,
    TCAM_MINI212_CMOS_CONTENT_YUV422_PARAM,
    TCAM_MINI212_CMOS_CONTENT_Y16,
    TCAM_MINI212_CMOS_CONTENT_Y16_PARAM,
    TCAM_MINI212_CMOS_CONTENT_Y16_YUV422,
    TCAM_MINI212_CMOS_CONTENT_Y16_PARAM_YUV422,
    TCAM_MINI212_CMOS_CONTENT_TMP,
    TCAM_MINI212_CMOS_CONTENT_TMP_PARAM,
    TCAM_MINI212_CMOS_CONTENT_TMP_YUV422,
    TCAM_MINI212_CMOS_CONTENT_TMP_PARAM_YUV422,
    TCAM_MINI212_CMOS_CONTENT_COUNT,
} tcam_mini212_cmos_content_t;

typedef enum {
    TCAM_MINI212_EXTERNAL_SYNC_MODE_DISABLE = 0U,
    TCAM_MINI212_EXTERNAL_SYNC_MODE_SLAVE,
    TCAM_MINI212_EXTERNAL_SYNC_MODE_MASTER,
    TCAM_MINI212_EXTERNAL_SYNC_MODE_COUNT,
} tcam_mini212_external_sync_mode_t;

typedef enum {
    TCAM_MINI212_SHUTTER_STATE_CLOSE = 0U,
    TCAM_MINI212_SHUTTER_STATE_OPEN,
    TCAM_MINI212_SHUTTER_STATE_COUNT,
} tcam_mini212_shutter_state_t;

typedef enum {
    TCAM_MINI212_AUTO_SHUTTER_COMPENSATION_STATE_ON = 0U,
    TCAM_MINI212_AUTO_SHUTTER_COMPENSATION_STATE_OFF,
    TCAM_MINI212_AUTO_SHUTTER_COMPENSATION_STATE_COUNT,
} tcam_mini212_auto_shutter_compensation_state_t;

#define TCAM_MINI212_AUTO_SHUTTER_COMPENSATION_INTERVAL_MIN_MIN (0U)
#define TCAM_MINI212_AUTO_SHUTTER_COMPENSATION_INTERVAL_MIN_MAX (100U)

#define TCAM_MINI212_BAD_POINT_X_MIN (0U)
#define TCAM_MINI212_BAD_POINT_X_MAX (256U - 1U)

#define TCAM_MINI212_BAD_POINT_Y_MIN (0U)
#define TCAM_MINI212_BAD_POINT_Y_MAX (192U - 1U)

typedef enum {
    TCAM_MINI212_RANGE_N20_150 = 0U,
    TCAM_MINI212_RANGE_0_550,
    TCAM_MINI212_RANGE_COUNT,
} tcam_mini212_range_t;

#define TCAM_MINI212_DISTANCE_DM_MIN (0U)
#define TCAM_MINI212_DISTANCE_DM_MAX (300U)

#define TCAM_MINI212_EMISSIVITY_PERCENT_MIN (0U)
#define TCAM_MINI212_EMISSIVITY_PERCENT_MAX (100U)

#define TCAM_MINI212_HUMIDITY_PERCENT_MIN (0U)
#define TCAM_MINI212_HUMIDITY_PERCENT_MAX (100U)

#define TCAM_MINI212_REFLECTED_TEMP_MULT10_MIN (-1000)
#define TCAM_MINI212_REFLECTED_TEMP_MULT10_MAX (10000)

#define TCAM_MINI212_ATMOSPHERIC_TEMP_MULT100_MIN (-10000)
#define TCAM_MINI212_ATMOSPHERIC_TEMP_MULT100_MAX (32767)

#define TCAM_MINI212_SINGLE_POINT_CALIBRATION_TEMP_MULT10_MIN (-400)
#define TCAM_MINI212_SINGLE_POINT_CALIBRATION_TEMP_MULT10_MAX (8000)

#define TCAM_MINI212_TWO_POINT_CALIBRATION_LOW_TEMP_MULT10_MIN (-400)
#define TCAM_MINI212_TWO_POINT_CALIBRATION_LOW_TEMP_MULT10_MAX (8000)

#define TCAM_MINI212_TWO_POINT_CALIBRATION_HIGH_TEMP_MULT10_MIN (-400)
#define TCAM_MINI212_TWO_POINT_CALIBRATION_HIGH_TEMP_MULT10_MAX (8000)

typedef struct {
    uint16_t x;
    uint16_t y;
    int16_t temp_mult10;
} tcam_mini212_stream_frame_point_t;

typedef struct {
    const uint8_t *data;
    uint16_t width;
    uint16_t height;
    uint8_t bytes_per_item;
    uint32_t stride_item;
} tcam_mini212_stream_frame_matrix_t;

typedef struct {
    uint16_t distance_dm;
    uint8_t emissivity_percent;
    uint8_t humidity_percent;
    int16_t reflected_temp_mult10;
    bool shutter_off;
    tcam_mini212_stream_frame_point_t max_point;
    tcam_mini212_stream_frame_point_t min_point;
    int16_t avg_temp_mult10;
    tcam_mini212_stream_frame_matrix_t mt_matrix;
    tcam_mini212_stream_frame_matrix_t yuv_matrix;
} tcam_mini212_stream_frame_info_t;

tcam_status_t tcam_mini212_probe(const tcam_t *tcam);

tcam_status_t tcam_mini212_get_status_page(const tcam_t *tcam, tcam_mini212_status_page_t *status_page);

tcam_status_t tcam_mini212_get_digital_video_page(const tcam_t *tcam, tcam_mini212_digital_video_page_t *digital_video_page);

tcam_status_t tcam_mini212_get_settings_page(const tcam_t *tcam, tcam_mini212_settings_page_t *settings_page);

tcam_status_t tcam_mini212_get_focus_page(const tcam_t *tcam, tcam_mini212_focus_page_t *focus_page);

tcam_status_t tcam_mini212_get_custom_data(const tcam_t *tcam, tcam_mini212_custom_data_t *custom_data);

tcam_status_t tcam_mini212_do_shutter_compensation(const tcam_t *tcam);

tcam_status_t tcam_mini212_do_scene_compensation(const tcam_t *tcam);

tcam_status_t tcam_mini212_set_image_freeze(const tcam_t *tcam, tcam_mini212_image_freeze_t image_freeze);

tcam_status_t tcam_mini212_set_image_color(const tcam_t *tcam, tcam_mini212_image_color_t image_color);

tcam_status_t tcam_mini212_set_image_tone(const tcam_t *tcam, tcam_mini212_image_tone_t image_tone);

tcam_status_t tcam_mini212_set_image_mode_preset(const tcam_t *tcam, tcam_mini212_image_mode_preset_t image_mode_preset);

tcam_status_t tcam_mini212_set_image_mode_spatial_denoising_level(const tcam_t *tcam, tcam_mini212_image_mode_spatial_denoising_level_t image_mode_spatial_denoising_level);

tcam_status_t tcam_mini212_set_image_mode_temporal_denoising_level(const tcam_t *tcam, tcam_mini212_image_mode_temporal_denoising_level_t image_mode_temporal_denoising_level);

tcam_status_t tcam_mini212_set_image_mode_detail_enhancement_level(const tcam_t *tcam, tcam_mini212_image_mode_detail_enhancement_level_t image_mode_detail_enhancement_level);

tcam_status_t tcam_mini212_set_image_mode_brightness_level(const tcam_t *tcam, tcam_mini212_image_mode_brightness_level_t image_mode_brightness_level);

tcam_status_t tcam_mini212_set_image_mode_contrast_level(const tcam_t *tcam, tcam_mini212_image_mode_contrast_level_t image_mode_contrast_level);

tcam_status_t tcam_mini212_set_image_mirror(const tcam_t *tcam, tcam_mini212_image_mirror_t image_mirror);

tcam_status_t tcam_mini212_set_image_zoom_center_x(const tcam_t *tcam, uint16_t image_zoom_center_x);

tcam_status_t tcam_mini212_set_image_zoom_center_y(const tcam_t *tcam, uint16_t image_zoom_center_y);

tcam_status_t tcam_mini212_set_image_zoom_ratio(const tcam_t *tcam, tcam_mini212_image_zoom_ratio_t image_zoom_ratio);

tcam_status_t tcam_mini212_set_frame_frequency(const tcam_t *tcam, tcam_mini212_frame_frequency_t frame_frequency);

tcam_status_t tcam_mini212_set_digital_type(const tcam_t *tcam, tcam_mini212_digital_type_t digital_type);

tcam_status_t tcam_mini212_set_cmos_interface(const tcam_t *tcam, tcam_mini212_cmos_interface_t cmos_interface);

tcam_status_t tcam_mini212_set_cmos_content(const tcam_t *tcam, tcam_mini212_cmos_content_t cmos_content);

tcam_status_t tcam_mini212_set_external_sync_mode(const tcam_t *tcam, tcam_mini212_external_sync_mode_t external_sync_mode);

tcam_status_t tcam_mini212_do_save_settings(const tcam_t *tcam);

tcam_status_t tcam_mini212_do_restore_factory_settings(const tcam_t *tcam);

tcam_status_t tcam_mini212_set_shutter_state(const tcam_t *tcam, tcam_mini212_shutter_state_t shutter_state);

tcam_status_t tcam_mini212_set_auto_shutter_compensation_state(const tcam_t *tcam, tcam_mini212_auto_shutter_compensation_state_t auto_shutter_compensation_state);

tcam_status_t tcam_mini212_set_auto_shutter_compensation_interval(const tcam_t *tcam, uint8_t auto_shutter_compensation_interval_min);

tcam_status_t tcam_mini212_set_bad_point_x(const tcam_t *tcam, uint16_t bad_point_x);

tcam_status_t tcam_mini212_set_bad_point_y(const tcam_t *tcam, uint16_t bad_point_y);

tcam_status_t tcam_mini212_do_bad_point_add(const tcam_t *tcam);

tcam_status_t tcam_mini212_do_bad_point_save(const tcam_t *tcam);

tcam_status_t tcam_mini212_set_range(const tcam_t *tcam, tcam_mini212_range_t range);

tcam_status_t tcam_mini212_set_distance(const tcam_t *tcam, uint16_t distance_dm);

tcam_status_t tcam_mini212_set_emissivity(const tcam_t *tcam, uint8_t emissivity_percent);

tcam_status_t tcam_mini212_set_humidity(const tcam_t *tcam, uint8_t humidity_percent);

tcam_status_t tcam_mini212_set_reflected_temp(const tcam_t *tcam, int16_t reflected_temp_mult10);

tcam_status_t tcam_mini212_set_atmospheric_temp(const tcam_t *tcam, int16_t atmospheric_temp_mult100);

tcam_status_t tcam_mini212_set_single_point_calibration_temp(const tcam_t *tcam, int16_t single_point_calibration_temp_mult10);

tcam_status_t tcam_mini212_do_single_point_calibration_acquisition(const tcam_t *tcam);

tcam_status_t tcam_mini212_do_single_point_calibration(const tcam_t *tcam);

tcam_status_t tcam_mini212_set_two_point_calibration_low_temp(const tcam_t *tcam, int16_t two_point_calibration_low_temp_mult10);

tcam_status_t tcam_mini212_do_two_point_calibration_low_acquisition(const tcam_t *tcam);

tcam_status_t tcam_mini212_set_two_point_calibration_high_temp(const tcam_t *tcam, int16_t two_point_calibration_high_temp_mult10);

tcam_status_t tcam_mini212_do_two_point_calibration_high_acquisition(const tcam_t *tcam);

tcam_status_t tcam_mini212_do_two_point_calibration(const tcam_t *tcam);

tcam_status_t tcam_mini212_set_custom_data(const tcam_t *tcam, const tcam_mini212_custom_data_t *custom_data);

tcam_status_t tcam_mini212_stream_start(const tcam_t *tcam);

tcam_status_t tcam_mini212_stream_frame_acquire(const tcam_t *tcam, void **frame, uint32_t timeout_ms);

tcam_status_t tcam_mini212_stream_frame_release(const tcam_t *tcam);

tcam_status_t tcam_mini212_stream_frame_decode(const tcam_t *tcam, const void *frame, tcam_mini212_stream_frame_info_t *frame_info);

#endif
