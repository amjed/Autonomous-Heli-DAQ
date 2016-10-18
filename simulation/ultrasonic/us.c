#include <12F675.h>
#include "stdlib.h"

#FUSES NOWDT                    //No Watch Dog Timer
#FUSES NOMCLR                   //Master Clear pin used for I/O
#FUSES NOPROTECT                //Code not protected from reading

#use delay(clock=4000000)
#use rs232(baud=19200,parity=N,xmit=PIN_A0,rcv=PIN_A1,bits=8)


unsigned int16 num;
unsigned char num_ascii[4];

void main()
{

   setup_timer_0(RTCC_INTERNAL|RTCC_DIV_1);
   while(1)
   {
      num = rand()%765;
      itoa(num,10,num_ascii);
      printf("R%s\r\n",num_ascii);
      delay_ms(100);
   }
   // TODO: USER CODE!!

}
