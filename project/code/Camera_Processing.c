/*
 * Camera_Processing.c
 *
 *  Created on: 2024年3月17日
 *      Author: 21335
 */

#include <stdio.h>
#include "math.h"
#include "stdlib.h"
#include "zf_common_headfile.h"

#define M_PI       3.14159265358979323846   // pi
//图像-----------------------------------------------------------------------------------------------------------------
uint8 image[image_h][image_w]={{0}};  //使用的图像
uint8 l_shade_255=0,r_shade_255=0; //左侧右侧阴影  也就是 白色方块
uint8 centre_line=34;
float track_width=26;//赛道宽度
uint8 midline[92];
//大津法---------------------------------------------------------------------------------------------------------------
uint8  My_Threshold = 0, My_Threshold_1 = 0; //二值化之后的阈值
float  My_Threshold_cha = 10;//阈值补偿 之后扔到ui里面可以进行调节
//逆透视---------------------------------------------------------------------------------------------------------------
uint8_t *PerImg_ip[RESULT_ROW][RESULT_COL]={{0}};//存储图像地址
//八邻域---------------------------------------------------------------------------------------------------------------

#define USE_num image_h*3   //定义找点的数组成员个数按理说300个点能放下，但是有些特殊情况确实难顶，多定义了一点
int image_ok=0;
uint8 Find_Boundary_l[300][2]={{0}};//左边界线 0为y  1为x 存储内容为八邻域扫描出来的边界
uint8 Find_Boundary_r[300][2]={{0}};//右边界线 0为y  1为x 存储内容为八邻域扫描出来的边界
uint8 Find_Boundary_z[300][2]={{0}};//中边界线 0为y  1为x 存储内容为八邻域扫描出来的边界
uint8 lost_line[300][2]={{0}};//0左 1右   是否丢线 数值等于0不丢 等于1丢线
uint8 lostline_flag_l=0,lostline_flag_r=0;  //左右丢线标志位
uint8 Find_Boundary_centre[300][2]={{0}};//中线 0为y  1为x 存储内容为八邻域扫描出来的边界
uint16 l_data_statics=0;//统计左边总共的长度
uint16 r_data_statics=0;//统计右边总共的长度
uint8 hight_image = 0;//最高点
uint16 centre=0,centre_last=centre_image,centre_s=0;//舵机使用的中线值  centre_s 是为了看前方弯道程度之后 分段pd
//曲率判断---------------------------------------------------------------------------------------------------------------
uint8 curvature_l[6]={0},curvature_r[6]={0};  //左右边线曲率 近段012  中段3  远段4  弯道5检测 近端直角
uint8 straight_l=0,straight_r=0;  //直线判断标志位 1为 近端直 2中段直 3远段直
uint8 Corners_l=0,Corners_r=0;  //弯道判断标志位 1为 弯道
uint8 ramp_flag=0,ramp_widch[4]={0};

double curvature_l_l=0;

//十字----------------------------------------------------------------------------------------------------------------
uint8 l_Down[4]={0},l_on[4]={0},r_Down[4]={0},r_on[4]={0}; //十字的四个拐点 0y1x2flag
int cross_flag=0;  //十字标志位


//斑马线---------------------------------------------------------------------------------------------------------------
int zebra_crossing_flag = 0;  //斑马线标志位
int region=0; //斑马线跳变点
//环岛---------------------------------------------------------------------------------------------------------------
int L_island_flag = 0,R_island_flag = 0;  //左右环岛标志位
uint8 is_L_on[4]={0},is_R_on[4]={0};  //环岛 左上 右上 两个拐点0y1x
uint8 is_L_down[3]={0},is_R_down[3]={0};  //环岛 左下 右下 两个拐点0y1x
#define island_in_limit_x  35 //环岛左上限制点位
uint32 island_length=0;
int island_big=0;
//直线加速---------------------------------------------------------------------------------------------------------------
int run_flag=0,out_flag=0;
int speed_flag=0,speed_in_flag=0;
int check_flag = 0;//发车检查
float speed_increase=0; //直线加速
uint16 lr_en_speed=0;//左右平均编码器累计值
int16 l_en_speed=0,r_en_speed=0; //左 右 编码器的值
int16 l_en_speed_last=0,r_en_speed_last=0; //左 右 编码器上一次的值
float speed_l=0,speed_r=0; //速度
float Servos_out; //舵机输出值
attitude_t g_attitude;
attitude_t angle;  //环岛使用
PID Motor_pid_l;
PID Motor_pid_r;


//--------------------------------------------------------------------------------------
// 函数简介     去畸变+逆透视处理初始化
// 备注信息     只需调用一次即可，随后使用ImageUsed指针即可
//--------------------------------------------------------------------------------------
void ImagePerspective_Init(void) {

    static uint8_t BlackColor = 0;
    double change_un_Mat[3][3] ={{-0.665414,0.876223,-62.464261},{0.000000,0.126916,-27.000158},{0.000000,0.009322,-0.983062}};
    for (int i = 0; i < RESULT_COL ;i++) {
        for (int j = 0; j < RESULT_ROW ;j++) {
            int local_x = (int) ((change_un_Mat[0][0] * i
                    + change_un_Mat[0][1] * j + change_un_Mat[0][2])
                    / (change_un_Mat[2][0] * i + change_un_Mat[2][1] * j
                            + change_un_Mat[2][2]));
            int local_y = (int) ((change_un_Mat[1][0] * i
                    + change_un_Mat[1][1] * j + change_un_Mat[1][2])
                    / (change_un_Mat[2][0] * i + change_un_Mat[2][1] * j
                            + change_un_Mat[2][2]));
            if (local_x
                    >= 0&& local_y >= 0 && local_y < USED_ROW && local_x < USED_COL){
                PerImg_ip[j][i] = &PER_IMG[local_y][local_x];
            }
            else {
                PerImg_ip[j][i] = &BlackColor;          //&PER_IMG[0][0];
            }

        }
    }

}


/*ImageUsed[0][0]代表图像左上角的值*/

/*完成摄像头初始化后，调用一次ImagePerspective_Init，此后，直接调用ImageUsed   即为去畸变结果*/

