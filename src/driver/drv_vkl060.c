// NOTE: based on https://github.com/RobTillaart/HT16K33.git MIT code
#include "../new_common.h"
#include "../new_pins.h"
#include "../new_cfg.h"
// Commands register, execution API and cmd tokenizer
#include "../cmnds/cmd_public.h"
#include "../mqtt/new_mqtt.h"
#include "../logging/logging.h"
#include "drv_local.h"
#include "drv_vkl060.h"
#include "../hal/hal_pins.h"





inline void _HAL_Pin_L(uint8_t pin) {
	HAL_PIN_Setup_Output(pin);
	HAL_PIN_SetOutputValue(pin, 0);
}




inline void _HAL_Pin_H(uint8_t pin) {
	HAL_PIN_Setup_Input_Pullup(pin);
}




#define Vkl060_SCL_L() 			_HAL_Pin_L(Vkl060_Pin_SCL)
#define Vkl060_SCL_H() 			_HAL_Pin_H(Vkl060_Pin_SCL)
#define Vkl060_SDA_L() 			_HAL_Pin_L(Vkl060_Pin_SCL)
#define Vkl060_SDA_H() 			_HAL_Pin_H(Vkl060_Pin_SCL)
#define Vkl060_SET_SDA_IN()		HAL_PIN_Setup_Input(Vkl060_Pin_SDA)
#define Vkl060_SET_SDA_OUT()	HAL_PIN_Setup_Output(Vkl060_Pin_SDA)
#define Vkl060_GET_SDA()		HAL_PIN_ReadDigitalInput(Vkl060_Pin_SDA)




/**
	******************************************************************************
	* @file    vkl060.c
	* @author  kevin_guo
	* @version V1.2
	* @date    05-17-2020
	* @brief   This file contains all the vkl060 functions. 
	*          ���ļ������� VKl060
	******************************************************************************
	* @attention
	******************************************************************************
	*/	
/* Includes ------------------------------------------------------------------*/
	
/* extern variables ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define VKl060_CLK 10 //SCL�ź���Ƶ��,��delay_nusʵ�� 50->10kHz 10->50kHz 5->100kHz
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
//segtab[]�����Ӧʵ�ʵ�оƬ��LCD���ߣ����߼�-VKl060�ο���·
unsigned char vkl060_segtab[Vkl060_SEGNUM]={
	11,12,13,14,15,16,17,18,19,		//SEG11-SEG19
	20,21,22,23,24,25,				//SEG20-SEG25
};
//����LCDʵ��ֻ����SEG11��SEG22ʹ����������
//.h�ļ�����VKl060_SEGNUM=12
//const unsigned char vkl060_segtab[VKl060_SEGNUM]={	
//	11,12,13,14,15,16,17,18,19,	//SEG11-SEG19
//	20,21,22										//SEG20-SEG22
//};
//vkl060_dispram��ӦоƬ����ʾRAM
unsigned char vkl060_dispram[Vkl060_SEGNUM/2];//ÿ���ֽ����ݶ�Ӧ2��SEG
//��Ӧ����vkl060_segtab[VKl060_SEGNUM]
//��ʾRAM bufferΪ8λ��Ӧ2��SEG��bit7/bit3->com3,bit6/bit2->com2,bit5/bit1->com1,bit4/bit0->com0
//LCD ʵ��  3��8��
//1�ǰ�λ��2��ʮλ��3�Ǹ�λ
/*��ʾbuffer��lcdͼ��ӳ���ϵ����
vkl060_dispram[VKl060_SEGNUM/2]=
{//com3   com2   com1     com0 //��ӦоƬSEG��  ��Ӧ��ʾRAM����
	1D,     1E,    1F,      1A,     //seg11   		vkl060_dispram[0]-bit3-bit0
	  ,     1C,    1G,      1B,     //seg12				vkl060_dispram[0]-bit7-bit4
	2D,     2E,    2F,      2A,     //seg13				vkl060_dispram[1]-bit3-bit0
	  ,     2C,    2G,      2B,     //seg14				vkl060_dispram[1]-bit7-bit4
	3D,     3E,    3F,      3A,		  //seg15				vkl060_dispram[2]-bit3-bit0
	  ,     3C,    3G,      3B,			//seg16				vkl060_dispram[2]-bit7-bit4
}
*/
/*8�ֵ��� 
		 A
	 F   B
		 G
	 E   C
	   D
*/
unsigned char shuzi_zimo[15]= //���ֺ��ַ���ģ
{
//0    1    2    3    4    5    6    7    8    9    -    L    o    H    i 
	0x5F,0x50,0x3D,0x79,0x72,0x6B,0x6F,0x51,0x7F,0x7B,0x20,0x0E,0x6C,0x76,0x50
};
unsigned char vkl060_segi,vkl060_comi;
unsigned char vkl060_maxcom;//������com��������4com
/* Private function prototypes -----------------------------------------------*/
unsigned char Vkl060_InitSequence(void);
/* Private function ----------------------------------------------------------*/
/*******************************************************************************
* Function Name  : delay_nus
* Description    : ��ʱ1uS����
* Input          : n->��ʱʱ��nuS
* Output         : None
* Return         : None
*******************************************************************************/
void delay_nus(unsigned int n)	   
{
	#ifdef WIN32
		// not possible on Windows port
	#else
		for (volatile int i = 0; i < n; i++)
			__asm__("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop");
	#endif

	// unsigned char i;
	// while(n--)
	// {
	// 	i=10;
	// 	while(i--)
	// 	{//nopָ����ݵ�Ƭ������Ӧ���޸�
	// 		__nop();
	// 	}
	// }
}
/*******************************************************************************
* Function Name  : delay_nms
* Description    : ��ʱ1mS����
* Input          : n->��ʱʱ��nmS
* Output         : None
* Return         : None
*******************************************************************************/
void delay_nms(unsigned long int n)
{
	while(n--)
	{
		delay_nus(1000);
	}
}

