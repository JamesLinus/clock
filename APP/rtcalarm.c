#include "2410lib.h"
#include "2410addr.h"
#include "def.h"

//�������õ��ĺ궨��
#define ADCPRS 39
#define ITERATION 5	

//LCD��ʾ�õ��ĺ궨��
#define MVAL		(13)
#define MVAL_USED 	(0)		//0=each frame   1=rate by MVAL
#define INVVDEN		(1)		//0=normal       1=inverted
#define BSWP		(0)		//Byte swap control
#define HWSWP		(1)		//Half word swap control

#define M5D(n) ((n) & 0x1fffff)	//��ȡ��͵�21λ

//TFT 800480
#define LCD_XSIZE_TFT_800480 	(800)	//����LCD��Ļ�Ŀ��
#define LCD_YSIZE_TFT_800480 	(480)	//����LCD��Ļ�ĸ߶�

//TFT 800480
#define SCR_XSIZE_TFT_800480	(800)
#define SCR_YSIZE_TFT_800480 	(480)

//TFT800480
#define HOZVAL_TFT_800480   (LCD_XSIZE_TFT_800480-1)
#define LINEVAL_TFT_800480	(LCD_YSIZE_TFT_800480-1)

//Timing parameter for LCD
#define VBPD_800480		(9)		  //��ֱͬ���źŵĺ��
#define VFPD_800480		(6)		  //��ֱͬ���źŵ�ǰ��
#define VSPW_800480		(2)		  //��ֱͬ���źŵ�����

#define HBPD_800480		(10)	  //ˮƽͬ���źŵĺ��
#define HFPD_800480		(67)	  //ˮƽͬ���źŵ�ǰ��
#define HSPW_800480		(20)	  //ˮƽͬ���źŵ�����
#define CLKVAL_TFT_800480	(1)   //22.5MHZ	

#define FONTDATAMAX 2048          //�����ʽ����ģ�����������С

#define CHAR_ROW_CNT (8)
#define CHAR_ROW_CNT_HALF (4)
#define CHAR_SIZE (64)
#define CHAR_SIZE_HALF (32)

#define BGCOLOR 0x00ff

extern const unsigned char fontdata_8x8[FONTDATAMAX];   //������ʽ����ģ������μ���font_8x8.c����
//������Ļ��ʹ�õ���"����ʱ����һ����ֹͣ��"��Щ���ֵ���ģ���飬������ʾ��LCD��Ļ�ϣ�����μ���font_8x8.c����
extern const unsigned char char_set[12][512];  
extern const unsigned char num_set[13][256];    //����"0123456789"��Щ���ֵ���ģ���飬������ʾ��LCD��Ļ�ϣ�����μ���font_8x8.c����

//�������Ӳ���Ҫ�õ��ĺ���
void OpenRtc(void);//����ʱ��
void OpenAlarm(void);//��������
void Get_Rtc(void); //ʵʱʱ����ʾ
void setRTCtime(U8,U8,U8,U8,U8,U8,U8);//���õ�ǰʱ��
void setRTCalm(U8,U8,U8,U8,U8,U8);//��������ʱ��
void setNextRTCalm(void);//������һ����ʱ�䣨�Զ���ȡ��ֵ��
void __irq IsrAlarm(void);//���������ж�
void __irq Tick_Isr(void);//����ʱ���ж�

//�������Ӳ���Ҫ�õ���ȫ�ֳ���
extern const int rtctime_init [6];  //�ⲿ���õ�ǰʱ���ֵ����ǰʱ��Ϊ2014��5��27��11ʱ1��15�루����μ���timedata.c����
extern const int rtcalm_init [3][6] ;  //�ⲿ����������Ӧʱ�䣬�Զ�ά�����ʾ�����˶�����ӡ�������μ���timedata.c����

//��������������Ҫ�õ��ĺ���
void Touch_Screen_Init(void);//��ʼ��������
void Touch_Screen_Off(void) ;//�رմ�����
void __irq Touch_Screen(void);//�����������ж�

