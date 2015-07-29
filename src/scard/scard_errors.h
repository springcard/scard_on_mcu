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
#ifndef __SCARD_ERRORS_H__
#define __SCARD_ERRORS_H__

typedef unsigned short SCARD_RC;

#define _SCARD_S_SUCCESS               (0x0000)
#define _SCARD_F_INTERNAL_ERROR        (0x0001)
#define _SCARD_E_CANCELLED             (0x0002)
#define _SCARD_E_INVALID_HANDLE        (0x0003)
#define _SCARD_E_INVALID_PARAMETER     (0x0004)
#define _SCARD_E_INVALID_TARGET        (0x0005)
#define _SCARD_E_NO_MEMORY             (0x0006)
#define _SCARD_F_WAITED_TOO_LONG       (0x0007)
#define _SCARD_E_INSUFFICIENT_BUFFER   (0x0008)
#define _SCARD_E_UNKNOWN_READER        (0x0009)
#define _SCARD_E_TIMEOUT               (0x000A)
#define _SCARD_E_SHARING_VIOLATION     (0x000B)
#define _SCARD_E_NO_SMARTCARD          (0x000C)
#define _SCARD_E_UNKNOWN_CARD          (0x000D)
#define _SCARD_E_CANT_DISPOSE          (0x000E)
#define _SCARD_E_PROTO_MISMATCH        (0x000F)
#define _SCARD_E_NOT_READY             (0x0010)
#define _SCARD_E_INVALID_VALUE         (0x0011)
#define _SCARD_E_SYSTEM_CANCELLED      (0x0012)
#define _SCARD_F_COMM_ERROR            (0x0013)
#define _SCARD_F_UNKNOWN_ERROR         (0x0014)
#define _SCARD_E_INVALID_ATR           (0x0015)
#define _SCARD_E_NOT_TRANSACTED        (0x0016)
#define _SCARD_E_READER_UNAVAILABLE    (0x0017)
#define _SCARD_E_P_SHUTDOWN            (0x0018)
#define _SCARD_E_PCI_TOO_SMALL         (0x0019)
#define _SCARD_E_READER_UNSUPPORTED    (0x001A)
#define _SCARD_E_DUPLICATE_READER      (0x001B)
#define _SCARD_E_CARD_UNSUPPORTED      (0x001C)
#define _SCARD_E_NO_SERVICE            (0x001D)
#define _SCARD_E_SERVICE_STOPPED       (0x001E)
#define _SCARD_E_UNEXPECTED            (0x001F)
#define _SCARD_E_UNSUPPORTED_FEATURE   (0x0022)
#define _SCARD_E_NO_READERS_AVAILABLE  (0x002E)
#define _SCARD_E_COMM_DATA_LOST        (0x002F)
#define _SCARD_W_UNSUPPORTED_CARD      (0x0065)
#define _SCARD_W_UNRESPONSIVE_CARD     (0x0066)
#define _SCARD_W_UNPOWERED_CARD        (0x0067)
#define _SCARD_W_RESET_CARD            (0x0068)
#define _SCARD_W_REMOVED_CARD          (0x0069)
#define _SCARD_W_INSERTED_CARD         (0x006A)

#endif