/*******************************************************************************
* Function Name  : I2CStart
* Description    : ʱ���߸�ʱ���������ɸߵ��͵����䣬��ʾI2C��ʼ�ź�
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Vkl060_I2CStart( void )
{
	Vkl060_SCL_H();
	Vkl060_SDA_H();
	delay_nus(VKl060_CLK);
	Vkl060_SDA_L();
	delay_nus(VKl060_CLK);
}
/*******************************************************************************
* Function Name  : I2CStop
* Description    : ʱ���߸�ʱ���������ɵ͵��ߵ����䣬��ʾI2Cֹͣ�ź�
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Vkl060_I2CStop( void )
{
	Vkl060_SCL_H();
	Vkl060_SDA_L();
	delay_nus(VKl060_CLK);
	Vkl060_SDA_H();
	delay_nus(VKl060_CLK);
}
/*******************************************************************************
* Function Name  : I2CSlaveAck
* Description    : I2C�ӻ��豸Ӧ���ѯ
* Input          : None
* Output         : None
* Return         : 0-ok 1-fail
*******************************************************************************/
unsigned char Vkl060_I2CSlaveAck( void )
{
	unsigned int TimeOut;
	unsigned char RetValue;
	
	Vkl060_SCL_L();
	//��Ƭ��SDA��Ϊ����IOҪ��Ϊ�����
	Vkl060_SET_SDA_IN();
	delay_nus(VKl060_CLK);
	Vkl060_SCL_H();//��9��sclk������

	TimeOut = 10000;
	while( TimeOut-- > 0 )
	{
		if( Vkl060_GET_SDA()!=0 )//��ȡack
		{
			RetValue = 1;
		}
		else
		{
			RetValue = 0;
			break;
		}
	} 
	Vkl060_SCL_L();
	//��Ƭ��SDA��Ϊ����IOҪ��Ϊ�����
	Vkl060_SET_SDA_OUT(); 
	
	return RetValue;
}
/*******************************************************************************
* Function Name  : I2CCmdByte
* Description    : I2Cдһ�ֽ�����,�������͸�λ
* Input          : byte-Ҫд�������
* Output         : None
* Return         : None
*******************************************************************************/
void Vkl060_I2CCmdByte( unsigned char byte )
{
	unsigned char i=8;
	while (i--)
	{ 
		Vkl060_SCL_L();
		if(byte&0x80)
			Vkl060_SDA_H();
		else
			Vkl060_SDA_L();
		byte<<=1; 
		delay_nus(VKl060_CLK);
		Vkl060_SCL_H();     
		delay_nus(VKl060_CLK);
	}
}
/*******************************************************************************
* Function Name  : I2CDatByte
* Description    : I2Cдһ�ֽ�����,�������͵�λ
* Input          : byte-Ҫд�������
* Output         : None
* Return         : None
*******************************************************************************/
void Vkl060_I2CDatByte( unsigned char byte )
{
	unsigned char i=8;
	while (i--)
	{ 
		Vkl060_SCL_L();
		if(byte&0x01)
			Vkl060_SDA_H();
		else
			Vkl060_SDA_L();
		byte>>=1; 
		delay_nus(VKl060_CLK);
		Vkl060_SCL_H();     
		delay_nus(VKl060_CLK);
	}
}
/*******************************************************************************
* Function Name  : Write1Data
* Description    : д1�ֽ����ݵ���ʾRAM
* Input          : Addr-д��ram�ĵ�ַ
*                : Dat->д��ram������
* Output         : None
* Return         : 0-ok 1-fail
*******************************************************************************/
unsigned char Write1DataVkl060(unsigned char Addr,unsigned char Dat)
{
	//START �ź�
	Vkl060_I2CStart(); 
	//SLAVE��ַ
	Vkl060_I2CCmdByte(Vkl060_ADDR); 
	if( 1 == Vkl060_I2CSlaveAck() )
	{		
		Vkl060_I2CStop();
		return 1; 
	}
	//��ʾRAM��ַ
	if(Addr>0x1f)
		Vkl060_I2CCmdByte(Vkl060_ADDR5_1); 
	else
		Vkl060_I2CCmdByte(Vkl060_ADDR5_0); 
	if( 1 == Vkl060_I2CSlaveAck() )
	{
		Vkl060_I2CStop();
		return 1;
	}
	Vkl060_I2CCmdByte(Addr&0x1f); 
	if( 1 == Vkl060_I2CSlaveAck() )
	{
		Vkl060_I2CStop();
		return 1;
	}
	//��ʾ���ݣ�1�ֽ����ݰ���2��SEG
	Vkl060_I2CDatByte(Dat);
	if( Vkl060_I2CSlaveAck()==1 )
	{
		Vkl060_I2CStop();
		return 1;
	}
	//STOP�ź�
 Vkl060_I2CStop();
 return 0;   
}
/*******************************************************************************
* Function Name  : WritenData
* Description    : д������ݵ���ʾRAM
* Input          : Addr-д��ram����ʼ��ַ
*                : Databuf->д��ram������bufferָ��
*                : Cnt->д��ram�����ݸ���
* Output         : None
* Return         : 0-ok 1-fail
*******************************************************************************/
unsigned char  WritenDataVkl060(unsigned char Addr,unsigned char *Databuf,unsigned char Cnt)
{
	unsigned char n;
	
	//START�ź�	
	Vkl060_I2CStart(); 									
	//SLAVE��ַ
	Vkl060_I2CCmdByte(Vkl060_ADDR); 	
	if( 1 == Vkl060_I2CSlaveAck() )
	{
		Vkl060_I2CStop();													
		return 0;										
	}
	//��ʾRAM��ʼ��ַ
	if(Addr>0x1f)
		Vkl060_I2CCmdByte(Vkl060_ADDR5_1); 
	else
		Vkl060_I2CCmdByte(Vkl060_ADDR5_0); 
	if( 1 == Vkl060_I2CSlaveAck() )
	{
		Vkl060_I2CStop();													
		return 0; 
	}
	Vkl060_I2CCmdByte(Addr&0x1f); 						
	if( 1 == Vkl060_I2CSlaveAck() )
	{
		Vkl060_I2CStop();													
		return 0;
	}
	//����Cnt�����ݵ���ʾRAM
	for(n=0;n<Cnt;n++)
	{ 
		Vkl060_I2CDatByte(*Databuf++);
		if( Vkl060_I2CSlaveAck()==1 )
		{
			Vkl060_I2CStop();													
			return 0;
		}
	}
	//STOP�ź�
	 Vkl060_I2CStop();											
	 return 0;    
}
/*******************************************************************************
* Function Name  : Vkl060_DisAll
* Description    : ����SEG��ʾͬһ�����ݣ�bit7/bit3-COM3 bit6/bit2-COM2 bit5/bit1-COM1 bit4/bit0-COM0
* 					     : ���磺0xffȫ�� 0x00ȫ�� 0x55�������� 0xaa�������� 0x33�������� 
* Input          ��dat->д��ram������(1���ֽ����ݶ�Ӧ2��SEG)  
* Output         : None
* Return         : None
*******************************************************************************/
void Vkl060_DisAll(unsigned char dat)
{
	unsigned char segi;
	unsigned char dispram[8];
	
	for(segi=0;segi<8;segi++)//SEG����Ϊ����Ҫ���1���ֽڻ���
	{
		dispram[segi]=dat;
	}
	dispram[7]&=0x0f; //������İ���ֽڻ�����0
	WritenDataVkl060(11,dispram,8);//������8bit���ݶ�Ӧ2��SEG��ÿ4bit���ݵ�ַ��1��ÿ8λ����1��ACK
}
/*******************************************************************************
* Function Name  : DisSegComOn
* Description    : ����1����(1��seg��1��com�����Ӧ����ʾ��)
* Input          ��seg->���Ӧ��seg��  
* 		           ��com->���Ӧcom��  
* Output         : None
* Return         : None
*******************************************************************************/
void Vkl060_DisSegComOn(unsigned char seg,unsigned char com)
{
		Write1DataVkl060(seg,(1<<(com)));//������8λ���ݵ�4bit��Ч��ÿ4bit���ݵ�ַ��1��ÿ8λ����1��ACK
}
/*******************************************************************************
* Function Name  : DisSegComOff
* Description    : �ر�1����(1��seg��1��com�����Ӧ����ʾ��)
* Input          ��seg->���Ӧ��seg��  
* 		           ��com->���Ӧcom��  
* Output         : None
* Return         : None
*******************************************************************************/
void Vkl060_DisSegComOff(unsigned char seg,unsigned char com)
{
		Write1DataVkl060(seg,~(1<<com));//������8λ���ݵ�4bit��Ч��ÿ4bit���ݵ�ַ��1��ÿ8λ����1��ACK
}
/*******************************************************************************
* Function Name  : Enter_Standby
* Description    : �������͹���ģʽ,��������ʾ
* Input          ��None 
* Output         : None
* Return         : 0-ok 1-fail
*******************************************************************************/
unsigned char Vkl060_Enter_Standby(void)
{			
	Vkl060_I2CStart();
	Vkl060_I2CCmdByte(Vkl060_ADDR); 
	if( 1 == Vkl060_I2CSlaveAck() )
	{
		Vkl060_I2CStop();
		return 1; 
	}
	Vkl060_I2CCmdByte(Vkl060_MODESET_1_3_OFF);		//����ʾ 1/3bias
//		Vkl060_I2CCmdByte(Vkl060_MODESET_1_2_OFF);	//����ʾ 1/2bias
	if( 1 == Vkl060_I2CSlaveAck() )
	{
		Vkl060_I2CStop();
		return 1; 
	}
	Vkl060_I2CStop();
	return 0; 
}
/*******************************************************************************
* Function Name  : Exit_Standby
* Description    : �˳�����͹���ģʽ
* Input          ��None 
* Output         : None
* Return         : None
*******************************************************************************/
unsigned char Vkl060_Exit_Standby(void)
{	
	unsigned char errorflag; //�����־λ��1Ϊ����0Ϊ��ȷ
	
	//�˳�����͹���״̬��������vkl060	
	errorflag=Vkl060_InitSequence();
	
	return(errorflag);
}

