#include "htc.h"
#include "configBits.h"




unsigned int ADCread(unsigned char channel){
    // Select A2D channel to read
    ADCON0 = ((channel << 2)); // Shift signal into appropriate data registers
    __delay_ms(2);
    ADON = 1;
    ADCON0bits.GO = 1;
    while(ADCON0bits.GO_NOT_DONE){__delay_ms(2);}
    return ((ADRESH<<8)|ADRESL);
}
