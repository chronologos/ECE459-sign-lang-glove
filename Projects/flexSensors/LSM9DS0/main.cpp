//Most of the Credit goes to jimblom and Judah Okeleye
#include "LSM9DS0.h"
#include "mbed.h"

// SDO_XM and SDO_G are both grounded, so our addresses are:
#define LSM9DS0_XM  0x1D // Would be 0x1E if SDO_XM is LOW
#define LSM9DS0_G   0x6B // Would be 0x6A if SDO_G is LOW


LSM9DS0 dof(p9, p10,LSM9DS0_G, LSM9DS0_XM);
Serial pc(USBTX, USBRX); // tx, rx

//Uncomment If you want to use interrupts
/*DigitalIn pinDRDYG(p14);
DigitalIn pinINTG(p13);
DigitalIn pinINT1_XM(p14);
DigitalIn pinINT2_XM(p15);*/

#define PRINT_CALCULATED
#define PRINT_SPEED 500 // 500 ms between prints

//Init Serial port and LSM9DS0 chip
void setup()
{
    pc.baud(115200); // Start serial at 115200 bps
    // Use the begin() function to initialize the LSM9DS0 library.
    // You can either call it with no parameters (the easy way):
    uint16_t status = dof.begin();
    
   
   //Make sure communication is working
    pc.printf("LSM9DS0 WHO_AM_I's returned: 0x");
    pc.printf("%x\n",status);
    pc.printf("Should be 0x49D4\n");
    pc.printf("\n");
}


void printGyro()
{
    // To read from the gyroscope, you must first call the
    // readGyro() function. When this exits, it'll update the
    // gx, gy, and gz variables with the most current data.
    
    //Uncomment code if you plan on using interrupts
    /*while(pinDRDYG == 0)
    {
        ;
    }*/
    
    
    dof.readGyro();

    // Now we can use the gx, gy, and gz variables as we please.
    // Either print them as raw ADC values, or calculated in DPS.
    pc.printf("G: ");
#ifdef PRINT_CALCULATED
    // If you want to print calculated values, you can use the
    // calcGyro helper function to convert a raw ADC value to
    // DPS. Give the function the value that you want to convert.
    //pc.printf("The bias: %2f, %2f, %2f \n", dof.gbias[0], dof.gbias[1], dof.gbias[2]);
    pc.printf("%2f",dof.calcGyro(dof.gx) - dof.gbias[0]);
    pc.printf(", ");
    pc.printf("%2f",dof.calcGyro(dof.gy) - dof.gbias[1]);
    pc.printf(", ");
    pc.printf("%2f\n",dof.calcGyro(dof.gz) - dof.gbias[2]);
#elif defined PRINT_RAW
    pc.printf("%d", dof.gx);
    pc.printf(", ");
    pc.printf("%d",dof.gy);
    pc.printf(", ");
    pc.printf("%d\n",dof.gz);
#endif
}

void printAccel()
{
    //Uncomment code if you plan on using interrupts
    /*while(pinINT1_XM == 0)
    {
        ;
    }*/
    

    dof.readAccel();
    
    // Now we can use the ax, ay, and az variables as we please.
    // Either print them as raw ADC values, or calculated in g's.
    pc.printf("A: ");
#ifdef PRINT_CALCULATED
    // If you want to print calculated values, you can use the
    // calcAccel helper function to convert a raw ADC value to
    // g's. Give the function the value that you want to convert.
    //pc.printf("The bias: %2f, %2f, %2f \n", dof.abias[0], dof.abias[1], dof.abias[2]);
    pc.printf("%2f",dof.calcAccel(dof.ax) - dof.abias[0]);
    pc.printf(", ");
    pc.printf("%2f",dof.calcAccel(dof.ay) - dof.abias[1]);
    pc.printf(", ");
    pc.printf("%2f\n",dof.calcAccel(dof.az) - dof.abias[2]);
#elif defined PRINT_RAW
    pc.printf("%d",dof.ax);
    pc.printf(", ");
    pc.printf("%d",dof.ay);
    pc.printf(", ");
    pc.printf("%d\n",dof.az);
#endif
    
}

void printMag()
{
     //Uncomment code if you plan on using interrupts
    /*while(pinINT2_XM == 0)
    {
        ;
    }*/
    
    // To read from the magnetometer, you must first call the
    // readMag() function. When this exits, it'll update the
    // mx, my, and mz variables with the most current data.
    dof.readMag();
    
    // Now we can use the mx, my, and mz variables as we please.
    // Either print them as raw ADC values, or calculated in Gauss.
    pc.printf("M: ");
#ifdef PRINT_CALCULATED
    // If you want to print calculated values, you can use the
    // calcMag helper function to convert a raw ADC value to
    // Gauss. Give the function the value that you want to convert.
    pc.printf("%2f",dof.calcMag(dof.mx));
    pc.printf(", ");
    pc.printf("%2f",dof.calcMag(dof.my));
    pc.printf(", ");
    pc.printf("%2f\n",dof.calcMag(dof.mz));
#elif defined PRINT_RAW
    pc.printf("%d", dof.mx);
    pc.printf(", ");
    pc.printf("%d", dof.my);
    pc.printf(", ");
    pc.printf("%d\n", dof.mz);
#endif
}

void loop()
{
    printGyro();  // Print "G: gx, gy, gz" 
    printAccel(); // Print "A: ax, ay, az"
    printMag();   // Print "M: mx, my, mz"    
    wait_ms(PRINT_SPEED);
}


int main()
{
    setup();  //Setup sensor and Serial
    pc.printf("Hello World\n");

    while (true)
    {
         //Calculate bias
         dof.calLSM9DS0(dof.gbias, dof.abias);
         //pc.printf("Setup Done\n");
         loop(); //Get sensor values
   }
}

