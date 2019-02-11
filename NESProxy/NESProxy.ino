
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
#define SNOOP_CLK   2
#define SNOOP_LATCH 3
#define SNOOP_DATA  4

// Specify the digitial pins you've connected to the controller
// interface that button state should be forwarded too...
#define PROXY_CLK   5
#define PROXY_LATCH 6
#define PROXY_DATA  7

#define LED   13

// The intermediate (in-progress) 'reading' state is cleared for
// each SNOOP_LATCH pulse and then updated for each subsequent
// SNOOP_CLK pulse until we've read 8 bits from the 4021 shift
// register at which point we copy this into 'last_read'...
volatile uint8_t reading = 0;
volatile uint8_t last_read = 0;

// Reset by a SNOOP_LATCH pulse and incremented each SNOOP_CLK
// pulse to track which register bit we're expecting to read
// next...
volatile uint8_t reading_bit = 0;

// When we get a PROXY_LATCH from the console that we're proxying
// to we copy 'last_read' into 'writing' and similar to how
// we bump 'reading_bit' for each SNOOP_CLK, we bump
// writing_bit for each PROXY_CLK until we've written all
// eight register bits...
volatile uint8_t writing = 0;
volatile uint8_t writing_bit = 0;

void snoop_latch_rise_isr()
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
  reading = (digitalRead(SNOOP_DATA) == LOW);
  reading_bit = 1;
}

void snoop_clock_fall_isr()
{
  // make sure the latch isn't still high before starting to
  // read bits 1-7...
  while (digitalRead(SNOOP_LATCH) == HIGH)
    ;

  // At least running with an AVS (FPGA NES) system, and
  // running on a fast (96MHz Teensy 3.2) cpu then it seems
  // the data isn't reliable exactly when the clock falls but
  // seems more reliable with a minimal delay here...
  delayMicroseconds(3);

  uint8_t rb = reading_bit; // only want one volatile read
  if (rb < 8)
  {
    if (digitalRead(SNOOP_DATA) == LOW)
      reading |= 1<<rb;
    if (rb == 7)
      last_read = reading;
    reading_bit++;
  }
}

void proxy_latch_rise_isr()
{
  uint8_t latest = last_read;
  writing = latest;

  digitalWrite(PROXY_DATA, (latest & 0x1) ? LOW : HIGH);
  writing_bit = 1;
}

void proxy_clock_rise_isr()
{
  uint8_t wb = writing_bit; // only want one volatile read
  if (wb < 8)
  {
    digitalWrite(PROXY_DATA, (writing & (1<<wb)) ? LOW : HIGH);
    writing_bit++;
  }
}

void setup()
{
  pinMode(SNOOP_DATA, INPUT);
  attachInterrupt(digitalPinToInterrupt(SNOOP_LATCH),
                  snoop_latch_rise_isr, RISING);
  attachInterrupt(digitalPinToInterrupt(SNOOP_CLK),
                  snoop_clock_fall_isr, FALLING);

  attachInterrupt(digitalPinToInterrupt(PROXY_LATCH),
                  proxy_latch_rise_isr, RISING);
  attachInterrupt(digitalPinToInterrupt(PROXY_CLK),
                  proxy_clock_rise_isr, RISING);
  pinMode(PROXY_DATA, OUTPUT);
  digitalWrite(PROXY_DATA, HIGH);

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
