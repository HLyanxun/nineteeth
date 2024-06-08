/*
 * revision_mt9v03x.c
 *
 *  Created on: 2024年4月28日
 *      Author: pc
 */
#include "zf_common_headfile.h"
//#include "revision_mt9v03x.h"

uint8* Image_Use[LCDH][LCDW];      //用来存储压缩之后灰度图像的二维数组
uint8 ex_mt9v03x_binarizeImage[LCDH][LCDW];          //图像处理时真正处理的二值化图像数组
uint8 Threshold;                                //通过大津法计算出来的二值化阈值
int ImageScanInterval=5;                        //扫边的范围
static uint8* PicTemp;                          //一个保存单行图像的指针变量
static int IntervalLow = 0, IntervalHigh = 0;   //扫描区间的上下限变量
static int Ysite = 0, Xsite = 0;                //Ysite就是图像的行，Xsite就是图像的列。
static int BottomBorderRight = 79,              //59行的右边界
BottomBorderLeft = 0,                           //59行的左边界
BottomCenter = 0;                               //59行的中点
uint8 ExtenLFlag = 0;                           //左边线是否需要补线的标志变量
uint8 ExtenRFlag = 0;                           //右边线是否需要补线的标志变量
uint8 Ring_Help_Flag = 0;                       //进环辅助标志
int Right_RingsFlag_Point1_Ysite, Right_RingsFlag_Point2_Ysite; //右圆环判断的两点纵坐标
int Left_RingsFlag_Point1_Ysite, Left_RingsFlag_Point2_Ysite;   //左圆环判断的两点纵坐标
int Point_Xsite,Point_Ysite;                   //拐点横纵坐标
int Repair_Point_Xsite,Repair_Point_Ysite;     //补线点横纵坐标
int Point_Ysite1;                               //判断大小圆环时用的纵坐标
int Black;                                      //判断大小圆环时用的黑点数量
int Less_Big_Small_Num;                         //判断大小圆环时用的丢线
int left_difference_num;                        //十字左边线与标准中线39作差的和（40-20行）
int right_difference_num;                       //十字右边线与标准中线39作差的和（40-20行）
uint8 Garage_Location_Flag = 0;                 //判断库的次数
float Big_Small_Help_Gradient;               //大小圆环的辅助判断斜率
static int ytemp = 0;                           //存放行的临时变量
static int TFSite = 0, left_FTSite = 0,right_FTSite = 0;              //补线计算斜率的时候需要用的存放行的变量。
static float DetR = 0, DetL = 0;                //存放补线斜率的变量
Sideline_status Sideline_status_array[60];             //记录单行信息的结构体数组
Sideline_status ImageDeal1[80];             //记录单列信息的结构体数组
Image_Status imagestatus;                 //图像处理的的全局变量
ImageFlagtypedef imageflag;
uint64 Gray_Value=0;

//uint8 All_Sobel_Image[LCDH][LCDW];
//uint16 threshold1,threshold2,threshold3,block_yuzhi=60;
//uint16 yuzhi1,yuzhi2,yuzhi3;
//uint16 Ramp_cancel,circle_stop,block_num,duan_num;

float Mh = MT9V03X_H;
float Lh = LCDH;
float Mw = MT9V03X_W;
float Lw = LCDW;

float variance, variance_acc;                   //判断直道计算的方差
int variance_limit_long,variance_limit_short;   //长直道、短直道的方差限定值
#define Middle_Center 39                        //屏幕中心

//int j=5;
uint8 Half_Road_Wide[60] =                      //直到赛道半宽
{  4, 5, 5, 6, 6, 6, 7, 7, 8, 8,
   9, 9,10,10,10,11,12,12,13,13,
  13,14,14,15,15,16,16,17,17,17,
  18,18,19,19,20,20,20,21,21,22,
  23,23,23,24,24,25,25,25,26,26,
  27,28,28,28,29,30,31,31,31,32,
};
uint8 Half_Bend_Wide[60] =                      //弯道赛道半宽
{   33,33,33,33,33,33,33,33,33,33,
    33,33,32,32,30,30,29,29,28,27,
    28,27,27,26,26,25,25,24,24,23,
    22,21,21,22,22,22,23,24,24,24,
    25,25,25,26,26,26,27,27,28,28,
    28,29,29,30,30,31,31,32,32,33,
};

/*****************直线判断******************/
//--------------------------------------------------------------------------------------------------------
// 函数简介     计算范围内边线的斜率，对是否为直线进行判断
// 参数说明            dir      判断模式1为检测左边线，2为检测右边线
//          start    检测开始行（从上向下）
//          end      检测结束行（从上向下）
// 使用实例       float a=Straight_Judge(1,imagestatus.Offline,50);
// 备注信息      计算起始点与结束点的链接线段的斜率，判断数组结构的方差，当方差小于1的时候说明线段拟合效果较好，可以认为是一条直线
//--------------------------------------------------------------------------------------------------------
float Straight_Judge(uint8 dir, uint8 start, uint8 end)     //返回结果小于1即为直线
{
    int i;
    float S = 0, Sum = 0, Err = 0, k = 0;
    switch (dir)
    {
    case 1:k = (float)(Sideline_status_array[start].leftline - Sideline_status_array[end].leftline) / (start - end);
        for (i = 0; i < end - start; i++)
        {
            Err = (Sideline_status_array[start].leftline + k * i - Sideline_status_array[i + start].leftline) * (Sideline_status_array[start].leftline + k * i - Sideline_status_array[i + start].leftline);
            Sum += Err;
        }
        S = Sum / (end - start);
        break;
    case 2:k = (float)(Sideline_status_array[start].rightline - Sideline_status_array[end].rightline) / (start - end);
        for (i = 0; i < end - start; i++)
        {
            Err = (Sideline_status_array[start].rightline + k * i - Sideline_status_array[i + start].rightline) * (Sideline_status_array[start].rightline + k * i - Sideline_status_array[i + start].rightline);
            Sum += Err;
        }
        S = Sum / (end - start);
        break;
    }
    return S;
}

void Straight_long_judge(void)     //返回结果小于1即为直线
{
    if( imageflag.Bend_Road ||  imageflag.Zebra_Flag || imageflag.Out_Road == 1 || imageflag.RoadBlock_Flag == 1
            || imageflag.image_element_rings == 1  || imageflag.image_element_rings == 2 ) return;
    if((Straight_Judge(1,10,50) < 1) && (Straight_Judge(2,10,50) < 1) && imagestatus.OFFLine < 3 && imagestatus.Miss_Left_lines < 2 && imagestatus.Miss_Right_lines < 2)
    {
        imageflag.straight_long = 1;
        //Stop=1;
//        Statu = Straight_long;
        //gpio_set_level(B0, 1);
    }
}

void Straight_long_handle(void)     //返回结果小于1即为直线
{

    if(imageflag.straight_long)
    {
        if((Straight_Judge(1,10,50) > 1) || (Straight_Judge(2,10,50) > 1) || (imagestatus.OFFLine >= 3)||imagestatus.Miss_Left_lines >= 2||imagestatus.Miss_Right_lines>=2)
        {
            imageflag.straight_long = 0;
            //gpio_set_level(B0, 0);
        }
    }
}
//用于变参数的直道检测
float sum1=0,sum2=0;
void Straight_xie_judge(void)
{
    float S = 0, Sum = 0, Err = 0 , midd_k=0 ;
    int i;
    if (imageflag.Zebra_Flag != 0 || imageflag.image_element_rings != 0 || imageflag.Ramp == 1  )
        return;

    imageflag.straight_xie = 0;

    midd_k = (float)(Sideline_status_array[55].midline - Sideline_status_array[imagestatus.OFFLine + 1].midline) / (55 - imagestatus.OFFLine - 1);
    for (i = 0 ; i < 55 - imagestatus.OFFLine - 1; i++)
    {
        Err = (Sideline_status_array[imagestatus.OFFLine + 1].midline + midd_k * i - Sideline_status_array[i + imagestatus.OFFLine + 1].midline) * (Sideline_status_array[imagestatus.OFFLine + 1].midline + midd_k * i - Sideline_status_array[i + imagestatus.OFFLine + 1].midline);
        Sum += Err;
    }
    S = Sum / (55 - imagestatus.OFFLine - 1);
//    ips200_show_float(160,200,midd_k,3,2);
//    ips200_show_float(160,240,S,3,2);
    if (S < 1 && imagestatus.OFFLine < 10 && (imagestatus.Miss_Left_lines > 30 || imagestatus.Miss_Right_lines > 30))
    {
        imageflag.straight_xie = 1;
//        Statu = Straight_xie;
        //gpio_set_level(B0, 1);
    }

}


