#ifndef _LCD__H
#define _LCD__H

#ifndef _HT66F2390_H_
#include "HT66F2390.h"
#endif
#define DATA_BUS _pf
#define DATA_BUSc _pfc

#define RS _pd6
#define RW _pd5
#define EN _pd4

void LCD_Cmd(unsigned char data);
void LCD_Reset();
void LCD_Senddata(unsigned char data);

#endif