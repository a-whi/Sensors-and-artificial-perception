/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "project.h"
#include <stdio.h>
#include <math.h>

int store_reading(int number) { //This function will store the reading into 4 place value variables to be returned
    //number needs to be in xxxx form, so I have not taken into account where the decimal point (dp) should be
    int 1000_count = 0; //these will be the variables to store the numbers of different place values
    int 100_count = 0;
    int 10_count = 0;
    int 1_count = 0;
    int result = 1001;
    for (int i = 0; i < 4; i++) {
        if (i == 0) {
            while (result > 1000) {
                result = number - 1000;
                1000_count++;
            }
        } else if (i == 1) {
            while (result > 100) {
                result = result - 100;
                100_count++;
            }
        } else if (i == 2) {
            while (result > 10) {
                result = result - 10;
                100_count++;
            }
        } else if (i == 3) {
            while (result > 1) {
                result = result - 1;
                100_count++;
            }
        }
    }
    return 1000_count,100_count,10_count,1_count;
}

int convert_count2binarray(int digit) { //converts the numbers saved in the count variables into array of binary numbers
    //for display_7seg function to use.
    int bin_array[8]; //initialises array of size 1x8 to show the on/off state of each segment and decimal point in 7-segment display
    if (digit == 0) {
        bin_array = [0,0,1,1,1,1,1,1]; //here the elements represent the following segments: [dp,G,F,E,D,C,B,A]
    } else if (digit == 1) {
        bin_array = [0,0,0,0,0,1,1,0];
    } else if (digit == 2) {
        bin_array = [0,1,0,1,1,0,1,1];
    } else if (digit == 3) {
        bin_array = [0,1,0,0,1,1,1,1];
    } else if (digit == 4) {
        bin_array = [0,1,1,0,0,1,1,0];
    } else if (digit == 5) {
        bin_array = [0,1,1,0,1,1,0,1];
    } else if (digit == 6) {
        bin_array = [0,1,1,1,1,1,0,1];
    } else if (digit == 7) {
        bin_array = [0,0,0,0,0,1,1,1];
    } else if (digit == 8) {
        bin_array = [0,1,1,1,1,1,1,1];
    } else if (digit == 9) {
        bin_array = [0,1,1,0,1,1,1,1];
    }
    return bin_array;
}

int display_7seg(int &array) { //takes array of size 1x8 of 0 and 1 for 1 count variable and send to correct transistor
    
int dec_num;
    char hex_num[2]; //note! This the hexadecimal number is defined as an array of characters which need to be converted to numbers
    for (int i = 0; i < 8; i++) {//this for loop is to invert the binary sequence becasue pin needs to be active low
        if (array[i] == 0)
            array[i] = 1;
        else {
            array[i] = 0;
        }
    }
    for (int j = 0; j < 8; j++) { //turning binary values into decimal
        if (array[j] == 1)
            dec_num += pow(2,j);
    }
    j = 0;
    while (dec_num != 0) { // code from https://www.sanfoundry.com/c-program-convert-decimal-hex/
        if ((dec_num%16) < 10)
            hex_num[j++] = 48 + (dec_num%16);
        else
            hex_num[j++] = 55 + (dec_num%16);
        dec_num = dec_num / 16;
    }
    /*long int binaryval, hexadecimalval = 0, j = 1, remainder; // I copied this code for converting binary to hexadecimal from https://www.sanfoundry.com/c-program-convert-binary-hex/
    while (binaryval != 0)
    {
        remainder = binaryval % 10;
        hexadecimalval = hexadecimalval + remainder * j;
        j = j * 2;
        binaryval = binaryval / 10;
    }
    printf("Equivalent hexadecimal value: %lX", hexadecimalval);
    */
}

int main(void)
{
CyGlobalIntEnable; /* Enable global interrupts. */
    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    Opamp_1_Start();
    PGA_1_Start();
    Comp_1_Start();
    VDAC8_1_Start();
    LCD_Seg_1_Start();
    for(;;)
    {
        /* Place your application code here. */
    }
}
/* [] END OF FILE */
