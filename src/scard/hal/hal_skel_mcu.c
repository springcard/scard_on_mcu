/**h* SCARD_On_MCU/hal_skeleton
 *
 * NAME
 *   hal_skeleton -- Function that must be implemented by yourself in your MCU!!!
 *
 * COPYRIGHT
 *   Copyright (c) 2015 SPRINGCARD SAS, FRANCE - www.springcard.com
 *
 * NOTES
 *   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
 *   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
 *   TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 *   PARTICULAR PURPOSE.
 *
 * AUTHOR
 *   Johann.D / SpringCard
 *
 * DESCRIPTION
 *   The HAL must provide 
 *
 *   1. Serial communication with the reader.
 *   The serial_init function shall configure the UART.
 *   The serial_send_byte and serial_send_bytes are used to transmit (TX) from the MCU to the reader.
 *   Receiving (RX) must be done in an ISR. The ISR shall call serial_recv_callback for every byte
 *   that comes from the reader to the MCU.
 *
 *   2. Wait for the end of the communication.
 *   ccid_recv_wait will be called in the context of the main task, and shall block until the ISR
 *   calls ccid_recv_wakeupFromISR (this is done by serial_recv_callback).
 *   If you don't have an OS, just use a volatile BOOL to do so.
 *   Under a multitasking system, you must use a 'Event' or 'Semaphore' object to implement this correctly.
 *   Under FreeRTOS you will typically use a Binary Semaphore as follow:
 *   - ccid_recv_wait blocks using xSemaphoreTake
 *   - ccid_recv_wakeupFromISR calls xSemaphoreGiveFromISR
 *   - you may create the semaphore in the platform_init function.
 *
 *   3. Provide a mean to wait for a specified time, from 1 to 10000ms. This is the role of sleep_ms.
 *
 **/
/*

 This code is Copyright (c) 2015 SPRINGCARD SAS, FRANCE - www.springcard.com

 Redistribution and use in source (source code) and binary (object code)
  forms, with or without modification, are permitted provided that the
  following conditions are met :
  1. Redistributed source code or object code must be used only in conjunction
     with a genuine SpringCard product,
  2. Redistributed source code must retain the above copyright notice, this
     list of conditions and the disclaimer below,
  3. Redistributed object code must reproduce the above copyright notice,
     this list of conditions and the disclaimer below in the documentation
     and/or other materials provided with the distribution,
  4. The name of SpringCard may not be used to endorse or promote products
     derived from this software or in any other form without specific prior
     written permission from SpringCard,
  5. Redistribution of any modified code must be labeled 
    "Code derived from original SpringCard copyrighted source code".

  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
  PARTICULAR PURPOSE.
  SPRINGCARD SHALL NOT BE LIABLE FOR INFRINGEMENTS OF THIRD PARTIES RIGHTS
  BASED ON THIS SOFTWARE. ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
  PARTICULAR PURPOSE ARE DISCLAIMED. SPRINGCARD DOES NOT WARRANT THAT THE
  FUNCTIONS CONTAINED IN THIS SOFTWARE WILL MEET THE USER'S REQUIREMENTS OR
  THAT THE OPERATION OF IT WILL BE UNINTERRUPTED OR ERROR-FREE. IN NO EVENT,
  UNLESS REQUIRED BY APPLICABLE LAW, SHALL SPRINGCARD BE LIABLE FOR ANY DIRECT,
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. ALSO, SPRINGCARD IS UNDER
  NO OBLIGATION TO MAINTAIN, CORRECT, UPDATE, CHANGE, MODIFY, OR OTHERWISE
  SUPPORT THIS SOFTWARE.
*/ 
#include "../../project.h"
#include "../ccid/ccid_exchange.h"
#include "../ccid/ccid_platform.h"

static volatile BOOL fRecvDone;

/**f* hal_skeleton/ccid_recv_wakeupFromISR
 *
 * NAME
 *   ccid_recv_wakeupFromISR
 *
 * DESCRIPTION
 *   This function is invoked from the context of serial_recv_callback when a complete
 *   buffer has been received. Its role is to wake the main task up. The main task
 *   is waiting using ccid_recv_wait.
 *
 * SYNOPSIS
 *   void ccid_recv_wakeupFromISR(void)
 *
 **/
void ccid_recv_wakeupFromISR(void)
{
  fRecvDone = TRUE;
}

/**f* hal_skeleton/ccid_recv_wait
 *
 * NAME
 *   ccid_recv_wait
 *
 * DESCRIPTION
 *   This function is invoked in the main context to wait for a response from the reader.
 *
 * SYNOPSIS
 *   SCARD_RC ccid_recv_wait(WORD timeout_ms)
 *
 * INPUTS
 *   BYTE timeout_ms          : the time to wait, in milliseconds
 *
 * RETURNS
 *   _SCARD_S_SUCCESS         : success, reception terminated
 *   _SCARD_E_TIMEOUT         : failed
 *
 * REMARKS
 *   The implementation of the timeout doesn't have to be very precise, provided that
 *   it doesn't wait shorter than expected (but a longer wait time is not an issue).
 *
 **/
