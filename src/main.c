/******************** Pins ****************************
 * PD0: Input, on/off switch, slow start
 * PD1: Input, cycle through possible durations
 * PD2: Input, on/off switch, external interrupt
 * PC0: Output, LED1
 * PC1: Output, LED2
 * PC2: Output, LED3
 * PC3: Output, LED4
 * PC4: Output, LED5
 * PC5: Output, LEDpwm
 *****************************************************/

#include <avr/io.h>
#include <avr/interrupt.h>
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

#define LED1 PORTC0
#define LED2 PORTC1
#define LED3 PORTC2
#define LED4 PORTC3
#define LED5 PORTC4
#define LEDpwm PORTC5

void turn_on();
void turn_off();
void toggle_led_1s(int);
void cycle_duration();
void pwm_start();

volatile int selected_duration = T1;
volatile int internal_state = 0; // 0 = off; 1 = on
volatile int flag = 0;

ISR(INT0_vect)
{
    flag = 1;
}

int main(void)
{
    // make the whole PORTC outputs and set LOW
    DDRC = 0xFF;
    PORTC = 0x00;

    // Pins PD0, PD1, PD2 will be receiving data from external switches
    // setup internal pull ups
    PORTD = (1 << PORTD0) | (1 << PORTD1) | (1 << PORTD2);

    EICRA = (1 << ISC01); // falling edge triggers interrupt
    EIMSK = (1 << INT0);  // enable PD2 as external interrupt
    sei();                // set external interrupt bit

    while (1)
    {
        if (!(PIND & (1 << PORTD0)))
        {
            _delay_ms(10);
            if (PIND & (1 << PORTD0))
                break;
            pwm_start();
        }

        if (!(PIND & (1 << PORTD1)))
        {
            _delay_ms(10);
            if (PIND & (1 << PORTD1))
                break;
            cycle_duration();
        }

        if (flag)
        {
            if (internal_state)
                turn_off();
            else
                turn_on();
            flag = 0;
        }
    }
    return 0;
}

void turn_on()
{
    PORTC = 0xFF;
    internal_state = 1;
    _delay_ms(200);
}

void turn_off()
{
    PORTC = 0x00;
    internal_state = 0;
    _delay_ms(200);
}

void toggle_led_1s(int led)
{
        PORTC ^= (1 << led);
    for (int i = 0; i < 5; i++)
    {
        _delay_ms(200);
        if (flag)
            return;
    }
        PORTC ^= (1 << led);
}

void cycle_duration()
{
    switch (selected_duration)
    {
    case T1:
        selected_duration = T2;
        toggle_led_1s(LED3);
        if (flag)
            return;
        break;
    case T2:
        selected_duration = T3;
        toggle_led_1s(LED4);
        if (flag)
            return;
        break;
    case T3:
        selected_duration = T4;
        toggle_led_1s(LED5);
        if (flag)
            return;
        break;
    case T4:
        selected_duration = T1;
        toggle_led_1s(LED2);
        if (flag)
            return;
        break;
    }
}

void pwm_start()
{
    if (internal_state)
    {
        turn_off();
        return;
    }
    internal_state = 1;
    for (int i = 0; i < N_STEPS; i++)
    {
        for (int j = 0; j < selected_duration; j++)
        {
            PORTC |= (1 << LEDpwm);
            for (int k = 0; k < i; k++)
            {
                _delay_ms(1);
            }
            PORTC &= ~(1 << LEDpwm);
            for (int k = 0; k <= (N_STEPS - i); k++)
            {
                _delay_ms(1);
            }
            if (flag)
                return;
        }

        if (i < PCT25) // less than 25% progress
            PORTC |= (1 << LED1);
        else if (i < PCT50) // less than 50% progress
            PORTC |= (1 << LED2);
        else if (i < PCT75) // less than 75% progress
            PORTC |= (1 << LED3);
        else // more than 75% progress
            PORTC |= (1 << LED4);
    }
    // pwm start process finished
    // turn on LED5 and LEDpwm
    PORTC |= (1 << LED5) | (1 << LEDpwm);
}
