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
#ifndef __CCID_ERRORS_H__
#define __CCID_ERRORS_H__

/* Errors */
#define	CCID_SUCCESS					                    0x81
#define	CCID_ERR_UNKNOWN				                  0x82
#define	CCID_ERR_PARAMETERS 		                  0x83

#define	CCID_ERR_CMD_NOT_SUPPORTED                0x00
#define	CCID_ERR_BAD_LENGTH                       0x01
#define	CCID_ERR_BAD_SLOT                         0x05
#define	CCID_ERR_BAD_POWERSELECT                  0x07
#define	CCID_ERR_BAD_PROTOCOLNUM                  0x07
#define	CCID_ERR_BAD_CLOCKCOMMAND                 0x07
#define	CCID_ERR_BAD_ABRFU_3B                     0x07
#define	CCID_ERR_BAD_ABRFU_2B                     0x08
#define	CCID_ERR_BAD_LEVELPARAMETER               0x08
#define	CCID_ERR_BAD_FIDI                         0x0A
#define	CCID_ERR_BAD_T01CONVCHECKSUM              0x0B
#define	CCID_ERR_BAD_GUARDTIME                    0x0C
#define	CCID_ERR_BAD_WAITINGINTEGER               0x0D
#define	CCID_ERR_BAD_CLOCKSTOP                    0x0E
#define	CCID_ERR_BAD_IFSC                         0x0F
#define	CCID_ERR_BAD_NAD                          0x10

/* Standard error codes */
#define	CCID_ERR_CMD_ABORTED                      0xFF
#define	CCID_ERR_ICC_MUTE                         0xFE
#define	CCID_ERR_XFR_PARITY_ERROR                 0xFD
#define	CCID_ERR_XFR_OVERRUN                      0xFC
#define	CCID_ERR_HW_ERROR                         0xFB
#define	CCID_ERR_BAD_ATR_TS                       0xF8
#define	CCID_ERR_BAD_ATR_TCK                      0xF7
#define	CCID_ERR_ICC_PROTOCOL_NOT_SUPPORTED       0xF6
#define	CCID_ERR_ICC_CLASS_NOT_SUPPORTED          0xF5
#define	CCID_ERR_PROCEDURE_BYTE_CONFLICT          0xF4
#define	CCID_ERR_DEACTIVATED_PROTOCOL             0xF3
#define	CCID_ERR_BUSY_WITH_AUTO_SEQUENCE          0xF2
#define	CCID_ERR_PIN_TIMEOUT                      0xF0
#define	CCID_ERR_PIN_CANCELLED                    0xEF
#define	CCID_ERR_CMD_SLOT_BUSY                    0xE0

#endif