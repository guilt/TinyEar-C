# ESP32-S3 target

Same `src/ear.c` on the watch. v0.1 is host-only; this is the port contract.

```
PDM CLK   GPIO 42
PDM DIN   GPIO 41
```

16 kHz, 16-bit, mono. Accumulate ~200 ms before classifying a vowel.
The chip does not caption. No transcript → `I did not catch words.`
