#include "stm32l432xx.h"
#include <stdint.h>

#define LED_PIN     3
#define BUZZER_PIN  5

typedef enum {
    MODE_OFF = 0,
    MODE_ON,
    MODE_BLINK,
    MODE_FAST,
    MODE_SLOW,
    MODE_SOS,
    MODE_ALARM
} SystemMode;

static volatile SystemMode current_mode = MODE_OFF;
static uint32_t tick_count = 0;
static uint32_t last_action_tick = 0;
static uint8_t led_state = 0;
static uint8_t sos_step = 0;

static void delay_tick(void)
{
    for (volatile uint32_t i = 0; i < 25000; i++);
    tick_count++;
}

static void led_on(void)
{
    GPIOB->ODR |= (1U << LED_PIN);
    led_state = 1;
}

static void led_off(void)
{
    GPIOB->ODR &= ~(1U << LED_PIN);
    led_state = 0;
}

static void buzzer_on(void)
{
    GPIOB->ODR |= (1U << BUZZER_PIN);
}

static void buzzer_off(void)
{
    GPIOB->ODR &= ~(1U << BUZZER_PIN);
}

static void outputs_off(void)
{
    led_off();
    buzzer_off();
}

static void uart2_send_char(char c)
{
    while (!(USART2->ISR & USART_ISR_TXE));
    USART2->TDR = (uint8_t)c;
}

static void uart2_send_string(const char *str)
{
    while (*str) {
        uart2_send_char(*str++);
    }
}

static int uart2_available(void)
{
    return (USART2->ISR & USART_ISR_RXNE) != 0;
}

static char uart2_read_char(void)
{
    return (char)USART2->RDR;
}

static void gpio_init(void)
{
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;

    GPIOB->MODER &= ~(3U << (LED_PIN * 2));
    GPIOB->MODER |=  (1U << (LED_PIN * 2));

    GPIOB->MODER &= ~(3U << (BUZZER_PIN * 2));
    GPIOB->MODER |=  (1U << (BUZZER_PIN * 2));
}

static void uart2_init(void)
{
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
    RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN;

    /* PA2 = USART2 TX */
    GPIOA->MODER &= ~(3U << (2 * 2));
    GPIOA->MODER |=  (2U << (2 * 2));
    GPIOA->AFR[0] &= ~(0xFU << (4 * 2));
    GPIOA->AFR[0] |=  (7U << (4 * 2));

    /* PA15 = USART2 RX on STM32L432KC Nucleo virtual COM port */
    GPIOA->MODER &= ~(3U << (2 * 15));
    GPIOA->MODER |=  (2U << (2 * 15));
    GPIOA->AFR[1] &= ~(0xFU << (4 * (15 - 8)));
    GPIOA->AFR[1] |=  (3U << (4 * (15 - 8)));

    USART2->CR1 = 0;
    USART2->BRR = 4000000U / 9600U;
    USART2->CR1 |= USART_CR1_TE;
    USART2->CR1 |= USART_CR1_RE;
    USART2->CR1 |= USART_CR1_UE;
}

static void print_menu(void)
{
    uart2_send_string("\r\n========================================\r\n");
    uart2_send_string(" PC Command Controlled LED Alarm System\r\n");
    uart2_send_string("========================================\r\n");
    uart2_send_string("1 = LED ON\r\n");
    uart2_send_string("2 = LED OFF\r\n");
    uart2_send_string("3 = NORMAL BLINK\r\n");
    uart2_send_string("4 = FAST BLINK\r\n");
    uart2_send_string("5 = SLOW BLINK\r\n");
    uart2_send_string("6 = SOS PATTERN\r\n");
    uart2_send_string("7 = ALARM MODE\r\n");
    uart2_send_string("8 = STATUS\r\n");
    uart2_send_string("----------------------------------------\r\n");
    uart2_send_string("Type 1-8 anytime. No reset required.\r\n\r\n");
}

