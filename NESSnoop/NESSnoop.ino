
// Copyright (C) 2019 Robert Bragg <robert@sixbynine.org>
//
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use, copy,
// modify, merge, publish, distribute, sublicense, and/or sell copies
// of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
// BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

// A minimal program for spying on the NES controller's shift
// register protocol and simply logging the button state to
// the serial port.

// Specify the digital pins you've connected the NES controller
// wires to here...
// NOTE: CLK and LATCH must be connected to pins supporting
// interrupts.
// NOTE: Arduino Uno/Nano/Duemilanove only support external
// interrupts on digital pins 2 and 3
#define CLK   2
#define LATCH 3
#define DATA  4

#define LED   13

// The latest state is cleared for each LATCH pulse and then
// updated by subsequent CLK pulse until we've read 8 bits
// from the 4021 shift register at which point we copy
// this into 'last_read'...
volatile uint8_t reading = 0;
volatile uint8_t last_read = 0;

// Reset by a LATCH pulse and incremented each CLK pulse to
// track which register bit we're expecting to read next...
volatile uint8_t reading_bit = 0;

void latch_rise_isr()
{
  // At this point we can reset the 'reading' state as the
  // console has just requested to latch and read the latest
  // sequence of 8 register bits.
  //
  // We can immediately read the level of bit 0 from the
  // 4021 shift register
  //
  // DATA == LOW implies the button is pressed.
  //
  reading = (digitalRead(DATA) == LOW);
  reading_bit = 1;
}

void clock_fall_isr()
{
  // make sure the latch isn't still high before starting to
  // read bits 1-7...
  while (digitalRead(LATCH) == HIGH)
    ;

  // At least running with an AVS (FPGA NES) system, and
  // running on a fast (96MHz Teensy 3.2) cpu then it seems
  // the data isn't reliable exactly when the clock falls but
  // seems more reliable with a minimal delay here...
  delayMicroseconds(3);

  uint8_t rb = reading_bit; // only want one volatile read
  if (rb < 8)
  {
    if (digitalRead(DATA) == LOW)
      reading |= 1<<rb;
    if (rb == 7)
      last_read = reading;
    reading_bit++;
  }
}

void setup()
{
  pinMode(DATA, INPUT);
  attachInterrupt(digitalPinToInterrupt(LATCH), latch_rise_isr, RISING);
  attachInterrupt(digitalPinToInterrupt(CLK), clock_fall_isr, FALLING);

  pinMode(LED, OUTPUT);

  Serial.begin(9600);
}

void loop()
{
  uint8_t latest = last_read; // just one volatile read per loop

  Serial.println(latest, BIN);
  if (latest)
    digitalWrite(LED, HIGH);
  else
    digitalWrite(LED, LOW);
  delay(10);
}
