/**
 * @file    DSP.cpp
 * @author  Ted Kotz <ted@kotz.us>
 * @version 0.1
 *
 * [Description]
 *
 */
/* Includes ******************************************************************/
#include "DSP.h"
#include <Arduino.h>

/* Defines *******************************************************************/

/* Types *********************************************************************/

/* Interfaces ****************************************************************/

/* Data **********************************************************************/


/**
 *  A table of reference cosine wave values for comparing against.
 *
 *  Generator:
 *   COSINE_TABLE[i] = int(0x7FFF*cos(2*pi*i/COSINE_TABLE_SIZE)+0.5)
 */
const PROGMEM Q_15 COSINE_TABLE[COSINE_TABLE_SIZE] =
{
   32767,  32757,  32728,  32678,  32609,  32521,  32412,  32285,
   32137,  31971,  31785,  31580,  31356,  31113,  30852,  30571,
   30273,  29956,  29621,  29268,  28898,  28510,  28105,  27683,
   27245,  26790,  26319,  25832,  25329,  24811,  24279,  23731,
   23170,  22594,  22005,  21402,  20787,  20159,  19519,  18867,
   18204,  17530,  16845,  16151,  15446,  14732,  14009,  13278,
   12539,  11792,  11039,  10278,   9511,   8739,   7961,   7179,
    6392,   5601,   4807,   4011,   3211,   2410,   1607,    804,
       0,   -804,  -1607,  -2410,  -3211,  -4011,  -4807,  -5601,
   -6392,  -7179,  -7961,  -8739,  -9511, -10278, -11039, -11792,
  -12539, -13278, -14009, -14732, -15446, -16151, -16845, -17530,
  -18204, -18867, -19519, -20159, -20787, -21402, -22005, -22594,
  -23170, -23731, -24279, -24811, -25329, -25832, -26319, -26790,
  -27245, -27683, -28105, -28510, -28898, -29268, -29621, -29956,
  -30273, -30571, -30852, -31113, -31356, -31580, -31785, -31971,
  -32137, -32285, -32412, -32521, -32609, -32678, -32728, -32757,
  -32767, -32757, -32728, -32678, -32609, -32521, -32412, -32285,
  -32137, -31971, -31785, -31580, -31356, -31113, -30852, -30571,
  -30273, -29956, -29621, -29268, -28898, -28510, -28105, -27683,
  -27245, -26790, -26319, -25832, -25329, -24811, -24279, -23731,
  -23170, -22594, -22005, -21402, -20787, -20159, -19519, -18867,
  -18204, -17530, -16845, -16151, -15446, -14732, -14009, -13278,
  -12539, -11792, -11039, -10278,  -9511,  -8739,  -7961,  -7179,
   -6392,  -5601,  -4807,  -4011,  -3211,  -2410,  -1607,   -804,
       0,    804,   1607,   2410,   3211,   4011,   4807,   5601,
    6392,   7179,   7961,   8739,   9511,  10278,  11039,  11792,
   12539,  13278,  14009,  14732,  15446,  16151,  16845,  17530,
   18204,  18867,  19519,  20159,  20787,  21402,  22005,  22594,
   23170,  23731,  24279,  24811,  25329,  25832,  26319,  26790,
   27245,  27683,  28105,  28510,  28898,  29268,  29621,  29956,
   30273,  30571,  30852,  31113,  31356,  31580,  31785,  31971,
   32137,  32285,  32412,  32521,  32609,  32678,  32728,  32757,
};

/* Functions *****************************************************************/

Q_15 cosine_table( BAM8 x )
{
  return pgm_read_word(&COSINE_TABLE[x%COSINE_TABLE_SIZE]);
}

Q16_15 Q15_MAC( Q_15* a, Q_15* b, int16_t count)
{
  Q16_15 total=0;
  //for(int i=0; i<count; ++i)
  while(count--)
  {
    total += (((Q16_15)(*a++) * (Q16_15)(*b++)) >>8);
  }

  return ( total + (1 << 6)) >> 7;
}

  // Initialization of tables of constants used by CORDIC
  // need a table of arctangents of negative powers of two:
  // hex(arctan(2^(-index))*0x8000/pi+0.5)

//  #define CORDIC16_ITERS 16
//  static const BAM16 arctanTable[CORDIC16_ITERS] =
//  {
//    0x2000, 0x12E4, 0x09FB, 0x0511,
//    0x028B, 0x0146, 0x00A3, 0x0051,
//    0x0029, 0x0014, 0x000A, 0x0005,
//    0x0003, 0x0001, 0x0001, 0x0000,
//  };

  // make use of 32 bits basically BAM32
  // hex(arctan(2^(-index))*0x80000000/pi+0.5)