/*****************冲出赛道判断******************/
uint32 break_road(uint8 line_start)//冲出赛道
{
    short i,j;
    int bai=0;
    for(i=58;i>line_start;i--)
    {
        for(j=5;j<74;j++)
        {
            if(ex_mt9v03x_binarizeImage[i][j] != 0)
            {
                bai++;
            }
        }
    }
    return bai;
}
uint32 white_point(uint8 line_end,uint8 line_start) //白点数量
{
    short i,j;
    int bai=0;
    for(i=line_end;i>line_start;i--)
    {
        for(j=29;j<49;j++)
        {
            if(ex_mt9v03x_binarizeImage[i][j]!=0)
            {
                bai++;
            }
        }
    }
    return bai;
}
uint32 black_point(uint8 line_end,uint8 line_start) //黑点数量
{
    short i,j;
    int hei=0;
    for(i=line_end;i>line_start;i--)
    {
        for(j=29;j<49;j++)
        {
            if(ex_mt9v03x_binarizeImage[i][j]==0)
            {
                hei++;
            }
        }
    }
    return hei;
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//  @name           Image_CompressInit
//  @brief          原始灰度图像压缩处理 压缩初始化
//  @brief          作用就是将原始尺寸的灰度图像压缩成你所需要的大小，这里我是把原始80行170列的灰度图像压缩成60行80列的灰度图像。
//  @brief          为什么要压缩？因为我不需要那么多的信息，60*80图像所展示的信息原则上已经足够完成比赛任务了，当然你可以根据自己的理解修改。
//  @parameter      void
//  @return         void
//  @time           2022年2月18日
//  @Author
//  Sample usage:   Image_CompressInit();
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
void Image_CompressInit(void)
{
  int i, j, row, line;
  const float div_h = Mh / Lh, div_w = Mw / Lw;         //根据原始的图像尺寸和你所需要的图像尺寸确定好压缩比例。
  for (i = 0; i < LCDH; i++)                            //遍历图像的每一行，从第零行到第59行。
  {
    row = i * div_h + 0.5;
    for (j = 0; j < LCDW; j++)                          //遍历图像的每一列，从第零列到第79列。
    {
      line = j * div_w + 0.5;
      Image_Use[i][j] = &mt9v03x_image[row][line];       //mt9v03x_image数组里面是原始灰度图像，Image_Use数组存储的是我之后要拿去处理的图像，但依然是灰度图像哦！只是压缩了一下而已。
    }
  }
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
uint8 get_Threshold(uint8* ex_mt9v03x_binarizeImage[][LCDW],uint16 col, uint16 row)
{
  #define GrayScale 256
  uint16 width = col;
  uint16 height = row;
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
      pixelCount[*ex_mt9v03x_binarizeImage[i][j]]++;       //将当前的像素点的像素值（灰度值）作为计数数组的下标。
      gray_sum += *ex_mt9v03x_binarizeImage[i][j];         //计算整幅灰度图像的灰度值总和。
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

//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//  @name           Get_BinaryImage
//  @brief          灰度图像二值化处理
//  @brief          整体思路就是：先调用Get_Threshold（）函数得到阈值，然后遍历原始灰度图像的每一个像素点，用每一个像素点的灰度值来跟阈值计较。
//  @brief          大于阈值的你就把它那个像素点的值赋值为1（记为白点），否则就赋值为0（记为黑点）。当然你可以把这个赋值反过来，只要你自己清楚1和0谁代表黑谁代表白就行。
//  @brief          所以我前面提到的60*80现在你们就应该明白是什么意思了吧！就是像素点嘛，一行有80个像素点，一共60行，也就是压缩后的每一幅图像有4800个像素点。
//  @parameter      void
//  @return         void
//  @time           2022年2月17日
//  @Author
//  Sample usage:   Get_BinaryImage();
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
int8 threshold_fix;
void Get_BinaryImage(void)
{
//    if(imageflag.Out_Road == 1)
//    {
//        if(duan_num==0)
//        {
//            Threshold  = block_yuzhi ;
//        }
//        else if(duan_num==1)
//        {
//            Threshold  = threshold1 ;
//        }
//        else if(duan_num==2)
//        {
//            Threshold  = threshold2 ;
//        }
//        else if(duan_num==3)
//        {
//            Threshold  = threshold3 ;
//        }
//    }
//    else if(imageflag.RoadBlock_Flag == 1)
//    {
//        if(block_num==1)
//        {
//            Threshold  = yuzhi1 ;
//        }
//        else if(block_num==2)
//        {
//            Threshold  = yuzhi2 ;
//        }
//        else if(block_num==3)
//        {
//            Threshold  = yuzhi3 ;
//        }
//    }
//    else {
//        Threshold = get_Threshold(Image_Use, LCDW, LCDH);      //这里是一个函数调用，通过该函数可以计算出一个效果很不错的二值化阈值。
//    }
    Threshold = get_Threshold(Image_Use, LCDW, LCDH)+threshold_fix;      //这里是一个函数调用，通过该函数可以计算出一个效果很不错的二值化阈值。

  uint8 i, j = 0;
  for (i = 0; i < LCDH; i++)                                //遍历二维数组的每一行
  {
    for (j = 0; j < LCDW; j++)                              //遍历二维数组的每一列
    {
      if (*Image_Use[i][j] > Threshold)                      //如果这个点的灰度值大于阈值Threshold
          ex_mt9v03x_binarizeImage[i][j] = 255;                                  //那么这个像素点就记为白点
      else                                                  //如果这个点的灰度值小于阈值Threshold
          ex_mt9v03x_binarizeImage[i][j] = 0;                                  //那么这个像素点就记为黑点
    }
  }
}


//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//  @name           Get_Border_And_SideType
//  @brief          得到边线和边线的类型，我在这里给边线分为了三种类型，T类型、W类型和H类型。分别代表正常边线、无边边线和大跳变边线。
//  @brief          和上面一样，要想看懂我的逻辑，前提是要搞懂T、W、H三种类型的边线到底是什么样子的。
//  @parameter      p        指向传进来数组的一个指针变量。
//  @parameter      type     只能是L或者是R，分别代表扫左边线和扫右边线。
//  @parameter      L        扫描的区间下限 ，也就是从哪一列开始扫。
//  @parameter      H        扫描的区间上限 ，也就是一直扫到哪一列。
//  @parameter      Q        是一个结构体指针变量，自己跳过去看看这个结构体里面的成员。
//  @time           2022年2月20日
//  @Author
//  Sample usage:   Get_SideType_And_Border(PicTemp, 'R', IntervalLow, IntervalHigh,&JumpPoint[1]);
//  Sample usage:   从PicTemp(PicTemp是个指针，指向一个数组)的IntervalLow列开始扫，扫到IntervalHigh列，然后把得到的边线所在的列和边线类型记录到JumpPoint结构体中。
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
void Get_Border_And_SideType(uint8* p,uint8 type,int L,int H,JumpPointtypedef* Q)
{
  int i = 0;
  if (type == 'L')                              //如果Type是L(Left),则扫描左边线。
  {
    for (i = H; i >= L; i--)                    //从右往左扫
    {
      if (*(p + i) != 0 && *(p + i - 1) == 0)   //如果有黑白跳变    1是白 0是黑
      {
        Q->point = i;                           //那就把这个列记录下来作为左边线
        Q->type = 'T';                          //并且把这一行当作是正常跳变，边线类型记为T，即边线正常。
        break;                                  //找到了就跳出循环不找了
      }
      else if (i == (L + 1))                    //要是扫到最后都没找到
      {
        if (*(p + (L + H) / 2) != 0)            //并且扫描区间的中间是白像素点
        {
          Q->point = (L + H) / 2;               //那么就认为这一行的左边线是传进来扫描区间的中点。
          Q->type = 'W';                        //并且把这一行当作是非正常跳变，边线类型记为W，即无边行。
          break;                                //跳出循环不找了
        }
        else                                    //要是扫到最后都没找到，并且扫描区间的中间是黑像素点
        {
          Q->point = H;                         //那么就认为这一行的左边线是传进来扫描区间的区间上限。
          Q->type = 'H';                        //并且也把这一行当作是非正常跳变，不过边线类型记为H，即大跳变行。
          break;                                //跳出循环不找了
        }
      }
    }
  }
  else if (type == 'R')                         //如果Type是R(Right),则扫描右边线。
  {
    for (i = L; i <= H; i++)                    //从左往右扫
    {
      if (*(p + i) != 0 && *(p + i + 1) == 0)   //如果有黑白跳变    1是白 0是黑
      {
        Q->point = i;                           //那就把这个列记录下来作为右边线
        Q->type = 'T';                          //并且把这一行当作是正常跳变，边线类型记为T，即边线正常。
        break;                                  //找到了就跳出循环不找了
      }
      else if (i == (H - 1))                    //要是扫到最后都没找到
      {
        if (*(p + (L + H) / 2) != 0)            //并且扫描区间的中间是白像素点
        {
          Q->point = (L + H) / 2;               //那么就认为这一行的右边线是传进来扫描区间的中点。
          Q->type = 'W';                        //并且把这一行当作是非正常跳变，边线类型记为W，即无边行。
          break;
        }
        else                                    //要是扫到最后都没找到，并且扫描区间的中间是黑像素点
        {
          Q->point = L;                         //那么就认为这一行的右边线是传进来扫描区间的区间下限。
          Q->type = 'H';                        //并且也把这一行当作是非正常跳变，不过边线类型记为H，即大跳变行。
          break;                                //跳出循环不找了
        }
      }
    }
  }
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//  @name           Get_BaseLine
//  @brief          用遍历的方法得到图像最底下五行（59-55行）的边线和中线信息。这五行边线和中线信息的准确度非常的重要，直接影响到整幅图像的处理结果。
//  @brief          Get_BaseLine函数是下面Get_AllLine函数的基础和前提，Get_AllLine函数是以Get_BaseLine为基础的。看名字应该也能看出对吧，一个叫得到基础线，一个叫得到所有线。
//  @brief          Get_BaseLine函数和Get_AllLine函数加在一起，也就组成了我优化之后的搜边线算法。
//  @parameter      void
//  @time           2022年2月21日
//  @Author
//  Sample usage:   Get_BaseLine();
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
void Get_BaseLine(void)
{
    /**************************************遍历搜索图像最底行（59行）左右边线从而确定中线的过程 ********************************************************************/
    /****************************************************Begin*****************************************************************************/

    PicTemp = ex_mt9v03x_binarizeImage[59];                                                //让PicTemp这个指针变量指向图像数组的Pixle[59]
    for (Xsite = ImageSensorMid; Xsite < 79; Xsite++)                   //假设39是中心列，从中心列开始一列一列的往右边搜索右边线
    {
      if (*(PicTemp + Xsite) == 0 && *(PicTemp + Xsite + 1) == 0)       //如果连续出现了两个黑点，说没找到了边线。
      {
        BottomBorderRight = Xsite;                                      //把这一列记录下来作为这一行的右边线
        break;                                                          //跳出循环
      }
      else if (Xsite == 78)                                             //如果找到了第58列都还没出现黑点，说明这一行的边线有问题。
      {
        BottomBorderRight = 79;                                         //所以我这里的处理就是，直接假设图像最右边的那一列（第79列）就是这一行的右边线。
        break;                                                          //跳出循环
      }
    }

    for (Xsite = ImageSensorMid; Xsite > 0; Xsite--)                    //假设39是中心列，从中心列开始一列一列的往左边搜索左边线
    {
      if (*(PicTemp + Xsite) == 0 && *(PicTemp + Xsite - 1) == 0)       //如果连续出现了两个黑点，说没找到了边线。
      {
        BottomBorderLeft = Xsite;                                       //把这一列记录下来作为这一行的左边线
        break;                                                          //跳出循环
      }
      else if (Xsite == 1)                                              //如果找到了第1列都还没出现黑点，说明这一行的边线有问题。
      {
        BottomBorderLeft = 0;                                           //所以我这里的处理就是，直接假设图像最左边的那一列（第0列）就是这一行的左边线。
        break;                                                          //跳出循环
      }
    }

    BottomCenter =(BottomBorderLeft + BottomBorderRight) / 2;           //根据左右边界计算出第59行的中线
    Sideline_status_array[59].leftline = BottomBorderLeft;                        //把第59行的左边界存储进数组，注意看ImageDeal这个数字的下标，是不是正好对应59。
    Sideline_status_array[59].rightline = BottomBorderRight;                      //把第59行的右边界存储进数组，注意看ImageDeal这个数字的下标，是不是正好对应59。
    Sideline_status_array[59].midline = BottomCenter;                                //把第59行的中线存储进数组，    注意看ImageDeal这个数字的下标，是不是正好对应59。
    Sideline_status_array[59].wide = BottomBorderRight - BottomBorderLeft;          //把第59行的赛道宽度存储数组，注意看ImageDeal这个数字的下标，是不是正好对应59。
    Sideline_status_array[59].IsLeftFind = 'T';                                     //记录第59行的左边线类型为T，即正常找到左边线。
    Sideline_status_array[59].IsRightFind = 'T';                                    //记录第59行的右边线类型为T，即正常找到右边线。

    /****************************************************End*******************************************************************************/
    /**************************************遍历搜索图像最底行（59行）左右边线从而确定中线的过程 ********************************************************************/



    /**************************************在第59行中线已经确定的情况下确定58-54这四行中线的过程 ******************************************/
    /****************************************************Begin*****************************************************************************/
    /*
         * 下面几行的的搜线过程我就不再赘述了，根据我的注释把第59行的搜线过程理解好，
         * 那么58到54行的搜线就完全没问题，是一模一样的逻辑和过程。
     */
    for (Ysite = 58; Ysite > 54; Ysite--)
    {
        PicTemp = ex_mt9v03x_binarizeImage[Ysite];
        for(Xsite = Sideline_status_array[Ysite + 1].midline; Xsite < 79;Xsite++)
        {
          if(*(PicTemp + Xsite) == 0 && *(PicTemp + Xsite + 1) == 0)
          {
            Sideline_status_array[Ysite].rightline = Xsite;
            break;
          }
          else if (Xsite == 78)
          {
            Sideline_status_array[Ysite].rightline = 79;
            break;
          }
        }

        for (Xsite = Sideline_status_array[Ysite + 1].midline; Xsite > 0;Xsite--)
        {
          if (*(PicTemp + Xsite) == 0 && *(PicTemp + Xsite - 1) == 0)
          {
            Sideline_status_array[Ysite].leftline = Xsite;
            break;
          }
          else if (Xsite == 1)
          {
            Sideline_status_array[Ysite].leftline = 0;
            break;
          }
        }

        Sideline_status_array[Ysite].IsLeftFind  = 'T';
        Sideline_status_array[Ysite].IsRightFind = 'T';
        Sideline_status_array[Ysite].midline =(Sideline_status_array[Ysite].rightline + Sideline_status_array[Ysite].leftline)/2;
        Sideline_status_array[Ysite].wide   = Sideline_status_array[Ysite].rightline - Sideline_status_array[Ysite].leftline;
    }

    /****************************************************End*****************************************************************************/
    /**************************************在第59行中线已经确定的情况下确定58-54这四行中线的过程 ****************************************/
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//  @name           Get_AllLine
//  @brief          在Get_BaseLine的基础上，针对部分特殊情况，利用一些特殊的处理算法得到剩余行的边线和中线信息。
//  @brief          这个函数应该是目前为止我代码里面最难理解的一部分了，一定要花大量时间静下心来去看，看的时候脑子里面要有那个二值化黑白图像。
//  @brief          如果你能一边想着二值化黑白图像，一边来思考我代码的逻辑的话，很多地方你就能慢慢理解了，不要光盯着那个代码一直看啊，那样是没用的，切忌！
//  @brief          多动脑思考，我能想出来，你们肯定也可以的。这个过程会很枯燥，但是你都理解清楚了之后，你的车车就已经可以跑直道和弯道了。
//  @parameter      void
//  @time           2023年2月21日
//  @Author
//  Sample usage:   Get_AllLine();
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
void Get_AllLine(void)
{
  uint8 L_Found_T  = 'F';    //确定无边斜率的基准有边行是否被找到的标志
  uint8 Get_L_line = 'F';    //找到这一帧图像的基准左斜率，为什么这里要置为F，看了下面的代码就知道了。
  uint8 R_Found_T  = 'F';    //确定无边斜率的基准有边行是否被找到的标志
  uint8 Get_R_line = 'F';    //找到这一帧图像的基准右斜率，为什么这里要置为F，看了下面的代码就知道了。
  float D_L = 0;             //左边线延长线的斜率
  float D_R = 0;             //右边线延长线的斜率
  int ytemp_W_L;             //记住首次左丢边行
  int ytemp_W_R;             //记住首次右丢边行
  ExtenRFlag = 0;            //标志位清0
  ExtenLFlag = 0;            //标志位清0
  imagestatus.OFFLine=2;     //这个结构体成员我之所以在这里赋值，是因为我ImageStatus结构体里面的成员太多了，但是暂时又只用到了OFFLine，所以我在哪用到它就在哪赋值。
  imagestatus.Miss_Right_lines = 0;
  imagestatus.WhiteLine = 0;
  imagestatus.Miss_Left_lines = 0;
  for (Ysite = 54 ; Ysite > imagestatus.OFFLine; Ysite--)                            //前5行在Get_BaseLine()中已经处理过了，现在从55行处理到自己设定的不处理行OFFLine。
  {                                                                                  //因为太前面的图像可靠性不搞，所以OFFLine的设置很有必要，没必要一直往上扫到第0行。
    PicTemp = ex_mt9v03x_binarizeImage[Ysite];
    JumpPointtypedef JumpPoint[2];                                                   // JumpPoint[0]代表左边线，JumpPoint[1]代表右边线。

  /******************************扫描本行的右边线******************************/
    IntervalLow  = Sideline_status_array[Ysite + 1].rightline  - ImageScanInterval;               //从上一行的右边线加减Interval对应的列开始扫描本行，Interval一般取5，当然你为了保险起见可以把这个值改的大一点。
    IntervalHigh = Sideline_status_array[Ysite + 1].rightline + ImageScanInterval;              //正常情况下只需要在上行边线左右5的基础上（差不多10列的这个区间）去扫线，一般就能找到本行的边线了，所以这个值其实不用太大。
    LimitL(IntervalLow);                                                             //这里就是对传给GetJumpPointFromDet()函数的扫描区间进行一个限幅操作。
    LimitH(IntervalHigh);                                                            //假如上一行的边线是第2列，那你2-5=-3，-3是不是就没有实际意义了？怎么会有-3列呢？
    Get_Border_And_SideType(PicTemp, 'R', IntervalLow, IntervalHigh,&JumpPoint[1]);  //扫线用的一个子函数，自己跳进去看明白逻辑。
  /******************************扫描本行的右边线******************************/

  /******************************扫描本行的左边线******************************/
    IntervalLow =Sideline_status_array[Ysite + 1].leftline  -ImageScanInterval;                //从上一行的左边线加减Interval对应的列开始扫描本行，Interval一般取5，当然你为了保险起见可以把这个值改的大一点。
    IntervalHigh =Sideline_status_array[Ysite + 1].leftline +ImageScanInterval;                //正常情况下只需要在上行边线左右5的基础上（差不多10列的这个区间）去扫线，一般就能找到本行的边线了，所以这个值其实不用太大。
    LimitL(IntervalLow);                                                             //这里就是对传给GetJumpPointFromDet()函数的扫描区间进行一个限幅操作。
    LimitH(IntervalHigh);                                                            //假如上一行的边线是第2列，那你2-5=-3，-3是不是就没有实际意义了？怎么会有-3列呢？
    Get_Border_And_SideType(PicTemp, 'L', IntervalLow, IntervalHigh,&JumpPoint[0]);  //扫线用的一个子函数，自己跳进去看明白逻辑。
  /******************************扫描本行的左边线******************************/

  /*
       下面的代码要是想看懂，想搞清楚我到底在赣神魔的话，
        请务必把GetJumpPointFromDet()这个函数的逻辑看懂，
        尤其是在这个函数里面，“T”、“W”、“H”三个标志代表什么，
        一定要搞懂!!!不然的话，建议不要往下看了，不要折磨自己!!!
  */
    if (JumpPoint[0].type =='W')                                                     //如果本行的左边线属于不正常跳变，即这10个点都是白点。
    {
      Sideline_status_array[Ysite].leftline =Sideline_status_array[Ysite + 1].leftline;                  //那么本行的左边线就采用上一行的边线。
    }
    else                                                                             //如果本行的左边线属于T或者是H类别
    {
      Sideline_status_array[Ysite].leftline = JumpPoint[0].point;                              //那么扫描到的边线是多少，我就记录下来是多少。
    }

    if (JumpPoint[1].type == 'W')                                                    //如果本行的右边线属于不正常跳变，即这10个点都是白点。
    {
      Sideline_status_array[Ysite].rightline =Sideline_status_array[Ysite + 1].rightline;                //那么本行的右边线就采用上一行的边线。
    }
    else                                                                             //如果本行的右边线属于T或者是H类别
    {
      Sideline_status_array[Ysite].rightline = JumpPoint[1].point;                             //那么扫描到的边线是多少，我就记录下来是多少。
    }

    Sideline_status_array[Ysite].IsLeftFind =JumpPoint[0].type;                                  //记录本行找到的左边线类型，是T？是W？还是H？这个类型后面是有用的，因为我还要进一步处理。
    Sideline_status_array[Ysite].IsRightFind = JumpPoint[1].type;                                //记录本行找到的右边线类型，是T？是W？还是H？这个类型后面是有用的，因为我还要进一步处理。


  /*
        下面就开始对W和H类型的边线分别进行处理， 为什么要处理？
        如果你看懂了GetJumpPointFromDet函数逻辑，明白了T W H三种类型分别对应什么情况，
        那你就应该知道W和H类型的边线都属于非正常类型，那我是不是要处理？
        这一部分的处理思路需要自己花大量时间好好的去琢磨，我在注释这里没法给你说清楚的。，
        实在想不通就来问我吧！
  */

    /************************************重新确定大跳变(即H类)的边界*************************************/

    if (( Sideline_status_array[Ysite].IsLeftFind == 'H' || Sideline_status_array[Ysite].IsRightFind == 'H'))
    {
      /**************************处理左边线的大跳变***************************/
      if (Sideline_status_array[Ysite].IsLeftFind == 'H')
      {
        for (Xsite = (Sideline_status_array[Ysite].leftline + 1);Xsite <= (Sideline_status_array[Ysite].rightline - 1);Xsite++)                                                           //左右边线之间重新扫描
        {
          if ((*(PicTemp + Xsite) == 0) && (*(PicTemp + Xsite + 1) != 0))
          {
            Sideline_status_array[Ysite].leftline =Xsite;
            Sideline_status_array[Ysite].IsLeftFind = 'T';
            break;
          }
          else if (*(PicTemp + Xsite) != 0)
            break;
          else if (Xsite ==(Sideline_status_array[Ysite].rightline - 1))
          {
            Sideline_status_array[Ysite].IsLeftFind = 'T';
            break;
          }
        }
      }
      /**************************处理左边线的大跳变***************************/


      /**************************处理右边线的大跳变***************************/
      if (Sideline_status_array[Ysite].IsRightFind == 'H')
      {
        for (Xsite = (Sideline_status_array[Ysite].rightline - 1);Xsite >= (Sideline_status_array[Ysite].leftline + 1); Xsite--)
        {
          if ((*(PicTemp + Xsite) == 0) && (*(PicTemp + Xsite - 1) != 0))
          {
            Sideline_status_array[Ysite].rightline =Xsite;
            Sideline_status_array[Ysite].IsRightFind = 'T';
            break;
          }
          else if (*(PicTemp + Xsite) != 0)
            break;
          else if (Xsite == (Sideline_status_array[Ysite].leftline + 1))
          {
            Sideline_status_array[Ysite].rightline = Xsite;
            Sideline_status_array[Ysite].IsRightFind = 'T';
            break;
          }
         }
       }
     }
    /**************************处理右边线的大跳变***************************/

  /*****************************重新确定大跳变的边界******************************/



 /************************************重新确定无边行（即W类）的边界****************************************************************/
    int ysite = 0;
    uint8 L_found_point = 0;
    uint8 R_found_point = 0;
    /**************************处理右边线的无边行***************************/
    if (Sideline_status_array[Ysite].IsRightFind == 'W' && Ysite > 10 && Ysite < 50)
    {
      if (Get_R_line == 'F')
      {
        Get_R_line = 'T';
        ytemp_W_R = Ysite + 2;
        for (ysite = Ysite + 1; ysite < Ysite + 15; ysite++)
        {
          if (Sideline_status_array[ysite].IsRightFind =='T')
          {
              R_found_point++;
          }
        }
        if (R_found_point >8)
        {
          D_R = ((float)(Sideline_status_array[Ysite + R_found_point].rightline - Sideline_status_array[Ysite + 3].rightline)) /((float)(R_found_point - 3));
          if (D_R > 0)
          {
            R_Found_T ='T';
          }
          else
          {
            R_Found_T = 'F';
            if (D_R < 0)
            {
                ExtenRFlag = 'F';
            }
          }
        }
      }
      if (R_Found_T == 'T')
      {
        Sideline_status_array[Ysite].rightline =Sideline_status_array[ytemp_W_R].rightline -D_R * (ytemp_W_R - Ysite);  //如果找到了 那么以基准行做延长线
      }
      LimitL(Sideline_status_array[Ysite].rightline);  //限幅
      LimitH(Sideline_status_array[Ysite].rightline);  //限幅
    }
    /**************************处理右边线的无边行***************************/


    /**************************处理左边线的无边行***************************/
    if (Sideline_status_array[Ysite].IsLeftFind == 'W' && Ysite > 10 && Ysite < 50 )
    {
      if (Get_L_line == 'F')
      {
        Get_L_line = 'T';
        ytemp_W_L = Ysite + 2;
        for (ysite = Ysite + 1; ysite < Ysite + 15; ysite++)
        {
          if (Sideline_status_array[ysite].IsLeftFind == 'T')
            {
              L_found_point++;
            }
        }
        if (L_found_point > 8)              //找到基准斜率边  做延长线重新确定无边
        {
          D_L = ((float)(Sideline_status_array[Ysite + 3].leftline -Sideline_status_array[Ysite + L_found_point].leftline)) /((float)(L_found_point - 3));
          if (D_L > 0)
          {
            L_Found_T = 'T';
          }
          else
          {
            L_Found_T = 'F';
            if (D_L < 0)
            {
                ExtenLFlag = 'F';
            }
          }
        }
      }

      if (L_Found_T == 'T')
      {
          Sideline_status_array[Ysite].leftline =Sideline_status_array[ytemp_W_L].leftline + D_L * (ytemp_W_L - Ysite);
      }

      LimitL(Sideline_status_array[Ysite].leftline);  //限幅
      LimitH(Sideline_status_array[Ysite].leftline);  //限幅
    }

    /**************************处理左边线的无边行***************************/
    /************************************重新确定无边行（即W类）的边界****************************************************************/


    /************************************都处理完之后，其他的一些数据整定操作*************************************************/
      if (Sideline_status_array[Ysite].IsLeftFind == 'W'&&Sideline_status_array[Ysite].IsRightFind == 'W')
      {
          imagestatus.WhiteLine++;  //要是左右都无边，丢边数+1
      }
     if (Sideline_status_array[Ysite].IsLeftFind == 'W' && Ysite < 55 )
     {
          imagestatus.Miss_Left_lines++;
     }
     if (Sideline_status_array[Ysite].IsRightFind == 'W'&&Ysite < 55)
     {
          imagestatus.Miss_Right_lines++;
     }

      LimitL(Sideline_status_array[Ysite].leftline);   //限幅
      LimitH(Sideline_status_array[Ysite].leftline);   //限幅
      LimitL(Sideline_status_array[Ysite].rightline);  //限幅
      LimitH(Sideline_status_array[Ysite].rightline);  //限幅

      Sideline_status_array[Ysite].wide =Sideline_status_array[Ysite].rightline - Sideline_status_array[Ysite].leftline;
      Sideline_status_array[Ysite].midline =(Sideline_status_array[Ysite].rightline + Sideline_status_array[Ysite].leftline) / 2;

      if (Sideline_status_array[Ysite].wide <= 7)
      {
          imagestatus.OFFLine = Ysite + 1;
          break;
      }
      else if (Sideline_status_array[Ysite].rightline <= 10||Sideline_status_array[Ysite].leftline >= 70)
      {
          imagestatus.OFFLine = Ysite + 1;
          break;
      }
      /************************************都处理完之后，其他的一些数据整定操作*************************************************/
  }
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//  @name           Get_ExtensionLine
//  @brief          这个函数的作用就是补线！
//  @brief          十字路口那里，按理来说车得直行对吧，但是这种情况摄像头扫线的时候，是不是会出现扫不到边线的情况？因为那几行都是白色的嘛，找不到黑白跳变点。
//  @brief          所以按照上面的搜边线算法，如果我们不对这种情况做算法处理的话，那我那些行的左右边界是不是就不对了？对应的中线是不是也不对了？那你能保证小车还直行嘛？
//  @brief          显然保证不了，所以这个时候小车可能就会根据算出来的中线，直接左转或者右转了，是不是违背比赛规则了，那是不是就寄了？所以说补线是非常重要的一环。
//  @parameter      void
//  @time           2023年2月21日
//  @Author
//  Sample usage:   Get_ExtensionLine();
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
void Get_ExtensionLine(void)
{
    //imagestatus.OFFLine=5;                                                  //这个结构体成员我之所以在这里赋值，是因为我ImageStatus结构体里面的成员太多了，但是暂时又只用到了OFFLine，所以我在哪用到它就在哪赋值。
    /************************************左边线的补线处理*************************************************/
    if (imagestatus.WhiteLine >= 8)                                       //如果丢边行的数量大于8
        TFSite = 55;                                                      //那就给TFSite赋值为55，这个变量是待会算补线斜率的一个变量。
    left_FTSite=0;
    right_FTSite=0;
    if (ExtenLFlag != 'F')                                                //如果ExtenLFlag标志量不等于F，那就开始进行补线操作。
        for (Ysite = 54; Ysite >= (imagestatus.OFFLine + 4);Ysite--)        //从第54开始往上扫，一直扫到顶边下面几行。
        {
            PicTemp = ex_mt9v03x_binarizeImage[Ysite];
            if (Sideline_status_array[Ysite].IsLeftFind =='W')                            //如果本行的左边线类型是W类型，也就是无边行类型。
            {
                if (Sideline_status_array[Ysite + 1].leftline >= 70)                      //如果左边界到了第70列右边去了，那大概率就是极端情况，说明已经快寄了。
                {
                  imagestatus.OFFLine = Ysite + 1;                              //这种情况最好的处理方法就是不处理，直接跳出循环。
                  break;
                }
                while (Ysite >= (imagestatus.OFFLine + 4))                      //如果左边界正常，那就进入while循环卡着，直到满足循环结束条件。
                {
                    Ysite--;                                                      //行数减减
                    if (Sideline_status_array[Ysite].IsLeftFind == 'T'
                      &&Sideline_status_array[Ysite - 1].IsLeftFind == 'T'
                      &&Sideline_status_array[Ysite - 2].IsLeftFind == 'T'
                      &&Sideline_status_array[Ysite - 2].leftline > 0
                      &&Sideline_status_array[Ysite - 2].leftline <70
                      )                                                         //如果扫到的无边行的上面连续三行都是正常边线
                    {
                      left_FTSite = Ysite - 2;                                         //那就把扫到的这一行的上面两行存入FTsite变量
                    break;                                                      //跳出while循环
                    }
                }
                DetL =((float)(Sideline_status_array[left_FTSite].leftline -Sideline_status_array[TFSite].leftline)) /((float)(left_FTSite - TFSite));  //计算左边线的补线斜率
                if (left_FTSite > imagestatus.OFFLine)                              //如果FTSite存储的那一行在图像顶边OFFline的下面
                    for (ytemp = TFSite; ytemp >= left_FTSite; ytemp--)               //那么我就从第一次扫到的左边界的下面第二行的位置开始往上一直补线，补到FTSite行。
                    {
                    Sideline_status_array[ytemp].leftline =(int)(DetL * ((float)(ytemp - TFSite))) +Sideline_status_array[TFSite].leftline;     //这里就是具体的补线操作了
                    }
            }
            else                                                              //注意看清楚这个else和哪个if是一对，搞清楚逻辑关系。
                TFSite = Ysite + 2;                                             //这里为什么要Ysite+2，我没法在注释里面讲清楚，自己领会吧。
        }
    /************************************左边线的补线处理*************************************************/


    /************************************右边线的补线处理（跟左边线处理思路一模一样）注释略*************************************************/
    if (imagestatus.WhiteLine >= 8)
    TFSite = 55;
    if (ExtenRFlag != 'F')
    for (Ysite = 54; Ysite >= (imagestatus.OFFLine + 4);Ysite--)
    {
      PicTemp = ex_mt9v03x_binarizeImage[Ysite];  //存当前行
      if (Sideline_status_array[Ysite].IsRightFind =='W')
      {
        if (Sideline_status_array[Ysite + 1].rightline <= 10)
        {
          imagestatus.OFFLine =Ysite + 1;
          break;
        }
        while (Ysite >= (imagestatus.OFFLine + 4))
        {
          Ysite--;
          if (Sideline_status_array[Ysite].IsRightFind == 'T'
              &&Sideline_status_array[Ysite - 1].IsRightFind == 'T'
              &&Sideline_status_array[Ysite - 2].IsRightFind == 'T'
              &&Sideline_status_array[Ysite - 2].rightline < 79
              &&Sideline_status_array[Ysite - 2].rightline > 10
              )
          {
              right_FTSite = Ysite - 2;
            break;
          }
        }

        DetR =((float)(Sideline_status_array[right_FTSite].rightline -Sideline_status_array[TFSite].rightline)) /((float)(right_FTSite - TFSite));
        if (right_FTSite > imagestatus.OFFLine)
          for (ytemp = TFSite; ytemp >= right_FTSite;ytemp--)
          {
            Sideline_status_array[ytemp].rightline =(int)(DetR * ((float)(ytemp - TFSite))) +Sideline_status_array[TFSite].rightline;
          }
      }
      else
        TFSite =Ysite +2;
    }
      /************************************右边线的补线处理（跟左边线处理思路一模一样）注释略*************************************************/



}

/*上交大左右手法则扫线，作为处理圆环等判断元素的第二依据*/
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//  @name           Search_Bottom_Line_OTSU
//  @brief          获取底层左右边线
//  @param          imageInput[IMAGE_ROW][IMAGE_COL]        传入的图像数组
//  @param          row                                     图像的Ysite
//  @param          col                                     图像的Xsite
//  @return         Bottonline                              底边行选择
//  @time           2022年10月9日
//  @Author         陈新云
//  Sample usage:   Search_Bottom_Line_OTSU(imageInput, row, col, Bottonline);
//--------------------------------------------------------------------------------------------------------------------------------------------

void Search_Bottom_Line_OTSU(uint8 imageInput[LCDH][LCDW], uint8 row, uint8 col, uint8 Bottonline)
{

    //寻找左边边界
    for (int Xsite = col / 2-2; Xsite > 1; Xsite--)
    {
        if (imageInput[Bottonline][Xsite] != 0 && imageInput[Bottonline][Xsite - 1] == 0)
        {
            Sideline_status_array[Bottonline].LeftBoundary = Xsite;//获取底边左边线
            break;
        }
    }
    for (int Xsite = col / 2+2; Xsite < LCDW-1; Xsite++)
    {
        if (imageInput[Bottonline][Xsite] != 0 && imageInput[Bottonline][Xsite + 1] == 0)
        {
            Sideline_status_array[Bottonline].RightBoundary = Xsite;//获取底边右边线
            break;
        }
    }


}
//void Search_Bottom_Line_OTSU(uint8 imageInput[LCDH][LCDW], uint8 row, uint8 col, uint8 Bottonline)
//{
//
//    //寻找左边边界
//    for (int Xsite = col / 2-2; Xsite > 1; Xsite--)
//    {
//        if (imageInput[Bottonline][Xsite] == 1 && imageInput[Bottonline][Xsite - 1] == 0)
//        {
//            Sideline_status_array[Bottonline].LeftBoundary = Xsite;//获取底边左边线
//            break;
//        }
//    }
//    for (int Xsite = col / 2+2; Xsite < LCDW-1; Xsite++)
//    {
//        if (imageInput[Bottonline][Xsite] == 1 && imageInput[Bottonline][Xsite + 1] == 0)
//        {
//            Sideline_status_array[Bottonline].RightBoundary = Xsite;//获取底边右边线
//            break;
//        }
//    }


//}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//  @name           Search_Left_and_Right_Lines
//  @brief          通过sobel提取左右边线
//  @param          imageInput[IMAGE_ROW][IMAGE_COL]        传入的图像数组
//  @param          row                                     图像的Ysite
//  @param          col                                     图像的Xsite
//  @param          Bottonline                              底边行选择
//  @return         无
//  @time           2022年10月7日
//  @Author         陈新云
//  Sample usage:   Search_Left_and_Right_Lines(imageInput, row, col, Bottonline);
//--------------------------------------------------------------------------------------------------------------------------------------------

void Search_Left_and_Right_Lines(uint8 imageInput[LCDH][LCDW], uint8 row, uint8 col, uint8 Bottonline)
{
    //定义小人的当前行走状态位置为 上 左 下 右 一次要求 上：左边为黑色 左：上边为褐色 下：右边为色  右：下面有黑色
/*  前进方向定义：
                *   0
                * 3   1
                *   2
*/
/*寻左线坐标规则*/
    uint8 Left_Rule[2][8] = {
                                  {0,-1,1,0,0,1,-1,0 },//{0,-1},{1,0},{0,1},{-1,0},  (x,y )
                                  {-1,-1,1,-1,1,1,-1,1} //{-1,-1},{1,-1},{1,1},{-1,1}
    };
    /*寻右线坐标规则*/
    int Right_Rule[2][8] = {
                              {0,-1,1,0,0,1,-1,0 },//{0,-1},{1,0},{0,1},{-1,0},
                              {1,-1,1,1,-1,1,-1,-1} //{1,-1},{1,1},{-1,1},{-1,-1}
    };
      int num=0;
    uint8 Left_Ysite = Bottonline;
    uint8 Left_Xsite = Sideline_status_array[Bottonline].LeftBoundary;
    uint8 Left_Rirection = 0;//左边方向
    uint8 Pixel_Left_Ysite = Bottonline;
    uint8 Pixel_Left_Xsite = 0;

    uint8 Right_Ysite = Bottonline;
    uint8 Right_Xsite = Sideline_status_array[Bottonline].RightBoundary;
    uint8 Right_Rirection = 0;//右边方向
    uint8 Pixel_Right_Ysite = Bottonline;
    uint8 Pixel_Right_Xsite = 0;
    uint8 Ysite = Bottonline;
    imagestatus.OFFLineBoundary = 5;
    while (1)
    {
            num++;
            if(num>400)
            {
                 imagestatus.OFFLineBoundary = Ysite;
                break;
            }
        if (Ysite >= Pixel_Left_Ysite && Ysite >= Pixel_Right_Ysite)
        {
            if (Ysite < imagestatus.OFFLineBoundary)
            {
                imagestatus.OFFLineBoundary = Ysite;
                break;
            }
            else
            {
                Ysite--;
            }
        }
        /*********左边巡线*******/
        if ((Pixel_Left_Ysite > Ysite) || Ysite == imagestatus.OFFLineBoundary)//右边扫线
        {
            /*计算前方坐标*/
            Pixel_Left_Ysite = Left_Ysite + Left_Rule[0][2 * Left_Rirection + 1];
            Pixel_Left_Xsite = Left_Xsite + Left_Rule[0][2 * Left_Rirection];

            if (imageInput[Pixel_Left_Ysite][Pixel_Left_Xsite] == 0)//前方是黑色
            {
                //顺时针旋转90
                if (Left_Rirection == 3)
                    Left_Rirection = 0;
                else
                    Left_Rirection++;
            }
            else//前方是白色
            {
                /*计算左前方坐标*/
                Pixel_Left_Ysite = Left_Ysite + Left_Rule[1][2 * Left_Rirection + 1];
                Pixel_Left_Xsite = Left_Xsite + Left_Rule[1][2 * Left_Rirection];

                if (imageInput[Pixel_Left_Ysite][Pixel_Left_Xsite] == 0)//左前方为黑色
                {
                    //方向不变  Left_Rirection
                    Left_Ysite = Left_Ysite + Left_Rule[0][2 * Left_Rirection + 1];
                    Left_Xsite = Left_Xsite + Left_Rule[0][2 * Left_Rirection];
                    if (Sideline_status_array[Left_Ysite].LeftBoundary_First == 0)
                        Sideline_status_array[Left_Ysite].LeftBoundary_First = Left_Xsite;
                    Sideline_status_array[Left_Ysite].LeftBoundary = Left_Xsite;
                }
                else//左前方为白色
                {
                    // 方向发生改变 Left_Rirection  逆时针90度
                    Left_Ysite = Left_Ysite + Left_Rule[1][2 * Left_Rirection + 1];
                    Left_Xsite = Left_Xsite + Left_Rule[1][2 * Left_Rirection];
                    if (Sideline_status_array[Left_Ysite].LeftBoundary_First == 0 )
                        Sideline_status_array[Left_Ysite].LeftBoundary_First = Left_Xsite;
                    Sideline_status_array[Left_Ysite].LeftBoundary = Left_Xsite;
                    if (Left_Rirection == 0)
                        Left_Rirection = 3;
                    else
                        Left_Rirection--;
                }

            }
        }
        /*********右边巡线*******/
        if ((Pixel_Right_Ysite > Ysite) || Ysite == imagestatus.OFFLineBoundary)//右边扫线
        {
            /*计算前方坐标*/
            Pixel_Right_Ysite = Right_Ysite + Right_Rule[0][2 * Right_Rirection + 1];
            Pixel_Right_Xsite = Right_Xsite + Right_Rule[0][2 * Right_Rirection];

            if (imageInput[Pixel_Right_Ysite][Pixel_Right_Xsite] == 0)//前方是黑色
            {
                //逆时针旋转90
                if (Right_Rirection == 0)
                    Right_Rirection = 3;
                else
                    Right_Rirection--;
            }
            else//前方是白色
            {
                /*计算右前方坐标*/
                Pixel_Right_Ysite = Right_Ysite + Right_Rule[1][2 * Right_Rirection + 1];
                Pixel_Right_Xsite = Right_Xsite + Right_Rule[1][2 * Right_Rirection];

                if (imageInput[Pixel_Right_Ysite][Pixel_Right_Xsite] == 0)//左前方为黑色
                {
                    //方向不变  Right_Rirection
                    Right_Ysite = Right_Ysite + Right_Rule[0][2 * Right_Rirection + 1];
                    Right_Xsite = Right_Xsite + Right_Rule[0][2 * Right_Rirection];
                    if (Sideline_status_array[Right_Ysite].RightBoundary_First == 79 )
                        Sideline_status_array[Right_Ysite].RightBoundary_First = Right_Xsite;
                    Sideline_status_array[Right_Ysite].RightBoundary = Right_Xsite;
                }
                else//左前方为白色
                {
                    // 方向发生改变 Right_Rirection  逆时针90度
                    Right_Ysite = Right_Ysite + Right_Rule[1][2 * Right_Rirection + 1];
                    Right_Xsite = Right_Xsite + Right_Rule[1][2 * Right_Rirection];
                    if (Sideline_status_array[Right_Ysite].RightBoundary_First == 79)
                        Sideline_status_array[Right_Ysite].RightBoundary_First = Right_Xsite;
                    Sideline_status_array[Right_Ysite].RightBoundary = Right_Xsite;
                    if (Right_Rirection == 3)
                        Right_Rirection = 0;
                    else
                        Right_Rirection++;
                }

            }
        }

        if (abs(Pixel_Right_Xsite - Pixel_Left_Xsite) < 3)//Ysite<80是为了放在底部是斑马线扫描结束  3 && Ysite < 30
        {

            imagestatus.OFFLineBoundary = Ysite;
            break;
        }

    }
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//  @name           Search_Border_OTSU
//  @brief          通过OTSU获取边线 和信息
//  @param          imageInput[IMAGE_ROW][IMAGE_COL]        传入的图像数组
//  @param          row                                     图像的Ysite
//  @param          col                                     图像的Xsite
//  @param          Bottonline                              底边行选择
//  @return         无
//  @time           2022年10月7日
//  @Author         陈新云
//  Sample usage:   Search_Border_OTSU(mt9v03x_image, IMAGE_ROW, IMAGE_COL, IMAGE_ROW-8);
//--------------------------------------------------------------------------------------------------------------------------------------------

void Search_Border_OTSU(uint8 imageInput[LCDH][LCDW], uint8 row, uint8 col, uint8 Bottonline)
{
    imagestatus.WhiteLine_L = 0;
    imagestatus.WhiteLine_R = 0;
    //imagestatus.OFFLine = 1;
    /*封上下边界处理*/
    for (int Xsite = 0; Xsite < LCDW; Xsite++)
    {
        imageInput[0][Xsite] = 0;
        imageInput[Bottonline + 1][Xsite] = 0;
    }
    /*封左右边界处理*/
    for (int Ysite = 0; Ysite < LCDH; Ysite++)
    {
            Sideline_status_array[Ysite].LeftBoundary_First = 0;
            Sideline_status_array[Ysite].RightBoundary_First = 79;

            imageInput[Ysite][0] = 0;
            imageInput[Ysite][LCDW - 1] = 0;
    }
    /********获取底部边线*********/
    Search_Bottom_Line_OTSU(imageInput, row, col, Bottonline);
    /********获取左右边线*********/
    Search_Left_and_Right_Lines(imageInput, row, col, Bottonline);



    for (int Ysite = Bottonline; Ysite > imagestatus.OFFLineBoundary + 1; Ysite--)
    {
        if (Sideline_status_array[Ysite].LeftBoundary < 3)
        {
            imagestatus.WhiteLine_L++;
        }
        if (Sideline_status_array[Ysite].RightBoundary > LCDW - 3)
        {
            imagestatus.WhiteLine_R++;
        }
    }
}


//--------------------------------------------------------------
//  @name           Element_Judgment_Left_Rings()
//  @brief          整个图像判断的子函数，用来判断左圆环类型.
//  @parameter      void
//  @time
//  @Author         MRCHEN
//  Sample usage:   Element_Judgment_Left_Rings();
//--------------------------------------------------------------
void Element_Judgment_Left_Rings()
{
    if (   imagestatus.Miss_Right_lines > 5 || imagestatus.Miss_Left_lines < 10
        || imagestatus.OFFLine > 20 || Straight_Judge(2, imagestatus.OFFLine, 55) > 1
        || imageflag.image_element_rings == 2
//        || imageflag.Out_Road == 1 || imageflag.RoadBlock_Flag == 1
//        || Sideline_status_array[52].IsLeftFind == 'W'
//        || Sideline_status_array[53].IsLeftFind == 'W'
//        || Sideline_status_array[54].IsLeftFind == 'W'
//        || Sideline_status_array[55].IsLeftFind == 'W'
        || Sideline_status_array[56].IsLeftFind == 'W'
        || Sideline_status_array[57].IsLeftFind == 'W'
        || Sideline_status_array[58].IsLeftFind == 'W')
        return;
    int ring_ysite = 25;
    uint8 Left_Less_Num = 0;
    Left_RingsFlag_Point1_Ysite = 0;
    Left_RingsFlag_Point2_Ysite = 0;
    for (int Ysite = 58; Ysite > ring_ysite; Ysite--)
    {
        if (Sideline_status_array[Ysite].LeftBoundary_First - Sideline_status_array[Ysite - 1].LeftBoundary_First > 4)
        {
            Left_RingsFlag_Point1_Ysite = Ysite;
            break;
        }
    }
    for (int Ysite = 58; Ysite > ring_ysite; Ysite--)
    {
        if (Sideline_status_array[Ysite + 1].LeftBoundary - Sideline_status_array[Ysite].LeftBoundary > 4)
        {
            Left_RingsFlag_Point2_Ysite = Ysite;
            break;
        }
    }
    for (int Ysite = Left_RingsFlag_Point1_Ysite; Ysite > Left_RingsFlag_Point1_Ysite - 11; Ysite--)
    {
        if (Sideline_status_array[Ysite].IsLeftFind == 'W')
            Left_Less_Num++;
    }
    for (int Ysite = Left_RingsFlag_Point1_Ysite; Ysite > imagestatus.OFFLine; Ysite--)
    {
//        if (Sideline_status_array[Ysite + 3].LeftBoundary_First < Sideline_status_array[Ysite].LeftBoundary_First
//            && Sideline_status_array[Ysite + 2].LeftBoundary_First < Sideline_status_array[Ysite].LeftBoundary_First
//            && Sideline_status_array[Ysite].LeftBoundary_First > Sideline_status_array[Ysite - 1].LeftBoundary_First
//            && Sideline_status_array[Ysite].LeftBoundary_First > Sideline_status_array[Ysite - 1].LeftBoundary_First
//            )
//        uint8 up_point=0,down_point=0;
//        for(uint8 Ysite_2=Ysite;Ysite_2 < (Ysite+4);Ysite_2++)
//        {
//            if(Sideline_status_array[Ysite_2].)
//        }
        if (   Sideline_status_array[Ysite + 6].LeftBoundary < Sideline_status_array[Ysite+3].LeftBoundary
            && Sideline_status_array[Ysite + 5].LeftBoundary < Sideline_status_array[Ysite+3].LeftBoundary
            && Sideline_status_array[Ysite + 3].LeftBoundary > Sideline_status_array[Ysite + 2].LeftBoundary
            && Sideline_status_array[Ysite + 3].LeftBoundary > Sideline_status_array[Ysite + 1].LeftBoundary
            )
        {
            Ring_Help_Flag = 1;
            break;
        }
    }
    if(Left_RingsFlag_Point2_Ysite > Left_RingsFlag_Point1_Ysite+1 && Ring_Help_Flag == 0 && Left_Less_Num>7)
    {
        if(imagestatus.Miss_Left_lines > 10)
            Ring_Help_Flag = 1;
    }
    if (Left_RingsFlag_Point2_Ysite > Left_RingsFlag_Point1_Ysite+1 && Ring_Help_Flag == 1 && Left_Less_Num>7)
    {
        imageflag.image_element_rings = 1;
        imageflag.image_element_rings_flag = 1;
        imageflag.ring_big_small=1;
//        Front_Wait_After_Enter_Ring_Flag = 0;
//        gpio_set_level(B0, 1);
    }
    Ring_Help_Flag = 0;
}


//--------------------------------------------------------------
//  @name           Element_Judgment_Right_Rings()
//  @brief          整个图像判断的子函数，用来判断右圆环类型.
//  @parameter      void
//  @time
//  @Author         MRCHEN
//  Sample usage:   Element_Judgment_Right_Rings();
//--------------------------------------------------------------
void Element_Judgment_Right_Rings()
{
    if (   imagestatus.Miss_Left_lines > 5 || imagestatus.Miss_Right_lines < 10
        || imagestatus.OFFLine > 20 || Straight_Judge(1,imagestatus.OFFLine, 55) > 1
        || imageflag.image_element_rings == 1 || imageflag.Out_Road == 1 || imageflag.RoadBlock_Flag == 1
//        || Sideline_status_array[52].IsRightFind == 'W'
//        || Sideline_status_array[53].IsRightFind == 'W'
//        || Sideline_status_array[54].IsRightFind == 'W'
//        || Sideline_status_array[55].IsRightFind == 'W'
        || Sideline_status_array[56].IsRightFind == 'W'
        || Sideline_status_array[57].IsRightFind == 'W'
        || Sideline_status_array[58].IsRightFind == 'W')
        return;

    int ring_ysite = 25;
    uint8 Right_Less_Num = 0;
    Right_RingsFlag_Point1_Ysite = 0;
    Right_RingsFlag_Point2_Ysite = 0;
    for (int Ysite = 58; Ysite > ring_ysite; Ysite--)
    {
        if (Sideline_status_array[Ysite - 2].RightBoundary_First - Sideline_status_array[Ysite].RightBoundary_First > 4)
        {
            Right_RingsFlag_Point1_Ysite = Ysite;
            break;
        }
    }
    for (int Ysite = 58; Ysite > ring_ysite; Ysite--)
    {
        if (Sideline_status_array[Ysite].RightBoundary - Sideline_status_array[Ysite + 2].RightBoundary > 4)
        {
            Right_RingsFlag_Point2_Ysite = Ysite;
            break;
        }
    }
    for (int Ysite = Right_RingsFlag_Point1_Ysite; Ysite > Right_RingsFlag_Point1_Ysite - 11; Ysite--)
    {
        if (Sideline_status_array[Ysite].IsRightFind == 'W')
            Right_Less_Num++;
    }
    //ips114_show_int(60,40, Right_Less_Num,3);
    for (int Ysite = Right_RingsFlag_Point1_Ysite; Ysite > imagestatus.OFFLine; Ysite--)
    {
        if (   Sideline_status_array[Ysite + 6].RightBoundary > Sideline_status_array[Ysite + 3].RightBoundary
            && Sideline_status_array[Ysite + 5].RightBoundary > Sideline_status_array[Ysite + 3].RightBoundary
            && Sideline_status_array[Ysite + 3].RightBoundary < Sideline_status_array[Ysite + 2].RightBoundary
            && Sideline_status_array[Ysite + 3].RightBoundary < Sideline_status_array[Ysite + 1].RightBoundary
           )
        {
            Ring_Help_Flag = 1;
            break;
        }
    }
    if(Right_RingsFlag_Point2_Ysite > Right_RingsFlag_Point1_Ysite+3 && Ring_Help_Flag == 0 && Right_Less_Num > 7)
    {
        if(imagestatus.Miss_Right_lines>10)
            Ring_Help_Flag = 1;
    }
    if (Right_RingsFlag_Point2_Ysite > Right_RingsFlag_Point1_Ysite+3 && Ring_Help_Flag == 1 && Right_Less_Num > 7)
    {
        //Stop=1;
        imageflag.image_element_rings = 2;
        imageflag.image_element_rings_flag = 1;
        imageflag.ring_big_small=1;
//        Front_Wait_After_Enter_Ring_Flag = 0;
//        gpio_set_level(B0, 1);
    }

        //ips200_show_uint(100,220,imageflag.image_element_rings,3);
    Ring_Help_Flag = 0;
}



//--------------------------------------------------------------
//  @name           Element_Judgment_Ramp()
//  @brief          整个图像判断的子函数，用来判断坡道
//  @parameter      void
//  @time
//  @Author         MRCHEN
//  Sample usage:   Element_Judgment_Ramp();
//--------------------------------------------------------------
//void Element_Judgment_Ramp()
//{
//    if (imagestatus.WhiteLine >= 3 || Ramp_cancel)
//        return;
//    int i = 0;
//    if (imagestatus.OFFLine <= 5)
//    {
//        for (Ysite = imagestatus.OFFLine + 1; Ysite < 7; Ysite++)
//        {
//            if ( Sideline_status_array[Ysite].wide > 18
//                        && (Sideline_status_array[Ysite].IsRightFind == 'T'
//                        && Sideline_status_array[Ysite].IsLeftFind == 'T')
//                        && Sideline_status_array[Ysite].leftline < 40
//                        && Sideline_status_array[Ysite].rightline > 40 && ex_mt9v03x_binarizeImage[Ysite][Sideline_status_array[Ysite].midline] == 1
//                        && ex_mt9v03x_binarizeImage[Ysite][Sideline_status_array[Ysite].midline-2] == 1
//                        && ex_mt9v03x_binarizeImage[Ysite][Sideline_status_array[Ysite].midline+2] == 1
//                        && dl1b_distance_mm < 700
//                        && imagestatus.Miss_Left_lines <7
//                        && imagestatus.Miss_Right_lines < 7
//                )
//                     i++;
////            ips114_show_int(20,0, i,3);
//            if (i >= 3)
//            {
////                Stop=1;
//                imageflag.Ramp = 1;
////                BeeOn;
////                Statu = Ramp;
//                i = 0;
//                break;
//            }
//        }
//    }
//
//}


//--------------------------------------------------------------
//  @name           Element_Judgment_OutRoad()
//  @brief          整个图像判断的子函数，用来判断断路类型.
//  @parameter      void
//  @time
//  @Author         MRCHEN
//  Sample usage:   Element_Judgment_Bend();
//--------------------------------------------------------------
float S = 0;
int Start=0,End=0,a=0;
int Out_Black_flag = 0;
void Element_Judgment_OutRoad()
{
    if( imageflag.RoadBlock_Flag == 1 || imageflag.Out_Road)   return;
    uint8 Right_Num=0,Left_Num=0;
    if(imagestatus.OFFLine > 30)
    {
        for(int Ysite = imagestatus.OFFLine + 1;Ysite < imagestatus.OFFLine + 11;Ysite++)
        {
            if(Sideline_status_array[Ysite].IsLeftFind == 'T')
                Left_Num++;
            if(Sideline_status_array[Ysite].IsRightFind == 'T')
                Right_Num++;
        }
    }
    uint8 W_Num=0;
    for(uint i=58;i>48;i--)
    {
        if(Sideline_status_array[i].IsLeftFind=='W'&& Sideline_status_array[i].IsRightFind=='W')
        {
            W_Num++;
        }
    }
//    ips114_show_int(20,0,Left_Num,5);
//    ips114_show_int(20,20,Right_Num,5);
    if((Left_Num > 7 && Right_Num > 7 && !imageflag.Out_Road)|| (W_Num>7))
    {
        imageflag.Out_Road = 1;
    }

}
//--------------------------------------------------------------
//  @name           Element_Judgment_RoadBlock()
//  @brief          整个图像判断的子函数，用来判断路障类型.
//  @parameter      void
//  @time
//  @Author         MRCHEN
//  Sample usage:   Element_Judgment_RoadBlock();
//--------------------------------------------------------------
//uint8 Auto_Through_RoadBlock = 0;
//int RoadBlock_length = 0;
//uint8 RoadBlock_Flag = 0;
//uint8 RoadBlock_Thruough_Flag = 2;
//uint8 Regular_RoadBlock = 0;
//uint8 RoadBlock_Regular_Way[8][3] = {{0,0,0},{1,0,0},{0,1,0},{1,1,0},{0,0,1},{1,0,1},{0,1,1},{1,1,1}};
//uint8 RoadBlock_Thruough_Flag_Record = 0;
//uint8 Bend_Thruough_RoadBlock_Flag = 0;
//uint8 ICM20602_Clear_Flag = 0;
//int RoadBlock_Length_Compensate = 0;
//void Element_Judgment_RoadBlock()
//{
////    if(imageflag.Ramp /*|| imagestatus.OFFLineBoundary < 20 || dl1a_distance_mm > 1100*/)   return;
////    if(imagestatus.OFFLine >= 10  && imagestatus.OFFLine < 50 && dl1b_distance_mm < 1100
////    && Straight_Judge(1, imagestatus.OFFLine+8, imagestatus.OFFLine+25) < 1
////    && Straight_Judge(2, imagestatus.OFFLine+8, imagestatus.OFFLine+25) < 1
////    && imagestatus.Miss_Left_lines< 3
////    && imagestatus.Miss_Right_lines< 3
////    )
////    {
////        imageflag.RoadBlock_Flag = 1;
////        //Stop = 1;
////        BeeOn;
////        Statu = RoadBlock;
////        Steer_Flag=1; //打开陀螺仪
////        Angle_block = Yaw_Now;//陀螺仪一段打角
////        RoadBlock_Flag++;
////    }
//
//    if(imageflag.Ramp || imagestatus.OFFLine < 10 || dl1b_distance_mm > 1100)   return;
//    int Right_Num=0,Left_Num=0;
//    for(int Ysite = imagestatus.OFFLine + 1;Ysite < imagestatus.OFFLine + 11;Ysite++)
//    {
//        //if((Sideline_status_array[Ysite].IsLeftFind))
//        if(Sideline_status_array[Ysite].IsLeftFind == 'T')
//            Left_Num++;
//        if(Sideline_status_array[Ysite].IsRightFind == 'T')
//            Right_Num++;
//    }
//    if(Left_Num > 7 && Right_Num > 7)
//    {
//        imageflag.RoadBlock_Flag = 1;
//        block_num++;
//        /********/
//        //Stop = 1;
////        Statu = RoadBlock;
//        if( !Regular_RoadBlock )
//            Auto_RoadBlock_Through();
//        else
//        {
//            if( RoadBlock_Regular_Way[Regular_RoadBlock][block_num-1] )
//            {
//                if(Parameter_Justment_Flag == 1)
//                   { RoadBlock_Thruough_Flag = 1; Auto_Through_RoadBlock = 0; }
//                if(Parameter_Justment_Flag == 2)
//                   { RoadBlock_Thruough_Flag = 2; Auto_Through_RoadBlock = 0; }
//                if( imagestatus.Det_True > 2 || imagestatus.Det_True < -2 )
//                   {
//                    RoadBlock_Length_Compensate = 267;
////                    BeeOn;
//                   }
//                else
//                    RoadBlock_Length_Compensate = 0;
//            }
////            else
////                Auto_RoadBlock_Through();
//        }
////        Steer_Flag=1; //打开陀螺仪
//        ICM20602_Clear_Flag++;
//        RoadBlock_Flag++;
//    }
//}
//
//void Auto_RoadBlock_Through(void)
//{
//    if( imagestatus.Det_True >= 0 )
//    {
//        RoadBlock_Thruough_Flag = 1;  //左
//        if( imagestatus.Det_True > 3 )
//         { Auto_Through_RoadBlock = 1;  }
//        else
//            Auto_Through_RoadBlock = 0;
//    }
//    else
//    {
//        RoadBlock_Thruough_Flag = 2;   //右
//        if( imagestatus.Det_True < -3 )
//         { Auto_Through_RoadBlock = 1;  }
//        else
//            Auto_Through_RoadBlock = 0;
//    }
//}
//--------------------------------------------------------------
//  @name           Element_Handle_RoadBlock()
//  @brief          整个图像处理的子函数，用来处理路障类型.
//  @parameter      void
//  @time
//  @Author         MRCHEN
//  Sample usage:   Element_Handle_RoadBlock();
//--------------------------------------------------------------

//void Element_Handle_RoadBlock()
//{
//    if( ICM20602_Clear_Flag == 2 )
//    {
//        if ( RoadBlock_Thruough_Flag == 1 )
//        {//(这里修改第一段陀螺仪打角)
//            if ( ((( Yaw_Now-Angle_block > 25 && !Auto_Through_RoadBlock ) || ( Yaw_Now-Angle_block > 20 && Auto_Through_RoadBlock )) &&
//                          RoadBlock_Flag == 1 && !Bend_Thruough_RoadBlock_Flag ) ||
//               ( ( Yaw_Now-Angle_block > 45 && RoadBlock_Flag == 1 &&  Bend_Thruough_RoadBlock_Flag ) ) )
//            { Steer_Flag = 0; RoadBlock_Flag++; /*Stop = 1;*/ } // 路障第一段打角，用陀螺仪判断  25
//            if ( RoadBlock_Flag == 2 )
//            { // 路障第二段，用编码器计数
//                RoadBlock_length = (Element_encoder1+Element_encoder2)/2;
//                if ( ( (( RoadBlock_length > (1123+RoadBlock_Length_Compensate) && !Auto_Through_RoadBlock ) || ( RoadBlock_length > 987 && Auto_Through_RoadBlock ))
//                        && !Bend_Thruough_RoadBlock_Flag ) || (RoadBlock_length > 678 && Bend_Thruough_RoadBlock_Flag) )
//                { RoadBlock_Flag++; Steer_Flag = 1;RoadBlock_length = 0; /*Stop = 1;*/ }//  这里修改编码器计数  3500 1970
//                if(RoadBlock_Flag == 3 && !block_flag)   //陀螺仪第二段打角
//                {
//                    Angle_block = Yaw_Now;
//                    block_flag = 1;
//                }
//            }
//            if( (((( Angle_block-Yaw_Now > 45 && !Auto_Through_RoadBlock ) || ( Angle_block-Yaw_Now > 40 && Auto_Through_RoadBlock )) &&
//                    RoadBlock_Flag == 3 && !Bend_Thruough_RoadBlock_Flag)  ||
//                 (Angle_block-Yaw_Now > 45 && RoadBlock_Flag == 3 &&  Bend_Thruough_RoadBlock_Flag)) || RoadBlock_Flag > 3 )  //修改第三段陀螺仪打角  50
//            {
//                //Stop = 1;
//                RoadBlock_Flag++;
//                RoadBlock_Flag = RoadBlock_Flag > 10 ? 10 : RoadBlock_Flag;
//                if(RoadBlock_Flag == 4)
//                {
//                    Bend_Thruough_RoadBlock_Flag = 0;
//                    Auto_Through_RoadBlock = 0;
//                    return;
//                }
//                Gray_Value=0;
//                for(Ysite = 55;Ysite>45;Ysite--)
//                    for(Xsite=35;Xsite<65;Xsite++)
//                        Gray_Value=Gray_Value+ex_mt9v03x_binarizeImage[Ysite][Xsite];
//                if(Gray_Value>270)//最大300
//                {
//                    //Stop = 1;
//                    imageflag.RoadBlock_Flag = 0;
//                    BeeOff;
//                    RoadBlock_Flag = 0;block_flag = 0;Angle_block=0;Steer_Flag=0;
//                    Element_encoder1 = 0;Element_encoder2 = 0; RoadBlock_Length_Compensate = 0;
//                    Through_RoadBlock_Flag_Delay = 0;ICM20602_Clear_Flag = 0;
//                    RoadBlock_Thruough_Flag = RoadBlock_Thruough_Flag_Record;
//                }
//            }
//        }
//        else if ( RoadBlock_Thruough_Flag == 2 )
//        {//(这里修改第一段陀螺仪打角)
//            if ( ((( Angle_block-Yaw_Now > 25 && !Auto_Through_RoadBlock ) || ( Angle_block-Yaw_Now > 20 && Auto_Through_RoadBlock )) &&
//                                  RoadBlock_Flag == 1 && !Bend_Thruough_RoadBlock_Flag ) ||
//                       ( ( Angle_block-Yaw_Now > 45 && RoadBlock_Flag == 1 &&  Bend_Thruough_RoadBlock_Flag ) ) )
//            { Steer_Flag = 0; RoadBlock_Flag++; /*Stop = 1;*/ } // 路障第一段打角，用陀螺仪判断
//            if ( RoadBlock_Flag == 2 )
//            { // 路障第二段，用编码器计数
//                RoadBlock_length = (Element_encoder1+Element_encoder2)/2;
//                if ( ( (( RoadBlock_length > (1423+RoadBlock_Length_Compensate) && !Auto_Through_RoadBlock ) || ( RoadBlock_length > 987 && Auto_Through_RoadBlock ))
//                                    && !Bend_Thruough_RoadBlock_Flag ) || (RoadBlock_length > 789 && Bend_Thruough_RoadBlock_Flag) )
//                { RoadBlock_Flag++; Steer_Flag = 1;RoadBlock_length = 0; /*Stop = 1;*/ }//  这里修改编码器计数  3500 1970
//                if(RoadBlock_Flag == 3 && !block_flag)   //陀螺仪第二段打角
//                {
//                    Angle_block = Yaw_Now;
//                    block_flag = 1;
//                }
//            }
//            if( (((( Yaw_Now-Angle_block > 45 && !Auto_Through_RoadBlock ) || ( Yaw_Now-Angle_block > 40 && Auto_Through_RoadBlock )) &&
//                            RoadBlock_Flag == 3 && !Bend_Thruough_RoadBlock_Flag)  ||
//                         (Yaw_Now-Angle_block > 45 && RoadBlock_Flag == 3 &&  Bend_Thruough_RoadBlock_Flag)) || RoadBlock_Flag > 3 )  //修改第三段陀螺仪打角  50
//            {
//                //Stop = 1;
//                RoadBlock_Flag++;
//                RoadBlock_Flag = RoadBlock_Flag > 10 ? 10 : RoadBlock_Flag;
//                if(RoadBlock_Flag == 4)
//                {
//                    Bend_Thruough_RoadBlock_Flag = 0;
//                    Auto_Through_RoadBlock = 0;
//                    return;
//                }
//                Gray_Value=0;
//                for(Ysite = 55;Ysite>45;Ysite--)
//                    for(Xsite=15;Xsite<45;Xsite++)
//                        Gray_Value=Gray_Value+ex_mt9v03x_binarizeImage[Ysite][Xsite];
//                if(Gray_Value>270)//最大400
//                {
//                    //Stop = 1;
//                    imageflag.RoadBlock_Flag = 0;
//                    BeeOff;
//                    RoadBlock_Flag = 0;block_flag = 0;Angle_block=0;Steer_Flag=0;
//                    Element_encoder1 = 0;Element_encoder2 = 0;
//                    Through_RoadBlock_Flag_Delay = 0;ICM20602_Clear_Flag = 0;
//                    RoadBlock_Thruough_Flag = RoadBlock_Thruough_Flag_Record;
//                    RoadBlock_Length_Compensate = 0;
//                }
//            }
//        }
//    }
//}

//--------------------------------------------------------------
//  @name           Element_Judgment_Zebra()
//  @brief          整个图像判断的子函数，用来判断斑马线
//  @parameter      void
//  @time
//  @Author         MRCHEN
//  Sample usage:   Element_Judgment_Zebra();
//--------------------------------------------------------------
void Element_Judgment_Zebra(void)//斑马线判断
{
    if(imageflag.Zebra_Flag || imageflag.image_element_rings == 1 || imageflag.image_element_rings == 2
            || imageflag.Out_Road == 1 || imageflag.RoadBlock_Flag == 1) return;
    int NUM = 0, net = 0;
    if(imagestatus.OFFLineBoundary<20)
    {
        for (int Ysite = 20; Ysite < 33; Ysite++)
        {
            net = 0;
            for (int Xsite =Sideline_status_array[Ysite].LeftBoundary + 2; Xsite < Sideline_status_array[Ysite].RightBoundary - 2; Xsite++)
            {
                if (ex_mt9v03x_binarizeImage[Ysite][Xsite] == 0 && ex_mt9v03x_binarizeImage[Ysite][Xsite + 1] != 0)
                {
                    net++;
                    if (net > 4)
                        NUM++;
                }
            }
        }
    }

    if (NUM >= 4 && imageflag.Zebra_Flag == 0)
    {
        if(imagestatus.Miss_Left_lines > (imagestatus.Miss_Right_lines + 3))//左车库
        {
            imageflag.Zebra_Flag = 1;
            Garage_Location_Flag++;
            gpio_set_level(B0, 1);
        }
        if((imagestatus.Miss_Left_lines + 3)<imagestatus.Miss_Right_lines)//右车库
        {
            imageflag.Zebra_Flag = 2;
            Garage_Location_Flag++;
            gpio_set_level(B0, 1);
        }
    }

}


//--------------------------------------------------------------
//  @name           Element_Handle_Zebra()
//  @brief          整个图像处理的子函数，用来处理斑马线
//  @parameter      void
//  @time
//  @Author         MRCHEN
//  Sample usage:   Element_Handle_Zebra();
//--------------------------------------------------------------
//int Zebra_length = 0;
void Element_Handle_Zebra()//斑马线处理
{
//    Zebra_length=((Element_encoder1+Element_encoder2)/2);
    static uint8 Thought_flag=0;
    if(imageflag.Zebra_Flag == 1)//左车库
    {
        for (int Ysite = 59; Ysite > imagestatus.OFFLineBoundary + 1; Ysite--)
        {
             Sideline_status_array[Ysite].midline = Sideline_status_array[Ysite].RightBoundary  - Half_Road_Wide[Ysite];
        }
    }
    else if(imageflag.Zebra_Flag == 2)//右车库
    {
        for (int Ysite = 59; Ysite > imagestatus.OFFLineBoundary + 1; Ysite--)
        {
             Sideline_status_array[Ysite].midline = Sideline_status_array[Ysite].LeftBoundary  + Half_Road_Wide[Ysite];
        }
    }
    uint16 wide=0;
    for(uint8 i=30;i<36;i++)
    {
        wide+=Sideline_status_array[i].wide;
    }
    wide=wide/5;

    uint8 account=0;
    for(uint8 i=59;i>54;i--)
    {
        if(Sideline_status_array[i].wide<=wide)account++;
    }
    if(Thought_flag==0)
    {
        if(account>=4)Thought_flag=1;
    }else
    {
        if(account<=1)
        {
            Thought_flag=0;
            imageflag.Zebra_Flag=0;
        }
    }
    /**********/
//    if(Garage_Location_Flag < Garage_num)
//    {
//        if(Zebra_length > Zebra_num)
//        {
//            imageflag.Zebra_Flag = 0;
//            gpio_set_level(B0, 0);
//            Element_encoder1 = 0;
//            Element_encoder2 = 0;
//            Zebra_length = 0;
//        }
//    }
//    else if(Garage_Location_Flag == Garage_num)
//    {
//        if(Zebra_length > Garage_length)
//        {
//            Stop=1;
//            //imageflag.Zebra_Flag = 0;
//            Element_encoder1 = 0;
//            Element_encoder2 = 0;
//            Zebra_length = 0;
//            gpio_set_level(B0, 0);
//        }
//    }
}

//--------------------------------------------------------------
//  @name           Element_Handle_OutRoad()
//  @brief          整个图像处理的子函数，用来处理坡道
//  @parameter      void
//  @time
//  @Author         MRCHEN
//  Sample usage:   Element_Handle_OutRoad();
//--------------------------------------------------------------
void Element_Handle_OutRoad()//断路处理
{
    Gray_Value=0;
    for(int Ysite = 35;Ysite < 55;Ysite++)
    {
        for(int Xsite = 30;Xsite < 50;Xsite++)
        {
            if(ex_mt9v03x_binarizeImage[Ysite][Xsite])
            Gray_Value++;
        }
    }
    if(Gray_Value > 360 && imagestatus.OFFLine < 20)//最大400
    {
        imageflag.Out_Road=0;

    }
}

//--------------------------------------------------------------
//  @name           Element_Handle_Ramp()
//  @brief          整个图像处理的子函数，用来处理坡道
//  @parameter      void
//  @time
//  @Author         MRCHEN
//  Sample usage:   Element_Handle_Ramp();
//--------------------------------------------------------------
//uint Ramp_length = 0;
//void Element_Handle_Ramp()//坡道处理
//{
//     Ramp_length = ((Element_encoder1+Element_encoder2)/2);
//
//     if( Ramp_length > Ramp_num )//    170高坡道
//     {
//         //Stop=1;
//         imageflag.Ramp = 0;
//         BeeOff;
//         Element_encoder1 = 0;
//         Element_encoder2 = 0;
//         Ramp_length = 0;
//     }
//}
//--------------------------------------------------------------------
// 函数简介     补线辅助函数
// 参数说明     y_up        补线上点
// 参数说明     y_down      补线下点
// 参数说明     x_up        补线上点
// 参数说明     x_down      补线下点
// 参数说明     mode        0补左边线，1补右边线
//---------------------------------------------------------------------
void connect_line_subsidiary(uint8 y_up,uint8 y_down,uint8 x_up,uint8 x_down,uint8 mode)
{

    DetL =((float)(x_up-x_down)) /((float)(y_up - y_down));  //计算左边线的补线斜率
        if(mode==0)
        {
            for (ytemp = y_down; ytemp >= y_up; ytemp--)               //那么我就从第一次扫到的左边界的下面第二行的位置开始往上一直补线，补到FTSite行。
                   {
                       Sideline_status_array[ytemp].leftline =(int)(DetL * ((float)(ytemp - y_down))) +Sideline_status_array[y_down].leftline;     //这里就是具体的补线操作了
                   }
        }
        else
        {
            for (ytemp = y_down; ytemp >= y_up; ytemp--)               //那么我就从第一次扫到的左边界的下面第二行的位置开始往上一直补线，补到FTSite行。
            {
                Sideline_status_array[ytemp].rightline =(int)(DetL * ((float)(ytemp - y_down))) +Sideline_status_array[y_down].rightline;     //这里就是具体的补线操作了
            }
        }
}


//--------------------------------------------------------------
//  @name           Element_Handle_Left_Rings()
//  @brief          整个图像处理的子函数，用来处理左圆环类型.
//  @parameter      void
//  @time
//  @Author         MRCHEN
//  Sample usage:   Element_Handle_Left_Rings();
//-------------------------------------------------------------
void Element_Handle_Left_Rings_Test(void)
{
    uint8 num = 0;
//    uint8 line_right_point_Y=0,line_rightpoint
    if(imageflag.image_element_rings_flag == 1)
    {
        for (int Ysite = 58; Ysite > imagestatus.OFFLine; Ysite--)
        {

            if(    Sideline_status_array[Ysite+3].IsLeftFind == 'W' && Sideline_status_array[Ysite+2].IsLeftFind == 'W'
                       && Sideline_status_array[Ysite+1].IsLeftFind == 'W' && Sideline_status_array[Ysite].IsLeftFind == 'T')
            {
                break;
            }

               }
        for(int Ysite = 58; Ysite > imagestatus.OFFLine; Ysite--)
        {
            Sideline_status_array[Ysite].midline=Sideline_status_array[Ysite].rightline-Half_Road_Wide[Ysite];
        }
        Point_Xsite=0;
        Point_Ysite=0;
        num=0;
        for(uint8 i=55;i>imagestatus.OFFLine;i--)//寻找拐点
        {
            if(Sideline_status_array[i].LeftBoundary<5)num++;
            if((num+5)>(55-imagestatus.OFFLine) && imageflag.image_element_rings_flag==2){imageflag.image_element_rings_flag=3;break;}
            if((Sideline_status_array[i].LeftBoundary-Sideline_status_array[i+1].LeftBoundary)>10 && (Sideline_status_array[i-1].LeftBoundary-Sideline_status_array[i+1].LeftBoundary)>10)
            {
                Point_Ysite=i;
                Point_Xsite=Sideline_status_array[i].LeftBoundary;
                break;
            }
        }
    }

       if(imageflag.image_element_rings_flag == 1 && Point_Ysite > 30)//即将到达圆环中间段落的的时候，左边是大圆环
       {
           imageflag.image_element_rings_flag=2;
       }

       /*补线处理*/

       if(imageflag.image_element_rings_flag == 2)
       {
           connect_line_subsidiary(Point_Ysite,59,Point_Xsite,79,1);
       }
       /*补线处理*/
       /*圆环内处理*/
       if(imageflag.image_element_rings_flag==2 )
       {
           for(uint8 i=55;i>imagestatus.OFFLine;i--)
           {
               Sideline_status_array[i].midline=Sideline_status_array[i].rightline-Half_Bend_Wide[i];
           }

       }
       if( imageflag.image_element_rings_flag==3)
       {
           for(uint8 i=55;i>imagestatus.OFFLine;i--)
           {
               Sideline_status_array[i].midline=Sideline_status_array[i].rightline-Half_Road_Wide[i];
           }
       }
       /*圆环内处理*/
       /*出环岛*/
       if(imageflag.image_element_rings_flag==3 && imagestatus.WhiteLine>20)
       {
           imageflag.image_element_rings_flag=4;
       }
       if(imageflag.image_element_rings_flag==4)
       {
           for(uint8 i=55;i>imagestatus.OFFLine;i--)
           {
               Sideline_status_array[i].midline=Sideline_status_array[i].rightline-Half_Bend_Wide[i];
           }

       }
       if(imageflag.image_element_rings_flag==4 && Straight_Judge(2, (imagestatus.OFFLine+15), 50)<1)
       {
           imageflag.image_element_rings_flag = 0;
           imageflag.image_element_rings = 0;
           imageflag.ring_big_small = 0;
       }
}
void Element_Handle_Left_Rings()
{
    /****************大小圆环判断*****************/
//    if(imageflag.ring_big_small == 0)
//    {
//        Black = 0;
//        for (int Ysite = 30; Ysite > 0; Ysite--)
//        {
//            if(ex_mt9v03x_binarizeImage[Ysite][2]==0)
//            {
//                Black++;
//            }
//            if(Ysite>2)
//            {
//                if(ex_mt9v03x_binarizeImage[Ysite-1][2]==0 && ex_mt9v03x_binarizeImage[Ysite][2]==1 && ex_mt9v03x_binarizeImage[Ysite-1][2]==1)
//                    break;
//            }
//        }
//        if(Black > 12) //大圆环
//            imageflag.ring_big_small = 1;
//        else                        //小圆环
//            {imageflag.ring_big_small = 2;
//             BeeOn;}
//    }

    /***************************************判断**************************************/
    int num = 0;
    for (int Ysite = 55; Ysite > 30; Ysite--)
    {
//        if(Sideline_status_array[Ysite].LeftBoundary_First < 3)
//        {
//            num++;
//        }
        if(Sideline_status_array[Ysite].IsLeftFind == 'W')
            num++;
        if(    Sideline_status_array[Ysite+3].IsLeftFind == 'W' && Sideline_status_array[Ysite+2].IsLeftFind == 'W'
            && Sideline_status_array[Ysite+1].IsLeftFind == 'W' && Sideline_status_array[Ysite].IsLeftFind == 'T')
            break;
    }
//    ips114_show_int(180,20,num,5);
        //准备进环
    if (imageflag.image_element_rings_flag == 1 && num>15)
    {
        imageflag.image_element_rings_flag = 2;
    }
    if (imageflag.image_element_rings_flag == 2 && num<10)
    {

        imageflag.image_element_rings_flag = 5;
    }
//    if(imageflag.image_element_rings_flag == 3 && num<3)//
//    {
//        imageflag.image_element_rings_flag = 4;
//    }
        //刚到圆弧
    if(imageflag.image_element_rings_flag == 4)
    {
        //Stop= 1;
        Point_Ysite = 0;
        Point_Xsite = 0;

        for (int Ysite = 20; Ysite > imagestatus.OFFLine + 2; Ysite--)
        {
            if(Sideline_status_array[Ysite].IsLeftFind == 'W' && Sideline_status_array[Ysite-1].IsLeftFind == 'T')
            {
                Point_Ysite = Ysite;
                Point_Xsite = 0;
                break;
            }
        }
        if(Point_Ysite > 6 && imageflag.ring_big_small == 1)
        {
            imageflag.image_element_rings_flag=5;
        }
        if(Point_Ysite > 11 && imageflag.ring_big_small == 2)
        {
            imageflag.image_element_rings_flag=5;
        }
    }
        //进环
    if(imageflag.image_element_rings_flag == 5 && imagestatus.Miss_Right_lines>15)
    {
        imageflag.image_element_rings_flag = 6;
    }
        //进环小圆环
    if(imageflag.image_element_rings_flag == 6 && imagestatus.Miss_Right_lines<7)
    {
        //Stop = 1;
        imageflag.image_element_rings_flag = 7;
    }
        //环内 大圆环判断
    if (imageflag.ring_big_small == 1 && imageflag.image_element_rings_flag == 7)
    {
        Point_Ysite = 0;
        Point_Xsite = 0;
        for (int Ysite = 45; Ysite > imagestatus.OFFLine + 7; Ysite--)
        {
            if (    Sideline_status_array[Ysite].rightline <= Sideline_status_array[Ysite + 2].rightline
                 && Sideline_status_array[Ysite].rightline <= Sideline_status_array[Ysite - 2].rightline
                 && Sideline_status_array[Ysite].rightline <= Sideline_status_array[Ysite + 1].rightline
                 && Sideline_status_array[Ysite].rightline <= Sideline_status_array[Ysite - 1].rightline
                 && Sideline_status_array[Ysite].rightline <= Sideline_status_array[Ysite + 4].rightline
                 && Sideline_status_array[Ysite].rightline <= Sideline_status_array[Ysite - 4].rightline
                 && Sideline_status_array[Ysite].rightline <= Sideline_status_array[Ysite + 6].rightline
                 && Sideline_status_array[Ysite].rightline <= Sideline_status_array[Ysite - 6].rightline
                 && Sideline_status_array[Ysite].rightline <= Sideline_status_array[Ysite + 5].rightline
                 && Sideline_status_array[Ysite].rightline <= Sideline_status_array[Ysite - 5].rightline
               )
            {
                Point_Xsite = Sideline_status_array[Ysite].rightline;
                Point_Ysite = Ysite;
                break;
            }
        }
        if (Point_Ysite > 22)
        {
            imageflag.image_element_rings_flag = 8;
            //Stop = 1;
        }
    }
        //环内 小圆环判断
    if (imageflag.image_element_rings_flag == 7 && imageflag.ring_big_small == 2)
    {
        Point_Ysite = 0;
        Point_Xsite = 0;
        for (int Ysite = 50; Ysite > imagestatus.OFFLineBoundary + 3; Ysite--)
        {
            if (    Sideline_status_array[Ysite].RightBoundary < Sideline_status_array[Ysite + 2].RightBoundary
                 && Sideline_status_array[Ysite].RightBoundary < Sideline_status_array[Ysite - 2].RightBoundary
               )
            {
                Point_Xsite = Sideline_status_array[Ysite].RightBoundary;
                Point_Ysite = Ysite;
                break;
            }
        }
        if (Point_Ysite > 20)
          imageflag.image_element_rings_flag = 8;
    }
        //出环后
    if (imageflag.image_element_rings_flag == 8)
    {
        if (    Straight_Judge(2, imagestatus.OFFLine+15, 50) < 1
             && imagestatus.Miss_Right_lines < 8
             && imagestatus.OFFLine < 7)    //右边为直线且截止行（前瞻值）很小

            imageflag.image_element_rings_flag = 9;
    }
        //结束圆环进程
//    if (imageflag.image_element_rings_flag == 9 )
//    {
//        int num=0;
//        for (int Ysite = 58; Ysite > 30; Ysite--)
//        {
//            if(Sideline_status_array[Ysite].LeftBoundary_First < 3 )
//            {
//                num++;
//            }
//        }
//        if(num > 8)
//        {
//            imageflag.image_element_rings_flag = 10;
//        }
//    }

    if (imageflag.image_element_rings_flag == 9)
    {
        int num=0;
        for (int Ysite = 40; Ysite > 10; Ysite--)
        {
            if(Sideline_status_array[Ysite].IsLeftFind == 'W' )
                num++;
        }
        if(num < 5)
        {
            imageflag.image_element_rings_flag = 0;
            imageflag.image_element_rings = 0;
            imageflag.ring_big_small = 0;
            /********/
//            Front_Wait_After_Enter_Ring_Count++;
            gpio_set_level(B0, 0);
        }
    }



    /***************************************处理**************************************/
        //准备进环  半宽处理
    if (   imageflag.image_element_rings_flag == 1
        || imageflag.image_element_rings_flag == 2
        || imageflag.image_element_rings_flag == 3
        || imageflag.image_element_rings_flag == 4)
    {
        for (int Ysite = 59; Ysite > imagestatus.OFFLine; Ysite--)
        {
            Sideline_status_array[Ysite].midline = Sideline_status_array[Ysite].rightline - Half_Road_Wide[Ysite];
        }
    }
        //进环  补线
    if  ( imageflag.image_element_rings_flag == 5
        ||imageflag.image_element_rings_flag == 6
        )
    {
        int  flag_Xsite_1=0;
        int flag_Ysite_1=0;
        float Slope_Rings=0;
        for(Ysite=55;Ysite>imagestatus.OFFLine;Ysite--)//下面弧点
        {
            for(Xsite=Sideline_status_array[Ysite].leftline + 1;Xsite<Sideline_status_array[Ysite].rightline - 1;Xsite++)
            {
                if(  ex_mt9v03x_binarizeImage[Ysite][Xsite] != 0 && ex_mt9v03x_binarizeImage[Ysite][Xsite + 1] == 0)
                 {
                   flag_Ysite_1 = Ysite;
                   flag_Xsite_1 = Xsite;
                   Slope_Rings=(float)(79-flag_Xsite_1)/(float)(59-flag_Ysite_1);
                   break;
                 }
            }
            if(flag_Ysite_1 != 0)
            {
                break;
            }
        }
        if(flag_Ysite_1 == 0)
        {
            for(Ysite=imagestatus.OFFLine+1;Ysite<30;Ysite++)
            {
                if(Sideline_status_array[Ysite].IsLeftFind=='T'&&Sideline_status_array[Ysite+1].IsLeftFind=='T'&&Sideline_status_array[Ysite+2].IsLeftFind=='W'
                    &&abs(Sideline_status_array[Ysite].leftline-Sideline_status_array[Ysite+2].leftline)>10
                  )
                {
                    flag_Ysite_1=Ysite;
                    flag_Xsite_1=Sideline_status_array[flag_Ysite_1].leftline;
                    imagestatus.OFFLine=Ysite;
                    Slope_Rings=(float)(79-flag_Xsite_1)/(float)(59-flag_Ysite_1);
                    break;
                }

            }
        }
        //补线
        if(flag_Ysite_1 != 0)
        {
            for(Ysite=flag_Ysite_1;Ysite<60;Ysite++)
            {
                Sideline_status_array[Ysite].rightline=flag_Xsite_1+Slope_Rings*(Ysite-flag_Ysite_1);
                //if(imageflag.ring_big_small==1)//大圆环不减半宽
                    Sideline_status_array[Ysite].midline = (Sideline_status_array[Ysite].rightline + Sideline_status_array[Ysite].leftline)/2;
                //else//小圆环减半宽
                //    Sideline_status_array[Ysite].midline = Sideline_status_array[Ysite].rightline - Half_Bend_Wide[Ysite];
//                if(Sideline_status_array[Ysite].midline<0)
//                    Sideline_status_array[Ysite].midline = 0;
            }
            Sideline_status_array[flag_Ysite_1].rightline=flag_Xsite_1;
            for(Ysite=flag_Ysite_1-1;Ysite>10;Ysite--) //A点上方进行扫线
            {
                for(Xsite=Sideline_status_array[Ysite+1].rightline-10;Xsite<Sideline_status_array[Ysite+1].rightline+2;Xsite++)
                {
                    if(ex_mt9v03x_binarizeImage[Ysite][Xsite]!= 0 && ex_mt9v03x_binarizeImage[Ysite][Xsite+1]==0)
                    {
                        Sideline_status_array[Ysite].rightline=Xsite;
                        //if(imageflag.ring_big_small==1)//大圆环不减半宽
                            Sideline_status_array[Ysite].midline = (Sideline_status_array[Ysite].rightline + Sideline_status_array[Ysite].leftline)/2;
                        //else//小圆环减半宽
                        //    Sideline_status_array[Ysite].midline = Sideline_status_array[Ysite].rightline - Half_Bend_Wide[Ysite];
                        //if(Sideline_status_array[Ysite].midline<0)
                        //    Sideline_status_array[Ysite].midline = 0;
                        //Sideline_status_array[Ysite].wide=Sideline_status_array[Ysite].rightline-Sideline_status_array[Ysite].leftline;
                        break;
                    }
                }

                if(Sideline_status_array[Ysite].wide>8 &&Sideline_status_array[Ysite].rightline< Sideline_status_array[Ysite+2].rightline)
                {
                    continue;
                }
                else
                {
                    imagestatus.OFFLine=Ysite+2;
                    break;
                }
            }
        }
    }
        //环内 小环弯道减半宽 大环不减
    if (imageflag.image_element_rings_flag == 7)
    {
//        for (int Ysite = 57; Ysite > imagestatus.OFFLine+1; Ysite--)
//        {
//            if(imageflag.ring_big_small==2)
//                Sideline_status_array[Ysite].midline = Sideline_status_array[Ysite].rightline - Half_Bend_Wide[Ysite];
//            if(Sideline_status_array[Ysite].midline<=0)
//            {
//                Sideline_status_array[Ysite].midline = 0;
//                imagestatus.OFFLine=Ysite-1;
//                break;
//            }
//        }
    }
        //大圆环出环 补线
    if (imageflag.image_element_rings_flag == 8 && imageflag.ring_big_small == 1)    //大圆环
    {
        Repair_Point_Xsite = 20;
        Repair_Point_Ysite = 0;
        for (int Ysite = 40; Ysite > 5; Ysite--)
        {
            if (ex_mt9v03x_binarizeImage[Ysite][23] != 0 && ex_mt9v03x_binarizeImage[Ysite-1][23] == 0)//28
            {
                Repair_Point_Xsite = 23;
                Repair_Point_Ysite = Ysite-1;
                imagestatus.OFFLine = Ysite + 1;  //截止行重新规划
                break;
            }
        }
        for (int Ysite = 57; Ysite > Repair_Point_Ysite-3; Ysite--)         //补线
        {
            Sideline_status_array[Ysite].rightline = (Sideline_status_array[58].rightline - Repair_Point_Xsite) * (Ysite - 58) / (58 - Repair_Point_Ysite)  + Sideline_status_array[58].rightline;
            Sideline_status_array[Ysite].midline = (Sideline_status_array[Ysite].rightline + Sideline_status_array[Ysite].leftline) / 2;
        }
    }
        //小圆环出环 补线
    if (imageflag.image_element_rings_flag == 8 && imageflag.ring_big_small == 2)    //小圆环
    {
        Repair_Point_Xsite = 0;
        Repair_Point_Ysite = 0;
        for (int Ysite = 55; Ysite > 5; Ysite--)
        {
            if (ex_mt9v03x_binarizeImage[Ysite][15] != 0 && ex_mt9v03x_binarizeImage[Ysite-1][15] == 0)
            {
                Repair_Point_Xsite = 15;
                Repair_Point_Ysite = Ysite-1;
                imagestatus.OFFLine = Ysite + 1;  //截止行重新规划
                break;
            }
        }
        for (int Ysite = 57; Ysite > Repair_Point_Ysite-3; Ysite--)         //补线
        {
            Sideline_status_array[Ysite].rightline = (Sideline_status_array[58].rightline - Repair_Point_Xsite) * (Ysite - 58) / (58 - Repair_Point_Ysite)  + Sideline_status_array[58].rightline;
            Sideline_status_array[Ysite].midline = (Sideline_status_array[Ysite].rightline + Sideline_status_array[Ysite].leftline) / 2;
        }
    }
        //已出环 半宽处理
    if (imageflag.image_element_rings_flag == 9 || imageflag.image_element_rings_flag == 10)
    {
        for (int Ysite = 59; Ysite > imagestatus.OFFLine; Ysite--)
        {
            Sideline_status_array[Ysite].midline = Sideline_status_array[Ysite].rightline - Half_Road_Wide[Ysite];
        }
    }
}

//--------------------------------------------------------------
//  @name           Element_Handle_Right_Rings()
//  @brief          整个图像处理的子函数，用来处理右圆环类型.
//  @parameter      void
//  @time
//  @Author         MRCHEN
//  Sample usage:   Element_Handle_Right_Rings();
//-------------------------------------------------------------
void Element_Handle_Right_Rings()
{
    /****************大小圆环判断*****************/
//    if(imageflag.ring_big_small == 0)
//    {
//        Less_Big_Small_Num = 0;
//        Black=0;
//        for (int Ysite = 30; Ysite > 0; Ysite--)
//        {
//            if(ex_mt9v03x_binarizeImage[Ysite][77]==0)
//            {
//                Black++;
//            }
//            if(Ysite>2)
//            {
//                if(ex_mt9v03x_binarizeImage[Ysite-1][77]==0 && ex_mt9v03x_binarizeImage[Ysite][77]==1 && ex_mt9v03x_binarizeImage[Ysite-1][77]==1)
//                    break;
//            }
//        }
//        if(Black > 12) //大圆环
//            imageflag.ring_big_small = 1;
//        else          //小圆环
//            {imageflag.ring_big_small = 2;
//             BeeOn;}
//    }
    /****************判断*****************/
    int num =0 ;
    for (int Ysite = 55; Ysite > 30; Ysite--)
    {
//        if(Sideline_status_array[Ysite].RightBoundary_First > 76)
//        {
//            num++;
//        }
        if(Sideline_status_array[Ysite].IsRightFind == 'W')
            num++;
        if(    Sideline_status_array[Ysite+3].IsRightFind == 'W' && Sideline_status_array[Ysite+2].IsRightFind == 'W'
            && Sideline_status_array[Ysite+1].IsRightFind == 'W' && Sideline_status_array[Ysite].IsRightFind == 'T')
            break;
    }
        //准备进环
    if (imageflag.image_element_rings_flag == 1 && num>15)
    {
        imageflag.image_element_rings_flag = 2;
    }
    if (imageflag.image_element_rings_flag == 2 && num<10)
    {
        imageflag.image_element_rings_flag = 5;
        //Stop = 1;
    }
        //刚到圆弧
    if (imageflag.image_element_rings_flag == 4)
    {
        Point_Ysite = 0;
        Point_Xsite = 0;
        for (int Ysite = 30; Ysite > imagestatus.OFFLine + 2; Ysite--)
        {
            if(Sideline_status_array[Ysite].IsRightFind == 'W' && Sideline_status_array[Ysite-1].IsRightFind == 'T')
            {
                Point_Ysite = Ysite;
                Point_Xsite = 0;
                break;
            }
        }
        if(Point_Ysite > 6 && imageflag.ring_big_small == 1)
        {
         //   Stop =1;
            imageflag.image_element_rings_flag=5;
        }
        if(Point_Ysite > 11 && imageflag.ring_big_small == 2)
        {
         //   Stop =1;
            imageflag.image_element_rings_flag=5;
        }

    }
        //进环
    if(imageflag.image_element_rings_flag == 5 && imagestatus.Miss_Left_lines>15)
    {
      //  Stop = 1;
        imageflag.image_element_rings_flag = 6;
    }
        //进环小圆环
    if(imageflag.image_element_rings_flag == 6 && imagestatus.Miss_Left_lines<7)
    {
        imageflag.image_element_rings_flag = 7;
       // Stop=1;


        //代码目前直接复制大圆环 大小圆环判断有问题



//        imageflag.ring_big_small = 1 ;
    }
        //环内 大圆环判断
    if (imageflag.ring_big_small == 1 && imageflag.image_element_rings_flag == 7)
    {
        Point_Xsite = 0;
        Point_Ysite = 0;
        for (int Ysite = 45; Ysite > imagestatus.OFFLine + 7; Ysite--)
        {
            if (    Sideline_status_array[Ysite].leftline >= Sideline_status_array[Ysite + 2].leftline
                 && Sideline_status_array[Ysite].leftline >= Sideline_status_array[Ysite - 2].leftline
                 && Sideline_status_array[Ysite].leftline >= Sideline_status_array[Ysite + 1].leftline
                 && Sideline_status_array[Ysite].leftline >= Sideline_status_array[Ysite - 1].leftline
                 && Sideline_status_array[Ysite].leftline >= Sideline_status_array[Ysite + 4].leftline
                 && Sideline_status_array[Ysite].leftline >= Sideline_status_array[Ysite - 4].leftline
                 && Sideline_status_array[Ysite].leftline >= Sideline_status_array[Ysite + 5].leftline
                 && Sideline_status_array[Ysite].leftline >= Sideline_status_array[Ysite - 5].leftline
                 && Sideline_status_array[Ysite].leftline >= Sideline_status_array[Ysite + 6].leftline
                 && Sideline_status_array[Ysite].leftline >= Sideline_status_array[Ysite - 6].leftline
                )

            {
                        Point_Xsite = Sideline_status_array[Ysite].leftline;
                        Point_Ysite = Ysite;
                        break;
            }
        }
        if (Point_Ysite > 22)
        {
            imageflag.image_element_rings_flag = 8;
            //Stop = 1;
        }
    }
        //环内 小圆环判断
    if (imageflag.ring_big_small == 2 && imageflag.image_element_rings_flag == 7)
    {
        Point_Xsite = 0;
        Point_Ysite = 0;
        for (int Ysite = 50; Ysite > imagestatus.OFFLineBoundary+3; Ysite--)
        {
            if (  Sideline_status_array[Ysite].LeftBoundary > Sideline_status_array[Ysite + 2].LeftBoundary
               && Sideline_status_array[Ysite].LeftBoundary > Sideline_status_array[Ysite - 2].LeftBoundary
              )

            {
                      Point_Xsite = Sideline_status_array[Ysite].LeftBoundary;
                      Point_Ysite = Ysite;
                      break;
            }
        }
        if (Point_Ysite > 20)
        {
            imageflag.image_element_rings_flag = 8;
        }
    }
        //出环后
    if (imageflag.image_element_rings_flag == 8)
    {
         if (   Straight_Judge(1, imagestatus.OFFLine+15, 50) < 1
             && imagestatus.Miss_Left_lines < 8
             && imagestatus.OFFLine < 7)    //右边为直线且截止行（前瞻值）很小
            {imageflag.image_element_rings_flag = 9;

            }
    }

    //结束圆环进程
    if (imageflag.image_element_rings_flag == 9)
    {
        int num=0;
        for (int Ysite = 50; Ysite > 10; Ysite--)
        {
            if(Sideline_status_array[Ysite].IsRightFind == 'W' )
                num++;
        }
        if(num < 5)
        {
            imageflag.image_element_rings_flag = 0;
            imageflag.image_element_rings = 0;
            imageflag.ring_big_small = 0;
            /********/
//            Front_Wait_After_Enter_Ring_Count++;
            gpio_set_level(B0, 0);
        }
    }
    /***************************************处理**************************************/
         //准备进环  半宽处理
    if (   imageflag.image_element_rings_flag == 1
        || imageflag.image_element_rings_flag == 2
        || imageflag.image_element_rings_flag == 3
        || imageflag.image_element_rings_flag == 4)
    {
        for (int Ysite = 59; Ysite > imagestatus.OFFLine; Ysite--)
        {
            Sideline_status_array[Ysite].midline = Sideline_status_array[Ysite].leftline + Half_Road_Wide[Ysite];
        }
    }

        //进环  补线
    if (   imageflag.image_element_rings_flag == 5
        || imageflag.image_element_rings_flag == 6
       )
    {
        int flag_Xsite_1=0;
        int  flag_Ysite_1=0;
        float Slope_Right_Rings = 0;
        for(Ysite=55;Ysite>imagestatus.OFFLine;Ysite--)
        {
            for(Xsite=Sideline_status_array[Ysite].leftline + 1;Xsite<Sideline_status_array[Ysite].rightline - 1;Xsite++)
            {
                if(ex_mt9v03x_binarizeImage[Ysite][Xsite]!= 0 && ex_mt9v03x_binarizeImage[Ysite][Xsite+1]==0)
                {
                    flag_Ysite_1=Ysite;
                    flag_Xsite_1=Xsite;
                    Slope_Right_Rings=(float)(0-flag_Xsite_1)/(float)(59-flag_Ysite_1);
                    break;
                }
            }
            if(flag_Ysite_1!=0)
            {
              break;
            }
        }
        if(flag_Ysite_1==0)
        {
        for(Ysite=imagestatus.OFFLine+1;Ysite<30;Ysite++)
        {
         if(Sideline_status_array[Ysite].IsRightFind=='T'&&Sideline_status_array[Ysite+1].IsRightFind=='T'&&Sideline_status_array[Ysite+2].IsRightFind=='W'
               &&abs(Sideline_status_array[Ysite].rightline-Sideline_status_array[Ysite+2].rightline)>10
         )
         {
             flag_Ysite_1=Ysite;
             flag_Xsite_1=Sideline_status_array[flag_Ysite_1].rightline;
             imagestatus.OFFLine=Ysite;
             Slope_Right_Rings=(float)(0-flag_Xsite_1)/(float)(59-flag_Ysite_1);
             break;
         }

        }

        }
        //补线
        if(flag_Ysite_1!=0)
        {
            for(Ysite=flag_Ysite_1;Ysite<60;Ysite++)
            {
                Sideline_status_array[Ysite].leftline=flag_Xsite_1+Slope_Right_Rings*(Ysite-flag_Ysite_1);
                //if(imageflag.ring_big_small==2)//小圆环加半宽
                //    Sideline_status_array[Ysite].midline=Sideline_status_array[Ysite].leftline+Half_Bend_Wide[Ysite];//板块
//              else//大圆环不加半宽
                    Sideline_status_array[Ysite].midline=(Sideline_status_array[Ysite].leftline+Sideline_status_array[Ysite].rightline)/2;//板块
                //if(Sideline_status_array[Ysite].midline>79)
                //    Sideline_status_array[Ysite].midline=79;
            }
            Sideline_status_array[flag_Ysite_1].leftline=flag_Xsite_1;
            for(Ysite=flag_Ysite_1-1;Ysite>10;Ysite--) //A点上方进行扫线
            {
                for(Xsite=Sideline_status_array[Ysite+1].leftline+8;Xsite>Sideline_status_array[Ysite+1].leftline-4;Xsite--)
                {
                    if(ex_mt9v03x_binarizeImage[Ysite][Xsite]!= 0 && ex_mt9v03x_binarizeImage[Ysite][Xsite-1]==0)
                    {
                     Sideline_status_array[Ysite].leftline=Xsite;
                     Sideline_status_array[Ysite].wide=Sideline_status_array[Ysite].rightline-Sideline_status_array[Ysite].leftline;
                  //   if(imageflag.ring_big_small==2)//小圆环加半宽
                  //       Sideline_status_array[Ysite].midline=Sideline_status_array[Ysite].leftline+Half_Bend_Wide[Ysite];//板块
                   //  else//大圆环不加半宽
                         Sideline_status_array[Ysite].midline=(Sideline_status_array[Ysite].leftline+Sideline_status_array[Ysite].rightline)/2;//板块
                   //  if(Sideline_status_array[Ysite].midline>79)
                   //      Sideline_status_array[Ysite].midline=79;
                     break;
                    }
                }
                if(Sideline_status_array[Ysite].wide>8 && Sideline_status_array[Ysite].leftline>  Sideline_status_array[Ysite+2].leftline)
                {
                    continue;
                }
                else
                {
                    imagestatus.OFFLine=Ysite+2;
                    break;
                }
            }
        }


    }
        //环内不处理
    if (imageflag.image_element_rings_flag == 7)
    {
//        for (int Ysite = 59; Ysite > imagestatus.OFFLine; Ysite--)
//        {
//            if(imageflag.ring_big_small==2)
//                Sideline_status_array[Ysite].midline = Sideline_status_array[Ysite].leftline + Half_Bend_Wide[Ysite];
//            if(Sideline_status_array[Ysite].midline >= 79)
//            {
//                Sideline_status_array[Ysite].midline = 79;
//                imagestatus.OFFLine=Ysite-1;
//                break;
//            }
//        }
    }

        //大圆环出环 补线
    if (imageflag.image_element_rings_flag == 8 && imageflag.ring_big_small == 1)  //大圆环
    {
        Repair_Point_Xsite = 60;
        Repair_Point_Ysite = 0;
        for (int Ysite = 50; Ysite > 5; Ysite--)
        {
            if (ex_mt9v03x_binarizeImage[Ysite][57] != 0 && ex_mt9v03x_binarizeImage[Ysite-1][57] == 0)
            {
                Repair_Point_Xsite = 57;
                Repair_Point_Ysite = Ysite-1;
                imagestatus.OFFLine = Ysite + 1;  //截止行重新规划
                        //  ips200_show_uint(200,200,Repair_Point_Ysite,2);
                break;
            }
        }
        for (int Ysite = 57; Ysite > Repair_Point_Ysite-3; Ysite--)         //补线
        {
            Sideline_status_array[Ysite].leftline = (Sideline_status_array[58].leftline - Repair_Point_Xsite) * (Ysite - 58) / (58 - Repair_Point_Ysite)  + Sideline_status_array[58].leftline;
            Sideline_status_array[Ysite].midline = (Sideline_status_array[Ysite].rightline + Sideline_status_array[Ysite].leftline) / 2;
        }
    }
        //小圆环出环 补线
    if (imageflag.image_element_rings_flag == 8 && imageflag.ring_big_small == 2)  //小圆环
    {
        Repair_Point_Xsite = 79;
        Repair_Point_Ysite = 0;
        for (int Ysite = 40; Ysite > 5; Ysite--)
        {
            if (ex_mt9v03x_binarizeImage[Ysite][58] != 0 && ex_mt9v03x_binarizeImage[Ysite-1][58] == 0)
            {
                Repair_Point_Xsite = 58;
                Repair_Point_Ysite = Ysite-1;
                imagestatus.OFFLine = Ysite + 1;  //截止行重新规划
                        //  ips200_show_uint(200,200,Repair_Point_Ysite,2);
                break;
            }
        }
        for (int Ysite = 55; Ysite > Repair_Point_Ysite-3; Ysite--)         //补线
        {
            Sideline_status_array[Ysite].leftline = (Sideline_status_array[58].leftline - Repair_Point_Xsite) * (Ysite - 58) / (58 - Repair_Point_Ysite)  + Sideline_status_array[58].leftline;
            Sideline_status_array[Ysite].midline = (Sideline_status_array[Ysite].rightline + Sideline_status_array[Ysite].leftline) / 2;
        }
    }
        //已出环 半宽处理
    if (imageflag.image_element_rings_flag == 9 || imageflag.image_element_rings_flag == 10)
    {
        for (int Ysite = 59; Ysite > imagestatus.OFFLine; Ysite--)
        {
            Sideline_status_array[Ysite].midline = Sideline_status_array[Ysite].leftline + Half_Road_Wide[Ysite];
        }
    }
}

//--------------------------------------------------------------
//  @name           Element_Judgment_Bend()
//  @brief          整个图像判断的子函数，用来判断左右弯道类型.
//  @parameter      void
//  @time
//  @Author         MRCHEN
//  Sample usage:   Element_Judgment_Bend();
//--------------------------------------------------------------
int Miss_Left_Num = 0 ;
int Miss_Right_Num = 0;
void Element_Judgment_Bend()
{
    if(imageflag.image_element_rings != 0 || imagestatus.OFFLine < 14 || imageflag.Zebra_Flag
            || imageflag.image_element_rings == 1 || imageflag.Out_Road == 1 || imageflag.RoadBlock_Flag == 1
            || imageflag.image_element_rings == 2)
        return;
        //ips114_show_int(20,20,imagestatus.Miss_Right_lines,3);
        //ips114_show_int(20,40,imagestatus.Miss_Left_lines,3);
        //ips114_show_int(20,80,imagestatus.OFFLine,3);

//        if(imagestatus.WhiteLine_R > 20 && imagestatus.WhiteLine_L < 5)
//        {
//            imageflag.Bend_Road = 1;
//            gpio_set_level(B0, 1);
////            Statu = Bend;
//        }
//
//        if(imagestatus.WhiteLine_L > 20 && imagestatus.WhiteLine_R < 5)
//        {
//            imageflag.Bend_Road = 2;
//            gpio_set_level(B0, 1);
////            Statu = Bend;
//        }
    if(Sideline_status_array[imagestatus.OFFLine+1].leftline > 30
            && imagestatus.Miss_Left_lines < 4
            && imagestatus.Miss_Right_lines > 15
            && Straight_Judge(1, imagestatus.OFFLine+2, 58) > 1)
    {

        imageflag.Bend_Road = 1;
//        BeeOn;
    }
    if(Sideline_status_array[imagestatus.OFFLine+1].rightline < 50
            && imagestatus.Miss_Right_lines < 4
            && imagestatus.Miss_Left_lines > 15
            && Straight_Judge(2, imagestatus.OFFLine+2, 58) > 1)
    {

        imageflag.Bend_Road = 2;
//        BeeOn;
    }
}

//--------------------------------------------------------------
//  @name           Element_Handle_Bend()
//  @brief          整个图像处理的子函数，用来处理左右弯道
//  @parameter      void
//  @time
//  @Author         MRCHEN
//  Sample usage:   Element_Handle_Bend();
//--------------------------------------------------------------
void Element_Handle_Bend()
{
    if(imagestatus.OFFLine<25 || Straight_Judge(imageflag.Bend_Road, 30, 50) < 1) { imageflag.Bend_Road = 0;}
    if(imageflag.Bend_Road==1)
    {
        for (int Ysite = 59; Ysite > imagestatus.OFFLine; Ysite--)
        {
            Sideline_status_array[Ysite].midline = Sideline_status_array[Ysite].leftline + Half_Bend_Wide[Ysite];
            if(Sideline_status_array[Ysite].midline >= 79)
                Sideline_status_array[Ysite].midline = 79;
        }
    }
    if(imageflag.Bend_Road==2)
    {
        for (int Ysite = 59; Ysite > imagestatus.OFFLine; Ysite--)
        {
            Sideline_status_array[Ysite].midline = Sideline_status_array[Ysite].rightline - Half_Bend_Wide[Ysite];
            if(Sideline_status_array[Ysite].midline < 0)
                Sideline_status_array[Ysite].midline = 0;
        }
    }


}

/*元素判断函数*/
void Scan_Element()
{
    if (       imageflag.Out_Road == 0   && imageflag.RoadBlock_Flag == 0
            && imageflag.Zebra_Flag == 0 && imageflag.image_element_rings == 0
            && imageflag.Ramp == 0       && imageflag.Bend_Road == 0
            &&  imageflag.straight_long== 0  )
    {
//        Element_Judgment_RoadBlock();       //路障
//        Element_Judgment_OutRoad();         //断路
        Element_Judgment_Left_Rings();      //左圆环
        Element_Judgment_Right_Rings();     //右圆环
        Element_Judgment_Zebra();           //斑马线
        Element_Judgment_Bend();            //弯道
        Straight_long_judge();              //长直道
    }
    if(imageflag.Bend_Road)
    {
        Element_Judgment_OutRoad();         //断路
        if(imageflag.Out_Road)
            imageflag.Bend_Road=0;
    }

    if(imageflag.Bend_Road)
    {
        Element_Judgment_Left_Rings();      //左圆环
        Element_Judgment_Right_Rings();     //右圆环
        if(imageflag.image_element_rings)
        {
            imageflag.Bend_Road=0;
        }
    }
//    if(imageflag.Bend_Road)
//    {
////        Element_Judgment_RoadBlock();         //路障
//        if(imageflag.RoadBlock_Flag)
//        {
//            Bend_Thruough_RoadBlock_Flag = 1;
//            if( imageflag.Bend_Road == 1 )
//                RoadBlock_Thruough_Flag = 2;
//            if( imageflag.Bend_Road == 2 )
//                RoadBlock_Thruough_Flag = 1;
//            imageflag.Bend_Road=0;
//        }
//    }

    if(imageflag.Bend_Road)
    {
        Element_Judgment_Zebra();           //斑马线
        if(imageflag.Zebra_Flag)
            imageflag.Bend_Road=0;
    }
}

/*元素处理函数*/
void Element_Handle()
{
    //Element_GetStraightLine_Bend();//直道修正

//    if(imageflag.RoadBlock_Flag == 1)
//        Element_Handle_RoadBlock();
//    else
        if(imageflag.Out_Road !=0)
        Element_Handle_OutRoad();
    else if (imageflag.image_element_rings == 1)
        Element_Handle_Left_Rings_Test();
    else if (imageflag.image_element_rings == 2)
        Element_Handle_Right_Rings();
    else if (imageflag.Zebra_Flag != 0 )
        Element_Handle_Zebra();
//    else if (imageflag.Ramp != 0)
//        Element_Handle_Ramp();
    else if(imageflag.straight_long)
        Straight_long_handle();
    else if(imageflag.Bend_Road !=0)
        Element_Handle_Bend();
    else if(imagestatus.WhiteLine >= 8) //十字处理
        Get_ExtensionLine();
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//  @name           Flag_init
//  @brief          标志位清0
//  @parameter      void
//  @time           2023年2月19日
//  @Author
//  Sample usage:   Flag_init();
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
void Flag_init(void)
{
    imageflag.Bend_Road = 0;
//    imageflag.Garage_Location = 0;
    imageflag.Ramp = 0;
    imageflag.Zebra_Flag = 0;
    imageflag.image_element_rings = 0;
    imageflag.image_element_rings_flag = 0;
    imageflag.straight_xie = 0;
    imageflag.straight_long = 0;
    imageflag.ring_big_small = 0;
    imageflag.RoadBlock_Flag = 0;
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//  函数简介        一键全初始化
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
void Init_overall(void)
{
    Image_CompressInit();
    Flag_init();
    mt9v03x_init();

}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//  @name           Image_Process
//  @brief          整个图像处理的主函数，里面包含了所有的图像处理子函数
//  @parameter      void
//  @time           2023年2月19日
//  @Author
//  Sample usage:   Image_Process();
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
void Image_Process(void)
{
    if(mt9v03x_finish_flag==1)
       {
        /***********/
        Get_BinaryImage();
        Get_BaseLine();
        Get_AllLine();
        if(!imageflag.Out_Road)
            Search_Border_OTSU(ex_mt9v03x_binarizeImage, LCDH, LCDW, LCDH - 2);//(MT9V03X_H/2-2)行位底行
        else
            imagestatus.OFFLineBoundary = 5;
        Scan_Element();
        Element_Handle();
        mt9v03x_finish_flag=0;
       }
}

