/*

  SCARD_On_MCU
  ------------

  Implementation of CCID over Serial for MCU targets


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
#include "project.h"
#include "scard/scard_functions.h"
#include "scard/ccid/ccid_platform.h"

/* This is a test on Windows, our reference reder uses COM4 */
const char *CommDevice = "COM4";

/* The contactless slot has numnber 0 */
#define CONTACTLESS_SLOT 0

/* These are very basic Command APDUs to access a contactless memory card, see PC/SC v2 chapter 3 for explanations */
static const BYTE APDU_GET_UID[] = { 0xFF, 0xCA, 0x00, 0x00, 0x00 };
static const BYTE APDU_READ_20[] = { 0xFF, 0xB0, 0x00, 0x00, 0x20 };

/* Some nice commands to drive the SpringCard reader, refer to the Developer's Guide */
static const BYTE CTRL_LED_RED_ON[]       = { 0x58, 0x1E, 0x01, 0x00 };
static const BYTE CTRL_LED_GREEN_ON[]     = { 0x58, 0x1E, 0x00, 0x01 };
static const BYTE CTRL_LED_GREEN_BLINKS[] = { 0x58, 0x1E, 0x00, 0x04 };
static const BYTE CTRL_LEDS_OFF[]         = { 0x58, 0x1E, 0x00, 0x00 };
static const BYTE CTRL_SET_PROTOCOLS[]    = { 0x58, 0x8D, 0xB0, 0x00, 0x03 }; /* Here we select only ISO 14443 (A & B) */

/* Retrieve the Status Word at the end of a response APDU */
WORD SW(BYTE abRecvApdu[], WORD wRecvLength)
{
  if (abRecvApdu == NULL)
    return 0xFFFF;
  if (wRecvLength < 2)
    return 0xFFFF;
  return (abRecvApdu[wRecvLength-2] * 0x0100) + abRecvApdu[wRecvLength-1];
}

/* The string descriptors are UNICODE strings, basically we show only one byte out of two (and skip the 'Lang' header) */
void print_string_descriptor(BYTE abRecvBuffer[], WORD wRecvLength)
{
  WORD i;

  for (i=2; i<wRecvLength; i+=2)
    printf("%c", abRecvBuffer[i]);
}

/*
 * The reference program
 * ---------------------
 */
