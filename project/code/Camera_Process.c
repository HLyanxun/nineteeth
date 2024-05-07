/*
 * Camera_Process.c
 *
 *  Created on: 2024年4月29日
 *      Author: pc
 */
#include "zf_common_headfile.h"
#include "math.h"

#define ImageSensorMid (39)                    //图像的屏幕中点
#define track_width (28)
#define island_in_limit_x  55 //环岛左上限制点位
#define USE_num image_h*3   //定义找点的数组成员个数按理说300个点能放下，但是有些特殊情况确实难顶，多定义了一点

int16 threshold_fix;                                          //二值化阈值补正

//static uint8* PicTemp;                          //一个保存单行图像的指针变量
uint8 l_shade_255=0,r_shade_255=0; //左侧右侧阴影  也就是 白色方块
uint16 l_data_statics=0;//统计左边总共的长度
uint16 r_data_statics=0;//统计右边总共的长度

uint8 Find_Boundary_l[300][2]={{0}};//左边界线 0为y  1为x 存储内容为八邻域扫描出来的边界
uint8 Find_Boundary_r[300][2]={{0}};//右边界线 0为y  1为x 存储内容为八邻域扫描出来的边界
uint8 Find_Boundary_z[300][2]={{0}};//中边界线 0为y  1为x 存储内容为八邻域扫描出来的边界
uint8 Find_Boundary_centre[300][2]={{0}};//中线 0为y  1为x 存储内容为八邻域扫描出来的边界

uint8 l_Down[4]={0},l_on[4]={0},r_Down[4]={0},r_on[4]={0}; //十字的四个拐点 0y1x
uint8 is_L_on[3]={0},is_R_on[3]={0};  //环岛 左上 右上 两个拐点0y1x
uint8 is_L_down[3]={0},is_R_down[3]={0};  //环岛 左下 右下 两个拐点0y1x

uint8 ex_mt9v03x_binarizeImage[image_h][image_w]={{0}};             //真正用于图像处理的数组

//逆透视---------------------------------------------------------------------------------------------------------------
uint8 *PerImg_ip[RESULT_ROW][RESULT_COL]={{0}};//存储图像地址

