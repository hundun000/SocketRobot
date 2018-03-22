/*-------------------------------------------------------------------------------------
  MAX7219Matrix.h - Library for LED Matrix based on max7219 
  Created by Junjie Tong From China, March 16, 2017.
  
 * copyright:
     (c) Flyelectronic, 2017.
 * version:
     2017-03-16:
       - V1.0.0;
 * Hardware
     Arduino
     Arduino IDE 1.0.5

 * cascade pattern，X axis increase to right，Y axis increase to up

   ......
   n+m+1	n+m+2	n+m+3	......		n+(2*m)
   n+1		n+2		n+3		......		n+m
   1		2		3		......		n

----------------------------------------------------------------------------------------*/

#ifndef _MAX7219MATRIX_H_
#define _MAX7219MATRIX_H_

#include <Arduino.h>

/*---------------------------defines------------------------------------------------*/
#ifndef SET
	#define SET 1
#endif
#ifndef RESET
	#define RESET 0
#endif
//define decode mode，0x00：nonedecode
//						  0x01：decode only digits 0
//						  0x0F :  decode digits 0-3
//						  0xFF :  decode all digits
#define DECODE_MODE_INIT			0x00	
//define scan limit:0x00 to 0x0F,scan 1 digits to 8 digits
#define SCAN_LIMIT_INIT				0x07		
//open or close display comand,0x00:close,0x01:open	
#define SHUT_DOWN_INIT			0x01
//display test comand,0x00:close display test,0x01:open display test
#define DISPLAY_TEST_INIT			0x00		

/*--------------------internal registers define--------------------------------------*/
//define the address of the internal registers
#define DECODE_MODE_REGISTER 	0x09              
#define INTENSITY_REGISTER			0x0A               
#define SCAN_LIMIT_REGISTER 		0x0B            
#define SHUT_DOWN_REGISTER 		0x0C          
#define DISPLAY_TEST_REGISTER 		0x0F              

/*---------------------------class----------------------------------------------------*/
class MAX7219MATRIX
{
	public:
		MAX7219MATRIX(int pin_CS,int pin_SCK,int pin_DIN,unsigned char xnum,unsigned char ynum);
		//初始化点阵模块，需要传递x方向模块数，y方向模块数
		void InitLedMatrix(unsigned char brightness);
		//全屏显示arrPtr所指向的数组
		void DisplayFullScreen(unsigned char *arrPtr);
		void DisplayNColum(unsigned char *arrPtr,unsigned char col,unsigned char n);
		void DisplayNColFromFullScreenArrary(unsigned char *arrPtr,unsigned char col,unsigned char n);
		void SetSinglePixel(unsigned char rowNum,unsigned char colNum,unsigned char *arrPtr,unsigned char setOrRst);
		
		void DISPLAY_TEST(void);											
		void DISPLAY_NORMAL(void);		  								
		void SHUT_DOWN(void);											
		void RE_DISPLAY(void);											
		void SET_INTENSITY(unsigned char level); 						
		void CLEAR_DISPLAY(void);			
		
		void Delay(unsigned char time);
	
	private:
	
		int _pin_CS;			//chip select pin
		int _pin_SCK;			//clock in pin
		int _pin_DIN;			//data in pin
		
		unsigned char ROWNUM;
		unsigned char COLNUM;
		//save how many modules in successive 
		unsigned char TOTALNUM;	
		//define lightness:0x00 to 0x0F,lightness increase												
		unsigned char  INTENSITY_INIT;	
		
		void Init_Max7219(unsigned char num,unsigned char brightness);	//init max7219 with the value defined above
		//write one data to the num'th 7219's internal register
		void Write7219(unsigned char num,unsigned char address,unsigned char dat);
		//display an arrary to the partNum'th module's rNum column
		void ArrDisplay(unsigned char partNum,unsigned char rNum,unsigned char *arrPtr,unsigned char arrLen);
};
#endif