//��������������Ҫ�õ��ĺ���
void Buzzer_PWM_Run(void);//��������������
void Buzzer_Stop(void);//�����������ر�
void Buzzer_Freq_Set(U32);//�����������趨

//����LCD��Ļ��ʾҪ�õ��ĺ���
void Test_Lcd_Tft_800480(void);	//������LCD��Ļ�������ʾ��ȫ��������
static void Lcd_Init(void);	//����LCD����ģ���ʼ����
static void Lcd_EnvidOnOff(int onoff);	//����LCD��Ƶ�Ϳ����ź��������ֹͣ��
static void PutPixel(U32 x,U32 y,U32 c);	//����LCD�������ص���ʾ���������
static void Lcd_ClearScr(U16 c);	//����LCDȫ������ض���ɫ��Ԫ������.
//static void Paint_Bmp(int x0,int y0,int h,int l,unsigned char bmp[]);	//������LCD��Ļ��ָ������㻭һ��ָ����С��ͼƬ.

/*����LCD������õ��ĺ���*/
void lcd_draw_char(int loc_x, int loc_y, unsigned char c);	//����һ����������д���֡�
void lcd_draw_num(int loc_x, int loc_y, unsigned char c);	//����һ����������д���֡�
//����һ����������LCD��Ļ�ϻ�������ʱ�Ӻ���һ���ӵ�ʱ�䡣
void lcd_draw_clock(int _year, int _month, int _date, int _hour, int _min, int _sec, int loc_x, int loc_y);	

//LCDҪ�õ���ȫ�ֱ���
//extern unsigned char GEC_800480[];	
volatile static unsigned short LCD_BUFER[SCR_YSIZE_TFT_800480][SCR_XSIZE_TFT_800480];


//����һ���ַ����飬ȥ����0λ��1-7λ�ֱ�����յ�����7�졣
char *week[8] = { "","SUN","MON", "TUES", "WED", "THURS","FRI", "SAT" } ;
int year,month,date,weekday,hour,min,sec;	//�����ꡢ�¡��ա����ڡ�ʱ���֡���
int enable_beep = 1;	//������ʹ�ñ�ʶΪ1����ʶ����������
unsigned int buf[ITERATION][2];

int rtcalm_init_index = 0;	//�йض�����

/********************************************************************
Function name: xmain
Description	 : ������
*********************************************************************/
void xmain(void)
{
	int index = 0;   //???
	
    ChangeClockDivider(1,1);
    ChangeMPllValue(0xa1,0x3,0x1);   
    Port_Init();
    Uart_Select(0);
    Uart_Init(0,115200);
	
	Test_Lcd_Tft_800480();  //Ϊ��ֹLCD��Ļ��ʾ���ݹ��������Ӧ�÷��ڳ������ǰ����м��ء�

	/*LCD���"����ʱ�䣺xxxx-xx-xx xx:xx:xx"*/
	for (index = 0; index <5 ;++index)
	{
		lcd_draw_char(index*64,0,index);
	}
	lcd_draw_clock(rtctime_init[0],rtctime_init[1],rtctime_init[2],rtctime_init[3],rtctime_init[4],rtctime_init[5],0,64);
	
	/*LCD���"��һ���ӣ�xxxx-xx-xx xx:xx:xx"*/
	for (index = 0; index <4 ;++index)
	{
		lcd_draw_char(index*64,150,index+5);
	}
	lcd_draw_char(256,150,4);
   
   	setRTCtime(rtctime_init[0],rtctime_init[1],rtctime_init[2],0x0,rtctime_init[3],rtctime_init[4],rtctime_init[5]);
   	//����RTCʱ��ʱ�䣬����Ϊ�ꡢ�¡��ա����ڡ�ʱ���֡���
	rtcalm_init_index = 0;
	setNextRTCalm();
   
	OpenAlarm();  //��������
	OpenRtc();    //����ʱ��
	while(1);
}

