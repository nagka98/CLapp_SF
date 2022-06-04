#include <stdio.h>

#include<iostream>
#include<chrono>
#include<string>
#include<stdlib.h>
#include"SerialPort.h"

using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::system_clock;

//general attributes 
char res[10];
char imu[3];
char input[MAX_DATA_LENGTH];
char port[10];
bool wave_flag = false;
bool updown_flag = false;
int data[1000];
int highest_label = 0;

//Resistor proterties
int R_elim[10] = {0,1,0,1,0,1,1,0,0,0};
int R_treshold[10] = {0,180000,185000,0,0,185000,180000,0,0,0};
int R_min[10] = {0};
int R_max[10] = {0};
int R_avg[10] = {0};

//IMU properties
int imu_avg[3] = {0};
 
int main(int argc, char** argv) {

    int s = 0;
    int data_read = 0;
    long size_updated = 0;
    int size_updated_float=0;
    char recv_data[100];
    bool startup = false;
    bool calib_min = false;
    bool calib_max = false;
    bool wait = false;

    if(argc == 2)
    {
        s = atoi(argv[1]);
    }
    else
    {
        std::cout << "please add port number" << argc;
        //exit(0);
    }
    sprintf(port,"\\\\.\\COM%d",s);

    SerialPort esp(port);
        if (esp.isConnected()) {
        std::cout << "connection is established" << "\n";
        std::flush(std::cout);
    }
    else
    {
        std::cout << "\nerror\n";
        std::flush(std::cout);
        while(true){}
        //exit(0);
    }
    auto millisec_start = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    while (true)
    {
        while (esp.isConnected())
        {
            auto millisec_now = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
            data_read = esp.readSerialPort(input, MAX_DATA_LENGTH);
            if(data_read > 0)
            {  
            for (int i = 0; i < data_read; i++)
                {
                    if (input[i] == '\n')
                    {
                        input[i] = ',';
                    }
                    if (input[i] != ',')
                    {
                        recv_data[size_updated] = input[i];
                        size_updated++;
                    }
                    else
                    {
                        size_updated = 0;
                        data[size_updated_float] = strtof((char *)recv_data, NULL);
                        //std::cout << size_updated_float << "\n";
                        size_updated_float++;
                        if (size_updated_float == 1000)
                        {
                            std::cout << "float size exceeded" << "\n";
                            size_updated_float = 0;
                        }
                        memset((char *)recv_data, 0, sizeof(recv_data));
                    }
                }
            }
            if(!startup)
            {
                std::cout << "startup" << "\n";
                std::flush(std::cout);
                millisec_start = millisec_now + 10000;
                while(millisec_now - millisec_start < 0)
                {
                    millisec_now = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
                    esp.readSerialPort(input, MAX_DATA_LENGTH);
                }
                startup = true;
                size_updated_float = 0;
            }
            else if(!calib_min)
            {
                if(!wait)
                {
                    std::cout << "calib_min" << "\n";
                    std::flush(std::cout);
                    millisec_start = millisec_now + 10000;
                    while(millisec_now - millisec_start < 0)
                    {
                        millisec_now = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
                        esp.readSerialPort(input, MAX_DATA_LENGTH);
                    }
                    wait = true;
                    size_updated_float = 0;
                }
                if(millisec_now - millisec_start > 200)
                {
                    millisec_start = millisec_now;
                    int temp = 0;
                    for(int m = 0; m < 10; m++)
                    {
                        int count = 0;
                        temp = 0;
                        for(int n = m; n < size_updated_float; n=n+13)
                        {
                            //std::cout << "data :" << data[n] << "\n";
                            temp += data[n];
                            count++;
                        }
                        R_min[m] = (int)(temp/count);
                        //std::cout << R_min[m] <<"\n"; 
                    }
                    for(int m = 10; m < 13; m++)
                    {
                        int count = 0;
                        temp = 0;
                        for(int n = m; n < size_updated_float; n=n+13)
                        {
                            //std::cout << "data :" << data[n] << "\n";
                            temp += data[n];
                            count++;
                        }
                        imu_avg[m-10] = (int)(temp/count);
                        //std::cout << imu_avg[m-10] <<"\n"; 
                    }
                    calib_min = true;
                    wait = false;
                }
            }
            else if(!calib_max)
            {
                if(!wait)
                {
                    std::cout << "calib_max" << "\n";
                    std::flush(std::cout);
                    millisec_start = millisec_now + 10000;
                    while(millisec_now - millisec_start < 0)
                    {
                        millisec_now = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
                        esp.readSerialPort(input, MAX_DATA_LENGTH);
                    }
                    wait = true;
                    size_updated_float = 0;
                }
                if(millisec_now - millisec_start > 200)
                {
                    millisec_start = millisec_now;
                    int temp = 0;
                    for(int m = 0; m < 10; m++)
                    {
                        int count = 0;
                        temp = 0;
                        for(int n = m; n < size_updated_float; n=n+13)
                        {
                            temp += data[n];
                            count++;
                        }
                        R_max[m] = (int)temp/count;
                        R_avg[m] = (int)((R_min[m] + R_max[m])/2);
                        //std::cout << R_max[m] <<"\n"; 
                        //std::cout << R_avg[m] << "\n";
                        std::flush(std::cout);
                    }
                    calib_max = true;
                }                
            }
            else if(millisec_now - millisec_start > 200)
            {
                millisec_start = millisec_now;
                int j=0;
                char temp[10];
                bool pass_bit = true;
                while(j < size_updated_float)
                    {
                        char *output = res;
                        char *output_imu = imu;
                        for(int k=0; k<10; k++)
                        {
                            if(!R_elim[k])
                            {
                                goto loop_end;
                            }
                            //std::cout << "data :" << data[j+k] << "\n"; 
                            //std::cout << "k : " << j+k << "\n";
                            if(data[j+k] <= R_avg[k])
                            {
                                output += sprintf(output,"0");
                            }
                            else
                            {
                                output += sprintf(output,"1");
                            }
                            loop_end:
                                asm("NOP");
                        }
                        for(int k=10; k<13; k++)
                        {
                            //std::cout << "IMU:" << abs(data[j+k] - abs(imu_avg[k-10])) << "\n";
                            if(abs(data[j+k] - abs(imu_avg[k-10])) >= 50000)
                            {
                                output_imu += sprintf(output_imu,"0");
                            }
                            else
                            {
                                output_imu += sprintf(output_imu,"1");
                            }
                        //printf("yaw: %d, pitch: %d, roll: %d \n",data[j+12]-imu_avg[2],data[j+11]-imu_avg[1],data[j+10]-imu_avg[0]);
                        } 
                        if(!strcmp(temp,res) && pass_bit)
                        {
                            pass_bit = true;
                        }
                        else
                        {
                            pass_bit = false;
                        }

                        sprintf(temp,"%s",res);
                        j=j+13;
                    }   
                //std::cout << "passbit : " << pass_bit << "\n";
                if(!(strcmp("010",imu) && strcmp("110",imu)))
                {
                    updown_flag = true;
                }
                else if(!(strcmp("001",imu) && strcmp("101",imu)))
                {
                    wave_flag = true;
                }
                else if(!strcmp("111",imu))
                {
                    if(updown_flag)
                    {
                        std::cout << "updown" << "\n";
                        updown_flag = false;
                    }
                    else if(wave_flag)
                    { 
                        std::cout << "wave" << "\n";
                        wave_flag = false;
                    }
                }
                if(pass_bit)
                {
                    std::cout << res << "\n";
                    std::flush(std::cout);
                }
                size_updated_float = 0;
                memset(data, 0, sizeof(data));
            }
        }
    }
    return 0;
}