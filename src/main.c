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
enum leds
{
    LED1 = 8,   // PD7
    LED2 = 7,   // PD6
    LED3 = 6,   // PD5
    LED4 = 5,   // PD4
    LED5 = 4,   // PD3
    LEDpwm = 1  // PD0
};

int main(void)
{
    // make the LED pin an output for PORTB5
    DDRB = 1 << 5;

    while (1)
    {
        _delay_ms(500);

        // toggle the LED
        PORTB ^= 1 << 5;
    }

    return 0;
}