//Sideline_status Sideline_status_array[image_h];
Image_Status imagestatus;
ImageFlagtypedef imageflag;

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     ImagePerspective_Init 逆透视初始化 初始运行一次就可以了
// 参数说明
// 返回参数
// 使用示例
// 备注信息  /*完成摄像头初始化后，调用一次ImagePerspective_Init，此后，直接调用ImageUsed   即为透视结果*/
//-------------------------------------------------------------------------------------------------------------------
void ImagePerspective_Init(void)
{

    static uint8 BlackColor = 0;
//    double change_un_Mat[3][3] ={{0.500000,-0.551282,45.243590},{-0.000000,0.262821,11.564103},{-0.000000,-0.006410,0.717949}};
//    double change_un_Mat[3][3] ={{0.416667,-0.523504,43.418803},{-0.000000,0.209402,11.965812},{-0.000000,-0.005983,0.658120}};
    double change_un_Mat[3][3] ={{0.450680,-0.533700,45.388247},{-0.000000,0.251432,12.396079},{-0.000000,-0.006167,0.695945}};
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

//-------------------------------------------------------------------------------------------------------------------
// 函数简介         二值化
// 参数说明         void
// 返回参数         void
// 使用示例         Binaryzation();
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
uint8 My_Threshold=0;//二值化后的阈值
void Binaryzation(void)
{
    My_Threshold = (int)my_adapt_threshold(mt9v03x_image[0],180, 248 )+(int)threshold_fix;//大津法计算出阈值，再添加阈值补正值
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     my_adapt_threshold  初次二值化
// 参数说明     *ex_mt9v03x_binarizeImage 图像数组指针
//        col    图像列（宽）
//        row    图像行（高）
// 返回参数     图像二值化阈值
// 使用示例     my_adapt_threshold(mt9v03x_image[0],MT9V03X_W,MT9V03X_H)
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
uint8 my_adapt_threshold(uint8 *ex_mt9v03x_binarizeImage, uint16 col, uint16 row)   //注意计算阈值的一定要是原图像
{
   #define GrayScale 256
    uint16 width = col;
    uint16 height = row;
    int pixelCount[GrayScale];
    float pixelPro[GrayScale];
    int i, j, pixelSum = width * height/4;
    uint8 threshold = 0;
    uint8* data = ex_mt9v03x_binarizeImage;  //指向像素数据的指针
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
                    threshold = j;
                }
                if (deltaTmp < deltaMax)
                {
                    break;
                }
        }

    return threshold;
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     my_adapt_threshold_2  第二次二值化
// 参数说明     *ex_mt9v03x_binarizeImage 图像数组指针
//        col    图像列（宽）
//        row    图像行（高）
// 返回参数     图像二值化阈值
// 使用示例     uint8 threshold = (int)my_adapt_threshold_2(mt9v03x_image[0],MT9V03X_W,MT9V03X_H)
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
//uint8 my_adapt_threshold_2(uint8 *ex_mt9v03x_binarizeImage, uint16 col, uint16 row)   //注意计算阈值的一定要是原图像
//{
//   #define GrayScale 256
//    uint16 width = col;
//    uint16 height = row;
//    int pixelCount[GrayScale];
//    float pixelPro[GrayScale];
//    int i, j, pixelSum = width * height/4;
//    uint8 threshold = 0;
//    uint8* data = ex_mt9v03x_binarizeImage;  //指向像素数据的指针
//    for (i = 0; i < GrayScale; i++)
//    {
//        pixelCount[i] = 0;
//        pixelPro[i] = 0;
//    }
//
//    uint32 gray_sum=0;
//    //统计灰度级中每个像素在整幅图像中的个数
//    for (i = 1; i < height; i+=2)
//    {
//        for (j = 1; j < width; j+=2)
//        {
//            if( data[i * width + j]  > My_Threshold_1 )
//            {
//                pixelCount[(int)data[i * width + j]]++ ;  //将当前的点的像素值作为计数数组的下标
//                gray_sum+=(int)data[i * width + j] ;       //灰度值总和
//            }
//            else
//            {
//                pixelCount[(int) My_Threshold_1]++ ;  //将当前的点的像素值作为计数数组的下标
//                gray_sum+= (int) My_Threshold_1 ;       //灰度值总和
//            }
//        }
//    }
//
//    //计算每个像素值的点在整幅图像中的比例
//
//    for (i = 0; i < GrayScale; i++)
//    {
//        pixelPro[i] = (float)pixelCount[i] / pixelSum;
//    }
//
//    //遍历灰度级[0,255]
//    float w0, w1, u0tmp, u1tmp, u0, u1, u, deltaTmp, deltaMax = 0;
//
//
//        w0 = w1 = u0tmp = u1tmp = u0 = u1 = u = deltaTmp = 0;
//        for (j = 0; j < GrayScale; j++)
//        {
//
//                w0 += pixelPro[j];  //背景部分每个灰度值的像素点所占比例之和   即背景部分的比例
//                u0tmp += j * pixelPro[j];  //背景部分 每个灰度值的点的比例 *灰度值
//
//               w1=1-w0;
//               u1tmp=gray_sum/pixelSum-u0tmp;
//
//                u0 = u0tmp / w0;              //背景平均灰度
//                u1 = u1tmp / w1;              //前景平均灰度
//                u = u0tmp + u1tmp;            //全局平均灰度
//                deltaTmp = w0 * pow((u0 - u), 2) + w1 * pow((u1 - u), 2);
//                if (deltaTmp > deltaMax)
//                {
//                    deltaMax = deltaTmp;
//                    threshold = j;
//                }
//                if (deltaTmp < deltaMax)
//                {
//                    break;
//                }
//        }
//
//    return threshold;
//}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     image_draw  画黑框 加上 对图像进行二值化
// 参数说明     void
// 返回参数     void
// 使用示例     image_draw();
// 备注信息     对图像进行二值化，并且根据四个方向的像素对白噪点进行清除
//-------------------------------------------------------------------------------------------------------------------
void image_draw(void)
{

    int i=0,j=0;
    int a=0; //出界保护
    for(i = image_h-3; i >= 0 ; i-- )//从逆透视图像数组里面提出,在进行一次阈值梯度递减的二值化
    {
//        if( i%8 == 1 )
//        {
//            My_Threshold --;
//        }
        for(j=0;j< image_w-2;j++)
        {

            if( ImageUsed[i][j] > My_Threshold-20)
                ex_mt9v03x_binarizeImage[i+1][j+1]=255;
//                ex_mt9v03x_binarizeImage[i+1][j+1]=ImageUsed[i][j];
            else
            {
                ex_mt9v03x_binarizeImage[i+1][j+1]=0;
//                ex_mt9v03x_binarizeImage[i+1][j+1]=ImageUsed[i][j];
                if(i==87)a++;
            }


        }
        if(imageflag.run_flag==1)
        {
            if(a>50)
            {
             imageflag.Out_Road=1; imageflag.run_flag=0;
            }
            a=0;
        }
    }

    uint16 N_zaoddian=0;
    for(int i=0;i<89;i++)       //白噪点 去除（通过上下左右四个方向的像素点来判断当前位置应该是什么像素，来删除白噪点）
        for(int j=1;j<70;j++)
        {
            if( ex_mt9v03x_binarizeImage[90-i][j] == 255)
            {
                N_zaoddian = ex_mt9v03x_binarizeImage[90-i-1][j] + ex_mt9v03x_binarizeImage[90-i][j-1] + ex_mt9v03x_binarizeImage[90-i][j+1]+ex_mt9v03x_binarizeImage[90-i+1][j] ;
                if( N_zaoddian <  255 * 2 )
                {
                    ex_mt9v03x_binarizeImage[90-i][j] = 0 ;
                }
            }
            else
            {
                N_zaoddian =                 ex_mt9v03x_binarizeImage[90-i-1][j]
                           + ex_mt9v03x_binarizeImage[90-i][j-1]               + ex_mt9v03x_binarizeImage[90-i]  [j+1]
                                            +ex_mt9v03x_binarizeImage[90-i+1][j] ;
                if( N_zaoddian >  255 * 2 )
                {
                    ex_mt9v03x_binarizeImage[90-i][j] = 255 ;
                }
            }

        }

}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     otsuThreshold   简易版大津法(图像压缩)
// 参数说明     *ex_mt9v03x_binarizeImage：图像地址  width：图像宽    height：图像高
// 返回参数
// 使用示例     otsuThreshold( mt9v03x_image[0], MT9V03X_W, MT9V03X_H);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
//uint8 otsuThreshold(uint8 *ex_mt9v03x_binarizeImage, uint16 width,  uint16 height)
//{
//    #define GrayScale 256
//    int pixelCount[GrayScale] = {0};//每个灰度值所占像素个数
//    float pixelPro[GrayScale] = {0};//每个灰度值所占总像素比例
//    int i,j;
//    int Sumpix = width * height/4;   //总像素点
//    uint8 threshold = 0;
//    uint8* data = ex_mt9v03x_binarizeImage;  //指向像素数据的指针
//
//
//    //统计灰度级中每个像素在整幅图像中的个数
//    for (i = 0; i < height; i+=2)
//    {
//        for (j = 0; j < width; j+=2)
//        {
//            pixelCount[(int)data[i * width + j]]++;  //将像素值作为计数数组的下标
//          //   pixelCount[(int)ex_mt9v03x_binarizeImage[i][j]]++;    若不用指针用这个
//        }
//    }
//    float u = 0;
//    for (i = 0; i < GrayScale; i++)
//    {
//        pixelPro[i] = (float)pixelCount[i] / Sumpix;   //计算每个像素在整幅图像中的比例
//        u += i * pixelPro[i];  //总平均灰度
//    }
//
//
//    float maxVariance=0.0;  //最大类间方差
//    float w0 = 0, avgValue  = 0;  //w0 前景比例 ，avgValue 前景平均灰度
//    for(int i = 0; i < 256; i++)     //每一次循环都是一次完整类间方差计算 (两个for叠加为1个)
//    {
//        w0 += pixelPro[i];  //假设当前灰度i为阈值, 0~i 灰度像素所占整幅图像的比例即前景比例
//        avgValue  += i * pixelPro[i];
//
//        float variance = pow((avgValue/w0 - u), 2) * w0 /(1 - w0);    //类间方差
//        if(variance > maxVariance)
//        {
//            maxVariance = variance;
//            threshold = (uint8)i;
//        }
//    }
//
//
//    return threshold;
//
//}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     阴影扫描辅助函数
// 使用示例     shade_compute();
// 备注信息
//-------------------------------------------------------------------------------------------------------------------

void shade_compute(void)   //判断是否扫描到阴影
{
    l_shade_255 = 0 ,r_shade_255 = 0 ;
    for(int i = 13 ; i < 20; i++)
        for(int j=78;j<86;j++)
            if( ex_mt9v03x_binarizeImage[j][i] == 255 ) l_shade_255++;


    for( int i = 61 ; i < 68 ; i ++)
        for(int j=78;j<86;j++)
            if( ex_mt9v03x_binarizeImage[j][i] == 255 )   r_shade_255++;

}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     Find_Boundary()   寻找最初的左右边界线
// 参数说明
// 返回参数     find_Boundary_flag 判断是否找到左右边界
// 使用示例     int aflag=Find_Boundary();
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
uint8 centre_line=35;
int Find_Boundary(void)
{
    int find_Boundary_flag = 0;

     l_data_statics=0;//统计左边总共的长度
     r_data_statics=0;//统计右边总共的长度

    if (imageflag.Zebra_Flag!=1) //当未出现斑马线时候就从中间向两侧扫描
    {
        for (int i = 0; i < 5; i++)// Y
        {
            for (int j = centre_line ; j >= 1; j--)   //左边线  X
            {
                if (ex_mt9v03x_binarizeImage[90 - i][j-1] == 0 &&ex_mt9v03x_binarizeImage[90 - i][j] == 0 && ex_mt9v03x_binarizeImage[90 - i][j + 1] == 255 && ex_mt9v03x_binarizeImage[90 - i][j + 2] == 255)
                {
                    Find_Boundary_l[i][0] = 90 - i;
                    Find_Boundary_l[i][1] = (uint8)j;
                    l_data_statics++;
                    find_Boundary_flag++;
                    break;

                }
            }

            for (int j = centre_line ; j <= image_w - 1; j++)   //右边线   X                                                                                                                                                                                              边线
            {
                if (ex_mt9v03x_binarizeImage[90 - i][j+1] == 0 &&ex_mt9v03x_binarizeImage[90 - i][j] == 0 && ex_mt9v03x_binarizeImage[90 - i][j - 1] == 255 && ex_mt9v03x_binarizeImage[90 - i][j - 2] == 255)
                {
                    Find_Boundary_r[i][0] = 90 - i;
                    Find_Boundary_r[i][1] = (uint8)j;
                    r_data_statics++;
                    find_Boundary_flag++;
                    break;
                }
            }
        }
    }

    else//当出现斑马线时候就从两侧向中间扫描
    {
        for (int i = 1; i < 4; i++)//0 1 行边界
        {
            for (int j = 1; j <= image_w / 2; j++)   //左边线
            {
                if (ex_mt9v03x_binarizeImage[90 - i][j] == 0 && ex_mt9v03x_binarizeImage[90 - i][j + 1] == 255 && ex_mt9v03x_binarizeImage[90 - i][j + 2] == 255)
                {
                    Find_Boundary_l[i][0] = 90 - i;
                    Find_Boundary_l[i][1] = (uint8)j;
                    find_Boundary_flag++;
                    l_data_statics++;
                    break;
                }
            }

            for (int j = image_w-1; j >= image_w / 2; j--)   //右边线                                                                                                                                                                                                      边线
            {
                if (ex_mt9v03x_binarizeImage[90 - i][j] == 0 && ex_mt9v03x_binarizeImage[90 - i][j - 1] == 255 && ex_mt9v03x_binarizeImage[90 - i][j - 2] == 255)
                {
                    Find_Boundary_r[i][0] = 90 - i;
                    Find_Boundary_r[i][1] = (uint8)j;
                    r_data_statics++;
                    find_Boundary_flag++;
                    break;
                }
            }
        }
    }
    centre_line=(uint8)(Find_Boundary_r[2][1]+Find_Boundary_l[2][1])/2;
    return find_Boundary_flag;  //返回两边是否找到起始点
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     search                   八邻域正式开始找边界的函数，输入参数有点多，调用的时候不要漏了，这个是左右线一次性找完。
// 参数说明     break_flag              ：最多需要循环的次数
// 参数说明     (*ex_mt9v03x_binarizeImage)[image_w]       ：需要进行找点的图像数组，必须是二值图,填入数组名称即可  特别注意，不要拿宏定义名字作为输入参数，否则数据可能无法传递过来
// 参数说明     hightest                ：循环结束所得到的最高高度
// 返回参数     void
// 使用示例
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void search(uint16 break_flag,uint8(*image_u)[image_w],uint8* hightest) //暂时没有理解
{

    if( Find_Boundary())
    {
        l_data_statics=2;//统计左边总共的长度
        r_data_statics=2;//统计右边总共的长度
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
                *hightest = (Find_Boundary_r[r_data_statics][1] + Find_Boundary_l[l_data_statics ][1]) >> 1;//取出最高点
                break;
            }



        }
    }
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     曲率/直线判断
// 参数说明     void
// 返回参数     void
// 使用示例     curvature_judge();
// 备注信息     将判断结果存入二维数组imageflag.image_element_rings中
//-------------------------------------------------------------------------------------------------------------------
void curvature_judge(void)
{
    int x[3]={0},y[3]={0};
//远段----------------------------------------------------------------------
    x[0]=Find_Boundary_l[10][1];
    x[1]=Find_Boundary_l[40][1];
    x[2]=Find_Boundary_l[80][1];

    y[0]=Find_Boundary_l[10][0];
    y[1]=Find_Boundary_l[40][0];
    y[2]=Find_Boundary_l[80][0];
    imageflag.image_curvature[0][4]=(uint8)Angel_compute(x[0],y[0],x[1],y[1],x[2],y[2]);

    x[0]=Find_Boundary_r[10][1];
    x[1]=Find_Boundary_r[40][1];
    x[2]=Find_Boundary_r[80][1];

    y[0]=Find_Boundary_r[10][0];
    y[1]=Find_Boundary_r[40][0];
    y[2]=Find_Boundary_r[80][0];
    imageflag.image_curvature[1][4]=(uint8)Angel_compute(x[0],y[0],x[1],y[1],x[2],y[2]);

//中段----------------------------------------------------------------------
    x[0]=Find_Boundary_l[6][1];
    x[1]=Find_Boundary_l[30][1];
    x[2]=Find_Boundary_l[55][1];

    y[0]=Find_Boundary_l[6][0];
    y[1]=Find_Boundary_l[30][0];
    y[2]=Find_Boundary_l[55][0];
    imageflag.image_curvature[0][3]=(uint8)Angel_compute(x[0],y[0],x[1],y[1],x[2],y[2]);

    x[0]=Find_Boundary_r[6][1];
    x[1]=Find_Boundary_r[30][1];
    x[2]=Find_Boundary_r[55][1];

    y[0]=Find_Boundary_r[6][0];
    y[1]=Find_Boundary_r[30][0];
    y[2]=Find_Boundary_r[55][0];
    imageflag.image_curvature[1][3]=(uint8)Angel_compute(x[0],y[0],x[1],y[1],x[2],y[2]);


//近段2----------------------------------------------------------------------
    x[0]=Find_Boundary_l[5][1];
    x[1]=Find_Boundary_l[17][1];
    x[2]=Find_Boundary_l[35][1];

    y[0]=Find_Boundary_l[5][0];
    y[1]=Find_Boundary_l[17][0];
    y[2]=Find_Boundary_l[35][0];

    imageflag.image_curvature[0][2]=(uint8)Angel_compute(x[0],y[0],x[1],y[1],x[2],y[2]);

    x[0]=Find_Boundary_r[5][1];
    x[1]=Find_Boundary_r[17][1];
    x[2]=Find_Boundary_r[35][1];

    y[0]=Find_Boundary_r[5][0];
    y[1]=Find_Boundary_r[17][0];
    y[2]=Find_Boundary_r[35][0];
    imageflag.image_curvature[1][2]=(uint8)Angel_compute(x[0],y[0],x[1],y[1],x[2],y[2]);


//近段1----------------------------------------------------------------------
    x[0]=Find_Boundary_l[5][1];
    x[1]=Find_Boundary_l[12][1];
    x[2]=Find_Boundary_l[25][1];

    y[0]=Find_Boundary_l[5][0];
    y[1]=Find_Boundary_l[12][0];
    y[2]=Find_Boundary_l[25][0];
    imageflag.image_curvature[0][1]=(uint8)Angel_compute(x[0],y[0],x[1],y[1],x[2],y[2]);

    x[0]=Find_Boundary_r[5][1];
    x[1]=Find_Boundary_r[12][1];
    x[2]=Find_Boundary_r[25][1];

    y[0]=Find_Boundary_r[5][0];
    y[1]=Find_Boundary_r[12][0];
    y[2]=Find_Boundary_r[25][0];
    imageflag.image_curvature[1][1]=(uint8)Angel_compute(x[0],y[0],x[1],y[1],x[2],y[2]);
//近段0----------------------------------------------------------------------
    x[0]=Find_Boundary_l[4][1];
    x[1]=Find_Boundary_l[13][1];
    x[2]=Find_Boundary_l[22][1];

    y[0]=Find_Boundary_l[4][0];
    y[1]=Find_Boundary_l[13][0];
    y[2]=Find_Boundary_l[22][0];
    imageflag.image_curvature[0][0]=(uint8)Angel_compute(x[0],y[0],x[1],y[1],x[2],y[2]);

    x[0]=Find_Boundary_r[4][1];
    x[1]=Find_Boundary_r[13][1];
    x[2]=Find_Boundary_r[22][1];

    y[0]=Find_Boundary_r[4][0];
    y[1]=Find_Boundary_r[13][0];
    y[2]=Find_Boundary_r[22][0];
    imageflag.image_curvature[1][0]=(uint8)Angel_compute(x[0],y[0],x[1],y[1],x[2],y[2]);

//弯道6----------------------------------------------------------------------
    x[0]=65;
    x[1]=Find_Boundary_l[2][1];
    x[2]=Find_Boundary_l[15][1];

    y[0]=89;
    y[1]=Find_Boundary_l[2][0];
    y[2]=Find_Boundary_l[15][0];
    imageflag.image_curvature[0][5]=(uint8)Angel_compute(x[0],y[0],x[1],y[1],x[2],y[2]);

    x[0]=4;
    x[1]=Find_Boundary_r[2][1];
    x[2]=Find_Boundary_r[15][1];

    y[0]=89;
    y[1]=Find_Boundary_r[2][0];
    y[2]=Find_Boundary_r[15][0];

    imageflag.image_curvature[1][5]=(uint8)Angel_compute(x[0],y[0],x[1],y[1],x[2],y[2]);



    if(imageflag.image_curvature[0][0] > 168 && imageflag.image_curvature[0][1] > 165 && imageflag.image_curvature[0][2] > 165
     && func_abs( imageflag.image_curvature[0][2] - imageflag.image_curvature[0][0]) <= 20  && func_abs(imageflag.image_curvature[0][1] - imageflag.image_curvature[0][2]) <= 25
     && Find_Boundary_l[2][1]>10 )
    {
        imageflag.straight[0]=1;              //近段直线
        if(imageflag.image_curvature[0][3] > 168 && abs( imageflag.image_curvature[0][3] - imageflag.image_curvature[0][1]) <= 15 )
        {
            imageflag.straight[0] = 2;  //中段
            if(imageflag.image_curvature[0][4] > 168 && abs( imageflag.image_curvature[0][4] - imageflag.image_curvature[0][1]) <= 15 )
            {
                imageflag.straight[0] = 3;       //全局直线
            }
        }
    }
    else imageflag.straight[0]=0;

     if(imageflag.image_curvature[1][0] > 168 && imageflag.image_curvature[1][1] > 165 && imageflag.image_curvature[1][2] > 165
      && func_abs( imageflag.image_curvature[1][2] - imageflag.image_curvature[1][0]) <= 20  && func_abs(imageflag.image_curvature[1][1] - imageflag.image_curvature[1][2]) <= 25
      && Find_Boundary_r[2][1]<60 )
     {
         imageflag.straight[1]=1;              //近段直线
         if(imageflag.image_curvature[1][3] > 168 && abs( imageflag.image_curvature[1][3] - imageflag.image_curvature[1][1]) <= 15 )
         {
             imageflag.straight[1] = 2;  //中段
             if(imageflag.image_curvature[1][4] > 168 && abs( imageflag.image_curvature[1][4] - imageflag.image_curvature[1][1]) <= 15 )
             {
                 imageflag.straight[1] = 3;       //全局直线
             }
         }
     }
     else imageflag.straight[1]=0;




     if( abs( 90- imageflag.image_curvature[0][5]) >= 15 && imageflag.straight[0]==0 )                   //弯路
     {
         imageflag.Bend_Road=2;
     }
     else if( abs( 90- imageflag.image_curvature[1][5]) >= 15 && imageflag.straight[1]==0 )
     {
         imageflag.Bend_Road=1;
     }
     else imageflag.Bend_Road=0;

}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     Stayguy_ADS  补线函数
// 参数说明      x_start     补线起始点横坐标
//        y_start     补线起始点纵坐标
//        x_start     补线目标点横坐标
//        y_start     补线目标点纵坐标
// 返回参数     void
// 使用示例     Stayguy_ADS(0,0,1,1);
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
                ex_mt9v03x_binarizeImage[y_start][x_start] = 0;
                ex_mt9v03x_binarizeImage[y_start][x_start+1] = 0;
                y_start += y_dir;
            }
            break;
        }

        if(func_abs(y_start - y_end) > func_abs(x_start - x_end))
        {
            while(y_start != y_end)
            {
                ex_mt9v03x_binarizeImage[y_start][x_start] = 0;
                ex_mt9v03x_binarizeImage[y_start][x_start+1] = 0;
                y_start += y_dir;
                x_start = (int16)(((float)y_start - temp_b) / temp_rate);
            }
        }
        else
        {
            while(x_start != x_end)
            {
                ex_mt9v03x_binarizeImage[y_start][x_start] = 0;
                ex_mt9v03x_binarizeImage[y_start][x_start+1] = 0;
                x_start += x_dir;
                y_start = (int16)((float)x_start * temp_rate + temp_b);
            }
        }
    }while(0);
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介    Angel_compute  获取三个坐标形成的夹角
// 参数说明      x2  所求角的顶点横坐标
//        y2  所求角的顶点纵坐标
//        x1,y1,x3,y3所求点另外两个边上的点的坐标
// 返回参数     void
// 使用示例     Angel_compute(0,0,1,0,0,1);
// 备注信息     根据计算与二维图像角度不符，推测为通过平行四边形法则利用二维图像计算三维中的角度
//-------------------------------------------------------------------------------------------------------------------
double Angel_compute(int x1, int y1, int x2, int y2, int x3, int y3)
 {
     double dotProduct = (double)((x2 - x1) * (x2 - x3) + (y2 - y1) * (y2 - y3));
     double magProduct =
             sqrt((double)((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1))) * sqrt((double)((x2 - x3) * (x2 - x3) + (y2 - y3) * (y2 - y3)));

     // 确保在 [-1, 1] 范围内
     double cos_angle = dotProduct / magProduct;
//     cos_angle = cos_angle < -1 ? -1 : (cos_angle > 1 ? 1 : cos_angle);

     double radian_angle = acos(cos_angle);
     double degree_angle = radian_angle * 180.0 / M_PI;

     return degree_angle;
 }
