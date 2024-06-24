#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>
#include <avr/pgmspace.h>

#define F_CPU 2400000UL
#include <util/delay.h>

FUSES = {
    .low = (FUSE_SPIEN & FUSE_SUT0 & FUSE_CKSEL0),
    .high = (HFUSE_DEFAULT),
};

#define S_PORT PORTB
#define S_DDR DDRB
#define S1_N PB0
#define S2_N PB1
#define S3_N PB2

#define ADC_VAL ADCH

uint8_t g_counter;

void send_pulse(uint8_t a, uint8_t b, uint8_t c)
{
    S_PORT |= _BV(S1_N) | _BV(S2_N) | _BV(S3_N); // start pulse

    g_counter = 0;
    TCNT0 = 0; // reset counters

    while (TCNT0 < 150)
        _NOP(); // wait 1ms (minimum period)

    TCNT0 = 0;
    // clear each pin when their period ends (timer matches)
    while (g_counter == 0)
    {
        if (TCNT0 >= a)
            S_PORT &= ~_BV(S1_N);
        if (TCNT0 >= b)
            S_PORT &= ~_BV(S2_N);
        if (TCNT0 >= c)
            S_PORT &= ~_BV(S3_N);
    }

    S_PORT &= ~(_BV(S1_N) | _BV(S2_N) | _BV(S3_N)); // clear every pin just in case
}

// sine lookup table/function
uint8_t const sin_lut[] PROGMEM = {0, 2, 3, 5, 6, 8, 9, 11,
                                   13, 14, 16, 17, 19, 20, 22, 23,
                                   25, 27, 28, 30, 31, 33, 34, 36,
                                   37, 39, 41, 42, 44, 45, 47, 48,
                                   50, 51, 53, 54, 56, 57, 59, 60,
                                   62, 63, 65, 67, 68, 70, 71, 73,
                                   74, 76, 77, 79, 80, 81, 83, 84,
                                   86, 87, 89, 90, 92, 93, 95, 96,
                                   98, 99, 100, 102, 103, 105, 106, 108,
                                   109, 110, 112, 113, 115, 116, 117, 119,
                                   120, 122, 123, 124, 126, 127, 128, 130,
                                   131, 132, 134, 135, 136, 138, 139, 140,
                                   142, 143, 144, 146, 147, 148, 149, 151,
                                   152, 153, 154, 156, 157, 158, 159, 161,
                                   162, 163, 164, 165, 167, 168, 169, 170,
                                   171, 172, 174, 175, 176, 177, 178, 179,
                                   180, 181, 183, 184, 185, 186, 187, 188,
                                   189, 190, 191, 192, 193, 194, 195, 196,
                                   197, 198, 199, 200, 201, 202, 203, 204,
                                   205, 206, 207, 208, 208, 209, 210, 211,
                                   212, 213, 214, 215, 215, 216, 217, 218,
                                   219, 220, 220, 221, 222, 223, 223, 224,
                                   225, 226, 226, 227, 228, 228, 229, 230,
                                   231, 231, 232, 232, 233, 234, 234, 235,
                                   236, 236, 237, 237, 238, 238, 239, 240,
                                   240, 241, 241, 242, 242, 243, 243, 244,
                                   244, 244, 245, 245, 246, 246, 247, 247,
                                   247, 248, 248, 248, 249, 249, 249, 250,
                                   250, 250, 251, 251, 251, 252, 252, 252,
                                   252, 252, 253, 253, 253, 253, 253, 254,
                                   254, 254, 254, 254, 254, 254, 255, 255,
                                   255, 255, 255, 255, 255, 255, 255, 255};
uint8_t mysin(uint8_t x)
{
    // uint8_t i = x / (256l / (sizeof(sin_lut) / sizeof(sin_lut[0]))); // 0-255 to 0-pi/2
    return pgm_read_byte(&sin_lut[x]);
}

int main(void)
{
    // setup system clock prescaler
    CLKPR = _BV(CLKPCE);
    CLKPR = _BV(CLKPS1); // div by 4 = 2.4Mhz

    // setup pins
    S_DDR |= _BV(S1_N) | _BV(S2_N) | _BV(S3_N);     // set as outputs
    S_PORT &= ~(_BV(S1_N) | _BV(S2_N) | _BV(S3_N)); // clear pins

    // setup timer
    TCCR0B = _BV(CS01);  // div by 8
    TIMSK0 = _BV(TOIE0); // enable interrupt

    // setup adc
    ADMUX = _BV(ADLAR) | _BV(MUX1) | _BV(MUX0); // ref=vcc, left adj, pin PB3
    // enable, start, free running, minimum frequency:
    ADCSRA = _BV(ADEN) | _BV(ADSC) | _BV(ADATE) | _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);

    _delay_ms(10);

    // main loop
    uint8_t s1 = 0, s2 = 0, s3 = 0, t1 = 0, t2 = 0, t3 = 0;
    sei();
    for (uint8_t i = 0xFF;; i++)
    {
        // send pulses
        if (s1 != 0)
            send_pulse(s1, s2, s3);

        // calculate next pulse widths
        if (i < t1)
        {
            uint8_t t = mysin(256l * i / t1);
            s1 = 224 - t / 2;
            s2 = s1;
            s3 = 126 + t / 4;
        }
        else if (i < t2)
        {
            s1 = 224;
            s3 = 126;
        }
        else if (i < t3)
        {
            s2 = 224;
        }
        else
        {
            i = 0;
            uint8_t speed = ADC_VAL / 2; // 0 to 127
            t1 = 25 + speed;
            t2 = t1 + 5 + speed / 9;
            t3 = t2 + 8 + speed / 3;
        }

        // wait for next pulse
        while (g_counter <= 9)
            _NOP();
    }

    return 0;
}

ISR(TIM0_OVF_vect)
{
    g_counter++;
}