//-------------------------------------------------------------------------------------------------------------------
// 函数简介
// 参数说明
// 返回参数
// 使用示例
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void empty_flag(void)
{
    l_Down[2]=0;
    l_on[2]=0;
    r_Down[2]=0;
    r_on[2]=0;

    if(lr_en_speed>60000)lr_en_speed=0;

    is_L_on[2] = 0;
    is_L_down[2] = 0;
    is_R_on[2] = 0;
    is_R_down[2] = 0;

    is_L_on[0]=0;
    is_R_on[0]=0;

    straight_l=0;
    straight_r=0;
    Corners_l=0;
    Corners_r=0;

    lostline_flag_l=0;
    lostline_flag_r=0;
    image_ok=0;


}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     my_adapt_threshold  初次二值化
// 参数说明
// 返回参数
// 使用示例
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
uint8 my_adapt_threshold(uint8 *image, uint16 col, uint16 row)   //注意计算阈值的一定要是原图像
{
   #define GrayScale 256
    uint16 width = col;
    uint16 height = row;
    int pixelCount[GrayScale];
    float pixelPro[GrayScale];
    int i, j, pixelSum = width * height/4;
    uint8 threshold = 0;
    uint8* data = image;  //指向像素数据的指针
    for (i = 0; i < GrayScale; i++)
    {
        pixelCount[i] = 0;
        pixelPro[i] = 0;
    }

    uint32 gray_sum=0;
    //统计灰度级中每个像素在整幅图像中的个数
    for (i = 0; i < height; i+=2)
    {
        for (j = 0; j < width; j+=2)
        {
            pixelCount[(int)data[i * width + j]]++;  //将当前的点的像素值作为计数数组的下标
            gray_sum+=(int)data[i * width + j];       //灰度值总和
        }
    }

    //计算每个像素值的点在整幅图像中的比例

    for (i = 0; i < GrayScale; i++)
    {
        pixelPro[i] = (float)pixelCount[i] / pixelSum;
    }

    //遍历灰度级[0,255]
    float w0, w1, u0tmp, u1tmp, u0, u1, u, deltaTmp, deltaMax = 0;


        w0 = w1 = u0tmp = u1tmp = u0 = u1 = u = deltaTmp = 0;
        for (j = 0; j < GrayScale; j++)
        {

                w0 += pixelPro[j];  //背景部分每个灰度值的像素点所占比例之和   即背景部分的比例
                u0tmp += j * pixelPro[j];  //背景部分 每个灰度值的点的比例 *灰度值

               w1=1-w0;
               u1tmp=gray_sum/pixelSum-u0tmp;

                u0 = u0tmp / w0;              //背景平均灰度
                u1 = u1tmp / w1;              //前景平均灰度
                u = u0tmp + u1tmp;            //全局平均灰度
                deltaTmp = w0 * pow((u0 - u), 2) + w1 * pow((u1 - u), 2);
                if (deltaTmp > deltaMax)
                {
                    deltaMax = deltaTmp;
                    threshold = (uint8)j;
                }
                if (deltaTmp < deltaMax)
                {
                    break;
                }
        }

    return threshold;
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介
// 参数说明
// 返回参数
// 使用示例
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
uint8 my_adapt_threshold_2(uint8 *image, uint16 col, uint16 row)   //注意计算阈值的一定要是原图像
{
   #define GrayScale 256
    uint16 width = col;
    uint16 height = row;
    int pixelCount[GrayScale];
    float pixelPro[GrayScale];
    int i, j, pixelSum = width * height/4;
    uint8 threshold = 0;
    uint8* data = image;  //指向像素数据的指针
    for (i = 0; i < GrayScale; i++)
    {
        pixelCount[i] = 0;
        pixelPro[i] = 0;
    }

    uint32 gray_sum=0;
    //统计灰度级中每个像素在整幅图像中的个数
    for (i = 1; i < height; i+=2)
    {
        for (j = 1; j < width; j+=2)
        {
            if( data[i * width + j]  > My_Threshold_1 )
            {
                pixelCount[(int)data[i * width + j]]++ ;  //将当前的点的像素值作为计数数组的下标
                gray_sum+=(int)data[i * width + j] ;       //灰度值总和
            }
            else
            {
                pixelCount[(int) My_Threshold_1]++ ;  //将当前的点的像素值作为计数数组的下标
                gray_sum+= (int) My_Threshold_1 ;       //灰度值总和
            }
        }
    }

    //计算每个像素值的点在整幅图像中的比例

    for (i = 0; i < GrayScale; i++)
    {
        pixelPro[i] = (float)pixelCount[i] / pixelSum;
    }

    //遍历灰度级[0,255]
    float w0, w1, u0tmp, u1tmp, u0, u1, u, deltaTmp, deltaMax = 0;


        w0 = w1 = u0tmp = u1tmp = u0 = u1 = u = deltaTmp = 0;
        for (j = 0; j < GrayScale; j++)
        {

                w0 += pixelPro[j];  //背景部分每个灰度值的像素点所占比例之和   即背景部分的比例
                u0tmp += j * pixelPro[j];  //背景部分 每个灰度值的点的比例 *灰度值

               w1=1-w0;
               u1tmp=gray_sum/pixelSum-u0tmp;

                u0 = u0tmp / w0;              //背景平均灰度
                u1 = u1tmp / w1;              //前景平均灰度
                u = u0tmp + u1tmp;            //全局平均灰度
                deltaTmp = w0 * pow((u0 - u), 2) + w1 * pow((u1 - u), 2);
                if (deltaTmp > deltaMax)
                {
                    deltaMax = deltaTmp;
                    threshold = (uint8)j;
                }
                if (deltaTmp < deltaMax)
                {
                    break;
                }
        }

    return threshold;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     otsuThreshold   图像压缩
// 参数说明     *image：图像地址  width：图像宽    height：图像高
// 返回参数
// 使用示例     otsuThreshold( mt9v03x_image[0], MT9V03X_W, MT9V03X_H);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
uint8 otsuThreshold(uint8 *image, uint16 width,  uint16 height)
{
    #define GrayScale 256
    int pixelCount[GrayScale] = {0};//每个灰度值所占像素个数
    float pixelPro[GrayScale] = {0};//每个灰度值所占总像素比例
    int i,j;
    int Sumpix = width * height/4;   //总像素点
    uint8 threshold = 0;
    uint8* data = image;  //指向像素数据的指针


    //统计灰度级中每个像素在整幅图像中的个数
    for (i = 0; i < height; i+=2)
    {
        for (j = 0; j < width; j+=2)
        {
            pixelCount[(int)data[i * width + j]]++;  //将像素值作为计数数组的下标
          //   pixelCount[(int)image[i][j]]++;    若不用指针用这个
        }
    }
    float u = 0;
    for (i = 0; i < GrayScale; i++)
    {
        pixelPro[i] = (float)pixelCount[i] / Sumpix;   //计算每个像素在整幅图像中的比例
        u += i * pixelPro[i];  //总平均灰度
    }


    float maxVariance=0.0;  //最大类间方差
    float w0 = 0, avgValue  = 0;  //w0 前景比例 ，avgValue 前景平均灰度
    for(int i = 0; i < 256; i++)     //每一次循环都是一次完整类间方差计算 (两个for叠加为1个)
    {
        w0 += pixelPro[i];  //假设当前灰度i为阈值, 0~i 灰度像素所占整幅图像的比例即前景比例
        avgValue  += i * pixelPro[i];

        float variance = pow((avgValue/w0 - u), 2) * w0 /(1 - w0);    //类间方差
        if(variance > maxVariance)
        {
            maxVariance = variance;
            threshold = (uint8)i;
        }
    }


    return threshold;

}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//  @name           get_Threshold  //指针
//  @brief          优化之后的的大津法。大津法就是一种能够算出一幅图像最佳的那个分割阈值的一种算法。
//  @brief          这个东西你们可以如果实在不能理解就直接拿来用，什么参数都不用修改，只要没有光照影响，那么算出来的这个阈值就一定可以得到一幅效果还不错的二值化图像。
//  @parameter      ex_mt9v03x_binarizeImage  原始的灰度图像数组
//  @parameter      clo    图像的宽（图像的列）
//  @parameter      row    图像的高（图像的行）
//  @return         uint8
//  @time           2022年2月17日
//  @Author
//  Sample usage:   Threshold = Threshold_deal(Image_Use[0], 80, 60); 把存放60行80列的二维图像数组Image_Use传进来，求出这幅图像的阈值，并将这个阈值赋给Threshold。
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
uint8 get_Threshold(void)
{
  #define GrayScale 256
  uint16 width = MT9V03X_W;
  uint16 height = MT9V03X_H;
  int pixelCount[GrayScale];
  float pixelPro[GrayScale];
  int i, j, pixelSum = width * height;
  uint8 threshold = 0;
  for (i = 0; i < GrayScale; i++)                    //先把pixelCount和pixelPro两个数组元素全部赋值为0
  {
    pixelCount[i] = 0;
    pixelPro[i] = 0;
  }

  uint32 gray_sum = 0;
  /**************************************统计每个灰度值(0-255)在整幅图像中出现的次数**************************************/
  for (i = 0; i < height; i += 1)                   //遍历图像的每一行，从第零行到第59行。
  {
    for (j = 0; j < width; j += 1)                  //遍历图像的每一列，从第零列到第79列。
    {
      pixelCount[mt9v03x_image[i][j]]++;          //将当前的像素点的像素值（灰度值）作为计数数组的下标。
      gray_sum += mt9v03x_image[i][j];          //计算整幅灰度图像的灰度值总和。
    }
  }
  /**************************************统计每个灰度值(0-255)在整幅图像中出现的次数**************************************/



  /**************************************计算每个像素值（灰度值）在整幅灰度图像中所占的比例*************************************************/
  for (i = 0; i < GrayScale; i++)
  {
      pixelPro[i] = (float)pixelCount[i] / pixelSum;
  }
  /**************************************计算每个像素值（灰度值）在整幅灰度图像中所占的比例**************************************************/



  /**************************************开始遍历整幅图像的灰度值（0-255），这一步也是大津法最难理解的一步***************************/
  /*******************为什么说他难理解？因为我也是不理解！！反正好像就是一个数学问题，你可以理解为数学公式。***************************/
  float w0, w1, u0tmp, u1tmp, u0, u1, u, deltaTmp, deltaMax = 0;
  w0 = w1 = u0tmp = u1tmp = u0 = u1 = u = deltaTmp = 0;
  for (j = 0; j < GrayScale; j++)
  {
    w0 += pixelPro[j];                          //求出背景部分每个灰度值的像素点所占的比例之和，即背景部分的比例。
    u0tmp += j * pixelPro[j];

    w1 = 1 - w0;
    u1tmp = gray_sum / pixelSum - u0tmp;

    u0 = u0tmp / w0;                            //背景平均灰度
    u1 = u1tmp / w1;                            //前景平均灰度
    u = u0tmp + u1tmp;                          //全局平均灰度
    deltaTmp = w0 * pow((u0 - u), 2) + w1 * pow((u1 - u), 2);
    if (deltaTmp > deltaMax)
    {
      deltaMax = deltaTmp;
      threshold = j;
    }
    if (deltaTmp < deltaMax)
    {
      break;
    }
  }
  /**************************************开始遍历整幅图像的灰度值（0-255），这一步也是大津法最难理解的一步***************************/
  /*******************为什么说他难理解？因为我也是不理解！！反正好像就是一个数学问题，你可以理解为数学公式。***************************/

  return threshold;                             //把上面这么多行代码算出来的阈值给return出去。
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介         二值化
// 参数说明
// 返回参数
// 使用示例
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void Binaryzation(void)
{

   //双阈值处理法  先处理出来一次阈值,把阈值之下的变为黑色再进行一次
   //优点  加偏振片处理出来的效果比较不错
   //缺点  费时间
//    My_Threshold_1 =  //1ms

    My_Threshold   = (int)get_Threshold()    + (uint8)My_Threshold_cha;  //1ms


}


//-------------------------------------------------------------------------------------------------------------------
// 函数简介     image_draw  画黑框 加上 对图像进行二值化
// 参数说明
// 返回参数
// 使用示例
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void image_draw(void)
{
    int a=0;
    for(int i = image_h-3; i >= 0 ; i-- )//从逆透视图像数组里面提出,在进行一次阈值梯度递减的二值化
    {
//        if( i%8 == 1 )
//        {
//            My_Threshold --;
//        }
        for(int j=0;j< image_w-2;j++)
        {

            if( ImageUsed[i][j] > My_Threshold)
                image[i+1][j+1]=255;
            else{
                image[i+1][j+1]=0;
                if(i < 88 && i > 86)a++;
            }
        }
        if(run_flag==1)
        {
            if(a>65)
            {
             out_flag=1; run_flag=0;
            }
            a=0;
        }
    }

//    uint16 N_zaoddian=0;
//    for(int i=0;i<89;i++)       //白噪点 去除
//        for(int j=1;j<70;j++)
//        {
//            if( image[90-i][j] == 255 )
//            {
//                N_zaoddian =                 image[90-i-1][j]
//                           + image[90-i][j-1]               + image[90-i]  [j+1]
//                                            +image[90-i+1][j] ;
//                if( N_zaoddian <  255 * 2 )
//                {
//                    image[90-i][j] = 0 ;
//                }
//            }
//            else
//            {
//                N_zaoddian =                 image[90-i-1][j]
//                           + image[90-i][j-1]               + image[90-i]  [j+1]
//                                            +image[90-i+1][j] ;
//                if( N_zaoddian >  255 * 2 )
//                {
//                    image[90-i][j] = 255 ;
//                }
//            }
//
//        }

}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     Find_Boundary()   寻找最初的左右边界线
// 参数说明
// 返回参数     find_Boundary_flag 判断是否找到左右边界
// 使用示例
// 备注信息
//-------------------------------------------------------------------------------------------------------------------

void shade_compute(void)   //判断是否扫描到阴影
{
    l_shade_255 = 0 ,r_shade_255 = 0 ;
    for(int i = 13 ; i < 20; i++)
        for(int j=78;j<86;j++)
            if( image[j][i] == 255 ) l_shade_255++;


    for( int i = 61 ; i < 68 ; i ++)
        for(int j=78;j<86;j++)
            if( image[j][i] == 255 )   r_shade_255++;


}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     Find_Boundary()   寻找最初的左右边界线
// 参数说明
// 返回参数     find_Boundary_flag 判断是否找到左右边界
// 使用示例
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
int Find_Boundary(void)
{
    int find_Boundary_flag = 0;

     l_data_statics=0;//统计左边总共的长度
     r_data_statics=0;//统计右边总共的长度


     if (zebra_crossing_flag!=1)  {//当未出现斑马线时候就从中间向两侧扫描 为啥 是i = 2 因为一圈是个黑框 最底下搜不出来边界线
         for (uint8 i = 2; i < 3; i++){// Y
             for (uint8 j = centre_line ; j >= 0; j--){//左边线  X
                 if (image[image_h - i][j] == 0
                         && image[image_h - i][j + 1] == 255
                         && image[image_h - i][j + 2] == 255){
                     Find_Boundary_l[i-2][0] = image_h - i;
                     Find_Boundary_l[i-2][1] = j;
                     l_data_statics++;
                     find_Boundary_flag++;
                     break;
                 }
             }

             for (uint8 j = centre_line ; j <= image_w - 1; j++){ //右边线   X
                 if (image[image_h - i][j] == 0
                         && image[image_h - i][j - 1] == 255
                         && image[image_h - i][j - 2] == 255){
                     Find_Boundary_r[i-2][0] = image_h - i;
                     Find_Boundary_r[i-2][1] = j;
                     r_data_statics++;
                     find_Boundary_flag++;
                     break;

                 }
             }
         }
     }

//     else{
//         for (uint8 i = 2; i < 8; i++){//0 1 行边界
//             for (uint8 j = 0; j <= image_w / 2; j++){ //左边线
//                 if (image[image_h - i][j] == 0
//                         && image[image_h - i][j + 1] == 255
//                         && image[image_h - i][j + 2] == 255){
//                     Find_Boundary_l[i-2][0] = image_h - i;
//                     Find_Boundary_l[i-2][1] = j;
//                     find_Boundary_flag++;
//                     l_data_statics++;
//                     break;
//                 }
//             }
//
//             for (uint8 j = image_w-1; j >= image_w / 2; j--){  //右边线
//                 if (image[image_h - i][j] == 0
//                         && image[image_h - i][j - 1] == 255
//                         && image[image_h - i][j - 2] == 255){
//                     Find_Boundary_r[i-2][0] = image_h - i;
//                     Find_Boundary_r[i-2][1] = j;
//                     r_data_statics++;
//                     find_Boundary_flag++;
//                     break;
//                 }
//             }
//         }
//     }
     centre_line=(Find_Boundary_r[0][1]+Find_Boundary_l[0][1])/2;
     return find_Boundary_flag;  //返回两边是否找到起始点
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     search                八邻域正式开始找边界的函数，输入参数有点多，调用的时候不要漏了，这个是左右线一次性找完。
// 参数说明     break_flag              ：最多需要循环的次数
// 参数说明     (*image)[image_w]       ：需要进行找点的图像数组，必须是二值图,填入数组名称即可  特别注意，不要拿宏定义名字作为输入参数，否则数据可能无法传递过来
// 参数说明     hightest                ：循环结束所得到的最高高度
// 返回参数
// 使用示例
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void search(uint16 break_flag,uint8(*image_u)[image_w],uint8* hight)
{

    if( Find_Boundary() >= 2){
        l_data_statics=0;//统计左边总共的长度
        r_data_statics=0;//统计右边总共的长度
        //定义八个邻域
        static int8 seeds_l[8][2] = { {-1,1},{0,1},{1,1},{1,0},{1,-1},{0,-1},{-1,-1},{-1,0}, };
        //{-1,-1},{0,-1},{+1,-1},6 5 4      这个是逆时针
        //{-1, 0},       {+1, 0},7   3
        //{-1,+1},{0,+1},{+1,+1},0 1 2
        static int8 seeds_r[8][2] = { {1, 1},{0,1},{-1,1},{-1,0},{-1,-1},{0,-1},{1,-1},{1,0}, };
        //{-1,-1},{0,-1},{+1,-1},4 5 6       这个是顺时针
        //{-1, 0},       {+1, 0},3   7
        //{-1,+1},{0,+1},{+1,+1},2 1 0


        uint8 search_filds_l[2] = { 0}; //只有一个数值，这个数值是之后显示图像的某个点的xy数值
        uint8 search_filds_r[2] = { 0};
        int direction_l=3,direction_r=3;//左右两个边线的方向  0-7
        //开启邻域循环
        while (break_flag--)//最大循环次数_
        {
            if(  Find_Boundary_l[ l_data_statics ][0] >= Find_Boundary_r[ r_data_statics ][0]  )  //开始  左边
            {
                for(int j=0;j<8;j++)
                {
                  if(direction_l<0)     {direction_l+=8;}
                  else if(direction_l>=8)    {direction_l-=8;}
                  search_filds_l[0]=Find_Boundary_l[l_data_statics][0]+seeds_l[direction_l][1];  //y
                  search_filds_l[1]=Find_Boundary_l[l_data_statics][1]+seeds_l[direction_l][0];

                  if(image_u[search_filds_l[0]][search_filds_l[1]]==0)
                  {
                      Find_Boundary_l[l_data_statics+1][0]=search_filds_l[0];
                      Find_Boundary_l[l_data_statics+1][1]=search_filds_l[1];
                      direction_l-=2;  //如果找到黑色的点 就会改变他的方向
                      break;
                  }
                  direction_l++;  //不是黑色 方向加1
                }
                l_data_statics++;
            }

            if (Find_Boundary_r[r_data_statics][0] >= Find_Boundary_l[l_data_statics-1][0])
            {
                for(int j=0;j<8;j++)
                {
                    if(direction_r<0)     {direction_r+=8;}
                    else if(direction_r>=8)    {direction_r-=8;}   //
                    search_filds_r[0]=Find_Boundary_r[r_data_statics][0]+seeds_r[direction_r][1]; //y
                    search_filds_r[1]=Find_Boundary_r[r_data_statics][1]+seeds_r[direction_r][0];

                    if(image_u[search_filds_r[0]][search_filds_r[1]]==0)
                    {
                        Find_Boundary_r[r_data_statics+1][0]=search_filds_r[0];
                        Find_Boundary_r[r_data_statics+1][1]=search_filds_r[1];
                        direction_r-=2;  //如果找到黑色的点 就会改变他的方向
                        break;
                    }
                    direction_r++;  //不是黑色 方向加1
                }
                r_data_statics++;

            }


            if (func_abs(Find_Boundary_r[r_data_statics][0] - Find_Boundary_l[l_data_statics][0]) < 2
                && func_abs(Find_Boundary_r[r_data_statics][1] - Find_Boundary_l[l_data_statics ][1]) < 3
                && r_data_statics>=10 && l_data_statics>=10 )
            {
                *hight = (Find_Boundary_r[r_data_statics][0]+Find_Boundary_l[l_data_statics][0])/2.0;//取出最高点
                image_ok=1;
                break;
            }



        }
    }
    else if(Find_Boundary() < 2 && run_flag == 1){
        out_flag=1;
        run_flag=0;
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介
// 参数说明
// 返回参数
// 使用示例
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
//void zebra_crossing_judge(void)
//{
//    for(int i=10;i<170;i++)
//    {
//        for(int j=;j<)
//    }
//
//}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     检测左右边线是否丢线
// 参数说明
// 返回参数
// 使用示例
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void lost_line_judge(void)
{
    int number = 0;
    for(uint8 i = 0; i < l_data_statics-3; i++){
        if(Find_Boundary_l[i][1]==0){
           number++;
        }
        if(number >= 10 ){
            lostline_flag_l=1;
            number=0;
            break;
        }
    }


    for(uint8 i = 0; i < r_data_statics-3; i++){
        if(Find_Boundary_r[i][1] == image_w-1){
           number++;
        }
        if(number >=10 ){
            lostline_flag_r=1;
            break;}
    }

}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     计算曲线曲率
// 参数说明     line 边线数组
// 参数说明     point  每次增加的点数
// 参数说明     step 一个点 左右两个点的差
// 返回参数
// 使用示例
// 备注信息     直接在curvatureThreePoints内部计算距离平方，避免函数调用
//-------------------------------------------------------------------------------------------------------------------
double calculateAverageCurvature(uint8 line[300][2],int n,int step,int point) {
    double totalCurvature = 0.0;
    int count = 0; // 计算了多少个曲率

    for (int i = step; i < n - step; i+=point) {
        float x0 =line[i-step][1], y0 = line[i-step][0];
        float x1 =line[i][1], y1 =line[i][0];
        float x2 =line[i+step][1], y2 = line[i+step][0];

        float dx0 = x1 - x0, dy0 = y1 - y0;
        float dx1 = x2 - x1, dy1 = y2 - y1;

        double numerator = fabs((x2-x1)*(y1-y0) - (x1-x0)*(y2-y1));
        double denominator = sqrt(pow(dx0, 2) + pow(dy0, 2)) * sqrt(pow(dx1, 2) + pow(dy1, 2));

        if (denominator != 0) {
            totalCurvature += numerator / denominator;
            count++;
        }
    }

    if (count == 0) {
        return 0;
    }

    double averageCurvature = totalCurvature / count;
    return averageCurvature;
}











//-------------------------------------------------------------------------------------------------------------------
// 函数简介
// 参数说明
// 返回参数
// 使用示例
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void curvature_judge(void)
{
    int x[3]={0},y[3]={0};
//远段----------------------------------------------------------------------
    x[0]=Find_Boundary_l[10][1];
    x[1]=Find_Boundary_l[40][1];
    x[2]=Find_Boundary_l[90][1];

    y[0]=Find_Boundary_l[10][0];
    y[1]=Find_Boundary_l[40][0];
    y[2]=Find_Boundary_l[90][0];
    curvature_l[4]=(uint8)Angel_compute(x[0],y[0],x[1],y[1],x[2],y[2]);

    x[0]=Find_Boundary_r[10][1];
    x[1]=Find_Boundary_r[40][1];
    x[2]=Find_Boundary_r[90][1];

    y[0]=Find_Boundary_r[10][0];
    y[1]=Find_Boundary_r[40][0];
    y[2]=Find_Boundary_r[90][0];
    curvature_r[4]=(uint8)Angel_compute(x[0],y[0],x[1],y[1],x[2],y[2]);

//中段----------------------------------------------------------------------
    x[0]=Find_Boundary_l[6][1];
    x[1]=Find_Boundary_l[35][1];
    x[2]=Find_Boundary_l[70][1];

    y[0]=Find_Boundary_l[6][0];
    y[1]=Find_Boundary_l[35][0];
    y[2]=Find_Boundary_l[70][0];
    curvature_l[3]=(uint8)Angel_compute(x[0],y[0],x[1],y[1],x[2],y[2]);

    x[0]=Find_Boundary_r[6][1];
    x[1]=Find_Boundary_r[35][1];
    x[2]=Find_Boundary_r[70][1];

    y[0]=Find_Boundary_r[6][0];
    y[1]=Find_Boundary_r[35][0];
    y[2]=Find_Boundary_r[70][0];
    curvature_r[3]=(uint8)Angel_compute(x[0],y[0],x[1],y[1],x[2],y[2]);


//近段2----------------------------------------------------------------------
    x[0]=Find_Boundary_l[5][1];
    x[1]=Find_Boundary_l[17][1];
    x[2]=Find_Boundary_l[35][1];

    y[0]=Find_Boundary_l[5][0];
    y[1]=Find_Boundary_l[17][0];
    y[2]=Find_Boundary_l[35][0];

    curvature_l[2]=(uint8)Angel_compute(x[0],y[0],x[1],y[1],x[2],y[2]);

    x[0]=Find_Boundary_r[5][1];
    x[1]=Find_Boundary_r[17][1];
    x[2]=Find_Boundary_r[35][1];

    y[0]=Find_Boundary_r[5][0];
    y[1]=Find_Boundary_r[17][0];
    y[2]=Find_Boundary_r[35][0];
    curvature_r[2]=(uint8)Angel_compute(x[0],y[0],x[1],y[1],x[2],y[2]);


//近段1----------------------------------------------------------------------
    x[0]=Find_Boundary_l[5][1];
    x[1]=Find_Boundary_l[12][1];
    x[2]=Find_Boundary_l[25][1];

    y[0]=Find_Boundary_l[5][0];
    y[1]=Find_Boundary_l[12][0];
    y[2]=Find_Boundary_l[25][0];
    curvature_l[1]=(uint8)Angel_compute(x[0],y[0],x[1],y[1],x[2],y[2]);

    x[0]=Find_Boundary_r[5][1];
    x[1]=Find_Boundary_r[12][1];
    x[2]=Find_Boundary_r[25][1];

    y[0]=Find_Boundary_r[5][0];
    y[1]=Find_Boundary_r[12][0];
    y[2]=Find_Boundary_r[25][0];
    curvature_r[1]=(uint8)Angel_compute(x[0],y[0],x[1],y[1],x[2],y[2]);

//弯道6----------------------------------------------------------------------
    x[0]=65;
    x[1]=Find_Boundary_l[2][1];
    x[2]=Find_Boundary_l[15][1];

    y[0]=89;
    y[1]=Find_Boundary_l[2][0];
    y[2]=Find_Boundary_l[15][0];
    curvature_l[5]=(uint8)Angel_compute(x[0],y[0],x[1],y[1],x[2],y[2]);

    x[0]=4;
    x[1]=Find_Boundary_r[2][1];
    x[2]=Find_Boundary_r[15][1];

    y[0]=89;
    y[1]=Find_Boundary_r[2][0];
    y[2]=Find_Boundary_r[15][0];

    curvature_r[5]=(uint8)Angel_compute(x[0],y[0],x[1],y[1],x[2],y[2]);

//边线跟随----------------------------------------------------------------------
    x[0]=68;
    x[1]=Find_Boundary_l[0][1];
    x[2]=Find_Boundary_l[90][1];

    y[0]=90;
    y[1]=Find_Boundary_l[0][0];
    y[2]=Find_Boundary_l[90][0];
    curvature_l[0]=(uint8)Angel_compute(x[0],y[0],x[1],y[1],x[2],y[2]);

    x[0]=1;
    x[1]=Find_Boundary_r[0][1];
    x[2]=Find_Boundary_r[90][1];

    y[0]=90;
    y[1]=Find_Boundary_r[0][0];
    y[2]=Find_Boundary_r[90][0];

    curvature_r[0]=(uint8)Angel_compute(x[0],y[0],x[1],y[1],x[2],y[2]);


    if( curvature_l[1] > 165 && curvature_l[2] > 165
     && func_abs(curvature_l[1] - curvature_l[2]) <= 25
     && Find_Boundary_l[2][1]>10 ){
        straight_l=1;              //近段直线
        if(curvature_l[3] > 168 && abs( curvature_l[3] - curvature_l[1]) <= 15 ){
            straight_l = 2;  //中段
            if(curvature_l[4] > 168 && abs( curvature_l[4] - curvature_l[1]) <= 15 ){
                straight_l = 3;       //全局直线
            }
        }
    }
    else straight_l=0;

     if( curvature_r[1] > 165 && curvature_r[2] > 165
      && func_abs(curvature_r[1] - curvature_r[2]) <= 25
      && Find_Boundary_r[2][1]<60 ){
         straight_r=1;              //近段直线
         if(curvature_r[3] > 168 && abs( curvature_r[3] - curvature_r[1]) <= 15 ){
             straight_r = 2;  //中段
             if(curvature_r[4] > 168 && abs( curvature_r[4] - curvature_r[1]) <= 15 ){
                 straight_r = 3;       //全局直线
             }
         }
     }
     else straight_r=0;




     if( abs( 90- curvature_l[5]) >= 15 && straight_l==0 ){//弯路
         Corners_l=1;
     }
     else Corners_l=0;
     if( abs( 90- curvature_r[5]) >= 15 && straight_r==0 ){
         Corners_r=1;
     }
     else Corners_r=0;

}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     Stayguy_ADS  补线函数
// 参数说明
// 返回参数
// 使用示例
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void  Stayguy_ADS(int x_start, int y_start, int x_end, int y_end)
{
    int16 x_dir = (x_start < x_end ? 1 : -1);
    int16 y_dir = (y_start < y_end ? 1 : -1);
    float temp_rate = 0;   //k
    float temp_b = 0;      //b

    do
    {
        if(x_start != x_end)
        {
            temp_rate = (float)(y_start - y_end) / (float)(x_start - x_end);
            temp_b = (float)y_start - (float)x_start * temp_rate;
        }
        else
        {
            while(y_start != y_end)
            {
                image[y_start][x_start] = 0;
                y_start += y_dir;
            }
            break;
        }

        if(func_abs(y_start - y_end) > func_abs(x_start - x_end))
        {
            while(y_start != y_end)
            {
                image[y_start][x_start] = 0;
                y_start += y_dir;
                x_start = (int16)(((float)y_start - temp_b) / temp_rate);
            }
        }
        else
        {
            while(x_start != x_end)
            {
                image[y_start][x_start] = 0;
                x_start += x_dir;
                y_start = (int16)((float)x_start * temp_rate + temp_b);
            }
        }
    }while(0);
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介   Angel_compute  获取三个坐标形成的夹角
// 参数说明
// 返回参数
// 使用示例
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
double Angel_compute(int x1, int y1, int x2, int y2, int x3, int y3)
 {
     double dotProduct = (double)((x2 - x1) * (x2 - x3) + (y2 - y1) * (y2 - y3));
     double magProduct =
             (sqrt((double)((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1)))) *
             (sqrt((double)((x2 - x3) * (x2 - x3) + (y2 - y3) * (y2 - y3))));

     // 确保在 [-1, 1] 范围内
     double cos_angle = dotProduct / magProduct;
     cos_angle = cos_angle < -1.0 ? -1.0 : (cos_angle > 1.0 ? 1.0 : cos_angle);
     double radian_angle = acos(cos_angle);
     double degree_angle = radian_angle * 180.0 / M_PI;

     return degree_angle;
 }
//-------------------------------------------------------------------------------------------------------------------
// 函数简介         find_cross_down 找十字左下拐点
// 参数说明
// 返回参数
// 使用示例
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void find_cross_down_l(void)
{
    int x[3]={0},y[3]={0};
    double angle=0;
    for(int i = 1 ;i <= 90 && i < l_data_statics - 12 ; i += 2 )
    {
        x[0] = Find_Boundary_l[i][1];     //a b c 三点  求得是b的角度
        x[1] = Find_Boundary_l[i+6][1];
        x[2] = Find_Boundary_l[i+12][1];

        y[0]=Find_Boundary_l[i][0];
        y[1]=Find_Boundary_l[i+6][0];
        y[2]=Find_Boundary_l[i+12][0];


        if( y[0] > y[2]
            && x[0] >= x[2] &&  x[1] > x[2]
            && x[2] > 2
            && image[(y[0]+y[2])/2][(x[2]+x[0])/2] == 0)//ac连线中点一定是 黑色

        {

            angle=Angel_compute( x[0] , y[0] , x[1] , y[1] , x[2] , y[2] );
            if(  func_abs((uint8)angle - 90) <= 20 )
            {
                l_Down[0]=Find_Boundary_l[i+6][0];
                l_Down[1]=Find_Boundary_l[i+6][1];
                l_Down[2]=1;
                l_Down[3]=i+10;


//                put_int32(0,(int)angle);
//                put_int32(0,(int)angle);


                break;
            }
        }
    }
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介         find_cross_down 找十字右下拐点
// 参数说明
// 返回参数
// 使用示例
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void find_cross_down_r(void)
{
    int x[3]={0},y[3]={0};
    double angle=0;
    for(int i = 1;i <= 90 && i < r_data_statics - 12; i += 2 )
    {
        x[0] = Find_Boundary_r[i][1];     //a b c 三点  求得是b的角度
        x[1] = Find_Boundary_r[i+6][1];
        x[2] = Find_Boundary_r[i+12][1];

        y[0]=Find_Boundary_r[i][0];
        y[1]=Find_Boundary_r[i+6][0];
        y[2]=Find_Boundary_r[i+12][0];


        if(    y[0] > y[2]
            && x[0] <= x[2] && x[2] > x[1]
            && x[2] < 68
            && image[(y[0]+y[2])/2][(x[2]+x[0])/2] == 0      //ac连线中点一定是 黑色
             )

        {
            angle=Angel_compute( x[0] , y[0] , x[1] , y[1] , x[2] , y[2] );
            if(  func_abs((uint8)angle - 90) <= 20 )
            {
                r_Down[0]=Find_Boundary_r[i+6][0];
                r_Down[1]=Find_Boundary_r[i+6][1];
                r_Down[2]=1;
                r_Down[3]=i+10;
                break;
            }
        }
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介         find_cross_on 找十字左上拐点
// 参数说明
// 返回参数
// 使用示例
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void find_cross_on_l(void)
{
    int x[3]={0},y[3]={0};
    double angle=0;
    int i = 3;
    if(l_Down[2]==1)i=l_Down[3];
    for( ;i < l_data_statics - 16 ; i += 2 )
    {
        x[0] = Find_Boundary_l[i][1];     //a b c 三点  求得是b的角度
        x[1] = Find_Boundary_l[i+6][1];
        x[2] = Find_Boundary_l[i+12][1];

        y[0]=Find_Boundary_l[i][0];
        y[1]=Find_Boundary_l[i+6][0];
        y[2]=Find_Boundary_l[i+12][0];


        if( y[0] > y[2] && y[1] > y[2]
            && x[0] < x[2]
            && x[0] > 2
            && image[(y[0]+y[2])/2][(x[2]+x[0])/2] == 0)//ac连线中点一定是 黑色

        {

            angle=Angel_compute( x[0] , y[0] , x[1] , y[1] , x[2] , y[2] );
            if(  func_abs((uint8)angle - 90) <= 20 )
            {
                l_on[0]=Find_Boundary_l[i+6][0];
                l_on[1]=Find_Boundary_l[i+6][1];
                l_on[2]=1;
                l_on[3]=i+5;
                break;
            }
        }
    }
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介         find_cross_on 找十字 右 上拐点
// 参数说明
// 返回参数
// 使用示例
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void find_cross_on_r(void)
{
    int x[3]={0},y[3]={0};
    double angle=0;
    int i = 3;
    if(r_Down[2]==1)i=r_Down[3];
    for(; i < r_data_statics - 12 ; i += 2 )
        {
        x[0] = Find_Boundary_r[i][1];     //a b c 三点  求得是b的角度
        x[1] = Find_Boundary_r[i+6][1];
        x[2] = Find_Boundary_r[i+12][1];

        y[0]=Find_Boundary_r[i][0];
        y[1]=Find_Boundary_r[i+6][0];
        y[2]=Find_Boundary_r[i+12][0];


        if(    y[0] > y[2] && y[1] > y[2]
            && x[0] > x[2]
            && x[0] < 68
            && image[(y[0]+y[2])/2][(x[2]+x[0])/2] == 0      //ac连线中点一定是 黑色
             )

        {
            angle=Angel_compute( x[0] , y[0] , x[1] , y[1] , x[2] , y[2] );
            if(  func_abs((uint8)angle - 90) <= 20 )
            {
                r_on[0]=Find_Boundary_r[i+6][0];
                r_on[1]=Find_Boundary_r[i+6][1];
                r_on[2]=1;
                r_on[3]=i+5;

                break;
            }
        }

    }
}



//-------------------------------------------------------------------------------------------------------------------
// 函数简介         find_cross_on 找十字左上拐点   利用 右边线扫描左侧上拐点
// 参数说明
// 返回参数
// 使用示例
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void find_incline_cross_on_l(void)
{
    int x[3]={0},y[3]={0};
    double angle=0;
    for(  int i = 3 ;i < r_data_statics - 16 ; i += 2 )
    {
        x[0] = Find_Boundary_r[i][1];     //a b c 三点  求得是b的角度
        x[1] = Find_Boundary_r[i+8][1];
        x[2] = Find_Boundary_r[i+16][1];

        y[0]=Find_Boundary_r[i][0];
        y[1]=Find_Boundary_r[i+8][0];
        y[2]=Find_Boundary_r[i+16][0];


        if( y[1] > y[0] && x[1] > x[2]
            && x[2] < 68
            && image[(y[0]+y[2])/2][(x[2]+x[0])/2] == 0)//ac连线中点一定是 黑色

        {

            angle=Angel_compute( x[0] , y[0] , x[1] , y[1] , x[2] , y[2] );
            if(  func_abs((uint8)angle - 90) <= 20 )
            {

                l_on[0]=Find_Boundary_r[i+8][0];
                l_on[1]=Find_Boundary_r[i+8][1];
                l_on[2]=1;
                l_on[3]=i+5;
                break;
            }
        }
    }
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介         find_cross_on 找十字左上拐点   利用 右边线扫描左侧上拐点
// 参数说明
// 返回参数
// 使用示例
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void find_incline_cross_on_r(void)
{
    int x[3]={0},y[3]={0};
    double angle=0;
    for(  int i = 3 ;i < l_data_statics - 16 ; i += 2 )
    {
        x[0] = Find_Boundary_l[i][1];     //a b c 三点  求得是b的角度
        x[1] = Find_Boundary_l[i+8][1];
        x[2] = Find_Boundary_l[i+16][1];

        y[0]=Find_Boundary_l[i][0];
        y[1]=Find_Boundary_l[i+8][0];
        y[2]=Find_Boundary_l[i+16][0];


        if( y[1] > y[0] && x[1] < x[2]
            && x[0] > 2
            && image[(y[0]+y[2])/2][(x[2]+x[0])/2] == 0)//ac连线中点一定是 黑色

        {

            angle=Angel_compute( x[0] , y[0] , x[1] , y[1] , x[2] , y[2] );
            if(  func_abs((uint8)angle - 90) <= 20 )
            {

                r_on[0]=Find_Boundary_l[i+8][0];
                r_on[1]=Find_Boundary_l[i+8][1];
                r_on[2]=1;
                r_on[3]=i+5;
                break;
            }

        }
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介  cross_checkout 十字补充检测
// 参数说明
// 返回参数
// 使用示例
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void cross_checkout(void)
{

    if(cross_flag==1)
    {
        int x1,y1;
        if( (r_Down[2] + l_Down[2] + l_on[2] + r_on[2])==3)   // 三补一
        {
           if( l_Down[2] == 0)
           {
               y1 = l_on[0]-r_on[0];
               x1 = l_on[1]-r_on[1];

               l_Down[0] = func_limit_ab(r_Down[0] + y1+2,2,90);
               l_Down[1] = func_limit_ab(r_Down[1] + x1-2,2,68);
               l_Down[2] = 1 ;
           }
            else if(r_Down[2] == 0)
           {
               y1 = l_on[0]-r_on[0];
               x1 = l_on[1]-r_on[1];

               r_Down[0] =func_limit_ab( l_Down[0] - y1+2,2,90);
               r_Down[1] =func_limit_ab( l_Down[1] - x1+2,2,68);
               r_Down[2] = 1 ;
           }
            else if(l_on[2] == 0)
           {
               y1 = l_Down[0]-r_Down[0];
               x1 = l_Down[1]-r_Down[1];

               l_on[0] = func_limit_ab(r_on[0] + y1-2,2,90);
               l_on[1] = func_limit_ab(r_on[1] + x1-2,2,68);
               l_on[2] = 1;
           }
           else
           {
               y1 = r_Down[0]-l_Down[0];
               x1 = r_Down[1]-l_Down[1];
               r_on[0] =func_limit_ab( l_on[0] + y1-2 ,2,90);
               r_on[1] =func_limit_ab( l_on[1] + x1+2,2,68);
               r_on[2] = 1;
           }
        }


        if( l_Down[2] == 1 &&  l_on[2] == 1
                && func_abs(  track_width -(int)( sqrt( ( l_Down[0] - l_on[0] )  *( l_Down[0] - l_on[0] )
                        +( ( l_Down[1] - l_on[1] )  * ( l_Down[1] - l_on[1] ) ) ) ) ) <= 4 )
        {
            l_Down[2] = 1 ;
            l_on[2] = 1 ;
        }
        else
        {
            l_Down[2] = 0 ;
            l_on[2] = 0 ;
        }




        if( r_Down[2] == 1 &&  r_on[2] == 1
                && func_abs(  track_width -(int)( sqrt( ( r_Down[0] - r_on[0] )  *( r_Down[0] - r_on[0] )
                        +( ( r_Down[1] - r_on[1] )  * ( r_Down[1] - r_on[1] ) ) ) ) ) <= 4 )
        {
            r_Down[2] = 1 ;
            r_on[2] = 1 ;
        }
        else
        {
            r_Down[2] = 0 ;
            r_on[2] = 0 ;
        }




    }



    else if( cross_flag==2 || cross_flag==3 )
    {



        if( r_on[2]==1 && l_on[2]==1 )
        {
            if(func_abs(  track_width - (int)(sqrt( ( l_on[0] - r_on[0] )  *( l_on[0] - r_on[0] )
                +( ( l_on[1] - r_on[1] )  * ( l_on[1] - r_on[1] ) ) ) ) )<= 3 )
            {
                l_on[2] = 1 ;
                r_on[2] = 1 ;
            }
            else
            {
                l_on[2] = 0 ;
                r_on[2] = 0 ;
            }
        }
        else if(l_on[2]==1 && sqrt( ( l_on[0] -  image_h-1 )  *( l_on[0] - image_h-1 )
                +( ( l_on[1] - image_xl )  * ( l_on[1] - image_xl ) ) ) <= track_width+5)l_on[2] = 1 ;
        else if(r_on[2]==1 && sqrt( ( r_on[0] -  image_h-1 )  *( r_on[0] - image_h-1 )
                +( ( r_on[1] - image_xr )  * ( r_on[1] - image_xr ) ) ) < track_width+5)r_on[2] = 1 ;
        else{
            l_on[2] = 0 ;
            r_on[2] = 0 ;
        }

    }


    else if(cross_flag==4)
    {

        if( l_on[2]==1 && l_Down[2]==1
             &&(func_abs(  track_width - (int)(sqrt( ( l_on[0] - l_Down[0] )  *( l_on[0] - l_Down[0] )
              +( ( l_on[1] - l_Down[1] )  * ( l_on[1] - l_Down[1] ) ) ) ) )<= 3))
        {
            l_on[2] = 1 ;
            l_Down[2] = 1 ;
        }
        else
        {
            l_on[2] = 0 ;
            l_Down[2] = 0 ;
        }

        if( r_on[2]==1 && r_Down[2]==1
            &&(func_abs(  track_width - (int)(sqrt( ( r_on[0] - r_Down[0] )  *( r_on[0] - r_Down[0] )
             +( ( r_on[1] - r_Down[1] )  * ( r_on[1] - r_Down[1] ) ) ) ) )<= 3) )
        {
           r_on[2] = 1 ;
           r_Down[2] = 1 ;
        }
        else
        {
           r_on[2] = 0 ;
           r_Down[2] = 0 ;
        }


    }

}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介   cross_judgment_first 正入十字判断 如果是有的话就是进入十字标志位
// 参数说明
// 返回参数
// 使用示例
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void cross_judgment_positive(void)
{
    find_cross_down_l();
    find_cross_down_r();

    if(l_Down[2] == 1 && r_Down[2] == 1)   //寻找到拐点
       {
           if(  func_abs(  track_width - (int)sqrt( ( l_Down[0] - r_Down[0] )  *( l_Down[0] - r_Down[0] )
                   +( ( l_Down[1] - r_Down[1] )  * ( l_Down[1] - r_Down[1] ) ) ) )<= 3   )//两点间距离合适

           {
               cross_flag = 1;  //正常进入
               lr_en_speed=0;   //清除编码器积分 使得进入第二阶段
           }
       }


}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介   cross_judgment_slanting   斜入十字判断 如果是有的话就是进入十字标志位
// 参数说明
// 返回参数
// 使用示例
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void cross_judgment_slanting(void)
{

    find_cross_down_l();
    find_cross_down_r();

    find_incline_cross_on_l();
    find_incline_cross_on_r();

    if(  l_on[2]==1 && l_Down[2]==1
      && func_abs(  track_width - (int)sqrt( ( l_on[0] - l_Down[0] )  *( l_on[0] - l_Down[0] )
        +( ( l_on[1] - l_Down[1] )  * ( l_on[1] - l_Down[1] ) ) ) )<= 5   )//两点间距离合适
    {
        cross_flag=4;
        lr_en_speed=0;
    }
    else if(  r_on[2]==1 && r_Down[2]==1
          && func_abs(  track_width - (int)sqrt( ( r_on[0] - r_Down[0] )  *( r_on[0] - r_Down[0] )
            +( ( r_on[1] - r_Down[1] )  * ( r_on[1] - r_Down[1] ) ) ) )<= 5 )//两点间距离合适
    {
        cross_flag=4;
        lr_en_speed=0;
    }
    else
    {
        l_on[2]=0; l_Down[2]=0;
        r_on[2]=0; r_Down[2]=0;
        cross_flag=0;
    }

}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介   cross_dispose  进入十字判断
// 参数说明
// 返回参数
// 使用示例
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void cross_dispose(void)
{

//    buzzer=10;

    if(cross_flag==1)
    {
        find_cross_down_l();
        find_cross_down_r();
        find_cross_on_l();
        find_cross_on_r();

        if(l_on[2] == 0)
        {
            find_incline_cross_on_l();
        }
        else if(r_on[2] == 0)
        {
            find_incline_cross_on_r();
        }


        cross_checkout();

        if(l_shade_255 >= 15 && r_shade_255 >= 15)
        {

            cross_flag = 2;
            find_cross_on_l();
            find_cross_on_r();
            lr_en_speed=0;
        }
        if( lr_en_speed >= 11000 )   //编码器积分强制清除
        {
            cross_flag = 2;
            find_cross_on_l();
            find_cross_on_r();
            lr_en_speed = 0;
        }
    }
    else if(cross_flag==2||cross_flag==3)
    {
        find_cross_on_l();
        find_cross_on_r();

        cross_checkout();


        if(l_shade_255 <= 5 && r_shade_255 <= 5)
        {
            cross_flag = 3;
            find_cross_on_l();
            find_cross_on_r();
        }
        if( lr_en_speed >= 6200 )
        {
            cross_flag = 0;
        }


    }


    else if(cross_flag==4) //斜入十字
    {
        find_cross_down_l();
        find_cross_down_r();

        find_incline_cross_on_l();
        find_incline_cross_on_r();

        find_cross_on_l();
        find_cross_on_r();

        cross_checkout();


        if(l_Down[2] == 1 && r_Down[2] == 1 )
        {
            cross_flag=1;
            find_cross_on_l();
            find_cross_on_r();
        }
        if( (l_shade_255 >= 20 && r_shade_255 >= 20) ||(l_on[2] == 1 && r_on[2] == 1 ))
        {
            cross_flag = 2;
            find_cross_on_l();
            find_cross_on_r();
            lr_en_speed=0;
        }
        if( lr_en_speed >= 4300 )   //编码器积分强制清除
        {
            cross_flag = 2;
            find_cross_on_l();
            find_cross_on_r();
            lr_en_speed = 0;
        }
    }




//开始十字补线-------------------------------------------------------------

    if(cross_flag == 1 )
    {
       if( l_Down[2] == 1 && l_on[2] == 1)     Stayguy_ADS(l_Down[1],l_Down[0],l_on[1],l_on[0]);
       if( r_Down[2] == 1 && r_on[2] == 1)     Stayguy_ADS(r_Down[1],r_Down[0],r_on[1],r_on[0]);

       if( l_Down[2] == 0 && l_on[2] == 1 )    Stayguy_ADS(image_xl,90,l_on[1],l_on[0]);
       if( r_Down[2] == 0 && r_on[2] == 1 )    Stayguy_ADS(image_xr,90,r_on[1],r_on[0]);
    }

    else if(cross_flag==2  ||  cross_flag == 3 )
    {
       if(l_on[2]==1)     Stayguy_ADS(image_xl,90,l_on[1],l_on[0]);
       if(r_on[2]==1)     Stayguy_ADS(image_xr,90,r_on[1],r_on[0]);
    }
    else if(cross_flag==4)
    {
        if( l_Down[2] == 1 && l_on[2] == 1)     Stayguy_ADS(l_Down[1],l_Down[0],l_on[1],l_on[0]);
        else if( r_Down[2] == 1 && r_on[2] == 1)     Stayguy_ADS(r_Down[1],r_Down[0],r_on[1],r_on[0]);

    }


}



//-------------------------------------------------------------------------------------------------------------------
// 函数简介      环岛判断
// 参数说明
// 返回参数
// 使用示例
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void find_island_on_l(void)
{
    int x[3]={0},y[3]={0};
    double angle=0;
    for( int i = 3;i < l_data_statics - 16 ; i += 2 )
    {
        x[0] = Find_Boundary_l[i][1];     //a b c 三点  求得是b的角度
        x[1] = Find_Boundary_l[i+8][1];
        x[2] = Find_Boundary_l[i+16][1];

        y[0]=Find_Boundary_l[i][0];
        y[1]=Find_Boundary_l[i+8][0];
        y[2]=Find_Boundary_l[i+16][0];

        if( x[0] > 2 && x[2] > x[0]
            && image[(y[0]+y[2])/2][(x[2]+x[0])/2] == 0)//ac连线中点一定是 黑色
        {
            angle=Angel_compute( x[0] , y[0] , x[1] , y[1] , x[2] , y[2] );

            if( angle >= 30.0 && angle <= 100.0)
            {
                is_L_on[0]=Find_Boundary_l[i+8][0];
                is_L_on[1]=Find_Boundary_l[i+8][1];
                is_L_on[2]=1;
                is_L_on[3]=i+8;
                break;
            }
        }
    }
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介      环岛判断
// 参数说明
// 返回参数
// 使用示例
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void find_island_on_r(void)
{
    int x[3]={0},y[3]={0};
    double angle=0;
    for( int i = 3;i < r_data_statics - 16 ; i += 2 )
    {
        x[0] = Find_Boundary_r[i][1];     //a b c 三点  求得是b的角度
        x[1] = Find_Boundary_r[i+8][1];
        x[2] = Find_Boundary_r[i+16][1];

        y[0]=Find_Boundary_r[i][0];
        y[1]=Find_Boundary_r[i+8][0];
        y[2]=Find_Boundary_r[i+16][0];





        if( x[0] < 67 && x[0] > x[2]
            && image[(y[0]+y[2])/2][(x[2]+x[0])/2] == 0)//ac连线中点一定是 黑色

        {

            angle=Angel_compute( x[0] , y[0] , x[1] , y[1] , x[2] , y[2] );

            if( angle >= 30.0 && angle <= 100.0)
            {

                is_R_on[0]=Find_Boundary_r[i+8][0];
                is_R_on[1]=Find_Boundary_r[i+8][1];
                is_R_on[2]=1;
                break;


            }
        }
    }
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介      环岛判断
// 参数说明
// 返回参数
// 使用示例
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void find_island_incline_on_l(void)
{
    int x[3]={0},y[3]={0};
    double angle=0;
    for( int i = 3;i < r_data_statics - 16 ; i += 2 )
    {
        x[0] = Find_Boundary_r[i][1];     //a b c 三点  求得是b的角度
        x[1] = Find_Boundary_r[i+8][1];
        x[2] = Find_Boundary_r[i+16][1];

        y[0]=Find_Boundary_r[i][0];
        y[1]=Find_Boundary_r[i+8][0];
        y[2]=Find_Boundary_r[i+16][0];


        if( x[0] < 68 && x[0] > x[2]
            && image[(y[0]+y[2])/2][(x[2]+x[0])/2] == 0)//ac连线中点一定是 黑色

        {

            angle=Angel_compute( x[0] , y[0] , x[1] , y[1] , x[2] , y[2] );



            if( angle >= 40.0 && angle <= 100.0 )
            {

                is_L_on[0]=Find_Boundary_r[i+8][0];
                is_L_on[1]=Find_Boundary_r[i+8][1];
                is_L_on[2]=1;


                break;


            }
        }
    }
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介      环岛判断
// 参数说明
// 返回参数
// 使用示例
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void find_island_incline_on_r(void)
{
    int x[3]={0},y[3]={0};
    double angle=0;
    for( int i = 3;i < l_data_statics - 16; i += 2 )
    {
        x[0] = Find_Boundary_l[i][1];     //a b c 三点  求得是b的角度
        x[1] = Find_Boundary_l[i+8][1];
        x[2] = Find_Boundary_l[i+16][1];

        y[0]=Find_Boundary_l[i][0];
        y[1]=Find_Boundary_l[i+8][0];
        y[2]=Find_Boundary_l[i+16][0];


        if( x[0] > 1 && x[2] > x[0]
            && image[(y[0]+y[2])/2][(x[2]+x[0])/2] == 0)//ac连线中点一定是 黑色

        {

            angle=Angel_compute( x[0] , y[0] , x[1] , y[1] , x[2] , y[2] );



            if( angle >= 40.0 && angle <= 100.0 )
            {

                is_R_on[0]=Find_Boundary_l[i+8][0];
                is_R_on[1]=Find_Boundary_l[i+8][1];
                is_R_on[2]=1;


                break;


            }
        }
    }
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介      环岛判断
// 参数说明
// 返回参数
// 使用示例
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void find_island_down_l(void)
{
    int x[3]={0},y[3]={0};
    double angle=0;
    for( int i = 3;i < r_data_statics -16 ; i += 2 )
    {
        x[0] = Find_Boundary_r[i][1];     //a b c 三点  求得是b的角度
        x[1] = Find_Boundary_r[i+8][1];
        x[2] = Find_Boundary_r[i+16][1];

        y[0]=Find_Boundary_r[i][0];
        y[1]=Find_Boundary_r[i+8][0];
        y[2]=Find_Boundary_r[i+16][0];


        if( x[0] < 68 && y[2] < y[0] && x[2] > x[1]
            && image[(y[0]+y[2])/2][(x[2]+x[0])/2] == 0)//ac连线中点一定是 黑色

        {

            angle=Angel_compute( x[0] , y[0] , x[1] , y[1] , x[2] , y[2] );



            if( angle >= 60.0 && angle <= 90.0 )
            {

                is_L_down[0]=Find_Boundary_r[i+8][0];
                is_L_down[1]=Find_Boundary_r[i+8][1];
                is_L_down[2]=1;


                break;


            }
        }
    }
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介      环岛判断
// 参数说明
// 返回参数
// 使用示例
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void find_island_down_r(void)
{
    int x[3]={0},y[3]={0};
    double angle=0;
    for( int i = 3;i < l_data_statics -16  ; i += 2 )
    {
        x[0] = Find_Boundary_l[i][1];     //a b c 三点  求得是b的角度
        x[1] = Find_Boundary_l[i+8][1];
        x[2] = Find_Boundary_l[i+16][1];

        y[0]=Find_Boundary_l[i][0];
        y[1]=Find_Boundary_l[i+8][0];
        y[2]=Find_Boundary_l[i+16][0];


        if( x[0] > 1
           && y[2] < y[0]
           && x[1] > x[2]
           && x[1] > x[0]
           && image[(y[0]+y[2])/2][(x[2]+x[0])/2] == 0)//ac连线中点一定是 黑色

        {

            angle=Angel_compute( x[0] , y[0] , x[1] , y[1] , x[2] , y[2] );



            if( angle >= 60 && angle <= 110 )
            {

                is_R_down[0]=Find_Boundary_l[i+8][0];
                is_R_down[1]=Find_Boundary_l[i+8][1];
                is_R_down[2]=1;


                break;


            }
        }
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      赛道宽度检测
// 参数说明      要检测的赛道行数
// 返回参数      白色点数
// 使用示例
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
uint8 track_judgment(uint8 i)
{
    uint8 track_widch=0; //赛道宽度检测所需
    for(int j= 0 ;j<68 ; j++)
    {
        if(image[i][j]==255)
        {
            track_widch++;
        }
    }
    return track_widch;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      环岛判断
// 参数说明
// 返回参数
// 使用示例
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void island_judgment(void)
{
    if(straight_r >=2 && straight_l == 0 )
    {
        find_island_on_l();
        if(is_L_on[2]==1
                && is_L_on[0] >= island_in_limit_x       //高度大于某个值才进环岛
                && is_L_on[0] <= island_in_limit_x +10
                && track_judgment(is_L_on[0]+3)>track_width+10
                                                                )    //初次寻找拐点 限制 拐点高度

        {
            L_island_flag=1;
            if(g_attitude.yaw + 70 >= 180)
            angle.yaw = g_attitude.yaw + 70 -360.0;   //入环初始角度
            else
            angle.yaw = g_attitude.yaw + 70;
        }
        else  L_island_flag=0;

    }
    else if (straight_l>=2 && straight_r == 0)
    {
        find_island_on_r();
        if(is_R_on[2]==1
                && is_R_on[0] >= island_in_limit_x
                && is_R_on[0] <= island_in_limit_x +10
                && track_judgment(is_R_on[0]+3)>track_width+10)  //初次寻找拐点 限制 拐点高度
        {
            R_island_flag=1;
            if(g_attitude.yaw - 70 <= -180)
            angle.yaw = g_attitude.yaw - 70 +360.0;   //入环初始角度
            else
            angle.yaw = g_attitude.yaw - 70;
        }
        else  R_island_flag=0;
    }
}


//-------------------------------------------------------------------------------------------------------------------
// 函数简介      环岛判断
// 参数说明
// 返回参数
// 使用示例
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
float f_abs(float x)
{
    if(x>=0) return x ;
    else return -x;
}
void island_dispose_L(void)
{
//    buzzer=10;


    if(L_island_flag == 1 )  //入环找左上拐点
    {
        find_island_on_l();

        if(is_L_on[2]==0)
        {
            find_island_incline_on_l();
        }

        if(f_abs(g_attitude.yaw - angle.yaw)<10.0)
        {
            L_island_flag = 2;
            if(g_attitude.yaw + 180 >=180)
            angle.yaw = g_attitude.yaw + 180 -360.0;   //入环初始角度
            else
            angle.yaw = g_attitude.yaw + 180;
        }
        lr_en_speed=0;

    }
    else  if(L_island_flag == 2){//环中旋转 找角度
        if(f_abs(g_attitude.yaw - angle.yaw)<10.0){
            L_island_flag = 3;
            find_island_down_l();
            if(g_attitude.yaw + 100 >=180)
            angle.yaw = g_attitude.yaw + 100 -360.0;
            else
            angle.yaw = g_attitude.yaw + 100;
        }

    }
    else if(L_island_flag == 3){ //寻找右下拐点
        find_island_down_l();
        if(f_abs(g_attitude.yaw - angle.yaw)<10.0){
            lr_en_speed = 0;
            L_island_flag = 4;
            find_island_on_l();
        }
    }
    else if( L_island_flag == 4) {//找左上出环
        find_island_on_l();
        if(lr_en_speed >= 9000
//                && lostline_flag_l == 0 && lostline_flag_r == 0
                && abs(track_judgment(55)-track_width)<=3)
        L_island_flag=0;
    }

//开始环岛补线-------------------------------------------------------------


    if(L_island_flag==1){
        if(is_L_on[2]==1 && is_L_on[0]+30 <= image_h-1){
            Stayguy_ADS(func_limit_ab(Find_Boundary_r[0][1]+3,1,68),func_limit_ab(is_L_on[0]+40,45,90),is_L_on[1] ,is_L_on[0]);
        }
        else if(is_L_on[2]==1 && is_L_on[0]+30 > image_h-1){
            Stayguy_ADS(func_limit_ab(Find_Boundary_l[0][1]+track_width,1,68),Find_Boundary_r[0][0],is_L_on[1] ,is_L_on[0]);
        }
        else{
            Stayguy_ADS(func_limit_ab(Find_Boundary_l[0][1]+track_width,1,68),Find_Boundary_l[0][1]
                        ,func_limit_ab(Find_Boundary_l[0][1]+track_width,1,68),Find_Boundary_l[0][1]-10);
        }
    }
    else if(L_island_flag==3 ){
        find_island_down_l();
        if(is_L_down[2] == 1){
            Stayguy_ADS(is_L_down[1] ,is_L_down[0],1,is_L_down[0]-8);
            lr_en_speed=0;
        }
        else{
            Stayguy_ADS(func_limit_ab(Find_Boundary_r[0][1]-2,1,68),91,1,hight_image+5);
        }
    }
    else if(L_island_flag==4 ){
        if(is_L_on[2] == 1){
            Stayguy_ADS(is_L_on[1],is_L_on[0],Find_Boundary_r[0][1]-track_width,91);
        }
        else{
            Stayguy_ADS(Find_Boundary_l[l_data_statics-10][1],Find_Boundary_l[l_data_statics-10][0],Find_Boundary_l[0][1],91);
        }
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      环岛判断
// 参数说明
// 返回参数
// 使用示例
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void island_dispose_R(void)
{
//    buzzer=10;

    if(R_island_flag == 1 )  {//入环找左上拐点
        find_island_on_r();
        if(is_R_on[2]==0){
            find_island_incline_on_r();
        }

        if(f_abs(g_attitude.yaw - angle.yaw)<10.0){
            R_island_flag = 2;
            if(g_attitude.yaw - 180 <= -180)  //环中变化角度
            angle.yaw = g_attitude.yaw - 180 +360.0;   //入环初始角度
            else
            angle.yaw = g_attitude.yaw - 180;
        }
    }
    else  if(R_island_flag == 2) { //环中旋转 找角度
        if(f_abs(g_attitude.yaw - angle.yaw)<10.0  ){// 当左上角出现 丢线
            R_island_flag = 3;
            find_island_down_r();
            if(g_attitude.yaw - 100 <= -180)
            angle.yaw = g_attitude.yaw - 100 +360.0;
            else
            angle.yaw = g_attitude.yaw - 100;
        }
    }
    else if(R_island_flag == 3)  {//寻找右下拐点
        find_island_down_r();
        if(f_abs(g_attitude.yaw - angle.yaw)<10.0){
            lr_en_speed = 0;
            R_island_flag = 4;
            find_island_on_r();
        }
    }
    else if( R_island_flag == 4){//找右上出环
        find_island_on_r();
        if(lr_en_speed >= 9000
                && lostline_flag_l == 0 && lostline_flag_r == 0)
            R_island_flag=0;
    }







//开始环岛补线-------------------------------------------------------------

    if(R_island_flag==1 ){
        if(is_R_on[2]==1 && is_R_on[0]+30 <= image_h-1){
            Stayguy_ADS(func_limit_ab(Find_Boundary_l[0][1]-3,1,68),func_limit_ab(is_R_on[0]+40,45,90),is_R_on[1] ,is_R_on[0]);
        }
        else if(is_R_on[2]==1 && is_R_on[0]+30 > image_h-1){
            Stayguy_ADS(func_limit_ab(Find_Boundary_r[0][1]-track_width,1,68),Find_Boundary_l[0][0],is_R_on[1] ,is_R_on[0]);
        }
        else{
            Stayguy_ADS(func_limit_ab(Find_Boundary_r[0][1]-track_width,1,68),Find_Boundary_r[0][1]
                        ,func_limit_ab(Find_Boundary_r[0][1]-track_width,1,68),Find_Boundary_r[0][1]-10);
        }
    }
    else if(R_island_flag==3 ){
        find_island_down_r();
        if(is_R_down[2] == 1){
            Stayguy_ADS(is_R_down[1] ,is_R_down[0],68,is_R_down[0]-8);
            lr_en_speed=0;
        }
        else{
            Stayguy_ADS(func_limit_ab(Find_Boundary_l[0][1]-2,1,68),91,68,hight_image+5);
        }

    }
    else if(R_island_flag==4 ){
        if(is_R_on[2] == 1){
            Stayguy_ADS(is_R_on[1],is_R_on[0],Find_Boundary_l[0][1]+track_width,91);
        }
        else if(straight_l >= 2){
            Stayguy_ADS(Find_Boundary_r[r_data_statics-10][1],Find_Boundary_r[r_data_statics-10][1],Find_Boundary_r[0][1],91);
        }
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      坡道检测
// 参数说明
// 返回参数
// 使用示例
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void ramp_find(void)
{
    for(uint8 i=1;i<=68;i++){
        for(uint8 j=0;j<4;j++){
            if(image[20*j+20][i]==255){
                ramp_widch[j]++;
            }
        }
    }
    if(ramp_widch[0]>ramp_widch[2]+3 && ramp_widch[1]>ramp_widch[3]+3)
        ramp_flag=1;
}



//-------------------------------------------------------------------------------------------------------------------
// 函数简介     中线数据记录与返回车身角度调整
// 参数说明     mode 返回数据的类型  0-船体与中线偏差，正车在道路左侧，负数车在道路右侧 1-车身角度偏移
// 返回参数     巡线角度及车身角度修正
// 使用示例     int angle = midline_and_anglereturn();
// 备注信息
//-------------------------------------------------------------------------------------------------------------------

float midline_and_anglereturn(uint8 mode)
{
    uint8 OFFLine=60;
    uint8 check_num=3;
    uint8 save=0;
    int point1,point2,point3;

    for(uint8 i=OFFLine;i<75;i++)
    {
        for(uint8 j=1;j<=check_num;j++)
        {
            if(midline[OFFLine]-j==0)save++;
            if(midline[OFFLine]+j==0)save++;
        }
        if(save < check_num){break;}
        else {OFFLine++;}
    }
    tft180_draw_line(0, OFFLine, 90, OFFLine, RGB565_GREEN);
    tft180_draw_line(0, 80, 90, 80, RGB565_GREEN);
    point1=midline[80];
    point2=midline[(uint8)(80+OFFLine)/2]-45 ;
    point3=midline[OFFLine];
    if(mode==0){return point2;}
    if(mode==1)
    {

         float angle = angle_compute(point3,OFFLine,point1,80);
        if(point3<=point1)angle=-angle;
        return angle;
    }
    return 999.999;

}
float angle_compute(uint x1,uint y1,uint x2,uint y2)
{
    double c,a;
    float angle;
    if(x1==x2)return 0;
    a=abs(x1-x2);
    c=pi/180;
    angle=atan2(a,(y2-y1))/c;
    return angle;
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     midline_scan()   中线扫描
// 参数说明
// 返回参数
// 使用示例
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void midline_scan(void)
{
    if(straight_l>=2 && R_island_flag != 3 && L_island_flag != 3){
        for(int i=0;i<=l_data_statics;i++){
           Find_Boundary_z[i][1]=Find_Boundary_l[i][1]+track_width/2/sin((float)curvature_l[0]/180.0*M_PI);
           Find_Boundary_z[i][0]=Find_Boundary_l[i][0];
           image[Find_Boundary_z[i][0]][Find_Boundary_z[i][1]]=0;
           midline[Find_Boundary_z[i][0]]=Find_Boundary_z[i][1];
        }
    }
    else if(straight_r>=2 && R_island_flag != 3 && L_island_flag != 3){
        for(int i=0;i<=r_data_statics;i++){
           Find_Boundary_z[i][1]=Find_Boundary_r[i][1]-track_width/2/sin((float)curvature_r[0]/180.0*M_PI);
           Find_Boundary_z[i][0]=Find_Boundary_r[i][0];
           image[Find_Boundary_z[i][0]][Find_Boundary_z[i][1]]=0;
           midline[Find_Boundary_z[i][0]]=Find_Boundary_z[i][1];
        }
    }
    else{
        for(int i=0;i<=(r_data_statics+l_data_statics)/2;i++){
           Find_Boundary_z[i][1]=(Find_Boundary_r[i][1]+Find_Boundary_l[i][1])/2;
           Find_Boundary_z[i][0]=(Find_Boundary_r[i][0]+Find_Boundary_l[i][0])/2;
           image[Find_Boundary_z[i][0]][Find_Boundary_z[i][1]]=0;
           midline[Find_Boundary_z[i][0]]=Find_Boundary_z[i][1];
        }
    }



}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      舵机中线
// 参数说明
// 返回参数
// 使用示例
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
//void centre_



void Inflexion_Point(void)  //四个拐点  显示
{

    ips200_draw_point( 100 + l_Down[1] + 0 , 150 + l_Down[0] + 0 ,RGB565_RED);
    ips200_draw_point( 100 + l_Down[1] + 0 , 150 + l_Down[0] - 1 ,RGB565_RED);
    ips200_draw_point( 100 + l_Down[1] + 0 , 150 + l_Down[0] - 2 ,RGB565_RED);
    ips200_draw_point( 100 + l_Down[1] + 1 , 150 + l_Down[0] + 0 ,RGB565_RED);
    ips200_draw_point( 100 + l_Down[1] + 1 , 150 + l_Down[0] - 1 ,RGB565_RED);
    ips200_draw_point( 100 + l_Down[1] + 2 , 150 + l_Down[0] + 0 ,RGB565_RED);
    ips200_draw_point( 100 + l_Down[1] - 1 , 150 + l_Down[0] + 1 ,RGB565_RED);

    ips200_draw_point( 100 + l_on[1] + 0 , 150 + l_on[0]+ 0 ,RGB565_YELLOW);
    ips200_draw_point( 100 + l_on[1] + 0 , 150 + l_on[0] - 1 ,RGB565_YELLOW);
    ips200_draw_point( 100 + l_on[1] + 0 , 150 + l_on[0] - 2 ,RGB565_YELLOW);
    ips200_draw_point( 100 + l_on[1] - 1 , 150 + l_on[0] + 0 ,RGB565_YELLOW);
    ips200_draw_point( 100 + l_on[1] - 1 , 150 + l_on[0] - 1 ,RGB565_YELLOW);
    ips200_draw_point( 100 + l_on[1] - 2 , 150 + l_on[0] + 0 ,RGB565_YELLOW);
    ips200_draw_point( 100 + l_on[1] + 1 , 150 + l_on[0] + 1 ,RGB565_YELLOW);

    ips200_draw_point( 100 + r_Down[1] + 0 , 150 + r_Down[0] + 0 ,RGB565_PURPLE);
    ips200_draw_point( 100 + r_Down[1] + 0 , 150 + r_Down[0] + 1 ,RGB565_PURPLE);
    ips200_draw_point( 100 + r_Down[1] + 0 , 150 + r_Down[0] + 2 ,RGB565_PURPLE);
    ips200_draw_point( 100 + r_Down[1] + 1 , 150 + r_Down[0] + 0 ,RGB565_PURPLE);
    ips200_draw_point( 100 + r_Down[1] + 1 , 150 + r_Down[0] + 1 ,RGB565_PURPLE);
    ips200_draw_point( 100 + r_Down[1] + 2 , 150 + r_Down[0] + 0 ,RGB565_PURPLE);
    ips200_draw_point( 100 + r_Down[1] - 1 , 150 + r_Down[0] - 1 ,RGB565_PURPLE);

    ips200_draw_point( 100 + r_on[1] + 0 , 150 + r_on[0] + 0 ,RGB565_CYAN);
    ips200_draw_point( 100 + r_on[1] + 0 , 150 + r_on[0] + 1 ,RGB565_CYAN);
    ips200_draw_point( 100 + r_on[1] + 0 , 150 + r_on[0] + 2 ,RGB565_CYAN);
    ips200_draw_point( 100 + r_on[1] - 1 , 150 + r_on[0] + 0 ,RGB565_CYAN);
    ips200_draw_point( 100 + r_on[1] - 1 , 150 + r_on[0] + 1 ,RGB565_CYAN);
    ips200_draw_point( 100 + r_on[1] - 2 , 150 + r_on[0] + 0 ,RGB565_CYAN);
    ips200_draw_point( 100 + r_on[1] + 1 , 150 + r_on[0] - 1 ,RGB565_CYAN);
}

void xianshi_a(void)
{
    for(int i=0 ;i < l_data_statics ; i++)
       {

         if(Find_Boundary_l[i][1] == Find_Boundary_l[i+1][1] && Find_Boundary_l[i][0] == Find_Boundary_l[i+1][0])
        {
            break;
        }

         tft180_draw_point( Find_Boundary_l[i][1] +2, Find_Boundary_l[i][0] ,RGB565_RED);
         tft180_draw_point( Find_Boundary_l[i][1] + 3, Find_Boundary_l[i][0] ,RGB565_RED);
         tft180_draw_point( Find_Boundary_l[i][1] + 1, Find_Boundary_l[i][0] ,RGB565_RED);
         tft180_draw_point( Find_Boundary_l[i][1]  +4, Find_Boundary_l[i][0]  ,RGB565_RED);


       }
       for(int i=0 ; i < r_data_statics ; i++)
       {
         if(Find_Boundary_r[i][1] == Find_Boundary_r[i-1][1] && Find_Boundary_r[i][0] == Find_Boundary_r[i-1][0])
         {
             break;
         }
         tft180_draw_point( Find_Boundary_r[i][1] -2 , Find_Boundary_r[i][0]  ,RGB565_BLUE);
         tft180_draw_point( Find_Boundary_r[i][1] -3, Find_Boundary_r[i][0],RGB565_BLUE);
         tft180_draw_point( Find_Boundary_r[i][1] -4, Find_Boundary_r[i][0],RGB565_BLUE);
         tft180_draw_point( Find_Boundary_r[i][1] -1, Find_Boundary_r[i][0] ,RGB565_BLUE);




       }

       for(int i=0;i<=(r_data_statics+l_data_statics)/2;i++)
       {

           tft180_draw_point( Find_Boundary_z[i][1]  , Find_Boundary_z[i][0]  ,RGB565_PURPLE);
           tft180_draw_point( Find_Boundary_z[i][1]+1 , Find_Boundary_z[i][0]  + 1,RGB565_PURPLE);
       }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介    Camera_tracking()   图像处理总函数
// 参数说明
// 返回参数
// 使用示例
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void Camera_tracking(void)
{

    empty_flag(); //标志位清除
    Binaryzation(); //大津法求阈值
    image_draw();  //二值化 画框 去噪点
    shade_compute();//看左下角阴影
    search((uint16)USE_num,image,&hight_image);
//判断本帧是否有问题 如果有问题 就不进行元素判断
    if(image_ok == 1){
//直道 弯道 判断-----------------------------
        curvature_judge();
//        curvature_l_l=calculateAverageCurvature(Find_Boundary_l,l_data_statics,20,10);
//左右丢线判断-----------------------------
        lost_line_judge();
//元素判断-----------------------------------
        if(zebra_crossing_flag==0 && L_island_flag==0 && R_island_flag == 0 && ramp_flag==0 ){
            if(cross_flag == 0 && lostline_flag_l == 1 && lostline_flag_r == 1){//正入十字
                cross_judgment_positive();
            }
            if(cross_flag == 0 && lostline_flag_l == 1 && lostline_flag_r == 1){//斜入十字
                cross_judgment_slanting();
            }
            if(cross_flag !=0){ //这个标志位代表十字不同的过程
                cross_dispose();
            }
        }

        if(cross_flag == 0 && zebra_crossing_flag==0 && ramp_flag==0) {
            if(L_island_flag == 0 && R_island_flag == 0
                    && ((lostline_flag_l == 1 && lostline_flag_r == 0) || (lostline_flag_l == 0 && lostline_flag_r == 1))){
                island_judgment();
            }
            if(L_island_flag != 0 && R_island_flag == 0){
                island_dispose_L();
            }
            else if(L_island_flag == 0 && R_island_flag != 0 ){
                island_dispose_R();
            }
        }

    //   if(cross_flag == 0 && zebra_crossing_flag==0
    //           && L_island_flag == 0 && R_island_flag == 0 )
    //   {
    //       if(ramp_flag==0)
    //       ramp_find();
    //       if(ramp_flag!=0)
    //           ;
    //   }



        search((uint16)USE_num, image,&hight_image);

        curvature_judge();
        midline_scan();


        uint16 a=0;
        for(int i=14;i<24;i++){
            a += Find_Boundary_z[i][1];
        }
        centre=a;
        for(int i=24;i<40;i++){
            a += Find_Boundary_z[i][1];
        }
        centre_s=a;
    }
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     定时器获取电机速度（左侧占用定时器5，右侧占用定时器4）
// 参数说明
// 返回参数
// 使用示例
// 备注信息
//-------------------------------------------------------------------------------------------------------------------

void pit_dispose(void)
{
    static int8 Time_100ms = 0,Time_10ms=1 ;
    static int8 time_5s=0,time_number=0;
    Time_100ms++;


    Motor_pid_r.Kp=Motor_pid_l.Kp;
    Motor_pid_r.Ki=Motor_pid_l.Ki;
    speed_r=speed_l;

    l_en_speed=-encoder_get_count(TIM5_ENCOEDER);
    l_en_speed_last=l_en_speed;
    r_en_speed=encoder_get_count(TIM4_ENCOEDER);
    r_en_speed_last=r_en_speed;

    lr_en_speed+=(l_en_speed+r_en_speed)/2;
    encoder_clear_count(TIM5_ENCOEDER);
    encoder_clear_count(TIM4_ENCOEDER);

     if((run_flag==1 && out_flag==0) || check_flag == 1){
         Time_10ms++;
         time_number++;
         if(time_number>=100){
             time_5s++;
             time_number=0;
         }

         if(Time_10ms>=2){

             if(Servos_out>600.0)Servos_out=600.0;
             if(Servos_out<-600.0)Servos_out=-600.0;
             centre_last=centre;

            Time_10ms=0;
         }



//         Pid_Position_increase(speed_l*(1-speed_change)
//                 +speed_increase*speed_flag*speed_in_flag,
//                 l_en_speed*0.85+l_en_speed_last*0.15,&Motor_pid_l);
//
//         Pid_Position_increase(speed_r*(1+speed_change)
//                 +speed_increase*speed_flag*speed_in_flag,
//                 r_en_speed*0.85+r_en_speed_last*0.15,&Motor_pid_r);

//          if(speed_change>=0){
//              Pid_Position_increase(speed_l*(1-speed_change)
//                      +speed_increase*speed_flag*speed_in_flag,
//                      l_en_speed*0.85+l_en_speed_last*0.15,&Motor_pid_l);
//
//              Pid_Position_increase(speed_r
//                      +speed_increase*speed_flag*speed_in_flag,
//                      r_en_speed*0.85+r_en_speed_last*0.15,&Motor_pid_r);
//          }
//          else{
//              Pid_Position_increase(speed_l
//                      +speed_increase*speed_flag*speed_in_flag,
//                      l_en_speed*0.85+l_en_speed_last*0.15,&Motor_pid_l);
//
//              Pid_Position_increase(speed_r*(1+speed_change)
//                      +speed_increase*speed_flag*speed_in_flag,
//                      r_en_speed*0.85+r_en_speed_last*0.15,&Motor_pid_r);
//          }



         if(check_flag == 0 && time_5s >=5 && ((Motor_pid_r.Out>=4000
                 && r_en_speed <= speed_r*0.3)
                 || (Motor_pid_l.Out>=4000 && l_en_speed <= speed_l*0.3))){// 堵转保护
             out_flag=1;
             run_flag=0;
         }
//         if(check_flag == 0)
//         Motor_Control((int16)Motor_pid_l.Out,(int16)Motor_pid_r.Out);
//     }
//     else if(run_flag==0)
//     {
//        Motor_Control(0,0);
//        if(out_flag==1){
//            Motor_Control((int16)Pid_Position_increase(0,l_en_speed*0.85+l_en_speed_last*0.15,&Motor_pid_l)
//                           ,(int16)Pid_Position_increase(0,r_en_speed*0.85+r_en_speed_last*0.15,&Motor_pid_r));
//        }
     }
//     if(buzzer!=0){
//         gpio_set_level(P33_10, 0);
//         buzzer--;
//         if( buzzer % 2 == 0)   gpio_set_level(P33_10, 1);
//     }
//     else if(buzzer==0)         gpio_set_level(P33_10, 0);   //蜂鸣器 停止鸣响
}
