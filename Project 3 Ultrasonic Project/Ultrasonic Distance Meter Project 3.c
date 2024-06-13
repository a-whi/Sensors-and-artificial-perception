 /* ========================================
 *
FINALLLLLLLL
 *
 * ========================================
*/
#include "project.h"
#include <stdio.h> 
#include <math.h>


volatile double total_time = 0;
volatile double distance = 0;
volatile double global_count = 0;
volatile double cal_table[54]; // 30 - 3 / 0.5 (Step Size)
volatile double avg_total = 0 ;
volatile uint8_t bool = 1;
char str[128];

int units;  // cm or inches
uint8 number; // Team number
volatile uint8_t button_pressed3 = 0;
volatile uint8_t program = 0;
volatile uint8_t calibration = 0;
volatile uint8_t cal_count = 0;
volatile uint8_t measure_count = 0;


/* 7 - Segment Guide - Chico State University */
const uint8 segment_map [ 10 ] = {
    0b11000000,
    0b11111001,
    0b10100100,
    0b10110000,
    0b10011001,
    0b10010010,
    0b10000010,
    0b11111000,
    0b10000000,
    0b10010000
} ;

const uint8 segment_map_d [ 10 ] = {
    0b01000000,
    0b01111001,
    0b00100100,
    0b00110000,
    0b00011001,
    0b00010010,
    0b00000010,
    0b01111000,
    0b00000000,
    0b00010000
} ;

const uint8 char_map [ 4 ] = {
    0b11111111, // Blank
    0b01111111, // Dot
    0b11000110, // C
    0b11001111  // I
} ;

void map_digit(int val, int disp_time){
    int thousands, hundreds, tens, ones;
    
    
    thousands = (uint8)(val/1000);
    hundreds = (uint8)((val-thousands*1000)/100);
    tens = (uint8)((val - thousands*1000 - hundreds*100)/10);
    ones = (uint8)(val - thousands*1000 - hundreds*100 - tens*10);
    
    thousands = segment_map[thousands];
    hundreds = segment_map[hundreds];
    tens = segment_map[tens];
    ones = segment_map[ones];
    
    for (int i = 0; i < disp_time; i++){
        digit_control_Write(14);
        segment_control_Write(thousands);
        CyDelay(1);
        digit_control_Write(13);
        segment_control_Write(hundreds);
        CyDelay(1);
        digit_control_Write(11);
        segment_control_Write(tens);
        CyDelay(1);
        digit_control_Write(7);
        segment_control_Write(ones);
        CyDelay(1);
    }
    
    segment_control_Write(char_map[0]);
      
}

void disp_dist(double dist, int disp_time){
    int intPart, fDigit, sDigit, tDigit, ftDigit;
    float fracPart;
    
    intPart = (int)dist;
    fracPart = dist - intPart;
    
    sDigit = intPart %10;
    fDigit = intPart / 10;
    
    fracPart *=100;
    int fracIntPart = (int)(fracPart+0.5);
    
    ftDigit = fracIntPart % 10;
    tDigit = (fracIntPart/10) % 10;
    
    for (int i = 0; i < disp_time; i++){
        digit_control_Write(14);
        segment_control_Write(segment_map[fDigit]);
        CyDelay(1);
        digit_control_Write(13);
        segment_control_Write(segment_map_d[sDigit]);
        CyDelay(1);
        digit_control_Write(11);
        segment_control_Write(segment_map[tDigit]);
        CyDelay(1);
        digit_control_Write(7);
        segment_control_Write(segment_map[ftDigit]);
        CyDelay(1);
    }
}

void idle(){
    digit_control_Write(14);
    segment_control_Write(char_map[1]);
    CyDelay(500);
    digit_control_Write(14);
    segment_control_Write(char_map[0]);
    CyDelay(500);
}

void buzzr(){
    buzz_Write(0);
    CyDelay(500);
    buzz_Write(1);
}

