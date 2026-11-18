# tcam

Thermal camera command and frame drivers for embedded C99 targets, with no OS dependency, no HAL, no dynamic allocation and no writable static data.

The library drives two camera modules behind one caller-supplied port. For each module it owns the command framing, the checksum, the response validation, the parameter units and ranges, and the parameter block that travels with the video stream. The transport is yours: three command callbacks and four stream callbacks in `tcam_ops_t`, bound once with `tcam_init()` against an explicit model. Power, reset, pin muxing, UART/DCMI start-up and the frame memory stay outside the library.

## Scope

Supported:

- One camera per `tcam_t` object, bound to `TCAM_MODEL_MINI212` or `TCAM_MODEL_TM30` at initialisation. Every driver function refuses an object bound to a different model.
- MINI212: status, digital-video, settings, focus and custom-data pages; image colour, tone, preset, spatial/temporal denoising, detail enhancement, brightness, contrast, mirror and zoom; shutter and scene compensation; bad-point add and save; distance, emissivity, humidity, reflected and atmospheric temperature; single- and two-point calibration; save and factory restore; stream start and frame decode.
- TM30: CID, serial number, model number, firmware and FPGA versions and build dates, camera calibration date and time; reset, save and factory restore; shutter, background and vignetting correction; auto-shutter mode and timed interval; cursor display, position and bad-point control; brightness, contrast, detail enhancement and both denoisers; 16 colour palettes, mirror and rotate; stream format and YUV resolution; measurement enable, unit and range; emissivity, distance, reflected and atmospheric temperature, humidity; calibration parameters and calibration state; stream start and frame decode.
- Complete-frame capture for both models: the port arms for the model's exact frame size, `acquire` hands over one complete frame, and `decode` validates the embedded parameter block and returns read-only MT and YUV matrix views plus frame statistics.

Not supported, by design:

