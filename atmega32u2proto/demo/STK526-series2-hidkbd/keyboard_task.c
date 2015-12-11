/*This file is prepared for Doxygen automatic documentation generation.*/
//! \file *********************************************************************
//!
//! \brief This file manages the keyboard task.
//!
//! - Compiler:           IAR EWAVR and GNU GCC for AVR
//! - Supported devices:  AT90USB162, AT90USB82
//!
//! \author               Atmel Corporation: http://www.atmel.com \n
//!                       Support and FAQ: http://support.atmel.no/
//!
//! ***************************************************************************

/* Copyright (c) 2009 Atmel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an Atmel
 * AVR product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY AND
 * SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

//_____  I N C L U D E S ___________________________________________________

#include "config.h"
#include "conf_usb.h"
#include "keyboard_task.h"
#include "lib_mcu/usb/usb_drv.h"
#include "usb_descriptors.h"
#include "modules/usb/device_chap9/usb_standard_request.h"
#include "lib_mcu/power/power_drv.h"


//_____ D E F I N I T I O N S ______________________________________________
//keybinds here
/*#define key_weaponsincrease		HID_KEYPAD_1
#define key_shieldsincrease		HID_KEYPAD_2
#define key_enginesincrease		HID_KEYPAD_3

#define key_shieldleft			HID_KEYPAD_4
#define key_shieldright			HID_KEYPAD_6
#define key_shieldfront			HID_KEYPAD_8
#define key_shieldrear			HID_KEYPAD_5
#define key_shieldup			HID_KEYPAD_7
#define key_shielddown			HID_KEYPAD_9
*/



#define key_weaponsincrease		HID_1
#define key_shieldsincrease		HID_2
#define key_enginesincrease		HID_3

#define key_shieldleft			HID_4
#define key_shieldright			HID_5
#define key_shieldfront			HID_6
#define key_shieldrear			HID_7
#define key_shieldup			HID_8
#define key_shielddown			HID_9


//const U8  code    usb_keys[]= {HID_M,};
const U8  code    usb_weaponsincrease[]= {key_weaponsincrease,};
const U8  code    usb_shieldsincrease[]= {key_shieldsincrease,};
const U8  code    usb_enginesincrease[]= {key_enginesincrease,};

const U8  code    usb_shieldleft[]= {key_shieldleft,};
const U8  code    usb_shieldright[]= {key_shieldright,};
const U8  code    usb_shieldfront[]= {key_shieldfront,};
const U8  code    usb_shieldrear[]= {key_shieldrear,};
const U8  code    usb_shieldup[]= {key_shieldup,};
const U8  code    usb_shielddown[]= {key_shielddown,};



//#define SIZEOF_USB_KEYS     (Uint16)sizeof(usb_keys)


//_____ D E C L A R A T I O N S ____________________________________________

volatile U8    cpt_sof;
         U8    transmit_no_key;
volatile bit   key_hit;
         U8    usb_key;
         U8    usb_kbd_state;
         U16   usb_data_to_send;
#ifdef __GNUC__
PGM_VOID_P     usb_key_pointer;
#else
U8   code *    usb_key_pointer;
#endif


//! This function initializes the hardware/software ressources required for keyboard task.
//!
void keyboard_task_init(void)
{
   transmit_no_key   = FALSE;
   key_hit           = FALSE;
   usb_kbd_state     = 0;
   Power_Button_init();
   Shield_Button_init();   
   cpt_sof           = 0;
   Usb_enable_sof_interrupt();
}


//! @brief Entry point of the mouse management
//! This function links the mouse and the USB bus.
//!
void keyboard_task(void)
{
   if(Is_device_enumerated())
   {
      // if USB ready to transmit new data :
      //        - if last time = 0, nothing
      //        - if key pressed -> transmit key
      //        - if !key pressed -> transmit 0
      if (key_hit==FALSE )
      {

         if (cpt_sof>= NB_IDLE_POLLING_SOF) 
         {
            kbd_test_hit();
            cpt_sof = 0;
         }
      }
      
      else
      {
         Usb_select_endpoint(EP_KBD_IN);   
         if(Is_usb_write_enabled())
         {
            if ( transmit_no_key==FALSE)
            {
               transmit_no_key = TRUE;
               Usb_write_byte(HID_MODIFIER_NONE);  // Byte0: Modifier
               Usb_write_byte(0);                  // Byte1: Reserved
               Usb_write_byte(usb_key);            // Byte2: Keycode 0
               Usb_write_byte(0);                  // Byte2: Keycode 1
               Usb_write_byte(0);                  // Byte2: Keycode 2
               Usb_write_byte(0);                  // Byte2: Keycode 3
               Usb_write_byte(0);                  // Byte2: Keycode 4
               Usb_write_byte(0);                  // Byte2: Keycode 5
               Usb_send_in();
               return;
            }
            else
            {
               key_hit = FALSE;
               transmit_no_key = FALSE;
               Usb_write_byte(0);
               Usb_write_byte(0);
               Usb_write_byte(0);
               Usb_write_byte(0);
               Usb_write_byte(0);
               Usb_write_byte(0);
               Usb_write_byte(0);
               Usb_write_byte(0);
               Usb_send_in();
            }
         }
      }
   }
}


