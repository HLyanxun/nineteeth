/*
 * better_mpu6050.c
 *
 *  Created on: 2024��3��19��
 *      Author: pc
 */
#include "zf_common_headfile.h"
#include "math.h"
float Angle_pitch,Angle_roll,Angle_yaw;
//------------------------------------------------------------------------------
// �������     ������ת��Ϊ�Ƕ�
//------------------------------------------------------------------------------
float radiansToDegrees(float radians)
{
    return radians * (180.0 / M_PI);
}

//-------------------------------------------------------------------------------
// �������     ��Ӳ��mpu6050���ص����ݽ��д������ŷ����
// ��ע��Ϣ     ���ڶ�ʱ���У�2msִ��1��
//-------------------------------------------------------------------------------
void mpu6050_data_dispose(void)
{
    mpu6050_get_acc();
    mpu6050_get_gyro();
    float temp_roll,temp_pitch;

    // ������ٶȼƵ���б��
    temp_roll = atan2((float)mpu6050_acc_y/16.4, (float)mpu6050_acc_z/16.4);
    temp_pitch = atan(-(mpu6050_acc_x/16.4) / (sqrt((float)mpu6050_acc_y/16.4 * (float)mpu6050_acc_y/16.4 + (float)mpu6050_acc_z/16.4 * (float)mpu6050_acc_z/16.4)));

    // ������ٶȼƵĽ��ٶ�
    float dt = 0.0002; // ����������Ϊ0.01��
    float gyro_roll = mpu6050_gyro_x * dt / 16.4;
    float gyro_pitch = mpu6050_gyro_y * dt / 16.4;
    float gyro_yaw = mpu6050_gyro_z * dt / 16.4;

    // ��ϼ��ٶȼƺͽ��ٶȼƵ����ݼ���ŷ����
    Angle_roll = (0.98 * (temp_roll + gyro_roll) + 0.02 * (temp_roll))*(180.0 / M_PI); // ʹ�û����˲�
    Angle_pitch = (0.98 * (temp_pitch + gyro_pitch) + 0.02 * (temp_pitch))*(180.0 / M_PI);
    Angle_yaw += (gyro_yaw/16.4)*(180.0 / M_PI)*2;

}
//---------------------------------------------------------------------------------
// �������     ��tft��Ļ����ʾŷ����
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
