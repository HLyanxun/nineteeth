/*
 * remake_camera.c
 *
 *  Created on: 2024��7��4��
 *      Author: pc
 */
#include "zf_common_headfile.h"
#include "math.h"

//��͸��---------------------------------------------------------------------------------------------------------------
uint8_t *PerImg_ip[RESULT_ROW][RESULT_COL]={{0}};//�洢ͼ���ַ
//��ֵ��---------------------------------------------------------------------------------------------------------------
uint8 My_Threshold;
uint8 My_Threshold_1;
//������ͼ�񼰴������------------------------------------------------------------------------------------------------------
uint8 image[image_h][image_w]={{0}};  //ʹ�õ�ͼ��
static uint8* PicTemp;                          //һ�����浥��ͼ���ָ�����
static int Ysite = 0, Xsite = 0;                //Ysite����ͼ����У�Xsite����ͼ����С�
static int BottomBorderRight = image_side_width,              //89�е��ұ߽�
BottomBorderLeft = 0,                           //89�е���߽�
Bottommidline = 0;                               //89�е��е�
uint8 ExtenLFlag = 0;                           //������Ƿ���Ҫ���ߵı�־����
uint8 ExtenRFlag = 0;                           //�ұ����Ƿ���Ҫ���ߵı�־����
int Right_RingsFlag_Point1_Ysite, Right_RingsFlag_Point2_Ysite; //��Բ���жϵ�����������
int Left_RingsFlag_Point1_Ysite, Left_RingsFlag_Point2_Ysite;   //��Բ���жϵ�����������
uint8 Ring_Help_Flag = 0;                       //����������־
int Point_Xsite,Point_Ysite;                   //�յ��������
int Repair_Point_Xsite,Repair_Point_Ysite;     //���ߵ��������

//ͼ�����--------------------------------------------------------------------------------------------------------------
Sideline_status Sideline_status_array[90];
Image_Status imagestatus;
ImageFlagtypedef imageflag;



//--------------------------------------------------------------------------------------
// �������     �жϷ�Χ�ڵı����Ƿ�Ϊֱ��
// ����˵�� dir    1�����ж�����ߣ�2�����ж��ұ��ߣ�3�����ж�����
// ����ֵ
//--------------------------------------------------------------------------------------
float Straight_Judge(uint8 dir, uint8 start, uint8 end)     //���ؽ��С��1��Ϊֱ��
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
// �������     ���ʽһ����ʼ��
//--------------------------------------------------------------------------------------
void ALL_init(void)
{
    ImagePerspective_Init();
    mt9v03x_init();
    tft180_init();
}
//--------------------------------------------------------------------------------------
// �������     ȥ����+��͸�Ӵ����ʼ��
// ��ע��Ϣ     ֻ�����һ�μ��ɣ����ʹ��ImageUsedָ�뼴��
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


/*ImageUsed[0][0]����ͼ�����Ͻǵ�ֵ*/

/*�������ͷ��ʼ���󣬵���һ��ImagePerspective_Init���˺�ֱ�ӵ���ImageUsed   ��Ϊȥ������*/
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// �������     ��ʾ����ͷѲ�߽��
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
void camera_tft180show(void)
{
    if(image_type)
    {
        tft180_show_gray_image(0, 0, mt9v03x_image[0], MT9V03X_W, MT9V03X_H, MT9V03X_W/2, MT9V03X_H/2, 0);

    }else {
        tft180_show_gray_image(0, 0, image[0], image_w, image_h, image_w, image_h, 0);
    }
    float a;
//    a=Straight_Judge(2, 50, 70);
    a=abs(angle_compute(Sideline_status_array[(imagestatus.OFFLine+10)].rightline,(imagestatus.OFFLine+10),Sideline_status_array[70].rightline,70));
    tft180_show_float(0, 140, a, 3, 3);

    tft180_show_int(0,92, imagestatus.OFFLine, 3);
    tft180_show_int(0,108, imageflag.image_element_rings, 3);
    tft180_show_int(0,124, imageflag.image_element_rings_flag, 3);
    if(track_show)
    {
        for(uint8 i=imagestatus.OFFLine;i<=image_bottom_value;i++)
            {

//            xx=Sideline_status_array[Ysite].midline;
//            LimitL(Sideline_status_array[Ysite].midline);  //�޷�
//            LimitH(Sideline_status_array[Ysite].midline);  //�޷�
////            xx=Sideline_status_array[Ysite].leftline;
//            LimitL( Sideline_status_array[Ysite].leftline);
//            LimitH( Sideline_status_array[Ysite].leftline);
//
//            LimitL(Sideline_status_array[Ysite].rightline);
//            LimitH(Sideline_status_array[Ysite].rightline);
//
//            LimitL(Sideline_status_array[Ysite].LeftBoundary);  //�޷�
//            LimitH(Sideline_status_array[Ysite].LeftBoundary);  //�޷�
//
//
//            LimitL(Sideline_status_array[Ysite].LeftBoundary_First);  //�޷�
//            LimitH(Sideline_status_array[Ysite].LeftBoundary_First);  //�޷�
//
//
//            LimitL(Sideline_status_array[Ysite].RightBoundary);  //�޷�
//            LimitH(Sideline_status_array[Ysite].RightBoundary);  //�޷�
//
//
//            LimitL(Sideline_status_array[Ysite].RightBoundary_First);  //�޷�
//            LimitH(Sideline_status_array[Ysite].RightBoundary_First);  //�޷�

                /*����Ѳ�߽��*/
//            if(Sideline_status_array[i].IsLeftFind =='W')tft180_draw_line(Sideline_status_array[i].leftline,i,90,i,RGB565_PURPLE);
                tft180_draw_point(Sideline_status_array[i].leftline, i, RGB565_RED);
                tft180_draw_point(Sideline_status_array[i].rightline, i, RGB565_RED);
                tft180_draw_point(Sideline_status_array[i].midline, i, RGB565_RED);



                if(track_width_debug)
                {
                    tft180_draw_line((line_midpoint-track_width/2), 0, (line_midpoint-track_width/2), 90, RGB565_YELLOW);
                    tft180_draw_line((line_midpoint+track_width/2), 0, (line_midpoint+track_width/2), 90, RGB565_YELLOW);
                }
            }
        if(track_show_8)
        {
            for(uint8 i=imagestatus.OFFLineBoundary;i<=image_bottom_value;i++)
            {
                /*������Ѳ�߽��*/
                tft180_draw_point(Sideline_status_array[i].LeftBoundary, i, RGB565_GREEN);
                tft180_draw_point(Sideline_status_array[i].LeftBoundary_First, i, RGB565_BLUE);
                tft180_draw_point(Sideline_status_array[i].RightBoundary, i, RGB565_GREEN);
                tft180_draw_point(Sideline_status_array[i].RightBoundary_First, i, RGB565_BLUE);
            }
        }

    }

}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//  @name           get_Threshold  //ָ��
//  @brief          �Ż�֮��ĵĴ�򷨡���򷨾���һ���ܹ����һ��ͼ����ѵ��Ǹ��ָ���ֵ��һ���㷨��
//  @brief          ����������ǿ������ʵ�ڲ�������ֱ�������ã�ʲô�����������޸ģ�ֻҪû�й���Ӱ�죬��ô������������ֵ��һ�����Եõ�һ��Ч��������Ķ�ֵ��ͼ��
//  @parameter      ex_mt9v03x_binarizeImage  ԭʼ�ĻҶ�ͼ������
//  @parameter      col    ͼ��Ŀ�ͼ����У�
//  @parameter      row    ͼ��ĸߣ�ͼ����У�
//  @return         uint8
//  @time           2022��2��17��
//  @Author
//  Sample usage:   Threshold = Threshold_deal(Image_Use[0], 80, 60); �Ѵ��60��80�еĶ�άͼ������Image_Use��������������ͼ�����ֵ�����������ֵ����Threshold��
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
  for (i = 0; i < GrayScale; i++)                    //�Ȱ�pixelCount��pixelPro��������Ԫ��ȫ����ֵΪ0
  {
    pixelCount[i] = 0;
    pixelPro[i] = 0;
  }

  uint32 gray_sum = 0;
  /**************************************ͳ��ÿ���Ҷ�ֵ(0-255)������ͼ���г��ֵĴ���**************************************/
  for (i = 0; i < height; i += 1)                   //����ͼ���ÿһ�У��ӵ����е���59�С�
  {
    for (j = 0; j < width; j += 1)                  //����ͼ���ÿһ�У��ӵ����е���79�С�
    {
      pixelCount[mt9v03x_image[i][j]]++;          //����ǰ�����ص������ֵ���Ҷ�ֵ����Ϊ����������±ꡣ
      gray_sum += mt9v03x_image[i][j];          //���������Ҷ�ͼ��ĻҶ�ֵ�ܺ͡�
    }
  }
  /**************************************ͳ��ÿ���Ҷ�ֵ(0-255)������ͼ���г��ֵĴ���**************************************/



  /**************************************����ÿ������ֵ���Ҷ�ֵ���������Ҷ�ͼ������ռ�ı���*************************************************/
  for (i = 0; i < GrayScale; i++)
  {
      pixelPro[i] = (float)pixelCount[i] / pixelSum;
  }
  /**************************************����ÿ������ֵ���Ҷ�ֵ���������Ҷ�ͼ������ռ�ı���**************************************************/



  /**************************************��ʼ��������ͼ��ĻҶ�ֵ��0-255������һ��Ҳ�Ǵ����������һ��***************************/
  /*******************Ϊʲô˵������⣿��Ϊ��Ҳ�ǲ���⣡�������������һ����ѧ���⣬��������Ϊ��ѧ��ʽ��***************************/
  float w0, w1, u0tmp, u1tmp, u0, u1, u, deltaTmp, deltaMax = 0;
  w0 = w1 = u0tmp = u1tmp = u0 = u1 = u = deltaTmp = 0;
  for (j = 0; j < GrayScale; j++)
  {
    w0 += pixelPro[j];                          //�����������ÿ���Ҷ�ֵ�����ص���ռ�ı���֮�ͣ����������ֵı�����
    u0tmp += j * pixelPro[j];

    w1 = 1 - w0;
    u1tmp = gray_sum / pixelSum - u0tmp;

    u0 = u0tmp / w0;                            //����ƽ���Ҷ�
    u1 = u1tmp / w1;                            //ǰ��ƽ���Ҷ�
    u = u0tmp + u1tmp;                          //ȫ��ƽ���Ҷ�
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
  /**************************************��ʼ��������ͼ��ĻҶ�ֵ��0-255������һ��Ҳ�Ǵ����������һ��***************************/
  /*******************Ϊʲô˵������⣿��Ϊ��Ҳ�ǲ���⣡�������������һ����ѧ���⣬��������Ϊ��ѧ��ʽ��***************************/

  return threshold;                             //��������ô���д������������ֵ��return��ȥ��
}