/********************************************************************
Function name: setRTCtime
Description	 : ����ʱ�Ӷ�ʱʱ�䣬����˳��Ϊ�ꡢ�¡��ա����ڡ�ʱ���֡���
*********************************************************************/
void setRTCtime(U8 wRTCyear,U8 wRTCmon,U8 wRTCdate,U8 wRTCday,U8 wRTChour,U8 wRTCmin,U8 wRTCsec)
{
	rRTCCON=0x01;	
	rBCDYEAR = wRTCyear;
	rBCDMON  = wRTCmon;
	rBCDDATE = wRTCdate;
	rBCDDAY  = wRTCday;
	rBCDHOUR = wRTChour;
	rBCDMIN  = wRTCmin;
	rBCDSEC  = wRTCsec;	
    rRTCCON = 0x0;	//disable RTC write
}

/********************************************************************
Function name: OpenRtc
Description	 : ����ʱ��
*********************************************************************/
void OpenRtc(void)
{
    pISR_TICK=(unsigned)Tick_Isr;
    rTICNT=0xB0;//Tick time interrupt enable;Tick time count value=127
    EnableIrq(BIT_TICK);//����RTC�ж�
}

/********************************************************************
Function name: CloseRtc
Description	 : �ر�ʱ��
*********************************************************************/
void CloseRtc(void)
{
    rTICNT &= ~(1<<7);
    DisableIrq(BIT_TICK);
}

/********************************************************************
Function name: setNextRTCalm
Description	 : ������һ��������Ӧʱ��
*********************************************************************/
void setNextRTCalm(void)
{
	while (1)
	{
		if (rtcalm_init_index > 3) 
		{
			lcd_draw_clock(0,0,0,0,0,0,0,214);
			break; //����һ��������
		}
		if (year > rtcalm_init[rtcalm_init_index][0])
		{
			++rtcalm_init_index;
			continue;
		}
		else if (year == rtcalm_init[rtcalm_init_index][0] && month > rtcalm_init[rtcalm_init_index][1])
		{
			++rtcalm_init_index;
			continue;
		}
		else if (year == rtcalm_init[rtcalm_init_index][0] && month == rtcalm_init[rtcalm_init_index][1] && 
		date > rtcalm_init[rtcalm_init_index][2])
		{
			++rtcalm_init_index;
			continue;
		}
		else if (year == rtcalm_init[rtcalm_init_index][0] && month == rtcalm_init[rtcalm_init_index][1] && 
		date == rtcalm_init[rtcalm_init_index][2] && hour > rtcalm_init[rtcalm_init_index][3])
		{
			++rtcalm_init_index;
			continue;
		}
		else if (year == rtcalm_init[rtcalm_init_index][0] && month == rtcalm_init[rtcalm_init_index][1] && 
		date == rtcalm_init[rtcalm_init_index][2] && hour == rtcalm_init[rtcalm_init_index][3] && 
		min > rtcalm_init[rtcalm_init_index][4])
		{
			++rtcalm_init_index;
			continue;
		}
		else if (year == rtcalm_init[rtcalm_init_index][0] && month == rtcalm_init[rtcalm_init_index][1] && 
		date == rtcalm_init[rtcalm_init_index][2] && hour == rtcalm_init[rtcalm_init_index][3] && 
		min == rtcalm_init[rtcalm_init_index][4] && sec > rtcalm_init[rtcalm_init_index][5])
		{
			++rtcalm_init_index;
			continue;
		}
		else 
		{
			rRTCCON=0x0001;  //���һ��λΪ1����RTC����������	
			rALMYEAR =  rtcalm_init[rtcalm_init_index][0];
			rALMMON  =  rtcalm_init[rtcalm_init_index][1];
			rALMDATE =  rtcalm_init[rtcalm_init_index][2];
			rALMHOUR =  rtcalm_init[rtcalm_init_index][3];
			rALMMIN  =  rtcalm_init[rtcalm_init_index][4];
			rALMSEC  =  rtcalm_init[rtcalm_init_index][5];
			rRTCCON = 0x0;	//�ر�RTC������
			//LCD��ʾ��ˢ�£���һ����ʱ��
			lcd_draw_clock(rtcalm_init[rtcalm_init_index][0],rtcalm_init[rtcalm_init_index][1],rtcalm_init[rtcalm_init_index][2],
				rtcalm_init[rtcalm_init_index][3],rtcalm_init[rtcalm_init_index][4],rtcalm_init[rtcalm_init_index][5],0,214);
			
			//Ϊ�´�����������׼��
			++rtcalm_init_index;
			break;
		}
	}
}

