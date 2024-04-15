#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include <hardware/pio.h>
#include "hardware/clocks.h"
#include "debounce.pio.h"

const uint BUTTON0_SOCD_PIN = 14; //output for SOCD cleaned input a, driving LED during testing
const uint BUTTON0 = 15; //input a, represents a direction on 1 axis
const uint PRE_SOCD_LED0 = 5; //used as outputs from PIO sm - debounced button presses

const uint BUTTON1_SOCD_PIN = 17; //output for SOCD cleaned input b, driving LED during testing
const uint BUTTON1 = 16; // input b, represents opposite direction from a, on the same, single axis
const uint PRE_SOCD_LED1 = 28; //used as outputs from PIO sm - debounced button presses

PIO pio = pio0;
const float pio_freq = 2000;
uint sm0;
uint sm1;
uint offset;
float clock_div = (float)clock_get_hz(clk_sys) / pio_freq;

//setup function for PIO debouncing program
void debounce_pio_setup(bool clock_div_enable = false, float clock_div = 1)
{
    //pull BUTTON inputs for state machines up
    gpio_pull_up(BUTTON0);
    gpio_pull_up(BUTTON1);

    //claim state machines
    sm0 = pio_claim_unused_sm(pio, true);
    sm1 = pio_claim_unused_sm(pio, true);
    offset = pio_add_program(pio, &debounce_program);

    //initialize state machines
     debounce_program_init(pio, sm0, offset, PRE_SOCD_LED0, BUTTON0, clock_div);
     debounce_program_init(pio, sm1, offset, PRE_SOCD_LED1, BUTTON1, clock_div);

    //start state machines
     pio_sm_set_enabled(pio, sm0, true);
     pio_sm_set_enabled(pio, sm1, true);
    return;
};


void gpio_setup()
{
    //initialize gpio pins
    gpio_init(BUTTON0_SOCD_PIN);
    gpio_init(BUTTON1_SOCD_PIN);

    //set pins to outputs
    gpio_set_dir(BUTTON0_SOCD_PIN, GPIO_OUT);
    gpio_set_dir(BUTTON1_SOCD_PIN, GPIO_OUT);

    stdio_init_all();
};

void socd_clean(uint gpio_a_debounced_input, uint gpio_b_debounced_input, uint socd_output_a, uint socd_output_b){

    uint32_t mask = gpio_get_all(); //sample the pins
    bool gpio_a_debounced_val = (mask>>gpio_a_debounced_input)%2; 
    bool gpio_b_debounced_val = (mask>>gpio_b_debounced_input)%2;

    //if both buttons pressed, drom both low
    if(gpio_a_debounced_val && gpio_b_debounced_val) 
    {
        gpio_put_masked( (1 << socd_output_a) | (1 << socd_output_b), false );
    }
    //otherwise, set corresponding pin high
    else{
        gpio_put(socd_output_a, gpio_a_debounced_val);
        gpio_put(socd_output_b,gpio_b_debounced_val);
    };
    return; 
 };


int main(){

    gpio_setup();
    debounce_pio_setup();
    while (true) {
        socd_clean(PRE_SOCD_LED0, PRE_SOCD_LED1, BUTTON0_SOCD_PIN, BUTTON1_SOCD_PIN);
    };
    return 0;
};

