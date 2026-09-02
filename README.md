# TinyEar-C

C port of [TinyEar](https://github.com/guilt/tinyear). Short clip in, class out.
Honest when words fail. Target: **ESP32-S3** (16 kHz I2S/PDM mic).

```bash
make test && make
./ear /tmp/a.wav
```

Belief without a human transcript is always `I did not catch words.`

See `esp32/README.md` for the S3 pin contract.