/*******************************************************************************
* Function Name  : InitSequence
* Description    : ��ʼ��ʱ��
* Input          ��None 
* Output         : None
* Return         : 0-ok 1-fail
*******************************************************************************/
unsigned char Vkl060_InitSequence(void)
{		
	unsigned char n;
	
	vkl060_maxcom=4;
	//�ϵ��ʼ��ʱ��	
	//�ϵ��100uS�ڳ�ʼ��
	delay_nus(100);
	//STOP�ź�
	Vkl060_I2CStop();
	//START�ź�
	Vkl060_I2CStart();
	//����SLAVE��ַ
	Vkl060_I2CCmdByte(Vkl060_ADDR); 
	if( 1 == Vkl060_I2CSlaveAck() )
	{
		Vkl060_I2CStop();	
		return 1;
	}
	//����λ
	Vkl060_I2CCmdByte(Vkl060_SOFTRST);
	if( 1 == Vkl060_I2CSlaveAck() )
	{
		Vkl060_I2CStop();	
		return 1;
	}
	//��ͬ�Ĺ���ģʽ������һ����com���������Ҳ��һ��
//	Vkl060_I2CCmdByte(Vkl060_DISCTL|Vkl060_FRNOR|Vkl060_SRNOR|Vkl060_LINER );  //�ϵ�Ĭ�� VDD=5V:<29uA  VDD=3.3V:<21uA 
	Vkl060_I2CCmdByte(Vkl060_DISCTL|Vkl060_FRPM3|Vkl060_SRPM1|Vkl060_FRAMER);  //��ʡ�� VDD=5V:<12.7uA  VDD=3.3V:<8.5uA 
//	Vkl060_I2CCmdByte(Vkl060_DISCTL|Vkl060_FRPM1|Vkl060_SRPM1|Vkl060_FRAMER);  //���� VDD=5V:<13uA  VDD=3.3V:<9uA 
	if( 1 == Vkl060_I2CSlaveAck() )
	{
		Vkl060_I2CStop();	
		return 1;
	}
	//ӳ����ʼ��ַ bit5=0
	Vkl060_I2CCmdByte(Vkl060_ADDR5_0); //SEG32-SEG63��ַ	
	if( 1 == Vkl060_I2CSlaveAck() )
	{
		Vkl060_I2CStop();	
		return 1;
	} 
	//ӳ����ʼ��ַ bit4~bit0=0
	Vkl060_I2CCmdByte(Vkl060_ADSET);//SEG0-SEG31��ַ
	if( 1 == Vkl060_I2CSlaveAck() )
	{
		Vkl060_I2CStop();	
		return 1;
	}
	//����ʾ���ݣ�ÿ���ֽڰ���2��seg����
	for(n=0;n<(Vkl060_SEGNUM/2+1);n++)  //SEG����Ϊ����Ҫ���1���ֽڻ���
	{ 
		Vkl060_I2CDatByte(0x00);    // ��ʼ������ʾ�ڴ�����,���޸�����. 
		if( Vkl060_I2CSlaveAck()==1 )
		{
			Vkl060_I2CStop();	
			return 1;
		}
	}
	//STOP�ź�
	Vkl060_I2CStop(); 
	//START�ź�  
	Vkl060_I2CStart();
	//����SLAVE��ַ
	Vkl060_I2CCmdByte(Vkl060_ADDR);
	if( 1 == Vkl060_I2CSlaveAck() )
	{
		Vkl060_I2CStop();	
		return 1;
	}
	//������ʾģʽ����ʾ��
	Vkl060_I2CCmdByte(Vkl060_MODESET_1_3_ON);		//����ʾ 1/3bias
//		Vkl060_I2CCmdByte(Vkl060_MODESET_1_2_ON);	//����ʾ 1/2bias
	if( 1 == Vkl060_I2CSlaveAck() )
	{
		Vkl060_I2CStop();	
		return 1;
	}
	Vkl060_I2CStop();	
		
	return  0; 
}
/*******************************************************************************
* Function Name  : Lowlevel_Init
* Description    : ����ͨ����GPIO
* Input          ��None 
* Output         : None
* Return         : None
*******************************************************************************/
void Vkl060_Lowlevel_Init(void)
{
	//ͨ���ߵ�ƽ��ͬ������ӵ�ƽת����·
	//�˺������ݿͻ���Ƭ������Ӧ���޸�	
	// Vkl060_SET_SDA_DIR();

	HAL_PIN_Setup_Output(Vkl060_Pin_SCL);
	HAL_PIN_Setup_Input_Pullup(Vkl060_Pin_SDA);
	// GPIO_SetMode(Vkl060_SCL_PORT, Vkl060_SCL_PIN, GPIO_MODE_OUTPUT);
	// GPIO_SetMode(Vkl060_SDA_PORT, Vkl060_SDA_PIN, GPIO_MODE_QUASI);
		     
	Vkl060_SCL_H();  
	Vkl060_SDA_H(); 	
}
/*******************************************************************************
* Function Name  : Init
* Description    : ��ʼ������
* Input          ��None 
* Output         : None
* Return         : None
*******************************************************************************/
void Vkl060_Init(void)
{	
	//�ܽ����ø��ݿͻ���Ƭ������Ӧ���޸�
	Vkl060_Lowlevel_Init();
	//��ʼ��ʱ��
	Vkl060_InitSequence();
}
/*******************************************************************************
* Function Name  : disp_3num
* Description    : ��ʾ3λ����
* Input          ��dat->3λ���� ʮ���� 
* Output         : None
* Return         : None
*******************************************************************************/
void disp_3num(unsigned int dat)
{		
	unsigned dat8;
	
	dat8=dat/100%10;	//��ʾ��λ
	vkl060_dispram[0]&=0x7f;
	vkl060_dispram[0]|=shuzi_zimo[dat8];
	
	dat8=dat/10%10; 	//��ʾʮλ
	vkl060_dispram[1]&=0x7f;
	vkl060_dispram[1]|=shuzi_zimo[dat8];
	
	dat8=dat%10;			//��ʾ��λ
	vkl060_dispram[2]&=0x7f;
	vkl060_dispram[2]|=shuzi_zimo[dat8];
		
	if(dat<100)				//����С��100����λ����ʾ
	{
		vkl060_dispram[0]&=0x7f;
	}
	if(dat<10) 	//����С��10��ʮλ����ʾ
	{
		vkl060_dispram[1]&=0x7f;
	}
	//SEG������1��1��������
//	Write1DataVkl060(11,vkl060_dispram[0]);
//	Write1DataVkl060(12,vkl060_dispram[1]);
//	Write1DataVkl060(13,vkl060_dispram[2]);
	//SEG�����Ͷ������
	WritenDataVkl060(11,&vkl060_dispram[0],3);
}	
/*******************************************************************************
* Function Name  : test_Main
* Description    : ����������
* Input          ��None 
* Output         : None
* Return         : None
*******************************************************************************/
commandResult_t Vkl060_Test(void)
{	
	Vkl060_Init();
	ADDLOG_INFO(LOG_FEATURE_DRV, "Starting test");
	Vkl060_DisAll(0x00);

	ADDLOG_INFO(LOG_FEATURE_DRV, "Vkl060_DisAll 0xff");
	Vkl060_DisAll(0xff);			//LCDȫ��
	delay_nms(1000);					//��ʱ1S
	
	ADDLOG_INFO(LOG_FEATURE_DRV, "Vkl060_DisAll 0x00");
	Vkl060_DisAll(0x00);			//LCDȫ��
	delay_nms(1000);					//��ʱ1S
	
	ADDLOG_INFO(LOG_FEATURE_DRV, "Vkl060_3num 123");
	disp_3num(123);           //��ʾ����123
	delay_nms(3000);					//��ʱ3S
	
	ADDLOG_INFO(LOG_FEATURE_DRV, "Vkl060_DisSegComOn rolling");
	Vkl060_DisAll(0x00);			//LCDȫ��
	for(vkl060_segi=0;vkl060_segi<Vkl060_SEGNUM;vkl060_segi++)//seg
	{
		for(vkl060_comi=0;vkl060_comi<vkl060_maxcom;vkl060_comi++)//com
		{
			Vkl060_DisSegComOn(vkl060_segtab[vkl060_segi],vkl060_comi);	//LCD�������
			delay_nms(300);				//��ʱ300mS
			Vkl060_DisAll(0x00);	//LCDȫ��
		}
	}
	
	ADDLOG_INFO(LOG_FEATURE_DRV, "Vkl060_DisSegComOff rolling");
	Vkl060_DisAll(0xff);			//LCDȫ��
	delay_nms(1000);					//��ʱ1S
	for(vkl060_segi=0;vkl060_segi<Vkl060_SEGNUM;vkl060_segi++)//seg
	{
		for(vkl060_comi=0;vkl060_comi<vkl060_maxcom;vkl060_comi++)//com
		{
			Vkl060_DisSegComOff(vkl060_segtab[vkl060_segi],vkl060_comi);	//LCD����ر�
			delay_nms(300);				//��ʱ300mS
			Vkl060_DisAll(0xff);	//LCDȫ��
		}
	}
	delay_nms(1000);					//��ʱ1S
	
	Vkl060_DisAll(0x00);			//LCDȫ��
	delay_nms(1000);					//��ʱ1S
	
	ADDLOG_INFO(LOG_FEATURE_DRV, "Vkl060_Enter_Standby");
	Vkl060_Enter_Standby();		//�������͹���ģʽ
	delay_nms(5000);					//��ʱ5S
	
	ADDLOG_INFO(LOG_FEATURE_DRV, "Vkl060_Exit_Standby");
	Vkl060_Exit_Standby();		//�˳�����͹���ģʽ

	return CMD_RES_OK;
}

