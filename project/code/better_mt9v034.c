/*
 * better_mt9v034.c
 *
 *  Created on: 2024年1月20日
 *      Author: pc
 */
#include "zf_common_headfile.h"
#include "math.h"
uint8 ex_mt9v03x_image_binaryzation[MT9V03X_H][MT9V03X_W];
uint8 ex_mt9v03x_image_binaryzation_flag;
int ex_binaryzation_conduct_adjustment=0;
uint16 ex_midline[MT9V03X_H];
uint16 ex_leftline[MT9V03X_H];
uint16 ex_rightline[MT9V03X_H];
uint8 ex_roundabout_type;   //1左环岛2为右环岛
uint8 ex_roundabout_state;
uint8 ex_zebra_crossing_flag;
uint8 ex_zebra_pass_count;
uint8 ex_crossroad_flag;
uint8 ex_crossroad_state;
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     通过三角函数计算角度
//-------------------------------------------------------------------------------------------------------------------
float arctan(float x,float y)
{
    float temp;
    temp=atan(x/y);
    temp=(180/pi)*temp;
    return temp;
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介      通过大津法返回阈值进行摄像头图像二值化处理，可以调节阈值
// 备注信息      用大津法计算出阈值（最好让大津法减少执行次数，如每10帧执行一次刷新）对灰度图像固定阈值的二值化处理,低于阈值的置0，高于阈值的置255
//-------------------------------------------------------------------------------------------------------------------
void mt9v034_binaryzation_conduct(void)
{

    uint8 temp=0;
    int8 value=0;
    value=binaryzation_value();                                         //获取大津法计算出的阈值
    for(uint8 i=0; i<MT9V03X_H; i++)
    {
        for(uint8 j=0; j<MT9V03X_W; j++)
        {
            temp=mt9v03x_image[i][j];
            if(temp<=(value+ex_binaryzation_conduct_adjustment))                    //判定像素是否低于阈值，若为是，则将其置0（黑色），若为否，则将其置255（白色）
            {
                ex_mt9v03x_image_binaryzation[i][j]=0;
            }
            else
            {
                ex_mt9v03x_image_binaryzation[i][j]=255;
            }
        }
    }
    ex_mt9v03x_image_binaryzation_flag=1;                               //完成一次图像二值化处理后，将标志位置1
}

//--------------------------------------------------------------------------------------------------------------------
// 函数简介     将处理后的二值化图像输出到tft屏幕上
// 备注信息     每完成一帧的图像处理，进行一次图像刷新，然后将标志位置0
//--------------------------------------------------------------------------------------------------------------------
void mt9v034_tft180_show(void)
{
    if(ex_mt9v03x_image_binaryzation_flag==1)
            {
                //tft180_show_gray_image(0, 0, ex_mt9v03x_image_binaryzation[0], MT9V03X_W, MT9V03X_H,MT9V03X_W/2, MT9V03X_H/2, 0);
                tft180_displayimage03x(ex_mt9v03x_image_binaryzation[0],MT9V03X_W/2, MT9V03X_H/2);       //tft屏幕显示二值化后的屏幕
                ex_mt9v03x_image_binaryzation_flag=0;            //将二值化完成标志位置0
            }
}
//--------------------------------------------------------------------------------------------------------------------
// 函数简介     利用索贝尔算子进行图像的二值化处理(存在问题，暂时无法使用)
//--------------------------------------------------------------------------------------------------------------------
void sobe_binaryzation_conduct(void)
{
    int mt9v03x_output_image[MT9V03X_H][MT9V03X_W];
    sobelAutoThreshold(mt9v03x_image[0],mt9v03x_output_image[0]);
    tft180_show_binary_image(0,0,mt9v03x_output_image[0],MT9V03X_W,MT9V03X_H,(MT9V03X_W/2),(MT9V03X_H/2));
}
//--------------------------------------------------------------------------------------------------------------------
// 函数简介     直接输出大津法作为阈值的二值化图像,通过调节ex_binaryzation_conduct_adjustment调节二值化阈值
// 备注信息     不将数据录入储存数组，仅显示二值化图像，仅用于调试和对比
//--------------------------------------------------------------------------------------------------------------------
void mt9v034_tft180_dajin_show(void)
{
    tft180_show_gray_image(0, 0, mt9v03x_image[0],MT9V03X_W, MT9V03X_H, MT9V03X_W/2, MT9V03X_H/2,(binaryzation_value()+ex_binaryzation_conduct_adjustment));
}

//----------------------------------------------------------------------------------------------------------------------
// 函数简介     通过大津法计算二值化阈值
// 返回值         二值化阈值
// 备注信息     通过大津法初步优化的二值化阈值算法，或许仍有改进空间
//----------------------------------------------------------------------------------------------------------------------

uint8 binaryzation_value(void)
{

    int GrayScale=256;//定义256个灰度级
    int pixelCount[GrayScale];//每个灰度值所占像素个数
    float pixelPro[GrayScale];//每个灰度值所占总像素比例
    int i,j,pixelSum=(MT9V03X_H*MT9V03X_W/4);//总像素点
    static int8 threshold=0;//最佳阈值
    int gray_sum=0;//像素点灰度总和
    memset(pixelCount, 0, GrayScale); //使用memset方法初始化数组
    memset(pixelPro, 0, GrayScale); //使用memset方法初始化数组
    static uint8 circulation_flag;
    if(circulation_flag>=3)
    {
    for(i=0;i<MT9V03X_H;i+=2)
    {
        for(j=0;j<MT9V03X_W;j+=2)
        {
            pixelCount[mt9v03x_image[i][j]]++;      //求灰度分布
            gray_sum+=mt9v03x_image[i][j];          //所有像素点的灰度总和
        }
    }

        float w0,w1,u0Sum,u1Sum,u0,u1,u,variance=0, maxVariance = 0;
        w0=w1=u0Sum=u1Sum=u0=u1=u=0;
        //目标像素占总图像比例w0
        //背景图像占总图像比例w1
        //目标平均灰度数值u0
        //背景平均灰度数值u1
        //总平均灰度值u
        //类间方差variance
        //最大类间方差maxVariance
        //计算每个像素在整副图像中的比例
        for(i=0;i<GrayScale;i++)
                {
                    pixelPro[i]=(float)pixelCount[i]/pixelSum;      //灰度分布除以总像素数量
                    u=i*pixelPro[i];//总平均灰度
                }
        for (i = 0; i < GrayScale; i++)     // i作为阈值 阈值从1-255遍历
        {

                   for (j = 0; j < GrayScale; j++)    //求目标前景和背景
                   {
                       if (j <= i)   //前景部分
                       {
                           w0 += pixelPro[j];
                           u0Sum += j * pixelPro[j];
                       }
                       else   //背景部分
                       {
                           w1 += pixelPro[j];
                           u1Sum += j * pixelPro[j];
                       }
                   }
                   u0 = u0Sum / w0;
                   u1 = u1Sum / w1;
                   variance = w0 * pow((u0 - u), 2) + w1 * pow((u1 - u), 2);//类间方差计算公式
            if(variance>maxVariance)
            {
                maxVariance=variance;
                threshold=(int)i;
            }
            if(variance<maxVariance)
            {

                break;
            }

        }
    }
        return threshold;
}

//---------------------------------------------------------------------------------------------------------------
// 函数简介                 通过索贝尔算子进行二值化图像处理
// 参数说明                 source          源图像（摄像头直接采集的图像）
// 参数说明                 target          输出边缘二值化图像
// 使用实例（推测）  sobeAutoThreshold(mt9v30x_image[0],mt9v30x_output_image[0]);
// 备注信息                 基于索贝尔算子的边缘处理算法，占用算力一般，相对于大津法的计算，在不均匀光照中有着更好的边缘识别表现，原理基于图像卷积运算。
//---------------------------------------------------------------------------------------------------------------
void sobelAutoThreshold (u8 source[MT9V03X_H/2][MT9V03X_W],u8 target[MT9V03X_H/2][MT9V03X_W])
{
    /** 卷积核大小 */
    short KERNEL_SIZE = 3;
    short xStart = KERNEL_SIZE / 2;
    short xEnd = MT9V03X_W - KERNEL_SIZE / 2;
    short yStart = KERNEL_SIZE / 2;
    short yEnd = MT9V03X_H - KERNEL_SIZE / 2;
    short i, j, k;
    short temp[3];
    for (i = yStart; i < yEnd; i++)
    {
        for (j = xStart; j < xEnd; j++)
        {
            /* 计算不同方向梯度幅值  */
            temp[0] = -(short) source[i - 1][j - 1] + (short) source[i - 1][j + 1]      // {-1, 0, 1},
            - (short) source[i][j - 1] + (short) source[i][j + 1]                     // {-1, 0, 1},
            - (short) source[i + 1][j - 1] + (short) source[i + 1][j + 1];             // {-1, 0, 1};

            temp[1] = -(short) source[i - 1][j - 1] + (short) source[i + 1][j - 1]      // {-1, -1, -1},
            - (short) source[i - 1][j] + (short) source[i + 1][j]                      // { 0,  0,  0},
            - (short) source[i - 1][j + 1] + (short) source[i + 1][j + 1];             // { 1,  1,  1};

            temp[2] = -(short) source[i - 1][j] + (short) source[i][j - 1]              //  {0, -1, -1},
            - (short) source[i][j + 1] + (short) source[i + 1][j]                       //  {1,  0, -1},
            - (short) source[i - 1][j + 1] + (short) source[i + 1][j - 1];              //  {1,  1,  0};

            temp[3] = -(short) source[i - 1][j] + (short) source[i][j + 1]               // {-1, -1,  0},
            - (short) source[i][j - 1] + (short) source[i + 1][j]                       // {-1,  0,  1},
            - (short) source[i - 1][j - 1] + (short) source[i + 1][j + 1];              //  {0,  1,  1};

            temp[0] = abs(temp[0]);
            temp[1] = abs(temp[1]);
            temp[2] = abs(temp[2]);
            temp[3] = abs(temp[3]);

            /* 找出梯度幅值最大值  */
            for (k = 1; k < 3; k++)
            {
                if (temp[0] < temp[k])
                {
                    temp[0] = temp[k];
                }
            }

            /* 使用像素点邻域内像素点之和的一定比例    作为阈值  */
            temp[3] =
                    (short) source[i - 1][j - 1] + (short) source[i - 1][j] + (short) source[i - 1][j + 1]
                    + (short) source[i][j - 1] + (short) source[i][j] + (short) source[i][j + 1]
                    + (short) source[i + 1][j - 1] + (short) source[i + 1][j] + (short) source[i + 1][j + 1];

            if (temp[0] > (temp[3] / 12.0f))
            {
                target[i][j] = 0xFF;
            }
            else
            {
                target[i][j] = 0x00;
            }
        }
    }
}
//-------------------------------------------------------------------------------------------------------------------------------
// 函数简介     计算多项式回归系数
// 参数说明    d_X  输入的数据的x值
// 参数说明     d_Y   输入的数据的y值
// 参数说明     d_N   输入的数据的个数
// 参数说明     rank  多项式的次数
// 参数说明     coeff 输出的系数
// 备注信息    double占用8字节，float占用4字节，可以考虑优化
//-------------------------------------------------------------------------------------------------------------------------------
void polyfit(double *d_X, double *d_Y, int d_N, int rank, double *coeff)
{
//    int rank;
//    rank=RANK_;
       if(RANK_!=rank)          //判断次数是否合法
           return;

    int i,j,k;
    double aT_A[RANK_ + 1][RANK_ + 1] = {0};
    double aT_Y[RANK_ + 1] = {0};


    for(i = 0; i < rank + 1; i++)   //行
    {
        for(j = 0; j < rank + 1; j++)   //列
        {
            for(k = 0; k < d_N; k++)
            {
                aT_A[i][j] +=  pow(d_X[k], i+j);        //At * A 线性矩阵
            }
        }
    }

    for(i = 0; i < rank + 1; i++)
    {
        for(k = 0; k < d_N; k++)
        {
            aT_Y[i] += pow(d_X[k], i) * d_Y[k];     //At * Y 线性矩阵
        }
    }

    //以下为高斯列主元素消去法解线性方程组
    for(k = 0; k < rank + 1 - 1; k++)
    {
        int row = k;
        double mainElement = fabs(aT_A[k][k]);
        double temp = 0.0;

        //找主元素
        for(i = k + 1; i < rank + 1 - 1; i++)
        {
            if( fabs(aT_A[i][i]) > mainElement )
            {
                mainElement = fabs(aT_A[i][i]);
                row = i;
            }
        }

        //交换两行
        if(row != k)
        {
            for(i = 0; i < rank + 1; i++)
            {
                temp = aT_A[k][i];
                aT_A[k][i] = aT_A[row][i];
                aT_A[row][i] = temp;
            }
            temp = aT_Y[k];
            aT_Y[k] = aT_Y[row];
            aT_Y[row] = temp;
        }


        //消元过程
        for(i = k + 1; i < rank + 1; i++)
        {
            for(j = k + 1; j < rank + 1; j++)
            {
                aT_A[i][j] -= aT_A[k][j] * aT_A[i][k] / aT_A[k][k];
            }
            aT_Y[i] -= aT_Y[k] * aT_A[i][k] / aT_A[k][k];
        }
    }

    //回代过程
    for(i = rank + 1 - 1; i >= 0; coeff[i] /= aT_A[i][i], i--)
    {
        for(j = i + 1, coeff[i] = aT_Y[i]; j < rank + 1; j++)
        {
            coeff[i] -= aT_A[i][j] * coeff[j];
        }
    }

    return;
}

//--------------------------------------------------------------------------------------------------------------------------
// 函数简介     横向巡线，将边线信息记录在ex_leftline,ex_midline,ex_rightline数组中
// 使用实例     Horizontal_line()
// 备注信息     或许存在问题，建议多试一试调整Row和Col的数值
//--------------------------------------------------------------------------------------------------------------------------
void horizontal_line(void)
{
  uint8 i,j;
  if(ex_mt9v03x_image_binaryzation[Row-1][Col/2]==0)                        //判断摄像机中线是否为黑色，即初始是否在赛道上
  {
    if(ex_mt9v03x_image_binaryzation[Row-1][5]==255)                      //判断图像的最低一行，第五列是否为白色（图像左下角）
      ex_midline[Row]=5;                               //将预定数字定为5
    else if(ex_mt9v03x_image_binaryzation[Row-1][Col-5]==255)
      ex_midline[Row]=Col-5;
    else
      ex_midline[Row]=Col/2;
  }
  else
    {
    ex_midline[Row]=Col/2;                             //如果是，那么将中线的行数记录为列数的二分之一（摄像机中线）
    }

  for(i=Row-1;i>0;i--)
  {
    for(j=ex_midline[i+1];j>=0;j--)
    {
      if(ex_mt9v03x_image_binaryzation[i][j]==0||j==0)
      {
        ex_leftline[i]=j;                              //左边线设定为i，且储存纵轴j；
        break;
      }
    }
    for(j=ex_midline[i+1];j<=Col-1;j++)
    {
      if(ex_mt9v03x_image_binaryzation[i][j]==0||j==Col-1)
      {
        ex_rightline[i]=j;
        break;
      }
    }
    ex_midline[i]=(ex_leftline[i]+ex_rightline[i])/2;   //以左右边线的中点作为赛道中线
   if(ex_mt9v03x_image_binaryzation[i-1][ex_midline[i]]==0||i==0)
    {
   for(j=i;j>0;j--)
   {
    ex_midline[j]=ex_midline[i];
    ex_leftline[j]=ex_midline[i];
    ex_rightline[j]=ex_midline[i];
   }
      break;
    }
  }
}
//--------------------------------------------------------------------------------------------------------------------------
// 函数简介     右侧图像丢线检查
// 返回值       -1为未丢线 其他为丢线起始行
// 使用实例     Lost_line_right();
//--------------------------------------------------------------------------------------------------------------------------
int8 lost_line_right(void)
{
  uint8 i;
  for(i=find_end;i>find_start;i--)
  if(ex_rightline[i]==(MT9V03X_W-1))     return i;
  return -1;
}
//--------------------------------------------------------------------------------------------------------------------------
// 函数简介     左侧图像丢线检查
// 返回值       -1为未丢线 其他为丢线起始行
// 使用实例     Lost_line_left();
//--------------------------------------------------------------------------------------------------------------------------
int8 lost_line_left(void)
{
    uint8 i;
  for(i=find_end;i>find_start;i--)
  if(ex_leftline[i]==0) return i;
  return -1;
}
//--------------------------------------------------------------------------------------------------------------------------
// 函数简介     拐点检测
// 参数说明 select      选择输出的拐点1左下，2左上，3右下，4右上
// 参数说明 location    选择输出x或者y坐标,1为x，2为y
// 返回值          -1为没有检测到拐点，-2检测到拐点，但是已经丢线，其他返回拐点坐标
//--------------------------------------------------------------------------------------------------------------------------
int16 change_detection(uint8 select,uint8 location)
{
    uint8 i;
    if(select==1)
    {
        for(i=find_end;i>find_start;i--)
            {
            if(ex_leftline[i]==0)
                {

                    if(location==2)
                    {
                        return (i+1);
                    }

                    if(location==1)
                    {
                        if(ex_leftline[i+1]!=0)return ex_leftline[i+1];
                        return -2;
                    }
                }

            }
    }
    if(select==2)
    {
        for(i=find_start;i<find_end;i++)
           {
               if(ex_leftline[i]==0)
               {
                   if(location==2)
                   {
                       return (i-1);
                   }

                   if(location==1)
                   {
                       if(ex_leftline[i-1]!=0)return ex_leftline[i-1];
                       return -2;
                   }
               }
           }
    }
   if(select==3)
   {
       for(i=find_end;i>find_start;i--)
                   {
                   if(ex_rightline[i]==0)
                       {

                           if(location==2)
                           {
                               return (i+1);
                           }

                           if(location==1)
                           {
                               if(ex_rightline[i+1]!=0)return ex_rightline[i+1];
                               return -2;
                           }
                       }

                   }
   }

   if(select==4)
   {
        for(i=find_start;i<find_end;i++)
                   {
                       if(ex_leftline[i]==0)
                       {
                           if(location==2)
                           {
                               return (i-1);
                           }

                           if(location==1)
                           {
                               if(ex_leftline[i-1]!=0)return ex_leftline[i-1];
                               return -2;
                           }
                       }
                   }
}

    return -1;
}
//-------------------------------------------------------------------------------------------------------------------------
// 函数简介     对弯道进行判定（或许有点鸡肋，大概用不上）
// 返回值         -1不是弯道，1为左转，2为右转
//-------------------------------------------------------------------------------------------------------------------------
int8 curve_detection(void)
{
    int8 left=0,right=0;
    left=lost_line_left();
    right=lost_line_right();
    if(left!=-1 && right==-1 && straight_line_judgment!=1)return 1;
    if(left==-1 && right!=-1 && straight_line_judgment!=1)return 2;
    return -1;
}

//--------------------------------------------------------------------------------------------------------------------------
// 函数简介     十字路口检测
// 返回值          如果当前处于十字路口，则返回1，否则返回-1。
// 使用实例     crossroad_detection();
//--------------------------------------------------------------------------------------------------------------------------
int8 crossroad_detection(void)
{
    int8 left_down=0,right_down=0,left_up=0;
    uint8 i,j,crossroad_flag=0,crossroad=0;
    uint temp=0;
    left_down=change_detection(1,2);
    right_down=change_detection(3,2);
    if(left_down!=-1 && right_down!=-1 && ex_crossroad_flag!=1)           //判断两边都丢线
    {
        left_up=change_detection(2, 2);
        for(i=(left_down-5);i>(left_up+5);i--)    //检测中心是否存在大量白点
        {
            for(j=72;j>24;j--)
            {
                 if(ex_mt9v03x_image_binaryzation[i][j]==255)temp++;
                 if(temp>200)temp=100;
            }
        }
        if(temp<50) ex_crossroad_flag=1;            //如果存在大量白点,则标记为进入十字路口
    }
    if(ex_crossroad_flag==1){crossroad=1;}else{crossroad=-1;}
    return crossroad;
}
//--------------------------------------------------------------------------------------------------------------------------
// 函数简介     十字路口处理
// 使用实例     crossroad_conduct();
//--------------------------------------------------------------------------------------------------------------------------

void crossroad_conduct(void)
{
    if(ex_crossroad_flag=1)//如果已经进入十字路口
    {
        switch(ex_crossroad_state)//十字路口状态判断
        {
        case 1:
            if(lost_line_left()==-1 && lost_line_right==-1)ex_crossroad_state=2;//进入十字路口弯道
            break;
        case 2:
            if(lost_line_left()!=-1 && lost_line_right!=-1)ex_crossroad_state=3;//出十字路口
            break;
        case 3:
            if(lost_line_left()==-1 && lost_line_right==-1){ex_crossroad_state=0;ex_crossroad_flag=0;}//清除标志位
            break;
        }
        if(ex_crossroad_state==1)//对十字路口补线
        {
            uint8 x1,y1,x2,y2;
            x1=change_detection(1,1);
            y1=change_detection(1,2);
            x2=change_detection(2,1);
            y2=change_detection(2,2);
            connect_line(x1,y1,x2,y2);
        }
        if(ex_crossroad_state==3)//对十字路口补线
        {
            uint8 x1,y1,x2,y2;
            x1=change_detection(1,1);
            y1=change_detection(1,2);
            x2=change_detection(2,1);
            y2=change_detection(2,2);
            connect_line(x1,y1,x2,y2);
        }
    }

}
//--------------------------------------------------------------------------------------------------------------------------
// 函数简介     环岛判断
// 使用实例     roundabout_detection();
//--------------------------------------------------------------------------------------------------------------------------
void roundabout_detection(void)
{
    if(lost_line_left()!=-1 || lost_line_right()!=-1)//进入元素识别
    {
        if(lost_line_left()!=-1 && lost_line_right()==-1)//左环岛判断
        {
            if(straight_line_judgment(ex_rightline)==1)//排除弯道，检索右边线是否为直线
            {
                ex_roundabout_state=1;
                ex_roundabout_type=1;
            }
        }
        if(lost_line_left()==-1 && lost_line_right()!=-1)//右环岛判断
        {
            if(straight_line_judgment(ex_leftline)==1)//排除弯道，检索左边线是否为直线
                        {
                            ex_roundabout_state=1;
                            ex_roundabout_type=2;
                        }
        }
    }
}
//--------------------------------------------------------------------------------------------------------------------------
// 函数简介     环岛处理
// 使用实例     roundabout_dispose();
//--------------------------------------------------------------------------------------------------------------------------

void roundabout_dispose(void)
{
    if(ex_roundabout_type==1)//左环岛标志
    {
        switch(ex_roundabout_state)//左环岛状态分析
        {
        case 1:
            if(change_detection(2,1)>not_lose_line_parameter)//如果左边即将不丢线
            {
                ex_roundabout_state=2;//环岛状态更新
            }
            break;
        case 2:
            if(roundabout_annular_detection()==1)//如果左边存在圆环
            {
                ex_roundabout_state=3;//环岛状态更新
            }
            break;
        case 3:
            if(lost_line_left()!=-1)//如果左边第二次丢线，找拐点补线进入环岛
            {
                ex_roundabout_state=4;//环岛状态更新
            }
            break;
        case 4:
            if(lost_line_left()==-1 && lost_line_right()==-1)//如果进入环岛，环岛内，两边都不丢线
            {
                ex_roundabout_state=5;//环岛状态更新
            }
            break;
        case 5:
            if(lost_line_left()!=-1 && lost_line_right()!=-1)//即将出环岛，找拐点补线出环岛
            {
                ex_roundabout_state=6;//环岛状态更新
            }
            break;
        case 6:
            if((lost_line_right()-change_detection(4, 2))<=5 && smartcar_state_detection()==1)//右边即将不丢线，车身摆正
            {
                ex_roundabout_state=7;//环岛状态更新
            }
            break;
        case 7:
            if((lost_line_left()-change_detection(2, 2))<=5 && (lost_line_left()-change_detection(2, 2))!=0)//如果左边即将不丢线
            {
                ex_roundabout_state=8;//环岛状态更新
            }
            break;
        case 8:
            if(straight_line_judgment(ex_leftline)==1 && straight_line_judgment(ex_rightline)==1)//完全出环岛
            {
                ex_roundabout_state=0;//环岛状态更新
                ex_roundabout_type=0;//清楚进入环岛标志
            }
            break;
        }
        if(ex_roundabout_state==1 || ex_roundabout_state==2)//补线通过环岛第一次丢线，抵达环岛入口
        {
            uint8 x1,y1,x2,y2;
            x1=change_detection(1, 1);
            x2=change_detection(2, 1);
            y1=change_detection(1, 2);
            y2=change_detection(2, 2);
            connect_line(x1,y1,x2,y2);
        }
        if(ex_roundabout_state==4)//补线进入环岛
        {
            uint8 x1,y1;
            x1=change_detection(2, 1);
            y1=change_detection(2, 2);
            connect_line(x1,y1,(MT9V03X_W-5),(MT9V03X_H-5));
        }
        if(ex_roundabout_state==6)//补线出环岛
        {
            uint8 x1,y1,y2;
            x1=change_detection(3, 1);
            y1=change_detection(3, 2);
            y2=change_detection(2, 2);
            connect_line(x1,y1,10,y2);
        }
        if(ex_roundabout_state==7)
        {
            uint8 x1,y1,x2=0,y2;
            x1=change_detection(2, 1);
            y1=change_detection(2, 2);
            for(uint8 i=(y1+1);i<find_start;i++)
            {
                if(ex_leftline[i]>x2)
                    {
                        x2=ex_leftline[i];
                        y2=i;
                    }
            }
            connect_line(x1,y1,x2,y2);
            connect_line(x2,y2,(x2-1),find_end);
        }
    }
    if(ex_roundabout_type==2)//右环岛处理，在左环岛基础上编写，比赛应该只有左环岛
    {
        switch(ex_roundabout_state)
        {
        case 1:
            if(change_detection(4,1)>not_lose_line_parameter)
            {
                ex_roundabout_state=2;
            }
            break;
        case 2:
            if(roundabout_annular_detection()==2)
            {
                ex_roundabout_state=3;
            }
            break;
        case 3:
            if(lost_line_right()!=-1)
            {
                ex_roundabout_state=4;
            }
            break;
        case 4:
            if(lost_line_left()==-1 && lost_line_right()==-1)
            {
                ex_roundabout_state=5;
            }
            break;
        case 5:
            if(lost_line_left()!=-1 && lost_line_right()!=-1)
            {
                ex_roundabout_state=6;
            }
            break;
        case 6:
            if((lost_line_left()-change_detection(2, 2))<=5 && smartcar_state_detection()==1)
            {
                ex_roundabout_state=7;
            }
            break;
        case 7:
            if((lost_line_right()-change_detection(4, 2))<=5 && (lost_line_right()-change_detection(4, 2))!=0)
            {
                ex_roundabout_state=8;
            }
            break;
        case 8:
            if(straight_line_judgment(ex_leftline)==1 && straight_line_judgment(ex_rightline)==1)//完全出环岛
            {
                ex_roundabout_state=0;
                ex_roundabout_type=0;
            }
            break;
        }
        if(ex_roundabout_state==1 || ex_roundabout_state==2)
        {
            uint8 x1,y1,x2,y2;
            x1=change_detection(3, 1);
            x2=change_detection(4, 1);
            y1=change_detection(3, 2);
            y2=change_detection(4, 2);
            connect_line(x1,y1,x2,y2);
        }
        if(ex_roundabout_state==4)
        {
            uint8 x1,y1;
            x1=change_detection(4, 1);
            y1=change_detection(4, 2);
            connect_line(x1,y1,5,(MT9V03X_H-5));
        }
        if(ex_roundabout_state==6)
        {
            uint8 x1,y1,y2;
            x1=change_detection(1, 1);
            y1=change_detection(1, 2);
            y2=change_detection(3, 2);
            connect_line(x1,y1,(MT9V03X_W-10),y2);
        }
        if(ex_roundabout_state==7)
        {
            uint8 x1,y1,x2=0,y2;
            x1=change_detection(4, 1);
            y1=change_detection(4, 2);
            for(uint8 i=(y1+1);i<find_start;i++)
            {
                if(ex_rightline[i]>x2)
                {
                    x2=ex_rightline[i];
                    y2=i;
                }
            }
            connect_line(x1,y1,x2,y2);
            connect_line(x2,y2,(x2+1),find_end);
        }
    }
}

//--------------------------------------------------------------------------------------------------------------------------
// 函数简介     车身摆正检测
// 返回值         1代表车身摆正,大于smartcar_state_parament的角度将被返回，范围为-90到90
//--------------------------------------------------------------------------------------------------------------------------
int8 smartcar_state_detection(void)
{
    if(straight_line_judgment(ex_midline)==1)
    {
        uint8 temp;
        temp=ex_midline[20]-ex_midline[55];
        temp=arctan(temp,35);
        if(func_abs(temp)<smartcar_state_parament)return 1;
        return temp;
    }
}
//--------------------------------------------------------------------------------------------------------------------------
// 函数简介     环形检测
// 返回值         是否检测到环形，1为左环形，2为右环形，-1未检测到环形；
// 使用实例     roundabout_annular_detection();
//--------------------------------------------------------------------------------------------------------------------------
int8 roundabout_annular_detection(void)
{
    uint8 temp_l=0,temp_r=0;
    uint8 increase=0,reduce=0;
    temp_l=lost_line_left();
    temp_r=lost_line_right();
    if((temp_l!=-1 && temp_r==-1)||(temp_l==-1 && temp_r!=-1))
    {
        if(temp_l!=-1)
        {
            for(uint8 i=temp_l;i>find_start;i--)
                {
                    if(ex_leftline[i]>ex_leftline[i+1] && ex_leftline[i-1]>ex_leftline[i])increase++;
                    if(ex_leftline[i]<ex_leftline[i+1] && ex_leftline[i-1]<ex_leftline[i])reduce++;
                    if(increase>=annular_peremeter && reduce>=annular_peremeter)
                        {
                            return 1;
                        }
                }
        }
        else
        {
            for(uint8 i=temp_r;i>find_start;i--)
                {
                if(ex_rightline[i]>ex_rightline[i+1] && ex_rightline[i-1]>ex_rightline[i])increase++;
                if(ex_rightline[i]<ex_rightline[i+1] && ex_rightline[i-1]<ex_rightline[i])reduce++;
                if(increase>=annular_peremeter && reduce>=annular_peremeter)
                {
                    return 2;
                }
                }
        }
    }
    return -1;
}
//--------------------------------------------------------------------------------------------------------------------------
// 函数简介     对直线进行判定
// 参数说明 arr     传入数组
// 返回值       1为直线 0为非直线
// 使用实例     Straight_line_judgment(left_arr);
// 备注信息     只判断30-100
//--------------------------------------------------------------------------------------------------------------------------
uint8 straight_line_judgment(uint16 arr[Row])
{
  short i,sum=0;
  float kk;
  kk=((float)arr[90]-(float)arr[20])/70.0;//计算k值
  sum = 0;
  for(i=20;i<=90;i++)
    if(((arr[20]+(float)(i-20)*kk)-arr[i])<=35) sum++;//如果理论值与实际值的差异小于等于35，则计数
    else break;
  if(sum>68 && kk>-1.1 && kk<1.1) return 1;
  else return 0;
}
//-----------------------------------------------------------------------------------------------------------------------------
// 函数简介     补线函数
// 参数说明 x1  中断点1的x坐标
// 参数说明 y1  中断点1的y坐标
// 参数说明 x2  中断点2的x坐标
// 参数说明 y2  中断点2的y坐标
// 使用实例     line(0,0,20,30);
//-----------------------------------------------------------------------------------------------------------------------------
void connect_line(uint8 x1,uint8 y1,uint8 x2,uint8 y2)
{
  short i,j,swap;
  float k;
  if(y1>y2)
  {
    swap = x1;
    x1 = x2;
    x2 = swap;
    swap = y1;
    y1 = y2;
    y2 = swap;
  }
  if(x1==x2)
  {
    for(i=y1;i<y2+1;i++)
        ex_mt9v03x_image_binaryzation[i][x1]=0;
  }
  else if(y1==y2)
  {
    for(i=x1;i<x2+1;i++)
        ex_mt9v03x_image_binaryzation[y1][i]=0;
  }
  else
  {
    k = ((float)x2-(float)x1)/((float)y2-(float)y1);
    for(i=y1;i<=y2;i++)
        ex_mt9v03x_image_binaryzation[i][(short)(x1+(i-y1)*k)]=0;
  }
}
//------------------------------------------------------------------------------------------------------------------------------
// 函数简介     斑马线识别（后续需要根据实际情况修改）
//------------------------------------------------------------------------------------------------------------------------------
void zebra_crossing(void)
{
    for(uint8 hang = find_start;hang<find_end;hang++)
    {
        uint8 garage_count=0,white_black,black_white ;
        for(uint8 lie = 10;lie<100;lie++)
         {
               if(ex_mt9v03x_image_binaryzation[hang][lie]==255)//通过突变（拐点）判断斑马线
               {
                   white_black=1;
               }
               else
               {
                   white_black=0;
               }

               if(white_black!=black_white)//如果是拐点，则对斑马线进行计数
               {
                 black_white = white_black;
                 garage_count++;
               }
               if(garage_count>11)//如果，拐点出现次数超过定值确定为斑马线，同时对斑马线通过次数加1
               {
                   ex_zebra_pass_count++;
               }
           }
        if(ex_zebra_pass_count>2)
           {
                ex_zebra_crossing_flag=1;
               break;
           }
       }
}