/********************************************************************
Function name: setRTCalm
Description	 : ����������Ӧʱ�䣬����˳��Ϊ�ꡢ�¡��ա�ʱ���֡���
*********************************************************************/
void setRTCalm(U8 almyear,U8 almmon,U8 almdate,U8 almhour,U8 almmin,U8 almsec)
{
	rRTCCON=0x0001;  //���һ��λΪ1����RTC����������
	rALMYEAR = almyear;
	rALMMON  = almmon;
	rALMDATE = almdate;
	rALMHOUR = almhour;
	rALMMIN  = almmin;
	rALMSEC  = almsec;
	rRTCCON = 0x0;	//�ر�RTC������
}

/********************************************************************
Function name: OpenAlarm
Description	 : ��������
*********************************************************************/
void OpenAlarm(void)
{
	pISR_RTC = (unsigned)IsrAlarm;
	ClearPending(BIT_RTC);	
	rRTCALM = (0x7f);	//enable alarm
	EnableIrq(BIT_RTC);		
}

/********************************************************************
Function name: CloseAlarm
Description	 : �ر�����
*********************************************************************/
void CloseAlarm(void)
{
	rRTCALM = 0;			//disable alarm
	DisableIrq(BIT_RTC);
}

/********************************************************************
Function name: Get_Rtc
Description	 : ʵʱʱ����ʾ����
*********************************************************************/
void Get_Rtc(void)
{
	rRTCCON = 0x01;  //RTC��дʹ�ܣ�ѡ��BCDʱ�ӡ����������޸�λ��1/32768
	if (rBCDYEAR == 0x99)    
		year = 0x1999;
	else    
		year = 0x2000 + rBCDYEAR;
	month=rBCDMON;
	date=rBCDDATE;
	weekday=rBCDDAY;
	hour=rBCDHOUR;
	min=rBCDMIN;
	sec=rBCDSEC;
	rRTCCON = 0x0;  //RTC��д��ֹ��ѡ��BCDʱ�ӡ����������޸�λ��1/32768
}

/********************************************************************
Function name: IsrAlarm
Description	 : �����жϳ���
*********************************************************************/
void __irq IsrAlarm(void)
{
    ClearPending(BIT_RTC);
    Uart_Printf("s3c244A RTCALM  oucer \n");
    Buzzer_PWM_Run();
    /*���"ֹͣ"����*/
	lcd_draw_char(336,400,9);
	lcd_draw_char(400,400,10);
	//Delay(1000);
    Touch_Screen_Init();
}

/********************************************************************
Function name: Tick_Isr
Description	 : ʱ���жϳ����ڴ�����ʾʱ��
*********************************************************************/
void __irq Tick_Isr(void)
{
	ClearPending(BIT_TICK);
	Get_Rtc();
	//Uart_Printf("RTC TIME : %4x-%02x-%02x - %s - %02x:%02x:%02x\n",year,month,date,week[weekday],hour,min,sec);
	lcd_draw_clock(year,month,date,hour,min,sec,0,64);
}