//  #define CORDIC16_ITERS 20
//  static const int32_t arctanTable[CORDIC16_ITERS] =
//  {
//    0x20000000, 0x12E4051E, 0x09FB385B, 0x051111D4,
//    0x028B0D43, 0x0145D7E1, 0x00A2F61E, 0x00517C55,
//    0x0028BE53, 0x00145F2F, 0x000A2F98, 0x000517CC,
//    0x00028BE6, 0x000145F3, 0x0000A2FA, 0x0000517D,
//    0x000028BE, 0x0000145F, 0x00000A30, 0x00000518,
//  };

  // Makes more effcient use of all 16 bits (BAM32 >> 14)
  // hex(arctan(2^(-index))*0x20000/pi+0.5)
  #define CORDIC16_ITERS 16
  static const uint16_t arctanTable[CORDIC16_ITERS] =
  {
    0x8000, 0x4B90, 0x27ED, 0x1444,
    0x0A2C, 0x0517, 0x028C, 0x0146,
    0x00A3, 0x0051, 0x0029, 0x0014,
    0x000A, 0x0005, 0x0003, 0x0001,
  };

extern "C" Complex16 CORDIC16_rotate( BAM16 angle, Complex16 vector )
{
  // Kvalues compensate for gain from CORDIC.
  // hex(0.5+0x8000*1/sqrt(1+(2^(-2*(0))))*1/sqrt(1+(2^(-2*(1))))*1/sqrt(1+(2^(-2*(2))))*1/sqrt(1+(2^(-2*(3))))*1/sqrt(1+(2^(-2*(4))))*1/sqrt(1+(2^(-2*(5))))*1/sqrt(1+(2^(-2*(6))))*1/sqrt(1+(2^(-2*(7))))*1/sqrt(1+(2^(-2*(8))))*1/sqrt(1+(2^(-2*(9)))))
  // and a table of products of reciprocal lengths of vectors [1, 2^-2j]:
  // K[k] = 0x8000
  // for( i=0; i< k; ++i)
  // {
  //   K[k] *= 1/sqrt(1+(2^(-2*(i))));
  // }
  // K[k] = hex(K[k] +0.5)
//  static const Q_15 K[N] =
//  {
//    0x5a82, 0x50F4, 0x4E8A, 0x4DEE,
//    0x4DC7, 0x4DBE, 0x4DBB, 0x4DBB,
//    0x4DBB, 0x4DBA, 0x4DBA, 0x4DBA,
//    0x4DBA, 0x4DBA, 0x4DBA, 0x4DBA,
//  };
  // As long as N > ~14 0x4DBA76D4 is a great Q_31 approximation


  //int32_t vsin = 0;
  //int32_t vcos = 0x4DB9DB5F; // K[N-1] * 0x7FFF * 2^16

  int32_t y = (int32_t)vector.y*0x04DBA; // *K[N-1] * 2^15
  int32_t x = (int32_t)vector.x*0x04DBA; // *K[N-1] * 2^15

  // Use Symmetry to get angle in quadrant 1 or 4.
  if( BAM16_Quad23(angle) )
  {
    angle += BAM16_180_DEGREES;
    x = -x;
    y = -y;
  }

  uint32_t angle32 = (uint32_t)angle << 16;

  for(int i=0; i<CORDIC16_ITERS; ++i)
  {
    // Check if angle is in Quadrant 3 or 4
    if(angle32 & 0x80000000)
    {
      // Rotate Counter-Clockwise
      int32_t tmp = x + (y >> i);
      y = y - (x >> i);
      x = tmp;
      angle32 += (uint32_t)arctanTable[i] << 14;
    }
    else
    {
      // Rotate Clockwise
      int32_t tmp = x - (y >> i);
      y = y + (x >> i);
      x = tmp;
      angle32 -= (uint32_t)arctanTable[i] << 14;
    }
  }

  // convert to Q_15, round and saturate
  x=(x+0x4000) >> 15;
  x=constrain(x,-Q15_ONE, Q15_ONE);
  y=(y+0x4000) >> 15;
  y=constrain(y,-Q15_ONE, Q15_ONE);

  return { (Q_15)x , (Q_15)y};

}

extern "C" Complex16 CORDIC16_polar2rect( Polar16 vector )
{
  return CORDIC16_rotate(vector.phase, {vector.mag, 0});
}

