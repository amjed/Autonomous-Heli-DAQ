#include <10F202.h>

#FUSES NOWDT                    //No Watch Dog Timer
#FUSES NOMCLR                   //Master Clear pin used for I/O
#FUSES NOPROTECT                //Code not protected from reading

#use delay(clock=4000000)
#use rs232(baud=19200,parity=N,xmit=PIN_B0,rcv=PIN_B1,bits=8)

