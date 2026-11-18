# API contract

The declarations live in `src/tcam.h` (object and port), `src/tcam_mini212.h` and `src/tcam_tm30.h` (drivers). No function allocates, no function keeps writable static data, and the only state between calls is the caller-owned `tcam_t`. Every function is blocking for the duration of its port exchanges and returns `TCAM_OK` or `TCAM_ERROR`; no other status values exist in this release.

A call that returns `TCAM_ERROR` has performed no processing the caller can use: either the exchange failed or the response failed validation. Output parameters are written only after the parts they depend on have been validated, but a multi-field getter may have stored an earlier field before a later one was rejected, so treat every output of a failed call as unspecified.

## tcam_init

```c
tcam_status_t tcam_init(tcam_t *tcam, tcam_model_t model, const tcam_ops_t *ops);
```

Validates the object pointer, the model (`TCAM_MODEL_UNKNOWN` and values at or beyond `TCAM_MODEL_COUNT` are rejected), the table pointer and all seven callbacks, then stores the model tag and the table pointer. Returns `TCAM_ERROR` without storing anything if a check fails.

The ops table must stay valid for the object's lifetime, normally as a `static const` object. The library keeps a pointer, not a copy. `tcam_init()` sends nothing: the module must already be powered and past its boot time when the first command is issued, and `probe()` is the first exchange.

## Model binding

| Model | Header | Entry points |
| --- | --- | --- |
| `TCAM_MODEL_MINI212` | `tcam_mini212.h` | `tcam_mini212_*` |
| `TCAM_MODEL_TM30` | `tcam_tm30.h` | `tcam_tm30_*` |

There is no common command dispatcher: the frame formats and parameter sets have nothing in common beyond the port. Every driver function checks `tcam->model` first and returns `TCAM_ERROR` without touching the port when the object is bound to the other model, so a mixed call fails fast instead of misaddressing the module.

`probe()` is the first command for either driver. MINI212 reads the status page; TM30 reads the CID. A probe failure means no valid answer, not necessarily a broken module: TM30 answers only a few seconds after power-on.

## Common call shapes