/************************END OF ORIGINAL FILE****/

// backlog startDriver HT16K33; HT16K33_Test
// backlog startDriver HT16K33; HT16K33_Print Help
void VKL060_Init() {
	ADDLOG_INFO(LOG_FEATURE_DRV, "Vkl060_Init");
	//cmddetail:{"name":"VKL060_Test","args":"VKL060_Test",
	//cmddetail:"descr":"",
	//cmddetail:"fn":"NULL);","file":"driver/drv_vkl060.c","requires":"",
	//cmddetail:"examples":""}
	CMD_RegisterCommand("VKL060_Test", Vkl060_Test, NULL);
}










// commandResult_t HT16K33_Test(const void* context, const char* cmd, const char* args, int cmdFlags)
// {


// 	g_softI2C.pin_clk = 26;
// 	g_softI2C.pin_data = 24;


// 	Soft_I2C_PreInit(&g_softI2C);

// 	rtos_delay_milliseconds(25);

// 	HT16K33_displayOn();
// 	rtos_delay_milliseconds(25);
// 	//HT16K33_displayClear();
// 	rtos_delay_milliseconds(25);
// 	HT16K33_display("1234", 4);
// 	rtos_delay_milliseconds(25);
// 	HT16K33_brightness(8);

// 	return CMD_RES_OK;
// }
// commandResult_t HT16K33_Print(const void* context, const char* cmd, const char* args, int cmdFlags) {

