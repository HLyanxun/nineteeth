/*
 * remake_camera.c
 *
 *  Created on: 2024年7月4日
 *      Author: pc
 */
#include "zf_common_headfile.h"
#include "math.h"

//逆透视---------------------------------------------------------------------------------------------------------------
uint8_t *PerImg_ip[RESULT_ROW][RESULT_COL]={{0}};//存储图像地址
//二值化---------------------------------------------------------------------------------------------------------------
uint8 My_Threshold;
int My_Threshold_cha=10;  //二值化阈值补偿值
//处理用图像及处理相关------------------------------------------------------------------------------------------------------
uint8 image[image_h][image_w]={{0}};  //使用的图像
static uint8* PicTemp;                          //一个保存单行图像的指针变量
static int Ysite = 0, Xsite = 0;                //Ysite就是图像的行，Xsite就是图像的列。
static int BottomBorderRight = image_side_width,              //89行的右边界
BottomBorderLeft = 0,                           //89行的左边界
BottomCenter = 0;                               //89行的中点
uint8 ExtenLFlag = 0;                           //左边线是否需要补线的标志变量
uint8 ExtenRFlag = 0;                           //右边线是否需要补线的标志变量

//图像参数--------------------------------------------------------------------------------------------------------------
Sideline_status Sideline_status_array[90];
Image_Status imagestatus;
ImageFlagtypedef imageflag;