- Runtime model detection or a common dispatcher. The protocols do not share an identification exchange, so the application states the model once and calls that model's entry points.
- The transport: UART exchange, DCMI/DVP capture, DMA, the frame buffer and in-transfer timeouts belong to the port.
- Power sequencing, reset lines, clocking, hot-plug handling and communication recovery.
- Converting MT words to temperatures: the matrices are returned raw. Only the decoded frame statistics are converted.
- Targets that are not little-endian or do not use IEEE-754 floats: the MINI212 parameter block is copied through a structure, and TM30 temperatures are little-endian float bit patterns.
- Stream layouts other than the ones in [Frames](#frames). A MINI212 digital-video content mode without the MT+YUV422 pair, or a TM30 YUV resolution other than 240x240, changes the device's frame size and is not decoded.

## Requirements

- C99 and `<stdint.h>`, `<stdbool.h>`, `<stddef.h>`, `<string.h>`. No CMSIS, device or OS header.
- One caller-owned `tcam_t` per camera and one caller-owned `const tcam_ops_t` with all seven callbacks, kept valid while bound.
- Blocking calls from task or main-loop context. The library has no lock and no interrupt-safety contract; serialize access to the command path and to the stream path yourself.

## Quick start

```c
#include "tcam.h"
#include "tcam_tm30.h"   /* or tcam_mini212.h */

static tcam_t camera;

static const tcam_ops_t port_ops = {
    .cmd.send            = my_cmd_send,
    .cmd.recv            = my_cmd_recv,
    .cmd.flush_rx        = my_cmd_flush_rx,
    .stream.configure    = my_stream_configure,
    .stream.start        = my_stream_start,
    .stream.acquire_frame = my_stream_acquire_frame,
    .stream.release_frame = my_stream_release_frame,
};

/* Power and reset the module, then wait for its boot time: up to a few
   seconds on TM30. The library does none of that. */
if (tcam_init(&camera, TCAM_MODEL_TM30, &port_ops) != TCAM_OK) {
    /* A NULL object, an unknown model, or a missing callback. */
}

if (tcam_tm30_probe(&camera) != TCAM_OK) {
    /* No answer, or a response that failed validation. Retry on your
       own schedule during start-up. */
}

if (tcam_tm30_set_emissivity(&camera, 95U) != TCAM_OK) {
    /* Out of range, or no valid acknowledgement. */
}

if (tcam_tm30_stream_start(&camera) != TCAM_OK) {
    /* The port refused the 174,720-byte receiver setup or could not start. */
}

void *frame;
if (tcam_tm30_stream_frame_acquire(&camera, &frame, 1000U) == TCAM_OK) {
    tcam_tm30_stream_frame_info_t info;

    if (tcam_tm30_stream_frame_decode(&camera, frame, &info) == TCAM_OK) {
        /* info.max_point.temp_mult10, info.mt_matrix, info.yuv_matrix, ... */
    }
    (void)tcam_tm30_stream_frame_release(&camera);
}
```

`tcam_init()` is called once; every later call takes only the camera object, because the model tag and the ops pointer are stored in it. The two model headers are independent: include only the one whose functions you call, and compile only the driver you use if you prefer.

## Frames

The stream path is sized for one layout per model. `stream_start()` calls the port's configure callback with the exact byte count.

| Model | Frame size | Content |
| --- | ---: | --- |
| MINI212 | 197,632 B (512 x 193 x 2) | Lines 0-191: MT 256 x 192 u16 in items 0-255 and YUV 256 x 192 u16 in items 256-511, stride 512 items / 1024 B. Line 192: 120-byte parameter block at byte offset 196,608. |
| TM30 | 174,720 B (240 x 364 x 2) | Lines 0-27: parameter block, 192 bytes used of each 480-byte line. Lines 28-123: MT 96 x 96 u16 in the first 192 bytes of each line, byte offset 13,440. Lines 124-363: YUV 240 x 240 u16, full lines, byte offset 59,520. |

`decode` validates the parameter signature (`0x55AA`/`0x0038`/`0x6666` for MINI212, `0x70827789` for TM30) and fills the frame info: minimum, maximum and average temperature with their coordinates, model-specific fields, and two matrix descriptors. A descriptor carries `data`, `width`, `height`, `bytes_per_item` and `stride_item`; the stride is in items per line, not bytes, and `data` points into the acquired frame, so the views are valid only until `release_frame()`.

The matrices are device data, not library conversions: MINI212 MT words are signed 16-bit in 0.1 °C, and TM30 MT words encode `raw / 64 - 50` °C. The exact parameter offsets are listed in [doc/api.md](doc/api.md).

## Timing

The two drivers issue every command exchange with fixed timeouts: `send` 20 ms, `recv` 200 ms, and `flush_rx` with a 2 ms idle window inside a 20 ms bound. They are compile-time constants, not API parameters, and the port must honour the argument it receives. A 200 ms receive window fits the command responses but not a module that is still booting: a TM30 answers roughly 2.6 s after power-on, so early probes fail with a timeout by design and the start-up retry policy belongs to the application.

## Porting

See [doc/porting.md](doc/porting.md) for the contract of each ops entry, the exchange order the drivers use, and the command frame formats for transport debugging. [examples/tcam_port_template.c](examples/tcam_port_template.c) is a non-functional skeleton with all seven callbacks.

## Integration

Add `src/` to the include path and compile `src/tcam.c` plus the driver for the model you use. Both drivers are independent translation units and share no configuration; nothing is generated and there is no configuration header.

## Verification

`sh build.sh` compiles the library and the example with the host compiler and every warning enabled (`-Wconversion`, `-Wsign-conversion`, `-Wshadow`, `-Wundef`, `-Wwrite-strings`, `-Werror`). Override the compiler with `CC=...`. The example skeletons the port and is not runnable; hardware behaviour must be verified on the target.

## License

BSD 3-Clause, see [LICENSE](LICENSE).

## Layout

```
src/        tcam.[ch]                 object binding and shared declarations
            tcam_mini212.[ch]         MINI212 driver
            tcam_tm30.[ch]            TM30 driver
doc/        api.md                    function contract, units and frame layout
            porting.md                ops contract and exchange order
examples/   tcam_port_template.c      annotated non-functional port
build.sh    strict-warning compile check, no hardware involved
```
