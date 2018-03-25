/*------------------------------------------------------------------------------------
  LED Matrix based on max7219  demo
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
/************************header files**************************************/

#include<max7219Matrix.h>

#define XNUM 1			 //define how many modules in x axis
#define YNUM 1			 //define how many modules in y axis

//define an empty arrary
unsigned char XY_Arrary[YNUM][XNUM][8] = {0};

unsigned char Table_of_Digits[]={	0x00,0x3e,0x41,0x41,0x41,0x3e,0x00,0x00,		//0 
    				      	0x00,0x00,0x00,0x21,0x7f,0x01,0x00,0x00,		//1     
					0x00,0x27,0x45,0x45,0x45,0x39,0x00,0x00,  	//2     
					0x00,0x22,0x49,0x49,0x49,0x36,0x00,0x00,  	//3     
					0x00,0x0c,0x14,0x24,0x7f,0x04,0x00,0x00,  	//4    
					0x00,0x72,0x51,0x51,0x51,0x4e,0x00,0x00,  	//5  
					0x00,0x3e,0x49,0x49,0x49,0x26,0x00,0x00,  	//6  
					0x00,0x40,0x40,0x40,0x4f,0x70,0x00,0x00,  	//7  
					0x00,0x36,0x49,0x49,0x49,0x36,0x00,0x00,  	//8    
					0x00,0x32,0x49,0x49,0x49,0x3e,0x00,0x00   	//9  
				     }; 
//brightness para,form 0x00 to 0x0F
unsigned char  brightness = 0x04;	
//define some variables
unsigned char i,j=0,k=0,l,m,t;

int	pin_CS = 13;
int	pin_SCK = 12;
int	pin_DIN = 11;

MAX7219MATRIX ledMatrix8x8(pin_CS,pin_SCK,pin_DIN,XNUM,YNUM);
/************************setup*******************************************************/
void setup()
{
	//init matrix
	ledMatrix8x8.InitLedMatrix(brightness);
	
	//all led on
	ledMatrix8x8.DISPLAY_TEST();
	delay(1000);
	//all led off
	ledMatrix8x8.CLEAR_DISPLAY();
	//exit test mode
	ledMatrix8x8.DISPLAY_NORMAL();
	delay(500);

	//display XY_Arrary to the whole screen
	ledMatrix8x8.DisplayFullScreen(&XY_Arrary[0][0][0]);
	delay(500);
	
}
/************************loop*******************************************************/
void loop()
{
	//display XY_Arrary to the whole screen
	ledMatrix8x8.DisplayFullScreen(&XY_Arrary[0][0][0]);
	delay(1000);
	//use SetSinglePixel() to create a amazing pattern
	//the algorithm may be hard to understand, ignore it if so
	//light on LED one by one
	for(i=1,t=0;i<=4;i++)
	{
		//Row++
		for(j=1+t;j<=(8-t);j++)
		{
			ledMatrix8x8.SetSinglePixel(j,i,&XY_Arrary[0][0][0],SET);
			delay(80);
		}
		j--;
		//column++
		for(k=1;k<=8-t-i;k++)
		{
			ledMatrix8x8.SetSinglePixel(j,i+k,&XY_Arrary[0][0][0],SET);
			delay(80);
		}
		k--;
		//Row--
		for(l=1;l<=8-t-i;l++)
		{
			ledMatrix8x8.SetSinglePixel(j-l,i+k,&XY_Arrary[0][0][0],SET);
			delay(80);
		}
		l--;
		t++;
		//Column--
		for(m=1;m<=8-t-i;m++)
		{
			ledMatrix8x8.SetSinglePixel(j-l,i+k-m,&XY_Arrary[0][0][0],SET);
			delay(80);
		}
	}
	//turn off LED one by one
	ledMatrix8x8.SetSinglePixel(4,5,&XY_Arrary[0][0][0],RESET);
	delay(80);
	l=4;
	m=5;
	for(i=1;i<=8;i++)
	{
		for(j=1;j<=i;j++)
		{

			if((i%2) == 1)
			{
				l++;
			}
			else
			{
				l--;
			}
			ledMatrix8x8.SetSinglePixel(l,m,&XY_Arrary[0][0][0],RESET);
			delay(80);
		 }
		 for(j=1;j<=i;j++)
		 {

			if((i%2) == 1)
			{
				m--;
			}
			else
			{
				m++;
			}

			ledMatrix8x8.SetSinglePixel(l,m,&XY_Arrary[0][0][0],RESET);
			delay(80);
		}
	}
	//use DisplayNColum() display 0-9 ten numbers
	for(i = 0;i<10;i++)
	{
		ledMatrix8x8.DisplayNColum((Table_of_Digits+i*8),1,8);
		delay(350);
	}
	//use DisplayNColFromFullScreenArrary() roll display 0-9 ten numbers
	for(i = 0;i<72;i++)
	{
		ledMatrix8x8.DisplayNColFromFullScreenArrary((Table_of_Digits+i),1,8);
		delay(100);
	}
	delay(1000);
}