SCARD_RC ccid_recv_wait(WORD timeout_ms)
{
  /* TODO : improve this by using a 'Event' or a 'Semaphore' object provided by the operating system. */
  while (timeout_ms)
  {
    if (fRecvDone)
      break;
    sleep_ms(1);
    timeout_ms--;
  }

  if (fRecvDone)
    return _SCARD_S_SUCCESS;
  return _SCARD_E_TIMEOUT;
}

/**f* hal_skeleton/serial_recv_callback
 *
 * NAME
 *   serial_recv_callback
 *
 * DESCRIPTION
 *   This function must be invoked by the UART's RX ISR for every byte coming from the reader to the MCU
 *
 * SYNOPSIS
 *   void serial_recv_callback(BYTE bValue)
 *
 * INPUTS
 *   BYTE bValue              : RX byte
 *
 * REMARKS
 *   The function serial_recv_callback must perform some real-time analysis.
 *   Please ensure that your MCU is fast enough to run serial_recv_callback totally before the next
 *   byte arrives (at 38400bps, the function must take at most 260µs).
 *
 **/
extern void serial_recv_callback(BYTE bValue);

/**f* hal_skeleton/serial_send_byte
 *
 * NAME
 *   serial_send_byte
 *
 * DESCRIPTION
 *   This function must be invoked by the UART's RX ISR for every byte coming from the reader to the MCU
 *
 * SYNOPSIS
 *   BOOL serial_send_byte(BYTE bValue)
 *
 * INPUTS
 *   BYTE bValue              : TX byte
 *
 * RETURNS
 *   TRUE                     : success
 *   FALSE                    : failed
 *
 * REMARKS
 *   The function may return immediately or may wait for the byte to be actually transmitted;
 *   whatever the implementation, the function must make sure that the caller doesn't overrun the UART.
 *   The simplest implementation is to block until the UART's TX register is empty when entering this
 *   function.
 *
 **/
BOOL serial_send_byte(BYTE bValue)
{
  /* TODO : send one byte. */
}

/**f* hal_skeleton/serial_send_bytes
 *
 * NAME
 *   serial_send_bytes
 *
 * DESCRIPTION
 *   This is a loop around serial_send_byte
 *
 * SYNOPSIS
 *   BOOL serial_send_bytes(const BYTE *abValue, WORD wLength)
 *
 * INPUTS
 *   const BYTE *abValue      : TX buffer
 *   WORD wLength             : size of buffer
 *
 * RETURNS
 *   TRUE                     : success
 *   FALSE                    : failed
 *
 **/
BOOL serial_send_bytes(const BYTE *abValue, WORD wLength)
{
  WORD i;
  for (i=0; i<wLength; i++)
    if (!serial_send_byte(abValue[i]))
      return FALSE;
  return TRUE;
}

/**f* hal_skeleton/serial_init
 *
 * NAME
 *   serial_init
 *
 * DESCRIPTION
 *   Configure the UART to communicate with the reader
 *
 * SYNOPSIS
 *   BOOL serial_init(void)
 *
 * RETURNS
 *   TRUE                     : success
 *   FALSE                    : failed
 *
 * REMARKS
 *   Default reader configuration is
 *   - baudrate = 38400bps
 *   - 8 data bits
 *   - 1 stop bit
 *   - no parity
 *   - no flow control
 *   Don't forget to bind the UART's RX ISR to serial_recv_callback
 *
 **/
BOOL serial_init(void)
{
  /* TODO : configure the UART (38400bps, 8 data bits, 1 stop bit, no parity, no flow control). */
}

/**f* hal_skeleton/platform_init
 *
 * NAME
 *   platform_init
 *
 * DESCRIPTION
 *   Implement in this function the initialisation of any global variable.
 *
 * SYNOPSIS
 *   BOOL platform_init(void)
 *
 * RETURNS
 *   TRUE                     : success
 *   FALSE                    : failed
 *
 **/
BOOL platform_init(void)
{
  /* TODO : perform necessary initialisations here */
  fRecvDone = FALSE;
  return TRUE;
}

/**f* hal_skeleton/sleep_ms
 *
 * NAME
 *   sleep_ms
 *
 * DESCRIPTION
 *   Suspend the execution for a specified amount of time
 *
 * SYNOPSIS
 *   void sleep_ms(WORD ms)
 *
 * REMARKS
 *   The implementation of the doesn't have to be very precise, provided that
 *   it doesn't wait shorter than expected (but a longer wait time is not an issue).
 *
 **/
void sleep_ms(WORD ms)
{
  /* TODO : wait for the specified amount of ms */
}

