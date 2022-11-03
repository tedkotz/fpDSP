/**
 * @file    DSP.h
 * @author  Ted Kotz <ted@kotz.us>
 * @version 0.1
 *
 * A collection of tools for Fixed Point arithmetic.
 *
 */
#ifndef   DSP_H
#define   DSP_H

#ifdef __cplusplus
extern "C" {
#endif


/* Includes ******************************************************************/
#include <inttypes.h>
#include <stdlib.h>

/* Defines *******************************************************************/
#define Q15_ZERO 0x0000
#define Q15_ONE  0x7FFF


/* Types *********************************************************************/

/**
 * Binary Angle Measurement(BAM) is a system to use all the bits in a number
 * to encode the angle around the unit circle as an unsigned fraction of turn. It
 * takes advantage of the fact that binary numbers modulus 2^N arithmetic wrap at
 * overflow circular to strictly enforce the fact that angles also wrap.
 *  i.e. 0 degrees = 360 degrees = -360 degrees
 *
 *  BAM16 is the 16 bit variant of this encoding
 *
 *  BAM16 = DEG * 0x8000 / 180
 *  BAM16 = RAD * 0x8000 / PI
 *  BAM16 = TURNS * 0x8000 / 2
 *
 *  |Degrees |Radians |Turns |BAM16    |
 *  |--------|--------|------|---------|
 *  |-360    |-2PI    |-1    |0x0000   |
 *  |-270    |-3PI/2  |-3/4  |0x4000   |
 *  |-180    |-PI     |-1/2  |0x8000   |
 *  |-90     |-PI/2   |-1/4  |0xC000   |
 *  |0       |0       |0     |0x0000   |
 *  |30      |PI/6    |1/12  |0x4000/3 |
 *  |45      |PI/4    |1/8   |0x2000   |
 *  |60      |PI/3    |1/6   |0x8000/3 |
 *  |90      |PI/2    |1/4   |0x4000   |
 *  |180     |PI      |1/2   |0x8000   |
 *  |270     |3PI/2   |3/4   |0xC000   |
 *  |360     |2PI     |1     |0x0000   |
 *
 *  https://en.wikipedia.org/wiki/Binary_angular_measurement
 *
 *  This type is unsigned so as to take advantage of the defined wrapping behaviour of unsigned integer types in C.
 *
 *  Real frequencies are represented in ratios as BAM16 degrees per sample. For example a 250 Hz @ 1000 samp/sec would be 1/4 of a turn per sample so 0x40.
 *  The FREQUENCY_BAM16_PER_SAMPLE macros is available to make these conversions at run or compile time.
 */

typedef uint8_t BAM8;
typedef uint16_t BAM16;
//typedef uint32_t BAM32;


/*
 *  Fixed Point is an encoding that allows a fractional value to be specified without using
 *  a floating point number. It can be generalized to signed types of the form Qm_n, where it
 *  has 1 sign bit, m bits before the radix point and n bits after.  It is the same as always
 *  representing the number divided by 2**n. This conversion can easily be accomplished with a shift.
 *  Regular integers are just a special case of fixed point where n=0;
 *
 *  Fixed Point numbers with the same n can be directly cast between with the rules and dangers as
 *  tradition  integers, they can also be added/subtracted with the same logic. Mutliplecation
 *  and division require some additional manipulation to remove the effects of the implied denominator.
 *
 *  As with integers wrap is bad so saturating addition and subtraction are often used (chacks for roll over)
 *
 */
/**
 *  Q_15 is a 16 bit fixed point quantity representing numbers on the range [-1..1] at a step of 1/32767.
 */
typedef int16_t   Q_15;

/**
 *  UQ1_15 is a 16 bit fixed point quantity representing numbers on the range [0..2] at a step of 1/32768.
 */
typedef uint16_t UQ1_15;

/**
 *  Q16_15 is a 16 bit fixed point quantity representing numbers on the range [-65536..65536] at a step of 1/32768.
 */
typedef int32_t   Q16_15;

/**
 *  UQ15_15 is a 16 bit fixed point quantity representing numbers on the range [0..131072] at a step of 1/32768.
 */
typedef uint32_t UQ17_15;

/**
 *  UQ_16 is a 16 bit fixed point quantity representing numbers on the range [0..1] at a step of 1/65536.
 */
typedef uint16_t UQ_16;

/**
 *  UQ16_16 is a 16 bit fixed point quantity representing numbers on the range [0..65536] at a step of 1/65536.
 */
typedef uint32_t UQ16_16;

typedef struct Q15_DIVMOD_t
{
  Q16_15 quot;
  Q_15 rem;
} Q15_DIVMOD_t;


typedef struct Complex16
{
  union
  {
    Q_15 real;
    Q_15 I;
    Q_15 x;
    Q_15 cos;
  };
  union
  {
    Q_15 imag;
    Q_15 Q;
    Q_15 y;
    Q_15 sin;
  };
} Complex16;

typedef Complex16 SINCOS16_t;

typedef struct Polar16
{
  Q_15  mag;
  BAM16 phase;
} Polar16;

/* Interfaces ****************************************************************/
/* Data **********************************************************************/
static const BAM16 BAM16_PI_RADIANS  = 0x8000;
static const BAM16 BAM16_0_DEGREES   = 0x0000;
static const BAM16 BAM16_270_DEGREES = 0xC000;
static const BAM16 BAM16_180_DEGREES = BAM16_PI_RADIANS;
static const BAM16 BAM16_90_DEGREES  = 0x4000;
static const BAM16 BAM16_60_DEGREES  = 0x10003/6; //BAM16_180_DEGREES/3+0.5
static const BAM16 BAM16_45_DEGREES  = BAM16_90_DEGREES/2;
static const BAM16 BAM16_30_DEGREES  = 0x8003/6;  //BAM16_90_DEGREES/3+0.5

#define COSINE_TABLE_SIZE 256

/* Functions *****************************************************************/

//*** BAM Conversions *********************************************************

/**
 * Converts a number of degrees in to BAM16
 *
 * @param X an angle in degrees to convert
 * @return the angle in BAM16
 */
#define DEG2BAM16(X)     ((BAM16)(((X)*BAM16_45_DEGREES)/45))

/**
 * Converts a BAM8 angle in to BAM16
 *
 * @param X an angle in BAM8
 * @return the angle in BAM16
 */
#define BAM8toBAM16(X)   ((BAM16)((X)<<8))

/**
 * Converts a BAM16 angle in to BAM8
 *
 * @param X an angle in BAM16
 * @return the angle in BAM8
 */
#define BAM16toBAM8(X)   ((BAM8)((X)>>8))

//#define BAM16toBAM32(X)  ((BAM32)((X)<<16))
//#define BAM32toBAM16(X)  ((BAM16)((X)>>16))

/**
 * Converts a Frequency in Hz to a number of BAM16 per Sample at the specified
 * sample rate.
 *
 * @param HZ a frequency in Hz
 * @param SAMPLE_RATE The sample rate of the reference signal
 * @return the frequency expressed in BAM16 "degrees" per sample.
 */
#define FREQUENCY_HZtoBAM16_PER_SAMPLE( HZ, SAMPLE_RATE ) ((BAM16)(((uint32_t)(HZ) << 16)/(SAMPLE_RATE)))

//*** BAM16 Quadrant tests ****************************************************

/**
 * Determines is an angle is in the 4th quadrant
 *
 * @param X a BAM16 angle
 * @return true if the angle is in Quadrant 4, otherwise false
 */
#define BAM16_Quad4(X) (0xC000U==((X)&0xC000U))

/**
 * Determines is an angle is in the 3rd quadrant
 *
 * @param X a BAM16 angle
 * @return true if the angle is in Quadrant 3, otherwise false
 */
#define BAM16_Quad3(X) (0x8000U==((X)&0xC000U))

/**
 * Determines is an angle is in the 2nd quadrant
 *
 * @param X a BAM16 angle
 * @return true if the angle is in Quadrant 2, otherwise false
 */
#define BAM16_Quad2(X) (0x4000U==((X)&0xC000U))

/**
 * Determines is an angle is in the 1st quadrant
 *
 * @param X a BAM16 angle
 * @return true if the angle is in Quadrant 1, otherwise false
 */
#define BAM16_Quad1(X) (0x0000U==((X)&0xC000U))


/**
 * Determines is an angle is in the 3rd or 4th quadrant
 *
 * @param X a BAM16 angle
 * @return true if the angle is in Quadrant 3 or 4, otherwise false
 */
#define BAM16_Quad34(X) (!BAM16_Quad12(X))

/**
 * Determines is an angle is in the 2nd or 4th quadrant
 *
 * @param X a BAM16 angle
 * @return true if the angle is in Quadrant 2 or 4, otherwise false
 */
#define BAM16_Quad24(X) (!BAM16_Quad13(X))

/**
 * Determines is an angle is in the 2nd or 3rd quadrant
 *
 * @param X a BAM16 angle
 * @return true if the angle is in Quadrant 2 or 3, otherwise false
 */
#define BAM16_Quad23(X) BAM16_Quad12((X)-0x4000U)

/**
 * Determines is an angle is in the 1st or 4th quadrant
 *
 * @param X a BAM16 angle
 * @return true if the angle is in Quadrant 1 or 4, otherwise false
 */
#define BAM16_Quad14(X) BAM16_Quad12((X)+0x4000U)

/**
 * Determines is an angle is in the 1st or 3rd quadrant
 *
 * @param X a BAM16 angle
 * @return true if the angle is in Quadrant 1 or 3, otherwise false
 */
#define BAM16_Quad13(X) (!((X)&0x4000U))

/**
 * Determines is an angle is in the 1st or 2nd quadrant
 *
 * @param X a BAM16 angle
 * @return true if the angle is in Quadrant 1 or 2, otherwise false
 */
#define BAM16_Quad12(X) (!((X)&0x8000U))

//*** Fixed Point Math ********************************************************

/**
 * Multiplies two Q*_15 numbers
 *
 * @param A a Q15 number on the scale -1 to 1
 * @param B a Q15 number on the scale -1 to 1
 * @return the product of A and B on the scale -1 to 1
 */
#define Q15_mult(A,B) ((((int32_t)(A))*((int32_t)(B)))>>15)

/**
 * Constrains a Q*_15 number to the range -1 to 1
 *
 * @param X a Q*_15 number on the scale -1 to 1
 * @return the Q15 number on the scale -1 to 1
 */
#define Q15_sat(X) constrain(X, -Q15_ONE, Q15_ONE)

/**
 * A simple Q_15 Multiply Accumulate with a Q16_15 Accumulator.
 * This does not saturate, but is safe against overflow up to a count of at least
 * 256, at the cost of some precision.
 *
 * @param a pointer to a list of count Q_15 numbers
 * @param b pointer to a list of count Q_15 numbers
 * @param count the size of the lists to run the calculation over
 * @return the accumulated total of the multiplications
 */
Q16_15 Q15_MAC( Q_15* a, Q_15* b, int16_t count);

//int16_t add_sat ( int16_t a, int16_t b );
//int16_t sub_sat ( int16_t a, int16_t b );
//int32_t Q15_mult ( int32_t a, int32_t b );
//int32_t UQ16_mult ( int32_t a, int32_t b );
//UQ16_16 UQ16_mac ( UQ_16* a, UQ_16* b , size_t count);
//Q15_DIVMOD_t Q15_divmod ( Q16_15 a, Q_15 b);

//*** BAM16 Trigonometric Transforms *******************************************

/**
 * Simple lookup in the 256 byte COSINE table internal to the DSP library.
 *
 * @param angle the BAM8 angle to lookup the cosine of
 * @return the cosine of angle stored as a Q_15 number
 */
Q_15 cosine_table( BAM8 angle );

/**
 * Rotates vector by angle.
 * Uses a 16 bit version of the CORDIC algorithm.
 *
 * @param angle the angle to rotate by in BAM16
 * @param vector a rectangular vector on the complex plane
 * @return the rectangular vector on the complex plane after rotating by angle
 */
Complex16 CORDIC16_rotate( BAM16 angle, Complex16 vector );

/**
 * Converts a polar vector to rectangular coordinates
 * Uses a 16 bit version of the CORDIC algorithm.
 *
 * @param vector a polar vector
 * @return the same vector, but converted to rectangular coordinates
 */
Complex16 CORDIC16_polar2rect( Polar16 vector );

/**
 * Converts a rectangular vector to polar coordinates
 * Uses an inverted, 16 bit version of the CORDIC algorithm.
 *
 * @param vector a rectangular vector
 * @return the same vector, but converted to polar coordinates
 */
Polar16 CORDIC16_rect2polar( Complex16 vector );

//SINCOS16_t CORDIC16_sincos( BAM16 angle );
/**
 * Simultaneously calculates the sin and cosine of a BAM16 angle.
 * Uses a 16 bit version of the CORDIC algorithm.
 *
 * @param angle a angle in BAM16
 * @return the sine and cosine of angle a Q_15 numbers
 */
#define CORDIC16_sincos(angle) (CORDIC16_rotate( angle, {Q15_ONE, 0}))

//*** Fourier Analysis and Transforms ******************************************

/**
 * Performs a power measurement of the signal in phase with the reference frequency
 *
 * @param src the signal under test
 * @param freq the frequency to analyze in BAM16 per SAMPLE
 * @param phase the phase offset to look for power at
 * @param N the number of samples in src
 * @return the power measurement
 */
Q16_15 powerMeasurement_inphase( const Q_15* src, BAM16 freq, BAM16 phase, int N);

/**
 * Performs a power measurement of the signal across phases with at a given
 * frequency
 *
 * @param src the signal under test
 * @param freq the frequency to analyze in BAM16 per SAMPLE
 * @param N the number of samples in src
 * @return the power measurement
 */
Q16_15 powerMeasurement_magnitude( const Q_15* src, BAM16 freq, int N);


/**
 * Performs a real mode Fourier Transform at a given single phase.
 *
 * @param dst buffer to write the output of the transform
 * @param src the signal under test
 * @param order the order of magnitude of the transform to perform. size = 2^order.
 * @param phase the phase offset to measure the energy at
 */
void FFT_inphase( Q_15* dst, const Q_15* src, int order, BAM8 phase );

/**
 * Performs a real mode Fourier Transform across all phases keeping only the
 * magnitude.
 *
 * @param dst buffer to write the output of the transform
 * @param src the signal under test
 * @param order the order of magnitude of the transform to perform. size = 2^order.
 */
void FFT_magnitude( Q_15* dst, const Q_15* src, int order );


//void Complex_FFT( Complex16* dst, Complex16* src, int order );
//void Complex_IFT( Complex16* dst, Complex16* src, int order );

//void Real2Complex_FFT( Complex16* dst, Q_15* src, int order );
//void Complex2Real_IFT( Q_15* dst, Complex16* src, int order );


// Filters
// LPF
// HPF
// BPF

// Define Filter
// Apply Filter




/*****************************************************************************/
#ifdef  __cplusplus
}
#endif

#endif // DSP_H

/**
 * [Description]
 *
 * @param
 * @return
 */
