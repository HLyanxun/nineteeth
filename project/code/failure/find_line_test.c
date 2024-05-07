/*
 * better_fix_mt9v034.c
 *
 *  Created on: 2024年3月17日
 *      Author: pc
 */
#include "zf_common_headfile.h"
uint8 ex_mt9v03x_binarizeImage[MT9V03X_H/2][MT9V03X_W/2];   //摄像头二值化数据存储数组
int16 threshold_fix;                                          //二值化阈值补正
int16 exposure_time;
int ImageScanInterval=10;                                    //扫边的范围

uint8 ex_threshold=0;                                         //大津法二值化阈值

uint8 Ring_Help_Flag = 0;                                   //进环辅助标志

int Right_RingsFlag_Point1_Ysite, Right_RingsFlag_Point2_Ysite; //右圆环判断的两点纵坐标
int Left_RingsFlag_Point1_Ysite, Left_RingsFlag_Point2_Ysite;   //左圆环判断的两点纵坐标
int Point_Xsite,Point_Ysite;                                //拐点横纵坐标
int Repair_Point_Xsite,Repair_Point_Ysite;                  //补线点横纵坐标
int THRESHOLD;

uint8 ExtenLFlag = 0;                                       //左边线是否需要补线的标志变量
uint8 ExtenRFlag = 0;                                       //右边线是否需要补线的标志变量
uint16 wide_sum;//////////////////
static float BottomBorderRight = (MT9V03X_W/2-10),              //(MT9V03X_H/2-1)行的右边界
BottomBorderLeft = 0,                           //(MT9V03X_H/2-1)行的左边界
Bottommidline = 0;                               //(MT9V03X_H/2-1)行的中点
//uint8 Garage_Location_Flag = 0;                 //判断库的次数
//
//uint16 Ramp_num,Zebra_num,Garage_length,mode;
//int16 Garage_num=0;
//int32 Element_encoder1 = 0,Element_encoder2 = 0;//坡道、斑马线、车库编码器计数

static float DetR = 0, DetL = 0;                            //存放补线斜率的变量
static int IntervalLow = 0, IntervalHigh = 0;               //扫描区间的上下限变量
static int TFSite = 0, left_FTSite = 0,right_FTSite = 0;    //补线计算斜率的时候需要用的存放行的变量。
static int ytemp = 0;                                       //存放行的临时变量
static uint8* Pixel;                                        //一个保存单行图像的指针

Sideline_status Sideline_status_array[MT9V03X_H/2];