/********************************************************************
Function name: Touch_Screen_Init
Description	 : ��������ʼ��
*********************************************************************/
void Touch_Screen_Init(void)
{
    rADCDLY = (30000);    // ADC Start or Interval Delay
    rADCCON = (1<<14)|(ADCPRS<<6)|(0<<3)|(0<<2)|(0<<1)|(0); 
    // Enable Prescaler,Prescaler,AIN5/7 fix,Normal,Disable read start,No operation
    
    rADCTSC = (0<<8)|(1<<7)|(1<<6)|(0<<5)|(1<<4)|(0<<3)|(0<<2)|(3);//tark
    // Down,YM:GND,YP:AIN5,XM:Hi-z,XP:AIN7,XP pullup En,Normal,Waiting for interrupt mode

	rSUBSRCPND |= BIT_SUB_TC;   //rSUBSRCPND��0x4a000018   BIT_SUB_TC��0x1<<9
    rINTSUBMSK  =~(BIT_SUB_TC);  //0x4a00001c
     
   	pISR_ADC    = (unsigned)Touch_Screen;
    
    rINTMSK    &= ~(BIT_ADC);
    rINTSUBMSK &= ~(BIT_SUB_TC);
    
    Uart_Printf( "\ntouch screen stop ring\n" ) ;   
}

/********************************************************************
Function name: Touch_Screen_Off
Description	 : �������ر�
*********************************************************************/
void Touch_Screen_Off(void)
{
	rINTMSK    |= (BIT_ADC);
    rINTSUBMSK |= (BIT_SUB_TC);
}

/********************************************************************
Function name: Touch_Screen
Description	 : �����������ж�
*********************************************************************/
void __irq Touch_Screen(void)
{
	int i;
	int total_x=0;
	int avg_x=0;
	int total_y=0;
	int avg_y=0;
    rINTSUBMSK |= (BIT_SUB_ADC | BIT_SUB_TC);     //Mask sub interrupt (ADC and TC) 

    //Uart_Printf("\nTS Down!\n");
        
      //Auto X-Position and Y-Position Read
    rADCTSC=(1<<7)|(1<<6)|(0<<5)|(1<<4)|(1<<3)|(1<<2)|(0);
          //[7] YM=GND, [6] YP is connected with AIN[5], [5] XM=Hi-Z, [4] XP is connected with AIN[7]
          //[3] XP pull-up disable, [2] Auto(sequential) X/Y position conversion mode, [1:0] No operation mode  

    for(i=0;i<ITERATION;i++)
    {
        rADCTSC  = (1<<7)|(1<<6)|(0<<5)|(1<<4)|(1<<3)|(1<<2)|(0);            
        rADCCON |= 0x1;             //Start Auto conversion
        while(rADCCON & 0x1);       //Check if Enable_start is low 	//0:0 skip loop
        while(!(0x8000&rADCCON));   //Check ECFLG   				//5:1 skip loop
    
        //��֤��6λ���ϣ�ǰ��10λ��Ч��0x3ff=0000 0011 1111 1111��rADCDAT0��������x��rADCDAT1����y��
        buf[i][0] = (0x3ff&rADCDAT0);   
        buf[i][1] = (0x3ff&rADCDAT1);
    }

    for(i=0;i<ITERATION;i++){
		total_x+=buf[i][0];
		total_y+=buf[i][1];
        //Uart_Printf("X %4d, Y %4d\n", buf[i][0], buf[i][1]);
    }
	avg_x=total_x/ITERATION;
	avg_y=total_y/ITERATION;
	if(avg_x<=550&&avg_x>400&&avg_y<=200&&avg_y>100)
	{
		Buzzer_Stop();
		setNextRTCalm();
		/*����"ֹͣ"����*/
		lcd_draw_char(336,400,11);
		lcd_draw_char(400,400,11);
		Touch_Screen_Off();
	}
    rADCTSC = (1<<7)|(1<<6)|(0<<5)|(1<<4)|(0<<3)|(0<<2)|(3);  
      //[7] YM=GND, [6] YP is connected with AIN[5], [5] XM=Hi-Z, [4] XP is connected with AIN[7]
      //[3] XP pull-up enable, [2] Normal ADC conversion, [1:0] Waiting for interrupt mode                
    rSUBSRCPND |= BIT_SUB_TC;
   	rINTSUBMSK  =~(BIT_SUB_TC);   //Unmask sub interrupt (TC)     
    ClearPending(BIT_ADC);
}