//-------------------------------------------------------------------------------------------------------------------
// 函数简介         find_cross_down 找十字左下拐点
// 参数说明         void
// 返回参数         void
// 使用示例         find_cross_down_l();
// 备注信息     `根据巡线所得边线数据对十字路口进行判断，如果所得拐点夹角的角度在一定范围内，则将其判定为十字路口拐点，（标记为十字路口），将十字路口拐点信息记录下来
//-------------------------------------------------------------------------------------------------------------------
void find_cross_down_l(void)
{
    int x[3]={0},y[3]={0};
    double angle=0;
    for(int i = 1 ;i <= 60 && i < l_data_statics; i += 2 )
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
            && ex_mt9v03x_binarizeImage[(y[0]+y[2])/2][(x[2]+x[0])/2] == 0)//ac连线中点一定是 黑色

        {

            angle=Angel_compute( x[0] , y[0] , x[1] , y[1] , x[2] , y[2] );
            if(  func_abs((int)angle - 90) <= 15 )
            {
//                put_int32(7,angle);


                l_Down[0]=Find_Boundary_l[i+6][0];
                l_Down[1]=Find_Boundary_l[i+6][1];
                l_Down[2]=1;
                l_Down[3]=i+10;
                break;
            }
        }

    }
//                put_int32(2,l_Down[2]);
//                if(l_Down[2]==1)
//                         sendimgAndLine_point(swj_RED,l_Down[1],l_Down[0],swj_point_type3);

}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介         find_cross_down_r 找十字右下拐点
// 参数说明         void
// 返回参数         void
// 使用示例         find_cross_down_r();
// 备注信息     `根据巡线所得边线数据对十字路口进行判断，如果所得拐点夹角的角度在一定范围内，则将其判定为十字路口拐点，（标记为十字路口），将十字路口拐点信息记录下来
//-------------------------------------------------------------------------------------------------------------------
void find_cross_down_r(void)
{
    int x[3]={0},y[3]={0};
    double angle=0;
    for(int i = 1;i <= 60 && i < r_data_statics; i += 2 )
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
            && ex_mt9v03x_binarizeImage[(y[0]+y[2])/2][(x[2]+x[0])/2] == 0      //ac连线中点一定是 黑色
             )

        {
            angle=Angel_compute( x[0] , y[0] , x[1] , y[1] , x[2] , y[2] );
            if(  func_abs((int)angle - 90) <= 15 )
            {

//                put_int32(8,angle);


                r_Down[0]=Find_Boundary_r[i+6][0];
                r_Down[1]=Find_Boundary_r[i+6][1];
                r_Down[2]=1;
                r_Down[3]=i+10;
                break;
            }
        }

    }