// 	Tokenizer_TokenizeString(args, 0);

// 	if (Tokenizer_GetArgsCount() < 1) {
// 		return CMD_RES_NOT_ENOUGH_ARGUMENTS;
// 	}
// 	const char *s = Tokenizer_GetArg(0);

// 	HT16K33_display(s, 4);

// 	return CMD_RES_OK;
// }

// commandResult_t HT16K33_Char(const void* context, const char* cmd, const char* args, int cmdFlags) {
// 	int pos, val;

// 	Tokenizer_TokenizeString(args, 0);

// 	if (Tokenizer_GetArgsCount() < 2) {
// 		return CMD_RES_NOT_ENOUGH_ARGUMENTS;
// 	}
// 	pos = Tokenizer_GetArgInteger(0);
// 	val = Tokenizer_GetArgInteger(1);

// 	HT16K33_writePos(pos, convert16seg(val));

// 	return CMD_RES_OK;
// }
// commandResult_t HT16K33_Raw(const void* context, const char* cmd, const char* args, int cmdFlags) {
// 	int pos, val;

// 	Tokenizer_TokenizeString(args, 0);

// 	if (Tokenizer_GetArgsCount() < 2) {
// 		return CMD_RES_NOT_ENOUGH_ARGUMENTS;
// 	}
// 	pos = Tokenizer_GetArgInteger(0);
// 	val = Tokenizer_GetArgInteger(1);

