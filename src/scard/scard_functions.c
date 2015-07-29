/**h* SCARD_On_MCU/scard_functions
 *
 * NAME
 *   scard_functions -- PC/SC-like functions to use a SpringCard Serial CCID reader in the context of a MCU
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
#include "../project.h"
#include "scard_errors.h"
#include "scard_functions.h"
#include "ccid/ccid_exchange.h"

/**f* scard_functions/scardStatus
 *
 * NAME
 *   scardStatus
 *
 * DESCRIPTION
 *   Retrieve the status of a slot:
 *   - is there a card in the slot or not?
 *   - is the card powered or not?
 *   This is (more or less) the MCU-counterpart of SCardStatus.
 *
 * SYNOPSIS
 *   SCARD_RC scardStatus(BYTE bSlot, BOOL *pfCardPresent, BOOL *pfCardPowered)
 *
 * INPUTS
 *   BYTE bSlot          : slot number (0 for the contactless slot)
 *   BOOL *pfCardPresent : pointer to receive the 'present' flag
 *   BOOL *pfCardPowered : pointer to receive the 'powered' flag
 *
 * RETURNS
 *   _SCARD_S_SUCCESS    : success, flags are reliable
 *   Other code if internal or communication error has occured. 
 * 
 * SEE ALSO
 *   scardConnect
 *
 * NOTES
 *   There's no need to loop around scardStatus to wait for card insertion.
 *   Looping around scardConnect is more efficient since when a card is
 *   inserted, the ATR will be returned at once.
 *   On the other hand, looping around scardStatus is useful to wait for
 *   card removal (after a call to scardDisconnect).
 *
 **/
SCARD_RC scardStatus(BYTE bSlot, BOOL *pfCardPresent, BOOL *pfCardPowered)
{
  CCID_PACKET_ST packet;
  SCARD_RC rc;

  ccid_packet_init(&packet);

  packet.bEndpoint = CCID_COMM_BULK_PC_TO_RDR;
  packet.Header.p.bRequest = PC_TO_RDR_GETSLOTSTATUS;
  packet.Header.p.Data.BulkOut.bSlot = bSlot;
  packet.Header.p.Data.BulkOut.bSequence = ccid_sequence(bSlot);

  rc = ccid_exchange(&packet, SHORT_TIMEOUT);

  if (!scard_IsReaderFatalError(rc))
  {
    if (packet.Header.p.bRequest != RDR_TO_PC_SLOTSTATUS)
    {
      printf("status!Request=%02X\n", packet.Header.p.bRequest);
      rc = _SCARD_E_READER_UNSUPPORTED;
    } else
    {
      rc = _SCARD_S_SUCCESS;
      switch (packet.Header.p.Data.BulkIn.bSlotStatus & 0x03)
      {
        case 0x00 :
          if (pfCardPresent != NULL)
            *pfCardPresent = TRUE;
          if (pfCardPowered != NULL)
            *pfCardPowered = TRUE;
          break;
        case 0x01 :
          if (pfCardPresent != NULL)
            *pfCardPresent = TRUE;
          if (pfCardPowered != NULL)
            *pfCardPowered = FALSE;
          break;
        case 0x02 :
          if (pfCardPresent != NULL)
            *pfCardPresent = FALSE;
          if (pfCardPowered != NULL)
            *pfCardPowered = FALSE;
          break;
        case 0x03 :
        default :
          printf("status!Status=%02X\n", packet.Header.p.Data.BulkIn.bSlotStatus);
          rc = _SCARD_E_READER_UNSUPPORTED;
      }
    }
  } else
  {
    printf("status:rc=%04X\n", rc);
  }

  if (rc != _SCARD_S_SUCCESS)
  {
    printf("Oups\n");
    system("pause");
  }

  if (rc == _SCARD_S_SUCCESS)
    ccid_next_sequence(bSlot);
  
  return rc;
}

/**f* scard_functions/scardConnect
 *
 * NAME
 *   scardConnect
 *
 * DESCRIPTION
 *   Retrieve the ATR of the card that is in a slot.
 *   This is (more or less) the MCU-counterpart of SCardConnect.
 *
 * SYNOPSIS
 *   SCARD_RC scardConnect(BYTE bSlot, BYTE abAtr[], WORD *wAtrLength)
 *
 * INPUTS
 *   BYTE bSlot                : slot number (0 for the contactless slot)
 *   BYTE abAtr[]              : buffer to receive the ATR (must be at least 32-byte long)
 *   WORD *wAtrLength          : IN  : the size of the ATR buffer
 *                               OUT : the actual size of the ATR
 *
 * RETURNS
 *   _SCARD_S_SUCCESS           : success, ATR is valid
 *   _SCARD_W_REMOVED_CARD      : there's no card in the slot
 *   _SCARD_W_UNSUPPORTED_CARD  : there's a card in the slot, but its ATR is invalid
 *   _SCARD_W_UNRESPONSIVE_CARD : there's a card in the slot, but it is mute
 *   Other code if internal or communication error has occured. 
 * 
 * SEE ALSO
 *   scardStatus
 *   scardDisconnect
 *
 **/
