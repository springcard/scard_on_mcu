/**h* SCARD_On_MCU/scard_reader
 *
 * NAME
 *   scard_reader -- Functions to control a SpringCard CCID Serial reader in the context of a MCU
 *
 * COPYRIGHT
 *   Copyright (c) 2015 SPRINGCARD SAS, FRANCE - www.springcard.com
 *
 * DISCLAIMER
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

/**f* scard_reader/scardPingReader
 *
 * NAME
 *   scardPingReader
 *
 * DESCRIPTION
 *   Test the communication with the reader
 *
 * SYNOPSIS
 *   SCARD_RC scardPingReader(void)
 *
 * INPUTS
 *   None
 *
 * RETURNS
 *   _SCARD_S_SUCCESS         : success
 *   Other code if internal or communication error has occured. 
 * 
 * SEE ALSO
 *   scardStartReader
 *
 **/
SCARD_RC scardPingReader(void)
{
  CCID_PACKET_ST packet;
  SCARD_RC rc;

  ccid_packet_init(&packet);

  packet.bEndpoint = CCID_COMM_CONTROL_TO_RDR;
  packet.Header.p.bRequest = GET_STATUS;

  rc = ccid_exchange(&packet, SHORT_TIMEOUT);
  
  return rc;
}

/**f* scard_reader/scardStartReader
 *
 * NAME
 *   scardStartReader
 *
 * DESCRIPTION
 *   Ask the reader to start working in PC/SC mode
 *
 * SYNOPSIS
 *   SCARD_RC scardStartReader(void)
 *
 * INPUTS
 *   None
 *
 * RETURNS
 *   _SCARD_S_SUCCESS         : success
 *   Other code if internal or communication error has occured. 
 * 
 * SEE ALSO
 *   scardPingReader
 *   scardStopReader
 *
 **/
SCARD_RC scardStartReader(void)
{
  CCID_PACKET_ST packet;
  SCARD_RC rc;

  ccid_packet_init(&packet);

  packet.bEndpoint = CCID_COMM_CONTROL_TO_RDR;
  packet.Header.p.bRequest = SET_CONFIGURATION;
  packet.Header.p.Data.Control.Value.w = 1;
  packet.Header.p.Data.Control.Index.w = 0;
  packet.Header.p.Data.Control.InOut.bOutOption = 0;

  rc = ccid_exchange(&packet, SHORT_TIMEOUT);
  
  if (rc == _SCARD_S_SUCCESS)
  {
    if (packet.Header.p.Data.Control.InOut.bInStatus != 0x01)
      rc = _SCARD_E_UNEXPECTED;
  }
  
  return rc;
}

/**f* scard_reader/scardStopReader
 *
 * NAME
 *   scardStopReader
 *
 * DESCRIPTION
 *   Ask the reader to stop. This is useful to limit power consumption.
 *
 * SYNOPSIS
 *   SCARD_RC scardStopReader(void)
 *
 * INPUTS
 *   None
 *
 * RETURNS
 *   _SCARD_S_SUCCESS         : success
 *   Other code if internal or communication error has occured. 
 * 
 * SEE ALSO
 *   scardStartReader
 *
 **/
SCARD_RC scardStopReader(void)
{
  CCID_PACKET_ST packet;
  SCARD_RC rc;

  ccid_packet_init(&packet);

  packet.bEndpoint = CCID_COMM_CONTROL_TO_RDR;
  packet.Header.p.bRequest = SET_CONFIGURATION;
  packet.Header.p.Data.Control.Value.w = 0;
  packet.Header.p.Data.Control.Index.w = 0;
  packet.Header.p.Data.Control.InOut.bOutOption = 0;

  rc = ccid_exchange(&packet, SHORT_TIMEOUT);
  
  if (rc == _SCARD_S_SUCCESS)
  {
    if (packet.Header.p.Data.Control.InOut.bInStatus != 0x00)
      rc = _SCARD_E_UNEXPECTED;
  }
  
  return rc;
}

/**f* scard_reader/scardGetReaderDescriptor
 *
 * NAME
 *   scardGetReaderDescriptor
 *
 * DESCRIPTION
 *   Retrieve a descriptor form the reader.
 *
 * SYNOPSIS
 *   SCARD_RC scardGetReaderDescriptor(BYTE bType, BYTE bIndex, BYTE abDescriptor[], WORD *pwDescriptorLength)
 *
 * INPUTS
 *   BYTE bType               : the type of the descriptor (see USB spec)
 *   BYTE bIndex              : the index of the descriptor (see USB spec)
 *   BYTE abDescriptor[]      : buffer to receive the response
 *   WORD *pwDescriptorLength : IN  : the size of the buffer
 *                              OUT : the actual size of the response
 *
 * RETURNS
 *   _SCARD_S_SUCCESS         : success
 *   Other code if internal or communication error has occured. 
 * 
 * SEE ALSO
 *   scardPingReader
 *   scardStartReader
 *
 **/
SCARD_RC scardGetReaderDescriptor(BYTE bType, BYTE bIndex, BYTE abDescriptor[], WORD *pwDescriptorLength)
{
  CCID_PACKET_ST packet;
  SCARD_RC rc;

  if ((abDescriptor != NULL) && (pwDescriptorLength == NULL))
    return _SCARD_E_INVALID_PARAMETER;

  ccid_packet_init(&packet);

  packet.bEndpoint = CCID_COMM_CONTROL_TO_RDR;
  packet.Header.p.bRequest = GET_DESCRIPTOR;
  packet.Header.p.Data.Control.Value.ab[0] = bType;
  packet.Header.p.Data.Control.Value.ab[1] = bIndex;

  if (pwDescriptorLength != NULL)
  {
    packet.abRecvPayload = abDescriptor;
    packet.wRecvPayloadMaxLen = *pwDescriptorLength;
  }

  rc = ccid_exchange(&packet, SHORT_TIMEOUT);
  
  if (rc == _SCARD_S_SUCCESS)
  {
    if (packet.Header.p.Data.Control.InOut.bInStatus != 0x00)
    {
      rc = _SCARD_E_UNEXPECTED;
    } else
    {
      if (pwDescriptorLength != NULL)
        *pwDescriptorLength = (WORD) packet.Header.p.Length.dw;
    }
  }
  
  return rc;
}

void scard_Cleanup(void)
{
  ccid_recv_wait(10);
  ccid_recv_cleanup();
  ccid_recv_wait(10);
}
