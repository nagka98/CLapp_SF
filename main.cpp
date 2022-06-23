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
int data[500];
int highest_label = 0;

//tcp attributes
int port_num = 5000;

//Resistor proterties
int R_elim[10] = {0,0,0,1,0,1,0,1,1,0};
int R_min[10] = {0};
int R_max[10] = {0};
int R_avg[10] = {0};
int Z_min[10] = {0};
int Z_max[10] = {0};
int Z_avg[10] = {0};
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
    int resistance_flag = 0;
    int impedance_flag =  0;

    if(argc == 4)
    {
        s = atoi(argv[1]);
        resistance_flag = atoi(argv[2]);
        impedance_flag = atoi(argv[3]);
    }
    else
    {
        std::cout << "please add port number : " << argc << "\n";
        //exit(0);
    }
    sprintf(port,"\\\\.\\COM%d",s);

    SerialPort esp(port);
        if (esp.isConnected()) {
        std::cout << "connection_ok\n" << "\n";
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
                // std::cout << input;
                // std::flush(std::cout);
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
                        if (size_updated_float == 500)
                        {
                            std::cout << "float size exceeded" << "\n";
                            size_updated_float = 0;
                        }
                        memset(recv_data, 0, sizeof(recv_data));
                    }
                }
            }
            if(!startup)
            {
                std::cout << "startup" << "\n";
                std::flush(std::cout);
                millisec_start = millisec_now + 8000;
                while(millisec_now - millisec_start < 0)
                {
                    millisec_now = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
                    printf("%d\n",(int)((millisec_start-millisec_now)/1000));//uncomment
                    std::flush(std::cout);
                    esp.readSerialPort(input, MAX_DATA_LENGTH);
                }
                startup = true;
                size_updated_float = 0;
                size_updated = 0;
            }
            else if(!calib_min)
            {
                if(!wait)
                {
                    std::cout << "calib_min" << "\n";
                    std::flush(std::cout);
                    millisec_start = millisec_now + 8000;
                    while(millisec_now - millisec_start < 0)
                    {
                        millisec_now = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
                        printf("%d\n",(int)((millisec_start-millisec_now)/1000));//uncomment
                        std::flush(std::cout);
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
                        for(int n = m; n < size_updated_float; n=n+29)
                        {
                            //std::cout << "data :" << data[n] << "\n";
                            temp += data[n];
                            count++;
                        }
                        R_min[m] = (int)(temp/count);
                        //std::cout << "R_min :\n"; 
                        //std::cout << R_min[m] <<"\n"; 
                    }
                    for(int m = 10; m < 20; m++)
                    {
                        int count = 0;
                        temp = 0;
                        for(int n = m; n < size_updated_float; n=n+29)
                        {
                            //std::cout << "data :" << data[n] << "\n";
                            temp += data[n];
                            count++;
                        }
                        Z_min[m-10] = (int)(temp/count);
                        //std::cout << Z_min[m] <<"\n"; 
                    }
                    for(int m = 20; m < 23; m++)
                    {
                        int count = 0;
                        temp = 0;
                        for(int n = m; n < size_updated_float; n=n+29)
                        {
                            //std::cout << "data :" << data[n] << "\n";
                            temp += data[n];
                            count++;
                        }
                        imu_avg[m-20] = (int)(temp/count);
                        //std::cout << imu_avg[m-20] <<"\n"; 
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
                    millisec_start = millisec_now + 8000;
                    while(millisec_now - millisec_start < 0)
                    {
                        millisec_now = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
                        printf("%d\n",(int)((millisec_start-millisec_now)/1000));//uncomment
                        std::flush(std::cout);                        
                        esp.readSerialPort(input, MAX_DATA_LENGTH);
                    }
                    wait = true;
                    size_updated_float = 0;
                    size_updated = 0;
                }
                if(millisec_now - millisec_start > 200)
                {
                    millisec_start = millisec_now;
                    int temp = 0;
                    for(int m = 0; m < 10; m++)
                    {
                        int count = 0;
                        temp = 0;
                        for(int n = m; n < size_updated_float; n=n+29)
                        {
                            temp += data[n];
                            count++;
                        }
                        R_max[m] = (int)temp/count;
                        R_avg[m] = (int)((R_min[m] + R_max[m])/2);
                        //std::cout << R_max[m] <<"\n"; 
                        //std::cout << R_avg[m] << "\n";//comment
                        std::flush(std::cout);
                    }
                    for(int m = 10; m < 20; m++)
                    {
                        int count = 0;
                        temp = 0;
                        for(int n = m; n < size_updated_float; n=n+29)
                        {
                            temp += data[n];
                            count++;
                        }
                        Z_max[m-10] = (int)temp/count;
                        Z_avg[m-10] = (int)((Z_min[m-10] + Z_max[m-10])/2);
                        //std::cout << Z_max[m] <<"\n"; 
                        //std::cout << Z_avg[m-10] << "\n";//comment
                        std::flush(std::cout);
                    }
                    calib_max = true;
                    std::cout << "calib_done" << "\n";
                    std::flush(std::cout);
                }                
            }
            else if(millisec_now - millisec_start > 100)
            {
                //std::cout << "size " << size_updated_float << "\n";
                millisec_start = millisec_now;
                int j=0;
                char temp[10];
                bool pass_bit = true;
                if(size_updated_float%29 == 0)
                {
                while(j < size_updated_float)
                    {
                        char *output = res;
                        char *output_imu = imu;
                        if(resistance_flag == 1)
                        {
                        for(int k=0; k<10; k++)
                        {
                            if(!R_elim[k])
                            {
                                goto loop_end1;
                            }
                            //std::cout << "data :" << data[j+k] << "\n"; 
                            //std::cout << "k : " << j+k << "\n";
                            if(data[j+k] != 0)
                            {
                            if(data[j+k] <= R_avg[k])
                            {
                                output += sprintf(output,"0");
                            }
                            else
                            {
                                output += sprintf(output,"1");
                            }
                            }
                            loop_end1:
                                asm("NOP");
                        }
                        }
                        if(impedance_flag == 1)
                        {
                        for(int k=10; k<20; k++)
                        {
                            if(!R_elim[k-10])
                            {
                                goto loop_end2;
                            }
                            //std::cout << "data :" << data[j+k] << "\n"; 
                            //std::cout << "k : " << j+k << "\n";
                            if(data[j+k] != 0)
                            {
                            if(data[j+k] <= Z_avg[k-10])
                            {
                                output += sprintf(output,"0");
                            }
                            else
                            {
                                output += sprintf(output,"1");
                            }
                            }
                            loop_end2:
                                asm("NOP");
                        }
                        }
                        for(int k=20; k<23; k++)
                        {
                            //std::cout << "IMU:" << abs(data[j+k] - abs(imu_avg[k-10])) << "\n";
                            // if(abs(data[j+k] - abs(imu_avg[k-10])) >= 30000)
                            if(data[j+k] != 0)
                            {
                            if((data[j+k] - imu_avg[k-20]) > 50000)
                            {
                                output_imu += sprintf(output_imu,"1");
                            }
                            else if((data[j+k] - imu_avg[k-20]) < -50000)
                            {
                                output_imu += sprintf(output_imu,"2");
                            }
                            else
                            {
                                output_imu += sprintf(output_imu,"0");
                            }
                            }
                        // printf("yaw: %d, pitch: %d, roll: %d \n",data[j+22]-imu_avg[2],data[j+21]-imu_avg[1],data[j+20]-imu_avg[0]);
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
                        j=j+29;
                    }
                                    //std::cout << "passbit : " << pass_bit << "\n";
                if(!(strcmp("020",imu) && strcmp("220",imu)))
                {
                    wave_flag = true;
                }
                else if(!(strcmp("001",imu) && strcmp("201",imu)))
                {
                    updown_flag = true;
                }
                else if(!strcmp("000",imu))
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
                }
                else{
                    pass_bit = false;
                } 
                if(pass_bit)
                {
                    //std::cout << size_updated_float << "\n";
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