/********************************************************************
Function name: Buzzer_Freq_Set
Description	 : �����������趨
*********************************************************************/
void Buzzer_Freq_Set(U32 freq)
{   
	rGPBCON = rGPBCON & ~(3<<0)|(1<<1);//set GPB0 as tout0, pwm output
		
	rTCFG0 = rTCFG0 & ~0xff|15; //prescaler = 15
	rTCFG1 = rTCFG1 & ~0xf|2;//divider = 1/8
			
	rTCNTB0 = (PCLK>>7)/freq;//rTCNTB0=PCLK/{(prescaler+1) * divider *freq}
	rTCMPB0 = rTCNTB0>>1;	//ռ�ձ�50%
	
	//disable deadzone, auto-reload, inv-off, update TCNTB0&TCMPB0, start timer 0
	rTCON = rTCON & ~0x1f|(0<<4)|(1<<3)|(0<<2)|(1<<1)|(1);
	rTCON &= ~(1<<1);			//clear manual update bit
}

/********************************************************************
Function name: Buzzer_Stop
Description	 : ������ֹͣ����
*********************************************************************/
void Buzzer_Stop( void )
{
	rGPBCON |= 1;		
	rGPBCON = rGPBCON & ~3|1;			//set GPB0 as output
	rGPBDAT &= ~1;//output 0
}

/********************************************************************
Function name: Buzzer_PWM_Run
Description	 : ����������
********************************************************************/ 
void Buzzer_PWM_Run( void )
{
	U16 freq = 1500 ;
	Buzzer_Freq_Set( freq ) ;
}

/********************************************************************
Function name: PutPixel
Description	 : LCD�������ص���ʾ�������
*********************************************************************/
static void PutPixel(U32 x,U32 y,U32 c)
{
	if ( (x < SCR_XSIZE_TFT_800480) && (y < SCR_YSIZE_TFT_800480) )
		LCD_BUFER[(y)][(x)] = c;
}

/********************************************************************
Function name: Lcd_Port_Init
Description	 : LCD���ݺͿ��ƶ˿ڳ�ʼ��
*********************************************************************/
static void Lcd_Port_Init(void)
{
    rGPCUP = 0x0; // enable Pull-up register
    rGPCCON = 0xaaaa56a9; //Initialize VD[7:0],LCDVF[2:0],VM,VFRAME,VLINE,VCLK,LEND 
    //rGPCCON = 0xaaaaaaaa;
    rGPDUP = 0x0 ; // enable Pull-up register
    rGPDCON=0xaaaaaaaa; //Initialize VD[15:8]
}

/********************************************************************
Function name: Lcd_Init
Description	 : LCD����ģ���ʼ��
*********************************************************************/
static void Lcd_Init(void)
{
	//CLKVAL=1;MMODE=0;PNRMODE=11:11 = TFT LCD panel
	//BPPMODE=1100=16 bpp for TFT;ENVID0=Disable the video output and the LCD control signal.
	rLCDCON1=(CLKVAL_TFT_800480<<8)|(MVAL_USED<<7)|(3<<5)|(12<<1)|0;
    //VBPD=9;LINEVAL=799;VFPD=6;VSPW=2
	rLCDCON2=(VBPD_800480<<24)|(LINEVAL_TFT_800480<<14)|(VFPD_800480<<6)|(VSPW_800480);
	//HBPD=10;HOZVAL=479;HFPD=16
	rLCDCON3=(HBPD_800480<<19)|(HOZVAL_TFT_800480<<8)|(HFPD_800480);
	//MVAL=13;HSPW=20
	rLCDCON4=(MVAL<<8)|(HSPW_800480);
	//FRM5:6:5,HSYNC and VSYNC are inverted
	rLCDCON5=(1<<11)|(0<<10) |(1<<9)|(1<<8)|(0<<7) |(0<<6)|(BSWP<<1)|(HWSWP);	
	
	rLCDSADDR1=(((U32)LCD_BUFER>>22)<<21)|M5D((U32)LCD_BUFER>>1);
	rLCDSADDR2=M5D( ((U32)LCD_BUFER+(SCR_XSIZE_TFT_800480*LCD_YSIZE_TFT_800480*2))>>1 );
	rLCDSADDR3=(((SCR_XSIZE_TFT_800480-LCD_XSIZE_TFT_800480)/1)<<11)|(LCD_XSIZE_TFT_800480/1);
	
	rLCDINTMSK|=(3); // MASK LCD Sub Interrupt
	rLPCSEL&=(~7); // Disable LPC3600
	rTPAL=0; // Disable Temp Palette
}

