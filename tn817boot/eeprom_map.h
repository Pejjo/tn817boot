#ifndef EEPROM_MAP_INCLUDE
#define EEPROM_MAP_INCLUDE

#include "avr/io.h"

#define EEAD_CRIS USER_SIGNATURES_START-EEPROM_START+0 // This ends up in -100
#define EEAD_SKLD USER_SIGNATURES_START-EEPROM_START+1 // This ends up in -99

#endif
