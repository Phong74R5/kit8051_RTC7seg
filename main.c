//Le Hong Phong - 22119210
#include <reg51.h> 	 
#define Display P0
#define chonLED P2 			
sbit SCLK = P3^6;
sbit IO = P3^4;
sbit CE = P3^5;
char CHEDO = 1;
sbit SW = P3^3;

unsigned char code Code7segCatot[] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d,
                                     0x7d, 0x07, 0x7f, 0x6f, 0x77};
//thu tu: giay phut gio nam thang ngay									
unsigned char Time[6] = {0x10, 0x16, 0x23, 0x24, 0x10, 0x01};


// RTC Register Map (Dia chi, Doc/Ghi, Bit, Pham vi)

// 81h-80h: Giay (00�59)
// Bit 7: CH (Clock Halt), Bit 6: 10s, Bits 5�0: Giay

// 83h-82h: Phut (00�59)
// Bit 6: 10m, Bits 5�0: Phut

// 85h-84h: Gio (1�12/0�23)
// Bit 7: 12/24, Bit 6: AM/PM, Bit 5: 10h, Bits 4�0: Gio

// 87h-86h: Ngay (1�31)
// Bit 5: 10d, Bits 4�0: Ngay

// 89h-88h: Thang (1�12)
// Bit 4: 10m, Bits 3�0: Thang

// 8Bh-8Ah: Thu (1�7)
// Bits 2�0: Thu

// 8Dh-8Ch: Nam (00�99)
// Bit 7: 10y, Bits 6�0: Nam

// 8Fh-8Eh: Thanh ghi trang thai
// Bit 7: WP (Write Protect)

// 91h-90h: Thanh ghi dieu khien
// Bits 7�4: TCS, Bits 3�2: DS, Bits 1�0: RS


unsigned char READ[6] = {0x81,0x83,0x85,0x8D,0x89,0x87};
unsigned char WRITE[6] = {0x80,0x82,0x84,0x8C,0x88,0x86};
/*******************************************************************************************************/
// Ham tao do tre bang us
void delay_us(unsigned int us_count)
{  
	while(us_count!=0)
	{
		us_count--;
  }
}
/*******************************************************************************************************/
// Doc du lieu tu RTC
unsigned char RTC_READ(unsigned char add)
{
	unsigned char dat, temp, i;
	CE = 1; // Kich hoat CE
	SCLK = 0;
	delay_us(1);
	for(i = 0;i<8;i++) // Gui dia chi
	{
		IO = add & 0x01;
		add >>= 1;
		SCLK = 1;
		delay_us(1);
		SCLK = 0;
		delay_us(1);
	}
	for(i = 0;i<8;i++) // Nhan du lieu
	{
		temp = IO;
		dat = (dat>>1)|(temp<<7);
		SCLK = 1;
		delay_us(1);
		SCLK = 0;
		delay_us(1);
	}
	delay_us(1);
	CE = 0; // Tat CE
	return dat;
}

// Ghi du lieu vao RTC
void RTC_WRITE(unsigned char add, unsigned char dat)
{
	unsigned char i;
	CE = 1; // Kich hoat CE
	SCLK = 0;
	delay_us(1);
	for(i = 0;i<8;i++) // Gui dia chi
	{
		IO = add & 0x01;
		add >>= 1;
		SCLK = 1;
		delay_us(1);
		SCLK = 0;
		delay_us(1);
	}
	for(i = 0;i<8;i++) // Gui du lieu
	{
		IO = dat & 0x01;
		dat >>= 1;
		SCLK = 1;
		delay_us(1);
		SCLK = 0;
		delay_us(1);
	}
	delay_us(1);
	CE = 0; // Tat CE
}

// Cai dat thoi gian cho RTC
void SetRTC()
{
	unsigned char i;
	RTC_WRITE(0x8e, 0x00); // Tat che do bao ve ghi
	for(i = 0;i< 6;i++)
	{
		RTC_WRITE(WRITE[i],Time[i]); // Ghi thoi gian
	}
	RTC_WRITE(0x8e, 0x80); // Bat lai che do bao ve ghi
}

// Lay thoi gian tu RTC
void GetRTC()
{
	char i;
	for(i = 0;i<6;i++)
	{
		Time[i] = RTC_READ(READ[i]); // Doc thoi gian
	}
}
/**********************************************************************************************************/
// Hien thi thoi gian
void HienThi(unsigned char *Time, unsigned char chedo) {
	unsigned char i, range;
	// range = 0: hien thi gio; range = 3: hien thi ngay
	if(chedo)
		range = 0;
	else
		range = 3;

	for(i = 0;i<7;i+=3) // Hien thi chu so hang chuc va don vi
	{
		chonLED = ((unsigned char)(i)<<2);
		Display = Code7segCatot[Time[i/3 + range] & 0x0f];
		delay_us(100);
		chonLED = ((unsigned char)(i+1)<<2);
		Display = Code7segCatot[(Time[i/3 + range]>>4) & 0x0f];
		delay_us(100);
	}
	// Hien thi dau gach ngang
	chonLED = ((unsigned char)(2)<<2);
	Display = 0x40;
	delay_us(100);
	chonLED = ((unsigned char)(5)<<2);
	Display = 0x40;
	delay_us(100);
}
/*******************************************************************************************************/
// Khoi tao ngat Timer0
void Interrupt_Timer0(void) {
	TMOD = 0x01; // Chon che do 1
	TH0 = 0xFC;  // Gia tri ban dau cua thanh ghi TH0
	TL0 = 0x18;  // Gia tri ban dau cua thanh ghi TL0
	ET0 = 1;     // Cho phep ngat Timer 0
	EA = 1;      // Cho phep ngat toan cuc
	TR0 = 1;     // Bat Timer 0
}

// Ham xu ly ngat Timer
void ISR_TIMER(void) interrupt 1 { 
	if (ET0 == 1) {
		if (SW == 0) { // Neu nut bam duoc nhan
			delay_us(100);
			if (SW == 0) {
				CHEDO++; // Thay doi che do hien thi
			}
			if (CHEDO > 1)
				CHEDO = 0;
		}
		TH0 = 0xFC; // Khoi phuc gia tri ban dau cua Timer
		TL0 = 0x18;
	}
}
/*******************************************************************************************************/
// Chuong trinh chinh
void main() 
{
	SetRTC();           // Cai dat RTC
	Interrupt_Timer0(); // Khoi tao ngat Timer0
	
	// Hien thi thoi gian va ngay lien tuc
	while(1)
	{
		GetRTC();           // Lay thoi gian tu RTC
		HienThi(Time, CHEDO); // Hien thi thoi gian len LED 7 doan
	}
}
/*******************************************************************************************************/
