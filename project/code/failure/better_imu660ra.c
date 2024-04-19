/*
 * better_imu660ra.c
 *
 *  Created on: 2024年3月18日
 *      Author: pc
 */
#include "zf_common_headfile.h"

float GX_Zero = -11.5;//零点偏移值
float Gy_Zerp = 6;
float GZ_Zerp = -1;
float imu660ra_g_x=imu660ra_gyro_x-GX_Zero,imu660ra_g_y=imu660ra_gyro_y-Gy_Zerp,imu660ra_g_z=imu660ra_gyro_z-GZ_Zerp;//去除零点偏移后的陀螺仪采集值
float Angle_yaw=0,Angle_pitch=0;
//--------------------------------------------------------------------------------------------------------------------------
// 函数简介 过滤参数，返回10次计算算术平均值
//--------------------------------------------------------------------------------------------------------------------------
float filtration(float x)
{
    float temp;
    for(uint i=0;i<10;i++)
    {
        temp+=x;
    }
    temp=temp/10;
    return temp;
}
//--------------------------------------------------------------------------------------------------------------------------
// 函数简介 互补滤波获取角度
// 备注信息 放在定时器中每2ms执行一次
//--------------------------------------------------------------------------------------------------------------------------
void imu660ra_AngleGet(void)
{

    Angle_yaw+=filtration(imu660ra_g_z)*0.00012480f;
    static int angle_start_flag=0;//加速度标志
    float angle_ratio =0;//加速度比值
    static float Angle_acc_last=0,Angle_acc,Angle;
    /*加速度角度处理*/
    Angle_acc_last=Angle_acc;
    angle_ratio=(float)(filtration(imu660ra_acc_x)/(filtration(imu660ra_acc_z)+0.1));
    Angle_acc=(float)(atan(angle_ratio)*57.29578049);
    if(angle_start_flag==0)
    {
        Angle_acc_last=Angle_acc;
    }
    Angle_acc=0.5*Angle_acc+0.5*Angle_acc_last;
    if (Angle_acc > 89)
        Angle_acc = 89;
    if (Angle_acc < -89)
        Angle_acc = -89;
    Angle=(float)(Angle-(float)(filtration(imu660ra_gyro_y)*0.0001249));
    Angle=Angle+(Angle_acc-Angle)*0.001;
    if(angle_start_flag<300)
    {
        Angle = Angle_acc;
        angle_start_flag++;
    }
    else {
        Angle_pitch=Angle;
    }
}