//! @brief Check keyboard key hit
//! This function scans the keyboard keys and update the scan_key word.
//!   if a key is pressed, the key_hit bit is set to TRUE.
//!
void kbd_test_hit(void)
{
   switch (usb_kbd_state)
   {
	case 0:
		if (Is_btn_weaponsincrease())
		{
			usb_kbd_state = 1;
			usb_key_pointer = usb_weaponsincrease;
			usb_data_to_send = (Uint16)sizeof(usb_weaponsincrease);
		}
		else if (Is_btn_shieldsincrease())
		{
			usb_kbd_state = 1;
			usb_key_pointer = usb_shieldsincrease;
			usb_data_to_send = (Uint16)sizeof(usb_shieldsincrease);
		}    
	  
		else if (Is_btn_enginesincrease())
		{
			usb_kbd_state = 1;
			usb_key_pointer = usb_enginesincrease;
			usb_data_to_send = (Uint16)sizeof(usb_enginesincrease);
		}
	  
		else if (Is_btn_shieldleft())
		{
			usb_kbd_state = 1;
			usb_key_pointer = usb_shieldleft;
			usb_data_to_send = (Uint16)sizeof(usb_shieldleft);
		} 
		else if (Is_btn_shieldright())
		{
			usb_kbd_state = 1;
			usb_key_pointer = usb_shieldright;
			usb_data_to_send = (Uint16)sizeof(usb_shieldright);
		}
		else if (Is_btn_shieldfront())
		{
			usb_kbd_state = 1;
			usb_key_pointer = usb_shieldfront;
			usb_data_to_send = (Uint16)sizeof(usb_shieldfront);
		}
		else if (Is_btn_shieldrear())
		{
			usb_kbd_state = 1;
			usb_key_pointer = usb_shieldrear;
			usb_data_to_send = (Uint16)sizeof(usb_shieldrear);
		}
		else if (Is_btn_shieldup())
		{
			usb_kbd_state = 1;
			usb_key_pointer = usb_shieldup;
			usb_data_to_send = (Uint16)sizeof(usb_shieldup);
		}  
		else if (Is_btn_shielddown())
		{
			usb_kbd_state = 1;
			usb_key_pointer = usb_shielddown;
			usb_data_to_send = (Uint16)sizeof(usb_shielddown);
		}
      
      break;

      case 1:
      if (usb_data_to_send != 0)
      {
         if ((key_hit == FALSE) && (transmit_no_key == FALSE))
         {
#ifndef __GNUC__
            usb_key = *usb_key_pointer++;
#else
            usb_key = pgm_read_byte_near(usb_key_pointer++);
#endif
            usb_data_to_send --;
            key_hit = TRUE;
         }
      }
      else
      {
         usb_kbd_state = 0;
      }
      break;
   }
}


//! @brief vbus_off_action
//! This function increments the action to be executed upon
//! the USB VBUS disconnection
//! Here a Vbus lost lead to detach
//!
void vbus_off_action(void)
{
   Usb_detach();
}


void suspend_action(void)
{
#if (USB_REMOTE_WAKEUP_FEATURE == ENABLED)
   if (remote_wakeup_feature == ENABLED)
   {
      Switches_enable_it()
   }
#endif 
   Enable_interrupt();  
   Enter_power_down_mode();
}


#ifdef __GNUC__
 ISR(PCINT0_vect)
#else
#pragma vector = PCINT0_vect
__interrupt void mouse_disco_int()
#endif
{
   Switches_disable_it();
   usb_generate_remote_wakeup();
}


void sof_action(void)
{
   cpt_sof++;
}