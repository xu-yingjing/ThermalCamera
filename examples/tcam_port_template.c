/* Non-functional command and stream port; implement the callback contracts in
   doc/porting.md. Nothing here touches hardware, so every call fails until the
   bodies are filled in. */

#include <stddef.h>
#include <stdint.h>

#include "tcam.h"
#include "tcam_tm30.h"

typedef struct {
    /* Everything your board needs to reach the module, kept next to the
       functions that use it: the UART used for commands, the capture
       peripheral used for video, and the buffer that acquire_frame() hands
       out. The ops table carries no context pointer, so this state lives in
       file scope. Add your register bases, DMA stream and pin setup here. */
    void *uart;
    void *capture;
    uint8_t *frame_buffer;
    uint32_t frame_size;
    uint32_t tick_ms;
} example_port_t;

static example_port_t example_port;

/* Transmit every byte within timeout_ms. The largest command frame is 24
   bytes for MINI212 and 56 bytes for TM30; the buffer belongs to the caller. */
static tcam_status_t example_cmd_send(const uint8_t *data, uint32_t len, uint32_t timeout_ms)
{
    (void)data;
    (void)len;
    (void)timeout_ms;

    return TCAM_ERROR;
}

/* Return only after all len bytes have arrived. The library asks for the exact
   frame length the protocol defines and validates header, checksum and tail
   itself, so a timeout or a short read is simply a failed exchange. */
static tcam_status_t example_cmd_recv(uint8_t *data, uint32_t len, uint32_t timeout_ms)
{
    (void)data;
    (void)len;
    (void)timeout_ms;

    return TCAM_ERROR;
}

/* Drop stale input until the line has been quiet for idle_ms, bounded by
   timeout_ms. Called before every command exchange: a line that never goes
   quiet before the bound must fail instead of sending into the noise. */
static tcam_status_t example_cmd_flush_rx(uint32_t idle_ms, uint32_t timeout_ms)
{
    (void)idle_ms;
    (void)timeout_ms;

    return TCAM_ERROR;
}

/* Arm the capture path for exactly frame_size bytes without starting it.
   197,632 bytes for MINI212, 174,720 for TM30; the buffer must hold a whole
   frame and suit your DMA engine. */
static tcam_status_t example_stream_configure(uint32_t frame_size)
{
    example_port.frame_size = frame_size;

    return TCAM_ERROR;
}

/* Start capture after a successful configure. */
static tcam_status_t example_stream_start(void)
{
    return TCAM_ERROR;
}

/* Wait up to timeout_ms for one complete frame and store its start address.
   The frame stays valid and unmodified until example_stream_release_frame();
   one outstanding frame at a time is enough for this library. */
static tcam_status_t example_stream_acquire_frame(void **frame, uint32_t timeout_ms)
{
    (void)frame;
    (void)timeout_ms;

    return TCAM_ERROR;
}

/* Hand the buffer back. The library calls this only after it has decoded what
   it needs, so the frame contents may be overwritten afterwards. */
static tcam_status_t example_stream_release_frame(void)
{
    return TCAM_ERROR;
}

static const tcam_ops_t example_ops = {
    .cmd.send = example_cmd_send,
    .cmd.recv = example_cmd_recv,
    .cmd.flush_rx = example_cmd_flush_rx,
    .stream.configure = example_stream_configure,
    .stream.start = example_stream_start,
    .stream.acquire_frame = example_stream_acquire_frame,
    .stream.release_frame = example_stream_release_frame,
};

/* Binding succeeds; probe fails until the callbacks above are implemented.
   Power, reset and the module's boot time are the board's job: a TM30 answers
   roughly 2.6 s after power-on, so plan retries around that. */
tcam_status_t example_tm30_up(tcam_t *camera)
{
    tcam_status_t ret;
    void *frame;
    tcam_tm30_stream_frame_info_t frame_info;

    ret = tcam_init(camera, TCAM_MODEL_TM30, &example_ops);
    if (ret != TCAM_OK) {
        return ret;
    }

    ret = tcam_tm30_probe(camera);
    if (ret != TCAM_OK) {
        return ret;
    }

    ret = tcam_tm30_stream_start(camera);
    if (ret != TCAM_OK) {
        return ret;
    }

    ret = tcam_tm30_stream_frame_acquire(camera, &frame, 1000U);
    if (ret != TCAM_OK) {
        return ret;
    }

    ret = tcam_tm30_stream_frame_decode(camera, frame, &frame_info);
    (void)frame_info;
    (void)tcam_tm30_stream_frame_release(camera);

    return ret;
}
