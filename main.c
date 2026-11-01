/**
  ******************************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    2013-xx-xx
  * @brief   I2C EEPROM(AT24C02)测试，测试信息通过USART1打印在电脑的超级终端
  ******************************************************************************
  * @attention
  *
  * 实验平台:野火 F103-霸道 STM32 开发板 
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :https://fire-stm32.taobao.com
  *
  ******************************************************************************
  */
  
#include "stm32f10x.h"
#include "./led/bsp_led.h"
#include "./usart/bsp_usart.h"
#include "./i2c/bsp_i2c_ee.h"
#include "./delay/delay.h"
#include "./LCD/lcd.h"

#define  EEP_Firstpage      0x00
uint8_t I2c_Buf_Write[10]={0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A};
uint8_t I2c_Buf_Read[10];
/**
  * @brief  主函数
  * @param  无
  * @retval 无
  */
int main(void)
{
  uint8_t i;
  uint8_t readFailed;

  LED_GPIO_Config();
  LED2_ON;
  LED3_ON;
  /* 串口初始化 */
  USART_Config();
  delay_init();

  /* I2C 外设初始化；A0、A1、A2 接地時，AT24C02 位址為 0xA0 */
  I2C_EE_Init();
  printf("I2C1 idle pins: PB6/SCL=%d, PB7/SDA=%d\r\n",(GPIOB->IDR & GPIO_Pin_6) ? 1 : 0,
         (GPIOB->IDR & GPIO_Pin_7) ? 1 : 0);

  /* 逐 byte 寫入 EEPROM，每 byte 寫入後等待 EEPROM 完成內部寫入 */
  for (i = 0; i < sizeof(I2c_Buf_Write); i++)
  {
    if (I2C_EE_ByteWrite(&I2c_Buf_Write[i], EEP_Firstpage + i) != 1)
    {
      printf("AT24C02 write failed at address 0x%02X\r\n",(unsigned int)(EEP_Firstpage + i));
      while (1);
    }
    if (I2C_EE_WaitEepromStandbyState() != 1)
    {
      printf("AT24C02 write cycle timeout at address 0x%02X\r\n",
             (unsigned int)(EEP_Firstpage + i));
      while (1);
    }
  }

  if (I2C_EE_BufferRead(&I2c_Buf_Read[0],EEP_Firstpage, 1) == 1)
  {
    printf("EEPROM read before LCD: 0x%02X\r\n",(unsigned int)I2c_Buf_Read[0]);
  }
  else
  {
    printf("EEPROM read before LCD failed at address 0x%02X\r\n", (unsigned int)EEP_Firstpage);
    while (1);
  }

  /* 先完成 EEPROM 首次寫入，再初始化使用 FSMC 的 LCD。 */

  /* LCD/FSMC 初始化後重設 I2C1 並重新配置 EEPROM 匯流排。 */
  I2C_EE_Init();

  while (1)
  {
    readFailed = 0;
    printf("@@AT24C02: ");
    for (i = 0; i < sizeof(I2c_Buf_Read); i++)
    {
      if (I2C_EE_BufferRead(&I2c_Buf_Read[i], EEP_Firstpage + i, 1) != 1)
      {
        printf("\r\n AT24C02 read failed at address 0x%02X\r\n",(unsigned int)(EEP_Firstpage + i));
        readFailed = 1;
        break;
      }

      printf("0x%02X ", (unsigned int)I2c_Buf_Read[i]);
    }
    if (readFailed == 0)
    {
      printf("\r\n");
    }
      // delay_ms(100);
  }
}

/**
  * @brief  I2C(AT24C02)读写测试
  * @param  无
  * @retval 正常返回1，异常返回0
  */
uint8_t I2C_Test(void)
{
	uint16_t i;

	//printf("写入的数据\n\r");
    
	for ( i=0; i<sizeof(I2c_Buf_Write); i++ ) //填充缓冲
  {   
    I2c_Buf_Write[i] = i;

   // printf("0x%02X ", I2c_Buf_Write[i]);
    if(i%16 == 15)    
        printf("\n\r");    
   }

  //将I2c_Buf_Write中顺序递增的数据写入EERPOM中 
	if (I2C_EE_BufferWrite(I2c_Buf_Write, EEP_Firstpage,
	                       sizeof(I2c_Buf_Write)) != 1)
	{
		return 0;
	}
//  EEPROM_INFO("\n\r写成功\n\r");
  // EEPROM_INFO("\n\r读出的数据\n\r");
  //将EEPROM读出数据顺序保持到I2c_Buf_Read中
	if (I2C_EE_BufferRead(I2c_Buf_Read, EEP_Firstpage,
	                      sizeof(I2c_Buf_Read)) != 1)
	{
		return 0;
	}
   
  //将I2c_Buf_Read中的数据通过串口打印
	for (i=0; i<sizeof(I2c_Buf_Read); i++)
	{	
		if(I2c_Buf_Read[i] != I2c_Buf_Write[i])
		{
			//EEPROM_ERROR("0x%02X ", I2c_Buf_Read[i]);
			//EEPROM_ERROR("错误:I2C EEPROM写入与读出的数据不一致\n\r");
			return 0;
		}
    printf("0x%02X ", I2c_Buf_Read[i]);
    if(i%16 == 15)    
        printf("\n\r");
    
	}
 // EEPROM_INFO("I2C(AT24C02)读写测试成功\n\r");
  
  return 1;
}
/*********************************************END OF FILE**********************/
