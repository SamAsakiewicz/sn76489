#include <stdint>

//
//                  +-----------+
// D5        --> _1 | SN76489AN | 16  <-- VCC (5v Nominal)
// D6        -->  2 |           | 15  <-- D4
// D7        -->  3 |           | 14  <-- CLOCK
// READY     <--  4 |           | 13  <-- D3
// !WE       -->  5 |           | 12  <-- D2
// !CE       -->  6 |           | 11  <-- D1
// AUDIO OUT <--  7 |           | 10  <-- D0
// GND       ---  8 |           | 9   --- NC
//                  +-----------+
//
// ref: http://www.smspower.org/Development/SN76489#HowTheSN76489MakesSound
//
// 4 Channels: 
// - Tone 0, 1, 2
// -- Tone Generator Control
// --- Frequency = CLOCK(hz) / 32 * 10bit counter
//
// - Attenuation Control (4 bit volume)
//                 +-------------------------+
//   Attenuation = | a0  +a1  +a2  +a3   bit |
//                 |-16  -8   -4   -2     db |
//                 +-------------------------+
enum attenuation {  // a0 a1 a3 a3
    vol_2db  = 0x1, //  0  0  0  1
    vol_4db  = 0x2, //  0  0  1  0
    vol_8db  = 0x4, //  0  1  0  0
    vol_16db = 0x8, //  1  0  0  0
    vol_off  = 0xf, //  1  1  1  1
    vol_mask = 0xf, //  1  1  1  1
    db_conv  = 2, // 2x multipler to get db from bit defines
}
//   0000 == 0db
//   0001 == -2db
//   1001 == -18db
// * 1111 == OFF
//
//
// - Noise 0
//
//
// Registers:
//
enum registers {
    reg_tone_0_frequency   = 0b000,
    reg_tone_0_attenuation = 0b001,
    reg_tone_1_frequency   = 0b010,
    reg_tone_1_attenuation = 0b011,
    reg_tone_2_frequency   = 0b100,
    reg_tone_2_attenuation = 0b101,
    reg_noise_control      = 0b110,
    reg_noise_attenuation  = 0b111,
    reg_shift = 4,
};

enum channels {
    channel_0, = 0,
    channel_1, = 1,
    channel_2, = 2,
    channel_3, = 3,
};

enum noise_feedback_control {
    noise_feedback_periodic = 0x0,
    noise_feedback_white    = 0x1,

    noise_feedback_mask     = 0x1
};

enum noise_shift_control {
    noise_shift_512 =  0x0, // n/512
    noise_shift_1024 = 0x1, // n/1024
    noise_shift_2048 = 0x2, // n/2048
    noise_shift_tone = 0x3, // Tone gen 2 output

    noise_shift_mask = 0x3
};

void send(uint8_t byte) {
}

void update_freq(uint8_t c, uint16_t f) {
    uint8_t reg = c * 2 + 0;
    if (reg > reg_noise_control)
        return;
   
    // write == | 1 | r0 r1 r2 | f6 f7 f8 f9 |
    // write += | 0 | x f0 f1 f2 f3 f4 f5  |
    send(0x10 | reg << reg_shift | f & (0xf << 6));
    send(0x00 | f & 0x3f);
}

void update_noise(uint8_t feedback, uint8_t shift) {
    feedback &= noise_feedback_mask;
    shift &= noise_shift_mask;

    // write == | 1 | r0 r1 r2 | x | fb | sh0 sh1 |
    send( 0x10 | reg_noise_control << reg_shift | feedback << 2 | shift);
}

void update_vol(uint8_t c, uint8_t v) {

    uint8_t reg = c * 2 + 1;
    if (reg > reg_noise_attenuation)
        return;

    
    // format | 1 | r0 r1 r2 | a0 a1 a2 a3 |
    uint8_t write = (0x10 | (reg << reg_shift | (v/db_conv & vol_mask));

    // printf("update volume - channel:%d, -%ddb (== %ddb)\r\n", c, v, v/db_conv & vol_mask);

    send(write);
}