SCARD_RC scardConnect(BYTE bSlot, BYTE abAtr[], WORD *wAtrLength)
{
  CCID_PACKET_ST packet;
  SCARD_RC rc;

  if ((abAtr == NULL) || (wAtrLength == NULL))
    return _SCARD_E_INVALID_PARAMETER;

  ccid_packet_init(&packet);

  packet.bEndpoint = CCID_COMM_BULK_PC_TO_RDR;
  packet.Header.p.bRequest = PC_TO_RDR_ICCPOWERON;
  packet.Header.p.Data.BulkOut.bSlot = bSlot;
  packet.Header.p.Data.BulkOut.bSequence = ccid_sequence(bSlot);

  packet.abRecvPayload = abAtr;
  packet.wRecvPayloadMaxLen = *wAtrLength;

  rc = ccid_exchange(&packet, SHORT_TIMEOUT);

  if (rc == _SCARD_S_SUCCESS)
  {
    if (packet.Header.p.bRequest != RDR_TO_PC_DATABLOCK)
    {
      rc = _SCARD_E_READER_UNSUPPORTED;
    } else
    {
      *wAtrLength = (WORD) packet.Header.p.Length.dw;
    }
  }

  if (rc == _SCARD_S_SUCCESS)
    ccid_next_sequence(bSlot);
  
  return rc;
}

/**f* scard_functions/scardDisconnect
 *
 * NAME
 *   scardDisconnect
 *
 * DESCRIPTION
 *   Disconnect from the card.
 *   This is (more or less) the MCU-counterpart of SCardDisconnect.
 *
 * SYNOPSIS
 *   SCARD_RC scardDisconnect(BYTE bSlot)
 *
 * INPUTS
 *   BYTE bSlot       : slot number (0 for the contactless slot)
 *
 * RETURNS
 *   _SCARD_S_SUCCESS : success
 *   Other code if internal or communication error has occured. 
 * 
 * SEE ALSO
 *   scardStatus
 *   scardConnect
 *
 **/
SCARD_RC scardDisconnect(BYTE bSlot)
{
  CCID_PACKET_ST packet;
  SCARD_RC rc;

  ccid_packet_init(&packet);

  packet.bEndpoint = CCID_COMM_BULK_PC_TO_RDR;
  packet.Header.p.bRequest = PC_TO_RDR_ICCPOWEROFF;
  packet.Header.p.Data.BulkOut.bSlot = bSlot;
  packet.Header.p.Data.BulkOut.bSequence = ccid_sequence(bSlot);

  rc = ccid_exchange(&packet, SHORT_TIMEOUT);

  if ((rc == _SCARD_W_UNSUPPORTED_CARD) || (rc == _SCARD_W_UNRESPONSIVE_CARD) || (rc == _SCARD_W_REMOVED_CARD))
    rc = _SCARD_S_SUCCESS;

  if (rc == _SCARD_S_SUCCESS)
    ccid_next_sequence(bSlot);

  return rc;
}

/**f* scard_functions/scardTransmit
 *
 * NAME
 *   scardTransmit
 *
 * DESCRIPTION
 *   Send a Command APDU to the card, and receive its Response APDU.
 *   This is the MCU-counterpart of SCardTransmit.
 *
 * SYNOPSIS
 *   SCARD_RC scardTransmit(BYTE bSlot, const BYTE abSendApdu[], WORD wSendLength, BYTE abRecvApdu[], WORD *pwRecvLength)
 *
 * INPUTS
 *   BYTE bSlot              : slot number (0 for the contactless slot)
 *   const BYTE abSendApdu[] : buffer holding the C-APDU
 *   WORD wSendLength        : the size of the C-APDU
 *   BYTE abRecvApdu[]       : buffer to receive the R-APDU
 *   WORD *pwRecvLength      : IN  : the size of the R-APDU buffer
 *                             OUT : the actual size of the R-APDU
 *
 * RETURNS
 *   _SCARD_S_SUCCESS        : success, R-APDU is valid
 *   _SCARD_W_REMOVED_CARD   : the card has been removed during the exchange
 *   Other code if internal or communication error has occured. 
 * 
 * SEE ALSO
 *   scardConnect
 *
 **/
