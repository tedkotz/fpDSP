/**
 * @file    samples.cpp
 * @author  Ted Kotz <ted@kotz.us>
 * @version 0.1
 *
 * Manages the collection of samples from the ADC at a set Sample Rate.
 *
 */

/* Includes ******************************************************************/
#include "samples.h"

#include <Arduino.h>

/* Defines *******************************************************************/
#warning Building fpDSP/samples.cpp

/* Types *********************************************************************/
/* Interfaces ****************************************************************/
/* Data **********************************************************************/
/* Functions *****************************************************************/

void SampleBuffer_init( SampleBuffer* sb)
{
  sb->in=0;
  sb->out=0;
}

int SampleBuffer_size( SampleBuffer* sb)
{
  // Wrap is automatically handled for 256 size buffer by using uint8_t indexes
  uint8_t tmp=sb->in - sb->out;

  return (tmp);
}

int SampleBuffer_free( SampleBuffer* sb)
{
  // (SAMPLE_BUFFER_SIZE - 1) - SampleBuffer_size
  // = ((SAMPLE_BUFFER_SIZE-1) - (in - out))  mod SAMPLE_BUFFER_SIZE
  // = -1 - (in - out)
  // = -1 - in + out
  // = -1 + (~in + 1) + out
  // = ~in + out
  uint8_t tmp = ~sb->in + sb->out;

  return (tmp);
}

bool SampleBuffer_empty( SampleBuffer* sb)
{
  return (sb->in == sb->out);
}

bool SampleBuffer_full( SampleBuffer* sb)
{
  // SampleBuffer_free == 0
  // = not SampleBuffer_free
  // = not( ~in + out )
  uint8_t tmp = ~sb->in + sb->out;

  return !tmp;
}

void SampleBuffer_push( SampleBuffer* sb, uint16_t sample)
{
  sb->buff[sb->in++]=sample;
}

uint16_t SampleBuffer_pop( SampleBuffer* sb)
{
  return sb->buff[sb->out++];
}

int SampleBuffer_popAllOrNothing( SampleBuffer* sb, Q_15* buf, int count)
{
  if (SampleBuffer_size( sb ) < count)
  {
    return 0;
  }
  else
  {
    for( int i=0; i<count; ++i)
    {
      *buf++ = SampleBuffer_pop(sb);
    }
    return count;
  }
}

int SampleBuffer_pushAllOrNothing( SampleBuffer* sb, Q_15* buf, int count)
{
  if (SampleBuffer_free( sb ) < count)
  {
    return 0;
  }
  else
  {
    for( int i=0; i<count; ++i)
    {
       SampleBuffer_push(sb, *buf++);
    }
    return count;
  }
}


void ADC_StreamSetup( int pin, int sampleTime_us)
{
  uint16_t ticks = (sampleTime_us << 1) - 1;     // ticks = period * 2 ticks per us - 1 event tick

  if (pin >= A0)
  {
    pin -= A0;
  }

  noInterrupts();

  // Setup Timer 1
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  TCCR1B = bit (CS11) | bit (WGM12);  // 16MHz clk / 8 prescaler -> 2 ticks per us
  TIMSK1 = bit (OCIE1B);
  OCR1A = ticks;
  OCR1B = ticks;

  // Setup ADC for auto trigger on Timer 1 B

  // REFS(7:6):
  //  0 0 AREF, internal VREF turned off
  // +0 1 AVCC with external capacitor at AREF pin
  //  1 0 Reserved
  //  1 1 Internal 1.1V voltage reference with external capacitor at AREF pi
  // ADLAR(5): 0 right adjust, 1 Left adjust
  // input select (3:0): normal pins 0-7
  ADMUX = bit (REFS0) | (pin & 0x07);

  // Bit 7,5-3: 0 Don't Care
  // Bit 6: 0 Disable Analog Comparator ACME
  // Bit 2-0: Auto Trigger Source ADTS
  //   0 0 0 Free running mode
  //   0 0 1 Analog comparator
  //   0 1 0 External interrupt request 0
  //   0 1 1 Timer/Counter0 compare match A
  //   1 0 0 Timer/Counter0 overflow
  //  +1 0 1 Timer/Counter1 compare match B
  //   1 1 0 Timer/Counter1 overflow
  //   1 1 1 Timer/Counter1 capture event
  // NO CLK MODE ADCSRB = 0;
  ADCSRB = bit (ADTS2) | bit (ADTS0);  // Timer/Counter1 Compare Match B

  // ADEN   7: 1 Enable ADC
  // ADSC   6: 0 Don't start a conversion
  // ADATE  5: 0 Auto Trigger Enable (Wait until ready to start)
  // ADIF   4: 1 Clear the interrupt flag
  // ADIE   3: 1 Enable ADC interrupt
  // ADPS 2-0: X Prescaler at slowest viable speed
  //ADCSRA = 0xA8;
  ADCSRA =  bit (ADEN) | bit (ADIE) | bit (ADIF);   // turn ADC on, want interrupt on completion
  // set prescaler to highest value less than ticks per sample assumes
  // steady state 13 clocks per reading
  if( ticks > 208 )      // < 9.5kHz
  {
     ADCSRA |= 7; // Prescaler of 128
  }
  else if( ticks > 104 ) // < 19kHz
  {
     ADCSRA |= 6; // Prescaler of 64
  }
  else if( ticks > 52 ) // < 37kHz
  {
     ADCSRA |= 5; // Prescaler of 32
  }
  else if( ticks > 26 ) // < 74kHz
  {
     ADCSRA |= 4; // Prescaler of 16
  }
  else if( ticks > 13 ) // < 142kHz
  {
     ADCSRA |= 3; // Prescaler of 8
  }
  else if( ticks > 7 )  // < 250kHz
  {
     ADCSRA |= 2; // Prescaler of 4
  }
  else // if( ticks > 4 ) < 400kHz
  {
     ADCSRA |= 1; // Prescaler of 2
  }

  ADCSRA |= bit (ADATE);   // turn on automatic triggering
  // NO CLK MODE ADCSRA |= bit (ADSC);   // get single sample

  interrupts();
}

void ADC_StreamStop()
{
  // Disable Timer 1B
  TIMSK1 &= ~(bit (OCIE1B));

  // Disable ADC
  ADCSRA = 0;
}

Q_15 ADC_readCurrentSample( void )
{
  // convert 10-bit unsigned Sample to 16-bit signed value with 12dB headroom
  return (ADC - 0x0200) << 4;
}

int getSamples( int pin, Q_15* buf, int count, int sampleTime_us)
{
  if(sampleTime_us < PERIOD_US_8KHZ)
  {
    return 0;
  }
  unsigned long deltaTime = sampleTime_us;
  unsigned long timestamp=micros()-deltaTime;
  int countDown=count;

  while(countDown--)
  {
    while((micros()-timestamp) < deltaTime)
    {
      // do nothing
    }
    // convert 10-bit unsigned Sample to 16-bit signed value with 12dB headroom
    *buf++ = (analogRead(pin) - 0x200) << 4;
    timestamp += deltaTime;
  }

  return count;
}




