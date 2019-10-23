/**
 * Using the PS2Mouse library available from
 *   http://github.com/kristopher/PS2-Mouse-Arduino/
 * Original by Kristopher Chambers <kristopher.chambers@gmail.com>
 * Updated by Jonathan Oxer <jon@oxer.com.au>
 * 
 * Using code from Spark cssvb94
 *   https://github.com/cssvb94/AmigaUSBMouseJoystick
 *   
 * 
 * 
 * 
 * 
 * Data from PS2-Mouse:
 * 
 * data0[2:1:0]  ->  middle,right, left mousebutton
 * data1         ->  x from -127 to + 127   signed int
 * data2         ->  y from -127 to + 127   signed int
 * 
 * 
 * amiga mouse-port, looking at the amiga:
 * 
 * 9 PIN D-SUB MALE
 * 
 *   _________________________
 *   \                       /
 *    \  1   2   3   4   5  /
 *     \                   /
 *      \  6   7   8   9  /
 *       \_______________/
 * 
 * 1  V-pulse               
 * 2  H-pulse
 * 3  VQ-pulse
 * 4  HQ-pulse
 * 5  BUTTON 3 (Middle)
 * 6  BUTTON 1 (Lefl)
 * 7  +5V
 * 8  GND
 * 9  BUTTON 2 (Right)
 * 
 * 
 * 
 * 
 * 
 */

 

#include <PS2Mouse.h>

#define MOUSE_DATA  12
#define MOUSE_CLOCK 11

PS2Mouse mouse(MOUSE_CLOCK, MOUSE_DATA, STREAM);



// quadrature encoding used by the amiga mouse protocol
//
uint8_t H[4]  = { LOW, LOW, HIGH, HIGH};
uint8_t HQ[4] = { LOW, HIGH, HIGH, LOW};

uint8_t QX = 3;
uint8_t QY = 3;

uint8_t XSTEPS;
uint8_t YSTEPS;
uint8_t XSIGN;
uint8_t YSIGN;



#define ADELAY  150



// pins from amiga-port to arduino 
//
#define V_PULSE  2
#define H_PULSE  3
#define VQ_PLSE  4
#define HQ_PLSE  5

#define LMB      6
#define RMB      7
#define MMB      8



// MB handling
//
void LeftButtonUp() {
    digitalWrite(LMB, HIGH);
}
void LeftButtonDown() {
    digitalWrite(LMB, LOW);
}

void RightButtonUp() {
    digitalWrite(RMB, HIGH);
}
void RightButtonDown() {
    digitalWrite(RMB, LOW);
}

void MiddleButtonUp() {
    digitalWrite(MMB, HIGH);
}
void MiddleButtonDown() {
    digitalWrite(MMB, LOW);
}



// x/y handling
//
void AMIGAHorizontalMove() {
  
    // set bits acc. to curr. position in quadr. sequence
    digitalWrite(H_PULSE, H[QX]);
    digitalWrite(HQ_PLSE, HQ[QX]);

    delayMicroseconds(ADELAY);
}


void AMIGAVerticalMove() {
  
    digitalWrite(V_PULSE, H[QY]);
    digitalWrite(VQ_PLSE, HQ[QY]);
    
    delayMicroseconds(ADELAY);
}


void AMIGA_Left() {
    
    // do a move by setting the port
    AMIGAHorizontalMove();

    // advance in the quadr. sequence
    QX = (QX >= 3) ? 0 : ++QX;
    
}

void AMIGA_Right() {
    AMIGAHorizontalMove();
    QX = (QX <= 0) ? 3 : --QX;
}

void AMIGA_Down() {
    AMIGAVerticalMove();
    QY = QY <= 0 ? 3 : --QY;
}

void AMIGA_Up() {
    AMIGAVerticalMove();
    QY = QY >= 3 ? 0 : ++QY;
}




void setup() {
    
    Serial.begin(57600);


    // init ps2 connection
    mouse.initialize();
  
    // tell me sth. 'bout you...
    Serial.println("Hi, I'm a PS2 to amiga mouse adapter! \nConnect");
    
    Serial.print("PS2 Data:       D");
    Serial.println(MOUSE_DATA);
    
    Serial.print("PS2 Clock:      D");
    Serial.println(MOUSE_CLOCK);
    

    Serial.print("Amiga V_PULSE:  D");
    Serial.println(V_PULSE);
    
    Serial.print("Amiga H_PULSE:  D");
    Serial.println(H_PULSE);
    
    Serial.print("Amiga VQ_PLSE:  D");
    Serial.println(VQ_PLSE);
    
    Serial.print("Amiga HQ_PLSE:  D");
    Serial.println(HQ_PLSE);


    Serial.print("Amiga Left MB:  D");
    Serial.println(LMB);

    Serial.print("Amiga Right MB: D");
    Serial.println(RMB);

    Serial.print("Amiga MB 3:     D");
    Serial.println(MMB);

    Serial.print("Amiga 5V:       5V");
    Serial.println();

    Serial.print("Amiga GND :     GND");
    Serial.println();


    

    
    
    // Set button and quadrature output pins to output
    pinMode(V_PULSE, OUTPUT);  // V-Pulse
    pinMode(H_PULSE, OUTPUT);  // H-Pulse
    pinMode(VQ_PLSE, OUTPUT);  // VQ-pulse
    pinMode(HQ_PLSE, OUTPUT);  // HQ-pulse
    pinMode(LMB, OUTPUT);  // LMB
    pinMode(RMB, OUTPUT);  // RMB
    pinMode(MMB, OUTPUT);  // MMB
    
        
    // Set quadrature output pins to zero
    digitalWrite(V_PULSE, LOW);
    digitalWrite(H_PULSE, LOW);
    digitalWrite(VQ_PLSE, LOW);
    digitalWrite(HQ_PLSE, LOW);
    
    
    // Set mouse button output pins to on, coz they are inverted
    digitalWrite(LMB, HIGH);
    digitalWrite(RMB, HIGH);
    digitalWrite(MMB, HIGH);
    
    delay(200);
}




void loop() {

    
    // get data from mouse
    //
    int16_t data[3];
    mouse.report(data);

    

    // handle buttons
    //
    if( data[0] & 1) 
      LeftButtonDown();
    else
      LeftButtonUp();
      
    if( data[0] & 2) 
      RightButtonDown();
    else
      RightButtonUp();
      
    if( data[0] & 4) 
      MiddleButtonDown();
    else
      MiddleButtonUp();




    // calc x/y movement
    //
    XSTEPS = abs(data[1]);
    YSTEPS = abs(data[2]);
    XSIGN = (data[1] > 0 ? 1 : 0) ;
    YSIGN = (data[2] > 0 ? 1 : 0) ;


    
    // handle x/y movement 
    //
    while ((XSTEPS | YSTEPS) != 0) {

        // steps left?
        if (XSTEPS != 0) {
          
            // direction
            if (XSIGN)
                AMIGA_Right();
            else
                AMIGA_Left();

            // decrease steps    
            XSTEPS--;
        }

        
        if (YSTEPS != 0) {
            if (YSIGN)
                AMIGA_Up(); 
            else
               AMIGA_Down();
            YSTEPS--;
        }
        
    }    
}