extern "C" Polar16 CORDIC16_rect2polar( Complex16 vector )
{
  // Determine angle by rotating based on y value.
  // when y==0 mag=x, phase = -angle
  // see CORDIC16_rotate for wher K comes from
  int32_t y = (int32_t)vector.y*0x04DBA; // *K[N-1] * 2^15
  int32_t x = (int32_t)vector.x*0x04DBA; // *K[N-1] * 2^15
  uint32_t angle32 = 0;

  // Use Symmetry to get angle in quadrant 1 or 4.
  if( x < 0 )
  {
    angle32 = (uint32_t)BAM16_180_DEGREES << 16;
    x = -x;
    y = -y;
  }

  for(int i=0; i<CORDIC16_ITERS; ++i)
  {
    // Check if vector is below the X axis
    if(y > 0)
    {
      // Rotate Counter-Clockwise
      int32_t tmp = x + (y >> i);
      y = y - (x >> i);
      x = tmp;
      angle32 += (uint32_t)arctanTable[i] << 14;
    }
    else
    {
      // Rotate Clockwise
      int32_t tmp = x - (y >> i);
      y = y + (x >> i);
      x = tmp;
      angle32 -= (uint32_t)arctanTable[i] << 14;
    }
  }

  // convert to Q_15, round and saturate
  x=(x+0x4000) >> 15;
  x=constrain(x,-Q15_ONE, Q15_ONE);

  return { (Q_15)x , (BAM16)(angle32 >>16) };

}

Q16_15 powerMeasurement_inphase( const Q_15* src, BAM16 freq, BAM16 phase, int N)
{
  Q16_15 sum = 0;
  BAM16 angle=phase;
  for( int j=0; j<N; ++j)
  {
    SINCOS16_t tmp=CORDIC16_sincos( angle );
    sum += ((Q16_15)tmp.cos * (Q16_15)(*src++))>>8;
    angle+=freq;
  }
  return (sum + (1 << 6)) >> 7;
}

Q16_15 powerMeasurement_magnitude( const Q_15* src, BAM16 freq, int N)
{
  Q16_15 sumI = 0;
  Q16_15 sumQ = 0;
  BAM16 angle=0;
  for( int j=0; j<N; ++j)
  {
    SINCOS16_t tmp=CORDIC16_sincos( angle );
    sumI += ((Q16_15)tmp.cos * (Q16_15)(*src++))>>8;
    sumQ -= ((Q16_15)tmp.sin * (Q16_15)(*src++))>>8;
    angle+=freq;
  }
  sumI = (sumI + (1 << 6)) >> 7;
  sumQ = (sumQ + (1 << 6)) >> 7;
  return sqrt(sumQ*sumQ + sumI*sumI);
}


// Minimum = 0
// Step = SAMPLE_RATE * 2^(-order)
// Maximum = SAMPLE_RATE/2
void FFT_inphase( Q_15* dst, const Q_15* src, int order , BAM8 phase)
{
  int N=1<<order;
  BAM8 dAngle=0;
  BAM8 ddAngle=(1<<(8-order));
  for( int i=0; i<N; ++i)
  {
    Q16_15 sum = 0;
    const Q_15* tmp=src;
    BAM8 angle=phase;
    for( int j=0; j<N; ++j)
    {
      sum += ((Q16_15)cosine_table(angle) * (Q16_15)(*tmp++))>>8;
      angle+=dAngle;
    }
    *dst++ = ((sum + (1 << (6+order)))>> (7+order));
    dAngle+=ddAngle;
  }
}


void FFT_magnitude( Q_15* dst, const Q_15* src, int order )
{
  const int N=1<<order;
  Polar16 pol;
  Q_15 qData[N];
  FFT_inphase(dst, src, order, 0);
  FFT_inphase(qData, src, order, BAM16toBAM8(BAM16_90_DEGREES)); // FFT needs negative sin so use +pi/2
  for( int i = 0 ; i<N; ++i)
  {
    // Convert to Polar to calculate magnitude of hypotenuse
    pol=CORDIC16_rect2polar({dst[i],qData[i]});
    dst[i]=pol.mag;
  }
}

void IFT_magnitude( Q_15* dst, const Q_15* src, int order )
{
  const int N=1<<order;
  Polar16 pol;
  Q_15 qData[N];
  FFT_inphase(dst, src, order, 0);
  FFT_inphase(qData, src, order, BAM16toBAM8(BAM16_270_DEGREES)); // IFFT needs sin so use -pi/2
  for( int i = 0 ; i<N; ++i)
  {
    // Convert to Polar to calculate magnitude of hypotenuse
    pol=CORDIC16_rect2polar({dst[i],qData[i]});
    dst[i]=pol.mag;
  }
}



/**
 * [Description]
 *
 * @param
 * @return
 */
