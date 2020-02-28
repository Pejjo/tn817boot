/**
 * \file
 *
 *
 (c) 2018 Microchip Technology Inc. and its subsidiaries.

    Subject to your compliance with these terms,you may use this software and
    any derivatives exclusively with Microchip products.It is your responsibility
    to comply with third party license terms applicable to your use of third party
    software (including open source software) that may accompany Microchip software.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIPs TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 */

#ifndef DRIVER_WRAPPER_H
#define DRIVER_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

/* definitions for SPM control */
//#define PAGESIZE SPM_PAGESIZE
//#define	LARGE_MEMORY

// Size of bootloader section. Equals first address in app section.
#define BOOTSIZE (256 * 4)

/* definitions for device recognition */
#define PARTCODE
#define SIGNATURE_BYTE_1 SIGNATURE_0
#define SIGNATURE_BYTE_2 SIGNATURE_1
#define SIGNATURE_BYTE_3 SIGNATURE_2

#ifdef LARGE_MEMORY
typedef uint32_t native_pointer_t;
#else
typedef uint16_t native_pointer_t;
#endif

void call_application(void);
bool hw_request(void);
void reset();

#ifdef __cplusplus
}
#endif

#endif /* DRIVER_WRAPPER_H*/
