/******************** Pins ****************************
 * PC3: Input, on/off switch, pwm start
 * PC4: Input, cycle through possible durations
 * PC5: Input, on/off switch, hardware interrupt
 * PD7: Output, LED1
 * PD6: Output, LED2
 * PD5: Output, LED3
 * PD4: Output, LED4
 * PD3: Output, LED5
 * PD0: Output, LEDpwm
 *****************************************************/

#include <avr/io.h>
#include <util/delay.h>
#define F_CPU 16000000L
#define T_OUTPUT 213 // us

enum total_duration // number of steps = total time / 213us
{
    // on each step we increment by 1 cycle the HIGH duration of the output
    // signal
    T1 = 23474,
    T2 = 37559,
    T3 = 51643,
    T4 = 65535
};

enum leds
{
    LED1 = PORTD7,
    LED2 = PORTD6,
    LED3 = PORTD5,
    LED4 = PORTD4,
    LED5 = PORTD3,
    LEDpwm = PORTD0
};

void switch_off();
void cycle_duration();
void pwm_start();

volatile int selected_duration = T1;
volatile int intertal_state = 0; // 0 = off; 1 = on

int main(void)
{
    // make the whole PORTD outputs and set LOW
    DDRD = 0xFF;
    PORTD = 0;

    // Pins PC3, PC4, PC5 will be receiving data from external switches
    // setup internal pull ups
    PORTC = (1 << PORTC3) | (1 << PORTC4) | (1 << PORTC5);

    while (1)
    {
        if (!(PINC & (1 << PORTC4)))
            cycle_duration();

        if (!(PINC & (1 << PORTC5)))
        {
            if (intertal_state)
            {
                switch_off();
            }
            else
            {
                pwm_start();
            }
        }

        // TODO: Hardware interrupt based switch on/off
    }
    return 0;
}

void switch_off()
{
    PORTD = 0;
    intertal_state = 0;
}

void cycle_duration()
{
    switch (selected_duration)
    {
    case T1:
        selected_duration = T2;
        PORTD ^= (1 << LED3);
        _delay_ms(1000);
        PORTD ^= (1 << LED3);
        break;
    case T2:
        selected_duration = T3;
        PORTD ^= (1 << LED4);
        _delay_ms(1000);
        PORTD ^= (1 << LED4);
        break;
    case T3:
        selected_duration = T4;
        PORTD ^= (1 << LED5);
        _delay_ms(1000);
        PORTD ^= (1 << LED5);
        break;
    case T4:
        selected_duration = T1;
        PORTD ^= (1 << LED2);
        _delay_ms(1000);
        PORTD ^= (1 << LED2);
        break;
    }
}

void pwm_start()
{
    for (int step = 0; step < T_OUTPUT; step++)
    {
        for (int i = 0; i < selected_duration; i++)
        {
            PORTD |= (1 << LEDpwm);
            int j = step;
            while (j != 0)
            {
                _delay_us(1);
                j--;
            }
            PORTD ^= (1 << LEDpwm);
            j = T_OUTPUT - step;
            while (j != 0)
            {
                _delay_us(1);
                j--;
            }
            // TODO: fix delay overhead
        }

        if (step < 4) // less than 25% progress
            PORTD |= (1 << LED1);
        else if (step < 8) // less than 50% progress
            PORTD |= (1 << LED2);
        else if (step < 12) // less than 75% progress
            PORTD |= (1 << LED3);
        else // more than 75% progress
            PORTD |= (1 << LED4);
    }
    // pwm start process finished
    // turn on LED5 and LEDpwm
    PORTD |= (1 << LED5) | (1 << LEDpwm);
    intertal_state = 1;
}
