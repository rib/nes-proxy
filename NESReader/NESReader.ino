
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

// A minimal test program for reading a controllers button state

// Digital pins we've connected the NES controller wires to...
#define CLK   2
#define LATCH 3
#define DATA  4

void setup()
{
   pinMode(LATCH, OUTPUT);
   digitalWrite(LATCH, LOW);
   
   pinMode(CLK, OUTPUT);
   digitalWrite(CLK, LOW);

   pinMode(DATA, INPUT);

   Serial.begin(9600);
}

void loop()
{
  char val = 0;
 
  digitalWrite(LATCH, HIGH);
  digitalWrite(LATCH, LOW);
 
  if (digitalRead(DATA) == HIGH)
    val |= 1<<0;

  digitalWrite(CLK, HIGH);
  digitalWrite(CLK, LOW);
  if (digitalRead(DATA) == HIGH)
    val |= 1<<1;
 
  digitalWrite(CLK, HIGH);
  digitalWrite(CLK, LOW);
  if (digitalRead(DATA) == HIGH)
    val |= 1<<2;
    
  digitalWrite(CLK, HIGH);
  digitalWrite(CLK, LOW);
  if (digitalRead(DATA) == HIGH)
    val |= 1<<3;

  digitalWrite(CLK, HIGH);
  digitalWrite(CLK, LOW);
  if (digitalRead(DATA) == HIGH)
    val |= 1<<4;

  digitalWrite(CLK, HIGH);
  digitalWrite(CLK, LOW);
  if (digitalRead(DATA) == HIGH)
    val |= 1<<5;

  digitalWrite(CLK, HIGH);
  digitalWrite(CLK, LOW);
  if (digitalRead(DATA) == HIGH)
    val |= 1<<6;

  digitalWrite(CLK, HIGH);
  digitalWrite(CLK, LOW);
  if (digitalRead(DATA) == HIGH)
    val |= 1<<7;

  Serial.print(val, BIN);
}