//    put_int32(4,r_Down[2]);
//    if(r_Down[2]==1)
//    sendimgAndLine_point(swj_BLUE,r_Down[1],r_Down[0],swj_point_type3);
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
    for( ;i < l_data_statics  ; i += 2 )
    {
        x[0] = Find_Boundary_l[i][1];     //a b c 三点  求得是b的角度
        x[1] = Find_Boundary_l[i+8][1];
        x[2] = Find_Boundary_l[i+16][1];

        y[0]=Find_Boundary_l[i][0];
        y[1]=Find_Boundary_l[i+8][0];
        y[2]=Find_Boundary_l[i+16][0];


        if( y[0] > y[2] && y[1] > y[2]
            && x[0] < x[2]
            && x[0] > 2
            && ex_mt9v03x_binarizeImage[(y[0]+y[2])/2][(x[2]+x[0])/2] == 0)//ac连线中点一定是 黑色

        {

            angle=Angel_compute( x[0] , y[0] , x[1] , y[1] , x[2] , y[2] );
            if(  func_abs((int)angle - 90) <= 15 )
            {

//                put_int32(9,angle);

                l_on[0]=Find_Boundary_l[i+8][0];
                l_on[1]=Find_Boundary_l[i+8][1];
                l_on[2]=1;
                l_on[3]=i+5;
                break;
            }
        }
    }
