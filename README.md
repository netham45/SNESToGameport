# SNESToGameport

Converts an SNES controller to a Gameport.

SNES wiring:    
    Power - 3.3V    
    Latch - PA4    
    Clock - PA5    
    Data - PA6    

Gameport Axis Schematic:    
    ![Axis Schematic](https://github.com/netham45/SNESToGameport/blob/main/axis_schematic.png?raw=true)
     
Gameport Wiring:    
    ![Gameport Pinout](https://github.com/netham45/SNESToGameport/blob/main/gameport_pinout.png?raw=true)    
	
Pin No - Description   - Connect To    
	 1 - 5v            - See Axis Schematic    
	 2 - Button 1      - PB10    
	 3 - Joy 1 X Axis  - X1 Out - See Gameport Axis Schematic    
	 4 - Ground        - Tie one ground to STM32 Ground    
	 5 - Ground        - Tie one ground to STM32 Ground    
	 6 - Joy 1 Y Axis  - Y1 Out - See Gameport Axis Schematic    
	 7 - Button 2      - PB12    
	 8 - 5v            - See Axis Schematic    
	 9 - 5v            - See Axis Schematic    
	10 - Button 3      - PB13    
	11 - Joy 2 X Axis  - X2 Out - See Gameport Axis Schematic    
	12 - Midi Out      - Disconnected    
	13 - Joy 2 Y Axis  - Y2 Out - See Gameport Axis Schematic    
	14 - Button 4      - PB14    
	15 - 5v            - See Axis Schematic    

STM32 wiring:    
    X1 Center  - PB0    
    X1 Max     - PB1    
    Y1 Center  - PB2    
    Y1 Max     - PB3    
    X2 Center  - PB15    
    X2 Max     - PC15    
    Y2 Center  - PB6    
    Y2 Max     - PB7    
    Button 1   - PB10    
    Button 2   - PB12    
    Button 3   - PB13    
    Button 4   - PB14    
	
    
16x2 hd44780 LCD i2c wiring:    
    Power - 5v    
    SDA - PB9    
    SCL - PB8    

Project made for an stm32f401ccu
