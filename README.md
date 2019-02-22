# Laser Harp

This is an arduino course project made using [the Arduino IDE][Arduino IDE]. My Laser Harp is trying to be a replica of a real harp, which uses only 8 lasers as chords. 

The working principle is as follows: each laser is shining on an LDR (Light Dependent Resistor), and when a sensor is getting lesser light (the laser beam has been interrupted), the corresponding note is played.  
At the initialization stage of the harp, each sensor reads some base values for the time when natural light, respectively a laser beam, shines on them. After this stage, the sensors will keep on reading values and compare them to the base values, in order to find out if a note is needed to be played.

For a demo of my laser harp you can check out [this video][video].

## Harp Features
- 8 chords (_C, D, E, F, G, A, B, C_);
- 7 octaves (playable notes from _C1_ to _C8_);
- easily changing the octaves by pressing one of the two buttons (_Increase Octave_/_Decrease Octave_);
- able to change sound volume by using the _Amplifier's Potentiometer_;
- able to pause (stop lasers and playing notes) by using the _Pause_ button;
- able to reset (rereading the base values) by using the _Reset_ button;
- when multiple notes are requested to be played at once, they will be played alternately fast enough to show that **this harp was not made to handle more than one note at a time** for an ordinary playing.

## Required Physical Components
- Arduino UNO Board;
- 3 x Breadboards;
- USB cable;
- 8 x 6mm Size 650nm Laser Head Copper Pointer Sight;
- 8 x 5mm LDRs;
- 8 x 1k Ω resistors;
- PAM8403 Mini 5V Digital Amplifier Board with Switch Potentiometer;
- 0.5W 8 OHM trumpet, Diameter 36mm;
- 4 x Push Buttons Switch;
- 4 x 10k Ω resistors;
- many long enough wires;
- expanded polystyrene (for harp's frame and box).

## The final product
<div>
  <img src="https://i.imgur.com/WD0i3eb.jpg" alt="Front View Image" width=50%>
  <img src="https://i.imgur.com/42DQdkE.jpg" alt="Back View Image" width=50%>
  <img src="https://i.imgur.com/R3jmAKC.jpg" alt="Top View Image" width=100%>
</div>

[Arduino IDE]: https://www.arduino.cc/en/main/software
[video]: https://youtu.be/Ydk5SONf4BQ
