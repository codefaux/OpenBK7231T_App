/**
  ******************************************************************************
  * @file    vkl060.h 
  * @author  kevin_guo
  * @version V1.2
  * @date    05-17-2020
  * @brief   Header files for Vkl060
  ******************************************************************************
  * @attention
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __VKl060_H
#define __VKl060_H



// Temporary hardcodes
#define Vkl060_Pin_SCL		28
#define Vkl060_Pin_SDA		9





/* Includes ------------------------------------------------------------------*/
//�˰����ļ����ݿͻ���Ƭ������Ӧ���޸�
// #include "M451Series.h"
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
//��������
#define Vkl060_ADDR   					0x7c    // IIC��ַ
//#define Vkl060_ICSET       			0xe8    // IC����  bit0ʱ��Դ  bit1����λ      bit2ӳ���ַbit5
//																				// Ĭ��Ϊ0 �ڲ�RC����  ��ִ��������λ  
#define Vkl060_SOFTRST       		0xea    //ִ��������λ   
#define Vkl060_ADDR5_0       		0xe8    //ӳ���ַbit5 
#define Vkl060_ADDR5_1       		0xec    //ӳ���ַbit5 
#define Vkl060_ADSET       			0x00    //ӳ���ַbit4-0 
#define Vkl060_MODESET_1_3_ON  	0xc8    // ����ʾ 1/3bias
#define Vkl060_MODESET_1_3_OFF 	0xc0    // �ر���ʾ 1/3bias
#define Vkl060_MODESET_1_2_ON  	0xcc    // ����ʾ 1/2bias
#define Vkl060_MODESET_1_2_OFF 	0xc4    // �ر���ʾ 1/2bias
//��ʾ����
#define Vkl060_DISCTL			      0xA0  	//bit7~bit5=101 
//bit4-bit3ʡ��ģʽFR bit2LINE,FRAME�������� bit1-bit0ʡ��ģʽSR
//����Inor=1  Im1=0.5Inor  Im2=0.67Inor  Ihp=1.8Inor
//ʡ��ģʽFR
#define Vkl060_FRNOR       0x00  //bit4-bit3=00   FR NORMAL 					�ϵ�Ĭ�� 
#define Vkl060_FRPM1       0x08  //bit4-bit3=01   FR POWER SAVE MODE1
#define Vkl060_FRPM2       0x10  //bit4-bit3=10   FR POWER SAVE MODE2
#define Vkl060_FRPM3       0x18  //bit4-bit3=11   FR POWER SAVE MODE3 ��ʡ��
//ʡ��ģʽSR
#define Vkl060_SRHP        0x03  //bit1-bit0=11   SR NORMAL 
#define Vkl060_SRNOR       0x02  //bit1-bit0=10   SR POWER SAVE MODE1 �ϵ�Ĭ�� 
#define Vkl060_SRPM2       0x01  //bit1-bit0=01   SR POWER SAVE MODE2
#define Vkl060_SRPM1       0x00  //bit1-bit0=00   SR POWER SAVE MODE1 ��ʡ��
//LINE FRAME��������
#define Vkl060_LINER       0x00  //bit2=0   LINE��ת	�ϵ�Ĭ��
#define Vkl060_FRAMER      0x04  //bit2=1   FRAME��ת ʡ��
//������ʾ����
//Vkl060_FRPM1|Vkl060_SRPM1|Vkl060_FRAMER    //����
//Vkl060_FRPM2|Vkl060_SRPM1|Vkl060_FRAMER    //
//Vkl060_FRPM3|Vkl060_SRPM1|Vkl060_FRAMER    //������ʡ
//Vkl060_FRNOR|Vkl060_SRHP|Vkl060_LINER      //�������
//��˸����
#define Vkl060_BLKCTL_OFF      0xF0  // �ر���˸
#define Vkl060_BLKCTL_05HZ     0xF1  // ��˸Ƶ��Ϊ0.5HZ
#define Vkl060_BLKCTL_1HZ      0xF2  // ��˸Ƶ��Ϊ1HZ
#define Vkl060_BLKCTL_2HZ      0xF3  // ��˸Ƶ��Ϊ2HZ
//ȫ��ǿ�п����,����ʾ�ڴ������޹�,����������ȫ����Ϊ����ִ��
#define Vkl060_APCTL_APON_ON   0xFE // ȫ��ǿ��ȫ��ʾ_��
#define Vkl060_APCTL_APON_OFF  0xFC // ȫ��ǿ��ȫ��ʾ_��
#define Vkl060_APCTL_APOFF_ON  0xFD // ȫ��ǿ�й���ʾ_��
#define Vkl060_APCTL_APOFF_OFF 0xFC // ȫ��ǿ�й���ʾ_��

//����seg��
#define 	Vkl060_SEGNUM				15

// //���¹ܽ����ø��ݿͻ���Ƭ������Ӧ���޸� 
// #define Vkl060_SCL_PIN    BIT13
// #define Vkl060_SCL_PORT   PA
// #define Vkl060_SCL_IO     PA13

// #define Vkl060_SDA_PIN    BIT15
// #define Vkl060_SDA_PORT   PA
// #define Vkl060_SDA_IO     PA15

// //���¹ܽ����������ݿͻ���Ƭ������Ӧ���޸�
// #define Vkl060_SCL_H() 				Vkl060_SCL_IO = 1 
// #define Vkl060_SCL_L() 				Vkl060_SCL_IO = 0 

// #define Vkl060_SCL_H() 				Vkl060_SCL_IO = 1 
// #define Vkl060_SCL_L() 				Vkl060_SCL_IO = 0 

// #define Vkl060_SDA_H() 				Vkl060_SDA_IO = 1 
// #define Vkl060_SDA_L() 				Vkl060_SDA_IO = 0 

// #define Vkl060_GET_SDA() 			Vkl060_SDA_IO
// //��Ƭ��SDA��Ϊ����IOҪ��Ϊ�����
// //���ݿͻ���Ƭ���޸�
// #define Vkl060_DIRDAT_PIN   	BIT0
// #define Vkl060_DIRDAT_PORT  	PB
// #define Vkl060_DIRDAT_IO    	PB0
// #define Vkl060_DIRDAT1_PIN   	BIT12
// #define Vkl060_DIRDAT1_PORT  	PA
// #define Vkl060_DIRDAT1_IO    	PA12
// #define Vkl060_SET_SDA_DIR() 	{\
// 	GPIO_SetMode(Vkl060_DIRDAT_PORT, Vkl060_DIRDAT_PIN, GPIO_MODE_OUTPUT);\
// 	GPIO_SetMode(Vkl060_DIRDAT1_PORT, Vkl060_DIRDAT1_PIN, GPIO_MODE_OUTPUT);\
// }
// #define Vkl060_SET_SDA_IN() 	{\
//   Vkl060_DIRDAT_IO = 0;\
//   Vkl060_DIRDAT1_IO = 0;\
// }
// #define Vkl060_SET_SDA_OUT() 	{\
//  	Vkl060_DIRDAT_IO = 1;\
//   Vkl060_DIRDAT1_IO = 1;\
// }
// /* Exported functions ------------------------------------------------------- */
// void Vkl060_Init(void);
// void Vkl060_DisAll(unsigned char dat);
// void Vkl060_DisSegComOn(unsigned char seg,unsigned char com);
// void Vkl060_DisSegComOff(unsigned char seg,unsigned char com);

// void Vkl060_Main(void);
#endif  /*__VKl060_H*/

/************************END OF FILE****/
