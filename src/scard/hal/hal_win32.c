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

static HANDLE hComm = INVALID_HANDLE_VALUE;
static HANDLE hThread = INVALID_HANDLE_VALUE;
static HANDLE hReceiverEvent;

extern const char *CommDevice;

BOOL platform_init(void)
{
  hReceiverEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
  if (hReceiverEvent == INVALID_HANDLE_VALUE)
    return FALSE;

  return TRUE;
}

void sleep_ms(WORD ms)
{
  Sleep(ms);
}

void ccid_recv_wakeupFromISR(void)
{
  SetEvent(hReceiverEvent);
}

SCARD_RC ccid_recv_wait(WORD timeout_ms)
{
  DWORD rc = WaitForSingleObject(hReceiverEvent, timeout_ms);

  if (rc == WAIT_OBJECT_0)
  {
    ResetEvent(hReceiverEvent);
    return _SCARD_S_SUCCESS;
  }

  return _SCARD_E_TIMEOUT;
}

static BOOL serial_config(void)
{
  DCB dcb;
  COMMTIMEOUTS tmo;

  if (!GetCommState(hComm, &dcb))
    return FALSE;

  dcb.BaudRate = CBR_38400;

  dcb.fBinary = TRUE;
  dcb.fParity = FALSE;
  dcb.fOutxCtsFlow = FALSE;
  dcb.fOutxDsrFlow = FALSE;
  dcb.fDsrSensitivity = FALSE;
  dcb.fOutX = FALSE;
  dcb.fInX = FALSE;
  dcb.fNull = FALSE;
  dcb.ByteSize = 8;
  dcb.Parity = NOPARITY;
  dcb.StopBits = ONESTOPBIT;
  dcb.fRtsControl = RTS_CONTROL_ENABLE; /* New 1.73 : for compatibility with RL78 flash board */
  dcb.fDtrControl = DTR_CONTROL_ENABLE; /* New 1.73 : for compatibility with RL78 flash board */

  dcb.fAbortOnError = TRUE;
  dcb.fTXContinueOnXoff = TRUE;

  if (!SetCommState(hComm, &dcb))
    return FALSE;

  tmo.ReadIntervalTimeout         = 0;
  tmo.ReadTotalTimeoutConstant    = 10;
  tmo.ReadTotalTimeoutMultiplier  = 1;
  tmo.WriteTotalTimeoutConstant   = 0;
  tmo.WriteTotalTimeoutMultiplier = 0;
    
  if (!SetCommTimeouts(hComm, &tmo))
    return FALSE;

  return TRUE;
}

static DWORD WINAPI serial_receive_thread(void *unused)
{
  DWORD dwRead;
  BYTE bValue;

  for (;;)
  {
    if (!ReadFile(hComm, &bValue, 1, &dwRead, 0))
    {
      printf("ReadFile failed (%d)\n", GetLastError());
      return FALSE;
    }

    if (dwRead)
    {
      D(printf("+%02X", bValue));
      serial_recv_callback(bValue);
    }
  }
}

BOOL serial_send_byte(BYTE bValue)
{
  DWORD dwWritten;

  if (!WriteFile(hComm, &bValue, 1, &dwWritten, NULL) || (dwWritten == 0))
  {
    printf("WriteFile failed (%d)\n", GetLastError());
    return FALSE;
  }

  D(printf("-%02X", bValue));

  return TRUE;
}

BOOL serial_send_bytes(const BYTE *abValue, WORD wLength)
{
  DWORD dwWritten;
  D(DWORD i);

  if (!WriteFile(hComm, abValue, wLength, &dwWritten, NULL) || (dwWritten < wLength))
  {
    printf("WriteFile failed (%d)\n", GetLastError());
    return FALSE;
  }

  D(for (i=0; i<wLength; i++) printf("-%02X", abValue[i]));

  return TRUE;
}

BOOL serial_init(void)
{
  if (hComm == INVALID_HANDLE_VALUE)
  {
    hComm = CreateFile(CommDevice, GENERIC_READ | GENERIC_WRITE, 0,  // comm devices must be opened w/exclusive-access
                    NULL,         // no security attributes
                    OPEN_EXISTING,  // comm devices must use OPEN_EXISTING
                    0,            // not overlapped I/O
                    NULL          // hTemplate must be NULL for comm devices
      );

    if (hComm == INVALID_HANDLE_VALUE)
    {
      printf("Failed to open comm. port '%s' (%d)\n", CommDevice, GetLastError());
      return FALSE;
    }
  }

  if (!serial_config())
  {
    printf("Failed to configure the comm. port '%s' (%d)\n", CommDevice, GetLastError());
    return FALSE;
  }

  if (hThread == INVALID_HANDLE_VALUE)
  {
    DWORD dwId;

    hThread = CreateThread(NULL, 0, serial_receive_thread, NULL, 0, &dwId);
    if (hThread == INVALID_HANDLE_VALUE)
    {
      printf("Failed to create a thread to simulate the RX ISR (%d)\n", GetLastError());
      return FALSE;
    }
  }

  return TRUE;
}