//    put_int32(1, l_on[2]);
//    if(l_on[2]==1)sendimgAndLine_point(swj_WHITE,l_on[1],l_on[0],swj_point_type3);

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
    for(; i < r_data_statics  ; i += 2 )
        {
        x[0] = Find_Boundary_r[i][1];     //a b c 三点  求得是b的角度
        x[1] = Find_Boundary_r[i+8][1];
        x[2] = Find_Boundary_r[i+16][1];

        y[0]=Find_Boundary_r[i][0];
        y[1]=Find_Boundary_r[i+8][0];
        y[2]=Find_Boundary_r[i+16][0];


        if(    y[0] > y[2] && y[1] > y[2]
            && x[0] > x[2]
            && x[0] < 68
            && ex_mt9v03x_binarizeImage[(y[0]+y[2])/2][(x[2]+x[0])/2] == 0      //ac连线中点一定是 黑色
             )

        {
            angle=Angel_compute( x[0] , y[0] , x[1] , y[1] , x[2] , y[2] );
            if(  func_abs((int)angle - 90) <= 15 )
            {
//                put_int32(10,angle);

                r_on[0]=Find_Boundary_r[i+8][0];
                r_on[1]=Find_Boundary_r[i+8][1];
                r_on[2]=1;
                r_on[3]=i+5;

                break;
            }
        }

    }

//    put_int32(3,r_on[2]);
//    if(r_on[2]==1)
//               sendimgAndLine_point(swj_GREEN,r_on[1],r_on[0],swj_point_type3);


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
    for(  int i = 3 ;i < r_data_statics  ; i += 2 )
    {
        x[0] = Find_Boundary_r[i][1];     //a b c 三点  求得是b的角度
        x[1] = Find_Boundary_r[i+8][1];
        x[2] = Find_Boundary_r[i+16][1];

        y[0]=Find_Boundary_r[i][0];
        y[1]=Find_Boundary_r[i+8][0];
        y[2]=Find_Boundary_r[i+16][0];


        if( y[1] > y[0] && x[1] > x[2]
            && x[2] < 68
            && ex_mt9v03x_binarizeImage[(y[0]+y[2])/2][(x[2]+x[0])/2] == 0)//ac连线中点一定是 黑色

        {

            angle=Angel_compute( x[0] , y[0] , x[1] , y[1] , x[2] , y[2] );
            if(  func_abs((int)angle - 90) <= 18 )
            {

                l_on[0]=Find_Boundary_r[i+8][0];
                l_on[1]=Find_Boundary_r[i+8][1];
                l_on[2]=1;
                l_on[3]=i+5;
                break;
            }
        }
    }
//    put_int32(1, l_on[2]);
//        if(l_on[2]==1)sendimgAndLine_point(swj_WHITE,l_on[1],l_on[0],swj_point_type3);
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
    for(  int i = 3 ;i < l_data_statics  ; i += 2 )
    {
        x[0] = Find_Boundary_l[i][1];     //a b c 三点  求得是b的角度
        x[1] = Find_Boundary_l[i+8][1];
        x[2] = Find_Boundary_l[i+16][1];

        y[0]=Find_Boundary_l[i][0];
        y[1]=Find_Boundary_l[i+8][0];
        y[2]=Find_Boundary_l[i+16][0];


        if( y[1] > y[0] && x[1] < x[2]
            && x[0] > 2
            && ex_mt9v03x_binarizeImage[(y[0]+y[2])/2][(x[2]+x[0])/2] == 0)//ac连线中点一定是 黑色

        {

            angle=Angel_compute( x[0] , y[0] , x[1] , y[1] , x[2] , y[2] );
            if(  func_abs((int)angle - 90) <= 18 )
            {

                r_on[0]=Find_Boundary_l[i+8][0];
                r_on[1]=Find_Boundary_l[i+8][1];
                r_on[2]=1;
                r_on[3]=i+5;
                break;
            }

        }
    }