// 	HT16K33_writePos(pos, val);

// 	return CMD_RES_OK;
// }
// commandResult_t HT16K33_Brightness(const void* context, const char* cmd, const char* args, int cmdFlags) {
// 	int val;

// 	Tokenizer_TokenizeString(args, 0);

// 	if (Tokenizer_GetArgsCount() < 1) {
// 		return CMD_RES_NOT_ENOUGH_ARGUMENTS;
// 	}
// 	val = Tokenizer_GetArgInteger(0);

// 	HT16K33_brightness(val);

// 	return CMD_RES_OK;
// }
// commandResult_t HT16K33_Blink(const void* context, const char* cmd, const char* args, int cmdFlags) {
// 	int type;

// 	Tokenizer_TokenizeString(args, 0);

// 	if (Tokenizer_GetArgsCount() < 1) {
// 		return CMD_RES_NOT_ENOUGH_ARGUMENTS;
// 	}
// 	type = Tokenizer_GetArgInteger(0);

// 	if (type == 0) {
// 		HT16K33_WriteCmd(HT16K33_BLINKOFF);
// 	}
// 	else if (type == 1) {
// 		HT16K33_WriteCmd(HT16K33_BLINKON0_5HZ);
// 	}
// 	else if (type == 2) {
// 		HT16K33_WriteCmd(HT16K33_BLINKON1HZ);
// 	}
// 	else {
// 		HT16K33_WriteCmd(HT16K33_BLINKON2HZ);
// 	}

