/**
 * @file    samples.h
 * @author  Ted Kotz <ted@kotz.us>
 * @version 0.1
 *
 * Manages the collection of samples from the ADC at a set Sample Rate.
 *
 *  Circular Sample buffer
 *  putSample
 *  getSample
 *  putSamples
 *  getSamples
 *  size
 *  full
 *  contents
 *
 */
#ifndef   SAMPLES_H
#define   SAMPLES_H

/* Includes ******************************************************************/
#include <inttypes.h>
#include "DSP.h"

/* Defines *******************************************************************/
// FIXED SIZE allows for a lot of optimizations vs parameterized size.
#define SAMPLE_BUFFER_SIZE 256

#define PERIOD_US_8KHZ 125
#define PERIOD_US_6666HZ 150
#define PERIOD_US_5KHZ 200
#define PERIOD_US_4KHZ 250
#define PERIOD_US_3333HZ 300
#define PERIOD_US_2KHZ 500

/* Types *********************************************************************/
typedef struct SampleBuffer
{
  uint16_t buff[SAMPLE_BUFFER_SIZE];
  uint8_t in;
  uint8_t out;
} SampleBuffer;

/* Interfaces ****************************************************************/
/* Data **********************************************************************/
/* Functions *****************************************************************/

/**
 * Initializes SampleBuffer sb
 *
 * @param sb the SampleBuffer to initialize
 */
void SampleBuffer_init( SampleBuffer* sb);

/**
 * Returns used space in SampleBuffer
 *
 * @param sb the SampleBuffer
 * @return number of used slots in SampleBuffer sb
 */
int SampleBuffer_size( SampleBuffer* sb);

/**
 * Returns free space in SampleBuffer
 *
 * @param sb the SampleBuffer
 * @return number of available slots in SampleBuffer sb
 */
int SampleBuffer_free( SampleBuffer* sb);

/**
 * [Description]
 *
 * @param
 * @return
 */
bool SampleBuffer_empty( SampleBuffer* sb);

/**
 * [Description]
 *
 * @param
 * @return
 */
bool SampleBuffer_full( SampleBuffer* sb);

/**
 * [Description]
 *
 * @param
 * @return
 */
void SampleBuffer_push( SampleBuffer* sb, uint16_t sample);

/**
 * [Description]
 *
 * @param
 * @return
 */
uint16_t SampleBuffer_pop( SampleBuffer* sb);

/**
 * If the SampleBuffer has free space for  at least count samples it will push
 * that many from buf, otherwise it will only return 0.
 *
 * @param sb the SampleBuffer
 * @param buf source buffer for samples to push
 * @param count number of samples to push
 * @return number of samples pushed
 */
int SampleBuffer_pushAllOrNothing( SampleBuffer* sb, Q_15* buf, int count);

/**
 * If the SampleBuffer has at least count samples it will pop them into
 * buf, otherwise it will only return 0.
 *
 * @param sb the SampleBuffer
 * @param buf destination buffer for popped samples
 * @param count number of samples to pop
 * @return number of samples popped
 */
int SampleBuffer_popAllOrNothing( SampleBuffer* sb, Q_15* buf, int count);

/**
 * Sets up a Real Time sample stream so that ADC samples are taken at the
 * specified sample rate resulting in triggers to the ADC_vect, where the
 * samples can be read with ADC_readCurrentSample and then stored for later
 * processing
 *
 *     ISR (ADC_vect)
 *     {
 *       SampleBuffer_push(&mySampleBuffer, ADC_readCurrentSample());
 *     }
 *
 *
 * @note Uses the Timer 1 and the B counter. an ISR for the timer must be
 * registered as it will be invoked
 *
 *     ISR(TIMER1_COMPB_vect) {}
 *
 * @param pin which analog input to sample on
 * @param sampleTime_us - the period of time between sampling in microseconds
 */
void ADC_StreamSetup( int pin, int sampleTime_us);

/**
 * Stops the ADC sample stream
 *
 */
void ADC_StreamStop(void);

/**
 * Reads and formats the current ADC sample from the ADC Stream.
 * This function is intended to be called in the ADC_vect.
 * Best use is probably pushing it into a SampleBuffer for latter processing
 *
 * @return A single Q_15 sample
 */
Q_15 ADC_readCurrentSample( void );

/**
 * An un-buffered blocking way to read a collection of sampled analog data from an analog input.
 * This is a attempt to limit the calls to only standard Arduino APIs, avoiding hw specific
 * interrupts and register accesses. It is perfect for getting a quick sample of an analog input
 * for future processing such as to update a spectrum display or detect inline tones.
 *
 * @param pin Arduino pin identifier for the analog pin to use
 * @param buf the buffer to write the samples to
 * @param count the number of samples to wait for
 * @param sampleTime_us time in microseconds between samples
 * @return Number of samples read
 */
int getSamples( int pin, Q_15* buf, int count, int sampleTime_us);

#endif // SAMPLES_H
