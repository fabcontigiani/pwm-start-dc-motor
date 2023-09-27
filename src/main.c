// Pines
// PD0: Entrada, interruptor de encendido/apagado, inicio lento (pwm)
// PD1: Entrada, ciclo a través de posibles duraciones
// PD2: Entrada, interruptor de encendido/apagado, inicio instantáneo
// PC0: Salida, LED1
// PC1: Salida, LED2
// PC2: Salida, LED3
// PC3: Salida, LED4
// PC4: Salida, LED5
// PC5: Salida, LEDpwm

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
#define F_CPU 16000000L
#define T_OUTPUT 20     // ms, período señal de salida
#define N_STEPS 20      // número de incrementos del ciclo útil (1ms)
#define BOUNCE_DELAY 10 // ms

// Define constantes para diferentes porcentajes de N_STEPS
#define PCT25 (N_STEPS * 0.25)
#define PCT50 (N_STEPS * 0.50)
#define PCT75 (N_STEPS * 0.75)

// Define intervalos de tiempo para diferentes duraciones
#define T1 (5000 / T_OUTPUT) / N_STEPS
#define T2 (8000 / T_OUTPUT) / N_STEPS
#define T3 (11000 / T_OUTPUT) / N_STEPS
#define T4 (14000 / T_OUTPUT) / N_STEPS

// Define pines de LED para mayor legibilidad
#define LED1 PORTC0
#define LED2 PORTC1
#define LED3 PORTC2
#define LED4 PORTC3
#define LED5 PORTC4
#define LEDpwm PORTC5

// Prototipos de funciones
void turn_on();
void turn_off();
void toggle_led_1s(int);
void cycle_duration();
void pwm_start();

int selected_duration = T1; // Inicializar la duración seleccionada en T1
int internal_state = 0;     // Estado interno (0 = apagado; 1 = encendido)
volatile int flag = 0; // Una bandera utilizada para el manejo de interrupciones

// Rutina de servicio de interrupción para la interrupción externa 0 (PD2)
ISR(INT0_vect) {
    _delay_ms(5 * BOUNCE_DELAY);
    if (!(PIND & (1 << PORTD2))) // Comprobar si PD2 sigue en BAJO
        flag = 1; // Establecer la bandera para indicar la interrupción
}

int main(void) {
    // Inicializar PORTC como salida y establecer todos los pines en BAJO
    DDRC = 0xFF;
    PORTC = 0x00;

    // Configurar PD0, PD1, PD2 como entradas con pull-ups internos
    PORTD = (1 << PORTD0) | (1 << PORTD1) | (1 << PORTD2);

    // Configurar la configuración de interrupción externa
    EICRA = (1 << ISC01); // Flanco descendente activa la interrupción
    EIMSK = (1 << INT0);  // Habilitar PD2 como interrupción externa
    sei();                // Habilitar interrupciones globales

    int lastPD0State = 1;
    int PD0State;
    int lastPD1State = 1;
    int PD1State;

    while (1) {
        if (flag) {
            if (internal_state)
                turn_off(); // Apagar si está encendido
            else
                turn_on(); // Encender si está apagado
            flag = 0;      // Borrar la bandera
            _delay_ms(BOUNCE_DELAY);
            continue;
        }

        // Comprobar el estado de PD0 (interruptor de encendido pwm/apagado)
        PD0State = (PIND & (1 << PORTD0));
        if (PD0State != lastPD0State) // Cambio de estado del botón
        {
            if (!PD0State) // El botón pasó de ALTO a BAJO
            {
                _delay_ms(BOUNCE_DELAY);
                if (!PD0State)   // El botón todavía está en BAJO (evitar
                                 // rebotes)
                    pwm_start(); // Iniciar PWM
            } else
                _delay_ms(200); // Retraso para evitar cambios rápidos
        }
        lastPD0State = PD0State;

        // Comprobar el estado de PD1 (ciclo a través de posibles duraciones)
        PD1State = (PIND & (1 << PORTD1));
        if (PD1State != lastPD1State) // Cambio de estado del botón
        {
            if (!PD1State) // El botón pasó de ALTO a BAJO
            {
                _delay_ms(BOUNCE_DELAY);
                if (!PD1State) // El botón todavía está en BAJO (evitar
                               // rebotes)
                    cycle_duration(); // Ciclar a través de las duraciones
            } else
                _delay_ms(200);
        }
        lastPD1State = PD1State;
    }
    return 0;
}

// Función para encender todos los LEDs
void turn_on() {
    PORTC = 0xFF;
    internal_state = 1; // Actualizar el estado interno
    _delay_ms(200);
}

// Función para apagar todos los LEDs
void turn_off() {
    PORTC = 0x00;
    internal_state = 0; // Actualizar el estado interno
    _delay_ms(200);
}

// Función para alternar un LED durante 1 segundo
void toggle_led_1s(int led) {
    PORTC ^= (1 << led);
    for (int i = 0; i < 5; i++) {
        _delay_ms(200);
        if (flag)
            return; // Salir de la función si se disparó una interrupción
    }
    PORTC ^= (1 << led);
}

// Función para ciclar a través de diferentes duraciones
void cycle_duration() {
    switch (selected_duration) {
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

// Función para iniciar el PWM
void pwm_start() {
    if (internal_state) {
        turn_off(); // Si ya está encendido, apagar
        return;
    }
    internal_state = 1;                 // Actualizar el estado interno
    for (int i = 0; i < N_STEPS; i++) { // Incrementos del ciclo útil
        for (int j = 0; j < selected_duration; j++) { // Repetimos para alcanzar
                                                      // la duración deseada
            PORTC |= (1 << LEDpwm);
            for (int k = 0; k < i; k++) {
                _delay_ms(1);
            }
            PORTC &= ~(1 << LEDpwm);
            for (int k = 0; k <= (N_STEPS - i); k++) {
                _delay_ms(1);
            }
            if (flag)
                return; // Salir de la función si se disparó una interrupción
        }

        if (i < PCT25) // menos del 25% de progreso
            PORTC |= (1 << LED1);
        else if (i < PCT50) // menos del 50% de progreso
            PORTC |= (1 << LED2);
        else if (i < PCT75) // menos del 75% de progreso
            PORTC |= (1 << LED3);
        else // más del 75% de progreso
            PORTC |= (1 << LED4);
    }
    // Proceso de inicio de PWM terminado, encender LED5 y LEDpwm
    PORTC |= (1 << LED5) | (1 << LEDpwm);
}