//-------------------------------------------------------------------------------------------------------------------
// �������     my_adapt_threshold  ���ζ�ֵ��
// ����˵��
// ���ز���
// ʹ��ʾ��
// ��ע��Ϣ
//-------------------------------------------------------------------------------------------------------------------
uint8 my_adapt_threshold(uint8 *image, uint16 col, uint16 row)   //ע�������ֵ��һ��Ҫ��ԭͼ��
{
   #define GrayScale 256
    uint16 width = col;
    uint16 height = row;
    int pixelCount[GrayScale];
    float pixelPro[GrayScale];
    int i, j, pixelSum = width * height/4;
    uint8 threshold = 0;
    uint8* data = image;  //ָ���������ݵ�ָ��
    for (i = 0; i < GrayScale; i++)
    {
        pixelCount[i] = 0;
        pixelPro[i] = 0;
    }

    uint32 gray_sum=0;
    //ͳ�ƻҶȼ���ÿ������������ͼ���еĸ���
    for (i = 0; i < height; i+=2)
    {
        for (j = 0; j < width; j+=2)
        {
            pixelCount[(int)data[i * width + j]]++;  //����ǰ�ĵ������ֵ��Ϊ����������±�
            gray_sum+=(int)data[i * width + j];       //�Ҷ�ֵ�ܺ�
        }
    }

    //����ÿ������ֵ�ĵ�������ͼ���еı���

    for (i = 0; i < GrayScale; i++)
    {
        pixelPro[i] = (float)pixelCount[i] / pixelSum;
    }

    //�����Ҷȼ�[0,255]
    float w0, w1, u0tmp, u1tmp, u0, u1, u, deltaTmp, deltaMax = 0;


        w0 = w1 = u0tmp = u1tmp = u0 = u1 = u = deltaTmp = 0;
        for (j = 0; j < GrayScale; j++)
        {

                w0 += pixelPro[j];  //��������ÿ���Ҷ�ֵ�����ص���ռ����֮��   ���������ֵı���
                u0tmp += j * pixelPro[j];  //�������� ÿ���Ҷ�ֵ�ĵ�ı��� *�Ҷ�ֵ

               w1=1-w0;
               u1tmp=gray_sum/pixelSum-u0tmp;

                u0 = u0tmp / w0;              //����ƽ���Ҷ�
                u1 = u1tmp / w1;              //ǰ��ƽ���Ҷ�
                u = u0tmp + u1tmp;            //ȫ��ƽ���Ҷ�
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
// �������
// ����˵��
// ���ز���
// ʹ��ʾ��
// ��ע��Ϣ
//-------------------------------------------------------------------------------------------------------------------
uint8 my_adapt_threshold_2(uint8 *image, uint16 col, uint16 row)   //ע�������ֵ��һ��Ҫ��ԭͼ��
{
   #define GrayScale 256
    uint16 width = col;
    uint16 height = row;
    int pixelCount[GrayScale];
    float pixelPro[GrayScale];
    int i, j, pixelSum = width * height/4;
    uint8 threshold = 0;
    uint8* data = image;  //ָ���������ݵ�ָ��
    for (i = 0; i < GrayScale; i++)
    {
        pixelCount[i] = 0;
        pixelPro[i] = 0;
    }

    uint32 gray_sum=0;
    //ͳ�ƻҶȼ���ÿ������������ͼ���еĸ���
    for (i = 1; i < height; i+=2)
    {
        for (j = 1; j < width; j+=2)
        {
            if( data[i * width + j]  > My_Threshold_1 )
            {
                pixelCount[(int)data[i * width + j]]++ ;  //����ǰ�ĵ������ֵ��Ϊ����������±�
                gray_sum+=(int)data[i * width + j] ;       //�Ҷ�ֵ�ܺ�
            }
            else
            {
                pixelCount[(int) My_Threshold_1]++ ;  //����ǰ�ĵ������ֵ��Ϊ����������±�
                gray_sum+= (int) My_Threshold_1 ;       //�Ҷ�ֵ�ܺ�
            }
        }
    }

    //����ÿ������ֵ�ĵ�������ͼ���еı���

    for (i = 0; i < GrayScale; i++)
    {
        pixelPro[i] = (float)pixelCount[i] / pixelSum;
    }

    //�����Ҷȼ�[0,255]
    float w0, w1, u0tmp, u1tmp, u0, u1, u, deltaTmp, deltaMax = 0;


        w0 = w1 = u0tmp = u1tmp = u0 = u1 = u = deltaTmp = 0;
        for (j = 0; j < GrayScale; j++)
        {

                w0 += pixelPro[j];  //��������ÿ���Ҷ�ֵ�����ص���ռ����֮��   ���������ֵı���
                u0tmp += j * pixelPro[j];  //�������� ÿ���Ҷ�ֵ�ĵ�ı��� *�Ҷ�ֵ

               w1=1-w0;
               u1tmp=gray_sum/pixelSum-u0tmp;

                u0 = u0tmp / w0;              //����ƽ���Ҷ�
                u1 = u1tmp / w1;              //ǰ��ƽ���Ҷ�
                u = u0tmp + u1tmp;            //ȫ��ƽ���Ҷ�
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
// �������         ��ֵ��
// ����˵��
// ���ز���
// ʹ��ʾ��
// ��ע��Ϣ
//-------------------------------------------------------------------------------------------------------------------
void Binaryzation(void)
{
    My_Threshold   = (int)get_Threshold()    + My_Threshold_cha;  //1ms
//    My_Threshold_1=(int)my_adapt_threshold(mt9v03x_image[0],MT9V03X_W,MT9V03X_H);
//    My_Threshold =(int)my_adapt_threshold_2(mt9v03x_image[0],MT9V03X_W,MT9V03X_H)+ My_Threshold_cha;
}
//-------------------------------------------------------------------------------------------------------------------
// �������     image_draw  ���ڿ� ���� ��ͼ����ж�ֵ��
// ����˵��
// ���ز���
// ʹ��ʾ��
// ��ע��Ϣ
//-------------------------------------------------------------------------------------------------------------------
void image_draw(void)
{

    for(int i = image_h-3; i >= 0 ; i-- )//����͸��ͼ�������������,�ڽ���һ����ֵ�ݶȵݼ��Ķ�ֵ��
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
    for(int i=0;i<90;i++)       //����� ȥ��
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
//  @brief          �õ����ߺͱ��ߵ����ͣ�������������߷�Ϊ���������ͣ�T���͡�W���ͺ�H���͡��ֱ�����������ߡ��ޱ߱��ߺʹ�������ߡ�
//  @brief          ������һ����Ҫ�뿴���ҵ��߼���ǰ����Ҫ�㶮T��W��H�������͵ı��ߵ�����ʲô���ӵġ�
//  @parameter      p        ָ�򴫽��������һ��ָ�������
//  @parameter      type     ֻ����L������R���ֱ����ɨ����ߺ�ɨ�ұ��ߡ�
//  @parameter      L        ɨ����������� ��Ҳ���Ǵ���һ�п�ʼɨ��
//  @parameter      H        ɨ����������� ��Ҳ����һֱɨ����һ�С�
//  @parameter      Q        ��һ���ṹ��ָ��������Լ�����ȥ��������ṹ������ĳ�Ա��
//  @time           2022��2��20��
//  @Author
//  Sample usage:   Get_SideType_And_Border(PicTemp, 'R', IntervalLow, IntervalHigh,&JumpPoint[1]);
//  Sample usage:   ��PicTemp(PicTemp�Ǹ�ָ�룬ָ��һ������)��IntervalLow�п�ʼɨ��ɨ��IntervalHigh�У�Ȼ��ѵõ��ı������ڵ��кͱ������ͼ�¼��JumpPoint�ṹ���С�
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
void Get_Border_And_SideType(uint8* p,uint8 type,int L,int H,JumpPointtypedef* Q)
{
  int i = 0;
  if (type == 'L')                              //���Type��L(Left),��ɨ������ߡ�
  {
    for (i = L; i <= H; i++)                    //��������ɨ
    {
      if (*(p + i) != 0 && *(p + i - 1) == 0)   //����кڰ�����    1�ǰ� 0�Ǻ�
      {
        Q->point = i;                           //�ǾͰ�����м�¼������Ϊ�����
        Q->type = 'T';                          //���Ұ���һ�е������������䣬�������ͼ�ΪT��������������
        break;                                  //�ҵ��˾�����ѭ��������
      }
      else if (i == H)                    //Ҫ��ɨ�����û�ҵ�
      {
        if (*(p + (L + H) / 2) != 0)            //����ɨ��������м��ǰ����ص�
        {
          Q->point = (L + H) / 2;               //��ô����Ϊ��һ�е�������Ǵ�����ɨ��������е㡣
          Q->type = 'W';                        //���Ұ���һ�е����Ƿ��������䣬�������ͼ�ΪW�����ޱ��С�
          break;                                //����ѭ��������
        }
        else                                    //Ҫ��ɨ�����û�ҵ�������ɨ��������м��Ǻ����ص�
        {
          Q->point = H;                         //��ô����Ϊ��һ�е�������Ǵ�����ɨ��������������ޡ�
          Q->type = 'H';                        //����Ҳ����һ�е����Ƿ��������䣬�����������ͼ�ΪH�����������С�
          break;                                //����ѭ��������
        }
      }
    }
  }
  else if (type == 'R')                         //���Type��R(Right),��ɨ���ұ��ߡ�
  {
    for (i = H; i >= L; i--)                    //��������ɨ
    {
      if (*(p + i) != 0 && *(p + i + 1) == 0)   //����кڰ�����    1�ǰ� 0�Ǻ�
      {
        Q->point = i;                           //�ǾͰ�����м�¼������Ϊ�ұ���
        Q->type = 'T';                          //���Ұ���һ�е������������䣬�������ͼ�ΪT��������������
        break;                                  //�ҵ��˾�����ѭ��������
      }
      else if (i == L)                    //Ҫ��ɨ�����û�ҵ�
      {
        if (*(p + (L + H) / 2) != 0)            //����ɨ��������м��ǰ����ص�
        {
          Q->point = (L + H) / 2;               //��ô����Ϊ��һ�е��ұ����Ǵ�����ɨ��������е㡣
          Q->type = 'W';                        //���Ұ���һ�е����Ƿ��������䣬�������ͼ�ΪW�����ޱ��С�
          break;
        }
        else                                    //Ҫ��ɨ�����û�ҵ�������ɨ��������м��Ǻ����ص�
        {
          Q->point = L;                         //��ô����Ϊ��һ�е��ұ����Ǵ�����ɨ��������������ޡ�
          Q->type = 'H';                        //����Ҳ����һ�е����Ƿ��������䣬�����������ͼ�ΪH�����������С�
          break;                                //����ѭ��������
        }
      }
    }
  }
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//  @name           Get_BaseLine
//  @brief          �ñ����ķ����õ�ͼ����������У�image_bottom_value-55�У��ı��ߺ�������Ϣ�������б��ߺ�������Ϣ��׼ȷ�ȷǳ�����Ҫ��ֱ��Ӱ�쵽����ͼ��Ĵ�������
//  @brief          Get_BaseLine����������Get_AllLine�����Ļ�����ǰ�ᣬGet_AllLine��������Get_BaseLineΪ�����ġ�������Ӧ��Ҳ�ܿ����԰ɣ�һ���еõ������ߣ�һ���еõ������ߡ�
//  @brief          Get_BaseLine������Get_AllLine��������һ��Ҳ����������Ż�֮����ѱ����㷨��
//  @parameter      void
//  @time           2022��2��21��
//  @Author
//  Sample usage:   Get_BaseLine();
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
void Get_BaseLine(void)
{
    /**************************************��������ͼ������У�59�У����ұ��ߴӶ�ȷ�����ߵĹ��� ********************************************************************/
    /****************************************************Begin*****************************************************************************/

    PicTemp = image[image_bottom_value];                                                //��PicTemp���ָ�����ָ��ͼ�������image[59]
    for (Xsite = line_midpoint; Xsite < image_side_width; Xsite++)                   //����39�������У��������п�ʼһ��һ�е����ұ������ұ���
    {
      if (*(PicTemp + Xsite) == 0 && *(PicTemp + Xsite + 1) == 0)       //������������������ڵ㣬˵û�ҵ��˱��ߡ�
      {
        BottomBorderRight = Xsite;                                      //����һ�м�¼������Ϊ��һ�е��ұ���
        break;                                                          //����ѭ��
      }
      else if (Xsite == (image_side_width-1))                                             //����ҵ��˵�58�ж���û���ֺڵ㣬˵����һ�еı��������⡣
      {
        BottomBorderRight = image_side_width;                                         //����������Ĵ�����ǣ�ֱ�Ӽ���ͼ�����ұߵ���һ�У���79�У�������һ�е��ұ��ߡ�
        break;                                                          //����ѭ��
      }
    }

    for (Xsite = line_midpoint; Xsite > 0; Xsite--)                    //����39�������У��������п�ʼһ��һ�е���������������
    {
      if (*(PicTemp + Xsite) == 0 && *(PicTemp + Xsite - 1) == 0)       //������������������ڵ㣬˵û�ҵ��˱��ߡ�
      {
        BottomBorderLeft = Xsite;                                       //����һ�м�¼������Ϊ��һ�е������
        break;                                                          //����ѭ��
      }
      else if (Xsite == 1)                                              //����ҵ��˵�1�ж���û���ֺڵ㣬˵����һ�еı��������⡣
      {
        BottomBorderLeft = 0;                                           //����������Ĵ�����ǣ�ֱ�Ӽ���ͼ������ߵ���һ�У���0�У�������һ�е�����ߡ�
        break;                                                          //����ѭ��
      }
    }

    Bottommidline =(BottomBorderLeft + BottomBorderRight) / 2;           //�������ұ߽�������59�е�����
    Sideline_status_array[image_bottom_value].leftline = BottomBorderLeft;                          //�ѵ�59�е���߽�洢�����飬ע�⿴Sideline_status_array������ֵ��±꣬�ǲ������ö�Ӧ59��
    Sideline_status_array[image_bottom_value].rightline = BottomBorderRight;                      //�ѵ�59�е��ұ߽�洢�����飬ע�⿴Sideline_status_array������ֵ��±꣬�ǲ������ö�Ӧ59��
    Sideline_status_array[image_bottom_value].midline = Bottommidline;                                //�ѵ�59�е����ߴ洢�����飬    ע�⿴Sideline_status_array������ֵ��±꣬�ǲ������ö�Ӧ59��
    Sideline_status_array[image_bottom_value].wide = BottomBorderRight - BottomBorderLeft;          //�ѵ�59�е�������ȴ洢���飬ע�⿴Sideline_status_array������ֵ��±꣬�ǲ������ö�Ӧ59��
    Sideline_status_array[image_bottom_value].IsLeftFind = 'T';                                     //��¼��59�е����������ΪT���������ҵ�����ߡ�
    Sideline_status_array[image_bottom_value].IsRightFind = 'T';                                    //��¼��59�е��ұ�������ΪT���������ҵ��ұ��ߡ�

    /****************************************************End*******************************************************************************/
    /**************************************��������ͼ������У�59�У����ұ��ߴӶ�ȷ�����ߵĹ��� ********************************************************************/



    /**************************************�ڵ�59�������Ѿ�ȷ���������ȷ��58-54���������ߵĹ��� ******************************************/
    /****************************************************Begin*****************************************************************************/
    /*
         * ���漸�еĵ����߹����ҾͲ���׸���ˣ������ҵ�ע�Ͱѵ�59�е����߹������ã�
         * ��ô58��54�е����߾���ȫû���⣬��һģһ�����߼��͹��̡�
     */
    for (Ysite = (image_bottom_value-1); Ysite > (image_bottom_value-5); Ysite--)
    {
        PicTemp = image[Ysite];
        for(Xsite = Sideline_status_array[Ysite + 1].midline; Xsite < image_side_width;Xsite++)
        {
          if(*(PicTemp + Xsite) == 0 && *(PicTemp + Xsite + 1) == 0)
          {
              Sideline_status_array[Ysite].rightline = Xsite;
            break;
          }
          else if (Xsite == (image_side_width-1))
          {
              Sideline_status_array[Ysite].rightline = image_side_width;
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
    /**************************************�ڵ�59�������Ѿ�ȷ���������ȷ��58-54���������ߵĹ��� ****************************************/
}

//void Get_BaseLine(void)
//{
//    /**************************************��������ͼ������У�59�У����ұ��ߴӶ�ȷ�����ߵĹ��� ********************************************************************/
//    /****************************************************Begin*****************************************************************************/
//
//    PicTemp = image[image_bottom_value];                                                //��PicTemp���ָ�����ָ��ͼ�������image[image_bottom_value]
////    for (Xsite = line_midpoint; Xsite < image_side_width; Xsite++)                   //����line_midpoint�������У��������п�ʼһ��һ�е����ұ������ұ���
////    {
////      if (*(PicTemp + Xsite) == 0 && *(PicTemp + Xsite + 1) == 0)       //������������������ڵ㣬˵û�ҵ��˱��ߡ�
////      {
////        BottomBorderRight = Xsite;                                      //����һ�м�¼������Ϊ��һ�е��ұ���
////        break;                                                          //����ѭ��
////      }
////      else if (Xsite == (image_side_width-1))                                             //����ҵ��˵�58�ж���û���ֺڵ㣬˵����һ�еı��������⡣
////      {
////        BottomBorderRight = (image_side_width-1);                                         //����������Ĵ�����ǣ�ֱ�Ӽ���ͼ�����ұߵ���һ�У���79�У�������һ�е��ұ��ߡ�
////        break;                                                          //����ѭ��
////      }
////    }
////
////    for (Xsite = line_midpoint; Xsite > 0; Xsite--)                    //����39�������У��������п�ʼһ��һ�е���������������
////    {
////      if (*(PicTemp + Xsite) == 0 && *(PicTemp + Xsite - 1) == 0)       //������������������ڵ㣬˵û�ҵ��˱��ߡ�
////      {
////        BottomBorderLeft = Xsite;                                       //����һ�м�¼������Ϊ��һ�е������
////        break;                                                          //����ѭ��
////      }
////      else if (Xsite == 1)                                              //����ҵ��˵�1�ж���û���ֺڵ㣬˵����һ�еı��������⡣
////      {
////        BottomBorderLeft = 0;                                           //����������Ĵ�����ǣ�ֱ�Ӽ���ͼ������ߵ���һ�У���0�У�������һ�е�����ߡ�
////        break;                                                          //����ѭ��
////      }
////    }
//    /*ȷ�������*/
//    uint8 temp_min=(line_midpoint-track_width/2)-ImageScanInterval;
//    uint8 temp_max=(line_midpoint-track_width/2)+ImageScanInterval;
//    for (uint8 i = temp_min; i <=temp_max ; i++)                    //��������ɨ
//      {
//        if (*(PicTemp + i) != 0 && *(PicTemp + i - 1) == 0)   //����кڰ�����    1�ǰ� 0�Ǻ�
//        {
//            BottomBorderLeft = i;                           //�ǾͰ�����м�¼������Ϊ�����
//          break;                                  //�ҵ��˾�����ѭ��������
//        }
//        else if (i == temp_max)                    //Ҫ��ɨ�����û�ҵ�
//        {
//          if (*(PicTemp + (temp_min + temp_max) / 2) != 0)            //����ɨ��������м��ǰ����ص�
//          {
//              for(uint8 j=(temp_min-1);j>0;j--)
//              {
//                  if (*(PicTemp + i) != 0 && *(PicTemp + i - 1) == 0)   //����кڰ�����    1�ǰ� 0�Ǻ�
//                  {
//                      BottomBorderLeft = i;                           //�ǾͰ�����м�¼������Ϊ�����
//                      break;                                  //�ҵ��˾�����ѭ��������
//                  }
//                  if(j == 1)
//                  {
//                      BottomBorderLeft = 0;
//                      break;
//                  }
//              }
//              break;
//          }
//          else                                    //Ҫ��ɨ�����û�ҵ�������ɨ��������м��Ǻ����ص�
//          {
//              for(uint8 j=(temp_max+1);j<line_midpoint;j++)
//              {
//                  if (*(PicTemp + i) != 0 && *(PicTemp + i - 1) == 0)   //����кڰ�����    1�ǰ� 0�Ǻ�
//                  {
//                      BottomBorderLeft = i;                           //�ǾͰ�����м�¼������Ϊ�����
//                      break;                                  //�ҵ��˾�����ѭ��������
//                  }
//                  if(j == (line_midpoint-1))
//                  {
//                      BottomBorderLeft = (line_midpoint-1);
//                      break;
//                  }
//              }
//          }
//        }
//      }
//    /*ȷ���ұ���*/
//    temp_min=(line_midpoint+track_width/2)-ImageScanInterval;
//    temp_max=(line_midpoint+track_width/2)+ImageScanInterval;
//    for (uint8 i = temp_max; i <=temp_min ; i--)                    //��������ɨ
//          {
//            if (*(PicTemp + i) != 0 && *(PicTemp + i + 1) == 0)   //����кڰ�����    1�ǰ� 0�Ǻ�
//            {
//                BottomBorderRight = i;                           //�ǾͰ�����м�¼������Ϊ�����
//              break;                                  //�ҵ��˾�����ѭ��������
//            }
//            else if (i == temp_min)                    //Ҫ��ɨ�����û�ҵ�
//            {
//              if (*(PicTemp + (temp_min + temp_max) / 2) != 0)            //����ɨ��������м��ǰ����ص�
//              {
//                  for(uint8 j=(temp_max+1);j<=image_side_width;j++)
//                  {
//                      if (*(PicTemp + i) != 0 && *(PicTemp + i + 1) == 0)   //����кڰ�����    1�ǰ� 0�Ǻ�
//                      {
//                          BottomBorderRight = i;                           //�ǾͰ�����м�¼������Ϊ�����
//                          break;                                  //�ҵ��˾�����ѭ��������
//                      }
//                      if(j == image_side_width)
//                      {
//                          BottomBorderRight = image_side_width;
//                          break;
//                      }
//                  }
//                  break;
//              }
//              else                                    //Ҫ��ɨ�����û�ҵ�������ɨ��������м��Ǻ����ص�
//              {
//                  for(uint8 j=(temp_min-1);j>line_midpoint;j++)
//                  {
//                      if (*(PicTemp + i) != 0 && *(PicTemp + i + 1) == 0)   //����кڰ�����    1�ǰ� 0�Ǻ�
//                      {
//                          BottomBorderRight = i;                           //�ǾͰ�����м�¼������Ϊ�����
//                          break;                                  //�ҵ��˾�����ѭ��������
//                      }
//                      if(j == (line_midpoint+1))
//                      {
//                          BottomBorderRight = (line_midpoint+1);
//                          break;
//                      }
//                  }
//              }
//            }
//          }
//
//    Bottommidline =(BottomBorderLeft + BottomBorderRight) / 2;           //�������ұ߽�������89�е�����
//    Sideline_status_array[image_bottom_value].leftline = BottomBorderLeft;                        //�ѵ�89�е���߽�洢�����飬ע�⿴Sideline_status_array������ֵ��±꣬�ǲ������ö�Ӧ89��
//    Sideline_status_array[image_bottom_value].rightline = BottomBorderRight;                      //�ѵ�89�е��ұ߽�洢�����飬ע�⿴Sideline_status_array������ֵ��±꣬�ǲ������ö�Ӧ89��
//    Sideline_status_array[image_bottom_value].midline = Bottommidline;                                //�ѵ�89�е����ߴ洢�����飬    ע�⿴Sideline_status_array������ֵ��±꣬�ǲ������ö�Ӧ89��
//    Sideline_status_array[image_bottom_value].wide = BottomBorderRight - BottomBorderLeft;          //�ѵ�89�е�������ȴ洢���飬ע�⿴Sideline_status_array������ֵ��±꣬�ǲ������ö�Ӧ89��
//    Sideline_status_array[image_bottom_value].IsLeftFind = 'T';                                     //��¼��89�е����������ΪT���������ҵ�����ߡ�
//    Sideline_status_array[image_bottom_value].IsRightFind = 'T';                                    //��¼��89�е��ұ�������ΪT���������ҵ��ұ��ߡ�
//
//    /****************************************************End*******************************************************************************/
//    /**************************************��������ͼ������У�59�У����ұ��ߴӶ�ȷ�����ߵĹ��� ********************************************************************/
//
//
//
//    /**************************************�ڵ�59�������Ѿ�ȷ���������ȷ��58-54���������ߵĹ��� ******************************************/
//    /****************************************************Begin*****************************************************************************/
//    /*
//         * ���漸�еĵ����߹����ҾͲ���׸���ˣ������ҵ�ע�Ͱѵ�89�е����߹������ã�
//         * ��ô88��84�е����߾���ȫû���⣬��һģһ�����߼��͹��̡�
//     */
//    for (Ysite = (image_side_width-1); Ysite > (image_side_width-5); Ysite--)
//    {
//        PicTemp = image[Ysite];
////        for(Xsite = Sideline_status_array[Ysite + 1].midline; Xsite < image_side_width;Xsite++)
////        {
////          if(*(PicTemp + Xsite) == 0 && *(PicTemp + Xsite + 1) == 0)
////          {
////            Sideline_status_array[Ysite].rightline = Xsite;
////            break;
////          }
////          else if (Xsite == (image_side_width-1))
////          {
////            Sideline_status_array[Ysite].rightline = image_side_width;
////            break;
////          }
////        }
////
////        for (Xsite = Sideline_status_array[Ysite + 1].midline; Xsite > 0;Xsite--)
////        {
////          if (*(PicTemp + Xsite) == 0 && *(PicTemp + Xsite - 1) == 0)
////          {
////            Sideline_status_array[Ysite].leftline = Xsite;
////            break;
////          }
////          else if (Xsite == 1)
////          {
////            Sideline_status_array[Ysite].leftline = 0;
////            break;
////          }
////        }
//        /*ȷ�������*/
//         temp_min=(Sideline_status_array[Ysite + 1].midline-track_width/2)-ImageScanInterval;
//         temp_max=(Sideline_status_array[Ysite + 1].midline-track_width/2)+ImageScanInterval;
//            for (uint8 i = temp_min; i <=temp_max ; i++)                    //��������ɨ
//              {
//                if (*(PicTemp + i) != 0 && *(PicTemp + i - 1) == 0)   //����кڰ�����    1�ǰ� 0�Ǻ�
//                {
//                    BottomBorderLeft = i;                           //�ǾͰ�����м�¼������Ϊ�����
//                  break;                                  //�ҵ��˾�����ѭ��������
//                }
//                else if (i == temp_max)                    //Ҫ��ɨ�����û�ҵ�
//                {
//                  if (*(PicTemp + (temp_min + temp_max) / 2) != 0)            //����ɨ��������м��ǰ����ص�
//                  {
//                      for(uint8 j=(temp_min-1);j>0;j--)
//                      {
//                          if (*(PicTemp + i) != 0 && *(PicTemp + i - 1) == 0)   //����кڰ�����    1�ǰ� 0�Ǻ�
//                          {
//                              BottomBorderLeft = i;                           //�ǾͰ�����м�¼������Ϊ�����
//                              break;                                  //�ҵ��˾�����ѭ��������
//                          }
//                          if(j == 1)
//                          {
//                              BottomBorderLeft = 0;
//                              break;
//                          }
//                      }
//                      break;
//                  }
//                  else                                    //Ҫ��ɨ�����û�ҵ�������ɨ��������м��Ǻ����ص�
//                  {
//                      for(uint8 j=(temp_max+1);j<line_midpoint;j++)
//                      {
//                          if (*(PicTemp + i) != 0 && *(PicTemp + i - 1) == 0)   //����кڰ�����    1�ǰ� 0�Ǻ�
//                          {
//                              BottomBorderLeft = i;                           //�ǾͰ�����м�¼������Ϊ�����
//                              break;                                  //�ҵ��˾�����ѭ��������
//                          }
//                          if(j == (line_midpoint-1))
//                          {
//                              BottomBorderLeft = (line_midpoint-1);
//                              break;
//                          }
//                      }
//                  }
//                }
//              }
//            /*ȷ���ұ���*/
//            temp_min=(Sideline_status_array[Ysite + 1].midline+track_width/2)-ImageScanInterval;
//            temp_max=(Sideline_status_array[Ysite + 1].midline+track_width/2)+ImageScanInterval;
//            for (uint8 i = temp_max; i <=temp_min ; i--)                    //��������ɨ
//                  {
//                    if (*(PicTemp + i) != 0 && *(PicTemp + i + 1) == 0)   //����кڰ�����    1�ǰ� 0�Ǻ�
//                    {
//                        BottomBorderRight = i;                           //�ǾͰ�����м�¼������Ϊ�����
//                      break;                                  //�ҵ��˾�����ѭ��������
//                    }
//                    else if (i == temp_min)                    //Ҫ��ɨ�����û�ҵ�
//                    {
//                      if (*(PicTemp + (temp_min + temp_max) / 2) != 0)            //����ɨ��������м��ǰ����ص�
//                      {
//                          for(uint8 j=(temp_max+1);j<=image_side_width;j++)
//                          {
//                              if (*(PicTemp + i) != 0 && *(PicTemp + i + 1) == 0)   //����кڰ�����    1�ǰ� 0�Ǻ�
//                              {
//                                  BottomBorderRight = i;                           //�ǾͰ�����м�¼������Ϊ�����
//                                  break;                                  //�ҵ��˾�����ѭ��������
//                              }
//                              if(j == image_side_width)
//                              {
//                                  BottomBorderRight = image_side_width;
//                                  break;
//                              }
//                          }
//                          break;
//                      }
//                      else                                    //Ҫ��ɨ�����û�ҵ�������ɨ��������м��Ǻ����ص�
//                      {
//                          for(uint8 j=(temp_min-1);j>line_midpoint;j++)
//                          {
//                              if (*(PicTemp + i) != 0 && *(PicTemp + i + 1) == 0)   //����кڰ�����    1�ǰ� 0�Ǻ�
//                              {
//                                  BottomBorderRight = i;                           //�ǾͰ�����м�¼������Ϊ�����
//                                  break;                                  //�ҵ��˾�����ѭ��������
//                              }
//                              if(j == (line_midpoint+1))
//                              {
//                                  BottomBorderRight = (line_midpoint+1);
//                                  break;
//                              }
//                          }
//                      }
//                    }
//                  }
//
//        Sideline_status_array[Ysite].IsLeftFind  = 'T';
//        Sideline_status_array[Ysite].IsRightFind = 'T';
//        Sideline_status_array[Ysite].midline =(Sideline_status_array[Ysite].rightline + Sideline_status_array[Ysite].leftline)/2;
//        Sideline_status_array[Ysite].wide   = Sideline_status_array[Ysite].rightline - Sideline_status_array[Ysite].leftline;
//    }
//
//    /****************************************************End*****************************************************************************/
//    /**************************************�ڵ�59�������Ѿ�ȷ���������ȷ��58-54���������ߵĹ��� ****************************************/
//}


//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//  @name           Get_AllLine
//  @brief          ��Get_BaseLine�Ļ����ϣ���Բ����������������һЩ����Ĵ����㷨�õ�ʣ���еı��ߺ�������Ϣ��
//  @brief          �������Ӧ����ĿǰΪֹ�Ҵ���������������һ�����ˣ�һ��Ҫ������ʱ�侲������ȥ��������ʱ����������Ҫ���Ǹ���ֵ���ڰ�ͼ��
//  @brief          �������һ�����Ŷ�ֵ���ڰ�ͼ��һ����˼���Ҵ�����߼��Ļ����ܶ�ط��������������ˣ���Ҫ�ⶢ���Ǹ�����һֱ������������û�õģ��мɣ�
//  @brief          �ද��˼������������������ǿ϶�Ҳ���Եġ�������̻�ܿ�������㶼��������֮����ĳ������Ѿ�������ֱ��������ˡ�
//  @parameter      void
//  @time           2023��2��21��
//  @Author
//  Sample usage:   Get_AllLine();
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
void Get_AllLine(void)
{
  uint8 L_Found_T  = 'F';    //ȷ���ޱ�б�ʵĻ�׼�б����Ƿ��ҵ��ı�־
  uint8 Get_L_line = 'F';    //�ҵ���һ֡ͼ��Ļ�׼��б�ʣ�Ϊʲô����Ҫ��ΪF����������Ĵ����֪���ˡ�
  uint8 R_Found_T  = 'F';    //ȷ���ޱ�б�ʵĻ�׼�б����Ƿ��ҵ��ı�־
  uint8 Get_R_line = 'F';    //�ҵ���һ֡ͼ��Ļ�׼��б�ʣ�Ϊʲô����Ҫ��ΪF����������Ĵ����֪���ˡ�
  float D_L = 0;             //������ӳ��ߵ�б��
  float D_R = 0;             //�ұ����ӳ��ߵ�б��
  int ytemp_W_L;             //��ס�״��󶪱���
  int ytemp_W_R;             //��ס�״��Ҷ�����
  ExtenRFlag = 0;            //��־λ��0
  ExtenLFlag = 0;            //��־λ��0
  imagestatus.OFFLine=2;     //����ṹ���Ա��֮���������︳ֵ������Ϊ��imagestatus�ṹ������ĳ�Ա̫���ˣ�������ʱ��ֻ�õ���OFFLine�������������õ��������ĸ�ֵ��
  imagestatus.Miss_Right_lines = 0;
  imagestatus.WhiteLine = 0;
  imagestatus.Miss_Left_lines = 0;
  imageflag.mid_choose=0;
  uint8 mid_choose_value=0;
  for (Ysite = (image_bottom_value-5) ; Ysite > imagestatus.OFFLine; Ysite--)                            //ǰ5����Get_BaseLine()���Ѿ�������ˣ����ڴ�55�д����Լ��趨�Ĳ�������OFFLine��
  {                                                                                  //��Ϊ̫ǰ���ͼ��ɿ��Բ��㣬����OFFLine�����ú��б�Ҫ��û��Ҫһֱ����ɨ����0�С�
    PicTemp = image[Ysite];
    JumpPointtypedef JumpPoint[2];                                                   // JumpPoint[0]��������ߣ�JumpPoint[1]�����ұ��ߡ�

  /******************************ɨ�豾�е��ұ���******************************/
    int IntervalLow  = Sideline_status_array[Ysite + 1].rightline  - ImageScanInterval;               //����һ�е��ұ��߼Ӽ�Interval��Ӧ���п�ʼɨ�豾�У�Intervalһ��ȡ5����Ȼ��Ϊ�˱���������԰����ֵ�ĵĴ�һ�㡣
    int IntervalHigh = Sideline_status_array[Ysite + 1].rightline + ImageScanInterval;              //���������ֻ��Ҫ�����б�������5�Ļ����ϣ����10�е�������䣩ȥɨ�ߣ�һ������ҵ����еı����ˣ��������ֵ��ʵ����̫��
    LimitL(IntervalLow);                                                             //������ǶԴ���GetJumpPointFromDet()������ɨ���������һ���޷�������
    LimitH(IntervalHigh);                                                            //������һ�еı����ǵ�2�У�����2-5=-3��-3�ǲ��Ǿ�û��ʵ�������ˣ���ô����-3���أ�
    Get_Border_And_SideType(PicTemp, 'R', IntervalLow, IntervalHigh,&JumpPoint[1]);  //ɨ���õ�һ���Ӻ������Լ�����ȥ�������߼���
  /******************************ɨ�豾�е��ұ���******************************/

  /******************************ɨ�豾�е������******************************/
    IntervalLow =Sideline_status_array[Ysite + 1].leftline  -ImageScanInterval;                //����һ�е�����߼Ӽ�Interval��Ӧ���п�ʼɨ�豾�У�Intervalһ��ȡ5����Ȼ��Ϊ�˱���������԰����ֵ�ĵĴ�һ�㡣
    IntervalHigh =Sideline_status_array[Ysite + 1].leftline +ImageScanInterval;                //���������ֻ��Ҫ�����б�������5�Ļ����ϣ����10�е�������䣩ȥɨ�ߣ�һ������ҵ����еı����ˣ��������ֵ��ʵ����̫��
    LimitL(IntervalLow);                                                             //������ǶԴ���GetJumpPointFromDet()������ɨ���������һ���޷�������
    LimitH(IntervalHigh);                                                            //������һ�еı����ǵ�2�У�����2-5=-3��-3�ǲ��Ǿ�û��ʵ�������ˣ���ô����-3���أ�
    Get_Border_And_SideType(PicTemp, 'L', IntervalLow, IntervalHigh,&JumpPoint[0]);  //ɨ���õ�һ���Ӻ������Լ�����ȥ�������߼���
  /******************************ɨ�豾�е������******************************/

  /*
       ����Ĵ���Ҫ���뿴�����������ҵ����ڸ���ħ�Ļ���
        ����ذ�GetJumpPointFromDet()����������߼�������
        ������������������棬��T������W������H��������־����ʲô��
        һ��Ҫ�㶮!!!��Ȼ�Ļ������鲻Ҫ���¿��ˣ���Ҫ��ĥ�Լ�!!!
  */
    if(default_side_choose)
       {
           if(JumpPoint[1].type!= 'T' || JumpPoint[1].point > (image_side_width-5))mid_choose_value++;
//           if(JumpPoint[1].type!= 'T' )mid_choose_value++;
       }
       else {
           if(JumpPoint[0].type != 'T' || JumpPoint[0].point < 5)mid_choose_value++;
//           if(JumpPoint[0].type != 'T')mid_choose_value++;
       }

    if (JumpPoint[0].type =='W')                                                     //������е���������ڲ��������䣬����10���㶼�ǰ׵㡣
    {
      Sideline_status_array[Ysite].leftline =Sideline_status_array[Ysite + 1].leftline;                  //��ô���е�����߾Ͳ�����һ�еı��ߡ�
    }
    else                                                                             //������е����������T������H���
    {
      Sideline_status_array[Ysite].leftline = JumpPoint[0].point;                              //��ôɨ�赽�ı����Ƕ��٣��Ҿͼ�¼�����Ƕ��١�
    }

    if (JumpPoint[1].type == 'W')                                                    //������е��ұ������ڲ��������䣬����10���㶼�ǰ׵㡣
    {
      Sideline_status_array[Ysite].rightline =Sideline_status_array[Ysite + 1].rightline;                //��ô���е��ұ��߾Ͳ�����һ�еı��ߡ�
    }
    else                                                                             //������е��ұ�������T������H���
    {
      Sideline_status_array[Ysite].rightline = JumpPoint[1].point;                             //��ôɨ�赽�ı����Ƕ��٣��Ҿͼ�¼�����Ƕ��١�
    }
        if(mid_choose_value>10)imageflag.mid_choose=1;
//        if(Sideline_status_array[Ysite].wide< track_width/2)imageflag.mid_choose=1;
    Sideline_status_array[Ysite].IsLeftFind =JumpPoint[0].type;                                  //��¼�����ҵ�����������ͣ���T����W������H��������ͺ��������õģ���Ϊ�һ�Ҫ��һ������
    Sideline_status_array[Ysite].IsRightFind = JumpPoint[1].type;                                //��¼�����ҵ����ұ������ͣ���T����W������H��������ͺ��������õģ���Ϊ�һ�Ҫ��һ������


  /*
        ����Ϳ�ʼ��W��H���͵ı��߷ֱ���д��� ΪʲôҪ����
        ����㿴����GetJumpPointFromDet�����߼���������T W H�������ͷֱ��Ӧʲô�����
        �����Ӧ��֪��W��H���͵ı��߶����ڷ��������ͣ������ǲ���Ҫ����
        ��һ���ֵĴ���˼·��Ҫ�Լ�������ʱ��úõ�ȥ��ĥ������ע������û������˵����ġ���
        ʵ���벻ͨ�������Ұɣ�
  */

    /************************************����ȷ��������(��H��)�ı߽�*************************************/

    if (( Sideline_status_array[Ysite].IsLeftFind == 'H' || Sideline_status_array[Ysite].IsRightFind == 'H'))
    {
      /**************************��������ߵĴ�����***************************/
      if (Sideline_status_array[Ysite].IsLeftFind == 'H')
      {
        for (Xsite = (Sideline_status_array[Ysite].leftline + 1);Xsite <= (Sideline_status_array[Ysite].rightline - 1);Xsite++)                                                           //���ұ���֮������ɨ��
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
      /**************************��������ߵĴ�����***************************/


      /**************************�����ұ��ߵĴ�����***************************/
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
    /**************************�����ұ��ߵĴ�����***************************/

  /*****************************����ȷ��������ı߽�******************************/



 /************************************����ȷ���ޱ��У���W�ࣩ�ı߽�****************************************************************/
    int ysite = 0;
    uint8 L_found_point = 0;
    uint8 R_found_point = 0;
    /**************************��������ߵ��ޱ���***************************/
    if (Sideline_status_array[Ysite].IsRightFind == 'W'&&Ysite > 10&&Ysite < (image_bottom_value-9))
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
        Sideline_status_array[Ysite].rightline =Sideline_status_array[ytemp_W_R].rightline -D_R * (ytemp_W_R - Ysite);  //����ҵ��� ��ô�Ի�׼�����ӳ���
      }
      LimitL(Sideline_status_array[Ysite].rightline);  //�޷�
      LimitH(Sideline_status_array[Ysite].rightline);  //�޷�
    }
    /**************************��������ߵ��ޱ���***************************/


    /**************************�����ұ��ߵ��ޱ���***************************/
    if (Sideline_status_array[Ysite].IsLeftFind == 'W' && Ysite > 10 && Ysite < (image_bottom_value-9) )
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
        if (L_found_point > 8)              //�ҵ���׼б�ʱ�  ���ӳ�������ȷ���ޱ�
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

      LimitL(Sideline_status_array[Ysite].leftline);  //�޷�
      LimitH(Sideline_status_array[Ysite].leftline);  //�޷�
    }

    /**************************�����ұ��ߵ��ޱ���***************************/
    /************************************����ȷ���ޱ��У���W�ࣩ�ı߽�****************************************************************/


    /************************************��������֮��������һЩ������������*************************************************/
      if (Sideline_status_array[Ysite].IsLeftFind == 'W'&&Sideline_status_array[Ysite].IsRightFind == 'W')
      {
          imagestatus.WhiteLine++;  //Ҫ�����Ҷ��ޱߣ�������+1
      }
     if (Sideline_status_array[Ysite].IsLeftFind == 'W'&&Ysite<(image_bottom_value-4))
     {
          imagestatus.Miss_Left_lines++;
     }
     if (Sideline_status_array[Ysite].IsRightFind == 'W'&&Ysite<(image_bottom_value-4))
     {
          imagestatus.Miss_Right_lines++;
     }

     LimitL(Sideline_status_array[Ysite].leftline);   //�޷�
      LimitH(Sideline_status_array[Ysite].leftline);   //�޷�
     LimitL(Sideline_status_array[Ysite].rightline);  //�޷�
      LimitH(Sideline_status_array[Ysite].rightline);  //�޷�

      Sideline_status_array[Ysite].wide =Sideline_status_array[Ysite].rightline - Sideline_status_array[Ysite].leftline;
//      Sideline_status_array[Ysite].midline =(Sideline_status_array[Ysite].rightline + Sideline_status_array[Ysite].leftline) / 2;

      if (Sideline_status_array[Ysite].wide <= 7)
      {
          imagestatus.OFFLine = Ysite + 1;
          break;
      }
      else if (Sideline_status_array[Ysite].rightline <= 10||Sideline_status_array[Ysite].leftline >= (image_side_width-9))
      {
          imagestatus.OFFLine = Ysite + 1;
          break;
      }
      /************************************��������֮��������һЩ������������*************************************************/
  }
      for (Ysite = imagestatus.OFFLine ; Ysite <= (image_bottom_value-5); Ysite++){
            int ysite = 0;
            uint8 L_found_point = 0;
            uint8 R_found_point = 0;
            /*������ޱ��еı궨*/
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
                        if (L_found_point > 8)              //�ҵ���׼б�ʱ�  ���ӳ�������ȷ���ޱ�
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

                        LimitL(Sideline_status_array[Ysite].leftline);  //�޷�
                        LimitH(Sideline_status_array[Ysite].leftline);  //�޷�

            }
            /*�ұ����ޱ��еı궨*/
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
                   Sideline_status_array[Ysite].rightline =Sideline_status_array[ytemp_W_R].rightline -D_R * (ytemp_W_R - Ysite);  //����ҵ��� ��ô�Ի�׼�����ӳ���
                 }
                 LimitL(Sideline_status_array[Ysite].rightline);  //�޷�
                 LimitH(Sideline_status_array[Ysite].rightline);  //�޷�


               }
      /*���ߵı궨*/

}
}
//----------------------------------------------------------------------------------
// �������     ���ߵı궨
//----------------------------------------------------------------------------------
void draw_midline(void)
{
    for(Ysite=image_bottom_value;Ysite>imagestatus.OFFLine;Ysite--)
    {
        if(imageflag.mid_choose)
                 {
                     if(default_side_choose){Sideline_status_array[Ysite].midline =Sideline_status_array[Ysite].leftline + (track_width / 2);}
                     else {Sideline_status_array[Ysite].midline =Sideline_status_array[Ysite].rightline -(track_width / 2);}
                 }
                 else {
                     if(default_side_choose){Sideline_status_array[Ysite].midline =Sideline_status_array[Ysite].rightline -(track_width / 2);}
                     else {Sideline_status_array[Ysite].midline =Sideline_status_array[Ysite].leftline + (track_width / 2);}
            }
                 LimitH(Sideline_status_array[Ysite].midline);
                 LimitL(Sideline_status_array[Ysite].midline);
    }
}




//void Get_AllLine(void)
//{
//  uint8 L_Found_T  = 'F';    //ȷ���ޱ�б�ʵĻ�׼�б����Ƿ��ҵ��ı�־
//  uint8 Get_L_line = 'F';    //�ҵ���һ֡ͼ��Ļ�׼��б�ʣ�Ϊʲô����Ҫ��ΪF����������Ĵ����֪���ˡ�
//  uint8 R_Found_T  = 'F';    //ȷ���ޱ�б�ʵĻ�׼�б����Ƿ��ҵ��ı�־
//  uint8 Get_R_line = 'F';    //�ҵ���һ֡ͼ��Ļ�׼��б�ʣ�Ϊʲô����Ҫ��ΪF����������Ĵ����֪���ˡ�
//  float D_L = 0;             //������ӳ��ߵ�б��
//  float D_R = 0;             //�ұ����ӳ��ߵ�б��
//  int ytemp_W_L;             //��ס�״��󶪱���
//  int ytemp_W_R;             //��ס�״��Ҷ�����
//  ExtenRFlag = 0;            //��־λ��0
//  ExtenLFlag = 0;            //��־λ��0
//  imagestatus.OFFLine=2;     //����ṹ���Ա��֮���������︳ֵ������Ϊ��imagestatus�ṹ������ĳ�Ա̫���ˣ�������ʱ��ֻ�õ���OFFLine�������������õ��������ĸ�ֵ��
//  imagestatus.Miss_Right_lines = 0;
//  imagestatus.WhiteLine = 0;
//  imagestatus.Miss_Left_lines = 0;
//  imageflag.mid_choose=0;
//  uint8 mid_choose_value=0;
//  for (Ysite = (image_bottom_value-5) ; Ysite > imagestatus.OFFLine; Ysite--)                            //ǰ5����Get_BaseLine()���Ѿ�������ˣ����ڴ�55�д����Լ��趨�Ĳ�������OFFLine��
//  {                                                                                  //��Ϊ̫ǰ���ͼ��ɿ��Բ��㣬����OFFLine�����ú��б�Ҫ��û��Ҫһֱ����ɨ����0�С�
//    PicTemp = image[Ysite];
//    JumpPointtypedef JumpPoint[2];                                                   // JumpPoint[0]��������ߣ�JumpPoint[1]�����ұ��ߡ�
//
//  /******************************ɨ�豾�е��ұ���******************************/
//    int IntervalLow  = Sideline_status_array[Ysite + 1].rightline  - ImageScanInterval;               //����һ�е��ұ��߼Ӽ�Interval��Ӧ���п�ʼɨ�豾�У�Intervalһ��ȡ5����Ȼ��Ϊ�˱���������԰����ֵ�ĵĴ�һ�㡣
//    int IntervalHigh = Sideline_status_array[Ysite + 1].rightline + ImageScanInterval;              //���������ֻ��Ҫ�����б�������5�Ļ����ϣ����10�е�������䣩ȥɨ�ߣ�һ������ҵ����еı����ˣ��������ֵ��ʵ����̫��
//    IntervalLow= (IntervalLow);                                                             //������ǶԴ���GetJumpPointFromDet()������ɨ���������һ���޷�������
//    IntervalHigh=LimitL(IntervalHigh);                                                            //������һ�еı����ǵ�2�У�����2-5=-3��-3�ǲ��Ǿ�û��ʵ�������ˣ���ô����-3���أ�
//    Get_Border_And_SideType(PicTemp, 'R', IntervalLow, IntervalHigh,&JumpPoint[1]);  //ɨ���õ�һ���Ӻ������Լ�����ȥ�������߼���
//  /******************************ɨ�豾�е��ұ���******************************/
//
//  /******************************ɨ�豾�е������******************************/
//    IntervalLow =Sideline_status_array[Ysite + 1].leftline  -ImageScanInterval;                //����һ�е�����߼Ӽ�Interval��Ӧ���п�ʼɨ�豾�У�Intervalһ��ȡ5����Ȼ��Ϊ�˱���������԰����ֵ�ĵĴ�һ�㡣
//    IntervalHigh =Sideline_status_array[Ysite + 1].leftline +ImageScanInterval;                //���������ֻ��Ҫ�����б�������5�Ļ����ϣ����10�е�������䣩ȥɨ�ߣ�һ������ҵ����еı����ˣ��������ֵ��ʵ����̫��
//    IntervalLow=LimitL(IntervalLow);                                                             //������ǶԴ���GetJumpPointFromDet()������ɨ���������һ���޷�������
//    IntervalHigh= LimitL(IntervalHigh);                                                            //������һ�еı����ǵ�2�У�����2-5=-3��-3�ǲ��Ǿ�û��ʵ�������ˣ���ô����-3���أ�
//    Get_Border_And_SideType(PicTemp, 'L', IntervalLow, IntervalHigh,&JumpPoint[0]);  //ɨ���õ�һ���Ӻ������Լ�����ȥ�������߼���
//  /******************************ɨ�豾�е������******************************/
//
//  /*
//       ����Ĵ���Ҫ���뿴�����������ҵ����ڸ���ħ�Ļ���
//        ����ذ�GetJumpPointFromDet()����������߼�������
//        ������������������棬��T������W������H��������־����ʲô��
//        һ��Ҫ�㶮!!!��Ȼ�Ļ������鲻Ҫ���¿��ˣ���Ҫ��ĥ�Լ�!!!
//  */
//
//    if(default_side_choose)
//    {
//        if(JumpPoint[1].type!= 'T')mid_choose_value++;
//    }
//    else {
//        if(JumpPoint[0].type != 'T')mid_choose_value++;
//    }
//    if (JumpPoint[0].type =='W')                                                     //������е���������ڲ��������䣬����10���㶼�ǰ׵㡣
//    {
//      Sideline_status_array[Ysite].leftline =Sideline_status_array[Ysite + 1].leftline;                  //��ô���е�����߾Ͳ�����һ�еı��ߡ�
//    }
//    else                                                                             //������е����������T������H���
//    {
//      Sideline_status_array[Ysite].leftline = JumpPoint[0].point;                              //��ôɨ�赽�ı����Ƕ��٣��Ҿͼ�¼�����Ƕ��١�
//    }
//
//    if (JumpPoint[1].type == 'W')                                                    //������е��ұ������ڲ��������䣬����10���㶼�ǰ׵㡣
//    {
//      Sideline_status_array[Ysite].rightline =Sideline_status_array[Ysite + 1].rightline;                //��ô���е��ұ��߾Ͳ�����һ�еı��ߡ�
//    }
//    else                                                                             //������е��ұ�������T������H���
//    {
//      Sideline_status_array[Ysite].rightline = JumpPoint[1].point;                             //��ôɨ�赽�ı����Ƕ��٣��Ҿͼ�¼�����Ƕ��١�
//    }
//    if(mid_choose_value>10)imageflag.mid_choose=1;
////    if(Sideline_status_array[Ysite].wide< track_width)imageflag.mid_choose=1;
//    Sideline_status_array[Ysite].IsLeftFind =JumpPoint[0].type;                                  //��¼�����ҵ�����������ͣ���T����W������H��������ͺ��������õģ���Ϊ�һ�Ҫ��һ������
//    Sideline_status_array[Ysite].IsRightFind = JumpPoint[1].type;                                //��¼�����ҵ����ұ������ͣ���T����W������H��������ͺ��������õģ���Ϊ�һ�Ҫ��һ������
//
//
//  /*
//        ����Ϳ�ʼ��W��H���͵ı��߷ֱ���д��� ΪʲôҪ����
//        ����㿴����GetJumpPointFromDet�����߼���������T W H�������ͷֱ��Ӧʲô�����
//        �����Ӧ��֪��W��H���͵ı��߶����ڷ��������ͣ������ǲ���Ҫ����
//        ��һ���ֵĴ���˼·��Ҫ�Լ�������ʱ��úõ�ȥ��ĥ������ע������û������˵����ġ���
//        ʵ���벻ͨ�������Ұɣ�
//  */
//
//    /************************************����ȷ��������(��H��)�ı߽�*************************************/
//
//    if (( Sideline_status_array[Ysite].IsLeftFind == 'H' || Sideline_status_array[Ysite].IsRightFind == 'H'))
//    {
//      /**************************��������ߵĴ�����***************************/
//      if (Sideline_status_array[Ysite].IsLeftFind == 'H')
//      {
//        for (Xsite = (Sideline_status_array[Ysite].leftline + 1);Xsite <= (Sideline_status_array[Ysite].rightline - 1);Xsite++)                                                           //���ұ���֮������ɨ��
//        {
//          if ((*(PicTemp + Xsite) == 0) && (*(PicTemp + Xsite + 1) != 0))
//          {
//            Sideline_status_array[Ysite].leftline =Xsite;
//            Sideline_status_array[Ysite].IsLeftFind = 'T';
//            break;
//          }
//          else if (*(PicTemp + Xsite) != 0)
//            break;
//          else if (Xsite ==(Sideline_status_array[Ysite].rightline - 1))
//          {
//            Sideline_status_array[Ysite].IsLeftFind = 'T';
//            break;
//          }
//        }
//      }
//      /**************************��������ߵĴ�����***************************/
//
//
//      /**************************�����ұ��ߵĴ�����***************************/
//      if (Sideline_status_array[Ysite].IsRightFind == 'H')
//      {
//        for (Xsite = (Sideline_status_array[Ysite].rightline - 1);Xsite >= (Sideline_status_array[Ysite].leftline + 1); Xsite--)
//        {
//          if ((*(PicTemp + Xsite) == 0) && (*(PicTemp + Xsite - 1) != 0))
//          {
//            Sideline_status_array[Ysite].rightline =Xsite;
//            Sideline_status_array[Ysite].IsRightFind = 'T';
//            break;
//          }
//          else if (*(PicTemp + Xsite) != 0)
//            break;
//          else if (Xsite == (Sideline_status_array[Ysite].leftline + 1))
//          {
//            Sideline_status_array[Ysite].rightline = Xsite;
//            Sideline_status_array[Ysite].IsRightFind = 'T';
//            break;
//          }
//         }
//       }
//     }
//    /**************************�����ұ��ߵĴ�����***************************/
//
//  /*****************************����ȷ��������ı߽�******************************/
//
//
//
// /************************************����ȷ���ޱ��У���W�ࣩ�ı߽�****************************************************************/
//    int ysite = 0;
//    uint8 L_found_point = 0;
//    uint8 R_found_point = 0;
//    /**************************�����ұ��ߵ��ޱ���***************************/
//    if (Sideline_status_array[Ysite].IsRightFind == 'W' && Ysite > 10 && Ysite < (image_side_width-10))
//    {
//      if (Get_R_line == 'F')
//      {
//        Get_R_line = 'T';
//        ytemp_W_R = Ysite + 2;
//        for (ysite = Ysite + 1; ysite < Ysite + 15; ysite++)
//        {
//          if (Sideline_status_array[ysite].IsRightFind =='T')
//          {
//              R_found_point++;
//          }
//        }
//        if (R_found_point >8)
//        {
//          D_R = ((float)(Sideline_status_array[Ysite + R_found_point].rightline - Sideline_status_array[Ysite + 3].rightline)) /((float)(R_found_point - 3));
//          if (D_R > 0)
//          {
//            R_Found_T ='T';
//          }
//          else
//          {
//            R_Found_T = 'F';
////            if (D_R < 0)
////            {
////                ExtenRFlag = 'F';
////            }
//          }
//        }
//      }
//      if (R_Found_T == 'T')
//      {
//        Sideline_status_array[Ysite].rightline =Sideline_status_array[ytemp_W_R].rightline -D_R * (ytemp_W_R - Ysite);  //����ҵ��� ��ô�Ի�׼�����ӳ���
//      }
//      Sideline_status_array[Ysite].rightline=LimitL(Sideline_status_array[Ysite].rightline);  //�޷�
//
//    }
//
//    /**************************�����ұ��ߵ��ޱ���***************************/
//
//
//    /**************************��������ߵ��ޱ���***************************/
//    if (Sideline_status_array[Ysite].IsLeftFind == 'W' && Ysite > 10 && Ysite < (image_side_width-10) )
//    {
//      if (Get_L_line == 'F')
//      {
//        Get_L_line = 'T';
//        ytemp_W_L = Ysite + 2;
//        for (ysite = Ysite + 1; ysite < Ysite + 15; ysite++)
//        {
//          if (Sideline_status_array[ysite].IsLeftFind == 'T')
//            {
//              L_found_point++;
//            }
//        }
//        if (L_found_point > 8)              //�ҵ���׼б�ʱ�  ���ӳ�������ȷ���ޱ�
//        {
//          D_L = ((float)(Sideline_status_array[Ysite + 3].leftline -Sideline_status_array[Ysite + L_found_point].leftline)) /((float)(L_found_point - 3));
//          if (D_L > 0)
//          {
//            L_Found_T = 'T';
//          }
//          else
//          {
//            L_Found_T = 'F';
////            if (D_L < 0)
////            {
////                ExtenLFlag = 'F';
////            }
//          }
//        }
//      }
//
//      if (L_Found_T == 'T')
//      {
//          Sideline_status_array[Ysite].leftline =Sideline_status_array[ytemp_W_L].leftline + D_L * (ytemp_W_L - Ysite);
//      }
//
//      Sideline_status_array[Ysite].leftline=LimitL(Sideline_status_array[Ysite].leftline);  //�޷�
//
//    }
//
//    /**************************��������ߵ��ޱ���***************************/
//    /************************************����ȷ���ޱ��У���W�ࣩ�ı߽�****************************************************************/
//
//
//    /************************************��������֮��������һЩ������������*************************************************/
//      if (Sideline_status_array[Ysite].IsLeftFind == 'W'&&Sideline_status_array[Ysite].IsRightFind == 'W')
//      {
//          imagestatus.WhiteLine++;  //Ҫ�����Ҷ��ޱߣ�������+1
//      }
//     if (Sideline_status_array[Ysite].IsLeftFind == 'W' && Ysite < (image_bottom_value-4) )
//     {
//          imagestatus.Miss_Left_lines++;
//     }
//     if (Sideline_status_array[Ysite].IsRightFind == 'W'&& Ysite < (image_bottom_value-4) )
//     {
//          imagestatus.Miss_Right_lines++;
//     }
//
//     Sideline_status_array[Ysite].leftline=LimitL(Sideline_status_array[Ysite].leftline);   //�޷�
//
//     Sideline_status_array[Ysite].rightline= LimitL(Sideline_status_array[Ysite].rightline);  //�޷�
//
//
//      Sideline_status_array[Ysite].wide =Sideline_status_array[Ysite].rightline - Sideline_status_array[Ysite].leftline;
////      Sideline_status_array[Ysite].midline =(Sideline_status_array[Ysite].rightline + Sideline_status_array[Ysite].leftline) / 2;
//
////      if (Sideline_status_array[Ysite].wide <= 7 )
////      {
////          imagestatus.OFFLine = Ysite + 1;
////          break;
////      }
////      else if (Sideline_status_array[Ysite].rightline <= 10||Sideline_status_array[Ysite].leftline >= (image_bottom_value-10))
////      {
////          imagestatus.OFFLine = Ysite + 1;
////          break;
////      }
//      /************************************��������֮��������һЩ������������*************************************************/
//  }
//  for (Ysite = imagestatus.OFFLine ; Ysite <= (image_bottom_value-5); Ysite++){
//      int ysite = 0;
//      uint8 L_found_point = 0;
//      uint8 R_found_point = 0;
//      /*������ޱ��еı궨*/
//      if (Sideline_status_array[Ysite].IsLeftFind == 'W' && Ysite < 10)
//      {
//          ytemp_W_L = Ysite - 2;
//                  for (ysite = Ysite - 1; ysite < Ysite - 15; ysite--)
//                  {
//                    if (Sideline_status_array[ysite].IsLeftFind == 'T')
//                      {
//                        L_found_point++;
//                      }
//                  }
//                  if (L_found_point > 8)              //�ҵ���׼б�ʱ�  ���ӳ�������ȷ���ޱ�
//                  {
//                    D_L = ((float)(Sideline_status_array[Ysite - 3].leftline -Sideline_status_array[Ysite - L_found_point].leftline)) /((float)(L_found_point - 3));
//                    if (D_L > 0)
//                    {
//                      L_Found_T = 'T';
//                    }
//                    else
//                    {
//                      L_Found_T = 'F';
//          //            if (D_L < 0)
//          //            {
//          //                ExtenLFlag = 'F';
//          //            }
//                    }
//                  }
//                  if (L_Found_T == 'T')
//                       {
//                           Sideline_status_array[Ysite].leftline =Sideline_status_array[ytemp_W_L].leftline + D_L * (ytemp_W_L - Ysite);
//                       }
//
//                  Sideline_status_array[Ysite].leftline=LimitL(Sideline_status_array[Ysite].leftline);  //�޷�
//
//      }
//      /*�ұ����ޱ��еı궨*/
//      if (Sideline_status_array[Ysite].IsRightFind == 'W' &&  Ysite > (image_side_width-10))
//         {
//             ytemp_W_R = Ysite - 2;
//             for (ysite = Ysite - 1; ysite < Ysite - 15; ysite--)
//             {
//               if (Sideline_status_array[ysite].IsRightFind =='T')
//               {
//                   R_found_point++;
//               }
//             }
//             if (R_found_point >8)
//             {
//               D_R = ((float)(Sideline_status_array[Ysite - R_found_point].rightline - Sideline_status_array[Ysite - 3].rightline)) /((float)(R_found_point - 3));
//               if (D_R > 0)
//               {
//                 R_Found_T ='T';
//               }
//               else
//               {
//                 R_Found_T = 'F';
//     //            if (D_R < 0)
//     //            {
//     //                ExtenRFlag = 'F';
//     //            }
//               }
//             }
//
//           if (R_Found_T == 'T')
//           {
//             Sideline_status_array[Ysite].rightline =Sideline_status_array[ytemp_W_R].rightline -D_R * (ytemp_W_R - Ysite);  //����ҵ��� ��ô�Ի�׼�����ӳ���
//           }
//           Sideline_status_array[Ysite].rightline=LimitL(Sideline_status_array[Ysite].rightline);  //�޷�
//
//         }
//      /*���ߵı궨*/
//      if(imageflag.mid_choose)
//      {
//          if(default_side_choose){Sideline_status_array[Ysite].midline =Sideline_status_array[Ysite].leftline + (track_width / 2);}
//          else {Sideline_status_array[Ysite].midline =Sideline_status_array[Ysite].rightline -(track_width / 2);}
//      }
//      else {
//          if(default_side_choose){Sideline_status_array[Ysite].midline =Sideline_status_array[Ysite].rightline -(track_width / 2);}
//          else {Sideline_status_array[Ysite].midline =Sideline_status_array[Ysite].leftline + (track_width / 2);}
//    }
//  }
//}



/*�Ͻ��������ַ���ɨ�ߣ���Ϊ����Բ�����ж�Ԫ�صĵڶ�����*/
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//  @name           Search_Bottom_Line_OTSU
//  @brief          ��ȡ�ײ����ұ���
//  @param          imageInput[IMAGE_ROW][IMAGE_COL]        �����ͼ������
//  @param          row                                     ͼ���Ysite
//  @param          col                                     ͼ���Xsite
//  @return         Bottonline                              �ױ���ѡ��
//  @time           2022��10��9��
//  @Author         ������
//  Sample usage:   Search_Bottom_Line_OTSU(imageInput, row, col, Bottonline);
//--------------------------------------------------------------------------------------------------------------------------------------------

void Search_Bottom_Line_OTSU(uint8 imageInput[image_h][image_w], uint8 row, uint8 col, uint8 Bottonline)
{

    //Ѱ����߽߱�
    for (int Xsite = col / 2-2; Xsite > 1; Xsite--)
    {
        if (imageInput[Bottonline][Xsite] != 0 && imageInput[Bottonline][Xsite - 1] == 0)
        {
            Sideline_status_array[Bottonline].LeftBoundary = Xsite;//��ȡ�ױ������
            break;
        }
    }
    for (int Xsite = col / 2+2; Xsite < image_side_width; Xsite++)
    {
        if (imageInput[Bottonline][Xsite] != 0 && imageInput[Bottonline][Xsite + 1] == 0)
        {
            Sideline_status_array[Bottonline].RightBoundary = Xsite;//��ȡ�ױ��ұ���
            break;
        }
    }


}
//void Search_Bottom_Line_OTSU(uint8 imageInput[LCDH][LCDW], uint8 row, uint8 col, uint8 Bottonline)
//{
//
//    //Ѱ����߽߱�
//    for (int Xsite = col / 2-2; Xsite > 1; Xsite--)
//    {
//        if (imageInput[Bottonline][Xsite] == 1 && imageInput[Bottonline][Xsite - 1] == 0)
//        {
//            Sideline_status_array[Bottonline].LeftBoundary = Xsite;//��ȡ�ױ������
//            break;
//        }
//    }
//    for (int Xsite = col / 2+2; Xsite < LCDW-1; Xsite++)
//    {
//        if (imageInput[Bottonline][Xsite] == 1 && imageInput[Bottonline][Xsite + 1] == 0)
//        {
//            Sideline_status_array[Bottonline].RightBoundary = Xsite;//��ȡ�ױ��ұ���
//            break;
//        }
//    }


//}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//  @name           Search_Left_and_Right_Lines
//  @brief          ͨ��sobel��ȡ���ұ���
//  @param          imageInput[IMAGE_ROW][IMAGE_COL]        �����ͼ������
//  @param          row                                     ͼ���Ysite
//  @param          col                                     ͼ���Xsite
//  @param          Bottonline                              �ױ���ѡ��
//  @return         ��
//  @time           2022��10��7��
//  @Author         ������
//  Sample usage:   Search_Left_and_Right_Lines(imageInput, row, col, Bottonline);
//--------------------------------------------------------------------------------------------------------------------------------------------

void Search_Left_and_Right_Lines(uint8 imageInput[image_h][image_w], uint8 row, uint8 col, uint8 Bottonline)
{
    //����С�˵ĵ�ǰ����״̬λ��Ϊ �� �� �� �� һ��Ҫ�� �ϣ����Ϊ��ɫ ���ϱ�Ϊ��ɫ �£��ұ�Ϊɫ  �ң������к�ɫ
/*  ǰ�������壺
                *   0
                * 3   1
                *   2
*/
/*Ѱ�����������*/
    uint8 Left_Rule[2][8] = {
                                  {0,-1,1,0,0,1,-1,0 },//{0,-1},{1,0},{0,1},{-1,0},  (x,y )
                                  {-1,-1,1,-1,1,1,-1,1} //{-1,-1},{1,-1},{1,1},{-1,1}
    };
    /*Ѱ�����������*/
    int Right_Rule[2][8] = {
                              {0,-1,1,0,0,1,-1,0 },//{0,-1},{1,0},{0,1},{-1,0},
                              {1,-1,1,1,-1,1,-1,-1} //{1,-1},{1,1},{-1,1},{-1,-1}
    };
      int num=0;
    uint8 Left_Ysite = Bottonline;
    uint8 Left_Xsite = Sideline_status_array[Bottonline].LeftBoundary;
    uint8 Left_Rirection = 0;//��߷���
    uint8 Pixel_Left_Ysite = Bottonline;
    uint8 Pixel_Left_Xsite = 0;

    uint8 Right_Ysite = Bottonline;
    uint8 Right_Xsite = Sideline_status_array[Bottonline].RightBoundary;
    uint8 Right_Rirection = 0;//�ұ߷���
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
        /*********���Ѳ��*******/
        if ((Pixel_Left_Ysite > Ysite) || Ysite == imagestatus.OFFLineBoundary)//�ұ�ɨ��
        {
            /*����ǰ������*/
            Pixel_Left_Ysite = Left_Ysite + Left_Rule[0][2 * Left_Rirection + 1];
            Pixel_Left_Xsite = Left_Xsite + Left_Rule[0][2 * Left_Rirection];

            if (imageInput[Pixel_Left_Ysite][Pixel_Left_Xsite] == 0)//ǰ���Ǻ�ɫ
            {
                //˳ʱ����ת90
                if (Left_Rirection == 3)
                    Left_Rirection = 0;
                else
                    Left_Rirection++;
            }
            else//ǰ���ǰ�ɫ
            {
                /*������ǰ������*/
                Pixel_Left_Ysite = Left_Ysite + Left_Rule[1][2 * Left_Rirection + 1];
                Pixel_Left_Xsite = Left_Xsite + Left_Rule[1][2 * Left_Rirection];

                if (imageInput[Pixel_Left_Ysite][Pixel_Left_Xsite] == 0)//��ǰ��Ϊ��ɫ
                {
                    //���򲻱�  Left_Rirection
                    Left_Ysite = Left_Ysite + Left_Rule[0][2 * Left_Rirection + 1];
                    Left_Xsite = Left_Xsite + Left_Rule[0][2 * Left_Rirection];
                    if (Sideline_status_array[Left_Ysite].LeftBoundary_First == 0)
                        Sideline_status_array[Left_Ysite].LeftBoundary_First = Left_Xsite;
                    Sideline_status_array[Left_Ysite].LeftBoundary = Left_Xsite;
                }
                else//��ǰ��Ϊ��ɫ
                {
                    // �������ı� Left_Rirection  ��ʱ��90��
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
        /*********�ұ�Ѳ��*******/
        if ((Pixel_Right_Ysite > Ysite) || Ysite == imagestatus.OFFLineBoundary)//�ұ�ɨ��
        {
            /*����ǰ������*/
            Pixel_Right_Ysite = Right_Ysite + Right_Rule[0][2 * Right_Rirection + 1];
            Pixel_Right_Xsite = Right_Xsite + Right_Rule[0][2 * Right_Rirection];

            if (imageInput[Pixel_Right_Ysite][Pixel_Right_Xsite] == 0)//ǰ���Ǻ�ɫ
            {
                //��ʱ����ת90
                if (Right_Rirection == 0)
                    Right_Rirection = 3;
                else
                    Right_Rirection--;
            }
            else//ǰ���ǰ�ɫ
            {
                /*������ǰ������*/
                Pixel_Right_Ysite = Right_Ysite + Right_Rule[1][2 * Right_Rirection + 1];
                Pixel_Right_Xsite = Right_Xsite + Right_Rule[1][2 * Right_Rirection];

                if (imageInput[Pixel_Right_Ysite][Pixel_Right_Xsite] == 0)//��ǰ��Ϊ��ɫ
                {
                    //���򲻱�  Right_Rirection
                    Right_Ysite = Right_Ysite + Right_Rule[0][2 * Right_Rirection + 1];
                    Right_Xsite = Right_Xsite + Right_Rule[0][2 * Right_Rirection];
                    if (Sideline_status_array[Right_Ysite].RightBoundary_First == image_side_width )
                        Sideline_status_array[Right_Ysite].RightBoundary_First = Right_Xsite;
                    Sideline_status_array[Right_Ysite].RightBoundary = Right_Xsite;
                }
                else//��ǰ��Ϊ��ɫ
                {
                    // �������ı� Right_Rirection  ��ʱ��90��
                    Right_Ysite = Right_Ysite + Right_Rule[1][2 * Right_Rirection + 1];
                    Right_Xsite = Right_Xsite + Right_Rule[1][2 * Right_Rirection];
                    if (Sideline_status_array[Right_Ysite].RightBoundary_First == image_side_width)
                        Sideline_status_array[Right_Ysite].RightBoundary_First = Right_Xsite;
                    Sideline_status_array[Right_Ysite].RightBoundary = Right_Xsite;
                    if (Right_Rirection == 3)
                        Right_Rirection = 0;
                    else
                        Right_Rirection++;
                }

            }
        }

        if (abs(Pixel_Right_Xsite - Pixel_Left_Xsite) < 3)//Ysite<80��Ϊ�˷��ڵײ��ǰ�����ɨ�����  3 && Ysite < 30
        {

            imagestatus.OFFLineBoundary = Ysite;
            break;
        }

    }
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//  @name           Search_Border_OTSU
//  @brief          ͨ��OTSU��ȡ���� ����Ϣ
//  @param          imageInput[IMAGE_ROW][IMAGE_COL]        �����ͼ������
//  @param          row                                     ͼ���Ysite
//  @param          col                                     ͼ���Xsite
//  @param          Bottonline                              �ױ���ѡ��
//  @return         ��
//  @time           2022��10��7��
//  @Author         ������
//  Sample usage:   Search_Border_OTSU(mt9v03x_image, IMAGE_ROW, IMAGE_COL, IMAGE_ROW-8);
//--------------------------------------------------------------------------------------------------------------------------------------------

void Search_Border_OTSU(uint8 imageInput[image_h][image_w], uint8 row, uint8 col, uint8 Bottonline)
{
    imagestatus.WhiteLine_L = 0;
    imagestatus.WhiteLine_R = 0;
    //imagestatus.OFFLine = 1;
    /*�����±߽紦��*/
//    for (int Xsite = 0; Xsite < LCDW; Xsite++)
//    {
//        imageInput[0][Xsite] = 0;
//        imageInput[Bottonline + 1][Xsite] = 0;
//    }
    /*�����ұ߽紦��*/
    for (int Ysite = 0; Ysite < image_bottom_value; Ysite++)
    {
            Sideline_status_array[Ysite].LeftBoundary_First = 0;
            Sideline_status_array[Ysite].RightBoundary_First = image_side_width;

//            imageInput[Ysite][0] = 0;
//            imageInput[Ysite][LCDW - 1] = 0;
    }
    /********��ȡ�ײ�����*********/
    Search_Bottom_Line_OTSU(imageInput, row, col, Bottonline);
    /********��ȡ���ұ���*********/
    Search_Left_and_Right_Lines(imageInput, row, col, Bottonline);



    for (int Ysite = Bottonline; Ysite > imagestatus.OFFLineBoundary + 1; Ysite--)
    {
        if (Sideline_status_array[Ysite].LeftBoundary < 3)
        {
            imagestatus.WhiteLine_L++;
        }
        if (Sideline_status_array[Ysite].RightBoundary > image_w - 3)
        {
            imagestatus.WhiteLine_R++;
        }
    }
}

//--------------------------------------------------------------
//  @name           Element_Judgment_Left_Rings()
//  @brief          ����ͼ���жϵ��Ӻ����������ж���Բ������.
//  @parameter      void
//  @time
//  Sample usage:   Element_Judgment_Left_Rings();
//--------------------------------------------------------------
void Element_Judgment_Left_Rings()
{
    if (   imagestatus.Miss_Right_lines > 5 || imagestatus.Miss_Left_lines < 10
        || imagestatus.OFFLine > 20
        || imageflag.image_element_rings !=0
//        || imageflag.Out_Road == 1 || imageflag.RoadBlock_Flag == 1
        || Sideline_status_array[(image_bottom_value-7)].IsLeftFind == 'W'
        || Sideline_status_array[(image_bottom_value-6)].IsLeftFind == 'W'
        || Sideline_status_array[(image_bottom_value-5)].IsLeftFind == 'W'
        || Sideline_status_array[(image_bottom_value-4)].IsLeftFind == 'W'
        || Sideline_status_array[(image_bottom_value-3)].IsLeftFind == 'W'
        || Sideline_status_array[(image_bottom_value-2)].IsLeftFind == 'W'
        || Sideline_status_array[(image_bottom_value-1)].IsLeftFind == 'W')
        return;
    int ring_ysite = 25;
    uint8 Left_Less_Num = 0;
    Left_RingsFlag_Point1_Ysite = 0;
    Left_RingsFlag_Point2_Ysite = 0;
    for (int Ysite = (image_bottom_value-1); Ysite > ring_ysite; Ysite--)
    {
        if (Sideline_status_array[Ysite].LeftBoundary_First - Sideline_status_array[Ysite - 1].LeftBoundary_First > 4)
        {
            Left_RingsFlag_Point1_Ysite = Ysite;
            break;
        }
    }
    for (int Ysite = (image_bottom_value-1); Ysite > ring_ysite; Ysite--)
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
//  @brief          ����ͼ���жϵ��Ӻ����������ж���Բ������.
//  @parameter      void
//  @time
//  @Author         MRCHEN
//  Sample usage:   Element_Judgment_Right_Rings();
//--------------------------------------------------------------
void Element_Judgment_Right_Rings()
{
    if (   imagestatus.Miss_Left_lines > 5 || imagestatus.Miss_Right_lines < 10
        || imagestatus.OFFLine > 20
        || imageflag.image_element_rings != 0 || imageflag.Out_Road == 1 || imageflag.RoadBlock_Flag == 1
        || Sideline_status_array[(image_bottom_value-7)].IsRightFind == 'W'
        || Sideline_status_array[(image_bottom_value-6)].IsRightFind == 'W'
        || Sideline_status_array[(image_bottom_value-5)].IsRightFind == 'W'
        || Sideline_status_array[(image_bottom_value-4)].IsRightFind == 'W'
        || Sideline_status_array[(image_bottom_value-3)].IsRightFind == 'W'
        || Sideline_status_array[(image_bottom_value-2)].IsRightFind == 'W'
        || Sideline_status_array[(image_bottom_value-1)].IsRightFind == 'W')
        return;

    int ring_ysite = 25;
    uint8 Right_Less_Num = 0;
    Right_RingsFlag_Point1_Ysite = 0;
    Right_RingsFlag_Point2_Ysite = 0;
    for (int Ysite = (image_bottom_value-1); Ysite > ring_ysite; Ysite--)
    {
        if (Sideline_status_array[Ysite - 2].RightBoundary_First - Sideline_status_array[Ysite].RightBoundary_First > 4)
        {
            Right_RingsFlag_Point1_Ysite = Ysite;
            break;
        }
    }
    for (int Ysite = (image_bottom_value-1); Ysite > ring_ysite; Ysite--)
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

//--------------------------------------------------------------------
// �������     ��Բ������
//--------------------------------------------------------------------
void Element_Handle_Left_Rings()
{

 if(imageflag.ring_big_small !=0 && imageflag.image_element_rings==1)
 {
     uint8 num=0,ypoint=0;
     /*����״̬�ж�*/
     if(imageflag.image_element_rings_flag==3)
     {
         for(uint8 i=(image_bottom_value-2);i>imagestatus.OFFLine;i--)
         {
             if((Sideline_status_array[i-1].IsLeftFind=='W'||Sideline_status_array[i-1].leftline<=10)&& (Sideline_status_array[i-2].IsLeftFind=='W'||Sideline_status_array[i-2].leftline)
                     && (Sideline_status_array[i+1].IsLeftFind=='T'&& Sideline_status_array[i].leftline>=10)&&(Sideline_status_array[i+2].IsRightFind=='T'&&Sideline_status_array[i].leftline>=10))
             {
                 ypoint=i;
                 break;
             }
             if(imageflag.image_element_rings_flag==7 || imageflag.image_element_rings_flag==8 ||  imageflag.image_element_rings_flag==9)
             {
                 if(Sideline_status_array[i].midline-Sideline_status_array[i+2].midline>5)num++;
             }
//             for (Ysite = image_bottom_value; Ysite > imagestatus.OFFLine + 3; Ysite--)
//                    {
//                        if (    Sideline_status_array[Ysite].leftline >= Sideline_status_array[Ysite + 2].leftline
//                             && Sideline_status_array[Ysite].leftline >= Sideline_status_array[Ysite - 2].leftline
//                             && Sideline_status_array[Ysite].leftline >= Sideline_status_array[Ysite + 1].leftline
//                             && Sideline_status_array[Ysite].leftline >= Sideline_status_array[Ysite - 1].leftline
//                             && Sideline_status_array[Ysite].leftline >= Sideline_status_array[Ysite + 4].leftline
//                             && Sideline_status_array[Ysite].leftline >= Sideline_status_array[Ysite - 4].leftline
//                           )
//                        {
////                            Point_Xsite = ImageDeal[Ysite].RightBorder;
////                            Point_Ysite = Ysite;
//                            ypoint=Ysite;
//                            break;
//                        }
//                    }
         }
     }
     if(imageflag.image_element_rings_flag==1 && imagestatus.Miss_Left_lines>=15)imageflag.image_element_rings_flag=2;
     if(imageflag.image_element_rings_flag==2 && imagestatus.Miss_Left_lines<=5)imageflag.image_element_rings_flag=3;//��ʱ�����뻷
     if(imageflag.image_element_rings_flag==3 && imagestatus.Miss_Left_lines>=15 && ypoint>60)imageflag.image_element_rings_flag=4;
     if(imageflag.image_element_rings_flag==4 && imagestatus.Miss_Right_lines>=15)imageflag.image_element_rings_flag=5;
//     if(imageflag.image_element_rings_flag==3 && imagestatus.Miss_Right_lines < 5 )imageflag.image_element_rings_flag=6;
     if(imageflag.image_element_rings_flag==5 && imagestatus.Miss_Right_lines < 5 )imageflag.image_element_rings_flag=6;
     if(imageflag.image_element_rings_flag==6 && imagestatus.Miss_Right_lines>=10 && imagestatus.Miss_Left_lines>=10 && imagestatus.OFFLine<15)imageflag.image_element_rings_flag=7;
     if(imageflag.image_element_rings_flag==7 && imagestatus.OFFLine >15 )imageflag.image_element_rings_flag=8;
     if((imageflag.image_element_rings_flag==7 || imageflag.image_element_rings_flag==8) && num>10)imageflag.image_element_rings_flag=9;//�����ͷ�򷴷���ƫ�ƹ���
     if(imageflag.image_element_rings_flag==9 && imagestatus.OFFLine>=25 && num<5)imageflag.image_element_rings_flag=8;
     if((imageflag.image_element_rings_flag==9 || imageflag.image_element_rings_flag==8) && imagestatus.Miss_Right_lines<5 && Straight_Judge(2,(imagestatus.OFFLine+10),70)<1 && abs(angle_compute(Sideline_status_array[(imagestatus.OFFLine+10)].rightline,(imagestatus.OFFLine+10),Sideline_status_array[70].rightline,70))<45 )imageflag.image_element_rings_flag=10;
     if(imageflag.image_element_rings_flag==10 && imagestatus.Miss_Left_lines<5)
     {
         imageflag.image_element_rings=0;
         imageflag.image_element_rings_flag=0;
         imageflag.ring_big_small=0;
     }
     /*������*/
     if(imageflag.image_element_rings_flag==4 || imageflag.image_element_rings_flag==5 )
     {
         int  flag_Xsite_1=0;
                int flag_Ysite_1=0;
                float Slope_Rings=0;
                for(Ysite=(image_bottom_value-4);Ysite>imagestatus.OFFLine;Ysite--)//���满��
                {
                    for(Xsite=Sideline_status_array[Ysite].leftline + 1;Xsite<Sideline_status_array[Ysite].rightline - 1;Xsite++)
                    {
                        if(  image[Ysite][Xsite] != 0 && image[Ysite][Xsite + 1] == 0)
                         {
                           flag_Ysite_1 = Ysite;
                           flag_Xsite_1 = Xsite;
                           Slope_Rings=(float)(Sideline_status_array[(image_bottom_value-5)].rightline-flag_Xsite_1)/(float)((image_bottom_value-5)-flag_Ysite_1);
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

                    for(Ysite=imagestatus.OFFLine+1;Ysite<60;Ysite++)
                    {
                        if(Sideline_status_array[Ysite].IsLeftFind=='T'&&Sideline_status_array[Ysite+1].IsLeftFind=='T'&&Sideline_status_array[Ysite+2].IsLeftFind=='W'
                            &&abs(Sideline_status_array[Ysite].leftline-Sideline_status_array[Ysite+2].leftline)>10
                          )
                        {
                            flag_Ysite_1=Ysite;
                            flag_Xsite_1=Sideline_status_array[flag_Ysite_1].leftline;
                            imagestatus.OFFLine=Ysite;
                            Slope_Rings=(float)(image_bottom_value-flag_Xsite_1)/(float)(image_side_width-flag_Ysite_1);
                            break;
                        }

                    }
                }
                //����
                if(flag_Ysite_1 != 0)
                {
                    for(Ysite=flag_Ysite_1;Ysite<image_bottom_value;Ysite++)
                    {
                        Sideline_status_array[Ysite].rightline=flag_Xsite_1+Slope_Rings*(Ysite-flag_Ysite_1);
        //                if(ImageFlag.ring_big_small==1)//��Բ���������
//                            Sideline_status_array[Ysite].midline = (Sideline_status_array[Ysite].rightline + Sideline_status_array[Ysite].leftline)/2;
        //                else//СԲ�������
        //                    Sideline_status_array[Ysite].midline = Sideline_status_array[Ysite].rightline - Half_Bend_Wide[Ysite];
//                        if(Sideline_status_array[Ysite].midline<0)
//                            Sideline_status_array[Ysite].midline = 0;
                            LimitL(Sideline_status_array[Ysite].rightline);
                            LimitH(Sideline_status_array[Ysite].rightline);
                    }
                    Sideline_status_array[flag_Ysite_1].rightline=flag_Xsite_1;
                    for(Ysite=flag_Ysite_1-1;Ysite>10;Ysite--) //A���Ϸ�����ɨ��
                    {
                        for(Xsite=Sideline_status_array[Ysite+1].rightline-10;Xsite<Sideline_status_array[Ysite+1].rightline+2;Xsite++)
                        {
                            if(image[Ysite][Xsite]!=0 && image[Ysite][Xsite+1]==0)
                            {
                                Sideline_status_array[Ysite].rightline=Xsite;
        //                        if(ImageFlag.ring_big_small==1)//��Բ���������
//                                    Sideline_status_array[Ysite].midline = (Sideline_status_array[Ysite].rightline + Sideline_status_array[Ysite].leftline)/2;
        //                        else//СԲ�������
        //                            Sideline_status_array[Ysite].midline = Sideline_status_array[Ysite].rightline - Half_Bend_Wide[Ysite];
//                                if(Sideline_status_array[Ysite].midline<0)
//                                    Sideline_status_array[Ysite].midline = 0;
                                Sideline_status_array[Ysite].wide=Sideline_status_array[Ysite].rightline-Sideline_status_array[Ysite].leftline;
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
//     {
//         uint8 x=0,y=0;
//         Point_Ysite = 0;
//         Point_Xsite = 0;
//         for (Ysite = (image_bottom_value-4); Ysite > imagestatus.OFFLine + 7; Ysite--)
//         {
//             if (
//                     Sideline_status_array[Ysite].leftline >= Sideline_status_array[Ysite - 2].leftline
//
//                     && Sideline_status_array[Ysite].leftline >= Sideline_status_array[Ysite - 1].leftline
//
//                     && Sideline_status_array[Ysite].leftline >= Sideline_status_array[Ysite - 4].leftline
//
//                     && Sideline_status_array[Ysite].leftline >= Sideline_status_array[Ysite - 6].leftline
//
//                     && Sideline_status_array[Ysite].leftline >= Sideline_status_array[Ysite - 5].leftline
//             )
//             {
//                 Point_Xsite = Sideline_status_array[Ysite].rightline;
//                 Point_Ysite = Ysite;
//                 break;
//             }
//         }
//         for(Ysite = (image_bottom_value-4); Ysite > imagestatus.OFFLineBoundary + 7; Ysite--)
//         {
//             if(Sideline_status_array[Ysite].LeftBoundary-Sideline_status_array[Ysite].LeftBoundary_First>5)
//             {
//                 x=Sideline_status_array[Ysite].LeftBoundary;
//                 y=Ysite;
//                 break;
//             }
//             if(Sideline_status_array[Ysite].RightBoundary_First-Sideline_status_array[Ysite].RightBoundary>5)
//             {
//                 x=Sideline_status_array[Ysite].RightBoundary;
//                 y=Ysite;
//                 break;
//             }
//         }
//         imagestatus.OFFLine=y+5;
//         connect_line_subsidiary(y,Point_Ysite,x,Point_Xsite,1);
//     }
//     if(imageflag.image_element_rings_flag==5)
//     {
//         Point_Ysite=0;
//         Point_Xsite=0;
//         for(Ysite = (image_bottom_value-4); Ysite > imagestatus.OFFLineBoundary + 7; Ysite--)
//                 {
//                     if(Sideline_status_array[Ysite].LeftBoundary-Sideline_status_array[Ysite].LeftBoundary_First>5)
//                     {
//                         Point_Xsite=Sideline_status_array[Ysite].LeftBoundary;
//                         Point_Ysite=Ysite;
//                         break;
//                     }
//                     if(Sideline_status_array[Ysite].RightBoundary_First-Sideline_status_array[Ysite].RightBoundary>5)
//                     {
//                         Point_Xsite=Sideline_status_array[Ysite].RightBoundary;
//                         Point_Ysite=Ysite;
//                         break;
//                     }
//                 }
//                 connect_line_subsidiary(Point_Ysite,image_bottom_value,Point_Xsite,image_side_width,1);
//     }
//     if(imageflag.image_element_rings_flag==8)
//     {
//         for(Ysite = (image_bottom_value-4);Ysite>imagestatus.OFFLine;Ysite--)
//         {
//             if(Sideline_status_array[Ysite].IsRightFind!='T')
//             {
//                 if(Sideline_status_array[Ysite].IsLeftFind=='T')
//                 {
//                     Sideline_status_array[Ysite].rightline=Sideline_status_array[Ysite].leftline+track_width;
//
//                     LimitL(Sideline_status_array[Ysite].rightline);  //�޷�
//                     LimitH(Sideline_status_array[Ysite].rightline);  //�޷�
//
//                     Sideline_status_array[Ysite].IsRightFind='T';
//
//                 }
//                 else {
////                     uint8 L_found_point = 0;
//                     uint8 R_found_point = 0;
//                     for (uint8 ysite = Ysite + 1; ysite < Ysite + 15; ysite++)
//                             {
//                               if (Sideline_status_array[ysite].IsRightFind == 'T')
//                                 {
//                                   R_found_point++;
//                                 }
//                             }
//                     if(R_found_point>=8)
//                     {
//                         float D_R = ((float)(Sideline_status_array[Ysite + R_found_point].rightline - Sideline_status_array[Ysite + 3].rightline)) /((float)(R_found_point - 3));
//
//                         Sideline_status_array[Ysite].rightline =Sideline_status_array[Ysite+2].rightline -D_R * 2;  //����ҵ��� ��ô�Ի�׼�����ӳ���
////                         (Sideline_status_array[Ysite].rightline = (((Sideline_status_array[Ysite].rightline) < (Sideline_status_array[Ysite].leftline+track_width/3)) ? (Sideline_status_array[Ysite].leftline+track_width/3) : (Sideline_status_array[Ysite].rightline)));
//                         LimitL(Sideline_status_array[Ysite].rightline);  //�޷�
//                         LimitH(Sideline_status_array[Ysite].rightline);  //�޷�
//                         Sideline_status_array[Ysite].IsRightFind='T';
//                     }
//
//                }
//             }
//     }

//     }
     if(imageflag.image_element_rings_flag==8)
     {
//         uint8 bit_x=image_side_width-5,bit_y=image_bottom_value-4;
//         for(Ysite=(image_bottom_value-4);Ysite>imagestatus.OFFLine;Ysite--)
//         {
//             if((Sideline_status_array[Ysite-1].IsRightFind=='W'||Sideline_status_array[Ysite-1].rightline>(image_side_width-5))&&(Sideline_status_array[Ysite-2].IsRightFind=='W'||Sideline_status_array[Ysite-2].rightline>(image_side_width-5))&&(Sideline_status_array[Ysite-3].IsRightFind=='W'||Sideline_status_array[Ysite-3].rightline>(image_side_width-5)))
//             {
//                 bit_x=Sideline_status_array[Ysite].rightline;
//                 bit_y=Ysite;
//                 break;
//             }
//         }
         for(Ysite=image_bottom_value-4;Ysite> imagestatus.OFFLine;Ysite--)
                             {
                                 Sideline_status_array[Ysite].rightline=Sideline_status_array[(image_bottom_value-4)].rightline+1.2*(Ysite-(image_bottom_value-4));
                 //                if(ImageFlag.ring_big_small==1)//��Բ���������
         //                            Sideline_status_array[Ysite].midline = (Sideline_status_array[Ysite].rightline + Sideline_status_array[Ysite].leftline)/2;
                 //                else//СԲ�������
                 //                    Sideline_status_array[Ysite].midline = Sideline_status_array[Ysite].rightline - Half_Bend_Wide[Ysite];
         //                        if(Sideline_status_array[Ysite].midline<0)
         //                            Sideline_status_array[Ysite].midline = 0;
                                     LimitL(Sideline_status_array[Ysite].rightline);
                                     LimitH(Sideline_status_array[Ysite].rightline);
                             }
     }
     if(imageflag.image_element_rings_flag==9)
     {
         uint8 bit=0;
         for(Ysite = (image_bottom_value-1); Ysite > (imagestatus.OFFLine+2); Ysite--)
         {
//             Sideline_status_array[Ysite].rightline=Sideline_status_array[Ysite+1].rightline-1;
//             LimitL(Sideline_status_array[Ysite].rightline);
             if(Sideline_status_array[Ysite-1].IsRightFind=='W'&&Sideline_status_array[Ysite].IsRightFind=='T'&& Sideline_status_array[Ysite+1].IsRightFind=='T'&& Sideline_status_array[Ysite+2].IsRightFind=='T')
                 bit=Ysite;
             if(Ysite==imagestatus.OFFLine+4)
                 bit=Ysite;
         }
         connect_line_subsidiary((imagestatus.OFFLine+2), bit, Sideline_status_array[(imagestatus.OFFLine+2)].leftline,Sideline_status_array[bit].rightline, 2);
     }
 }
}
//--------------------------------------------------------------------
// �������     ʮ�ִ���
//--------------------------------------------------------------------
void Cross_road_Handle(void)
{
    if(imagestatus.Miss_Left_lines<5||imagestatus.Miss_Right_lines<5||imageflag.CrossRoad_Flag==0||imageflag.image_element_rings!=0||imageflag.Zebra_Flag!=0) return;

}
//--------------------------------------------------------------------
// �������     ���߸�������
// ����˵��     y_up        �����ϵ�
// ����˵��     y_down      �����µ�
// ����˵��     x_up        �����ϵ�
// ����˵��     x_down      �����µ�
// ����˵��     mode        0������ߣ�1���ұ���
//---------------------------------------------------------------------
void connect_line_subsidiary(uint8 y_up,uint8 y_down,uint8 x_up,uint8 x_down,uint8 mode)
{

   float DetL =((float)(x_up-x_down)) /((float)(y_up - y_down));  //��������ߵĲ���б��
        if(mode==0)
        {
            for (uint8 ytemp = y_down; ytemp >= y_up; ytemp--)               //��ô�Ҿʹӵ�һ��ɨ������߽������ڶ��е�λ�ÿ�ʼ����һֱ���ߣ�����FTSite�С�
                   {
                       Sideline_status_array[ytemp].leftline =(int)(DetL * ((float)(ytemp - y_down))) +Sideline_status_array[y_down].leftline;     //������Ǿ���Ĳ��߲�����
                   }
        }
        else
        {
            for (uint8 ytemp = y_down; ytemp >= y_up; ytemp--)               //��ô�Ҿʹӵ�һ��ɨ������߽������ڶ��е�λ�ÿ�ʼ����һֱ���ߣ�����FTSite�С�
            {
                Sideline_status_array[ytemp].rightline =(int)(DetL * ((float)(ytemp - y_down))) +Sideline_status_array[y_down].rightline;     //������Ǿ���Ĳ��߲�����
            }
        }
}
//------------------------------------------------------------------------------------------------------
// �������     ֱ������ʽ���/���ֳ��߱���
// ��ע��Ϣ     ͨ��midline�жϵ�ǰ������ֱ���������������ص�imageflag.straight_long[];
//------------------------------------------------------------------------------------------------------
void Straight_Judgment_Third(void)
{
    float a;
    /*��־λ���*/
    imageflag.straight_long[0]=0;
    imageflag.straight_long[1]=0;
    imageflag.straight_long[2]=0;

    /*Զ��ֱ���ж�*/
    if(imagestatus.OFFLine < 15)
    {
        a=Straight_Judge(3,(imagestatus.OFFLine+1),30);
        if(a<1){imageflag.straight_long[0]=1;}
    }
    /*�ж�ֱ���ж�*/
    if(imagestatus.OFFLine < 30)
    {
        a=Straight_Judge(3,31,60);
        if(a<1){imageflag.straight_long[1]=1;}
    }
    else if(imagestatus.OFFLine < 45)
    {
        a=Straight_Judge(3,(imagestatus.OFFLine+1),60);
        if(a<1){imageflag.straight_long[1]=1;}
    }
    /*����ֱ���ж�*/
    if(imagestatus.OFFLine < 60)
    {
        a=Straight_Judge(3,61,89);
        if(a<1){imageflag.straight_long[2]=1;}
    }

//    if (imagestatus.OFFLine >= 70)
//    {
//        imageflag.Out_Road=1;
//    }else {
//        imageflag.Out_Road=0;
//    }
}
//-------------------------------------------------------------------------------------------------------------------
// �������     �������ݼ�¼�뷵�س���Ƕȵ���
// ����˵��     mode �������ݵ�����  0-����������ƫ������ڵ�·��࣬�������ڵ�·�Ҳ� 1-����Ƕ�ƫ��
// ���ز���     Ѳ�߽Ƕȼ�����Ƕ�����
// ʹ��ʾ��     int angle = midline_and_anglereturn();
// ��ע��Ϣ
//-------------------------------------------------------------------------------------------------------------------

float midline_and_anglereturn(uint8 mode)
{
    uint8 OFFLine=70;
    int point1,point2,point3;
    float angle;
    if(OFFLine<imagestatus.OFFLine)OFFLine=imagestatus.OFFLine;

    if(mode==0){point2=Sideline_status_array[(uint8)(80+OFFLine)/2].midline-45 ;return point2;}
    if(mode==1){

        point1=Sideline_status_array[(uint8)(80+OFFLine)/2].midline;
        point3=Sideline_status_array[80].midline;
        angle=angle_compute(point1,((uint8)(80+OFFLine)/2),point3,80);
        return angle;}

    return 0;

}
float angle_compute(uint x1,uint y1,uint x2,uint y2)
{
    double c,a;
    float angle;
    if(x1==x2)return 0;
    a=abs(x1-x2);
    c=PI/180;
    angle=atan2(a,(y2-y1))/c;
    if(x2>x1){angle=-angle;}
    return angle;
}
//------------------------------------------------------------------------------------------------------
// �������     ͼ�����ܺ���
//------------------------------------------------------------------------------------------------------
void Camera_tracking(void)
{
    Binaryzation();
    image_draw();
    Get_BaseLine();
    Get_AllLine();
    draw_midline();
    if(!imageflag.Out_Road  && !imageflag.RoadBlock_Flag)
               Search_Border_OTSU(image, image_h, image_w, image_bottom_value - 1);//58��λ����
           else
               imagestatus.OFFLineBoundary = 5;
    Element_Judgment_Left_Rings();
    Element_Judgment_Right_Rings();
    Straight_Judgment_Third();
    if(imageflag.image_element_rings==1)
    Element_Handle_Left_Rings();
    draw_midline();
    camera_tft180show();
}
