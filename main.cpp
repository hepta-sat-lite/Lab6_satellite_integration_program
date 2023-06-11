#include "mbed.h"
#include "HEPTA_EPS.h"
#include "HEPTA_CDH.h"
#include "HEPTA_SENSOR.h"
#include "HEPTA_COM.h"
HEPTA_CDH cdh(PB_5, PB_4, PB_3, PA_8, "sd");
HEPTA_EPS eps(PA_0,PA_4);
HEPTA_SENSOR sensor(PA_7,PB_7,PB_6,0xD0);
HEPTA_COM com(PA_9,PA_10,9600);
DigitalOut condition(PB_1);
RawSerial sat(USBTX,USBRX, 38400);
Timer sattime;
int main()
{
    sat.printf("From Sat : Nominal Operation\r\n");
    com.printf("From Sat : Nominal Operation\r\n");
    int flag = 0; //condition flag
    float batvol, temp; //voltage, temperature 
    int rcmd=0,cmdflag=0;  //command variable
    sattime.start();
    eps.turn_on_regulator();//turn on 3.3V conveter
    sensor.setup();  
    for(int i=0;i<50;i++){
        com.xbee_receive(&rcmd,&cmdflag);
        
        //satellite condition led
        condition = !condition;
        
        //senssing HK data
        eps.vol(&batvol);
        sensor.temp_sense(&temp);
        
        //Transmitting HK data to Ground Station(GS)
        com.printf("HEPTASAT::Condition = %d, Time = %f [s], batvol = %2f [V], temp = %2f [deg C]\r\n",flag,sattime.read(),batvol,temp);
        wait_ms(1000);
        
        //Power Saving Mode 
        if((batvol <= 3.5)  | (temp > 35.0)){
            eps.shut_down_regulator();
            com.printf("Power saving mode ON\r\n"); 
            flag = 1;
        } else if((flag == 1) & (batvol > 3.7) & (temp <= 25.0)) {
            eps.turn_on_regulator();
            com.printf("Power saving mode OFF\r\n");
            flag = 0;
        }
        
        if(cmdflag == 1){
            if(rcmd == 'a'){
                sat.printf("rcmd=%c,cmdflag=%d\r\n",rcmd,cmdflag);
                com.printf("Hepta-Sat Lite Uplink Ok\r\n");
                for(int j=0;j<5;j++){
                    com.printf("Hello World!\r\n");
                    condition = 1;
                    wait_ms(1000);
                }
            }else if(rcmd == 'b') {
                sat.printf("rcmd=%c,cmdflag=%d\r\n",rcmd,cmdflag);
                com.printf("Hepta-Sat Lite Uplink Ok\r\n");
                char str[100];
                mkdir("/sd/mydir", 0777);
                FILE *fp = fopen("/sd/mydir/satdata.txt","w");
                if(fp == NULL) {
                    error("Could not open file for write\r\n");
                }
                for(int i = 0; i < 10; i++) {
                    eps.vol(&batvol);
                    fprintf(fp,"%f\r\n",batvol);
                    condition = 1;
                    wait_ms(1000);
                }
                fclose(fp);
                fp = fopen("/sd/mydir/satdata.txt","r");
                for(int i = 0; i < 10; i++) {
                    fgets(str,100,fp);
                    com.puts(str);
                }
                fclose(fp);
            }else if(rcmd == 'c'){
                sat.printf("rcmd=%c,cmdflag=%d\r\n",rcmd,cmdflag);
                com.printf("Hepta-Sat Lite Uplink Ok\r\n");
                float ax,ay,az;
                for(int i = 0; i < 10; i++) {
                    sensor.sen_acc(&ax,&ay,&az);
                    com.printf("%f,%f,%f\r\n",ax,ay,az);
                    wait_ms(1000); 
                }
            }else if(rcmd == 'd'){
                sat.printf("rcmd=%c,cmdflag=%d\r\n",rcmd,cmdflag);
                com.printf("Hepta-Sat Lite Uplink Ok\r\n");
                float gx,gy,gz;
                for(int i = 0; i < 10; i++) {
                    sensor.sen_gyro(&gx,&gy,&gz);
                    com.printf("%f,%f,%f\r\n",gx,gy,gz); 
                    wait_ms(1000);   
                }            
            }else if(rcmd == 'e'){
                
                
                
            }
            com.initialize(); //initializing
        }
    }
    sattime.stop();
    sat.printf("From Sat : End of operation\r\n");
    com.printf("From Sat : End of operation\r\n");
}