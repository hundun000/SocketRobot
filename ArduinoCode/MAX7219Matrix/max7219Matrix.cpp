/*------------------------------------------------------------------------------------
  
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

-------------------------------------------------------------------------------------*/

/************************include header files***************************************/

#include<max7219Matrix.h>

//define a decode arrary, do not change
const unsigned char decodeArr[8]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};

/**************************************************************************************/
MAX7219MATRIX::MAX7219MATRIX(int pin_CS,int pin_SCK,int pin_DIN,unsigned char xnum,unsigned char ynum)
{
	pinMode(pin_CS, OUTPUT);
	pinMode(pin_SCK, OUTPUT);
	pinMode(pin_DIN, OUTPUT);
	
	_pin_CS = pin_CS;
	_pin_SCK = pin_SCK;
	_pin_DIN = pin_DIN;
	
	ROWNUM = ynum * 8;		//计算并保存行数
	COLNUM = xnum * 8;		//计算并保存列数
	TOTALNUM = xnum * ynum;	//计算并保存一共有多少个模块
}
/**************************************************************************************/
void MAX7219MATRIX::InitLedMatrix(unsigned char brightness)
{
	unsigned char i;

	INTENSITY_INIT = brightness;
	
	for(i=1;i<=TOTALNUM;i++)
	{
		Write7219(i,SHUT_DOWN_REGISTER,SHUT_DOWN_INIT);			
		Write7219(i,DISPLAY_TEST_REGISTER,DISPLAY_TEST_INIT);	
		Write7219(i,DECODE_MODE_REGISTER,DECODE_MODE_INIT);		
		Write7219(i,SCAN_LIMIT_REGISTER,SCAN_LIMIT_INIT);		
		Write7219(i,INTENSITY_REGISTER,INTENSITY_INIT);		
	}
}

/**************************************************************************************/
void MAX7219MATRIX::DISPLAY_TEST(void)
{
	unsigned char i;
	for(i=1;i<=TOTALNUM;i++) 
	{
		Write7219(i,DISPLAY_TEST_REGISTER,0x01);
	}
}
/**************************************************************************************/
void MAX7219MATRIX::DISPLAY_NORMAL(void)
{
	unsigned char i;
	for(i=1;i<=TOTALNUM;i++) 
	{
		Write7219(i,DISPLAY_TEST_REGISTER,0x00);
	}
}
/**************************************************************************************/
void MAX7219MATRIX::SHUT_DOWN(void)
{
	unsigned char i;
	for(i=1;i<=TOTALNUM;i++) 
	{
		Write7219(i,SHUT_DOWN_REGISTER,0x00);
	}
}
/**************************************************************************************/
void MAX7219MATRIX::RE_DISPLAY(void)
{
	unsigned char i;
	for(i=1;i<=TOTALNUM;i++) 
	{
		Write7219(i,SHUT_DOWN_REGISTER,0x01);
	}
}

/**************************************************************************************/
void MAX7219MATRIX::SET_INTENSITY(unsigned char level)
{
	unsigned char i;
	for(i=1;i<=TOTALNUM;i++) 
	{
		Write7219(i,INTENSITY_REGISTER,level);
	}
}
/**************************************************************************************/
void MAX7219MATRIX::CLEAR_DISPLAY(void)
{
	unsigned char i,j;
	for(i=1;i<=TOTALNUM;i++) 
	{
		for(j=1;j<=8;j++)
		{
			Write7219(i,j,0x00);
		}
	}
}

