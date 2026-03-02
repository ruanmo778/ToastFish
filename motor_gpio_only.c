/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "fsl_debug_console.h"
#include "board.h"
#include "app.h"

#include "fsl_gpio.h"
#include "fsl_clock.h"
#include "fsl_common.h"

// ===================== Direction pins (GPIO) =====================
// GPIO2_10 routed to P2_10 (J8[15])
// GPIO2_8  routed to P2_8  (J8[13])
#define IN1_GPIO   GPIO2
#define IN1_PIN    10U

#define IN2_GPIO   GPIO2
#define IN2_PIN    8U

// ===================== Motor direction control =====================
typedef enum {
    MOTOR_COAST = 0,
    MOTOR_BRAKE,
    MOTOR_FWD,
    MOTOR_REV
} motor_dir_t;

static void Motor_GPIO_Init(void)
{
    gpio_pin_config_t cfg = {
        .pinDirection = kGPIO_DigitalOutput,
        .outputLogic  = 0U
    };

    GPIO_PinInit(IN1_GPIO, IN1_PIN, &cfg);
    GPIO_PinInit(IN2_GPIO, IN2_PIN, &cfg);

    // safest default: coast
    GPIO_PinWrite(IN1_GPIO, IN1_PIN, 0U);
    GPIO_PinWrite(IN2_GPIO, IN2_PIN, 0U);
}

static void Motor_SetDir(motor_dir_t dir)
{
    switch (dir)
    {
        case MOTOR_FWD:   // IN1=1 IN2=0
            GPIO_PinWrite(IN1_GPIO, IN1_PIN, 1U);
            GPIO_PinWrite(IN2_GPIO, IN2_PIN, 0U);
            break;

        case MOTOR_REV:   // IN1=0 IN2=1
            GPIO_PinWrite(IN1_GPIO, IN1_PIN, 0U);
            GPIO_PinWrite(IN2_GPIO, IN2_PIN, 1U);
            break;

        case MOTOR_BRAKE: // IN1=1 IN2=1
            GPIO_PinWrite(IN1_GPIO, IN1_PIN, 1U);
            GPIO_PinWrite(IN2_GPIO, IN2_PIN, 1U);
            break;

        default:          // COAST: IN1=0 IN2=0
            GPIO_PinWrite(IN1_GPIO, IN1_PIN, 0U);
            GPIO_PinWrite(IN2_GPIO, IN2_PIN, 0U);
            break;
    }
}

static void Motor_SafeChangeDir(motor_dir_t dir)
{
    // stop motor before switching direction
    Motor_SetDir(MOTOR_COAST);
    SDK_DelayAtLeastUs(50000U, CLOCK_GetFreq(kCLOCK_CoreSysClk)); // 50ms stop

    Motor_SetDir(dir);
    SDK_DelayAtLeastUs(20000U, CLOCK_GetFreq(kCLOCK_CoreSysClk)); // 20ms settle
}

// ===================== main =====================
int main(void)
{
    BOARD_InitHardware();

    Motor_GPIO_Init();

    while (1)
    {
        // forward
        Motor_SafeChangeDir(MOTOR_FWD);
        SDK_DelayAtLeastUs(1500000U, CLOCK_GetFreq(kCLOCK_CoreSysClk));

        // reverse
        Motor_SafeChangeDir(MOTOR_REV);
        SDK_DelayAtLeastUs(1500000U, CLOCK_GetFreq(kCLOCK_CoreSysClk));

        // brake
        Motor_SetDir(MOTOR_BRAKE);
        SDK_DelayAtLeastUs(500000U, CLOCK_GetFreq(kCLOCK_CoreSysClk));

        // coast
        Motor_SetDir(MOTOR_COAST);
        SDK_DelayAtLeastUs(500000U, CLOCK_GetFreq(kCLOCK_CoreSysClk));
    }
}