//--------------------------------------------------------------------------------------
// 函数简介     判断范围内的边线是否为直线
// 参数说明 dir    1代表判断左边线，2代表判断右边线，3代表判断中线
// 返回值
//--------------------------------------------------------------------------------------
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
    case 3:
        k = (float)(Sideline_status_array[start].midline - Sideline_status_array[end].midline) / (start - end);
        for (i = 0; i < end - start; i++)
        {
            Err = (Sideline_status_array[start].midline + k * i - Sideline_status_array[i + start].midline) * (Sideline_status_array[start].midline + k * i - Sideline_status_array[i + start].midline);
            Sum += Err;
        }
        S = Sum / (end - start);
        break;

    }
    return S;
}
//--------------------------------------------------------------------------------------
// 函数简介     便捷式一键初始化
//--------------------------------------------------------------------------------------
void ALL_init(void)
{
    ImagePerspective_Init();
    mt9v03x_init();
    tft180_init();
}
//--------------------------------------------------------------------------------------
// 函数简介     去畸变+逆透视处理初始化
// 备注信息     只需调用一次即可，随后使用ImageUsed指针即可
//--------------------------------------------------------------------------------------
void ImagePerspective_Init(void)
{
    static uint8_t BlackColor = 0;
    double change_un_Mat[3][3] ={{0.430435,-0.459130,40.776522},{0.000000,0.078913,6.162391},{0.000000,-0.004783,0.626522}};
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
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// 函数简介     显示摄像头巡线结果
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
void camera_tft180show(void)
{
    tft180_show_gray_image(0, 0, image[0], image_w, image_h, image_w, image_h, 0);
    if(track_show)
    {
        for(uint8 i=imagestatus.OFFLine;i<=image_bottom_value;i++)
            {
                LimitL(Sideline_status_array[Ysite].rightline);  //限幅
                LimitH(Sideline_status_array[Ysite].rightline);  //限幅
                LimitL(Sideline_status_array[Ysite].leftline);  //限幅
                LimitH(Sideline_status_array[Ysite].leftline);  //限幅
                LimitL(Sideline_status_array[Ysite].midline);  //限幅
                LimitH(Sideline_status_array[Ysite].midline);  //限幅
                tft180_draw_point(Sideline_status_array[i].leftline, i, RGB565_RED);
                tft180_draw_point(Sideline_status_array[i].rightline, i, RGB565_RED);
                tft180_draw_point(Sideline_status_array[i].midline, i, RGB565_RED);
                if(track_width_debug)
                {
                    tft180_draw_line((line_midpoint-track_width/2), 0, (line_midpoint-track_width/2), 90, RGB565_YELLOW);
                    tft180_draw_line((line_midpoint+track_width/2), 0, (line_midpoint+track_width/2), 90, RGB565_YELLOW);
                }
            }
    }

}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//  @name           get_Threshold  //指针
//  @brief          优化之后的的大津法。大津法就是一种能够算出一幅图像最佳的那个分割阈值的一种算法。
//  @brief          这个东西你们可以如果实在不能理解就直接拿来用，什么参数都不用修改，只要没有光照影响，那么算出来的这个阈值就一定可以得到一幅效果还不错的二值化图像。
//  @parameter      ex_mt9v03x_binarizeImage  原始的灰度图像数组
//  @parameter      col    图像的宽（图像的列）
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
    My_Threshold   = (int)get_Threshold()    + My_Threshold_cha;  //1ms
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

    for(int i = image_h-3; i >= 0 ; i-- )//从逆透视图像数组里面提出,在进行一次阈值梯度递减的二值化
    {

        for(int j=0;j< image_w-2;j++)
        {

            if( ImageUsed[i][j] > My_Threshold)
                image[i+1][j+1]=255;
            else{
                image[i+1][j+1]=0;
            }
        }


    }

    uint16 N_zaoddian=0;
    for(int i=0;i<90;i++)       //白噪点 去除
        for(int j=1;j<90;j++)
        {
            if( image[90-i][j] == 255 )
            {
                N_zaoddian =                 image[90-i-1][j]
                           + image[90-i][j-1]               + image[90-i]  [j+1]
                                            +image[90-i+1][j] ;
                if( N_zaoddian <  255 * 2 )
                {
                    image[90-i][j] = 0 ;
                }
            }
            else
            {
                N_zaoddian =                 image[90-i-1][j]
                           + image[90-i][j-1]               + image[90-i]  [j+1]
                                            +image[90-i+1][j] ;
                if( N_zaoddian >  255 * 2 )
                {
                    image[90-i][j] = 255 ;
                }
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
    for (i = L; i <= H; i++)                    //从左往右扫
    {
      if (*(p + i) != 0 && *(p + i - 1) == 0)   //如果有黑白跳变    1是白 0是黑
      {
        Q->point = i;                           //那就把这个列记录下来作为左边线
        Q->type = 'T';                          //并且把这一行当作是正常跳变，边线类型记为T，即边线正常。
        break;                                  //找到了就跳出循环不找了
      }
      else if (i == H)                    //要是扫到最后都没找到
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
    for (i = H; i >= L; i--)                    //从右往左扫
    {
      if (*(p + i) != 0 && *(p + i + 1) == 0)   //如果有黑白跳变    1是白 0是黑
      {
        Q->point = i;                           //那就把这个列记录下来作为右边线
        Q->type = 'T';                          //并且把这一行当作是正常跳变，边线类型记为T，即边线正常。
        break;                                  //找到了就跳出循环不找了
      }
      else if (i == L)                    //要是扫到最后都没找到
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
//  @brief          用遍历的方法得到图像最底下五行（image_bottom_value-55行）的边线和中线信息。这五行边线和中线信息的准确度非常的重要，直接影响到整幅图像的处理结果。
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

    PicTemp = image[image_bottom_value];                                                //让PicTemp这个指针变量指向图像数组的Pixle[image_bottom_value]
//    for (Xsite = line_midpoint; Xsite < image_side_width; Xsite++)                   //假设line_midpoint是中心列，从中心列开始一列一列的往右边搜索右边线
//    {
//      if (*(PicTemp + Xsite) == 0 && *(PicTemp + Xsite + 1) == 0)       //如果连续出现了两个黑点，说没找到了边线。
//      {
//        BottomBorderRight = Xsite;                                      //把这一列记录下来作为这一行的右边线
//        break;                                                          //跳出循环
//      }
//      else if (Xsite == (image_side_width-1))                                             //如果找到了第58列都还没出现黑点，说明这一行的边线有问题。
//      {
//        BottomBorderRight = (image_side_width-1);                                         //所以我这里的处理就是，直接假设图像最右边的那一列（第79列）就是这一行的右边线。
//        break;                                                          //跳出循环
//      }
//    }
//
//    for (Xsite = line_midpoint; Xsite > 0; Xsite--)                    //假设39是中心列，从中心列开始一列一列的往左边搜索左边线
//    {
//      if (*(PicTemp + Xsite) == 0 && *(PicTemp + Xsite - 1) == 0)       //如果连续出现了两个黑点，说没找到了边线。
//      {
//        BottomBorderLeft = Xsite;                                       //把这一列记录下来作为这一行的左边线
//        break;                                                          //跳出循环
//      }
//      else if (Xsite == 1)                                              //如果找到了第1列都还没出现黑点，说明这一行的边线有问题。
//      {
//        BottomBorderLeft = 0;                                           //所以我这里的处理就是，直接假设图像最左边的那一列（第0列）就是这一行的左边线。
//        break;                                                          //跳出循环
//      }
//    }
    /*确定左边线*/
    uint8 temp_min=(line_midpoint-track_width/2)-ImageScanInterval;
    uint8 temp_max=(line_midpoint-track_width/2)+ImageScanInterval;
    for (uint8 i = temp_min; i <=temp_max ; i++)                    //从左往右扫
      {
        if (*(PicTemp + i) != 0 && *(PicTemp + i - 1) == 0)   //如果有黑白跳变    1是白 0是黑
        {
            BottomBorderLeft = i;                           //那就把这个列记录下来作为左边线
          break;                                  //找到了就跳出循环不找了
        }
        else if (i == temp_max)                    //要是扫到最后都没找到
        {
          if (*(PicTemp + (temp_min + temp_max) / 2) != 0)            //并且扫描区间的中间是白像素点
          {
              for(uint8 j=(temp_min-1);j>0;j--)
              {
                  if (*(PicTemp + i) != 0 && *(PicTemp + i - 1) == 0)   //如果有黑白跳变    1是白 0是黑
                  {
                      BottomBorderLeft = i;                           //那就把这个列记录下来作为左边线
                      break;                                  //找到了就跳出循环不找了
                  }
                  if(j == 1)
                  {
                      BottomBorderLeft = 0;
                      break;
                  }
              }
              break;
          }
          else                                    //要是扫到最后都没找到，并且扫描区间的中间是黑像素点
          {
              for(uint8 j=(temp_max+1);j<line_midpoint;j++)
              {
                  if (*(PicTemp + i) != 0 && *(PicTemp + i - 1) == 0)   //如果有黑白跳变    1是白 0是黑
                  {
                      BottomBorderLeft = i;                           //那就把这个列记录下来作为左边线
                      break;                                  //找到了就跳出循环不找了
                  }
                  if(j == (line_midpoint-1))
                  {
                      BottomBorderLeft = (line_midpoint-1);
                      break;
                  }
              }
          }
        }
      }
    /*确定右边线*/
    temp_min=(line_midpoint+track_width/2)-ImageScanInterval;
    temp_max=(line_midpoint+track_width/2)+ImageScanInterval;
    for (uint8 i = temp_max; i <=temp_min ; i--)                    //从右往左扫
          {
            if (*(PicTemp + i) != 0 && *(PicTemp + i + 1) == 0)   //如果有黑白跳变    1是白 0是黑
            {
                BottomBorderRight = i;                           //那就把这个列记录下来作为左边线
              break;                                  //找到了就跳出循环不找了
            }
            else if (i == temp_min)                    //要是扫到最后都没找到
            {
              if (*(PicTemp + (temp_min + temp_max) / 2) != 0)            //并且扫描区间的中间是白像素点
              {
                  for(uint8 j=(temp_max+1);j<=image_side_width;j++)
                  {
                      if (*(PicTemp + i) != 0 && *(PicTemp + i + 1) == 0)   //如果有黑白跳变    1是白 0是黑
                      {
                          BottomBorderRight = i;                           //那就把这个列记录下来作为左边线
                          break;                                  //找到了就跳出循环不找了
                      }
                      if(j == image_side_width)
                      {
                          BottomBorderRight = image_side_width;
                          break;
                      }
                  }
                  break;
              }
              else                                    //要是扫到最后都没找到，并且扫描区间的中间是黑像素点
              {
                  for(uint8 j=(temp_min-1);j>line_midpoint;j++)
                  {
                      if (*(PicTemp + i) != 0 && *(PicTemp + i + 1) == 0)   //如果有黑白跳变    1是白 0是黑
                      {
                          BottomBorderRight = i;                           //那就把这个列记录下来作为左边线
                          break;                                  //找到了就跳出循环不找了
                      }
                      if(j == (line_midpoint+1))
                      {
                          BottomBorderRight = (line_midpoint+1);
                          break;
                      }
                  }
              }
            }
          }

    BottomCenter =(BottomBorderLeft + BottomBorderRight) / 2;           //根据左右边界计算出第89行的中线
    Sideline_status_array[image_bottom_value].leftline = BottomBorderLeft;                        //把第89行的左边界存储进数组，注意看ImageDeal这个数字的下标，是不是正好对应89。
    Sideline_status_array[image_bottom_value].rightline = BottomBorderRight;                      //把第89行的右边界存储进数组，注意看ImageDeal这个数字的下标，是不是正好对应89。
    Sideline_status_array[image_bottom_value].midline = BottomCenter;                                //把第89行的中线存储进数组，    注意看ImageDeal这个数字的下标，是不是正好对应89。
    Sideline_status_array[image_bottom_value].wide = BottomBorderRight - BottomBorderLeft;          //把第89行的赛道宽度存储数组，注意看ImageDeal这个数字的下标，是不是正好对应89。
    Sideline_status_array[image_bottom_value].IsLeftFind = 'T';                                     //记录第89行的左边线类型为T，即正常找到左边线。
    Sideline_status_array[image_bottom_value].IsRightFind = 'T';                                    //记录第89行的右边线类型为T，即正常找到右边线。

    /****************************************************End*******************************************************************************/
    /**************************************遍历搜索图像最底行（59行）左右边线从而确定中线的过程 ********************************************************************/



    /**************************************在第59行中线已经确定的情况下确定58-54这四行中线的过程 ******************************************/
    /****************************************************Begin*****************************************************************************/
    /*
         * 下面几行的的搜线过程我就不再赘述了，根据我的注释把第89行的搜线过程理解好，
         * 那么88到84行的搜线就完全没问题，是一模一样的逻辑和过程。
     */
    for (Ysite = (image_side_width-1); Ysite > (image_side_width-5); Ysite--)
    {
        PicTemp = image[Ysite];
//        for(Xsite = Sideline_status_array[Ysite + 1].midline; Xsite < image_side_width;Xsite++)
//        {
//          if(*(PicTemp + Xsite) == 0 && *(PicTemp + Xsite + 1) == 0)
//          {
//            Sideline_status_array[Ysite].rightline = Xsite;
//            break;
//          }
//          else if (Xsite == (image_side_width-1))
//          {
//            Sideline_status_array[Ysite].rightline = image_side_width;
//            break;
//          }
//        }
//
//        for (Xsite = Sideline_status_array[Ysite + 1].midline; Xsite > 0;Xsite--)
//        {
//          if (*(PicTemp + Xsite) == 0 && *(PicTemp + Xsite - 1) == 0)
//          {
//            Sideline_status_array[Ysite].leftline = Xsite;
//            break;
//          }
//          else if (Xsite == 1)
//          {
//            Sideline_status_array[Ysite].leftline = 0;
//            break;
//          }
//        }
        /*确定左边线*/
         temp_min=(Sideline_status_array[Ysite + 1].midline-track_width/2)-ImageScanInterval;
         temp_max=(Sideline_status_array[Ysite + 1].midline-track_width/2)+ImageScanInterval;
            for (uint8 i = temp_min; i <=temp_max ; i++)                    //从左往右扫
              {
                if (*(PicTemp + i) != 0 && *(PicTemp + i - 1) == 0)   //如果有黑白跳变    1是白 0是黑
                {
                    BottomBorderLeft = i;                           //那就把这个列记录下来作为左边线
                  break;                                  //找到了就跳出循环不找了
                }
                else if (i == temp_max)                    //要是扫到最后都没找到
                {
                  if (*(PicTemp + (temp_min + temp_max) / 2) != 0)            //并且扫描区间的中间是白像素点
                  {
                      for(uint8 j=(temp_min-1);j>0;j--)
                      {
                          if (*(PicTemp + i) != 0 && *(PicTemp + i - 1) == 0)   //如果有黑白跳变    1是白 0是黑
                          {
                              BottomBorderLeft = i;                           //那就把这个列记录下来作为左边线
                              break;                                  //找到了就跳出循环不找了
                          }
                          if(j == 1)
                          {
                              BottomBorderLeft = 0;
                              break;
                          }
                      }
                      break;
                  }
                  else                                    //要是扫到最后都没找到，并且扫描区间的中间是黑像素点
                  {
                      for(uint8 j=(temp_max+1);j<line_midpoint;j++)
                      {
                          if (*(PicTemp + i) != 0 && *(PicTemp + i - 1) == 0)   //如果有黑白跳变    1是白 0是黑
                          {
                              BottomBorderLeft = i;                           //那就把这个列记录下来作为左边线
                              break;                                  //找到了就跳出循环不找了
                          }
                          if(j == (line_midpoint-1))
                          {
                              BottomBorderLeft = (line_midpoint-1);
                              break;
                          }
                      }
                  }
                }
              }
            /*确定右边线*/
            temp_min=(Sideline_status_array[Ysite + 1].midline+track_width/2)-ImageScanInterval;
            temp_max=(Sideline_status_array[Ysite + 1].midline+track_width/2)+ImageScanInterval;
            for (uint8 i = temp_max; i <=temp_min ; i--)                    //从右往左扫
                  {
                    if (*(PicTemp + i) != 0 && *(PicTemp + i + 1) == 0)   //如果有黑白跳变    1是白 0是黑
                    {
                        BottomBorderRight = i;                           //那就把这个列记录下来作为左边线
                      break;                                  //找到了就跳出循环不找了
                    }
                    else if (i == temp_min)                    //要是扫到最后都没找到
                    {
                      if (*(PicTemp + (temp_min + temp_max) / 2) != 0)            //并且扫描区间的中间是白像素点
                      {
                          for(uint8 j=(temp_max+1);j<=image_side_width;j++)
                          {
                              if (*(PicTemp + i) != 0 && *(PicTemp + i + 1) == 0)   //如果有黑白跳变    1是白 0是黑
                              {
                                  BottomBorderRight = i;                           //那就把这个列记录下来作为左边线
                                  break;                                  //找到了就跳出循环不找了
                              }
                              if(j == image_side_width)
                              {
                                  BottomBorderRight = image_side_width;
                                  break;
                              }
                          }
                          break;
                      }
                      else                                    //要是扫到最后都没找到，并且扫描区间的中间是黑像素点
                      {
                          for(uint8 j=(temp_min-1);j>line_midpoint;j++)
                          {
                              if (*(PicTemp + i) != 0 && *(PicTemp + i + 1) == 0)   //如果有黑白跳变    1是白 0是黑
                              {
                                  BottomBorderRight = i;                           //那就把这个列记录下来作为左边线
                                  break;                                  //找到了就跳出循环不找了
                              }
                              if(j == (line_midpoint+1))
                              {
                                  BottomBorderRight = (line_midpoint+1);
                                  break;
                              }
                          }
                      }
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
  imageflag.mid_choose=0;
  uint8 mid_choose_value=0;
  for (Ysite = (image_bottom_value-5) ; Ysite > imagestatus.OFFLine; Ysite--)                            //前5行在Get_BaseLine()中已经处理过了，现在从55行处理到自己设定的不处理行OFFLine。
  {                                                                                  //因为太前面的图像可靠性不搞，所以OFFLine的设置很有必要，没必要一直往上扫到第0行。
    PicTemp = image[Ysite];
    JumpPointtypedef JumpPoint[2];                                                   // JumpPoint[0]代表左边线，JumpPoint[1]代表右边线。

  /******************************扫描本行的右边线******************************/
    int IntervalLow  = Sideline_status_array[Ysite + 1].rightline  - ImageScanInterval;               //从上一行的右边线加减Interval对应的列开始扫描本行，Interval一般取5，当然你为了保险起见可以把这个值改的大一点。
    int IntervalHigh = Sideline_status_array[Ysite + 1].rightline + ImageScanInterval;              //正常情况下只需要在上行边线左右5的基础上（差不多10列的这个区间）去扫线，一般就能找到本行的边线了，所以这个值其实不用太大。
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
    if(JumpPoint[0].type != 'T' || JumpPoint[1].type!= 'T')mid_choose_value++;
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
    if(mid_choose_value>10)imageflag.mid_choose=1;
//    if(Sideline_status_array[Ysite].wide< track_width)imageflag.mid_choose=1;
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
    if (Sideline_status_array[Ysite].IsRightFind == 'W' && Ysite > 10 && Ysite < (image_side_width-10))
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
//            if (D_R < 0)
//            {
//                ExtenRFlag = 'F';
//            }
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
    if (Sideline_status_array[Ysite].IsLeftFind == 'W' && Ysite > 10 && Ysite < (image_side_width-10) )
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
//            if (D_L < 0)
//            {
//                ExtenLFlag = 'F';
//            }
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
     if (Sideline_status_array[Ysite].IsLeftFind == 'W' && Ysite < (image_bottom_value-4) )
     {
          imagestatus.Miss_Left_lines++;
     }
     if (Sideline_status_array[Ysite].IsRightFind == 'W'&& Ysite < (image_bottom_value-4) )
     {
          imagestatus.Miss_Right_lines++;
     }

      LimitL(Sideline_status_array[Ysite].leftline);   //限幅
      LimitH(Sideline_status_array[Ysite].leftline);   //限幅
      LimitL(Sideline_status_array[Ysite].rightline);  //限幅
      LimitH(Sideline_status_array[Ysite].rightline);  //限幅

      Sideline_status_array[Ysite].wide =Sideline_status_array[Ysite].rightline - Sideline_status_array[Ysite].leftline;
//      Sideline_status_array[Ysite].midline =(Sideline_status_array[Ysite].rightline + Sideline_status_array[Ysite].leftline) / 2;

      if (Sideline_status_array[Ysite].wide <= 7 )
      {
          imagestatus.OFFLine = Ysite + 1;
          break;
      }
      else if (Sideline_status_array[Ysite].rightline <= 10||Sideline_status_array[Ysite].leftline >= (image_bottom_value-10))
      {
          imagestatus.OFFLine = Ysite + 1;
          break;
      }
      /************************************都处理完之后，其他的一些数据整定操作*************************************************/
  }
  for (Ysite = imagestatus.OFFLine ; Ysite <= (image_bottom_value-5); Ysite++){
      int ysite = 0;
      uint8 L_found_point = 0;
      uint8 R_found_point = 0;
      /*左边线无边行的标定*/
      if (Sideline_status_array[Ysite].IsLeftFind == 'W' && Ysite < 10)
      {
          ytemp_W_L = Ysite - 2;
                  for (ysite = Ysite - 1; ysite < Ysite - 15; ysite--)
                  {
                    if (Sideline_status_array[ysite].IsLeftFind == 'T')
                      {
                        L_found_point++;
                      }
                  }
                  if (L_found_point > 8)              //找到基准斜率边  做延长线重新确定无边
                  {
                    D_L = ((float)(Sideline_status_array[Ysite - 3].leftline -Sideline_status_array[Ysite - L_found_point].leftline)) /((float)(L_found_point - 3));
                    if (D_L > 0)
                    {
                      L_Found_T = 'T';
                    }
                    else
                    {
                      L_Found_T = 'F';
          //            if (D_L < 0)
          //            {
          //                ExtenLFlag = 'F';
          //            }
                    }
                  }
                  if (L_Found_T == 'T')
                       {
                           Sideline_status_array[Ysite].leftline =Sideline_status_array[ytemp_W_L].leftline + D_L * (ytemp_W_L - Ysite);
                       }

                       LimitL(Sideline_status_array[Ysite].leftline);  //限幅
                       LimitH(Sideline_status_array[Ysite].leftline);  //限幅
      }
      /*右边线无边行的标定*/
      if (Sideline_status_array[Ysite].IsRightFind == 'W' &&  Ysite > (image_side_width-10))
         {
             ytemp_W_R = Ysite - 2;
             for (ysite = Ysite - 1; ysite < Ysite - 15; ysite--)
             {
               if (Sideline_status_array[ysite].IsRightFind =='T')
               {
                   R_found_point++;
               }
             }
             if (R_found_point >8)
             {
               D_R = ((float)(Sideline_status_array[Ysite - R_found_point].rightline - Sideline_status_array[Ysite - 3].rightline)) /((float)(R_found_point - 3));
               if (D_R > 0)
               {
                 R_Found_T ='T';
               }
               else
               {
                 R_Found_T = 'F';
     //            if (D_R < 0)
     //            {
     //                ExtenRFlag = 'F';
     //            }
               }
             }

           if (R_Found_T == 'T')
           {
             Sideline_status_array[Ysite].rightline =Sideline_status_array[ytemp_W_R].rightline -D_R * (ytemp_W_R - Ysite);  //如果找到了 那么以基准行做延长线
           }
           LimitL(Sideline_status_array[Ysite].rightline);  //限幅
           LimitH(Sideline_status_array[Ysite].rightline);  //限幅
         }
      /*中线的标定*/
      if(imageflag.mid_choose)
      {
          if(default_side_choose){Sideline_status_array[Ysite].midline =Sideline_status_array[Ysite].leftline + (track_width / 2);}
          else {Sideline_status_array[Ysite].midline =Sideline_status_array[Ysite].rightline -(track_width / 2);}
      }
      else {
          if(default_side_choose){Sideline_status_array[Ysite].midline =Sideline_status_array[Ysite].rightline -(track_width / 2);}
          else {Sideline_status_array[Ysite].midline =Sideline_status_array[Ysite].leftline + (track_width / 2);}
    }
  }
}

//------------------------------------------------------------------------------------------------------
// 函数简介     环岛检测
//------------------------------------------------------------------------------------------------------
void Element_Judgment(void)
{

}

//------------------------------------------------------------------------------------------------------
// 函数简介     图像处理总函数
//------------------------------------------------------------------------------------------------------
void Camera_tracking(void)
{
    Binaryzation();
    image_draw();
    Get_BaseLine();
    Get_AllLine();
    camera_tft180show();
}
