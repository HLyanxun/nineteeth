/*
 * better_mpu6050.c
 *
 *  Created on: 2024年3月19日
 *      Author: pc
 */
#include "zf_common_headfile.h"
#include "math.h"
float Angle_pitch,Angle_roll,Angle_yaw;
//------------------------------------------------------------------------------
// 函数简介     将弧度转化为角度
//------------------------------------------------------------------------------
float radiansToDegrees(float radians)
{
    return radians * (180.0 / M_PI);
}

//-------------------------------------------------------------------------------
// 函数简介     对硬件mpu6050返回的数据进行处理，输出欧拉角
// 备注信息     置于定时器中，2ms执行1次
//-------------------------------------------------------------------------------
void mpu6050_data_dispose(void)
{
    mpu6050_get_acc();
    mpu6050_get_gyro();
    float temp_roll,temp_pitch;

    // 计算加速度计的倾斜角
    temp_roll = atan2((float)mpu6050_acc_y/16.4, (float)mpu6050_acc_z/16.4);
    temp_pitch = atan(-(mpu6050_acc_x/16.4) / (sqrt((float)mpu6050_acc_y/16.4 * (float)mpu6050_acc_y/16.4 + (float)mpu6050_acc_z/16.4 * (float)mpu6050_acc_z/16.4)));

    // 计算角速度计的角速度
    float dt = 0.0002; // 假设采样间隔为0.01秒
    float gyro_roll = mpu6050_gyro_x * dt / 16.4;
    float gyro_pitch = mpu6050_gyro_y * dt / 16.4;
    float gyro_yaw = mpu6050_gyro_z * dt / 16.4;

    // 结合加速度计和角速度计的数据计算欧拉角
    Angle_roll = (0.98 * (temp_roll + gyro_roll) + 0.02 * (temp_roll))*(180.0 / M_PI); // 使用互补滤波
    Angle_pitch = (0.98 * (temp_pitch + gyro_pitch) + 0.02 * (temp_pitch))*(180.0 / M_PI);
    Angle_yaw += (gyro_yaw/16.4)*(180.0 / M_PI)*2;

}
//---------------------------------------------------------------------------------
// 函数简介     在tft屏幕上显示欧拉角
//---------------------------------------------------------------------------------
void mpu6050_parameter_tft180_show(void)
{
    tft180_show_float(0, 0, Angle_pitch, 3, 1);
    tft180_show_float(0, 16, Angle_yaw, 3, 1);
    tft180_show_float(0, 32, Angle_roll, 3, 1);
    tft180_show_string(80, 0, "pitch");
    tft180_show_string(80, 16, "yaw");
    tft180_show_string(80, 32, "roll");
}