//    put_int32(3,r_on[2]);
//       if(r_on[2]==1)
//                  sendimgAndLine_point(swj_GREEN,r_on[1],r_on[0],swj_point_type3);
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

    if(imageflag.CrossRoad_Flag==1)
    {


        int x1,y1;
        if( (r_Down[2] + l_Down[2] + l_on[2] + r_on[2])==3)   // 三补一
        {
           if( l_Down[2] == 0)
           {
               y1 = l_on[0]-r_on[0];
               x1 = l_on[1]-r_on[1];

               l_Down[0] = r_Down[0] + y1+1;
               l_Down[1] = r_Down[1] + x1-1;
               l_Down[2] = 1 ;
           }
            else if(r_Down[2] == 0)
           {
               y1 = l_on[0]-r_on[0];
               x1 = l_on[1]-r_on[1];

               r_Down[0] = l_Down[0] - y1+1;
               r_Down[1] = l_Down[1] - x1+1;
               r_Down[2] = 1 ;
           }
            else if(l_on[2] == 0)
           {
               y1 = l_Down[0]-r_Down[0];
               x1 = l_Down[1]-r_Down[1];
               l_on[0] = r_on[0] + y1-2;
               l_on[1] = r_on[1] + x1-2;
               l_on[2] = 1;
           }
           else
           {
               y1 = r_Down[0]-l_Down[0];
               x1 = r_Down[1]-l_Down[1];
               r_on[0] = l_on[0] + y1-2;
               r_on[1] = l_on[1] + x1+2;
               r_on[2] = 1;
           }
        }
//
//put_int32(6,(int)(sqrt( ( l_Down[0] - l_on[0] )  *( l_Down[0] - l_on[0] )
//        +( ( l_Down[1] - l_on[1] )  * ( l_Down[1] - l_on[1] ) ) ) ));


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



    else if( imageflag.CrossRoad_Flag==2 || imageflag.CrossRoad_Flag==3 )
    {



        if( r_on[2]==1 && l_on[2]==1
                && func_abs(  track_width - (int)(sqrt( ( l_on[0] - r_on[0] )  *( l_on[0] - r_on[0] )
                +( ( l_on[1] - r_on[1] )  * ( l_on[1] - r_on[1] ) ) ) ) )<= 3 )
        {
            l_on[2] = 1 ;
            r_on[2] = 1 ;
        }
        else if(r_on[2]==1);
        else if(l_on[2]==1);
        else
        {
            l_on[2] = 0 ;
            r_on[2] = 0 ;
        }

    }


    else if(imageflag.CrossRoad_Flag==4)
    {

        if( l_on[2]==1 && l_Down[2]==1
             &&(func_abs(  track_width - (int)(sqrt( ( l_on[0] - l_Down[0] )  *( l_on[0] - l_Down[0] )
              +( ( l_on[1] - l_Down[1] )  * ( l_on[1] - l_Down[1] ) ) ) ) )<= 3) )
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
               imageflag.CrossRoad_Flag = 1;  //正常进入
               //lr_en_speed=0;   //清除编码器积分 使得进入第二阶段
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


//    put_int32(6,(int)sqrt( ( l_on[0] - l_Down[0] )  *( l_on[0] - l_Down[0] )
//            +( ( l_on[1] - l_Down[1] )  * ( l_on[1] - l_Down[1] ) ) ));


    if(  l_on[2]==1 && l_Down[2]==1
      && func_abs(  track_width - (int)sqrt( ( l_on[0] - l_Down[0] )  *( l_on[0] - l_Down[0] )
        +( ( l_on[1] - l_Down[1] )  * ( l_on[1] - l_Down[1] ) ) ) )<= 4   )//两点间距离合适
    {
            imageflag.CrossRoad_Flag=4;
            //lr_en_speed=0;
    }
    else if(  r_on[2]==1 && r_Down[2]==1
          && func_abs(  track_width - (int)sqrt( ( r_on[0] - r_Down[0] )  *( r_on[0] - r_Down[0] )
            +( ( r_on[1] - r_Down[1] )  * ( r_on[1] - r_Down[1] ) ) ) )<= 4   )//两点间距离合适
        {
                imageflag.CrossRoad_Flag=4;
                //lr_en_speed=0;
        }
    else
    {
        l_on[2]=0; l_Down[2]=0;
        r_on[2]=0; r_Down[2]=0;
        imageflag.CrossRoad_Flag=0;
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

    if(imageflag.CrossRoad_Flag==1)
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

        if(l_shade_255 >= 20 && r_shade_255 >= 20)
        {

            imageflag.CrossRoad_Flag = 2;
            find_cross_on_l();
            find_cross_on_r();
            //lr_en_speed=0;
        }
//        if( //lr_en_speed >= 11000 )   //编码器积分强制清除
//        {
//            imageflag.CrossRoad_Flag = 2;
//            find_cross_on_l();
//            find_cross_on_r();
//            //lr_en_speed = 0;
//        }
    }
    else if(imageflag.CrossRoad_Flag==2||imageflag.CrossRoad_Flag==3)
    {
        find_cross_on_l();
        find_cross_on_r();

        cross_checkout();


        if(l_shade_255 <= 5 && r_shade_255 <= 5)
        {
            imageflag.CrossRoad_Flag = 3;
            find_cross_on_l();
            find_cross_on_r();
        }
//        if( //lr_en_speed >= 7000 )
//        {
//            imageflag.CrossRoad_Flag = 0;
//        }


    }


    else if(imageflag.CrossRoad_Flag==4) //斜入十字
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
            imageflag.CrossRoad_Flag=1;
            find_cross_down_l();
            find_cross_down_r();
            find_cross_on_l();
            find_cross_on_r();
        }
        if( (l_shade_255 >= 20 && r_shade_255 >= 20) ||(l_on[2] == 1 && r_on[2] == 1 ))
        {
            imageflag.CrossRoad_Flag = 2;
            find_cross_on_l();
            find_cross_on_r();
            //lr_en_speed=0;
        }
//        if( //lr_en_speed >= 4300 )   //编码器积分强制清除
//        {
//            imageflag.CrossRoad_Flag = 2;
//            find_cross_on_l();
//            find_cross_on_r();
//            //lr_en_speed = 0;
//        }
    }




