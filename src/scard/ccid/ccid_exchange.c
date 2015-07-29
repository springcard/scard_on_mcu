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
#include "ccid_errors.h"
#include "ccid_platform.h"

typedef struct
{
  BYTE bSequence;
} CCID_SLOT_ST;

#define CCID_SLOT_COUNT 1

static CCID_SLOT_ST ccid_slot[CCID_SLOT_COUNT];

BYTE ccid_sequence(BYTE bSlot)
{
  if (bSlot < CCID_SLOT_COUNT)
    return ccid_slot[bSlot].bSequence;
  return (BYTE) -1;
}

void ccid_next_sequence(BYTE bSlot)
{
  if (bSlot < CCID_SLOT_COUNT)
    ccid_slot[bSlot].bSequence++;
}

void ccid_packet_init(CCID_PACKET_ST *packet)
{
  if (packet != NULL)
    memset(packet, 0, sizeof(CCID_PACKET_ST));
}

static SCARD_RC ccid_recv_to_slot_error(BYTE bSlotError)
{
  SCARD_RC rc;

  switch (bSlotError)
  {
    case CCID_ERR_UNKNOWN :
    case CCID_ERR_PARAMETERS :
    case CCID_ERR_CMD_NOT_SUPPORTED :
    case CCID_ERR_BAD_LENGTH :
    case CCID_ERR_BAD_SLOT :
    case CCID_ERR_BAD_POWERSELECT :
    case CCID_ERR_BAD_LEVELPARAMETER :
    case CCID_ERR_BAD_FIDI :
    case CCID_ERR_BAD_T01CONVCHECKSUM :
    case CCID_ERR_BAD_GUARDTIME :
    case CCID_ERR_BAD_WAITINGINTEGER :
    case CCID_ERR_BAD_CLOCKSTOP :
    case CCID_ERR_BAD_IFSC :
    case CCID_ERR_BAD_NAD :
    case CCID_ERR_CMD_ABORTED :    
      rc = _SCARD_E_UNEXPECTED;
      break;    
    case CCID_ERR_ICC_MUTE :
    case CCID_ERR_XFR_PARITY_ERROR :
    case CCID_ERR_XFR_OVERRUN :
    case CCID_ERR_HW_ERROR :
      rc = _SCARD_W_UNRESPONSIVE_CARD;
      break;    
    case CCID_ERR_BAD_ATR_TS :
    case CCID_ERR_BAD_ATR_TCK :
    case CCID_ERR_ICC_PROTOCOL_NOT_SUPPORTED :
    case CCID_ERR_ICC_CLASS_NOT_SUPPORTED :
    case CCID_ERR_PROCEDURE_BYTE_CONFLICT :
    case CCID_ERR_DEACTIVATED_PROTOCOL :    
      rc = _SCARD_W_UNSUPPORTED_CARD;
      break;    
    case CCID_ERR_BUSY_WITH_AUTO_SEQUENCE :
    case CCID_ERR_CMD_SLOT_BUSY :
      rc = _SCARD_E_UNEXPECTED;
      break;    
    case CCID_SUCCESS :
    default :
      rc = _SCARD_S_SUCCESS;
      break;
  }

  return rc;
}

static SCARD_RC ccid_recv_to_slot_status(CCID_PACKET_ST *packet)
{
  switch (packet->Header.p.Data.BulkIn.bSlotStatus & 0xC0)
  {
    case 0x00 :
      break;
    case 0x40 :
      return ccid_recv_to_slot_error(packet->Header.p.Data.BulkIn.bSlotError);
    case 0x80 :
    case 0xC0 :
    default :
      return _SCARD_E_READER_UNSUPPORTED;
  }

  switch (packet->Header.p.Data.BulkIn.bSlotStatus & 0x03)
  {
    case 0x00 :
      return _SCARD_S_SUCCESS;
    case 0x01 :
      return _SCARD_W_UNRESPONSIVE_CARD;
    case 0x02 :
      return _SCARD_W_REMOVED_CARD;
    case 0x03 :
    default :
      return _SCARD_E_READER_UNSUPPORTED;
  }
}

SCARD_RC ccid_exchange(CCID_PACKET_ST *packet, WORD timeout_ms)
{
  SCARD_RC rc;
  BYTE bEndpoint;
  BYTE bSlot, bSequence;
  WORD wIndex, wValue;

  if (packet == NULL)
    return _SCARD_F_INTERNAL_ERROR;

  bEndpoint = packet->bEndpoint;
  wIndex    = packet->Header.p.Data.Control.Index.w;
  wValue    = packet->Header.p.Data.Control.Value.w;
  bSlot     = packet->Header.p.Data.BulkOut.bSlot;
  bSequence = packet->Header.p.Data.BulkOut.bSequence;

  rc = ccid_send(packet);
  if (rc != _SCARD_S_SUCCESS)
    return rc;

  rc = ccid_recv_wait(timeout_ms);
  if (rc != _SCARD_S_SUCCESS)
    return rc;

  rc = ccid_recv_status();
  if (rc != _SCARD_S_SUCCESS)
    return rc;

  rc = ccid_recv(packet);
  ccid_recv_cleanup();
  if (rc != _SCARD_S_SUCCESS)
    return rc;

  switch (bEndpoint)
  {
    case CCID_COMM_CONTROL_TO_RDR :
      if (packet->bEndpoint != CCID_COMM_CONTROL_TO_PC)
      {
        rc = _SCARD_E_READER_UNSUPPORTED;
      } else
      if ((packet->Header.p.Data.Control.Index.w != wIndex) || (packet->Header.p.Data.Control.Value.w != wValue))
      {
        rc = _SCARD_E_READER_UNSUPPORTED;
      }
      break;

    case CCID_COMM_BULK_PC_TO_RDR :
      if (packet->bEndpoint != CCID_COMM_BULK_RDR_TO_PC)
      {
        rc = _SCARD_E_READER_UNSUPPORTED;
      } else
      if ((packet->Header.p.Data.BulkIn.bSlot != bSlot) || (packet->Header.p.Data.BulkIn.bSequence != bSequence))
      {
        rc = _SCARD_E_READER_UNSUPPORTED;
      } else
      {
        rc = ccid_recv_to_slot_status(packet);
      }
      break;

    default :
      break;
  }

  return rc;
}
