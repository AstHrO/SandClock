# Sandclock for Arduino

easy Hourglass for Arduino

V1.1 -  february 2016
Blenderlab (http://www.blenderlab.fr / http://www.asthrolab.fr)
Fab4U (http://www.fab4U.de)

- Randomly choose the amout of sand to fill the upperglass
- Drops one marble every second

# Hardware needed : 
- Arduino (Uno,Mini,Micro...)
- 2 Led Matrix with MAX7219 controller
- Wires, powersupply ...;-D
- Standing stuff is ready to print : http://www.thingiverse.com/thing:1316240

# How To :
- Connect the first matrix to the arduino (5 pins, as follow)
- Connect (chain) the second matrix to the first one : All same pins except (DOUT->DIN)
- That's it !

# Todo 
- Manage an Upside/down movement -> Reset (Shake captor)
- Interface to select the duration (Touch button)
