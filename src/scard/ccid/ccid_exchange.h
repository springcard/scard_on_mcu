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
#ifndef __CCID_EXCHANGE_H__
#define __CCID_EXCHANGE_H__

#include "../scard_errors.h"
#include "ccid_typedefs.h"

void ccid_packet_init(CCID_PACKET_ST *packet);
SCARD_RC ccid_exchange(CCID_PACKET_ST *packet, WORD timeout_ms);

SCARD_RC ccid_send(CCID_PACKET_ST *packet);
SCARD_RC ccid_recv(CCID_PACKET_ST *packet);

void ccid_recv_cleanup(void);
SCARD_RC ccid_recv_status(void);
SCARD_RC ccid_recv_wait(WORD timeout_ms);
void ccid_recv_wakeupFromISR(void);

BYTE ccid_sequence(BYTE bSlot);
void ccid_next_sequence(BYTE bSlot);

#endif