void start_sequence(){
    buzzr();
    int segs[] = {7,11,13,14};
    for (uint8 i = 0; i < 4; i++){
        // 14 = left most, 7 = right most
        digit_control_Write(segs[i]);
        segment_control_Write(0b00000000);
        CyDelay(1000);
        segment_control_Write(0b11111111);
    }
    
    // Display team number
    map_digit(number,250);
    
    if (units%2 == 0){ //Even == cm
        // Dislay C
        digit_control_Write(14);
        segment_control_Write(char_map[2]);
        CyDelay(1000);
        segment_control_Write(char_map[0]);
        CyDelay(1000);
    }else{
        // Dislay I
        digit_control_Write(14);
        segment_control_Write(char_map[3]);
        CyDelay(1000);
        segment_control_Write(char_map[0]);
        CyDelay(1000);
    }
}

void program_mode(){
    /*
    isr_b1_ClearPending();
    isr_b2_ClearPending();
    isr_b3_ClearPending();
    isr_timer_ClearPending();*/
    // Both LEDs turn on and off to signal we are in program mode
    LED_CM_Write(0);
    LED_I_Write(0);
    CyDelay(1000);
    if(btn_1_Read() == 1 && btn_2_Read() == 1){
        LED_CM_Write(1);
        LED_I_Write(1);
        LED_P_Write(0);
        program = 1;    // By setting to 1, button 1 & 2 can do increments
        // Set global button variables to 0
        
        button_pressed3 = 0;
        
    }else{
        LED_CM_Write(1);
        LED_I_Write(1);
        program = 0;
    }
}

void measure() {
    Control_Reg_1_Write(1);
    CyDelayUs(400);
    Control_Reg_1_Write(0);
}



void temp(){
    total_time = 1.09*((total_time*1e-5*343.2)*50) - 1.76; // Distance
    // Converting to inches
    units = EEPROM_ReadByte(0);
    if (units){
        total_time *= 2.54;
    }
    
    sprintf(str, "Distance: %d.%02d\n", (int)total_time, (int)(100 * total_time)%100) ;
    UART_PutString(str) ;
    
    int intPart, fDigit, sDigit, tDigit, ftDigit;
    float fracPart;
    
    intPart = (int)total_time;
    fracPart = total_time - intPart;
    
    sDigit = intPart %10;
    fDigit = intPart / 10;
    
    fracPart *=100;
    int fracIntPart = (int)(fracPart+0.5);
    
    ftDigit = fracIntPart % 10;
    tDigit = (fracIntPart/10) % 10;
    
    sprintf(str, "%d \n %d \n %d\n %d\n a", fDigit,sDigit,tDigit,ftDigit) ;
    UART_PutString(str); 
    
    for (int i = 0; i < 250; i++){
        digit_control_Write(14);
        segment_control_Write(segment_map[fDigit]);
        CyDelay(1);
        digit_control_Write(13);
        segment_control_Write(segment_map_d[sDigit]);
        CyDelay(1);
        digit_control_Write(11);
        segment_control_Write(segment_map[tDigit]);
        CyDelay(1);
        digit_control_Write(7);
        segment_control_Write(segment_map[ftDigit]);
        CyDelay(1);
    }
    
    isr_b1_ClearPending();
    isr_timer_ClearPending();
    isr_b2_ClearPending();
    isr_b3_ClearPending();
    isr_p_ClearPending();

}

CY_ISR(TimerISR){
    total_time = Timer_1_ReadCapture();  
    sprintf(str, "Timer value: %d.%02d\n", (int)total_time, (int)(100 * total_time)%100) ;
    UART_PutString(str) ;
    
    if(calibration){
        cal_table[cal_count] = total_time;
        sprintf(str, "Cal Count: %d out of 54\n", (int)cal_count) ;
        UART_PutString(str) ;
    }

    total_time = -0.0013627798457271276*total_time + 92.05995729344076;
    disp_dist(total_time, 250);
    
}

CY_ISR(Button1){
    buzzr();
    if(program){
    // BUTTON ACTS AS DECREMENT
        if (button_pressed3%2 == 0){
            number = EEPROM_ReadByte(1);
            number--;
            EEPROM_WriteByte(number,1);
          
            map_digit(number, 250);
            isr_b1_ClearPending();
        }else{
            units = 0;
            digit_control_Write(14);
            segment_control_Write(char_map[2]);
        }
    }else{ // Measure Mode
        
        measure();
        
    }
    isr_b1_ClearPending();
    isr_timer_ClearPending();
    isr_b2_ClearPending();
    isr_b3_ClearPending();
    isr_p_ClearPending();
    

}

