#include "HT66F2390.h"
#include "LCD.h"
#include "stdlib.h"
#define IN1 _pb1
#define IN2 _pb0
#define COUNT 2000
#define key_port _pe
#define key_portc _pec
#define led_port _pf
#define led_portc _pfc
volatile unsigned s = 25;
volatile unsigned c = 0, i = 0;
volatile char t_flag = 0, lcd_flag = 0, d = 1, t = 0;
char key = 0x0f, key_hold = 0x0f, hold = 0;
volatile unsigned int adr = 0;
volatile unsigned long adv = 0, test = 0;
unsigned char  text[] = {"Pressure:        "},
			  text2[] = {"P set:           "},
			  P_set[4][3] = {"20","25","29","34"},
			  off_text[] = {"OFF"};
unsigned n1 = 0, un2 = 400;
float P = 0, n2 = 800;
unsigned long adv_history[8] = {0};
unsigned char adv_h_ind = 0;
void get_adc(unsigned char channel)
{
	_sadc0 = 0x30 | channel;
	_start = 1;
	_start = 0;
	while (_adbz);
	adr = ((unsigned int) _sadoh << 8)| _sadol;
	adv = (unsigned long) adr * 122 / 100;
}
unsigned long update_adv()
{
	adv_history[adv_h_ind] = adv;
	adv_h_ind = (adv_h_ind+1)&7;
	char i = 0;
	unsigned long adv_tmp[8];
	for (i=0;i<8;i++)
		adv_tmp[i] = adv_history[i];
	//for (i=0;i<8;i++)
		//result+=adv_history[i];
	qsort(adv_tmp, 8, sizeof(unsigned long));
	return (adv_tmp[3]>> 1) + (adv_tmp[4]>> 1);
	//unsigned long result = 0;
	//char i = 0;
	//for (i=0;i<8;i++)
		//result+=adv_history[i];
	
	//return result >> 3;	
}
void stm0_init()
{
	//timer interrupt setting
	_stm0c0 = 0x00;
	_stm0c1 = 0xc1;
	_stm0ah = COUNT/256;
	_stm0al = COUNT%256;
	_mf0e = 1;
	_stm0ae = 1;
	_st0on = 1;
}
void update_p()
{
	char i;
	unsigned p_INT = (unsigned) P;
	
	for (i=13;i>=10;i--){
		text[i] = p_INT%10 + '0';
		p_INT/=10;
	}
}
void show_LCD()
{
	/*if (!state)
		LCD_Cmd(0x80);
	else if (state >=1 && state <= 16)
		LCD_Senddata(text[i-1]);
	else if (state == 17)
		LCD_Cmd(0xc0);
	else if (state >= 18 && state <= 33)
		LCD_Senddata()*/
	LCD_Cmd(0x80);
	update_p();
	char i;
	delay_ms(1);
	for (i=0;i<16;i++)
		LCD_Senddata(text[i]);
	
	LCD_Cmd(0xc0);
	delay_ms(1);
	
	if (d) {
		unsigned char tmp = (s-20)/5;
		text2[7] = P_set[tmp][0];
		text2[8] = P_set[tmp][1];
		text2[6] = ' ';
	}
	else {
		text2[6] = off_text[0];
		text2[7] = off_text[1];
		text2[8] = off_text[2];
	}
	for (i = 0; i <16 ; i++)
		LCD_Senddata(text2[i]);
	/*unsigned char gauge = (1 << n1) - 1;
	for (i = 0; i <16 ; i++)
		(gauge & (1 << i))?LCD_Senddata('#'):LCD_Senddata(' ');*/
	
}
/*void delay_ms(unsigned del)
{
	 unsigned int i;
	 for (i=0;i<del;i++)
	 	GCC_DELAY(2000);
}*/
float get_p(unsigned long adv_avg)
{
	if (adv_avg <500)
		return 0;
	else if (adv_avg > 4500)
		return 100;
	else
		return 0.025 * adv_avg - 12.5; //y = 40x + 500
}