Image_Status imagestatus;
Image_Flag imageflag;
ChangePoint changepoint={0,0};
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
    if (imageflag.Zebra_Flag != 0 || imageflag.image_element_rings != 0  )
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
    for(i=(MT9V03X_H/2-2);i>line_start;i--)
    {
        for(j=5;j<88;j++)
        {
            if(ex_mt9v03x_binarizeImage[i][j]==1)
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
            if(ex_mt9v03x_binarizeImage[i][j]==1)
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
//-----------------------------------------------------------------------------------------------------------------
// 函数简介     得到边线及边线的类型
// 参数说明             p        指向传进来数组的一个指针变量。
// 参数说明            type     只能是L或者是R，分别代表扫左边线和扫右边线。
// 参数说明             L        扫描的区间下限 ，也就是从哪一列开始扫。
// 参数说明             H        扫描的区间上限 ，也就是一直扫到哪一列。
// 参数说明             Q        是一个结构体指针变量，自己跳过去看看这个结构体里面的成员。
// 使用示例     Get_Border_And_SideType(Pixel, 'R', IntervalLow, IntervalHigh,&jumppoint[1])
// 备注信息     从Pixel(Pixel是个指针，指向一个数组)的IntervalLow列开始扫，扫到IntervalHigh列，然后把得到的边线所在的列和边线类型记录到JumpPoint结构体中。
//-----------------------------------------------------------------------------------------------------------------
void Get_Border_And_SideType(uint8* p,uint8 type,int L,int H,Jumppoint* Q)
{
  int i = 0;
  if (type == 'L')                              //如果Type是L(Left),则扫描左边线。
  {
    for (i = H; i >= L; i--)                    //从右往左扫
    {
      if (*(p + i) == 255 && *(p + i - 1) != 255)   //如果有黑白跳变    255是白 0是黑
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
      if (*(p + i) == 255 && *(p + i + 1) != 255)   //如果有黑白跳变    1是白 0是黑
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

//----------------------------------------------------------------------------------------------------------------------
// 函数简介     通过大津法计算二值化阈值
// 使用实例 ·int bestThreshold = otsuThreshold(inputImage);
// 备注信息     通过大津法初步优化的二值化阈值算法，或许仍有改进空间，放在定时器中，定时执行(暂不清楚最佳定时时间，建议2ms)
//----------------------------------------------------------------------------------------------------------------------

int16 otsuThreshold( uint8 inputImage[MT9V03X_H][MT9V03X_W])
{
    int histogram[256] = {0};
    float sum = 0.0;
    float sumB = 0.0;
    int wB = 0;
    int wF = 0;
    float varMax = 0.0;
    int16 threshold = 0;

    // 计算像素值的直方图
    for (int i = 0; i < MT9V03X_H; i++)
    {
        for (int j = 0; j < MT9V03X_W; j++)
        {
            histogram[inputImage[i][j]]++;
        }
    }

    // 计算总和
    for (int i = 0; i < 256; i++) {
        sum += i * histogram[i];
    }

    int total = MT9V03X_W * MT9V03X_H;

    for (int i = 0; i < 256; i++) {
        wB += histogram[i];
        if (wB == 0) continue;
        wF = total - wB;

        if (wF == 0) break;

        sumB += (float)(i * histogram[i]);

        float mB = sumB / wB;
        float mF = (sum - sumB) / wF;

        float varBetween = (float)wB * (float)wF * (mB - mF) * (mB - mF);

        if (varBetween > varMax) {
            varMax = varBetween;
            threshold = i;
        }
    }

    return threshold;
}

//---------------------------------------------------------------------------------------
// 函数简介     将摄像头数据进行二值化后储存到新的二维数组中
// 参数说明      x_s     横坐标起始值（左上）
//        y_s     纵坐标起始值（左上）
//        x_e     横坐标终点值（右下）
//        y_s     纵坐标终点值（右下）
// 使用实例     binarizeImage(20,20,40,40);
//---------------------------------------------------------------------------------------
void binarizeImage(uint8 x_s,uint8 y_s,uint8 x_e,uint8 y_e)
{
    if(mt9v03x_finish_flag)
         {
          ex_threshold=otsuThreshold(mt9v03x_image);
          THRESHOLD=ex_threshold+threshold_fix;
          for(uint8 i= x_s;i < x_e;i++)
          {
              for(uint8 j=y_s;j<y_e;j++)
              {
                 (ex_mt9v03x_binarizeImage[i/2][j/2])=(mt9v03x_image[i][j] > THRESHOLD)? (255) : (0);
              }
          }

//         for (int i = 0; i < MT9V03X_H; i+=2) {
//             for (int j = 0; j < MT9V03X_W; j+=2) {
//                 if (mt9v03x_image[i][j] > THRESHOLD) {
//                     ex_mt9v03x_binarizeImage[i/2][j/2] = 255; // 设置为白色
//                 } else {
//                     ex_mt9v03x_binarizeImage[i/2][j/2] = 0;   // 设置为黑色
//                 }
//             }
//         }
         }

}
//---------------------------------------------------------------------------------------
// 函数简介     对二值化数据进行降噪处理（内存占用过大，暂时不建议使用）
//---------------------------------------------------------------------------------------
//void medianFilter(uint8 binaryImage[IMAGE_HEIGHT][IMAGE_WIDTH]) {
//    uint8 tempImage[IMAGE_HEIGHT][IMAGE_WIDTH];
//
//    for (int row = 1; row < IMAGE_HEIGHT - 1; row += 2) {
//        for (int col = 1; col < IMAGE_WIDTH - 1; col += 2) {
//            // 获取3x3的像素值
//            uint8 pixels[9] = {
//                binaryImage[row-1][col-1], binaryImage[row-1][col], binaryImage[row-1][col+1],
//                binaryImage[row][col-1], binaryImage[row][col], binaryImage[row][col+1],
//                binaryImage[row+1][col-1], binaryImage[row+1][col], binaryImage[row+1][col+1]
//            };
//
//            // 对像素值进行排序
//            for (int i = 0; i < 9; i++) {
//                for (int j = i + 1; j < 9; j++) {
//                    if (pixels[i] > pixels[j]) {
//                        uint8 temp = pixels[i];
//                        pixels[i] = pixels[j];
//                        pixels[j] = temp;
//                    }
//                }
//            }
//
//            // 取中值作为当前像素的值
//            tempImage[row][col] = pixels[4];
//        }
//    }
//
//    // 将处理后的图像数据拷贝回原数组
//    for (int i = 0; i < IMAGE_HEIGHT; i++) {
//        for (int j = 0; j < IMAGE_WIDTH; j++) {
//            binaryImage[i][j] = tempImage[i][j];
//        }
//    }
//}
//--------------------------------------------------------------------------------------------------------------------------
// 函数简介     标志位初始化
//--------------------------------------------------------------------------------------------------------------------------
void flag_init(void)
{
    imageflag.Bend_Road=0;
    imageflag.CrossRoad_Flag=0;
    imageflag.Garage_Location=0;
    imageflag.Out_Road=0;
    imageflag.Ramp=0;
    imageflag.RoadBlock_Flag=0;
    imageflag.Zebra_Flag=0;
    imageflag.image_element_rings=0;
    imageflag.image_element_rings_flag=0;
    imageflag.ring_big_small=0;
    imageflag.stop_flag=0;
    imageflag.straight_long=0;
    imageflag.straight_xie=0;
}

//--------------------------------------------------------------------------------------------------------------------------
// 函数简介    横向寻线修复版(底边寻找)
//--------------------------------------------------------------------------------------------------------------------------
void baseline_get(void)
{
    /*下面进行底边中线，边线的搜索*/

    Pixel=ex_mt9v03x_binarizeImage[(MT9V03X_H/2)-1];
        for(uint8 i=((MT9V03X_H/4)-2);i<MT9V03X_W/2-2;i++)
            {
                if (*(Pixel + i) == 0 && *(Pixel + i + 1) == 0)       //如果连续出现了两个黑点，说没找到了边线。
                {
                    BottomBorderRight = i;                                      //把这一列记录下来作为这一行的右边线
                    break;                                                          //跳出循环
                }
                else if (i == MT9V03X_W/2-3)                                             //如果找到了第(MT9V03X_H/2-2)列都还没出现黑点，说明这一行的边线有问题。
                {
                    BottomBorderRight = MT9V03X_W/2-3;                                         //所以我这里的处理就是，直接假设图像最右边的那一列（第(MT9V03X_W/2-11)列）就是这一行的右边线。
                    break;                                                          //跳出循环
                }
            }
        for(uint8 i=((MT9V03X_H/4)-2);i>0;i--)
            {
                if (*(Pixel + i) == 0 && *(Pixel + i - 1) == 0)       //如果连续出现了两个黑点，说没找到了边线。
                {
                    BottomBorderLeft = i;                                      //把这一列记录下来作为这一行的右边线
                    break;                                                          //跳出循环
                }
                else if (i == MT9V03X_W/2-3)                                             //如果找到了第(MT9V03X_H/2-2)列都还没出现黑点，说明这一行的边线有问题。
                {
                    BottomBorderLeft = 0;                                         //所以我这里的处理就是，直接假设图像最右边的那一列（第(MT9V03X_W/2-11)列）就是这一行的右边线。
                    break;                                                          //跳出循环
                }
            }


        Bottommidline =(BottomBorderLeft + BottomBorderRight) / 2.0;
        Sideline_status_array[(MT9V03X_H/2)-1].leftline = BottomBorderLeft;                        //把第(MT9V03X_H/2-1)行的左边界存储进数组，注意看Sideline_status_array这个数字的下标，是不是正好对应(MT9V03X_H/2-1)。
        Sideline_status_array[(MT9V03X_H/2)-1].rightline = BottomBorderRight;                      //把第(MT9V03X_H/2-1)行的右边界存储进数组，注意看Sideline_status_array这个数字的下标，是不是正好对应(MT9V03X_H/2-1)。
        Sideline_status_array[(MT9V03X_H/2)-1].midline=Bottommidline;
        Sideline_status_array[(MT9V03X_H/2)-1].wide=(BottomBorderRight-BottomBorderLeft);
        Sideline_status_array[(MT9V03X_H/2)-1].De_m=(Bottommidline-(MT9V03X_W/4-2));
        Sideline_status_array[(MT9V03X_H/2)-1].IsLeftFind = 'T';                                     //记录第(MT9V03X_H/2-1)行的左边线类型为T，即正常找到左边线。
        Sideline_status_array[(MT9V03X_H/2)-1].IsRightFind = 'T';                                    //记录第(MT9V03X_H/2-1)行的右边线类型为T，即正常找到右边线。
        /*根据搜索到的底边中线，扫描(MT9V03X_H/2-2)到54行*/
    for(uint8 j=((MT9V03X_H/2)-2);j>((MT9V03X_H/2)-6);j--)
    {
        Pixel=ex_mt9v03x_binarizeImage[j];
    for(uint8 i=Sideline_status_array[j+1].midline;i<MT9V03X_W/2-2;i++)
        {
            if (*(Pixel + i) == 0 && *(Pixel + i + 1) == 0)       //如果连续出现了两个黑点，说没找到了边线。
            {
                BottomBorderRight = i;                                      //把这一列记录下来作为这一行的右边线
                break;                                                          //跳出循环
            }
            else if (i == MT9V03X_W/2-1)                                             //如果找到了第(MT9V03X_H/2-2)列都还没出现黑点，说明这一行的边线有问题。
            {
                BottomBorderRight = MT9V03X_W/2-2;                                         //所以我这里的处理就是，直接假设图像最右边的那一列（第(MT9V03X_W/2-11)列）就是这一行的右边线。
                break;                                                          //跳出循环
            }
        }
    for(uint8 i=Sideline_status_array[i+1].midline;i>0;i--)
        {
            if (*(Pixel + i) == 0 && *(Pixel + i - 1) == 0)       //如果连续出现了两个黑点，说没找到了边线。
            {
                BottomBorderLeft = i;                                      //把这一列记录下来作为这一行的右边线
                break;                                                          //跳出循环
            }
            else if (i == MT9V03X_W/2-2)                                             //如果找到了第(MT9V03X_H/2-2)列都还没出现黑点，说明这一行的边线有问题。
            {
                BottomBorderLeft = 0;                                         //所以我这里的处理就是，直接假设图像最右边的那一列（第(MT9V03X_W/2-11)列）就是这一行的右边线。
                break;                                                          //跳出循环
            }
        }
    Bottommidline =(BottomBorderLeft + BottomBorderRight) / 2;
    Sideline_status_array[j].midline=Bottommidline;
    Sideline_status_array[j].leftline=BottomBorderLeft;
    Sideline_status_array[j].rightline=BottomBorderRight;
    Sideline_status_array[j].wide=(BottomBorderRight-BottomBorderLeft);
    Sideline_status_array[j].De_m=(Bottommidline-(MT9V03X_W/4-2));
    Sideline_status_array[j].IsLeftFind = 'T';                                     //记录第(MT9V03X_H/2-1)行的左边线类型为T，即正常找到左边线。
    Sideline_status_array[j].IsRightFind = 'T';
    }

}
//-------------------------------------------------------------------------------------------------------------------------
// 函数简介     将所有的边线记录
//-------------------------------------------------------------------------------------------------------------------------
void allline_get(void)
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
    imagestatus.OFFLine=2;     //这个结构体成员我之所以在这里赋值，是因为我imagestatus结构体里面的成员太多了，但是暂时又只用到了OFFLine，所以我在哪用到它就在哪赋值。
    imagestatus.Miss_Right_lines = 0;
    imagestatus.WhiteLine = 0;
    imagestatus.Miss_Left_lines = 0;

    for(uint8 i=((MT9V03X_H/2)-6);i>imagestatus.OFFLine;i--)
    {
        static uint8 L_temp=0,R_temp=0;             //保存最后一次正常线的x值

        Pixel=ex_mt9v03x_binarizeImage[i];
        Jumppoint jumppoint[2];

        /******************************扫描本行的右边线******************************/
        IntervalLow  = Sideline_status_array[i + 1].rightline - ImageScanInterval;       //从上一行的右边线加减Interval对应的列开始扫描本行，Interval一般取5，当然你为了保险起见可以把这个值改的大一点。
        IntervalHigh = Sideline_status_array[i + 1].rightline + ImageScanInterval;       //正常情况下只需要在上行边线左右5的基础上（差不多10列的这个区间）去扫线，一般就能找到本行的边线了，所以这个值其实不用太大。
        LimitL(IntervalLow);                                                             //这里就是对传给Get_Border_And_SideType()函数的扫描区间进行一个限幅操作。
        LimitH(IntervalHigh);                                                            //假如上一行的边线是第2列，那你2-5=-3，-3是不是就没有实际意义了？怎么会有-3列呢？
        Get_Border_And_SideType(Pixel, 'R', IntervalLow, IntervalHigh,&jumppoint[1]);    //扫线用的一个子函数，自己跳进去看明白逻辑。
        /******************************扫描本行的右边线******************************/

        /******************************扫描本行的左边线******************************/
        IntervalLow =Sideline_status_array[i + 1].leftline  -ImageScanInterval;          //从上一行的左边线加减Interval对应的列开始扫描本行，Interval一般取5，当然你为了保险起见可以把这个值改的大一点。
        IntervalHigh =Sideline_status_array[i + 1].leftline +ImageScanInterval;          //正常情况下只需要在上行边线左右5的基础上（差不多10列的这个区间）去扫线，一般就能找到本行的边线了，所以这个值其实不用太大。
        LimitL(IntervalLow);                                                             //这里就是对传给Get_Border_And_SideType()函数的扫描区间进行一个限幅操作。
        LimitH(IntervalHigh);                                                            //假如上一行的边线是第2列，那你2-5=-3，-3是不是就没有实际意义了？怎么会有-3列呢？
        Get_Border_And_SideType(Pixel, 'L', IntervalLow, IntervalHigh,&jumppoint[0]);    //扫线用的一个子函数，自己跳进去看明白逻辑。
        /******************************扫描本行的左边*******************************/

        /*
                       下面的代码要是想看懂，想搞清楚我到底在赣神魔的话，
                        请务必把GetJumpPointFromDet()这个函数的逻辑看懂，
                        尤其是在这个函数里面，“T”、“W”、“H”三个标志代表什么，
                        一定要搞懂!!!不然的话，建议不要往下看了，不要折磨自己!!!
        */
        //如果本行的左边线属于不正常跳变，即这10个点都是白点。
        if (jumppoint[0].type =='W')                                                                     //如果本行的左边线属于不正常跳变，即这10个点都是白点。
        {
//            Sideline_status_array[i].leftline =Sideline_status_array[i + 1].leftline;                     //那么本行的左边线就采用上一行的边线。
            Sideline_status_array[i].leftline =L_temp;
        }
        else                                                                                             //如果本行的左边线属于T或者是H类别
        {
            Sideline_status_array[i].leftline = jumppoint[0].point;                                  //那么扫描到的边线是多少，我就记录下来是多少。
        }

         if (jumppoint[1].type == 'W')                                                                //如果本行的右边线属于不正常跳变，即这10个点都是白点。
         {
//             Sideline_status_array[i].rightline =Sideline_status_array[i + 1].rightline;            //那么本行的右边线就采用上一行的边线。
             Sideline_status_array[i].rightline = R_temp;
         }
         else                                                                                         //如果本行的右边线属于T或者是H类别
         {
             Sideline_status_array[i].rightline = jumppoint[1].point;                                 //那么扫描到的边线是多少，我就记录下来是多少。
         }
         if(jumppoint[0].type == 'T')L_temp=Sideline_status_array[i].leftline;
         if(jumppoint[1].type == 'T')R_temp=Sideline_status_array[i].rightline;


         Sideline_status_array[i].IsLeftFind =jumppoint[0].type;                                  //记录本行找到的左边线类型，是T？是W？还是H？这个类型后面是有用的，因为我还要进一步处理。
         Sideline_status_array[i].IsRightFind = jumppoint[1].type;                                //记录本行找到的右边线类型，是T？是W？还是H？这个类型后面是有用的，因为我还要进一步处理。
         /*
                            下面就开始对W和H类型的边线分别进行处理， 为什么要处理？
                            如果你看懂了GetJumpPointFromDet函数逻辑，明白了T W H三种类型分别对应什么情况，
                            那你就应该知道W和H类型的边线都属于非正常类型，那我是不是要处理？
                            这一部分的处理思路需要自己花大量时间好好的去琢磨，我在注释这里没法给你说清楚的。，
                            实在想不通就来问我吧！
          */

            /************************************重新确定大跳变(即H类)的边界*************************************/

         if (( Sideline_status_array[i].IsLeftFind == 'H' || Sideline_status_array[i].IsRightFind == 'H'))
             {
               /**************************处理左边线的大跳变***************************/
               if (Sideline_status_array[i].IsLeftFind == 'H')
               {
                 for (uint8 j = (Sideline_status_array[i].leftline + 1);j <= (Sideline_status_array[i].rightline - 1);j++)                                                           //左右边线之间重新扫描
                 {
                   if ((*(Pixel + j) == 0) && (*(Pixel + j + 1) != 0))
                   {
                       Sideline_status_array[i].leftline =j;
                       Sideline_status_array[i].IsLeftFind = 'T';
                     break;
                   }
                   else if (*(Pixel + j) != 0)
                     break;
                   else if (j ==(Sideline_status_array[i].rightline - 1))
                   {

                       Sideline_status_array[i].IsLeftFind = 'T';
                     break;
                   }
                 }
               }
               /**************************处理左边线的大跳变***************************/


               /**************************处理右边线的大跳变***************************/
               if (Sideline_status_array[i].IsRightFind == 'H')
               {
                 for (uint8 j = (Sideline_status_array[i].rightline - 1);j >= (Sideline_status_array[i].leftline + 1); j--)
                 {
                   if ((*(Pixel + j) == 0) && (*(Pixel + j - 1) != 0))
                   {
                       Sideline_status_array[i].rightline =j;
                       Sideline_status_array[i].IsRightFind = 'T';
                     break;
                   }
                   else if (*(Pixel + j) != 0)
                     break;
                   else if (j == (Sideline_status_array[i].leftline + 1))
                   {
                       Sideline_status_array[i].rightline = j;
                       Sideline_status_array[i].IsRightFind = 'T';
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
             /**************************处理左边线的无边行***************************/
             if (Sideline_status_array[i].IsRightFind == 'W' && i > MT9V03X_H/12 && i < (MT9V03X_H/2-MT9V03X_H/12))
             {
               if (Get_R_line == 'F')
               {
                 Get_R_line = 'T';
                 ytemp_W_R = i + 2;
                 for (ysite = i + 1; ysite < i + 15; ysite++)
                 {
                   if (Sideline_status_array[ysite].IsRightFind =='T')
                   {
                       R_found_point++;
                   }
                 }
                 if (R_found_point >8)
                 {
                   D_R = ((float)(Sideline_status_array[i + R_found_point].rightline - Sideline_status_array[i + 3].rightline)) /((float)(R_found_point - 3));
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
                   Sideline_status_array[i].rightline =Sideline_status_array[ytemp_W_R].rightline -D_R * (ytemp_W_R - i);  //如果找到了 那么以基准行做延长线
               }
               LimitL(Sideline_status_array[i].rightline);  //限幅
               LimitH(Sideline_status_array[i].rightline);  //限幅
             }
             /**************************处理右边线的无边行***************************/


             /**************************处理左边线的无边行***************************/
             if (Sideline_status_array[i].IsLeftFind == 'W' && i > MT9V03X_H/12 && i < (MT9V03X_H/2-MT9V03X_H/12) )
             {
               if (Get_L_line == 'F')
               {
                 Get_L_line = 'T';
                 ytemp_W_L = i + 2;
                 for (ysite = i + 1; ysite < i + 15; ysite++)
                 {
                   if (Sideline_status_array[ysite].IsLeftFind == 'T')
                     {
                       L_found_point++;
                     }
                 }
                 if (L_found_point > 8)              //找到基准斜率边  做延长线重新确定无边
                 {
                   D_L = ((float)(Sideline_status_array[i + 3].leftline -Sideline_status_array[i + L_found_point].leftline)) /((float)(L_found_point - 3));
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
                   Sideline_status_array[i].leftline =Sideline_status_array[ytemp_W_L].leftline + D_L * (ytemp_W_L - i);
               }

               LimitL(Sideline_status_array[i].leftline);  //限幅
               LimitH(Sideline_status_array[i].leftline);  //限幅
             }

             /**************************处理左边线的无边行***************************/

             /************************************重新确定无边行（即W类）的边界****************************************************************/
             /************************************都处理完之后，其他的一些数据整定操作*************************************************/
                  if (Sideline_status_array[i].IsLeftFind == 'W' && Sideline_status_array[i].IsRightFind == 'W')
                  {
                      imagestatus.WhiteLine++;  //要是左右都无边，丢边数+1
                  }
                 if (Sideline_status_array[i].IsLeftFind == 'W' && i < (MT9V03X_H-5) )
                 {
                     imagestatus.Miss_Left_lines++;
                 }
                 if (Sideline_status_array[i].IsRightFind == 'W'&& i < (MT9V03X_H-5))
                 {
                     imagestatus.Miss_Right_lines++;
                 }

                  LimitL(Sideline_status_array[i].leftline);   //限幅
                  LimitH(Sideline_status_array[i].leftline);   //限幅
                  LimitL(Sideline_status_array[i].rightline);  //限幅
                  LimitH(Sideline_status_array[i].rightline);  //限幅

                  Sideline_status_array[i].wide =Sideline_status_array[i].rightline - Sideline_status_array[i].leftline;
                  Sideline_status_array[i].midline =(Sideline_status_array[i].rightline + Sideline_status_array[i].leftline) / 2;
                  Sideline_status_array[i].De_m=(Sideline_status_array[i].midline-(MT9V03X_W/4-2));
                  int16 temp;
                  temp=Sideline_status_array[i].midline-Sideline_status_array[i+1].midline;
                  temp=func_abs(temp);
                  if(temp>3){changepoint.y=i;changepoint.num+=1;}

                  if (Sideline_status_array[i].wide <= 9)//判断异常循迹
                  {
                      imagestatus.OFFLine = i + 1;
                      break;
                  }
                  else if (Sideline_status_array[i].rightline <= (MT9V03X_W/19)||Sideline_status_array[i].leftline >= (MT9V03X_W/2-(MT9V03X_W/19)))//判断驶出边线
                  {
                      imagestatus.OFFLine = i + 1;
                      break;
                  }
                  /************************************都处理完之后，其他的一些数据整定操作*************************************************/

    }
}
/*上交大左右手法则扫线，作为处理圆环等判断元素的第二依据*/
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//  @name           Search_Bottom_Line_OTSU
//  @brief          获取底层左右边线
//  @param          imageInput[IMAGE_ROW][IMAGE_COL]        传入的图像数组
//  @param          Row                                     图像的Ysite
//  @param          Col                                     图像的Xsite
//  @return         Bottonline                              底边行选择
//  @time           2022年10月9日
//  @Author         陈新云
//  Sample usage:   Search_Bottom_Line_OTSU(imageInput, Row, Col, Bottonline);
//--------------------------------------------------------------------------------------------------------------------------------------------

void Search_Bottom_Line_OTSU(uint8 imageInput[LCDH][LCDW], uint8 row, uint8 col, uint8 Bottonline)
{

    //寻找左边边界
    for (int Xsite = col / 2-2; Xsite > 1; Xsite--)
    {
        if (imageInput[Bottonline][Xsite] == 255 && imageInput[Bottonline][Xsite - 1] == 0)
        {
            Sideline_status_array[Bottonline].LeftBoundary = Xsite;//获取底边左边线
            break;
        }
    }
    for (int Xsite = col / 2+2; Xsite < LCDW-1; Xsite++)
    {
        if (imageInput[Bottonline][Xsite] == 255 && imageInput[Bottonline][Xsite + 1] == 0)
        {
            Sideline_status_array[Bottonline].RightBoundary = Xsite;//获取底边右边线
            break;
        }
    }


}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//  @name           Search_Left_and_Right_Lines
//  @brief          通过sobel提取左右边线
//  @param          imageInput[IMAGE_ROW][IMAGE_COL]        传入的图像数组
//  @param          Row                                     图像的Ysite
//  @param          Col                                     图像的Xsite
//  @param          Bottonline                              底边行选择
//  @return         无
//  Sample usage:   Search_Left_and_Right_Lines(imageInput, Row, Col, Bottonline);
//--------------------------------------------------------------------------------------------------------------------------------------------

void Search_Left_and_Right_Lines(uint8 imageInput[LCDH][LCDW],uint8 row,uint8 col,uint8 Bottonline)
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
                    if (Sideline_status_array[Right_Ysite].RightBoundary_First == (MT9V03X_W/2-2) )
                        Sideline_status_array[Right_Ysite].RightBoundary_First = Right_Xsite;
                    Sideline_status_array[Right_Ysite].RightBoundary = Right_Xsite;
                }
                else//左前方为白色
                {
                    // 方向发生改变 Right_Rirection  逆时针90度
                    Right_Ysite = Right_Ysite + Right_Rule[1][2 * Right_Rirection + 1];
                    Right_Xsite = Right_Xsite + Right_Rule[1][2 * Right_Rirection];
                    if (Sideline_status_array[Right_Ysite].RightBoundary_First == (MT9V03X_W/2-2))
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
//  @param          Row                                     图像的Ysite
//  @param          Col                                     图像的Xsite
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
            Sideline_status_array[Ysite].RightBoundary_First = 93;

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
        if (Sideline_status_array[Ysite].RightBoundary > LCDW - 5)
        {
            imagestatus.WhiteLine_R++;
        }
    }
}


//--------------------------------------------------------------------------------------------------------------------------
// 函数简介     自动补线函数（用于十字路口）
//--------------------------------------------------------------------------------------------------------------------------
void auto_extension_line(void)
{

    /************************************左边线的补线处理*************************************************/
    if (imagestatus.WhiteLine >= 8)                                       //如果丢边行的数量大于8
        TFSite = MT9V03X_W/4;                                                      //那就给TFSite赋值为55，这个变量是待会算补线斜率的一个变量。
    left_FTSite=0;
    if (ExtenLFlag != 'F')                                                //如果ExtenLFlag标志量不等于F，那就开始进行补线操作。
        for (uint8 Ysite = (MT9V03X_H/2-6); Ysite >= (imagestatus.OFFLine + 4);Ysite--)        //从第54开始往上扫，一直扫到顶边下面几行。
        {
            Pixel = ex_mt9v03x_binarizeImage[Ysite];
            if (Sideline_status_array[Ysite].IsLeftFind =='W')                            //如果本行的左边线类型是W类型，也就是无边行类型。
            {
                if (Sideline_status_array[Ysite + 1].leftline >= 80)                      //如果左边界到了第80列右边去了，那大概率就是极端情况，说明已经快寄了。
                {
                    imagestatus.OFFLine = Ysite + 1;                              //这种情况最好的处理方法就是不处理，直接跳出循环。
                    break;
                }
                while     (Ysite >= (imagestatus.OFFLine + 4))                      //如果左边界正常，那就进入while循环卡着，直到满足循环结束条件。
                {
                    Ysite--;                                                      //行数减减
                    if (Sideline_status_array[Ysite].IsLeftFind == 'T' && Sideline_status_array[Ysite - 1].IsLeftFind == 'T' && Sideline_status_array[Ysite - 2].IsLeftFind == 'T'
 && Sideline_status_array[Ysite - 2].leftline > 0 && Sideline_status_array[Ysite - 2].leftline <(MT9V03X_W/2-10))     //如果扫到的无边行的上面连续三行都是正常边线
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
      if (imagestatus.WhiteLine >= 8)//重新设定TFSite，为了避免十字路口这种两边都需要补线的情况下，左右边补线函数冲突
      TFSite = MT9V03X_W/4;
      if (ExtenRFlag != 'F')
      for (uint8 Ysite = (MT9V03X_H/2-6); Ysite >= (imagestatus.OFFLine + 4);Ysite--)
      {
        Pixel = ex_mt9v03x_binarizeImage[Ysite];  //存当前行
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
                &&Sideline_status_array[Ysite - 2].rightline < 93
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
//--------------------------------------------------------------------------------------------------------------------------
// 函数简介     路障判断
// 备注信息     过于敏感，仅对3~13行进行判断，如果总宽度发生一定值内的突变，将其判定为存在路障，通过midline的位置判断障碍物位置
// 改进意见     通过短路识别进行辅助元素排除
//--------------------------------------------------------------------------------------------------------------------------
uint8 RoadBlocktype_flag=0;//1为右边，2为左边
void Element_Judgment_RoadBlock(void)
{
    if(imageflag.RoadBlock_Flag==1)return;

    wide_sum=0;
    static uint16 wide_sum_last=0;
    uint8 Right_Num=0,Left_Num=0;
//    uint16 De_sum;
    if(imagestatus.OFFLine>MT9V03X_H/12)
    {

        for(int Ysite = imagestatus.OFFLine + 1;Ysite < imagestatus.OFFLine + 11;Ysite++)
        {
            //if((Sideline_status_array[Ysite].IsLeftFind))
            if(Sideline_status_array[Ysite].IsLeftFind == 'T')
                Left_Num++;
            if(Sideline_status_array[Ysite].IsRightFind == 'T')
                Right_Num++;

        }
        if(Left_Num>7&&Right_Num>7)
        {
            imageflag.RoadBlock_Flag=1;
        }
    }else {
        for(int Ysite = imagestatus.OFFLine + 1;Ysite < imagestatus.OFFLine + 11;Ysite++)
        {
            if(Sideline_status_array[Ysite].IsLeftFind == 'T')
                Left_Num++;
            if(Sideline_status_array[Ysite].IsRightFind == 'T')
                Right_Num++;
            wide_sum+=Sideline_status_array[Ysite].wide;
        }
        if(func_abs(wide_sum-wide_sum_last)>50 && wide_sum_last>100 && wide_sum>100 && wide_sum<600 && wide_sum_last<600&&func_abs(wide_sum-wide_sum_last)<150)
        {
            imageflag.RoadBlock_Flag=1;
        }
    }
    if(imageflag.RoadBlock_Flag)
    {
        Right_Num=0;
        Left_Num=0;
        for(int Ysite = imagestatus.OFFLine + 1;Ysite < imagestatus.OFFLine + 11;Ysite++)
                {
                    if(Sideline_status_array[Ysite].De_m>2)Left_Num++;
                    if(Sideline_status_array[Ysite].De_m<-2)Right_Num++;
                }
        if(Left_Num>7)RoadBlocktype_flag=2;
        if(Right_Num>7)RoadBlocktype_flag=1;
    }
    wide_sum_last=wide_sum;

}
//--------------------------------------------------------------
//  函数简介        路障处理
//--------------------------------------------------------------
//--------------------------------------------------------------
//  @name           Element_Judgment_Left_Rings()
//  @brief          整个图像判断的子函数，用来判断左圆环类型.
//  @parameter      void
//  @time
//  @Author         MRCHEN
//  Sample usage:   Element_Judgment_Left_Rings();
//--------------------------------------------------------------
void Element_Judgment_Left_Rings(void)
{
    if (   imagestatus.Miss_Right_lines > 3 || imagestatus.Miss_Left_lines < 13
        || imagestatus.OFFLine > 5 || Straight_Judge(2, 5, 55) > 1
        || imageflag.image_element_rings == 2 || imageflag.Out_Road == 1 || imageflag.RoadBlock_Flag == 1
        || Sideline_status_array[(MT9V03X_H/2-8)].IsLeftFind == 'W'
        || Sideline_status_array[(MT9V03X_H/2-7)].IsLeftFind == 'W'
        || Sideline_status_array[(MT9V03X_H/2-6)].IsLeftFind == 'W'
        || Sideline_status_array[(MT9V03X_H/2-5)].IsLeftFind == 'W'
        || Sideline_status_array[(MT9V03X_H/2-4)].IsLeftFind == 'W'
        || Sideline_status_array[(MT9V03X_H/2-3)].IsLeftFind == 'W'
        || Sideline_status_array[(MT9V03X_H/2-2)].IsLeftFind == 'W')
        return;
    int ring_ysite = 25;
    uint8 Left_Less_Num = 0;
    Left_RingsFlag_Point1_Ysite = 0;
    Left_RingsFlag_Point2_Ysite = 0;
    for (int Ysite = (MT9V03X_H/2-2); Ysite > ring_ysite; Ysite--)
    {
        if (Sideline_status_array[Ysite].LeftBoundary_First - Sideline_status_array[Ysite - 1].LeftBoundary_First > 4)
        {
            Left_RingsFlag_Point1_Ysite = Ysite;
            break;
        }
    }
    for (int Ysite = (MT9V03X_H/2-2); Ysite > ring_ysite; Ysite--)
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
        if (   Sideline_status_array[Ysite + 6].leftline < Sideline_status_array[Ysite+3].leftline
            && Sideline_status_array[Ysite + 5].leftline < Sideline_status_array[Ysite+3].leftline
            && Sideline_status_array[Ysite + 3].leftline > Sideline_status_array[Ysite + 2].leftline
            && Sideline_status_array[Ysite + 3].leftline > Sideline_status_array[Ysite + 1].leftline
            )
        {
            Ring_Help_Flag = 1;
            break;
        }
    }
    if(Left_RingsFlag_Point2_Ysite > Left_RingsFlag_Point1_Ysite+3 && Ring_Help_Flag == 0 && Left_Less_Num>7)
    {
        if(imagestatus.Miss_Left_lines > 13)
            Ring_Help_Flag = 1;
    }
    if (Left_RingsFlag_Point2_Ysite > Left_RingsFlag_Point1_Ysite+3 && Ring_Help_Flag == 1 && Left_Less_Num>7)
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
void Element_Judgment_Right_Rings(void)
{
    if (   imagestatus.Miss_Left_lines > 3 || imagestatus.Miss_Right_lines < 15
        || imagestatus.OFFLine > 5 || Straight_Judge(1, 5, 55) > 1
        || imageflag.image_element_rings == 1 || imageflag.Out_Road == 1 || imageflag.RoadBlock_Flag == 1
        || Sideline_status_array[(MT9V03X_H/2-8)].IsRightFind == 'W'
        || Sideline_status_array[(MT9V03X_H/2-7)].IsRightFind == 'W'
        || Sideline_status_array[(MT9V03X_H/2-6)].IsRightFind == 'W'
        || Sideline_status_array[(MT9V03X_H/2-5)].IsRightFind == 'W'
        || Sideline_status_array[(MT9V03X_H/2-4)].IsRightFind == 'W'
        || Sideline_status_array[(MT9V03X_H/2-3)].IsRightFind == 'W'
        || Sideline_status_array[(MT9V03X_H/2-2)].IsRightFind == 'W')
        return;

    int ring_ysite = 25;
    uint8 Right_Less_Num = 0;
    Right_RingsFlag_Point1_Ysite = 0;
    Right_RingsFlag_Point2_Ysite = 0;
    for (int Ysite = (MT9V03X_H/2-2); Ysite > ring_ysite; Ysite--)
    {
        if (Sideline_status_array[Ysite - 1].RightBoundary_First - Sideline_status_array[Ysite].RightBoundary_First > 4)
        {
            Right_RingsFlag_Point1_Ysite = Ysite;
            break;
        }
    }
    for (int Ysite = (MT9V03X_H/2-2); Ysite > ring_ysite; Ysite--)
    {
        if (Sideline_status_array[Ysite].RightBoundary - Sideline_status_array[Ysite + 1].RightBoundary > 4)
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
//        if (Sideline_status_array[Ysite + 3].RightBoundary_First > Sideline_status_array[Ysite].RightBoundary_First
//            && Sideline_status_array[Ysite + 2].RightBoundary_First > Sideline_status_array[Ysite].RightBoundary_First
//            && Sideline_status_array[Ysite].RightBoundary_First < Sideline_status_array[Ysite - 1].RightBoundary_First
//            && Sideline_status_array[Ysite].RightBoundary_First < Sideline_status_array[Ysite - 2].RightBoundary_First
//           )
        if (   Sideline_status_array[Ysite + 6].rightline > Sideline_status_array[Ysite + 3].rightline
            && Sideline_status_array[Ysite + 5].rightline > Sideline_status_array[Ysite + 3].rightline
            && Sideline_status_array[Ysite + 3].rightline < Sideline_status_array[Ysite + 2].rightline
            && Sideline_status_array[Ysite + 3].rightline < Sideline_status_array[Ysite + 1].rightline
           )
        {
            Ring_Help_Flag = 1;
            break;
        }
    }
    if(Right_RingsFlag_Point2_Ysite > Right_RingsFlag_Point1_Ysite+3 && Ring_Help_Flag == 0 && Right_Less_Num > 7)
    {
        if(imagestatus.Miss_Right_lines>13)
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
//------------------------------------------------------------------
// 函数简介     元素识别
//------------------------------------------------------------------

void Scan_Element(void)
{
    if (          imageflag.RoadBlock_Flag == 0
            && imageflag.Zebra_Flag == 0 && imageflag.image_element_rings == 0
                  && imageflag.Bend_Road == 0
            &&  imageflag.straight_long== 0  )
    {
//        Statu = Normal;                     //
//        Element_Judgment_RoadBlock();       //路障

        Element_Judgment_Left_Rings();      //左圆环
        Element_Judgment_Right_Rings();     //右圆环
        Element_Judgment_Zebra();           //斑马线
        Element_Judgment_Bend();            //弯道
        Straight_long_judge();              //长直道
    }
//    if(imageflag.Bend_Road)
//    {
//        Element_Judgment_OutRoad();         //断路
//        if(imageflag.Out_Road)
//            imageflag.Bend_Road=0;
//    }
//    if(imageflag.Ramp)
//    {
//        Element_Judgment_OutRoad();         //断路
//        if(imageflag.Ramp)
//            imageflag.Bend_Road=0;
//    }
//    if(imageflag.Bend_Road)
//    {
//        Element_Judgment_RoadBlock();         //路障
//        if(imageflag.RoadBlock_Flag)
//        {
////            Bend_Thruough_RoadBlock_Flag = 1;
////            if( imageflag.Bend_Road == 1 )
////                RoadBlock_Thruough_Flag = 2;
////            if( imageflag.Bend_Road == 2 )
////                RoadBlock_Thruough_Flag = 1;
//            imageflag.Bend_Road=0;
//        }
//    }
    /*if(imageflag.Out_Road)
    {
        Element_Judgment_Ramp();            //坡道
    }*/
    if(imageflag.Bend_Road)
    {
        Element_Judgment_Zebra();           //斑马线
        if(imageflag.Zebra_Flag)
            imageflag.Bend_Road=0;
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
    for (int Ysite = (MT9V03X_H/2-5); Ysite > MT9V03X_H/4; Ysite--)
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
    if (imageflag.image_element_rings_flag == 2 && num<5)
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
        for (int Ysite = (MT9V03X_H/6); Ysite > imagestatus.OFFLine + 2; Ysite--)
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
//        for (int Ysite = (MT9V03X_H/2-2); Ysite > 30; Ysite--)
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
        for (int Ysite = MT9V03X_H/3; Ysite > 10; Ysite--)
        {
            if(Sideline_status_array[Ysite].IsLeftFind == 'W' )
                num++;
        }
        if(num < 5)
        {
            imageflag.image_element_rings_flag = 0;
            imageflag.image_element_rings = 0;
            imageflag.ring_big_small = 0;
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
        for (int Ysite = (MT9V03X_H/2-1); Ysite > imagestatus.OFFLine; Ysite--)
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
        for(int Ysite=(MT9V03X_H/2-5);Ysite>imagestatus.OFFLine;Ysite--)//下面弧点
        {
            for(int Xsite=Sideline_status_array[Ysite].leftline + 1;Xsite<Sideline_status_array[Ysite].rightline - 1;Xsite++)
            {
                if(  ex_mt9v03x_binarizeImage[Ysite][Xsite] == 1 && ex_mt9v03x_binarizeImage[Ysite][Xsite + 1] == 0)
                 {
                   flag_Ysite_1 = Ysite;
                   flag_Xsite_1 = Xsite;
                   Slope_Rings=(float)((MT9V03X_W/2-10)-flag_Xsite_1)/(float)((MT9V03X_H/2-1)-flag_Ysite_1);
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
            for(int Ysite=imagestatus.OFFLine+1;Ysite<MT9V03X_H/4;Ysite++)
            {
                if(Sideline_status_array[Ysite].IsLeftFind=='T'&&Sideline_status_array[Ysite+1].IsLeftFind=='T'&&Sideline_status_array[Ysite+2].IsLeftFind=='W'
                    &&abs(Sideline_status_array[Ysite].leftline-Sideline_status_array[Ysite+2].leftline)>10
                  )
                {
                    flag_Ysite_1=Ysite;
                    flag_Xsite_1=Sideline_status_array[flag_Ysite_1].leftline;
                    imagestatus.OFFLine=Ysite;
                    Slope_Rings=(float)((MT9V03X_W/2-11)-flag_Xsite_1)/(float)((MT9V03X_H/2-1)-flag_Ysite_1);
                    break;
                }

            }
        }
        //补线
        if(flag_Ysite_1 != 0)
        {
            for(int Ysite=flag_Ysite_1;Ysite<MT9V03X_H/2;Ysite++)
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
            for(int Ysite=flag_Ysite_1-1;Ysite>MT9V03X_H/6;Ysite--) //A点上方进行扫线
            {
                for(int Xsite=Sideline_status_array[Ysite+1].rightline-10;Xsite<Sideline_status_array[Ysite+1].rightline+2;Xsite++)
                {
                    if(ex_mt9v03x_binarizeImage[Ysite][Xsite]==1 && ex_mt9v03x_binarizeImage[Ysite][Xsite+1]==0)
                    {
                        Sideline_status_array[Ysite].rightline=Xsite;
                        //if(imageflag.ring_big_small==1)//大圆环不减半宽
                            Sideline_status_array[Ysite].midline = (Sideline_status_array[Ysite].rightline + Sideline_status_array[Ysite].leftline)/2;
                        //else//小圆环减半宽
                        //    Sideline_status_array[Ysite].midline = Sideline_status_array[Ysite].rightline - Half_Bend_Wide[Ysite];
                        //if(Sideline_status_array[Ysite].midline<0)
                        //    Sideline_status_array[Ysite].midline = 0;
                        //Sideline_status_array[Ysite].Wide=Sideline_status_array[Ysite].rightline-Sideline_status_array[Ysite].leftline;
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
//        for (int Ysite = (MT9V03X_H/2-3); Ysite > imagestatus.OFFLine+1; Ysite--)
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
        Repair_Point_Xsite = MT9V03X_W/9.4;
        Repair_Point_Ysite = 0;
        for (int Ysite = MT9V03X_H/3; Ysite > MT9V03X_H/24; Ysite--)
        {
            if (ex_mt9v03x_binarizeImage[Ysite][(uint8)(MT9V03X_W/9.4)+3] == 1 && ex_mt9v03x_binarizeImage[Ysite-1][(uint8)(MT9V03X_W/9.4)+3] == 0)//28
            {
                Repair_Point_Xsite = (MT9V03X_W/9.4+3);
                Repair_Point_Ysite = Ysite-1;
                imagestatus.OFFLine = Ysite + 1;  //截止行重新规划&
                break;
            }
        }
        for (int Ysite = (MT9V03X_H/2-3); Ysite > Repair_Point_Ysite-3; Ysite--)         //补线
        {
            Sideline_status_array[Ysite].rightline = (Sideline_status_array[(MT9V03X_H/2)-2].rightline - Repair_Point_Xsite) * (Ysite - (MT9V03X_H/2-2)) / ((MT9V03X_H/2-2) - Repair_Point_Ysite)  + Sideline_status_array[(MT9V03X_H/2-2)].rightline;
            Sideline_status_array[Ysite].midline = (Sideline_status_array[Ysite].rightline + Sideline_status_array[Ysite].leftline) / 2;
        }
    }
        //小圆环出环 补线
    if (imageflag.image_element_rings_flag == 8 && imageflag.ring_big_small == 2)    //小圆环
    {
        Repair_Point_Xsite = 0;
        Repair_Point_Ysite = 0;
        for (int Ysite = (MT9V03X_H/2-5); Ysite > 5; Ysite--)
        {
            if (ex_mt9v03x_binarizeImage[Ysite][15] == 1 && ex_mt9v03x_binarizeImage[Ysite-1][15] == 0)
            {
                Repair_Point_Xsite = 15;
                Repair_Point_Ysite = Ysite-1;
                imagestatus.OFFLine = Ysite + 1;  //截止行重新规划
                break;
            }
        }
        for (int Ysite = (MT9V03X_H/2-3); Ysite > Repair_Point_Ysite-3; Ysite--)         //补线
        {
            Sideline_status_array[Ysite].rightline = (Sideline_status_array[(MT9V03X_H/2-2)].rightline - Repair_Point_Xsite) * (Ysite - (MT9V03X_H/2-2)) / ((MT9V03X_H/2-2) - Repair_Point_Ysite)  + Sideline_status_array[(MT9V03X_H/2-2)].rightline;
            Sideline_status_array[Ysite].midline = (Sideline_status_array[Ysite].rightline + Sideline_status_array[Ysite].leftline) / 2;
        }
    }
        //已出环 半宽处理
    if (imageflag.image_element_rings_flag == 9 || imageflag.image_element_rings_flag == 10)
    {
        for (int Ysite = (MT9V03X_H/2-1); Ysite > imagestatus.OFFLine; Ysite--)
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
    for (int Ysite = (MT9V03X_H/2-5); Ysite > (MT9V03X_H/4); Ysite--)
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
    if (imageflag.image_element_rings_flag == 2 && num<5)
    {
        imageflag.image_element_rings_flag = 5;
        //Stop = 1;
    }
        //刚到圆弧
    if (imageflag.image_element_rings_flag == 4)
    {
        Point_Ysite = 0;
        Point_Xsite = 0;
        for (int Ysite = (MT9V03X_H/4); Ysite > imagestatus.OFFLine + 2; Ysite--)
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
        for (int Ysite = (MT9V03X_H/3+5); Ysite > imagestatus.OFFLine + 7; Ysite--)
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
        if (Point_Ysite > (MT9V03X_H/6+1))
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
        for (int Ysite = (MT9V03X_H/2-10); Ysite > imagestatus.OFFLineBoundary+3; Ysite--)
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
        if (Point_Ysite > MT9V03X_H/6)
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
        for (int Ysite = (MT9V03X_H/2-10); Ysite > 10; Ysite--)
        {
            if(Sideline_status_array[Ysite].IsRightFind == 'W' )
                num++;
        }
        if(num < 5)
        {
            imageflag.image_element_rings_flag = 0;
            imageflag.image_element_rings = 0;
            imageflag.ring_big_small = 0;
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
        for (int Ysite = (MT9V03X_H/2-1); Ysite > imagestatus.OFFLine; Ysite--)
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
        for(int Ysite=(MT9V03X_H/2-5);Ysite>imagestatus.OFFLine;Ysite--)
        {
            for(int Xsite=Sideline_status_array[Ysite].leftline + 1;Xsite<Sideline_status_array[Ysite].rightline - 1;Xsite++)
            {
                if(ex_mt9v03x_binarizeImage[Ysite][Xsite]==1 && ex_mt9v03x_binarizeImage[Ysite][Xsite+1]==0)
                {
                    flag_Ysite_1=Ysite;
                    flag_Xsite_1=Xsite;
                    Slope_Right_Rings=(float)(0-flag_Xsite_1)/(float)((MT9V03X_H/2-1)-flag_Ysite_1);
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
        for(int Ysite=imagestatus.OFFLine+1;Ysite<MT9V03X_H/4;Ysite++)
        {
         if(Sideline_status_array[Ysite].IsRightFind=='T'&&Sideline_status_array[Ysite+1].IsRightFind=='T'&&Sideline_status_array[Ysite+2].IsRightFind=='W'
               &&abs(Sideline_status_array[Ysite].rightline-Sideline_status_array[Ysite+2].rightline)>10
         )
         {
             flag_Ysite_1=Ysite;
             flag_Xsite_1=Sideline_status_array[flag_Ysite_1].rightline;
             imagestatus.OFFLine=Ysite;
             Slope_Right_Rings=(float)(0-flag_Xsite_1)/(float)((MT9V03X_H/2-1)-flag_Ysite_1);
             break;
         }

        }

        }
        //补线
        if(flag_Ysite_1!=0)
        {
            for(int Ysite=flag_Ysite_1;Ysite<(MT9V03X_H/2);Ysite++)
            {
                Sideline_status_array[Ysite].leftline=flag_Xsite_1+Slope_Right_Rings*(Ysite-flag_Ysite_1);
                //if(imageflag.ring_big_small==2)//小圆环加半宽
                //    Sideline_status_array[Ysite].midline=Sideline_status_array[Ysite].leftline+Half_Bend_Wide[Ysite];//板块
//              else//大圆环不加半宽
                    Sideline_status_array[Ysite].midline=(Sideline_status_array[Ysite].leftline+Sideline_status_array[Ysite].rightline)/2;//板块
                //if(Sideline_status_array[Ysite].midline>(MT9V03X_W/2-11))
                //    Sideline_status_array[Ysite].midline=(MT9V03X_W/2-11);
            }
            Sideline_status_array[flag_Ysite_1].leftline=flag_Xsite_1;
            for(int Ysite=flag_Ysite_1-1;Ysite>10;Ysite--) //A点上方进行扫线
            {
                for(int Xsite=Sideline_status_array[Ysite+1].leftline+8;Xsite>Sideline_status_array[Ysite+1].leftline-4;Xsite--)
                {
                    if(ex_mt9v03x_binarizeImage[Ysite][Xsite]==1 && ex_mt9v03x_binarizeImage[Ysite][Xsite-1]==0)
                    {
                     Sideline_status_array[Ysite].leftline=Xsite;
                     Sideline_status_array[Ysite].wide=Sideline_status_array[Ysite].rightline-Sideline_status_array[Ysite].leftline;
                  //   if(imageflag.ring_big_small==2)//小圆环加半宽
                  //       Sideline_status_array[Ysite].midline=Sideline_status_array[Ysite].leftline+Half_Bend_Wide[Ysite];//板块
                   //  else//大圆环不加半宽
                         Sideline_status_array[Ysite].midline=(Sideline_status_array[Ysite].leftline+Sideline_status_array[Ysite].rightline)/2;//板块
                   //  if(Sideline_status_array[Ysite].midline>(MT9V03X_W/2-11))
                   //      Sideline_status_array[Ysite].midline=(MT9V03X_W/2-11);
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
//        for (int Ysite = (MT9V03X_H/2-1); Ysite > imagestatus.OFFLine; Ysite--)
//        {
//            if(imageflag.ring_big_small==2)
//                Sideline_status_array[Ysite].midline = Sideline_status_array[Ysite].leftline + Half_Bend_Wide[Ysite];
//            if(Sideline_status_array[Ysite].midline >= (MT9V03X_W/2-11))
//            {
//                Sideline_status_array[Ysite].midline = (MT9V03X_W/2-11);
//                imagestatus.OFFLine=Ysite-1;
//                break;
//            }
//        }
    }

        //大圆环出环 补线
    if (imageflag.image_element_rings_flag == 8 && imageflag.ring_big_small == 1)  //大圆环
    {
        Repair_Point_Xsite = (MT9V03X_H/2);
        Repair_Point_Ysite = 0;
        for (int Ysite = (MT9V03X_H/2-10); Ysite > 5; Ysite--)
        {
            if (ex_mt9v03x_binarizeImage[Ysite][(MT9V03X_H/2-3)] == 1 && ex_mt9v03x_binarizeImage[Ysite-1][(MT9V03X_H/2-3)] == 0)
            {
                Repair_Point_Xsite = (MT9V03X_H/2-3);
                Repair_Point_Ysite = Ysite-1;
                imagestatus.OFFLine = Ysite + 1;  //截止行重新规划
                        //  ips200_show_uint(200,200,Repair_Point_Ysite,2);
                break;
            }
        }
        for (int Ysite = (MT9V03X_H/2-3); Ysite > Repair_Point_Ysite-3; Ysite--)         //补线
        {
            Sideline_status_array[Ysite].leftline = (Sideline_status_array[(MT9V03X_H/2-2)].leftline - Repair_Point_Xsite) * (Ysite - (MT9V03X_H/2-2)) / ((MT9V03X_H/2-2) - Repair_Point_Ysite)  + Sideline_status_array[(MT9V03X_H/2-2)].leftline;
            Sideline_status_array[Ysite].midline = (Sideline_status_array[Ysite].rightline + Sideline_status_array[Ysite].leftline) / 2;
        }
    }
        //小圆环出环 补线
    if (imageflag.image_element_rings_flag == 8 && imageflag.ring_big_small == 2)  //小圆环
    {
        Repair_Point_Xsite = (MT9V03X_W/2-11);
        Repair_Point_Ysite = 0;
        for (int Ysite = (MT9V03X_H/3); Ysite > 5; Ysite--)
        {
            if (ex_mt9v03x_binarizeImage[Ysite][(MT9V03X_H/2-2)] == 1 && ex_mt9v03x_binarizeImage[Ysite-1][(MT9V03X_H/2-2)] == 0)
            {
                Repair_Point_Xsite = (MT9V03X_H/2-2);
                Repair_Point_Ysite = Ysite-1;
                imagestatus.OFFLine = Ysite + 1;  //截止行重新规划
                        //  ips200_show_uint(200,200,Repair_Point_Ysite,2);
                break;
            }
        }
        for (int Ysite = (MT9V03X_H/2-5); Ysite > Repair_Point_Ysite-3; Ysite--)         //补线
        {
            Sideline_status_array[Ysite].leftline = (Sideline_status_array[(MT9V03X_H/2-2)].leftline - Repair_Point_Xsite) * (Ysite - (MT9V03X_H/2-2)) / ((MT9V03X_H/2-2) - Repair_Point_Ysite)  + Sideline_status_array[(MT9V03X_H/2-2)].leftline;
            Sideline_status_array[Ysite].midline = (Sideline_status_array[Ysite].rightline + Sideline_status_array[Ysite].leftline) / 2;
        }
    }
        //已出环 半宽处理
    if (imageflag.image_element_rings_flag == 9 || imageflag.image_element_rings_flag == 10)
    {
        for (int Ysite = (MT9V03X_H/2-1); Ysite > imagestatus.OFFLine; Ysite--)
        {
            Sideline_status_array[Ysite].midline = Sideline_status_array[Ysite].leftline + Half_Road_Wide[Ysite];
        }
    }
}

//--------------------------------------------------------------
//  @name           Element_Judgment_Zebra()
//  @brief          整个图像判断的子函数，用来判断斑马线
//  @parameter      void
//  @time
//  @Author         MRCHEN
//  Sample usage:   Element_Judgment_Zebra();
//--------------------------------------------------------------
uint8 Garage_Location_Flag = 0;                 //判断库的次数
void Element_Judgment_Zebra()//斑马线判断
{
    if(imageflag.Zebra_Flag || imageflag.image_element_rings == 1 || imageflag.image_element_rings == 2
            || imageflag.Out_Road == 1 || imageflag.RoadBlock_Flag == 1) return;
    int NUM = 0,net = 0;
    if(imagestatus.OFFLineBoundary<(MT9V03X_H/6))
    {
        for (int Ysite = (MT9V03X_H/6); Ysite < 33; Ysite++)
        {
            net = 0;
            for (int Xsite =Sideline_status_array[Ysite].LeftBoundary + 2; Xsite < Sideline_status_array[Ysite].RightBoundary - 2; Xsite++)
            {
                if (ex_mt9v03x_binarizeImage[Ysite][Xsite] == 0 && ex_mt9v03x_binarizeImage[Ysite][Xsite + 1] == 255)
                {
                    net++;
                    if (net > 4)
                        NUM++;
                }
            }
        }
    }

    if (NUM >= 4)
    {
        imageflag.Zebra_Flag =1;
//        Garage_Location_Flag++;
    }

}
//------------------------------------------------------------------
//  函数简介        斑马线处理
//------------------------------------------------------------------

void Element_Handle_Zebra(void)
{
    int left_num=0,right_num=0;

    if(imageflag.Zebra_Flag == 1)//车库标志位
        {
            for (int Ysite = (MT9V03X_H/2-1); Ysite > imagestatus.OFFLineBoundary + 1; Ysite--)
            {
                 Sideline_status_array[Ysite].midline = Sideline_status_array[Ysite].RightBoundary  - Half_Road_Wide[Ysite];
                 if(Sideline_status_array[Ysite].IsLeftFind=='T')
                 {left_num++;}
                 if(Sideline_status_array[Ysite].IsRightFind=='T')
                 {right_num++;}
            }

            if(Garage_Location_Flag < 2)
            {

                if(left_num>52&&right_num>52)
                {
                    imageflag.Zebra_Flag=0;

                }
            }
            else if(Garage_Location_Flag >= 2)
            {
                if(left_num>52&&right_num>52)
                {
//                    imageflag.stop_flag=1;
                    imageflag.Zebra_Flag=0;
                }
            }
        }
}


////----------------------------------------------------------------
//// 函数简介     弯道识别修复
////----------------------------------------------------------------
//int Miss_Left_Num = 0 ;
//int Miss_Right_Num = 0;
//void Element_Judgment_Bend_fix(void)
//{
//    if(Sideline_status_array[imagestatus.OFFLine+1].leftline > 30
//                && imagestatus.Miss_Left_lines < 4
//                && imagestatus.Miss_Right_lines > 8
//                && Straight_Judge(1, imagestatus.OFFLine+2, (MT9V03X_H/2-2)) > 1)
//    {
//
//    }
//}
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
    if(Sideline_status_array[imagestatus.OFFLine+1].leftline > (MT9V03X_H/2)
            && imagestatus.Miss_Left_lines < 4
            && imagestatus.Miss_Right_lines > 8
            && Straight_Judge(1, imagestatus.OFFLine+2, (MT9V03X_H/2-2)) > 1)
    {

        imageflag.Bend_Road = 1;
//        BeeOn;
    }
    if(Sideline_status_array[imagestatus.OFFLine+1].rightline < 50
            && imagestatus.Miss_Right_lines < 4
            && imagestatus.Miss_Left_lines > 8
            && Straight_Judge(2, imagestatus.OFFLine+2, (MT9V03X_H/2-2)) > 1)
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
    if(imagestatus.OFFLine<10) { imageflag.Bend_Road = 0;}
    if(imageflag.Bend_Road==1)
    {
        for (int Ysite = (MT9V03X_H/2-1); Ysite > imagestatus.OFFLine; Ysite--)
        {
            Sideline_status_array[Ysite].midline = Sideline_status_array[Ysite].leftline + Half_Bend_Wide[Ysite];
            if(Sideline_status_array[Ysite].midline >= (MT9V03X_W/2-11))
                Sideline_status_array[Ysite].midline = (MT9V03X_W/2-11);
        }
    }
    if(imageflag.Bend_Road==2)
    {
        for (int Ysite = (MT9V03X_H/2-1); Ysite > imagestatus.OFFLine; Ysite--)
        {
            Sideline_status_array[Ysite].midline = Sideline_status_array[Ysite].rightline - Half_Bend_Wide[Ysite];
            if(Sideline_status_array[Ysite].midline < 0)
                Sideline_status_array[Ysite].midline = 0;
        }
    }
}
    //--------------------------------------------------------------
    //  @name           Element_GetStraightLine_Bend()
    //  @brief          整个图像处理的子函数，用来处理左右弯道
    //  @parameter      void
    //  @time
    //  @Author         MRCHEN
    //  Sample usage:   Element_Handle_Bend();
    //--------------------------------------------------------------
    void Element_GetStraightLine_Bend()
    {
        if(imagestatus.OFFLine < 4 && imagestatus.WhiteLine_R>10 && Straight_Judge(1, 20, 50)<1)
        {
            for (int Ysite = (MT9V03X_H/2-1); Ysite > imagestatus.OFFLine; Ysite--)
            {
                Sideline_status_array[Ysite].midline = Sideline_status_array[Ysite].leftline + Half_Road_Wide[Ysite];
            }
        }
        if(imagestatus.OFFLine < 4 && imagestatus.WhiteLine_L>10 && Straight_Judge(2, 20, 50)<1)
        {
            for (int Ysite = (MT9V03X_H/2-1); Ysite > imagestatus.OFFLine; Ysite--)
            {
                Sideline_status_array[Ysite].midline = Sideline_status_array[Ysite].rightline - Half_Road_Wide[Ysite];
            }
        }
    }
    /*元素处理函数*/
    void Element_Handle(void)
    {
        Element_GetStraightLine_Bend();//直道修正

//        if(imageflag.RoadBlock_Flag == 1)
//            Element_Handle_RoadBlock();
//        else if(imageflag.Out_Road !=0)
//            Element_Handle_OutRoad();
//        else
        if (imageflag.image_element_rings == 1)
            Element_Handle_Left_Rings();
        else if (imageflag.image_element_rings == 2)
            Element_Handle_Right_Rings();
        else if (imageflag.Zebra_Flag != 0 )
            Element_Handle_Zebra();
//        else if (imageflag.Ramp != 0)
//            Element_Handle_Ramp();
        else if(imageflag.straight_long)
            Straight_long_handle();
        else if(imageflag.Bend_Road !=0)
            Element_Handle_Bend();
        else if(imagestatus.WhiteLine >= 8) //十字处理
            auto_extension_line();
    }
//-------------------------------------------------------------------------------------------------
//  函数名称        图像处理（一键操作傻瓜型）
//  备注信息        扔主函数里这一页就不用管了
//--------------------------------------------------------------------------------------------------------
void Image_Process(void)
{
    if(mt9v03x_finish_flag==1)
    {
   //         otsuThreshold_get();

//        binarizeImage();
        baseline_get();
        allline_get();
        Scan_Element();
        Element_Handle();
   //         if(!imageflag.Out_Road)
   //                             Search_Border_OTSU(ex_mt9v03x_binarizeImage, LCDH, LCDW, LCDH - 2);//(MT9V03X_H/2-2)行位底行
   //                         else
   //                             imagestatus.OFFLineBoundary = 5;
   //         Forward_control();
        mt9v03x_finish_flag=0;
    }
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  函数简介        基于八邻域法的障碍判断函数
//
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//void Element_judgment(uint8 y_h,uint8 y_l,uint8 dir)
//{
//    uint8 x,y,area,area_l,growth_u,growth_d;
//    switch(dir)
//    {
//    case 0:
//        x=Sideline_status_array[y_l].leftline;
//        area=Sideline_status_array[y_l-1].leftline;
//        for(uint8 i=0;i<2;i++)
//        {
//            area_l=area;
//            area=;
//        }
//        break;
//
//    }
//}
