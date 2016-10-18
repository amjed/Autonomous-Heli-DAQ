/***************** (C) COPYRIGHT 2009 VectorNav Technologies *******************
* File Name          : VN100.c
* Author             : John Brashear
* Version            : V1.0.0
* Date               : 09/26/2009
* Description        : This file provides all of the firmware functions specific
*                    : to the VN100.
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING
* CUSTOMERS WITH EXAMPLE CODE IN ORDER TO SAVE THEM TIME. AS A RESULT,
* VECTORNAV SHALL NOT BE HELD LIABLE FOR ANY DIRECT, INDIRECT OR 
* CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE 
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE 
* CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "../inc/VN100.h"
#include "../inc/VN_lib.h"

#ifdef _VN100
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Buffer used for SPI read and write responses */
/* Both the read and write register SPI routines below use this packet 
   to store the returned SPI response. None of the write register commands 
   implemented in this library check the data that is returned by the sensor 
   to ensure that it is consistent with the data that was sent.  For normal
   cases this isn't necessary however if you wish to implement your own
   checking then this is the structure that you need to check after each 
   register set command.  The structure has the following form:
   VN_SPI_LastReceivedPacket.CmdID -> This is the ID for the command that
                                   the response is for
   VN_SPI_LastReceivedPacket.RegID -> This is the ID for the register that
                                   the response is for
   VN_SPI_LastReceivedPacket.Data[] -> This is the data that was returned by
                                    the sensor as an array of unsigned 32-bit
                                    integers  */
VN100_SPI_Packet VN_SPI_LastReceivedPacket = {0, 0, 0, 0, {0}};

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : VN100_SPI_ReadRegister(unsigned char sensorID, unsigned char regID, unsigned char regWidth)
* Description    : Read the register with the ID regID on a VN-100 sensor
*                  using the SPI interface.                                     
* Input          : sensorID -> The sensor to get the requested data from.
*                : regID -> The requested register ID number
*                : regWidth -> The width of the requested register in 32-bit words
* Output         : None
* Return         : Pointer to SPI packet returned by the sensor
*******************************************************************************/
VN100_SPI_Packet* VN100_SPI_ReadRegister(unsigned char sensorID, unsigned char regID, unsigned char regWidth){

  unsigned long i;

  /* Pull SS line low to start transaction*/
  VN_SPI_SetSS(sensorID, VN_PIN_LOW);

  /* Send request */
  VN_SPI_SendReceive(VN_BYTES2WORD(0, 0, regID, VN100_CmdID_ReadRegister));
  VN_SPI_SendReceive(0);

  /* Pull SS line high to end SPI transaction */
  VN_SPI_SetSS(sensorID, VN_PIN_HIGH);

  /* Delay for 50us */
  VN_Delay(100);

  /* Pull SS line low to start SPI transaction */
  VN_SPI_SetSS(sensorID, VN_PIN_LOW);

  /* Get response over SPI */
  for(i=0;i<=regWidth;i++){
    *(((unsigned long*)&VN_SPI_LastReceivedPacket) + i) = VN_SPI_SendReceive(0);
  }

  /* Pull SS line high to end SPI transaction */
  VN_SPI_SetSS(sensorID, VN_PIN_HIGH);  


  /* Return Error code */
  return &VN_SPI_LastReceivedPacket;  
}

/*******************************************************************************
* Function Name  : VN100_SPI_WriteRegister(unsigned char sensorID, unsigned char regID, unsigned char regWidth, unsigned long* ptrWriteValues)
* Description    : Write to the register with the ID regID on VN-100 sensor
*                  using the SPI interface.                                        
* Input          : sensorID -> The sensor to write the requested data to.
*                : regID -> The register ID number
*                : regWidth -> The width of the register in 32-bit words
* Output         : ptrWriteValues -> The data to write to the requested register.
* Return         : Pointer to SPI packet returned by the sensor
*******************************************************************************/
VN100_SPI_Packet* VN100_SPI_WriteRegister(unsigned char sensorID, unsigned char regID, unsigned char regWidth, unsigned long* ptrWriteValues){

  unsigned long i;

  /* Pull SS line low to start transaction*/
  VN_SPI_SetSS(sensorID, VN_PIN_LOW);

  /* Send write command */
  VN_SPI_SendReceive(VN_BYTES2WORD(0, 0, regID, VN100_CmdID_WriteRegister));
  for(i=0;i<regWidth;i++){
    VN_SPI_SendReceive(ptrWriteValues[i]);
  }

  /* Pull SS line high to end SPI transaction */
  VN_SPI_SetSS(sensorID, VN_PIN_HIGH);

  /* Delay for 50us */
  VN_Delay(100);

  /* Pull SS line low to start SPI transaction */
  VN_SPI_SetSS(sensorID, VN_PIN_LOW);

  /* Get response over SPI */
  for(i=0;i<4;i++){
    *(((unsigned long*)&VN_SPI_LastReceivedPacket) + i) = VN_SPI_SendReceive(0);
  }

  /* Pull SS line high to end SPI transaction */
  VN_SPI_SetSS(sensorID, VN_PIN_HIGH);  


  /* Return pointer to SPI packet */
  return &VN_SPI_LastReceivedPacket;
}


/*******************************************************************************
* Function Name  : VN100_SPI_GetYPRMagAccRates(unsigned char sensorID, float* YPR, float* mag, float* acc, float* rates)
* Description    : Get the yaw, pitch, roll, magnetic, acceleration, and angular rates.                                        
* Input          : sensorID -> The sensor to get the requested data from.
* Output         : YPR -> Euler angles (Yaw, Pitch, Roll) in deg.
*                  mag -> The magnetic measured vector (3x1).
*                  acc -> Measured acceleration (3x1) in m/s^2.
*                  rates -> Measured angular rates (3x1) in rad/s.
* Return         : Pointer to SPI packet returned by the sensor
*******************************************************************************/
VN100_SPI_Packet* VN100_SPI_GetYPRMagAccRates(unsigned char sensorID, float* YPR, float* mag, float* acc, float* rates){

  unsigned long i;
  
  /* Read register */
  VN100_SPI_ReadRegister(sensorID, VN100_REG_YMR, 12);
  
  /* Get Euler angles */
  for(i=0;i<3;i++){
    YPR[i] = VN_SPI_LastReceivedPacket.Data[i].Float;
  }
  
  /* Get Magnetic */
  for(i=0;i<3;i++){
    mag[i] = VN_SPI_LastReceivedPacket.Data[i+3].Float;
  }  
  
  /* Get Acceleration */
  for(i=0;i<3;i++){
    acc[i] = VN_SPI_LastReceivedPacket.Data[i+6].Float;
  }
  
  /* Get Angular Rates */
  for(i=0;i<3;i++){
    rates[i] = VN_SPI_LastReceivedPacket.Data[i+9].Float;
  }
    
  /* Return pointer to SPI packet */
  return &VN_SPI_LastReceivedPacket;
}

#endif /* _VN100 */

/******************* (C) COPYRIGHT 2009 VectorNav Technologies *****************
***********************************END OF FILE*********************************/