void main()
{
	_wdtc = 0xaf;
	
	//timer interrupt setting
	_stm0c0 = 0x00;
	_stm0c1 = 0xc1;
	_stm0ah = COUNT/256;
	_stm0al = COUNT%256;
	_mf0e = 1;
	_stm0ae = 1;
	_st0on = 1;
	
	//LCD setting
	DATA_BUSc = 0;//_pfc = 0;
	_pdc4 = 0;
	_pdc5 = 0;
	_pdc6 = 0;
	_pac1 = 0;
	//adc setting
	_sadc0 = 0x38;
	_sadc1 = 0x07;
	_sadc2 = 0;
	_pds0 = 0x0f;
	_ade = 1;
	
	//pwm setting
	_pcs1 = 0x02;
	//_pes0 = 0x80;
	_pcc4 = 0;
	//_pec3 = 0;
	_ptm1c0 = 0x08;
	_ptm1c1 = 0xA8;
	_ptm1rpl = 1600 & 255;
	_ptm1rph = 1600 >> 8;
	_ptm1al = 800 & 255;
	_ptm1ah = 800 >> 8;
	
	
	key_portc = 0x0f;
	_pbc1 = 0;
	_pbc0 = 0;
	_emi = 1;
	LCD_Reset();
	_start = 1;
	_start = 0;
	delay_ms(1);
	
	
	_pa1 = 1;
	show_LCD();
	while (1){
		if (t_flag){
			_start = 1;
			_start = 0;
			char key_in = key_port & 0x0f;
			if (key_hold == 0x0f){
				key = 0x0f;
				hold = 0;
				key_hold = key_in;
			}
			else if (key_in == key_hold){
				if (!hold){
					hold = 1;
					key = key_hold;
				}
				else {
					key = 0x0f;
				}
				//key = (key_in == key_hold)? key_hold: 0x0f;
			}
			else 
				key_hold = key_in;
			//get_adc(8);
			
			if (key == 0x0e)
				s = (s == 35)? 35:s+5;
			else if (key == 0x0d)
				s = (s == 20)? 20:s-5;
			else if (key == 0x0b)
				d = !d;
			unsigned long adv_avg = update_adv();
			test = adv_avg;
			P = get_p(adv_avg);
			update_p();
			n1 = 8 * adv_avg / 4900;
			//led_port = (1 << n1) - 1;
			//get_adc(9);
			//n2 = (n1 >= 8)? 0:400 * adv / 4900;
			//unsigned long tmp = (unsigned)(400.0 * P / 35);
			//tmp = 400 - tmp;
			n2 += (s-P)*P/32;
			if (n2 > 1600)
				n2 = 1600;
			else if (P > 55 || n2 < 0)
				n2 = 0;
			else if (P < 5)
				n2 = 400;// 回到初始的狀況
			//n2 = n2>=0x8000? 0:n2;
				//n2 = (tmp > 0x8000)? 120: tmp;
			un2 = ((unsigned) n2)*d;
			_ptm1al = un2 & 255;
			_ptm1ah = un2 >> 8;
			
			t_flag=0;
		}
		if (lcd_flag){
			show_LCD();
			lcd_flag = 0;
		}
		IN1 = d;
		IN2 = 0;
		
	}
}
DEFINE_ISR(ISR_STM0, 0x14)
{
	if(!(++c&15)){
		t_flag = 1;
	}
	if (!(c&511))
		lcd_flag = 1;
	_stm0af = 0;
}

DEFINE_ISR(ISR_ADC, 0x1c)
{
	adr = ((unsigned int) _sadoh << 8)| _sadol;
	adv = (unsigned long) adr * 122 / 100;
	//t_flag = 1;
	//t = !t;
	//_sadc0 = 0x30 | 8+t;
	
}