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
#include "ccid_exchange.h"
#include "ccid_platform.h"

typedef struct
{
  BYTE eStatus;
  BYTE bEndpoint;
  WORD wLength;
  WORD wOffset;
  BYTE abBuffer[CCID_HEADER_LENGTH + CCID_MAX_PAYLOAD_LENGTH];
  BYTE bChecksum;
} CCID_RECEIVER_ST;

static CCID_RECEIVER_ST ccid_receiver;

#define STATUS_ERROR (BYTE) -1
#define STATUS_IDLE 0
#define STATUS_RECV_ENDPOINT 1
#define STATUS_RECV_HEADER 2
#define STATUS_RECV_PAYLOAD 3
#define STATUS_RECV_CHECKSUM 4
#define STATUS_READY 5

SCARD_RC ccid_recv_status(void)
{
  if (ccid_receiver.eStatus == STATUS_READY)
    return _SCARD_S_SUCCESS;
  if (ccid_receiver.eStatus == STATUS_ERROR)
    return _SCARD_E_READER_UNAVAILABLE;
  printf("Status=%02X\n", ccid_receiver.eStatus);
  return _SCARD_E_NOT_READY;
}

void ccid_recv_cleanup(void)
{
  memset(&ccid_receiver, 0, sizeof(ccid_receiver));
}

void serial_recv_callback(BYTE bValue)
{
  switch (ccid_receiver.eStatus)
  {
    case STATUS_IDLE :
      if (bValue == START_BYTE)
      {
        ccid_recv_cleanup();
        ccid_receiver.eStatus = STATUS_RECV_ENDPOINT;
      } else
      {
        ccid_receiver.eStatus = STATUS_ERROR;
        ccid_recv_wakeupFromISR();
      }
      break;

    case STATUS_RECV_ENDPOINT :
      ccid_receiver.bChecksum = bValue;
      ccid_receiver.bEndpoint = bValue;
      ccid_receiver.eStatus = STATUS_RECV_HEADER;
      ccid_receiver.wLength = CCID_HEADER_LENGTH;
      break;

    case STATUS_RECV_HEADER :
      ccid_receiver.bChecksum ^= bValue;
      ccid_receiver.abBuffer[ccid_receiver.wOffset++] = bValue;
      if (ccid_receiver.wOffset >= CCID_HEADER_LENGTH)
      {
        DWORD dwLength = utohl(&ccid_receiver.abBuffer[CCID_POS_LENGTH]);
        if (dwLength > CCID_MAX_PAYLOAD_LENGTH)
        {
          ccid_receiver.eStatus = STATUS_ERROR;
          ccid_recv_wakeupFromISR();
        } else
        if (dwLength)
        {
          ccid_receiver.eStatus = STATUS_RECV_PAYLOAD;
          ccid_receiver.wLength = (WORD) (CCID_HEADER_LENGTH + dwLength);
        } else
        {
          ccid_receiver.eStatus = STATUS_RECV_CHECKSUM;
        }
      }
      break;

    case STATUS_RECV_PAYLOAD :
      ccid_receiver.bChecksum ^= bValue;
      ccid_receiver.abBuffer[ccid_receiver.wOffset++] = bValue;
      if (ccid_receiver.wOffset >= ccid_receiver.wLength)
      {
        ccid_receiver.eStatus = STATUS_RECV_CHECKSUM;
      }
      break;

    case STATUS_RECV_CHECKSUM :
      ccid_receiver.bChecksum ^= bValue;
      if (!ccid_receiver.bChecksum)
      {
        ccid_receiver.eStatus = STATUS_READY;
      } else
      {
        ccid_receiver.eStatus = STATUS_ERROR;
      }
      ccid_recv_wakeupFromISR();
      break;

    case STATUS_ERROR :
      /* Silently discard, the error will be handled later on */
      break;

    default :
      ccid_receiver.eStatus = STATUS_ERROR;
      ccid_recv_wakeupFromISR();
      break;
  }  
}

SCARD_RC ccid_recv(CCID_PACKET_ST *packet)
{
  if (packet == NULL)
    return _SCARD_E_INVALID_PARAMETER;

  packet->bEndpoint = ccid_receiver.bEndpoint;
  memcpy(packet->Header.u, ccid_receiver.abBuffer, CCID_HEADER_LENGTH);
  packet->Header.p.Length.dw = utohl(&ccid_receiver.abBuffer[CCID_POS_LENGTH]);

  if (packet->bEndpoint == CCID_COMM_CONTROL_TO_PC)
  {
    packet->Header.p.Data.Control.Value.w = utohs(packet->Header.p.Data.Control.Value.ab);
    packet->Header.p.Data.Control.Index.w = utohs(packet->Header.p.Data.Control.Index.ab);
  }

  if (packet->Header.p.Length.dw)
  {
    if (packet->abRecvPayload == NULL)
      return _SCARD_E_INSUFFICIENT_BUFFER;
    if (packet->wRecvPayloadMaxLen < packet->Header.p.Length.dw)
      return _SCARD_E_INSUFFICIENT_BUFFER;

    memcpy(packet->abRecvPayload, &ccid_receiver.abBuffer[CCID_HEADER_LENGTH], packet->Header.p.Length.dw);
  }

  D(printf("\n"));
  return _SCARD_S_SUCCESS;
}
