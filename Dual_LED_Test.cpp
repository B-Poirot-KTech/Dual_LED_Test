#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include <hardware/pio.h>
#include "hardware/clocks.h"
#include "debounce.pio.h"


//CONSTANTS & GLOBALS
const uint BUTTON0_SOCD_PIN = 14; //output for SOCD cleaned input a, driving LED during testing
const uint BUTTON0 = 15; //input a
const uint PRE_SOCD_LED0 = 5; //used as outputs from PIO sm - debounced button presses

const uint BUTTON1_SOCD_PIN = 17; //output for SOCD cleaned input b, driving LED during testing
const uint BUTTON1 = 16; // input b
const uint PRE_SOCD_LED1 = 28; //used as outputs from PIO sm - debounced button presses

uint PRE_SOCD_GPIOS[2] = {PRE_SOCD_LED0, PRE_SOCD_LED1};

PIO pio;
const float pio_freq = 2000;
uint sm0;
uint sm1;
uint offset;
float clock_div = (float)clock_get_hz(clk_sys) / pio_freq;


//setup for debounce PIO program
void debounce_pio_setup(bool clock_div_enable = false, float clock_div = 1)
{
    //pull buttons high
    gpio_pull_up(BUTTON0);
    gpio_pull_up(BUTTON1);

    //grab state machines from PIO_0
    pio = pio0;
    sm0 = pio_claim_unused_sm(pio, true);
    sm1 = pio_claim_unused_sm(pio, true);

    //load program into pio 
    offset = pio_add_program(pio, &debounce_program);

    //initialize state machines
     debounce_program_init(pio, sm0, offset, PRE_SOCD_LED0, BUTTON0, clock_div);
     debounce_program_init(pio, sm1, offset, PRE_SOCD_LED1, BUTTON1, clock_div);

    //start state machines
     pio_sm_set_enabled(pio, sm0, true);
     pio_sm_set_enabled(pio, sm1, true);
    return;
};

//processor GPIO setup, for SOCD cleaned outputs
void gpio_setup()
{
    gpio_init(BUTTON0_SOCD_PIN);
    gpio_set_dir(BUTTON0_SOCD_PIN, GPIO_OUT);

    gpio_init(BUTTON1_SOCD_PIN);
    gpio_set_dir(BUTTON1_SOCD_PIN, GPIO_OUT);

    stdio_init_all();
};

//SOCD impleneted: A+B = Neutral
void socd_clean(uint gpio_a_debounced, uint gpio_b_debounced, uint output_a_gpio, uint output_b_gpio){

    //put SOCD cleaned A on A output, A = (A & !B)
    gpio_put(
        output_a_gpio,
        (gpio_get(gpio_a_debounced) && !(gpio_get(gpio_b_debounced)))
    );
    //put SOCD cleaned B on B output, B = (A & !B)
    gpio_put(
        output_b_gpio,
        (gpio_get(gpio_b_debounced) && !(gpio_get(gpio_a_debounced)))
    );
    return; 
 };


int main(){
    //array for holding current SOCD status, for passing to socd_clean(uint . . . )
    gpio_setup();
    debounce_pio_setup();

    bool pre_socd_input_satuses[2] = {
        gpio_get(PRE_SOCD_GPIOS[0]),
        gpio_get(PRE_SOCD_GPIOS[1])
    };
    
    while (true) {
        pre_socd_input_satuses[0] = gpio_get(PRE_SOCD_GPIOS[0]);
        pre_socd_input_satuses[1] = gpio_get(PRE_SOCD_GPIOS[1]);
        socd_clean(PRE_SOCD_LED0, PRE_SOCD_LED1, BUTTON0_SOCD_PIN, BUTTON1_SOCD_PIN);
    };
    return 0;
};