- `get_*` sends one read request and validates the whole response, including the echoed page/command identifiers and the ranges or enum values of every field, before storing outputs.
- `set_*` and `do_*` range-check or enum-check the arguments first, then send one write command and require a valid acknowledgement.
- `*_stream_*` functions delegate to the port (see [Stream](#stream)); `*_stream_frame_decode` is pure and makes no port call.

## MINI212 driver

### Pages

| Function | Returns |
| --- | --- |
| `tcam_mini212_probe` | Reads the status page and discards it; the liveness check. |
| `tcam_mini212_get_status_page` | `pid`; firmware version year/month/day as the raw bytes of the page; `focal_temp_mult100` (0.01 °C units); `machine_id`. |
| `tcam_mini212_get_digital_video_page` | External sync mode, digital type, CMOS content, CMOS interface, frame frequency and clock phase as enums. |
| `tcam_mini212_get_settings_page` | `auto_compensation_time` as reported (pass-through byte); temperature-rise compensation enable. |
| `tcam_mini212_get_focus_page` | `lens_type` byte. |
| `tcam_mini212_get_custom_data` | 15 opaque bytes. |

### Image, shutter and calibration

Setters and actions; every value is checked against the range macro of its declaration:

| Function group | Unit and range |
| --- | --- |
| `set_image_freeze`, `set_image_color`, `set_image_tone`, `set_image_mode_preset` | Enumerators from the header. |
| `set_image_mode_spatial_denoising_level`, `..._temporal_denoising_level`, `..._detail_enhancement_level`, `..._brightness_level`, `..._contrast_level` | Level enumerators. |
| `set_image_mirror` | None, Y, X or XY. |
| `set_image_zoom_center_x`, `..._center_y` | MT pixel coordinates, 0-255 and 0-191. |
| `set_image_zoom_ratio` | x8 up to x64. |
| `set_frame_frequency` | 50, 30, 25 or 9 Hz. |
| `set_digital_type`, `set_cmos_interface`, `set_cmos_content`, `set_external_sync_mode` | Enumerators from the header. |
| `set_shutter_state` | Open or closed. |
| `set_auto_shutter_compensation_state` | On or off; `set_auto_shutter_compensation_interval` takes 0-100 minutes. |
| `set_bad_point_x`, `..._y`, `do_bad_point_add`, `do_bad_point_save` | MT coordinates 0-255 and 0-191, then add and persist. |
| `set_range` | -20 to 150 °C or 0 to 550 °C. |
| `set_distance` | `distance_dm`, 0-300 dm. |
| `set_emissivity`, `set_humidity` | Percent, 0-100 each. |
| `set_reflected_temp` | `reflected_temp_mult10`, 0.1 °C units, -1000 to 10000. |
| `set_atmospheric_temp` | `atmospheric_temp_mult100`, 0.01 °C units, -10000 to 32767. |
| `set_single_point_calibration_temp`, `set_two_point_calibration_low_temp`, `..._high_temp` | 0.1 °C units, -400 to 8000; each has an acquisition action, and the two-point pair also has an apply action. |
| `set_custom_data` | 15 opaque bytes. |
| `do_scene_compensation`, `do_shutter_compensation`, `do_save_settings`, `do_restore_factory_settings` | No arguments. |

Every temperature field carries its scale in the name. The driver converts nothing: a value passed as `_mult10` stays in 0.1 °C, a `_mult100` value stays in 0.01 °C.

## TM30 driver

### Identification, versions and state

| Function | Returns |
| --- | --- |
| `tcam_tm30_probe` | Reads the CID and discards it; the liveness check. |
| `tcam_tm30_get_cid`, `..._get_sn`, `..._get_model_number` | 5, 9 and 48 raw bytes respectively; not NUL-terminated. |
| `tcam_tm30_get_fpga_firmware_version`, `..._get_firmware_version` | Major/minor/patch, decoded from packed BCD to binary. |
| `tcam_tm30_get_fpga_firmware_build_time`, `..._get_firmware_build_time`, `..._get_camera_calibration_time` | Year/month/day, decoded from packed BCD. |
| `tcam_tm30_get_time` | Milliseconds, second, minute, hour, day, week, month and year, binary; the `TCAM_TM30_TIME_*` macros bound each field. |
| `tcam_tm30_do_reset`, `..._do_save_settings`, `..._do_restore_factory_settings` | No arguments. |

Version and date replies are the only BCD-encoded parameters in the driver; time and measurement values are binary.

### Corrections, cursor and image

| Function group | Unit and range |
| --- | --- |
| `do_shutter_correction`, `do_background_correction`, `do_vignetting_correction` | No arguments. |
| `set_auto_shutter_compensation`, `get_auto_shutter_compensation` | Off, timed, temperature-delta or auto. |
| `set_auto_shutter_compensation_timed_interval`, `get_...` | 0-65535 minutes. |
| `set_cursor_display` | On or off. |
| `set_cursor_position` | Direction enumerator with a step of 1-15. |
| `set_cursor_bad_point` | Add or delete. |
| `set_image_brightness`, `..._contrast`, `..._detail_enhancement`, `..._spatial_denoising`, `..._temporal_denoising` | Percent, 0-100; each has a getter. |
| `set_image_color`, `get_image_color` | 16 palettes. |
| `set_image_mirror`, `get_image_mirror` | None, centre, left/right or up/down. |
| `set_image_rotate`, `get_image_rotate` | Enable or disable. |
| `set_stream_format`, `get_stream_format` | MT, raw, YUV, MT+YUV, raw+YUV or YUV image. The stream path expects MT+YUV. |
| `set_yuv_resolution` | 96x96 or 240x240. The stream path is sized for 240x240. |

### Measurement and calibration

| Function group | Unit and range |
| --- | --- |
| `set_temp_measurement`, `get_temp_measurement` | Enable or disable. |
| `set_temp_unit`, `get_temp_unit` | Celsius, Fahrenheit or Kelvin. |
| `set_temp_measurement_range`, `get_temp_measurement_range` | -20 to 150 °C, 100 to 550 °C or automatic. |
| `set_emissivity`, `get_emissivity` | Percent, 1-100. |
| `set_distance`, `get_distance` | `distance_dm`, 3-30 dm. |
| `set_reflected_temp`, `get_reflected_temp` | Enable flag plus `reflected_temp_add100_mult10`, (T + 100) x 10 in 0-11000. |
| `set_atmospheric_temp`, `get_atmospheric_temp` | Enable flag plus `atmospheric_temp_add100_mult10`, (T + 100) x 10 in 0-11000. |
| `set_humidity`, `get_humidity` | `humidity_permille`, 0-1000. |
| `set_calibration_param` | Structure with distance in dm (3-1000), atmospheric temperature (T + 300) x 10 (270-13000), emissivity in percent (1-100), calibration temperature (T + 100) x 10 (600-7500) and normalised X/Y (13-993). Declared with a non-const pointer, matching the in-tree contract; the library only reads it. |
| `do_calibration` | Enable or disable the second calibration. |

## Stream

All four stream functions follow the same shape for both models:

- `*_stream_start()` calls the port's `configure` callback with the model's frame size and then `start`. Calling it again re-arms the port with the same size.
- `*_stream_frame_acquire(tcam, &frame, timeout_ms)` waits up to `timeout_ms` in the port for one complete frame and stores its address. The library never copies the frame; it stays valid until release.
- `*_stream_frame_release()` hands the frame back to the port. Decode before release.
- `*_stream_frame_decode(tcam, frame, &info)` makes no port call. It validates the parameter signature and fills the info structure; matrix views point into `frame`.

| Model | Frame size passed to `configure` |
| --- | ---: |
| MINI212 | 197,632 bytes |
| TM30 | 174,720 bytes |

## Frame decoding

### MINI212

The frame is 512 x 193 x 2 bytes. Lines 0-191 carry MT in items 0-255 and YUV in items 256-511 of each 1024-byte line. Line 192 starts at byte offset 196,608 and its first 120 bytes are the parameter block:

| Offset | Type | Field |
| ---: | --- | --- |
| 0 | u16 | Header, `0x55AA` |
| 2 | u16 | Header, `0x0038` |
| 4 | u16 | Humidity, percent |
| 6 | u16 | Distance, dm |
| 8 | u16 | Emissivity, percent |
| 10 | i16 | Reflected temperature, 0.1 °C |
| 56 | u16 | Shutter-off flag |
| 88, 90 | u16 | Maximum temperature X, Y |
| 92 | i16 | Maximum temperature, 0.1 °C |
| 94, 96 | u16 | Minimum temperature X, Y |
| 98 | i16 | Minimum temperature, 0.1 °C |
| 100, 102 | u16 | Cursor X, Y |
| 104 | i16 | Cursor temperature, 0.1 °C |
| 106 | i16 | Average temperature, 0.1 °C |
| 118 | u16 | Tail, `0x6666` |

Reserved words sit between the listed fields; the struct in `tcam_mini212.c` fixes the spacing. The block is copied through a structure, so the target must be little-endian with 16-bit fields at those offsets. Coordinates are returned as reported (the MT is 256 x 192); temperatures are returned in the `mult10` units above.

Decoded views: MT 256 x 192 u16 at offset 0, stride 512 items; YUV 256 x 192 u16 at byte offset 512, stride 512 items.

### TM30

The frame is 240 x 364 x 2 bytes: 28 parameter lines of 480 bytes, 96 MT lines, then 240 YUV lines. The parameter block is read little-endian at these byte offsets:

| Offset | Type | Field |
| ---: | --- | --- |
| 0 | u32 | Header, `0x70827789` |
| 32 | u32 | Image-frozen flag |
| 144 | f32 | Minimum temperature, °C |
| 148 | f32 | Maximum temperature, °C |
| 152 | f32 | Average temperature, °C |
| 156, 160 | u32 | Maximum temperature X, Y (0-1000) |
| 164, 168 | u32 | Minimum temperature X, Y (0-1000) |

Temperatures are IEEE-754 bit patterns and are multiplied by 10 into `temp_mult10`. Coordinates are scaled from the 0-1000 normalised range onto the 96 x 96 MT grid (0-95). The frozen flag and both points are returned, and the average temperature has no coordinate.

Decoded views: MT 96 x 96 u16 at byte offset 13,440, stride 240 items; YUV 240 x 240 u16 at byte offset 59,520, stride 240 items.

### Matrix contents

The library does not interpret matrix data. On the module side, MINI212 MT words are signed 16-bit in 0.1 °C, and TM30 MT words encode `raw / 64 - 50` °C; YUV words are the module's 16-bit colour pairs. `bytes_per_item` is 2 and `stride_item` counts items per line, not bytes, for every view in this release.
