# Porting

The library uses `tcam_ops_t` (`src/tcam.h`): seven required callbacks and no `context` pointer. Three of them operate the command UART, four operate the video capture path. Ports normally keep the controller state in file-scope variables next to the callbacks.

Call `tcam_init()` before anything else. It validates the object, the model and all seven callbacks, and stores the table by pointer: the table must have static storage duration and stay valid while bound. Nothing is generated, and there is no configuration header.

The library never calls a port callback from interrupt context and never re-enters the port from inside a callback. It does not lock: a port shared by more than one task needs external serialization, and the camera object itself must not be used concurrently.

## Exchange order

Each driver function performs one exchange. Both drivers flush stale input first; the difference is the acknowledgement style.

| Step | MINI212 | TM30 |
| --- | --- | --- |
| 1 | `cmd.flush_rx(2, 20)` | `cmd.flush_rx(2, 20)` |
| 2 | `cmd.send(frame, len, 20)` — request | `cmd.send(frame, len, 20)` — request |
| 3 | `cmd.recv(ack, 6, 200)` — `55 AA 01 00 01 F0` | `cmd.recv(response, len, 200)` — one response frame |
| 4 | Reads only: `cmd.recv(response, len, 200)` | none |

A write ends after step 3 and requires the fixed `0x00` acknowledgement data. A read accepts the acknowledgement and then the response frame. TM30 write acknowledgements are response frames whose single data byte is `0x01`; the library checks the class, sub and flag echo itself.

The timeouts are compile-time constants in each driver and appear in the table as (idle ms, total ms) for `flush_rx` and as milliseconds for the others.

## Command frame formats

For transport debugging only; the library encodes and validates these itself.

MINI212, maximum data 19 bytes and maximum frame 24 bytes:

```
55 AA | len | data[len] | xor | F0
```

`len` counts data bytes. `xor` is the XOR of the `len` byte and every data byte.

TM30, maximum data 48 bytes and maximum frame 56 bytes:

```
F0 | size | 36 | class | sub | flag | data[size - 4] | sum | FF
```

`size` counts four routing bytes plus the data. `sum` is the 8-bit sum of the four routing bytes, the data and nothing else. Requests use flag `0x00` (write) or `0x01` (read); responses carry flag `0x03`.

## Contracts

### `cmd.send(const uint8_t *data, uint32_t len, uint32_t timeout_ms)`

Transmit every byte within `timeout_ms`. Return `TCAM_ERROR` if the transport cannot take them all in time; do not report success for a partial write, because the frame that follows would be addressed and checksummed against a truncated prefix. The largest frame is 56 bytes. The library passes `data` on the stack of the calling function, so the callback must have finished with it before returning.

### `cmd.recv(uint8_t *data, uint32_t len, uint32_t timeout_ms)`

Receive exactly `len` bytes into `data` within `timeout_ms` and return `TCAM_OK`. A shorter read at the deadline is a failure: the library has already decided the exact frame length from the protocol, validates header, size, checksum and tail itself, and treats a timeout as a failed exchange. Leave `data` in whatever state the failure produced; the library will not read it after an error.

### `cmd.flush_rx(uint32_t idle_ms, uint32_t timeout_ms)`

Discard incoming bytes until the line has been quiet for `idle_ms`, bounded by `timeout_ms`, and return `TCAM_OK` once idle is reached. The drivers call it before every command with a 2 ms idle window and a 20 ms bound. Continuous noise that never goes quiet before the bound must return `TCAM_ERROR`; the command is then skipped rather than sent into a dirty line.

### `stream.configure(uint32_t frame_size)`

Arm the capture path for exactly `frame_size` bytes without starting it: a DCMI-style receiver programs its buffer and transfer length, a DMA setup points at the frame buffer, and so on. The library calls it from `tcam_stream_start()` with the model's size (197,632 for MINI212, 174,720 for TM30) and expects success on every call, including a repeated `tcam_stream_start()`.

### `stream.start(void)`

Start capture now that the receiver is armed. Called immediately after a successful `configure`. Return `TCAM_ERROR` if the capture cannot be started; the library then reports the `tcam_stream_start()` failure.

### `stream.acquire_frame(void **frame, uint32_t timeout_ms)`

Wait up to `timeout_ms` for one complete frame and store its start address in `*frame`. The buffer must hold a whole frame, stay unmodified and stay valid until `release_frame` returns; the library reads the parameter block and hands out matrix views into it. One outstanding frame at a time is enough: the drivers never call `acquire_frame` twice without a release. Return `TCAM_ERROR` on timeout.

### `stream.release_frame(void)`

Hand the buffer back to the capture path. The library calls it only after a successful acquire and only when it no longer needs the frame contents, so decode before release and do not touch the frame afterwards. Return `TCAM_ERROR` only if the capture path cannot be reused; the library propagates the error.

## Which callbacks a call uses

| Library call | Port callbacks |
| --- | --- |
| `tcam_init` | none |
| `tcam_device_probe`, `tcam_*_probe`, every `get_*` | `cmd.flush_rx`, `cmd.send`, `cmd.recv` (twice for MINI212) |
| every `set_*`, every `do_*` | `cmd.flush_rx`, `cmd.send`, `cmd.recv` (once) |
| `tcam_stream_start` | `stream.configure`, `stream.start` |
| `tcam_stream_frame_acquire` | `stream.acquire_frame` |
| `tcam_stream_frame_release` | `stream.release_frame` |
| `tcam_stream_frame_decode` | none; frame memory only |

## Board bring-up

Power sequencing, reset, pin muxing, the UART baud rate, DCMI/DVP configuration and the module's boot time are outside the library. Two points that the library makes visible: the first `tcam_device_probe()` must not run before the module is ready (TM30 answers roughly 2.6 s after power-on, so early probes time out), and the stream layout decoded by this release assumes the module's MT+YUV content mode with the default geometry. A MINI212 frame buffer needs 197,632 bytes and an alignment suitable for the DMA engine that fills it; the library adds no alignment requirement of its own.