// 	return CMD_RES_OK;
// }
// // backlog startDriver HT16K33; HT16K33_Test
// // backlog startDriver HT16K33; HT16K33_Print Help
// void HT16K33_Init() {

// 	//cmddetail:{"name":"HT16K33_Test","args":"HT16K33_Test",
// 	//cmddetail:"descr":"",
// 	//cmddetail:"fn":"NULL);","file":"driver/drv_ht16k33.c","requires":"",
// 	//cmddetail:"examples":""}
// 	CMD_RegisterCommand("HT16K33_Test", HT16K33_Test, NULL);
// 	//cmddetail:{"name":"HT16K33_Char","args":"HT16K33_Char",
// 	//cmddetail:"descr":"",
// 	//cmddetail:"fn":"NULL);","file":"driver/drv_ht16k33.c","requires":"",
// 	//cmddetail:"examples":""}
// 	CMD_RegisterCommand("HT16K33_Char", HT16K33_Char, NULL);
// 	//cmddetail:{"name":"HT16K33_Raw","args":"HT16K33_Raw",
// 	//cmddetail:"descr":"",
// 	//cmddetail:"fn":"NULL);","file":"driver/drv_ht16k33.c","requires":"",
// 	//cmddetail:"examples":""}
// 	CMD_RegisterCommand("HT16K33_Raw", HT16K33_Raw, NULL);
// 	//cmddetail:{"name":"HT16K33_Print","args":"HT16K33_Print",
// 	//cmddetail:"descr":"",
// 	//cmddetail:"fn":"NULL);","file":"driver/drv_ht16k33.c","requires":"",
// 	//cmddetail:"examples":""}
// 	CMD_RegisterCommand("HT16K33_Print", HT16K33_Print, NULL);
// 	//cmddetail:{"name":"HT16K33_Brightness","args":"HT16K33_Brightness",
// 	//cmddetail:"descr":"",
// 	//cmddetail:"fn":"NULL);","file":"driver/drv_ht16k33.c","requires":"",
// 	//cmddetail:"examples":""}
// 	CMD_RegisterCommand("HT16K33_Brightness", HT16K33_Brightness, NULL);
// 	//cmddetail:{"name":"HT16K33_Blink","args":"HT16K33_Blink",
// 	//cmddetail:"descr":"",
// 	//cmddetail:"fn":"NULL);","file":"driver/drv_ht16k33.c","requires":"",
// 	//cmddetail:"examples":""}
// 	CMD_RegisterCommand("HT16K33_Blink", HT16K33_Blink, NULL);
// }
