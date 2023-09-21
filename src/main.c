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
#define T_OUTPUT 20 // ms
#define N_STEPS 20

#define PCT25 N_STEPS * 0.25
#define PCT50 N_STEPS * 0.50
#define PCT75 N_STEPS * 0.75

#define T1 (5000 / T_OUTPUT) / N_STEPS
#define T2 (8000 / T_OUTPUT) / N_STEPS
#define T3 (11000 / T_OUTPUT) / N_STEPS
#define T4 (14000 / T_OUTPUT) / N_STEPS

#define LED1 PORTD7
#define LED2 PORTD6
#define LED3 PORTD5
#define LED4 PORTD4
#define LED5 PORTD3
#define LEDpwm PORTD0

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
        {
            _delay_ms(6);
            if (PINC & (1 << PORTC4))
                break;
            cycle_duration();
        }

        if (!(PINC & (1 << PORTC5)))
        {
            _delay_ms(6);
            if (PINC & (1 << PORTC5))
                break;
            pwm_start();
        }
        // TODO: Hardware interrupt based switch on/off
    }
    return 0;
}

void cycle_duration()
{
    switch (selected_duration)
    {
    case T1:
        selected_duration = T2;
        PORTD ^= (1 << LED3);
        _delay_ms(500);
        PORTD ^= (1 << LED3);
        break;
    case T2:
        selected_duration = T3;
        PORTD ^= (1 << LED4);
        _delay_ms(500);
        PORTD ^= (1 << LED4);
        break;
    case T3:
        selected_duration = T4;
        PORTD ^= (1 << LED5);
        _delay_ms(500);
        PORTD ^= (1 << LED5);
        break;
    case T4:
        selected_duration = T1;
        PORTD ^= (1 << LED2);
        _delay_ms(500);
        PORTD ^= (1 << LED2);
        break;
    }
}

void pwm_start()
{
    if (intertal_state)
    {
        PORTD = 0;
        intertal_state = 0;
        return;
    }
    intertal_state = 1;
    for (int i = 0; i < N_STEPS; i++)
    {
        for (int j = 0; j < selected_duration; j++)
        {
            PORTD |= (1 << LEDpwm);
            for (int k = 0; k < i; k++)
            {
                _delay_ms(1);
            }
            PORTD &= ~(1 << LEDpwm);
            for (int k = 0; k < (N_STEPS - i); k++)
            {
                _delay_ms(1);
            }
        }

        if (i < PCT25) // less than 25% progress
            PORTD |= (1 << LED1);
        else if (i < PCT50) // less than 50% progress
            PORTD |= (1 << LED2);
        else if (i < PCT75) // less than 75% progress
            PORTD |= (1 << LED3);
        else // more than 75% progress
            PORTD |= (1 << LED4);
    }
    // pwm start process finished
    // turn on LED5 and LEDpwm
    PORTD |= (1 << LED5) | (1 << LEDpwm);
}
