#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>

#define F_CPU 8000000UL
#include <util/delay.h>

FUSES = {
    .low = (FUSE_CKSEL0 & FUSE_CKSEL1 & FUSE_CKSEL3 & FUSE_SUT0),
    .high = (FUSE_SPIEN & FUSE_BODLEVEL & FUSE_BODEN),
};

uint8_t adc_val, counter;

void send_pulse(uint8_t a, uint8_t b, uint8_t c)
{

}

void main(void)
{
    uint8_t s1=0, s2=0, s3=0;

    // setup timer
    TCCR0A = 0;
    TCCR0B = 0;
    TIMSK0 = 0;

    // setup adc
    ADMUX = 0;
    ADCSRA = 0;
    ADCSRB = 0;


    for(uint16_t i;;i++) // main loop
    {
        // send pulses
        send_pulse(s1, s2, s3);

        // calculate pulse widths


        // wait for next cycle (20ms period)
    }
}

ISR(TIM0_OVF_vect)
{
    counter++;
}

ISR(ADC_vect)
{
    adc_val = ADCH;
}