/*************************************************************************************
*function:void Write7219(unsigned char address,unsigned char dat)
*		  write one data to the num'th 7219's internal register:
*parameters：num——form 1、2、3...
	   		    address——comand register address or display address
	   		    dat——anydata
**************************************************************************************/
void MAX7219MATRIX::Write7219(unsigned char num,unsigned char address,unsigned char dat)
{
	unsigned char i,j;
	digitalWrite(_pin_CS , LOW);						

	for(i=0;i<(TOTALNUM-num);i++)
		for(j=0;j<16;j++)
		{
			//CLK=0;			
			//DI=0;
			//CLK=1;
			digitalWrite(_pin_SCK , LOW);
			digitalWrite(_pin_DIN , LOW);
			digitalWrite(_pin_SCK , HIGH);
		}

	for (i=0;i<8;i++) 		
	{
		//CLK=0;	
		digitalWrite(_pin_SCK , LOW);			
		if((address &0x80))	
		{
			digitalWrite(_pin_DIN , HIGH);
		}
		else
		{
			digitalWrite(_pin_DIN , LOW);
		}

		address <<=1; 		
		digitalWrite(_pin_SCK , HIGH);				
	}

	for (i=0;i<8;i++)
	{
		//CLK=0;
		digitalWrite(_pin_SCK , LOW);	
		if((dat&0x80))
		digitalWrite(_pin_DIN , HIGH);
		else
		digitalWrite(_pin_DIN , LOW);
		dat <<=1; 
		digitalWrite(_pin_SCK , HIGH);	
	}
	for(i=0;i<(num-1);i++)
		for(j=0;j<16;j++)
		{
			digitalWrite(_pin_SCK , LOW);
			digitalWrite(_pin_DIN , LOW);
			digitalWrite(_pin_SCK , HIGH);
		}

	digitalWrite(_pin_CS , HIGH);					
}
/*************************************************************************************
*Function:ArrDisplay(unsigned char partNum,unsigned char rNum,unsigned char *arrPtr,unsigned char arrLen)
*		 ：//display an arrary to the partNum'th module's rNum column
*para	：partNum——from 1 to .....
	   	   rNum——1 to 8
	   	   arrPtr——pointer to arrary
	   	   arrLen——length of the arrary
**************************************************************************************/
void MAX7219MATRIX::ArrDisplay(unsigned char partNum,unsigned char rNum,unsigned char *arrPtr,unsigned char arrLen)
{
	unsigned char j;
	unsigned char *p;
	unsigned char address=0;
	unsigned char temp;
	unsigned char num;

	p = arrPtr;

	if(arrLen<=0)			
	{
		return;
	}

	for(j=0;j<arrLen;j++)
	{						
		address=(j%8)+rNum;		
		temp=(*(p++));
		num=(j>>3)+partNum;

		Write7219(num,address,temp);
	}
}

/**************************************************************************************/
void MAX7219MATRIX::DisplayFullScreen(unsigned char *arrPtr)
{
	ArrDisplay(1,1,arrPtr,(TOTALNUM*8));
}
/*************************************************************************************
void MAX7219MATRIX::DisplayNColum(unsigned char *arrPtr,unsigned char col,unsigned char n)
*function：display n column to board from the column of col of board, module style 2
*para：  col——from to COLNUM
		 n——from to COLNUM
**************************************************************************************/
void MAX7219MATRIX::DisplayNColum(unsigned char *arrPtr,unsigned char col,unsigned char n)
{
	unsigned char i,j;
	unsigned char pNum,colNum;

	for(j=0;j<n;j++)
	{
		if(col <= COLNUM)
		{
			for(i=0;i<(ROWNUM/8);i++)
			{
				pNum = ((col-1)/8+1)+(i*(COLNUM/8));
				colNum = ((col-1)%8)+1;
				ArrDisplay(pNum,colNum,arrPtr,1);
				arrPtr++;
			}
		}
		col++;
	}
}
/*************************************************************************************
void MAX7219MATRIX::DisplayNColFromFullScreenArrary(unsigned char *arrPtr,unsigned char col,unsigned char n)
*function：display n column from arrary to board begin form board's col column
*para：arrPtr——arrary address
		 col——from 1 to COLNUM
		 n——from 1 to COLNUM
**************************************************************************************/
void MAX7219MATRIX::DisplayNColFromFullScreenArrary(unsigned char *arrPtr,unsigned char col,unsigned char n)
{
	unsigned char i,j;
	unsigned char pNum,colNum;
	unsigned char *p;
	
	for(j=0;j<n;j++)
	{
		if(col <= COLNUM)
		{
			for(i=0;i<(ROWNUM/8);i++)
			{
				pNum = ((col-1)/8+1)+(i*(COLNUM/8));
				colNum = ((col-1)%8)+1;
				p =  arrPtr+(i*COLNUM)+(col-1);
				ArrDisplay(pNum,colNum,p,1);
			}
		}
		col++;
	}
}
/**************************************************************************************/
void MAX7219MATRIX::SetSinglePixel(unsigned char rowNum,unsigned char colNum,unsigned char *arrPtr,unsigned char setOrRst)
{
	unsigned char pNum,cNum,bNum;	
	unsigned char *p;		   
	unsigned char temp;

	if((rowNum==0)||(colNum==0)||(rowNum>ROWNUM)||(colNum>COLNUM))
	{							 
		return;
	}
	pNum = (((rowNum-1)/8)*(COLNUM/8))+((colNum-1)/8)+1;
	cNum = ((colNum-1)%8)+1;
	p = arrPtr+((pNum-1)*8)+(cNum-1);
	temp = *p;
	bNum = 	((rowNum-1)%8);
	if(setOrRst == SET)
	{
		temp |= decodeArr[bNum];
	}
	else if(setOrRst == RESET)
	{
		temp &= ~decodeArr[bNum];
	}
	*p = temp;
	ArrDisplay(pNum,cNum,p,1);	
}
/**************************************************************************************/
void MAX7219MATRIX::Delay(unsigned char time)
{
	int i,j,k;
	for(i=0;i<time;i++)
		for(j=0;j<50;j++)
			for(k=0;k<150;k++)
			{
				;
			}
}