//开始十字补线-------------------------------------------------------------

    if(imageflag.CrossRoad_Flag == 1 )
    {
       if( l_Down[2] == 1 && l_on[2] == 1)     Stayguy_ADS(l_Down[1]-1,l_Down[0]+1,l_on[1],l_on[0]-1);
       if( r_Down[2] == 1 && r_on[2] == 1)     Stayguy_ADS(r_Down[1]+1,r_Down[0]+1,r_on[1],r_on[0]-1);

       if( l_Down[2] == 0 && l_on[2] == 1 )    Stayguy_ADS(21,90,l_on[1],l_on[0]-1);
       if( r_Down[2] == 0 && r_on[2] == 1 )    Stayguy_ADS(48,90,r_on[1],r_on[0]-1);
    }

    else if(imageflag.CrossRoad_Flag==2  ||  imageflag.CrossRoad_Flag == 3 )
    {
       if(l_on[2]==1)     Stayguy_ADS(21,90,l_on[1],l_on[0]-1);
       if(r_on[2]==1)     Stayguy_ADS(48,90,r_on[1],r_on[0]-1);
    }
    else if(imageflag.CrossRoad_Flag==4)
    {
        if( l_Down[2] == 1 && l_on[2] == 1)     Stayguy_ADS(l_Down[1]-1,l_Down[0]+1,l_on[1],l_on[0]-1);
        if( r_Down[2] == 1 && r_on[2] == 1)     Stayguy_ADS(r_Down[1]+1,r_Down[0]+1,r_on[1],r_on[0]-1);

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
    for( int i = 3;i < l_data_statics  ; i += 2 )
    {
        x[0] = Find_Boundary_l[i][1];     //a b c 三点  求得是b的角度
        x[1] = Find_Boundary_l[i+8][1];
        x[2] = Find_Boundary_l[i+16][1];

        y[0]=Find_Boundary_l[i][0];
        y[1]=Find_Boundary_l[i+8][0];
        y[2]=Find_Boundary_l[i+16][0];





        if( x[0] > 2 && x[2] > x[0]
            && ex_mt9v03x_binarizeImage[(y[0]+y[2])/2][(x[2]+x[0])/2] == 0)//ac连线中点一定是 黑色

        {

            angle=Angel_compute( x[0] , y[0] , x[1] , y[1] , x[2] , y[2] );

            if( angle >= 60 && angle <= 100)
            {

                is_L_on[0]=Find_Boundary_l[i+8][0];
                is_L_on[1]=Find_Boundary_l[i+8][1];
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
void find_island_incline_on_l(void)
{
    int x[3]={0},y[3]={0};
    double angle=0;
    for( int i = 3;i < r_data_statics  ; i += 2 )
    {
        x[0] = Find_Boundary_r[i][1];     //a b c 三点  求得是b的角度
        x[1] = Find_Boundary_r[i+8][1];
        x[2] = Find_Boundary_r[i+16][1];

        y[0]=Find_Boundary_r[i][0];
        y[1]=Find_Boundary_r[i+8][0];
        y[2]=Find_Boundary_r[i+16][0];


        if( x[0] < 68 && x[0] > x[2]
            && ex_mt9v03x_binarizeImage[(y[0]+y[2])/2][(x[2]+x[0])/2] == 0)//ac连线中点一定是 黑色

        {

            angle=Angel_compute( x[0] , y[0] , x[1] , y[1] , x[2] , y[2] );



            if( angle >= 60 && angle <= 100 )
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
void find_island_down_l(void)
{
    int x[3]={0},y[3]={0};
    double angle=0;
    for( int i = 3;i < r_data_statics  ; i += 2 )
    {
        x[0] = Find_Boundary_r[i][1];     //a b c 三点  求得是b的角度
        x[1] = Find_Boundary_r[i+8][1];
        x[2] = Find_Boundary_r[i+16][1];

        y[0]=Find_Boundary_r[i][0];
        y[1]=Find_Boundary_r[i+8][0];
        y[2]=Find_Boundary_r[i+16][0];


        if( x[0] < 68 && y[2] < y[0] && x[2] > x[1]
            && ex_mt9v03x_binarizeImage[(y[0]+y[2])/2][(x[2]+x[0])/2] == 0)//ac连线中点一定是 黑色

        {

            angle=Angel_compute( x[0] , y[0] , x[1] , y[1] , x[2] , y[2] );



            if( angle >= 60 && angle <= 90 )
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
void island_judgment(void)
{
    if(imageflag.straight[1] >=2 && imageflag.straight[0] == 0 )
    {
        find_island_on_l();
        if(is_L_on[2]==1 && is_L_on[0] >= island_in_limit_x && is_L_on[0] <= island_in_limit_x +10 )  //初次寻找拐点 限制 拐点高度
        {
            imageflag.image_element_rings[0]=1;
//            if(g_attitude.yaw + 55 >= 180)
//            angle.yaw = g_attitude.yaw + 55 -360.0;   //入环初始角度
//            else
//            angle.yaw = g_attitude.yaw + 55;
        }
        else  imageflag.image_element_rings[0]=0;

    }
    else if (imageflag.straight[0]>2 && imageflag.straight[1] == 0)
    {

    }
}


//-------------------------------------------------------------------------------------------------------------------
// 函数简介      环岛处理
// 参数说明
// 返回参数
// 使用示例
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
//void island_dispose(void)
//{
////    buzzer=10;
//
//    if(imageflag.image_element_rings[0] == 1 )  //入环找左上拐点
//    {
//        find_island_on_l();
//
//        if(is_L_on[2]==1 && is_L_on[0] <= island_in_limit_x-3)imageflag.image_element_rings[0] = 0;
//
//        if(is_L_on[2]==0)
//        {
//            find_island_incline_on_l();
//        }
//
//        if(f_abs(g_attitude.yaw - angle.yaw)<5.0)
//        {
//            imageflag.image_element_rings[0] = 2;
//            if(g_attitude.yaw + 170 >=180)
//            angle.yaw = g_attitude.yaw + 170 -360.0;   //入环初始角度
//            else
//            angle.yaw = g_attitude.yaw + 170;
//        }
//
//
//    }
//    else  if(imageflag.image_element_rings[0] == 2)  //环中旋转 找角度
//    {
//        if(f_abs(g_attitude.yaw - angle.yaw)<5.0)
//        {
//            imageflag.image_element_rings[0] = 3;
//
//            find_island_down_l();
//
//
//            if(g_attitude.yaw + 100 >=180)
//            angle.yaw = g_attitude.yaw + 100 -360.0;
//            else
//            angle.yaw = g_attitude.yaw + 100;
//        }
//    }
//    else if(imageflag.image_element_rings[0] == 3)  //寻找右下拐点
//    {
//        find_island_down_l();
//        if(f_abs(g_attitude.yaw - angle.yaw)<5.0)
//        {
//            lr_en_speed = 0;
//            imageflag.image_element_rings[0] = 4;
//            find_island_on_l();
//        }
//
//
//    }
//    else if( imageflag.image_element_rings[0] == 4) //找左上出环
//    {
//        find_island_on_l();
//
//        if(lr_en_speed >= 9000)imageflag.image_element_rings[0]=0;
//    }
//
//
//
//
//
//
//
////开始环岛补线-------------------------------------------------------------
//
//    if(imageflag.image_element_rings[0]==1 && is_L_on[2]==1)
//    {
//        Stayguy_ADS(50,91,is_L_on[1] ,is_L_on[0]);
//    }
//
//
//    else if(imageflag.image_element_rings[0]==3 )
//    {
//        find_island_down_l();
//        if(is_L_down[2] == 1)
//        {
//            Stayguy_ADS(1,60,is_L_down[1] ,is_L_down[0]);
//            lr_en_speed=0;
//        }
//        else
//        {
//            Stayguy_ADS(1,65,50,90);
//        }
//
//    }
//    else if(imageflag.image_element_rings[0]==4 )
//    {
//
//        if(is_L_on[2] == 1)
//        {
//            Stayguy_ADS(is_L_on[1],is_L_on[0],15,90);
//        }
//        else
//        {
//            Stayguy_ADS(1,37,15,90);
//        }
//    }
//
//
//
//}
//
//
//
//
//




//-------------------------------------------------------------------------------------------------------------------
// 函数简介     midline_scan()   中线扫描
// 参数说明
// 返回参数
// 使用示例
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void midline_scan(void)
{
    if(imageflag.straight[0]>=2 && imageflag.image_element_rings[0] == 0 && imageflag.image_element_rings[1] == 0 && imageflag.CrossRoad_Flag==0)
    {
        for(int i=0;i<=l_data_statics;i++)
        {
           Find_Boundary_z[i][1]=Find_Boundary_l[i][1]+track_width/2;
           Find_Boundary_z[i][0]=Find_Boundary_l[i][0];
           ex_mt9v03x_binarizeImage[Find_Boundary_z[i][0]][Find_Boundary_z[i][1]]=0;
        }
    }
    else if(imageflag.straight[1]>=2 && imageflag.image_element_rings[0] == 0 && imageflag.image_element_rings[1] == 0 && imageflag.CrossRoad_Flag==0)
    {
        for(int i=0;i<=r_data_statics;i++)
        {
           Find_Boundary_z[i][1]=Find_Boundary_r[i][1]-track_width/2;
           Find_Boundary_z[i][0]=Find_Boundary_r[i][0];
           ex_mt9v03x_binarizeImage[Find_Boundary_z[i][0]][Find_Boundary_z[i][1]]=0;
        }
    }
    else
    {
        for(int i=0;i<=(r_data_statics+l_data_statics)/2;i++)
        {

           Find_Boundary_z[i][1]=(Find_Boundary_r[i][1]+Find_Boundary_l[i][1])/2;
           Find_Boundary_z[i][0]=(Find_Boundary_r[i][0]+Find_Boundary_l[i][0])/2;
           ex_mt9v03x_binarizeImage[Find_Boundary_z[i][0]][Find_Boundary_z[i][1]]=0;
        }
    }

}



void Inflexion_Point(void)  //四个拐点  显示
{

    tft180_draw_point( 100 + l_Down[1] + 0 , 150 + l_Down[0] + 0 ,RGB565_RED);
    tft180_draw_point( 100 + l_Down[1] + 0 , 150 + l_Down[0] - 1 ,RGB565_RED);
    tft180_draw_point( 100 + l_Down[1] + 0 , 150 + l_Down[0] - 2 ,RGB565_RED);
    tft180_draw_point( 100 + l_Down[1] + 1 , 150 + l_Down[0] + 0 ,RGB565_RED);
    tft180_draw_point( 100 + l_Down[1] + 1 , 150 + l_Down[0] - 1 ,RGB565_RED);
    tft180_draw_point( 100 + l_Down[1] + 2 , 150 + l_Down[0] + 0 ,RGB565_RED);
    tft180_draw_point( 100 + l_Down[1] - 1 , 150 + l_Down[0] + 1 ,RGB565_RED);

    tft180_draw_point( 100 + l_on[1] + 0 , 150 + l_on[0]+ 0 ,RGB565_YELLOW);
    tft180_draw_point( 100 + l_on[1] + 0 , 150 + l_on[0] - 1 ,RGB565_YELLOW);
    tft180_draw_point( 100 + l_on[1] + 0 , 150 + l_on[0] - 2 ,RGB565_YELLOW);
    tft180_draw_point( 100 + l_on[1] - 1 , 150 + l_on[0] + 0 ,RGB565_YELLOW);
    tft180_draw_point( 100 + l_on[1] - 1 , 150 + l_on[0] - 1 ,RGB565_YELLOW);
    tft180_draw_point( 100 + l_on[1] - 2 , 150 + l_on[0] + 0 ,RGB565_YELLOW);
    tft180_draw_point( 100 + l_on[1] + 1 , 150 + l_on[0] + 1 ,RGB565_YELLOW);

    tft180_draw_point( 100 + r_Down[1] + 0 , 150 + r_Down[0] + 0 ,RGB565_PURPLE);
    tft180_draw_point( 100 + r_Down[1] + 0 , 150 + r_Down[0] + 1 ,RGB565_PURPLE);
    tft180_draw_point( 100 + r_Down[1] + 0 , 150 + r_Down[0] + 2 ,RGB565_PURPLE);
    tft180_draw_point( 100 + r_Down[1] + 1 , 150 + r_Down[0] + 0 ,RGB565_PURPLE);
    tft180_draw_point( 100 + r_Down[1] + 1 , 150 + r_Down[0] + 1 ,RGB565_PURPLE);
    tft180_draw_point( 100 + r_Down[1] + 2 , 150 + r_Down[0] + 0 ,RGB565_PURPLE);
    tft180_draw_point( 100 + r_Down[1] - 1 , 150 + r_Down[0] - 1 ,RGB565_PURPLE);

    tft180_draw_point( 100 + r_on[1] + 0 , 150 + r_on[0] + 0 ,RGB565_CYAN);
    tft180_draw_point( 100 + r_on[1] + 0 , 150 + r_on[0] + 1 ,RGB565_CYAN);
    tft180_draw_point( 100 + r_on[1] + 0 , 150 + r_on[0] + 2 ,RGB565_CYAN);
    tft180_draw_point( 100 + r_on[1] - 1 , 150 + r_on[0] + 0 ,RGB565_CYAN);
    tft180_draw_point( 100 + r_on[1] - 1 , 150 + r_on[0] + 1 ,RGB565_CYAN);
    tft180_draw_point( 100 + r_on[1] - 2 , 150 + r_on[0] + 0 ,RGB565_CYAN);
    tft180_draw_point( 100 + r_on[1] + 1 , 150 + r_on[0] - 1 ,RGB565_CYAN);
}

void xianshi_a(void)
{
    for(int i=0 ;i < l_data_statics ; i++)
       {

         if(Find_Boundary_l[i][1] == Find_Boundary_l[i+1][1] && Find_Boundary_l[i][0] == Find_Boundary_l[i+1][0])
        {
            break;
        }

         tft180_draw_point( Find_Boundary_l[i][1] + 100+2, Find_Boundary_l[i][0] +150,RGB565_RED);
         tft180_draw_point( Find_Boundary_l[i][1] + 100+ 3, Find_Boundary_l[i][0] +150,RGB565_RED);
         tft180_draw_point( Find_Boundary_l[i][1] + 100 + 1, Find_Boundary_l[i][0] +150,RGB565_RED);
         tft180_draw_point( Find_Boundary_l[i][1] + 100 +4, Find_Boundary_l[i][0] +150 ,RGB565_RED);


       }
       for(int i=0 ; i < r_data_statics ; i++)
       {
         if(Find_Boundary_r[i][1] == Find_Boundary_r[i-1][1] && Find_Boundary_r[i][0] == Find_Boundary_r[i-1][0])
         {
             break;
         }
         tft180_draw_point( Find_Boundary_r[i][1] + 100-2 , Find_Boundary_r[i][0] +150 ,RGB565_BLUE);
         tft180_draw_point( Find_Boundary_r[i][1] + 100-3, Find_Boundary_r[i][0]+150,RGB565_BLUE);
         tft180_draw_point( Find_Boundary_r[i][1] + 100-4, Find_Boundary_r[i][0]+150 ,RGB565_BLUE);
         tft180_draw_point( Find_Boundary_r[i][1] + 100-1, Find_Boundary_r[i][0] +150,RGB565_BLUE);




       }

       for(int i=0;i<=(r_data_statics+l_data_statics)/2;i++)
       {

           tft180_draw_point( Find_Boundary_z[i][1] + 100 , Find_Boundary_z[i][0] + 150 ,RGB565_PURPLE);
           tft180_draw_point( Find_Boundary_z[i][1] + 101 , Find_Boundary_z[i][0] + 150 ,RGB565_PURPLE);
       }

}

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
//    if(lr_en_speed>60000)lr_en_speed=0;

    is_L_on[2] = 0;
    is_L_down[2] = 0;

    is_L_on[0]=0;
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介    Camera_tracking()   图像处理总函数
// 参数说明
// 返回参数
// 使用示例
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
uint8 hightest = 0;//最高点
uint16 centre=0,centre_last=0,centre_s=0;//舵机使用的中线值  centre_s 是为了看前方弯道程度之后 分段pd
void Camera_tracking(void)
{

    empty_flag(); //标志位清除
    Binaryzation(); //大津法求阈值
    image_draw();  //二值化 画框 去噪点
    shade_compute();//看左下角阴影

    search((uint16)USE_num, ex_mt9v03x_binarizeImage,&hightest);


//直道 弯道 判断-----------------------------
    curvature_judge();


//元素判断-----------------------------------


    if(imageflag.Zebra_Flag==0 && imageflag.image_element_rings[0]==0 && imageflag.image_element_rings[1] == 0)
    {
        if(imageflag.CrossRoad_Flag == 0)
            cross_judgment_positive();//正入十字
        if(imageflag.CrossRoad_Flag == 0)
            cross_judgment_slanting();

        if(imageflag.CrossRoad_Flag != 0) //这个标志位代表十字不同的过程
            cross_dispose();
    }

    if(imageflag.CrossRoad_Flag == 0 && imageflag.Zebra_Flag==0)
    {
        if(imageflag.image_element_rings[0] == 0 && imageflag.image_element_rings[1] == 0 )
            island_judgment();
//        if(imageflag.image_element_rings[0] != 0 && imageflag.image_element_rings[1] == 0 )
//            island_dispose();
//
//        else
//            if(imageflag.image_element_rings[0] == 0 && imageflag.image_element_rings[1] != 0)
//        {
//
//        }

    }

//    put_int32(0,imageflag.image_element_rings[0]);





    search((uint16)USE_num, ex_mt9v03x_binarizeImage,&hightest);
//
//    curvature_judge();

    midline_scan();




    uint16 a=0;
    for(int i=2;i<24;i++)
    {
        a += Find_Boundary_z[i][1];
    }
    centre=a;
    for(int i=24;i<37;i++)
    {
        a += Find_Boundary_z[i][1];
    }
    centre_s=a;

}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介
// 参数说明
// 返回参数
// 使用示例
// 备注信息
//-------------------------------------------------------------------------------------------------------------------



