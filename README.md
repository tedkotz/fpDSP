# fpDSP
Fixed point 16-bit Digital Signal Processing library for arduino, embedded and 8-bit systems.

The PlusCLI project has some sample test commands to play with this.

## Inspirations
- https://en.wikipedia.org/wiki/CORDIC
- https://en.wikipedia.org/wiki/Binary_angular_measurement
- [Reducible's FFT video](https://www.youtube.com/watch?v=h7apO7q16V0)
- https://en.wikipedia.org/wiki/WAV


### Features/TODO
- [ ] Add Doxygen comments to header
- [ ] Split CORDIC in generic functions (rect2polar, polar2rec, rotateVector)
- [ ] Realtime sample processing.
    - interrupt driven
    - http://gammon.com.au/forum/?id=11488&reply=5#reply5
    - https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/
    - https://forum.arduino.cc/t/adcsra-adc-control-and-status-register-a/319966
    - https://forum.arduino.cc/t/what-timers-do-the-arduino-libraries-use/43133
    - https://forum.arduino.cc/t/timer-interrupt-in-microseconds/335219
    - sample rate
        - Target should divide evenly into 44.1k or 48k
             - easier to convert to most popular audio sample rates
             - GCD 300
        - Not too high to keep processing load down
        - Cover telephone audio range (300Hz-3.4kHz)
            - Covers human voice and classic modem experiments
        - 6000, 6300, 7350, 8000, 8820, 11025