static void set_mode(SystemMode new_mode, const char *message)
{
    current_mode = new_mode;
    outputs_off();
    last_action_tick = tick_count;
    sos_step = 0;

    uart2_send_string("\r\nCOMMAND RECEIVED: ");
    uart2_send_string(message);
    uart2_send_string("\r\n");
}

static const char *mode_name(void)
{
    switch (current_mode) {
    case MODE_OFF:
        return "LED OFF";
    case MODE_ON:
        return "LED ON";
    case MODE_BLINK:
        return "NORMAL BLINK";
    case MODE_FAST:
        return "FAST BLINK";
    case MODE_SLOW:
        return "SLOW BLINK";
    case MODE_SOS:
        return "SOS PATTERN";
    case MODE_ALARM:
        return "ALARM MODE";
    default:
        return "UNKNOWN";
    }
}

static void print_status(void)
{
    uart2_send_string("\r\nSTATUS: Current mode is ");
    uart2_send_string(mode_name());
    uart2_send_string("\r\n");
}

static void check_command(void)
{
    while (uart2_available()) {
        char command = uart2_read_char();

        if (command == '\r' || command == '\n') {
            continue;
        }

        if (command == '1') {
            set_mode(MODE_ON, "1 - LED ON");
        } else if (command == '2') {
            set_mode(MODE_OFF, "2 - LED OFF");
        } else if (command == '3') {
            set_mode(MODE_BLINK, "3 - NORMAL BLINK");
        } else if (command == '4') {
            set_mode(MODE_FAST, "4 - FAST BLINK");
        } else if (command == '5') {
            set_mode(MODE_SLOW, "5 - SLOW BLINK");
        } else if (command == '6') {
            set_mode(MODE_SOS, "6 - SOS PATTERN");
        } else if (command == '7') {
            set_mode(MODE_ALARM, "7 - ALARM MODE");
        } else if (command == '8') {
            print_status();
        } else if (command == 'm' || command == 'M') {
            print_menu();
        } else {
            uart2_send_string("\r\nInvalid command. Use 1-8 or M for menu.\r\n");
        }
    }
}

static void toggle_led_after(uint32_t interval_ticks)
{
    if ((tick_count - last_action_tick) >= interval_ticks) {
        if (led_state) {
            led_off();
        } else {
            led_on();
        }
        last_action_tick = tick_count;
    }
}

static void run_current_mode(void)
{
    /* SOS in Morse code: dot dot dot, dash dash dash, dot dot dot. */
    static const uint32_t sos_intervals[] = {
        4, 4, 4, 4, 4, 12,
        12, 4, 12, 4, 12, 12,
        4, 4, 4, 4, 4, 36
    };

    switch (current_mode) {
    case MODE_OFF:
        outputs_off();
        break;

    case MODE_ON:
        led_on();
        buzzer_off();
        break;

    case MODE_BLINK:
        buzzer_off();
        toggle_led_after(12);
        break;

    case MODE_FAST:
        buzzer_off();
        toggle_led_after(4);
        break;

    case MODE_SLOW:
        buzzer_off();
        toggle_led_after(28);
        break;

    case MODE_SOS:
        if ((tick_count - last_action_tick) >= sos_intervals[sos_step]) {
            if (sos_step % 2 == 0) {
                led_on();
                buzzer_on();
            } else {
                led_off();
                buzzer_off();
            }

            sos_step++;
            if (sos_step >= (sizeof(sos_intervals) / sizeof(sos_intervals[0]))) {
                sos_step = 0;
            }

            last_action_tick = tick_count;
        }
        break;

    case MODE_ALARM:
        if ((tick_count - last_action_tick) >= 8) {
            if (led_state) {
                led_off();
                buzzer_off();
            } else {
                led_on();
                buzzer_on();
            }
            last_action_tick = tick_count;
        }
        break;

    default:
        outputs_off();
        current_mode = MODE_OFF;
        break;
    }
}

int main(void)
{
    gpio_init();
    uart2_init();
    outputs_off();
    print_menu();

    while (1) {
        check_command();
        run_current_mode();
        delay_tick();
    }
}