/********************************************************************
Function name: Lcd_EnvidOnOff
Description	 : LCD��Ƶ�Ϳ����ź��������ֹͣ��1������Ƶ���
*********************************************************************/
static void Lcd_EnvidOnOff(int onoff)
{
    if(onoff==1)
	rLCDCON1|=1; // ENVID=ON
    else
	rLCDCON1 =rLCDCON1 & 0x3fffe; // ENVID Off
}

/********************************************************************
Function name: Lcd_ClearScr
Description	 : LCDȫ������ض���ɫ��Ԫ������
*********************************************************************/
static void Lcd_ClearScr(U16 c)
{
	unsigned int x,y ;
		
    for( y = 0 ; y < SCR_YSIZE_TFT_800480; y++ )
    {
    	for( x = 0 ; x < SCR_XSIZE_TFT_800480; x++ )
    	{
			LCD_BUFER[y][x] = c;
    	}
    }
}

/********************************************************************
Function name: Paint_Bmp
Description	 : ��LCD��Ļ��ָ������㻭һ��ָ����С��ͼƬ
*********************************************************************/
/*
static void Paint_Bmp(int x0,int y0,int h,int l,unsigned char bmp[])
{
	int x,y;
	U32 c;
	int p = 0;

	//��ͼƬ��ʾ��������ʾ��ʽ
    for( y = 0 ; y < l ; y++ )
    {
    	for( x = 0 ; x < h ; x++ )
    	{
    		c = bmp[p+1] | (bmp[p]<<8) ;

			if ( ( (x0+x) < SCR_XSIZE_TFT_800480) && ( (y0+y) < SCR_YSIZE_TFT_800480) )
				LCD_BUFER[y0+y][x0+x] = c ;

    		p = p + 2 ;
    	}
    }
}
*/
/********************************************************************
Function name: Test_Lcd_Tft_800480
Description	 : ��LCD��Ļ�������ʾ��ȫ������
*********************************************************************/
void Test_Lcd_Tft_800480( void )
{
    Lcd_Port_Init();          //LCD���ݺͿ��ƶ˿ڳ�ʼ��
    Lcd_Init();				  //LCD����ģ���ʼ��
    Lcd_EnvidOnOff(1);		  //������Ƶ���
	Lcd_ClearScr(BGCOLOR);   //���ñ���ɫ
    //Paint_Bmp( 0,0,800,480, GEC_800480) ;		//��LCD��Ļ��ָ������㻭һ��ָ����С��ͼƬ
}

/********************************************************************
Function name: lcd_draw_char
Description	 : ��LCD��Ļ��ָ������дһ������
*********************************************************************/
void lcd_draw_char(int loc_x, int loc_y, unsigned char c)
{
    int i,j;
    unsigned char line_dots[CHAR_ROW_CNT];
    
    for (i = 0; i < CHAR_SIZE; ++i)
    {
        for (j = 0 ; j < CHAR_ROW_CNT ; ++j)
        {
        	line_dots[j] = char_set[c][(i * CHAR_ROW_CNT) + j];
        }
        
        for (j = 0; j < CHAR_SIZE; ++j)
        {
            if (line_dots[(j / 8)] & (0x80 >> (j % 8)))
            {
                PutPixel(loc_x+j,loc_y+i,0xffffff);
            }
            else
            {
                PutPixel(loc_x+j,loc_y+i,BGCOLOR);
            }
        }
    }
}

