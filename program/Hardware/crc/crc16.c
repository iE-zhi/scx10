/*
CRC16 MODBUS校验，多项式0xa001
*/
#include "crc16.h"

unsigned short CRC16(char src_data[])
{
	unsigned short wcrc = 0xffff;
  unsigned int len=0;
  char i = 0,j = 0;
  len=strlen(src_data);
  for( i = 0;i < len-2;i++ )
  {
		wcrc ^= src_data[i];
    for( j = 0; j < 8;j++ )
    {
			if( wcrc & 0x0001 )
			{
				wcrc >>= 1;
				wcrc ^= 0xa001;
			}
			else
				wcrc >>= 1;
		}
	}
  return wcrc;
}
