#include "gps.h"


void main()
{

   setup_timer_0(RTCC_INTERNAL|RTCC_DIV_1);
   while(1)
   {
      printf("$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A\r\n");
      printf("$GPGLL,4916.45,N,12311.12,W,225444,A,*1D\r\n");
      printf("$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47\r\n");
      printf("$GPVTG,060.0,T,048.7,M,004.0,N,008.0,K*43\r\n");

      delay_ms(100);
   }
   // TODO: USER CODE!!

}