SCARD_RC scardTransmit(BYTE bSlot, const BYTE abSendApdu[], WORD wSendLength, BYTE abRecvApdu[], WORD *pwRecvLength)
{
  CCID_PACKET_ST packet;
  SCARD_RC rc;

  if ((abSendApdu == NULL) || (wSendLength > CCID_MAX_PAYLOAD_LENGTH))
    return _SCARD_E_INVALID_PARAMETER;
  if ((abRecvApdu != NULL) && (pwRecvLength == NULL))
    return _SCARD_E_INVALID_PARAMETER;

  ccid_packet_init(&packet);

  packet.bEndpoint = CCID_COMM_BULK_PC_TO_RDR;
  packet.Header.p.bRequest = PC_TO_RDR_XFRBLOCK;
  packet.Header.p.Data.BulkOut.bSlot = bSlot;
  packet.Header.p.Data.BulkOut.bSequence = ccid_sequence(bSlot);

  packet.abSendPayload = abSendApdu;
  packet.Header.p.Length.dw = wSendLength;

  if (pwRecvLength != NULL)
  {
    packet.abRecvPayload = abRecvApdu;
    packet.wRecvPayloadMaxLen = *pwRecvLength;
  }

  rc = ccid_exchange(&packet, LONG_TIMEOUT);
  
  if (rc == _SCARD_S_SUCCESS)
    if (pwRecvLength != NULL)
      *pwRecvLength = (WORD) packet.Header.p.Length.dw;

  if ((rc == _SCARD_W_UNSUPPORTED_CARD) || (rc == _SCARD_W_UNRESPONSIVE_CARD) || (rc == _SCARD_W_UNPOWERED_CARD) || (rc == _SCARD_W_RESET_CARD))
    rc = _SCARD_W_REMOVED_CARD;

  if (rc == _SCARD_S_SUCCESS)
    ccid_next_sequence(bSlot);

  return rc;
}

/**f* scard_functions/scardControl
 *
 * NAME
 *   scardControl
 *
 * DESCRIPTION
 *   Send a direct ('Vendor Escape') command to the reader, and receive its response.
 *   This is the MCU-counterpart of SCardControl.
 *
 * SYNOPSIS
 *   SCARD_RC scardControl(const BYTE abSendBuffer[], WORD wSendLength, BYTE abRecvBuffer[], WORD *pwRecvLength)
 *
 * INPUTS
 *   const BYTE abSendBuffer[] : buffer holding the command
 *   WORD wSendLength          : the size of the command
 *   BYTE abRecvBuffer[]       : buffer to receive the response
 *   WORD *pwRecvLength        : IN  : the size of the response buffer
 *                               OUT : the actual size of the response
 *
 * RETURNS
 *   _SCARD_S_SUCCESS           : success
 *   Other code if internal or communication error has occured. 
 * 
 * SEE ALSO
 *   scardTransmit
 *
 **/
SCARD_RC scardControl(const BYTE abSendBuffer[], WORD wSendLength, BYTE abRecvBuffer[], WORD *pwRecvLength)
{
  CCID_PACKET_ST packet;
  BYTE bDummyRecvByte;
  SCARD_RC rc;

  if ((abSendBuffer == NULL) || (wSendLength > CCID_MAX_PAYLOAD_LENGTH))
    return _SCARD_E_INVALID_PARAMETER;
  if ((abRecvBuffer != NULL) && (pwRecvLength == NULL))
    return _SCARD_E_INVALID_PARAMETER;

  ccid_packet_init(&packet);

  packet.bEndpoint = CCID_COMM_BULK_PC_TO_RDR;
  packet.Header.p.bRequest = PC_TO_RDR_ESCAPE;

  packet.abSendPayload = abSendBuffer;
  packet.Header.p.Length.dw = wSendLength;

  if (pwRecvLength != NULL)
  {
    packet.abRecvPayload = abRecvBuffer;
    packet.wRecvPayloadMaxLen = *pwRecvLength;
  } else
  {
    packet.abRecvPayload = &bDummyRecvByte;
    packet.wRecvPayloadMaxLen = 1;
  }

  rc = ccid_exchange(&packet, LONG_TIMEOUT);

  if (!scard_IsReaderFatalError(rc))
  {
    if (packet.Header.p.bRequest != RDR_TO_PC_ESCAPE)
    {
      rc = _SCARD_E_READER_UNSUPPORTED;
    } else
    {
      rc = _SCARD_S_SUCCESS;
      if (pwRecvLength != NULL)
      {
        *pwRecvLength = (WORD) packet.Header.p.Length.dw;
      } else
      if (bDummyRecvByte != 0)
      {
        /* The reader has returned an error */
        return _SCARD_F_UNKNOWN_ERROR;
      }
    }
  }

  return rc;
}