void main(int argc, char **argv)
{
  SCARD_RC rc;
  BYTE abRecvBuffer[256];
  WORD wRecvLength;
  BOOL fCardPresent, fDataRead;
  WORD sw;
  WORD i;

  printf("\n");
  printf("SpringCard PC/SC SDK : implementation of CCID over Serial for MCU targets\n");
  printf("-------------------------------------------------------------------------\n");
  printf("Copyright (c) 2015 SPRINGCARD SAS, FRANCE - www.springcard.com\n");
  printf("See LICENSE.txt for disclaimer and license requirements\n");
  printf("\n");
  printf("usage: scard_on_mcu [comm. device]\n");

  if (argc >= 2)
  {
    CommDevice = argv[1];
  }

  printf("Using communication device: %s\n", CommDevice);  

  /* Prepare the underlying hardware and lower layer software */
  /* -------------------------------------------------------- */

  if (!platform_init())
  {
    printf("Failed to init the platform\n");
    goto failed;
  }

  if (!serial_init())
  {
    printf("Failed to init the serial link\n");
    goto failed;
  }

  printf("\n\n***** Now running forever, hit Ctrl+C to exit ****\n\n");

  /*
   * Connect or reconnect to the reader
   * ----------------------------------
   */

restart_reader:

  /* We use scardPingReader to verify that we're actually able to communicate with a compliant device */
  /* ------------------------------------------------------------------------------------------------ */
  rc = scardPingReader();
  if (rc != _SCARD_S_SUCCESS)
  {
    printf("Failed to ping the reader (rc=%04X)\n", rc);
    /* In case of a communication error, we shall wait at least 1200ms so the reader may reset its state machine */
    sleep_ms(1200);
    /* Clear the state machine */
    scard_Cleanup();
    /* Try again */
    goto restart_reader;
  }

  printf("It seems that we've found a reader...\n");

  /* Read the device descriptor (see USB specs for details) */
  /* ----------------------------------------------------- */

  /* Getting this descriptor is optional if you already know which reader is attached to your hardware. */
  wRecvLength = sizeof(abRecvBuffer);
  rc = scardGetReaderDescriptor(0x01, 0x00, abRecvBuffer, &wRecvLength);
  if (rc != _SCARD_S_SUCCESS)
  {
    printf("Failed to read the device descriptor (rc=%04X)\n", rc);
    goto restart_reader;
  }

  /* Read the configuration descriptor (see USB + CCID specs for details) */
  /* -------------------------------------------------------------------- */

  /* Getting this descriptor is also optional if you already know which reader is attached to your hardware.  */
  /* If your hardware may accomodate different kind of readers, it could be a good idea to parse the device's */
  /* descriptor in order to know how many slots it actually supports.                                         */
  wRecvLength = sizeof(abRecvBuffer);
  rc = scardGetReaderDescriptor(0x02, 0x00, abRecvBuffer, &wRecvLength);
  if (rc != _SCARD_S_SUCCESS)
  {
    printf("Failed to read the configuration descriptor (rc=%04X)\n", rc);
    goto restart_reader;
  }

  /* Red LED is OFF */
  rc = scardControl(CTRL_LED_RED_ON, sizeof(CTRL_LED_RED_ON), NULL, NULL);
  if (rc != _SCARD_S_SUCCESS)
  {
    printf("Failed to control the LEDs (rc=%04X)\n", rc);
    goto restart_reader;
  }

  /* Define the protocol(s) we want to look for */
  rc = scardControl(CTRL_SET_PROTOCOLS, sizeof(CTRL_SET_PROTOCOLS), NULL, NULL);
  if (rc != _SCARD_S_SUCCESS)
  {
    printf("Failed to control the LEDs (rc=%04X)\n", rc);
    goto restart_reader;
  }

  /* Read the vendor name, product name, serial number (see USB specs for details) */
  /* ----------------------------------------------------------------------------- */

  /* Getting the Vendor Name is also fully optional, don't do it unless you have a good reason */
  wRecvLength = sizeof(abRecvBuffer);
  rc = scardGetReaderDescriptor(0x03, 0x01, abRecvBuffer, &wRecvLength);
  if (rc != _SCARD_S_SUCCESS)
  {
    printf("Failed to read the reader's Vendor Name (rc=%04X)\n", rc);
    goto restart_reader;
  }

  /* Show the Vendor Name */
  printf("Vendor Name=");
  print_string_descriptor(abRecvBuffer, wRecvLength);
  printf("\n");

  /* Getting the Product Name is also fully optional, don't do it unless you have a good reason */
  wRecvLength = sizeof(abRecvBuffer);
  rc = scardGetReaderDescriptor(0x03, 0x02, abRecvBuffer, &wRecvLength);
  if (rc != _SCARD_S_SUCCESS)
  {
    printf("Failed to read the reader's Product Name (rc=%04X)\n", rc);
    goto restart_reader;
  }

  /* Show the Product Name */
  printf("Product Name=");
  print_string_descriptor(abRecvBuffer, wRecvLength);
  printf("\n");

  /* Getting the Serial Number is also fully optional, don't do it unless you have a good reason */
  wRecvLength = sizeof(abRecvBuffer);
  rc = scardGetReaderDescriptor(0x03, 0x03, abRecvBuffer, &wRecvLength);
  if (rc != _SCARD_S_SUCCESS)
  {
    printf("Failed to read the reader's serial number (rc=%04X)\n", rc);
    goto restart_reader;
  }

  /* Show the Serial Number */
  printf("Serial Number=");
  print_string_descriptor(abRecvBuffer, wRecvLength);
  printf("\n");

  /*
   * At this step we know that we have a reader there, we must activate it
   * ---------------------------------------------------------------------
   */

  rc = scardStartReader();
  if (rc != _SCARD_S_SUCCESS)
  {
    printf("Failed to start the reader (rc=%04X)\n", rc);
    goto restart_reader;
  }

  printf("\nReader started\n");

  /*
   * The reader is working, let's wait for a contactless card
   * --------------------------------------------------------
   */

wait_for_a_card:

  /* All LEDs are OFF */
  rc = scardControl(CTRL_LEDS_OFF, sizeof(CTRL_LEDS_OFF), NULL, NULL);
  if (rc != _SCARD_S_SUCCESS)
  {
    printf("Failed to control the LEDs (rc=%04X)\n", rc);
    goto restart_reader;
  }

  /* Connect to the card (don't worry if there's no card, it will fail of course...) */
  wRecvLength = sizeof(abRecvBuffer);
  rc = scardConnect(CONTACTLESS_SLOT, abRecvBuffer, &wRecvLength);
  if (rc != _SCARD_S_SUCCESS)
  {
    if (scard_IsReaderFatalError(rc))
    {
      printf("Failed to connect to the card (rc=%04X)\n", rc);
      goto restart_reader;
    }
    goto wait_for_a_card;
  }

  /* We've got a contactless card. The ATR tells us which kind of card it is. See PC/SC v2 chapter 3 for details */
  /* ----------------------------------------------------------------------------------------------------------- */

  printf("Card found, ATR=");
  for (i=0; i<wRecvLength; i++)
    printf("%02X", abRecvBuffer[i]);
  printf("\n");

  /* Green LED starts blinking */
  rc = scardControl(CTRL_LED_GREEN_BLINKS, sizeof(CTRL_LED_GREEN_BLINKS), NULL, NULL);
  if (rc != _SCARD_S_SUCCESS)
  {
    printf("Failed to control the LEDs (rc=%04X)\n", rc);
    goto restart_reader;
  }

  /* Sometimes it is useful to know the card's protocol identifier (UID in 14443-A or 15693, PUPI in 14443-B) */
  /* -------------------------------------------------------------------------------------------------------- */

  /* We use the GET DATA ( GET UID ) command APDU defined by PC/SC v2 chapter 3 for contactless cards */
  wRecvLength = sizeof(abRecvBuffer);
  rc = scardTransmit(CONTACTLESS_SLOT, APDU_GET_UID, sizeof(APDU_GET_UID), abRecvBuffer, &wRecvLength);
  if (rc != _SCARD_S_SUCCESS)
  {
    printf("Failed to retrieve the UID (rc=%04X)\n", rc);
    if (scard_IsReaderFatalError(rc))
      goto restart_reader;
    goto forget_the_card;
  }
  
  /* Every response APDU is terminated by a Status Word. SW=9000 means 'OK' */
  sw = SW(abRecvBuffer, wRecvLength);
  if (sw != 0x9000)
  {
    printf("Failed to retrieve the UID (SW=%04X)\n", sw);
  } else
  {
    printf("UID=");
    for (i=0; i<wRecvLength-2; i++)
      printf("%02X", abRecvBuffer[i]);
    printf("\n");
  }

  /* Memory cards hold data. We try to read 32B of data at the beginning of the card's memory mapping */
  /* ------------------------------------------------------------------------------------------------ */

  /* We use the READ BINARY command APDU defined by PC/SC v2 chapter 3 for memory cards */
  wRecvLength = sizeof(abRecvBuffer);
  rc = scardTransmit(CONTACTLESS_SLOT, APDU_READ_20, sizeof(APDU_READ_20), abRecvBuffer, &wRecvLength);
  if (rc != _SCARD_S_SUCCESS)
  {
    printf("Failed to read the Data (rc=%04X)\n", rc);
    if (scard_IsReaderFatalError(rc))
      goto restart_reader;
    goto forget_the_card;
  }

  /* Every response APDU is terminated by a Status Word. SW=9000 means 'OK' */
  sw = SW(abRecvBuffer, wRecvLength);
  if (sw != 0x9000)
  {
    printf("Failed to read the Data (SW=%04X)\n", sw);
    fDataRead = FALSE;
  } else
  {
    printf("Data=");
    for (i=0; i<wRecvLength-2; i++)
      printf("%02X", abRecvBuffer[i]);
    printf("\n");
    fDataRead = TRUE;
  }

forget_the_card:

  /* We're done with the card, let's disconnect */
  /* ------------------------------------------ */

  /* NB: this does virtually nothing on a contactless memory card, but for a contactless smartcard this is a DESELECT */
  rc = scardDisconnect(CONTACTLESS_SLOT);
  if (rc != _SCARD_S_SUCCESS)
  {
    printf("Failed to disconnect from the card (rc=%04X)\n", rc);
    if (scard_IsReaderFatalError(rc))
      goto restart_reader;
  }

  if (fDataRead)
  {
    /* Green LED is ON */
    rc = scardControl(CTRL_LED_GREEN_ON, sizeof(CTRL_LED_GREEN_ON), NULL, NULL);
    if (rc != _SCARD_S_SUCCESS)
    {
      printf("Failed to control the LEDs (rc=%04X)\n", rc);
      goto restart_reader;
    }
  } else
  {
    /* Red LED is ON */
    rc = scardControl(CTRL_LED_RED_ON, sizeof(CTRL_LED_RED_ON), NULL, NULL);
    if (rc != _SCARD_S_SUCCESS)
    {
      printf("Failed to control the LEDs (rc=%04X)\n", rc);
      goto restart_reader;
    }
  }

wait_for_card_removal:

  /* We could avoid processing the same card again if we wait for its departure */
  /* -------------------------------------------------------------------------- */
  rc = scardStatus(CONTACTLESS_SLOT, &fCardPresent, NULL);
  if (rc != _SCARD_S_SUCCESS)
  {
    printf("Failed to get the card's status (rc=%04X)\n", rc);
    if (scard_IsReaderFatalError(rc))
      goto restart_reader;
    goto wait_for_a_card;
  }

  if (fCardPresent)
    goto wait_for_card_removal;

  printf("The card has been removed\n");

  goto wait_for_a_card;

failed:

  /* There's a fatal error here... */
  /* ----------------------------- */

  printf("A fatal error has occured, exiting...\n");
  exit(EXIT_FAILURE);
}