CY_ISR(Button2){
    buzzr();
    if(program){
    // BUTTON ACTS AS INCREMENT
        if (button_pressed3%2 == 0){
            number = EEPROM_ReadByte(1);
            number++;
            EEPROM_WriteByte(number,1);
          
            map_digit(number, 250);
            isr_b1_ClearPending();
        }else{
            units = 1;
            digit_control_Write(14);
            segment_control_Write(char_map[3]);
        }
    }
    
    if(calibration){
        measure();
        cal_count = cal_count + 1;
    }
    
    isr_b1_ClearPending();
    isr_timer_ClearPending();
    isr_b2_ClearPending();
    isr_b3_ClearPending();
    isr_p_ClearPending();
}

CY_ISR(Button3){
    buzzr();
    if (program){
        button_pressed3++;
        if (button_pressed3%2 != 0){
            // Unit changing mode
            units = EEPROM_ReadByte(0);
            if (units){
                digit_control_Write(14);
                segment_control_Write(char_map[3]); // inch
                CyDelay(500);
                digit_control_Write(14);
                segment_control_Write(char_map[0]);
            }else{
                digit_control_Write(14);
                segment_control_Write(char_map[2]); // cm
                CyDelay(500);
                digit_control_Write(14);
                segment_control_Write(char_map[0]);
            }
        }else{
            // Save units to EEPROM 
            EEPROM_WriteByte(units,0);
            program = 0;
            LED_P_Write(1);
            button_pressed3 = 0;
            segment_control_Write(char_map[0]);
        }
    }
    isr_b1_ClearPending();
    isr_timer_ClearPending();
    isr_b2_ClearPending();
    isr_b3_ClearPending();
    isr_p_ClearPending();
}

CY_ISR(Program){
    program_mode();
}

CY_ISR(Calibrate){
    calibration = 1; 
    while(cal_count<54){
        LED_CM_Write(0);
        LED_I_Write(0);
        LED_P_Write(0);    
    }
    
    for(int i = 0; i<54; i++){
        sprintf(str, "Timer value @ %d : %d.%02d\n",(int)(i/2)+3, (int)cal_table[i], (int)(100 * cal_table[i])%100) ;
        UART_PutString(str) ;
    }
    
    calibration = 0; 
    LED_CM_Write(1);
    LED_I_Write(1);
    LED_P_Write(1);
}


int main(void){
    
    CyGlobalIntEnable; /* Enable global interrupts. */
    
    /* Start Components*/    
    Opamp_1_Start();
    PGA_1_Start();
    Comp_1_Start();///
    VDAC8_1_Start();///
    UART_Start();
    Count7_1_Start();
    Timer_1_Start();
    EEPROM_Start();
    //EEPROM_WriteByte(240,1);
    //EEPROM_WriteByte(0,0);
    
    units = EEPROM_ReadByte(0);
    number = EEPROM_ReadByte(1);
    
    /*Start Interrupts*/
    isr_b1_ClearPending();
    isr_b2_ClearPending();
    isr_b3_ClearPending();
    isr_p_ClearPending();
    isr_timer_ClearPending();  
    ////////////////////
  
    
    isr_timer_StartEx(TimerISR);
    isr_b1_StartEx(Button1);
    isr_b2_StartEx(Button2);
    isr_b3_StartEx(Button3);
    isr_c_StartEx(Calibrate);
    isr_p_StartEx(Program);
    
    //////////////////////////
    start_sequence();
    
    for(;;)
    {
        if (!program){
            idle();
        }else{
            if (button_pressed3%2 == 0){
                number = EEPROM_ReadByte(1);
                map_digit(number, 250);
                segment_control_Write(char_map[0]);
                CyDelay(500);
            }else{
                if (units){
                    digit_control_Write(14);
                    segment_control_Write(char_map[3]);
                    CyDelay(1000);
                    segment_control_Write(char_map[0]);
                    CyDelay(500);
                }else{
                    digit_control_Write(14);
                    segment_control_Write(char_map[2]);
                    CyDelay(1000);
                    segment_control_Write(char_map[0]);
                    CyDelay(500);
                }
            }
        }
    }
}

/* [] END OF FILE */
