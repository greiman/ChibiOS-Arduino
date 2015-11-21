/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/**
 * @file    st_lld.c
 * @brief   PLATFORM ST subsystem low level driver source.
 *
 * @addtogroup ST
 * @{
 */

#include "hal.h"

#if (OSAL_ST_MODE != OSAL_ST_MODE_NONE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local types.                                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/
#if (OSAL_ST_MODE == OSAL_ST_MODE_PERIODIC) || defined(__DOXYGEN__)
/**
 * @brief   System Timer vector.
 * @details This interrupt is used for system tick in periodic mode.
 *
 * @isr
 */
 // WHG
int sysTickEnabled = 0;
#ifdef CORE_TEENSY
// Teensy 3.x

extern volatile uint32_t systick_millis_count;
void systick_isr() {
  systick_millis_count++;
  if (sysTickEnabled) {
    OSAL_IRQ_PROLOGUE();

    osalSysLockFromISR();
    osalOsTimerHandlerI();
    osalSysUnlockFromISR();

    OSAL_IRQ_EPILOGUE();
  }
}

#else  // CORE_TEENSY
// Due

int sysTickHook(void) { 
  if (sysTickEnabled) {

    OSAL_IRQ_PROLOGUE();

    osalSysLockFromISR();
    osalOsTimerHandlerI();
    osalSysUnlockFromISR();

    OSAL_IRQ_EPILOGUE();
  }
  return 0;
}
#endif  // CORE_TEENSY
#endif /* OSAL_ST_MODE == OSAL_ST_MODE_PERIODIC */

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level ST driver initialization.
 *
 * @notapi
 */
void st_lld_init(void) {
#if OSAL_ST_MODE == OSAL_ST_MODE_PERIODIC
  nvicSetSystemHandlerPriority(HANDLER_SYSTICK, WHG_ST_IRQ_PRIORITY);
#endif /* OSAL_ST_MODE == OSAL_ST_MODE_PERIODIC */  
}

#endif /* OSAL_ST_MODE != OSAL_ST_MODE_NONE */

/** @} */
