/*
  Copyright (c) 2020 LHW - createskyblue
  Arduino-Pomodoro is licensed under Mulan PSL v2.
  You can use this software according to the terms and conditions of the Mulan PSL v2.
  You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
  THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
  EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
  MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
  See the Mulan PSL v2 for more details.

  光电门下位机程序版本:
  V2.0
*/
#include <U8g2lib.h>
//#include <SPI.h>
//U8G2_ST7565_JLX12864_F_4W_SW_SPI u8g2(U8G2_R0,19,20,14,22,21);
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* clock=*/ PB6, /* data=*/ PB7, /* reset=*/ U8X8_PIN_NONE);
#define TmpDateMax 64
int DATA[2][TmpDateMax]; //目标传感器|历史数据
bool TriggerTimer = false;
bool GUIState = false;
bool State[2] = {1, 1}; //传感器状态
int grap[2][17]; //图形缓冲区
byte GrapX;
int DataDeal[2][2]; //目标传感器|累加&去抖均值
unsigned long timestamp[2][3]; //目标传感器|电平突变起始&电平突变结束&突变维持时间
unsigned long deltaT; //A,B传感器从起始到结束的时间差
unsigned long GrapSampling; //波形捕获时间戳
#define SysVersion 2.0
#define MutationThreshold 100
#define SamplingRate 10
#define PI 3.141592654
byte io[2] = {0, 1};
byte a = 0;
int LightIntensity = 0;
// width: 128, height: 64
const unsigned char Logo1[] PROGMEM = { 0xff, 0xbb, 0x56, 0x21, 0x00, 0x04, 0x81, 0xdb, 0xb6, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfb, 0xbb, 0x84, 0x24, 0x40, 0x48, 0xb6, 0x6d, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xef, 0x56, 0x10, 0x80, 0x10, 0x80, 0x6a, 0xdb, 0xfe, 0x00, 0x00, 0x00, 0xfe, 0xff, 0xff, 0xff, 0xbf, 0x2b, 0x05, 0x21, 0x02, 0x08, 0xdd, 0xb6, 0x7f, 0xff, 0xff, 0xff, 0xfd, 0xff, 0xff, 0xff, 0xfb, 0x5b, 0x20, 0x08, 0x80, 0x00, 0xa8, 0x6d, 0x7f, 0xef, 0x7f, 0xc7, 0xfd, 0xff, 0xff, 0xff, 0x6f, 0x2b, 0x01, 0x00, 0x08, 0x48, 0xda, 0x5a, 0x7f, 0x83, 0x7d, 0xd7, 0xfd, 0xff, 0xff, 0xff, 0xdf, 0x15, 0x92, 0x84, 0x20, 0x02, 0x68, 0xb7, 0x7e, 0xef, 0x7e, 0xd7, 0xfd, 0xff, 0xff, 0xff, 0x2b, 0x55, 0x00, 0x10, 0x0a, 0x00, 0xd2, 0xaa, 0x7e, 0xc7, 0x3b, 0xd4, 0xfd, 0xff, 0xff, 0xff, 0x57, 0x8d, 0x24, 0xa5, 0x40, 0x91, 0x50, 0x55, 0x7f, 0xff, 0x7c, 0xd5, 0xfd, 0xff, 0xff, 0xff, 0x27, 0x09, 0x80, 0x54, 0x1b, 0x02, 0x20, 0xa9, 0x7f, 0xc7, 0x7b, 0xd5, 0xfd, 0xff, 0xff, 0xff, 0x57, 0xa5, 0x2a, 0xb5, 0x6a, 0x49, 0x44, 0xab, 0x7e, 0xc7, 0x7f, 0xd5, 0xfd, 0xff, 0xff, 0xff, 0x93, 0x0a, 0x44, 0xab, 0x95, 0x02, 0xa1, 0x76, 0x7f, 0xd7, 0x7b, 0xd5, 0xfd, 0xff, 0xff, 0xff, 0x57, 0x85, 0xa8, 0xda, 0xaa, 0x14, 0xc8, 0x8a, 0x7e, 0x81, 0xbc, 0xc5, 0xfd, 0xff, 0xff, 0xff, 0xab, 0x12, 0x55, 0xb7, 0x55, 0x89, 0x20, 0x55, 0x7f, 0xff, 0xff, 0xff, 0xfd, 0xff, 0xff, 0xff, 0xd7, 0x26, 0xda, 0xda, 0xae, 0x12, 0x44, 0x52, 0x7e, 0xff, 0xff, 0xff, 0xfd, 0xff, 0xff, 0xff, 0x5b, 0x85, 0xaa, 0xb7, 0x55, 0x55, 0x40, 0x95, 0x7e, 0xff, 0xff, 0xff, 0xfd, 0xff, 0xff, 0xff, 0xbf, 0x2a, 0x75, 0x7d, 0xab, 0x8a, 0x88, 0xa4, 0x7e, 0xbf, 0xbf, 0xf7, 0xfd, 0xff, 0xff, 0xff, 0xff, 0x83, 0xaa, 0xd7, 0xb6, 0x2a, 0xa2, 0x2a, 0x7f, 0xbf, 0xbf, 0xf7, 0xfd, 0xff, 0xff, 0xff, 0xff, 0x2a, 0x6d, 0xad, 0x55, 0x55, 0x80, 0xa4, 0x7e, 0xbf, 0x7f, 0xf7, 0xfd, 0xff, 0xff, 0xff, 0xff, 0x85, 0xda, 0x76, 0xad, 0x95, 0x48, 0x49, 0x7f, 0x07, 0xfe, 0xc1, 0xfd, 0xff, 0xff, 0xff, 0xff, 0x0a, 0x55, 0xaf, 0x6b, 0x55, 0x82, 0x2a, 0x7e, 0xb7, 0xbe, 0xd5, 0xfd, 0xff, 0xff, 0xff, 0xff, 0xa5, 0xb6, 0xdd, 0x5a, 0x95, 0x50, 0x52, 0x7f, 0x07, 0x7e, 0xc1, 0xfd, 0xff, 0xff, 0xff, 0x7f, 0x0b, 0xd5, 0xb6, 0x55, 0x55, 0x80, 0xa4, 0x7e, 0xbf, 0xff, 0xd5, 0xfd, 0xff, 0xff, 0xff, 0xdf, 0x2a, 0x6d, 0x6b, 0xbb, 0x2a, 0xaa, 0x4a, 0x7e, 0xbf, 0x7f, 0xd5, 0xfd, 0xff, 0xff, 0xff, 0x7f, 0x8b, 0xda, 0xde, 0xd6, 0x4a, 0x20, 0x91, 0x7e, 0xbf, 0x3f, 0xc1, 0xfd, 0xff, 0xff, 0xff, 0xf7, 0x1a, 0x25, 0x55, 0xb5, 0xaa, 0x44, 0x56, 0x7f, 0xff, 0xff, 0xff, 0xfd, 0xff, 0xff, 0xff, 0x7f, 0xab, 0x5a, 0xa9, 0x2d, 0x44, 0xa8, 0x52, 0x7e, 0xff, 0xff, 0xff, 0xfd, 0xff, 0xff, 0xff, 0xff, 0x16, 0xab, 0x6a, 0x4b, 0xa9, 0xa8, 0xea, 0x7e, 0xff, 0xff, 0xff, 0xfd, 0xff, 0xff, 0xff, 0x6f, 0x5d, 0x6d, 0x55, 0x95, 0x96, 0x5c, 0xf5, 0x7f, 0xff, 0xff, 0xff, 0xfd, 0xff, 0xff, 0xff, 0xff, 0x12, 0xd1, 0xaa, 0x52, 0x55, 0xf4, 0x62, 0x7f, 0x8f, 0xff, 0xff, 0xfd, 0xff, 0xff, 0xff, 0xbf, 0x0d, 0x20, 0xd4, 0xd5, 0xaa, 0xbe, 0xed, 0x7f, 0x8f, 0xff, 0xff, 0xfd, 0xff, 0xff, 0xff, 0x7b, 0x6b, 0x4d, 0xa1, 0x2a, 0xa9, 0x74, 0xf5, 0x7e, 0x8f, 0xff, 0xff, 0xfd, 0xff, 0xff, 0xff, 0xbf, 0x9a, 0x9a, 0x8a, 0x05, 0x00, 0x7a, 0xed, 0x7f, 0x8f, 0xff, 0xff, 0xfd, 0xff, 0xff, 0xff, 0xff, 0x96, 0x77, 0x1d, 0xd0, 0xb6, 0xb8, 0x6a, 0x7f, 0x8f, 0xff, 0xff, 0xfd, 0xff, 0xff, 0xff, 0xbf, 0xad, 0xaa, 0xaa, 0xa5, 0xaa, 0xbb, 0x95, 0x7e, 0x8f, 0xff, 0xff, 0xfd, 0xff, 0xff, 0xff, 0x7f, 0x2b, 0x6d, 0x87, 0xb1, 0x6a, 0x79, 0xab, 0x7e, 0x9f, 0xff, 0xff, 0xfd, 0xff, 0xff, 0xff, 0xbf, 0x1a, 0xdb, 0x6a, 0x63, 0xdb, 0x7e, 0x56, 0x7f, 0xff, 0xff, 0xff, 0xfd, 0xff, 0xff, 0xff, 0x77, 0x75, 0xb6, 0xc5, 0xaa, 0x56, 0xdf, 0xaa, 0x7e, 0x8f, 0xff, 0xff, 0xfd, 0xff, 0xff, 0xff, 0xdf, 0xee, 0xa9, 0x6a, 0x43, 0x49, 0xbf, 0x96, 0x7e, 0x8f, 0xff, 0xff, 0xfd, 0xff, 0xff, 0xff, 0xbf, 0x55, 0xab, 0xda, 0x5e, 0xf5, 0xde, 0x7a, 0x07, 0xff, 0xff, 0xff, 0xfd, 0xff, 0xff, 0xff, 0x5b, 0xfb, 0xb6, 0xea, 0x51, 0x4d, 0xfb, 0xff, 0xf7, 0xff, 0xff, 0xff, 0xfd, 0xff, 0xff, 0xff, 0xff, 0xae, 0xed, 0xb6, 0xd6, 0xda, 0xef, 0xef, 0xef, 0xff, 0xff, 0xff, 0xfd, 0xff, 0xff, 0xff, 0x9f, 0x75, 0x5b, 0xed, 0xad, 0xd5, 0xbe, 0x7d, 0xdf, 0xff, 0xff, 0xff, 0xfd, 0xff, 0xff, 0xff, 0x7b, 0xdb, 0xb6, 0x5b, 0xab, 0xb6, 0xfb, 0xef, 0xdf, 0xff, 0xff, 0xff, 0xfd, 0xff, 0xff, 0xff, 0xaf, 0xfe, 0x6d, 0xd5, 0xda, 0xed, 0x6f, 0x7d, 0xbf, 0xff, 0xff, 0xff, 0xfd, 0xff, 0xff, 0xff, 0xdf, 0x55, 0xdb, 0xae, 0x52, 0xb5, 0xda, 0xd7, 0x7f, 0x00, 0x00, 0x00, 0xfe, 0xff, 0xff, 0xff, 0xab, 0xee, 0xb6, 0xb5, 0xaa, 0xf6, 0xbd, 0x56, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xbf, 0xbd, 0xab, 0xae, 0xd5, 0x5a, 0x77, 0xa9, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xd7, 0xd6, 0x6e, 0x75, 0xad, 0xf6, 0x5b, 0x45, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xbf, 0xfb, 0x5b, 0x45, 0xb5, 0x6d, 0x57, 0xfd, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x6f, 0x57, 0x57, 0xb5, 0xa2, 0xfa, 0xea, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xef, 0xfe, 0x68, 0x6b, 0xad, 0xbe, 0xf7, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x5b, 0xd5, 0xaa, 0x56, 0x69, 0xed, 0xfd, 0xff, 0xff, 0xff, 0xff, 0xbf, 0xff, 0xf3, 0xff, 0xff, 0xdf, 0x6f, 0xd8, 0xaa, 0x56, 0xfd, 0xf3, 0xe3, 0xf1, 0x78, 0xfc, 0xbe, 0xfd, 0xef, 0xff, 0xff, 0xb7, 0x3a, 0xb5, 0xb6, 0xaa, 0xba, 0xef, 0xdd, 0x66, 0xb7, 0xf9, 0xbd, 0xfd, 0xff, 0xff, 0xff, 0xef, 0xb7, 0xa8, 0x6d, 0xb5, 0xfc, 0xf7, 0xdd, 0x66, 0xb7, 0xf9, 0x3d, 0x70, 0x00, 0xfc, 0xff, 0xb7, 0x2e, 0x5a, 0x55, 0x4b, 0xf2, 0xff, 0xdd, 0x66, 0xb7, 0xf9, 0xdb, 0x7d, 0x7e, 0xff, 0xff, 0x6f, 0x4b, 0x74, 0xab, 0x5a, 0xf9, 0xff, 0xdf, 0xe6, 0xb7, 0x79, 0xdf, 0xfd, 0xbe, 0xff, 0xff, 0xb7, 0x9f, 0xaa, 0xaa, 0x92, 0xb2, 0xef, 0xdf, 0xea, 0xb7, 0xfa, 0x7e, 0xf0, 0xdd, 0xff, 0xff, 0xef, 0x3b, 0xa8, 0x55, 0x55, 0xea, 0xff, 0xef, 0xec, 0x3b, 0xfb, 0xf6, 0xff, 0xeb, 0xff, 0xff, 0xfb, 0xae, 0x52, 0x5b, 0xa5, 0xf0, 0xee, 0xf7, 0xec, 0x3d, 0xfb, 0x3b, 0xf0, 0xf3, 0xff, 0xff, 0xbf, 0x5b, 0xb4, 0x56, 0x2d, 0xab, 0xff, 0xfb, 0xec, 0x3e, 0xfb, 0xbd, 0xf7, 0xed, 0xff, 0xff, 0xf7, 0x3f, 0xc1, 0x6a, 0xd5, 0xe4, 0xff, 0xfd, 0x6c, 0x3f, 0xfb, 0xbe, 0x77, 0x9e, 0xff, 0xff, 0xdb, 0xaa, 0xa4, 0x5a, 0xab, 0xd2, 0xf7, 0xc1, 0x71, 0x70, 0xfc, 0x3e, 0xb0, 0x7f, 0xfe };
const unsigned char Logo2[] PROGMEM = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xfb, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xdf, 0x9d, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xbf, 0x9d, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xcf, 0xb3, 0xfd, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x9f, 0xb9, 0x9d, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x3f, 0xbc, 0x9d, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0xbe, 0x9d, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x3f, 0xbc, 0x9d, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x9f, 0xb9, 0x9d, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xcf, 0xb3, 0xfd, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xbf, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xfd, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xbf, 0xfd, 0xff, 0xff, 0xff, 0xff, 0xf9, 0x0f, 0x00, 0xfe, 0xf3, 0xe7, 0xe1, 0x3f, 0xff, 0xe4, 0xbc, 0xfd, 0xff, 0xff, 0xff, 0xff, 0xfc, 0xcf, 0x7f, 0x06, 0xe7, 0xe7, 0xc9, 0x3f, 0xff, 0x49, 0xbe, 0xfd, 0xff, 0xff, 0xff, 0x1f, 0x00, 0x0c, 0x00, 0x26, 0xff, 0x07, 0xc9, 0x3f, 0xff, 0x3f, 0xbf, 0xfd, 0xff, 0xff, 0xff, 0x9f, 0xff, 0xcc, 0x79, 0x26, 0x00, 0xe7, 0xf9, 0x3f, 0x3f, 0x00, 0xb0, 0xfd, 0xff, 0xff, 0xff, 0x9f, 0xff, 0xcc, 0x79, 0x86, 0xf9, 0xe7, 0x81, 0x3f, 0x3f, 0xff, 0xb3, 0xfd, 0xff, 0xff, 0xff, 0x9f, 0x80, 0x4c, 0x40, 0x86, 0xf9, 0x67, 0xf8, 0x00, 0x00, 0xff, 0xb3, 0xfd, 0xff, 0xff, 0xff, 0x9f, 0x9c, 0xcc, 0x79, 0x26, 0x81, 0x01, 0xc9, 0x3f, 0xff, 0x00, 0xbe, 0xfd, 0xff, 0xff, 0xff, 0x9f, 0x9c, 0xcc, 0x61, 0x26, 0x99, 0x39, 0xc9, 0x3f, 0xff, 0x3f, 0xbf, 0xfd, 0xff, 0xff, 0xff, 0x9f, 0x9c, 0xcc, 0x49, 0x26, 0x99, 0x39, 0xe1, 0x3f, 0xff, 0x8f, 0xbf, 0xfd, 0xff, 0xff, 0xff, 0x9f, 0x80, 0x0c, 0x00, 0x26, 0x99, 0x39, 0xe1, 0x3f, 0x3f, 0x00, 0xb0, 0xfd, 0xff, 0xff, 0xff, 0x9f, 0xff, 0xcc, 0x7f, 0x86, 0x99, 0x39, 0x93, 0x3f, 0xff, 0xcf, 0xbf, 0xfd, 0xff, 0xff, 0xff, 0x9f, 0xff, 0xcc, 0x7f, 0xe6, 0x9c, 0x01, 0x81, 0x3f, 0xff, 0xcf, 0xbf, 0xfd, 0xff, 0xff, 0xff, 0x9f, 0xff, 0x0c, 0x00, 0xe6, 0x9c, 0x39, 0x8c, 0x3f, 0xff, 0xcf, 0xbf, 0xfd, 0xff, 0xff, 0xff, 0x9f, 0x3f, 0xcc, 0x7f, 0x66, 0xc6, 0x7f, 0x9e, 0x01, 0xe0, 0xc3, 0xbf, 0xfd, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xbf, 0xfd, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xbf, 0xfd, 0xff, 0xff, 0xfc, 0x7f, 0xce, 0x9f, 0xcf, 0xff, 0xf9, 0x7f, 0xfe, 0x3f, 0xe7, 0xff, 0xbf, 0x7d, 0x00, 0xf8, 0xfc, 0x7f, 0xce, 0x3f, 0xcf, 0xff, 0xf0, 0xff, 0xfc, 0x67, 0x66, 0x10, 0xb8, 0xfd, 0xff, 0xf9, 0xfc, 0x7f, 0x02, 0xfc, 0xcf, 0x7f, 0xe6, 0x07, 0x80, 0x67, 0xe6, 0x93, 0xb9, 0xfd, 0xff, 0x19, 0x00, 0x78, 0xcc, 0x0c, 0xce, 0x3f, 0xcf, 0xff, 0xfc, 0xcf, 0xf3, 0x93, 0xb9, 0xfd, 0xf9, 0xf9, 0x84, 0x19, 0xc8, 0x7c, 0xce, 0x9f, 0x9f, 0xff, 0xfc, 0xcf, 0x73, 0x10, 0xb8, 0xfd, 0xf3, 0xf9, 0x84, 0x19, 0xce, 0x3c, 0x01, 0x06, 0x00, 0xfe, 0xfc, 0x9f, 0x79, 0x7e, 0xbe, 0xfd, 0x33, 0xf9, 0x84, 0x19, 0xce, 0x3c, 0xcf, 0xff, 0xff, 0xff, 0xfc, 0x9f, 0x79, 0x0e, 0xb0, 0xfd, 0x8f, 0xf9, 0x84, 0x79, 0x00, 0x18, 0xce, 0xff, 0xff, 0x0f, 0xc0, 0x3f, 0x7c, 0x4e, 0xb2, 0xfd, 0xe3, 0x79, 0x86, 0x79, 0x0e, 0x0f, 0xcc, 0x0f, 0x00, 0xff, 0xfc, 0x7f, 0x7e, 0x40, 0xb2, 0xfd, 0xf8, 0x79, 0x86, 0x79, 0x0e, 0x3f, 0xcf, 0xff, 0xfc, 0xff, 0xfc, 0x3f, 0xfc, 0x03, 0xb0, 0x3d, 0xfe, 0x79, 0x86, 0x79, 0x66, 0x3e, 0xcf, 0x7f, 0xe6, 0xff, 0xfc, 0x9f, 0xf9, 0x73, 0xbe, 0xfd, 0xff, 0x39, 0x07, 0x78, 0x66, 0x3e, 0xcf, 0x3f, 0xcf, 0xff, 0xfc, 0xcf, 0xf3, 0x73, 0xb2, 0xfd, 0xff, 0x3c, 0x87, 0x79, 0xf2, 0x3c, 0xcf, 0x1f, 0x80, 0xff, 0xfc, 0xe7, 0xe7, 0x03, 0xa0, 0xfd, 0x3f, 0x9e, 0xf1, 0x7f, 0xf8, 0x39, 0x00, 0xfc, 0x9f, 0x03, 0x00, 0xf1, 0x8f, 0xf8, 0xa7, 0xfd, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xbf, 0xfd, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xbf, 0x1d, 0x00, 0xfc, 0x9f, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xbf, 0x9d, 0xff, 0xfc, 0x9f, 0x7f, 0x06, 0xfe, 0xf1, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xbf, 0x1d, 0x00, 0x1c, 0x00, 0x7c, 0x66, 0xfe, 0xf1, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xbf, 0x9d, 0xf3, 0xfc, 0x9c, 0xfc, 0x67, 0xfe, 0xf1, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xbf, 0x9d, 0xf3, 0x7c, 0x00, 0xf8, 0x67, 0xfe, 0xf1, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xbf, 0x9d, 0x80, 0x7c, 0x9e, 0x1c, 0xf2, 0xf8, 0xf1, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xbf, 0x9d, 0xf3, 0x3c, 0x03, 0x7c, 0xf8, 0xff, 0xf1, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xbf, 0x9d, 0xc3, 0x3c, 0x9c, 0x7f, 0x02, 0xfc, 0xf1, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xbf, 0x9d, 0x93, 0xfc, 0x00, 0x7c, 0xe6, 0xfc, 0xf1, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xbf, 0x1d, 0x00, 0x3c, 0x9c, 0x7f, 0x66, 0xfe, 0xf1, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xbf, 0x9d, 0xff, 0x7c, 0x00, 0x78, 0x08, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xbf, 0x9d, 0xff, 0x7c, 0x9e, 0x7f, 0x9c, 0xff, 0xf1, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xbf, 0x1d, 0x00, 0x3c, 0x9c, 0x7f, 0x06, 0xfe, 0xf1, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xbf, 0x9d, 0xff, 0x9c, 0x01, 0xf8, 0xf1, 0xf8, 0xf1, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xbf, 0xfd, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xbf, 0xfb, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xdf, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
//60x60
const unsigned char QRCode[] PROGMEM = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0f, 0x1f, 0x00, 0xf8, 0x1f, 0xe0, 0x01, 0x80, 0x0f, 0x1f, 0x00, 0xf8, 0x1f, 0xe0, 0x01, 0x80, 0x0f, 0x1f, 0x00, 0x18, 0x86, 0x87, 0x01, 0x80, 0x0f, 0x1f, 0xff, 0x19, 0x86, 0x87, 0xf1, 0x9f, 0x0f, 0x1f, 0x01, 0x79, 0x80, 0x99, 0x11, 0x90, 0x0f, 0x1f, 0x01, 0x79, 0x80, 0x99, 0x11, 0x90, 0x0f, 0x1f, 0x01, 0x79, 0xe0, 0x9f, 0x11, 0x90, 0x0f, 0x1f, 0x01, 0x79, 0xe0, 0x9f, 0x11, 0x90, 0x0f, 0x1f, 0x01, 0x99, 0x81, 0xe7, 0x11, 0x90, 0x0f, 0x1f, 0x01, 0x99, 0x81, 0xe7, 0x11, 0x90, 0x0f, 0x1f, 0x01, 0xf9, 0x01, 0x80, 0x11, 0x90, 0x0f, 0x1f, 0xff, 0xf9, 0x01, 0x80, 0xf1, 0x9f, 0x0f, 0x1f, 0x00, 0x98, 0x99, 0x99, 0x01, 0x80, 0x0f, 0x1f, 0x00, 0x98, 0x99, 0x99, 0x01, 0x80, 0x0f, 0xff, 0xff, 0xff, 0x07, 0x86, 0xff, 0xff, 0x0f, 0xff, 0xff, 0xff, 0x07, 0x86, 0xff, 0xff, 0x0f, 0x9f, 0x99, 0xf9, 0xff, 0x9f, 0x9f, 0xe7, 0x0f, 0x9f, 0x99, 0xf9, 0xff, 0x9f, 0x9f, 0xe7, 0x0f, 0x9f, 0xff, 0x07, 0xf8, 0x87, 0xe1, 0x9f, 0x0f, 0x9f, 0xff, 0x07, 0xf8, 0x87, 0xe1, 0x9f, 0x0f, 0x1f, 0x86, 0x79, 0xfe, 0xff, 0xf9, 0x81, 0x0f, 0x1f, 0x86, 0x79, 0xfe, 0xff, 0xf9, 0x81, 0x0f, 0x1f, 0x60, 0x7e, 0x1e, 0x18, 0xe0, 0xe7, 0x0f, 0x1f, 0x60, 0x7e, 0x1e, 0x18, 0xe0, 0xe7, 0x0f, 0x7f, 0x80, 0x81, 0x01, 0x86, 0x61, 0x86, 0x0f, 0x7f, 0x80, 0x81, 0x01, 0x86, 0x61, 0x86, 0x0f, 0x7f, 0x9e, 0x9f, 0x01, 0xf8, 0x79, 0x9e, 0x0f, 0x7f, 0x9e, 0x9f, 0x01, 0xf8, 0x79, 0x9e, 0x0f, 0x9f, 0x67, 0x00, 0x06, 0x9e, 0x81, 0x81, 0x0f, 0x9f, 0x67, 0x00, 0x06, 0x9e, 0x81, 0x81, 0x0f, 0x7f, 0x80, 0x7f, 0x1e, 0x98, 0x61, 0xe6, 0x0f, 0x7f, 0x80, 0x7f, 0x1e, 0x98, 0x61, 0xe6, 0x0f, 0x9f, 0x61, 0xe0, 0xe1, 0x01, 0x00, 0xfe, 0x0f, 0x9f, 0x61, 0xe0, 0xe1, 0x01, 0x00, 0xfe, 0x0f, 0xff, 0xff, 0x1f, 0xe0, 0x87, 0x1f, 0x80, 0x0f, 0xff, 0xff, 0x1f, 0xe0, 0x87, 0x1f, 0x80, 0x0f, 0x1f, 0x00, 0xf8, 0x7f, 0x86, 0x99, 0x87, 0x0f, 0x1f, 0x00, 0xf8, 0x7f, 0x86, 0x99, 0x87, 0x0f, 0x1f, 0x00, 0x78, 0xe6, 0x9f, 0x1f, 0xfe, 0x0f, 0x1f, 0xff, 0x79, 0xe6, 0x9f, 0x1f, 0xfe, 0x0f, 0x1f, 0x01, 0x19, 0x98, 0x1f, 0x80, 0x87, 0x0f, 0x1f, 0x01, 0x19, 0x98, 0x1f, 0x80, 0x87, 0x0f, 0x1f, 0x01, 0xf9, 0x01, 0x1e, 0x9e, 0xf9, 0x0f, 0x1f, 0x01, 0xf9, 0x01, 0x1e, 0x9e, 0xf9, 0x0f, 0x1f, 0x01, 0x19, 0x80, 0x99, 0x07, 0x9e, 0x0f, 0x1f, 0x01, 0x19, 0x80, 0x99, 0x07, 0x9e, 0x0f, 0x1f, 0x01, 0x79, 0x1e, 0x80, 0x19, 0xe6, 0x0f, 0x1f, 0xff, 0x79, 0x1e, 0x80, 0x19, 0xe6, 0x0f, 0x1f, 0x00, 0x18, 0xe0, 0x81, 0xe7, 0x87, 0x0f, 0x1f, 0x00, 0x18, 0xe0, 0x81, 0xe7, 0x87, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0f };
//32x32
const unsigned char photoresistance[] PROGMEM = { 0xf0, 0xff, 0xff, 0x0f, 0x88, 0x02, 0x00, 0x18, 0x88, 0xff, 0x3f, 0x18, 0x04, 0x03, 0x40, 0x30, 0x04, 0xfe, 0xbf, 0x30, 0x04, 0x00, 0xa0, 0x30, 0x04, 0x00, 0xa0, 0x30, 0x02, 0x00, 0xa0, 0x60, 0x02, 0xfc, 0xbf, 0x60, 0x02, 0x02, 0x40, 0x60, 0x02, 0xfd, 0x3f, 0x60, 0x01, 0x05, 0x00, 0xe0, 0x01, 0x05, 0x00, 0xc0, 0x19, 0x05, 0x00, 0xd8, 0x25, 0xff, 0x3f, 0xe4, 0x2d, 0x06, 0x40, 0xec, 0x3d, 0xfc, 0xbf, 0xfc, 0x19, 0x00, 0xa0, 0xd8, 0x01, 0x00, 0xa0, 0xc0, 0x01, 0x00, 0xa0, 0xc0, 0x01, 0xfc, 0xbf, 0xc0, 0x02, 0x02, 0x40, 0x60, 0x02, 0xfd, 0x3f, 0x60, 0x02, 0x05, 0x00, 0x60, 0x02, 0x05, 0x00, 0x60, 0x04, 0x05, 0x00, 0x30, 0x04, 0xff, 0xff, 0x30, 0x04, 0x06, 0x80, 0x30, 0x04, 0xfc, 0xbf, 0x30, 0x08, 0x00, 0xa0, 0x18, 0xf8, 0xff, 0xff, 0x1f, 0xf0, 0xff, 0xff, 0x0f };
//82x16
const unsigned char Error1[] PROGMEM = { 0xbf, 0xf7, 0x0f, 0xe0, 0x01, 0x80, 0xbf, 0x7f, 0x00, 0xfd, 0x03, 0x3f, 0x00, 0xee, 0xef, 0xff, 0xfe, 0x00, 0xe0, 0xf7, 0xe1, 0x03, 0xdf, 0xff, 0xef, 0xef, 0xff, 0xfe, 0xfe, 0xef, 0x80, 0xf6, 0x03, 0xdf, 0xff, 0xef, 0xef, 0x7f, 0xff, 0xfe, 0xf7, 0x36, 0xf6, 0x03, 0xcf, 0x00, 0x0f, 0xe0, 0x7f, 0xfa, 0x07, 0xfc, 0x80, 0xf5, 0x03, 0xcf, 0xff, 0xff, 0xff, 0xbf, 0xf6, 0xff, 0xff, 0xf3, 0xfb, 0x03, 0xd7, 0x00, 0x03, 0x00, 0xdf, 0xee, 0xff, 0xff, 0xe5, 0xf5, 0x03, 0xdf, 0xff, 0xbf, 0xff, 0xef, 0xee, 0x00, 0xe0, 0xd6, 0xee, 0x03, 0xdf, 0xff, 0xdf, 0xff, 0xf7, 0xde, 0x6f, 0xff, 0x01, 0xf0, 0x03, 0xdf, 0x00, 0x0f, 0xe0, 0xf9, 0xde, 0x6f, 0xff, 0xdf, 0xff, 0x03, 0xdf, 0x7e, 0xff, 0xef, 0xff, 0xfe, 0x77, 0xef, 0x1b, 0xfc, 0x03, 0xdf, 0x7e, 0xff, 0xef, 0xff, 0xfe, 0x77, 0xef, 0xdb, 0xff, 0x03, 0xdf, 0x00, 0xff, 0xed, 0xff, 0xfe, 0x7b, 0x6f, 0x00, 0xe0, 0x03, 0xdf, 0x7e, 0xff, 0xf3, 0xff, 0xfe, 0xfc, 0xf0, 0xff, 0xff, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x03 };
//66x64
const unsigned char note[] PROGMEM = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x03, 0xbf, 0xf7, 0x0f, 0x80, 0xf7, 0xf6, 0x7f, 0xe0, 0x03, 0x5f, 0x00, 0xee, 0xbd, 0xf7, 0xfa, 0x60, 0xef, 0x03, 0xef, 0xf7, 0xef, 0xbd, 0x3f, 0xe0, 0x6e, 0xef, 0x03, 0xcf, 0xf7, 0xef, 0xbd, 0xbf, 0xef, 0x6e, 0xe0, 0x03, 0x57, 0x00, 0x0f, 0x80, 0xb1, 0xef, 0x60, 0xef, 0x03, 0x5f, 0x77, 0xef, 0xbd, 0xb7, 0xef, 0x6e, 0xef, 0x03, 0x5f, 0x77, 0xef, 0xbd, 0x37, 0xe0, 0x6e, 0xef, 0x03, 0x5f, 0x00, 0xef, 0xbd, 0xf7, 0xfa, 0x6e, 0xe0, 0x03, 0xdf, 0xf6, 0x0f, 0x80, 0xf7, 0xfa, 0x60, 0xef, 0x03, 0xdf, 0xf9, 0xef, 0xbd, 0xf7, 0xfa, 0x6e, 0xef, 0x03, 0xdf, 0xf3, 0xef, 0xbd, 0x57, 0xbb, 0xbf, 0xef, 0x03, 0xdf, 0xcd, 0xf7, 0xbd, 0x67, 0xbb, 0xbf, 0xef, 0x03, 0x5f, 0x3e, 0xf6, 0xad, 0xb7, 0xbb, 0xdf, 0xeb, 0x03, 0x9f, 0x7f, 0xfb, 0xdf, 0xdf, 0xc7, 0xef, 0xf7, 0x03, 0x7f, 0xbf, 0x07, 0xe0, 0xff, 0xff, 0xff, 0xff, 0x03, 0xef, 0xbe, 0xdf, 0xf7, 0xff, 0xff, 0xff, 0xff, 0x03, 0xef, 0xbc, 0xdf, 0xf7, 0xff, 0xff, 0xff, 0xff, 0x03, 0xef, 0xbd, 0xdf, 0xfb, 0xff, 0xff, 0xff, 0xff, 0x03, 0xef, 0xbf, 0xdf, 0xfb, 0xff, 0xff, 0xff, 0xff, 0x03, 0xef, 0xbf, 0xdf, 0xc1, 0xff, 0xff, 0xff, 0xff, 0x03, 0xef, 0xdd, 0x9f, 0xdf, 0xff, 0xff, 0xff, 0xff, 0x03, 0xef, 0xde, 0x6f, 0xef, 0xff, 0xff, 0xff, 0xff, 0x03, 0x6f, 0xdf, 0xef, 0xee, 0xff, 0xff, 0xff, 0xff, 0x03, 0xaf, 0xef, 0xef, 0xf5, 0xff, 0xff, 0xff, 0xff, 0x03, 0xcf, 0xcf, 0xf7, 0xf9, 0xff, 0xff, 0xff, 0xff, 0x03, 0xef, 0xb7, 0xf7, 0xe6, 0xff, 0xff, 0xff, 0xff, 0x03, 0xff, 0x79, 0x3b, 0x1f, 0xff, 0xff, 0xff, 0xff, 0x03, 0x7f, 0x7e, 0xcf, 0xbf, 0xff, 0xff, 0xff, 0xff, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xff, 0x03, 0xff, 0x00, 0x0e, 0xe0, 0x03, 0xc0, 0x09, 0xc0, 0x03, 0x07, 0xef, 0xef, 0xef, 0xdf, 0xfb, 0xeb, 0xfe, 0x03, 0xdf, 0xf7, 0xef, 0xef, 0xdf, 0xfb, 0x6f, 0xff, 0x03, 0xdf, 0x01, 0xef, 0xef, 0xdf, 0xfb, 0x2e, 0xe0, 0x03, 0xdf, 0x7d, 0x0f, 0xe0, 0xdf, 0xfb, 0xa9, 0xef, 0x03, 0xdf, 0x6d, 0xef, 0xef, 0x01, 0x80, 0x2b, 0xe0, 0x03, 0xdf, 0x6d, 0xef, 0xef, 0xdf, 0xfb, 0xaf, 0xef, 0x03, 0xdf, 0x6d, 0xef, 0xef, 0xdf, 0xfb, 0x2b, 0xe0, 0x03, 0x1f, 0x6d, 0x0f, 0xe0, 0xdf, 0xfb, 0xec, 0xfd, 0x03, 0xc7, 0x6d, 0xef, 0xef, 0xdf, 0xfb, 0x6d, 0xf5, 0x03, 0xff, 0x6d, 0xef, 0xef, 0xef, 0xfb, 0x2d, 0xcd, 0x03, 0xff, 0x97, 0xef, 0xef, 0xef, 0xfb, 0xd5, 0xdd, 0x03, 0xff, 0x7b, 0x0f, 0xe0, 0xf7, 0xfb, 0x75, 0xfd, 0x03, 0xff, 0xfc, 0xee, 0xef, 0xfb, 0xfb, 0xf9, 0xfe, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x03 };
// width: 128, height: 64
const unsigned char TestModeNote[] PROGMEM = { 0xff, 0xff, 0xff, 0xff, 0xfd, 0xdf, 0xfd, 0xfd, 0xbd, 0xfb, 0xdf, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1b, 0xdc, 0xfb, 0xf5, 0xbd, 0xfb, 0xdf, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xd7, 0xd5, 0xfb, 0xed, 0x0d, 0xe0, 0xdf, 0xfd, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x5f, 0xd5, 0x0f, 0x40, 0xb0, 0x3b, 0x00, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x5d, 0xd5, 0xff, 0xfd, 0x1d, 0xf0, 0xdf, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x5b, 0xd5, 0xf8, 0xfd, 0xdc, 0xf7, 0xdf, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x5f, 0xd5, 0x1b, 0xfc, 0x18, 0x70, 0xd0, 0xff, 0xff, 0xff, 0xff, 0xff, 0x07, 0x07, 0x83, 0xc1, 0x57, 0xd5, 0xbb, 0xfd, 0xd4, 0xf7, 0xdd, 0x3f, 0x18, 0x0c, 0x06, 0xff, 0xff, 0xff, 0xff, 0xff, 0x57, 0xd5, 0xbb, 0x7d, 0x1d, 0xf0, 0xbd, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x59, 0xd5, 0xbb, 0xfd, 0xfd, 0xfe, 0xbd, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7b, 0xdf, 0xbb, 0xfb, 0x05, 0xe0, 0x7d, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xbb, 0xde, 0x2b, 0xda, 0xfd, 0xfe, 0x61, 0xf7, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xdb, 0xdd, 0x93, 0xd7, 0x7d, 0x3d, 0xfe, 0xf6, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xeb, 0xd7, 0xdb, 0xcf, 0x9d, 0x63, 0xff, 0xf1, 0xff, 0xff, 0xff, 0xff, 0xef, 0xc0, 0xff, 0xff, 0xfb, 0xef, 0xfb, 0xdd, 0x65, 0xf7, 0xfe, 0xb7, 0xef, 0xff, 0xf7, 0xff, 0xef, 0xde, 0x07, 0xf0, 0xff, 0xff, 0xdd, 0xfd, 0x7d, 0xff, 0x02, 0xbe, 0xef, 0xf7, 0xef, 0xff, 0xef, 0xde, 0xff, 0xff, 0xff, 0xff, 0x9d, 0xed, 0x0e, 0xf8, 0xfa, 0xbe, 0x03, 0x6e, 0x00, 0xfe, 0x83, 0xc0, 0xff, 0xff, 0xff, 0xff, 0x56, 0xf5, 0x7e, 0x7f, 0xfb, 0xbe, 0xef, 0x6f, 0xdb, 0xff, 0xef, 0xde, 0xff, 0xff, 0xff, 0x7f, 0xd0, 0x78, 0x7b, 0x7f, 0x03, 0x0e, 0xee, 0x7f, 0xdb, 0xff, 0xaf, 0xde, 0x01, 0xc0, 0xff, 0xff, 0xdb, 0x3d, 0x08, 0x30, 0xdf, 0xbf, 0x01, 0x7c, 0x00, 0xfe, 0xcf, 0xc0, 0x7f, 0xff, 0xe7, 0xff, 0x1d, 0xe0, 0xdd, 0x56, 0xdf, 0xbf, 0xbf, 0x67, 0xdb, 0xff, 0xe7, 0xff, 0x7f, 0xff, 0xe7, 0xff, 0xde, 0xfd, 0xbe, 0x7a, 0x01, 0x38, 0xbf, 0x6f, 0xdb, 0xff, 0x2b, 0x00, 0x6f, 0xfb, 0xff, 0x7f, 0xd0, 0x78, 0xef, 0x7e, 0xdf, 0x9f, 0x00, 0x6c, 0xc3, 0xff, 0xef, 0xf7, 0x67, 0xf7, 0xff, 0xff, 0x5e, 0x35, 0xd8, 0x7e, 0x8f, 0xaf, 0xbb, 0x6f, 0xff, 0xff, 0xef, 0xf6, 0x7b, 0xcf, 0xe7, 0xff, 0x9f, 0xed, 0x07, 0x70, 0x4f, 0xbf, 0xb7, 0x6f, 0x55, 0xff, 0xef, 0x86, 0x7d, 0xdf, 0xe7, 0xff, 0xd3, 0xfd, 0x7f, 0x7f, 0xd7, 0xbe, 0xb7, 0xaf, 0xd5, 0xfe, 0xef, 0xf6, 0x7f, 0xff, 0xff, 0x7f, 0xdc, 0xfd, 0x79, 0x7e, 0xdb, 0xb9, 0xbf, 0xcf, 0xd6, 0xfe, 0x6b, 0xf5, 0x5f, 0xff, 0xff, 0xff, 0x1e, 0x20, 0xbe, 0x7d, 0xdd, 0xaf, 0xaf, 0x2f, 0xff, 0xff, 0xb7, 0x03, 0xbf, 0xff, 0xff, 0xff, 0xff, 0x7f, 0xcf, 0x7b, 0xdf, 0xdf, 0xdf, 0xf7, 0x00, 0xfe, 0xff, 0xfe, 0xef, 0xfe, 0xbf, 0xfb, 0xf7, 0xbf, 0xff, 0xbb, 0xbf, 0xbf, 0x77, 0xff, 0xfb, 0xff, 0xf7, 0xde, 0xef, 0xfe, 0xbd, 0xfb, 0xef, 0x7f, 0x83, 0x7b, 0xbf, 0xbe, 0x77, 0xff, 0xdb, 0xff, 0xef, 0x9e, 0xef, 0xc0, 0xbb, 0xfb, 0xdf, 0xff, 0xba, 0x7a, 0xbf, 0xbd, 0x01, 0xfc, 0xbb, 0xff, 0xdf, 0xee, 0x6d, 0xef, 0x0b, 0xc0, 0xdf, 0xff, 0xab, 0xfa, 0x01, 0x08, 0x76, 0x07, 0x00, 0xfe, 0xdf, 0xf6, 0x6b, 0xf7, 0xbf, 0xfb, 0xdf, 0xbf, 0xab, 0xfa, 0xbf, 0xbf, 0x03, 0xfe, 0xfb, 0xff, 0xff, 0xfe, 0xa7, 0xfa, 0xbf, 0xfb, 0xaf, 0x7f, 0xab, 0x1a, 0xbf, 0x9f, 0xfb, 0xfe, 0xfb, 0xff, 0x03, 0x00, 0xcf, 0xed, 0xb8, 0xfb, 0xaf, 0xff, 0xab, 0x7a, 0x83, 0x1f, 0x03, 0x0e, 0xfa, 0xff, 0xbf, 0xfb, 0xef, 0xee, 0xbb, 0xfb, 0xaf, 0xff, 0xaa, 0x7a, 0xb7, 0x9f, 0xfa, 0xbe, 0xfb, 0xff, 0xbf, 0xfb, 0x2f, 0x80, 0x0b, 0xc0, 0x77, 0xff, 0xaa, 0x7a, 0xb7, 0xaf, 0x03, 0xbe, 0xf7, 0xff, 0xbf, 0xfb, 0xe7, 0xef, 0xbb, 0xfb, 0xfb, 0x3e, 0xab, 0x7a, 0xb7, 0xbf, 0xdf, 0xbf, 0xf7, 0xff, 0xdf, 0x7b, 0x6b, 0xef, 0xbb, 0xfb, 0xfb, 0x7e, 0xef, 0x7b, 0x77, 0xbf, 0x00, 0xbc, 0xef, 0xff, 0xdf, 0x7b, 0xed, 0xee, 0xbb, 0xfb, 0xfd, 0x7d, 0xd7, 0x7b, 0x45, 0xbb, 0xdf, 0x3f, 0xec, 0xfe, 0xef, 0x7b, 0xef, 0xee, 0xdb, 0xfb, 0xfd, 0x7d, 0xbb, 0x7b, 0xf2, 0xba, 0xaf, 0xc7, 0xdf, 0xfe, 0xf7, 0x87, 0xef, 0xeb, 0xf5, 0xff, 0xfe, 0x63, 0xfd, 0x7a, 0xfb, 0xb9, 0x73, 0xec, 0x3f, 0xfe, 0xfb, 0xff, 0xef, 0xf7, 0x0e, 0x40, 0xff, 0x77, 0xff, 0xfd, 0xff, 0xbb, 0xfc, 0xfe, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xe3, 0xbf, 0xff, 0x0f, 0xf8, 0xef, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x03, 0x00, 0x07, 0xfc, 0x7f, 0xff, 0xee, 0xfb, 0xef, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xef, 0x7f, 0xff, 0x03, 0xe0, 0xed, 0x7b, 0xef, 0xfd, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xef, 0x01, 0x80, 0xfb, 0xef, 0x0b, 0x78, 0xef, 0xfd, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0f, 0xec, 0x7f, 0xff, 0xfb, 0xef, 0xef, 0x7b, 0xef, 0xfd, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xef, 0xed, 0x07, 0xf0, 0xfb, 0xef, 0xef, 0x7b, 0xef, 0xfd, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xef, 0xed, 0x77, 0xf7, 0x03, 0x60, 0x0c, 0x78, 0x00, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xef, 0xed, 0x07, 0xf0, 0xfb, 0xff, 0xad, 0xfb, 0xef, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xef, 0xed, 0x77, 0xf7, 0xfb, 0xff, 0x6d, 0xbd, 0xef, 0xfb, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0f, 0xec, 0x07, 0xf0, 0x0b, 0xe0, 0xed, 0xbe, 0xef, 0xfb, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xef, 0xed, 0x7f, 0xff, 0xeb, 0xef, 0xad, 0xb9, 0xef, 0xfb, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xef, 0x07, 0xe0, 0xed, 0xef, 0xcd, 0xb3, 0xef, 0xfb, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xef, 0x7f, 0xff, 0xed, 0xef, 0xed, 0xb7, 0xef, 0xfb, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xeb, 0x7f, 0xff, 0x0e, 0xe0, 0xfa, 0x3f, 0x00, 0xf8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf7, 0x01, 0x80, 0xef, 0x6f, 0x07, 0xe0, 0xff, 0xfb, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
// width: 51, height: 27
const unsigned char BANote[] PROGMEM  = { 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x40, 0x02, 0xe1, 0x40, 0x02, 0x3c, 0x1c, 0x20, 0x07, 0x11, 0x3c, 0x6e, 0x40, 0x22, 0xa0, 0x07, 0x11, 0x10, 0x92, 0x20, 0x22, 0xe0, 0x07, 0x61, 0x10, 0x92, 0x1c, 0x20, 0xe0, 0x07, 0x81, 0x10, 0x92, 0x20, 0x18, 0xe0, 0x05, 0x89, 0x10, 0x92, 0x20, 0x04, 0xe0, 0x04, 0x71, 0x10, 0x92, 0x1c, 0x7e, 0x20, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00, 0x20, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00, 0x20, 0x06, 0x11, 0x42, 0x80, 0x00, 0xf6, 0x2f, 0x07, 0x21, 0xc2, 0x80, 0x00, 0x05, 0xa8, 0x07, 0x41, 0x22, 0xfc, 0x1f, 0x01, 0xe8, 0x07, 0x41, 0x12, 0x84, 0x10, 0x01, 0xe8, 0x07, 0x01, 0x02, 0x84, 0x10, 0x01, 0xe8, 0x05, 0xf9, 0xff, 0xfd, 0x1f, 0x01, 0xe8, 0x04, 0x81, 0x08, 0x84, 0x10, 0x01, 0x28, 0x04, 0x81, 0x08, 0x84, 0x10, 0x01, 0x28, 0x04, 0x81, 0x08, 0xfc, 0x1f, 0x01, 0x28, 0x06, 0x41, 0x08, 0x85, 0x00, 0x01, 0x28, 0x07, 0x41, 0x08, 0x81, 0x40, 0x01, 0xa8, 0x07, 0x21, 0x08, 0x81, 0x40, 0x01, 0xe8, 0x07, 0x11, 0xf0, 0x80, 0x40, 0x01, 0xea, 0x07, 0x09, 0x00, 0x00, 0x3f, 0x01, 0xe4, 0x05, 0x02, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x02, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01 };
void setup() {
  Serial.begin(115200);
  u8g2.begin();
  u8g2.setFont(u8g2_font_6x13_tf);
  u8g2.setFontDirection(0); //设置字体方向
  u8g2.setCursor(0, 12);
  u8g2.print(">_ booting");
  u8g2.setCursor(0, 24);
  u8g2.sendBuffer();
  delay(200);
  u8g2.print("F/W:  ");
  u8g2.print(SysVersion);
  Serial.println(String("F/W: ") + SysVersion);
  u8g2.setCursor(0, 36);
  u8g2.print("Auto white balance");
  u8g2.setCursor(0, 62);
  u8g2.print("LHW - createskyblue");
  u8g2.sendBuffer();
  delay(200);
  LightIntensity = ((analogRead(0) + analogRead(1)) / 2);
  //if (LightIntensity % 2) u8g2.drawXBMP(0, 0, 128, 64, Logo1); else u8g2.drawXBMP(0, 0, 128, 64, Logo2);
  BootAnimation(); //启动动画
  u8g2.sendBuffer();
  delay(1000);
  if (analogRead(0) > LightIntensity * 1.2 | analogRead(1) > LightIntensity * 1.2) TestMode();

  //显示项目二维码
  u8g2.clear();
  u8g2.drawXBMP(0, 2, 60, 60, QRCode);
  u8g2.drawXBMP(62, 0, 66, 64, note);
  u8g2.sendBuffer();
  delay(2000);

  u8g2.setFontMode(1); //字体模式:透明
  u8g2.setDrawColor(2); //设置字体前景颜色:xor
  GUI();
}

void loop() {
  collect();
  MakeGrap();
  SerialOutPut();
  //if (State[0]&State[1] & timestamp[1][1] > timestamp[0][0]) GUI(); //捕捉信号时限制刷新画面以保证采样性能  //因为动画带来不必要的刷新过于消耗资源，所以禁用了
}
//启动动画
void BootAnimation() {
  byte BAPointObj_PD_X = 48;
  //动画第一阶段：光敏电阻
  for (BAPointObj_PD_X; BAPointObj_PD_X > 16; BAPointObj_PD_X--) {
    u8g2.clearBuffer();
    Draw_BAPointObj_BJ();
    Draw_BAPointObj_PD(BAPointObj_PD_X, map(BAPointObj_PD_X, 48, 16, 0, 12));
    u8g2.sendBuffer();
  }
  //第二阶段：标题->sin(x)
  for (float x = 0; x < 2 * PI; x += 0.1) {
    u8g2.clearBuffer();
    Draw_BAPointObj_BJ();
    u8g2.setDrawColor(0);
    u8g2.drawXBMP(byte(32 + x * 5.09), byte(20 + sin(x) * 16), 51, 27, BANote); //浮动的标题
    Draw_BAPointObj_PD(16, 12);
    u8g2.sendBuffer();
  }
  //最后阶段：首创者信息
  u8g2.drawBox(0, 51, 128, 14 );
  u8g2.setDrawColor(1);
  u8g2.setCursor(4, 62);
  u8g2.print("LHW - createskyblue");
}
void Draw_BAPointObj_PD(byte x, byte y) {
  u8g2.drawXBMP(x, y, 32, 32, photoresistance);
  u8g2.drawLine(x + 3, y + 32, x + 3, y + 60);
  u8g2.drawLine(x + 27, y + 32, x + 27, y + 60);
}
void Draw_BAPointObj_BJ() {
  u8g2.setDrawColor(1);
  u8g2.drawBox(0, 0, 128, 64 );
  u8g2.setDrawColor(0);
}
void SerialOutPut() {
  int CheckCode = 4096 + DataDeal[0][1] + DataDeal[1][1] - LightIntensity + timestamp[0][2] + timestamp[1][2] + deltaT;
  Serial.println((4096 - LightIntensity) + String(",") + DataDeal[0][1]  + String(",") + DataDeal[1][1]  + String(",") + CheckCode + String(",") + timestamp[0][2] + String(",") + timestamp[1][2] + String(",") + deltaT + String(","));
}
void collect() {
  int IoALeng = sizeof(io) / sizeof(io[0]);
  bool Trigger = false;
  for (byte Iio = 0; Iio < IoALeng; Iio++) {
    //进行数据移位
    DataDeal[Iio][0] = 0; //移位前把历史累加数据清零
    for (int i = 0; i < TmpDateMax - 1; i++) {
      DATA[Iio][i] = DATA[Iio][i + 1];
      DataDeal[Iio][0] += DATA[Iio][i + 1];
    }
    //写入新的传感器数据
    //DATA[Iio][7] = analogRead(io[Iio]) - LightIntensity;
    DATA[Iio][TmpDateMax - 1] = analogRead(io[Iio]);
    DataDeal[Iio][0] += DATA[Iio][TmpDateMax - 1];
    DataDeal[Iio][1] = DataDeal[Iio][0] / float(TmpDateMax); //计算均值
    //记录电平突变时间戳
    Trigger = false;
    //限定幅度均值动态调整，效率高 v1.2
    // Serial.println(Iio+String("->目前：") + DATA[Iio][TmpDateMax-1] + String("大于阈值：") + (1.2 * DataDeal[Iio][1])+String("小于阈值：") + (0.8 * DataDeal[Iio][1]));
    if (DATA[Iio][TmpDateMax - 1] > (1.05 * DataDeal[Iio][1]) && State[Iio] == 1) {
      // Serial.println(Iio+String("->####"));
      timestamp[Iio][0] = millis();
      State[Iio] = 0;
      //Serial.println(timestamp[1][0]+String(" , ")+timestamp[1][1]);
      //Serial.println(Iio + String("-触发电平突变起始 当前：") + DATA[Iio][0] + String("大于阈值：") + (DataDeal[Iio][1] + MutationThreshold));
    }
    if (DATA[Iio][TmpDateMax - 1] < (0.95 * DataDeal[Iio][1]) && millis() > timestamp[Iio][0] && State[Iio] == 0) {
      // Serial.println(Iio+String("->#############"));
      timestamp[Iio][1] = millis();
      Trigger = true;
      State[Iio] = 1;
      //Serial.println(timestamp[1][0]+String(" , ")+timestamp[1][1]);
    }
    if (Trigger) {
      timestamp[Iio][2] = timestamp[Iio][1] - timestamp[Iio][0];
      if (TriggerTimer) {
        if (timestamp[1][0] > timestamp[0][1]) {
          deltaT = timestamp[1][0] - timestamp[0][1];
          GUIState = true;
          GUI();
        } else {
          GUIState = false;
          GUI();
        }
        //Serial.println(Iio + String("-突变起始时间") + timestamp[Iio][0] + String("-突变结束时间") + timestamp[Iio][1] + String("-突变维持时间") + timestamp[Iio][2]);
        Trigger = false;
      }
      if (State[0] = 1) TriggerTimer = !TriggerTimer;
    }
  }
}

void TestMode() {
  u8g2.clearBuffer();
  u8g2.drawXBMP(0, 0, 128, 64, TestModeNote);
  for (byte i; i < 128; i += 1) {
    //delay(6);
    u8g2.setDrawColor(0);
    u8g2.drawBox(0, 60, i, 4);
    u8g2.sendBuffer();
  }
  u8g2.setDrawColor(1);
  if (analogRead(0) > LightIntensity * 1.4 | analogRead(1) > LightIntensity * 1.4) {
    GUIState = true;
    while (1) {
      u8g2.clearBuffer();
      //u8g2.drawBox(0, 0, 128, 64);
      u8g2.drawXBMP(0, 7, 128, 14, TestModeNote);
      grap[0][GrapX] = map(analogRead(io[0]), 0, 4095, 0, 33);
      grap[1][GrapX] = map(analogRead(io[1]), 0, 4095, 0, 33);
      GrapX++;
      if (GrapX >= 17) GrapX = 0;
      DrawWave();//绘制波形
      u8g2.drawLine(GrapX * 8, 28, GrapX * 8, 64); //扫描线
      u8g2.sendBuffer();
      delay(15);
    }
  }
}

void MakeGrap() {
  if (timestamp[1][1] < timestamp[0][0]) { //只捕捉实验范围内的信号，节省资源
    if (!State[0] && State[1] && GrapX > 8) {
      GrapX = 0;
      for (byte i = 0; i < 17; i++) {
        grap[0][i] = 0;
        grap[1][i] = 0;
      }
    }
    if (GrapX < 17 && GrapSampling + SamplingRate < millis()) {
      grap[0][GrapX] = map(DataDeal[0][1], 0, 4095, 0, 33);
      grap[1][GrapX] = map(DataDeal[1][1], 0, 4095 , 0, 33);
      GrapX++;
      GrapSampling = millis();
    }
  }
}
void GUI() {
  u8g2.clearBuffer();
  u8g2.setCursor(4, 12);
  if (GUIState) u8g2.drawBox(0, 0, 128, 26);
  u8g2.print("A ");
  u8g2.print(timestamp[0][2]);
  u8g2.print(" ms");
  u8g2.setCursor(64, 12);
  u8g2.print("B ");
  u8g2.print(timestamp[1][2]);
  u8g2.print(" ms");

  DrawWave();//绘制波形

  //显示时间差
  u8g2.setCursor(4, 24);
  if (GUIState) {
    u8g2.print("Diff-> ");
    u8g2.print(deltaT);
    u8g2.print(" ms");
  } else {
    if ((millis() / 1000) % 2|1) u8g2.drawBox(0, 14, 128, 12); //'|1'因为动画带来不必要的刷新过于消耗资源，所以禁用了
    u8g2.print("#Unrecognized Wave!");
    u8g2.drawXBMP(23, 36, 82, 16, Error1);
  }


  u8g2.sendBuffer();
}
void DrawWave() {
  //绘制图形框
  u8g2.drawFrame(0, 28, 128, 36);
  //绘制计数点，看起来更有逼格
  for (byte y = 29; y < 64; y += 8)
    for (byte x = 1; x < 128; x += 8)
      u8g2.drawPixel(x, y);
  //绘制实时曲线
  if (GUIState) for (int i = 0; i < 16; i++) {
      u8g2.drawLine(8 * i, 63 - grap[0][i], 8 * (i + 1), 63 - grap[0][i + 1]);
      u8g2.drawLine(8 * i, 63 - grap[1][i], 8 * (i + 1), 63 - grap[1][i + 1]);
    }
}
