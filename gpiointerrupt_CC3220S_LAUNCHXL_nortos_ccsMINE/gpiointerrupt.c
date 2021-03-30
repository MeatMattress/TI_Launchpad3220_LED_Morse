/*
 * Copyright (c) 2015-2020, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to ENDorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== gpiointerrupt.c ========
 */
#include <stdint.h>
#include <stddef.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>

#include <stdio.h>

/* Timer header file */
#include <ti/drivers/Timer.h>

/* Driver configuration */
#include "ti_drivers_config.h"

/* State machine declarations */
enum SM_States {SM_START, SM_PLAYING_SOS, SM_PLAYING_OK, SM_END}; // State-machine states
enum SM_Chars {SM_DIT, SM_DAH, SM_SPACE, SM_SPACECHAR}; // State-machine characters that words are made of
enum SM_Modes {SM_SOS, SM_OK}; // Which message will be played
enum SM_States SM_State; // SM handle
enum SM_Modes SM_Mode; // SM Mode handle

enum SM_Chars SOS[17]={SM_DIT, SM_SPACE, SM_DIT, SM_SPACE, SM_DIT, SM_SPACECHAR,
                     SM_DAH, SM_SPACE, SM_DAH, SM_SPACE, SM_DAH, SM_SPACECHAR,
                     SM_DIT, SM_SPACE, SM_DIT, SM_SPACE, SM_DIT};
enum SM_Chars OK[11] = {SM_DAH, SM_SPACE, SM_DAH, SM_SPACE, SM_DAH, SM_SPACECHAR,
                      SM_DAH, SM_SPACE, SM_DIT, SM_SPACE, SM_DAH};

/* Quick note to professor:
 * I tried creating the words in a more clever and scalable way, storing characters as Dits and Dahs,
 * and creating a 2-dimensional array from those characters to form a word.
 * However, I kept getting horrible issues due to the way enumerations work, which formed gibberish.
 * Working with indexes inside enumerated arrays in C is terrible, and thus I decided to hard-code
 * these words to save myself some headaches for another day.
 * */

int holdState = 0; // How many periods to continue playing the current state

void gpioButtonFxn(uint_least8_t index) // Button-press callback function
{
    /* Toggle message mode */
    SM_Mode = SM_Mode == SM_SOS ? SM_OK : SM_SOS;
}

int i = 0;

void tickMessage(enum SM_Chars message[], int length){ // This is basically a state-based for-loop to iterate over the message
    enum SM_Chars currOutput;
    if (i > length - 1) { // Sequence ended
        SM_State = SM_END;
        i = 0;
        GPIO_write(CONFIG_GPIO_LED_1, CONFIG_GPIO_LED_OFF);
        GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_OFF);
        holdState = 7;
        return;
    }
    else {
        currOutput = message[i];
        i++;
    }

    switch(currOutput) {
    case SM_DIT:
        GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_ON);
        holdState = 1; // Dit = RED 500ms ON
        break;
    case SM_DAH:
        GPIO_write(CONFIG_GPIO_LED_1, CONFIG_GPIO_LED_ON);
        holdState = 3; // Dah = GREEN 1500ms ON
        break;
    case SM_SPACE:
        GPIO_write(CONFIG_GPIO_LED_1, CONFIG_GPIO_LED_OFF);
        GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_OFF);
        holdState = 1; // Space = RED && GREEN 500ms OFF
        break;
    case SM_SPACECHAR:
        GPIO_write(CONFIG_GPIO_LED_1, CONFIG_GPIO_LED_OFF);
        GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_OFF);
        holdState = 3; // Space between characters = RED && GREEN 1500ms OFF
        break;
    default:
        break;
    }



}


/* SM Ticker */
void SM_Tick() {
    if (holdState > 0) { // State is holding over n periods
        holdState--;
        return;
    }
    else {
        switch(SM_State) {
            case SM_START:
                switch(SM_Mode){ // Get play mode and tick the mode's message
                    case SM_SOS:
                        SM_State = SM_PLAYING_SOS;
                        tickMessage(SOS, sizeof(SOS));
                        break;

                    case SM_OK:
                        SM_State = SM_PLAYING_OK;
                        tickMessage(OK, sizeof(OK));
                        break;
                }
                break;

            case SM_PLAYING_SOS:
                tickMessage(SOS, sizeof(SOS)); // continue playing SOS
                break;

            case SM_PLAYING_OK:
                tickMessage(OK, sizeof(OK)); // continue playing OK
                break;

            case SM_END:
                SM_State = SM_START; // Restart the sequence
                break;
        }
    }
}



/* Timer callback */
void timerCallback(Timer_Handle myHandle, int_fast16_t status)
{
    SM_Tick(); // Tick the state machine
}






/* Initiate the timer */
void initTimer(void)
{
 Timer_Handle timer0;
 Timer_Params params;
 Timer_init();
 Timer_Params_init(&params);
 params.period = 500000;
 params.periodUnits = Timer_PERIOD_US;
 params.timerMode = Timer_CONTINUOUS_CALLBACK;
 params.timerCallback = timerCallback;
 timer0 = Timer_open(CONFIG_TIMER_0, &params);
 if (timer0 == NULL) {
 /* Failed to initialized timer */
 while (1) {}
 }
 if (Timer_start(timer0) == Timer_STATUS_ERROR) {
  /* Failed to start timer */
  while (1) {}
  }

}

/*
 *  ======== mainThread ========
 */
void *mainThread(void *arg0)
{
    /* Call driver init functions */
    GPIO_init();

    /* Configure the LED and button pins */
    GPIO_setConfig(CONFIG_GPIO_LED_0, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(CONFIG_GPIO_LED_1, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(CONFIG_GPIO_BUTTON_0, GPIO_CFG_IN_PU | GPIO_CFG_IN_INT_FALLING);


    /* Install Button callback */
    GPIO_setCallback(CONFIG_GPIO_BUTTON_0, gpioButtonFxn);

    /* Enable interrupts */
    GPIO_enableInt(CONFIG_GPIO_BUTTON_0);

    /*
     *  If more than one input pin is available for your device, interrupts
     *  will be enabled on CONFIG_GPIO_BUTTON1.
     */
    if (CONFIG_GPIO_BUTTON_0 != CONFIG_GPIO_BUTTON_1) {
        /* Configure BUTTON1 pin */
        GPIO_setConfig(CONFIG_GPIO_BUTTON_1, GPIO_CFG_IN_PU | GPIO_CFG_IN_INT_FALLING);

        /* Install Button callback */
        GPIO_setCallback(CONFIG_GPIO_BUTTON_1, gpioButtonFxn);
        GPIO_enableInt(CONFIG_GPIO_BUTTON_1);
    }

    initTimer(); // Begin sequence
    return NULL;
}