/********************************************************************
Function name: lcd_draw_num
Description	 : ��LCD��Ļ��ָ������дһ������
*********************************************************************/
void lcd_draw_num(int loc_x, int loc_y, unsigned char c)
{
    int i,j;
    unsigned char line_dots[CHAR_ROW_CNT_HALF];
    
    for (i = 0; i < CHAR_SIZE; ++i)
    {
        for (j = 0 ; j < (CHAR_ROW_CNT_HALF) ; ++j)
        {
        	line_dots[j] = num_set[c][(i * CHAR_ROW_CNT_HALF) + j];
        }
        
        for (j = 0; j < CHAR_SIZE_HALF; ++j)
        {
            if (line_dots[(j / 8)] & (0x80 >> (j % 8)))
            {
                PutPixel(loc_x+j,loc_y+i,0xffffff);
            }
            else
            {
                PutPixel(loc_x+j,loc_y+i,BGCOLOR);
            }
        }
    }
}

/********************************************************************
Function name: lcd_draw_clock
Description	 : ��LCD��Ļ�����ʱ�䣬�����ʽΪXXXX-XX-XX XX:XX:XX
*********************************************************************/
void lcd_draw_clock(int _year, int _month, int _date, int _hour, int _min, int _sec, int loc_x, int loc_y)
{
	int i;
	
	//�����������Ϊ0�������ʾ
	if (_year == 0)
	{
		for (i = 0; i < 19 ;++i)
		{
			lcd_draw_num(loc_x+i*CHAR_SIZE_HALF,loc_y,12);
		}
		return ;
	}
	
	//����̶���ʽ ����ܡ�ð�ţ�
	for (i = 0; i < 19 ;++i)
	{
		if (i == 4 || i == 7)
		{
			lcd_draw_num(loc_x+i*CHAR_SIZE_HALF,loc_y,10);
		}
		else if (i == 13 || i == 16)
		{
			lcd_draw_num(loc_x+i*CHAR_SIZE_HALF,loc_y,11);
		}
	}
	
	//������
	lcd_draw_num(loc_x,loc_y,((_year >> 12) & 0x000f));
	lcd_draw_num(loc_x+CHAR_SIZE_HALF,loc_y,((_year >> 8) & 0x000f));
	lcd_draw_num(loc_x+2*CHAR_SIZE_HALF,loc_y,((_year >> 4) & 0x000f));
	lcd_draw_num(loc_x+3*CHAR_SIZE_HALF,loc_y,(_year & 0x000f));
	
	//����·�
	lcd_draw_num(loc_x+5*CHAR_SIZE_HALF,loc_y,((_month >> 4) & 0x000f));
	lcd_draw_num(loc_x+6*CHAR_SIZE_HALF,loc_y,(_month & 0x000f));
	
	//�������
	lcd_draw_num(loc_x+8*CHAR_SIZE_HALF,loc_y,((_date >> 4) & 0x000f));
	lcd_draw_num(loc_x+9*CHAR_SIZE_HALF,loc_y,(_date & 0x000f));
	
	//���Сʱ
	lcd_draw_num(loc_x+11*CHAR_SIZE_HALF,loc_y,((_hour >> 4) & 0x000f));
	lcd_draw_num(loc_x+12*CHAR_SIZE_HALF,loc_y,(_hour & 0x000f));
	
	//�������
	lcd_draw_num(loc_x+14*CHAR_SIZE_HALF,loc_y,((_min >> 4) & 0x000f));
	lcd_draw_num(loc_x+15*CHAR_SIZE_HALF,loc_y,(_min & 0x000f));
	
	//�������
	lcd_draw_num(loc_x+17*CHAR_SIZE_HALF,loc_y,((_sec >> 4) & 0x000f));
	lcd_draw_num(loc_x+18*CHAR_SIZE_HALF,loc_y,(_sec & 0x000f));
}