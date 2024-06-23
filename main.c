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
uint8_t const sin_lut[] PROGMEM = {0, 6, 12, 18, 25, 31, 37, 43,
                                   49, 56, 62, 68, 74, 80, 86, 92,
                                   97, 103, 109, 115, 120, 126, 131, 136,
                                   142, 147, 152, 157, 162, 167, 171, 176,
                                   181, 185, 189, 193, 197, 201, 205, 209,
                                   212, 216, 219, 222, 225, 228, 231, 234,
                                   236, 238, 241, 243, 244, 246, 248, 249,
                                   251, 252, 253, 254, 254, 255, 255, 255};
uint8_t mysin(uint8_t x)
{
    uint8_t i = x / (256l / (sizeof(sin_lut) / sizeof(sin_lut[0]))); // 0-255 to 0-pi/2
    return pgm_read_byte(&sin_lut[i]);
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
            s3 = 126;
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
