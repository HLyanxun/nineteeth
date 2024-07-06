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
int My_Threshold_cha=10;  //��ֵ����ֵ����ֵ
//������ͼ�񼰴������------------------------------------------------------------------------------------------------------
uint8 image[image_h][image_w]={{0}};  //ʹ�õ�ͼ��
static uint8* PicTemp;                          //һ�����浥��ͼ���ָ�����
static int Ysite = 0, Xsite = 0;                //Ysite����ͼ����У�Xsite����ͼ����С�
static int BottomBorderRight = image_side_width,              //89�е��ұ߽�
BottomBorderLeft = 0,                           //89�е���߽�
BottomCenter = 0;                               //89�е��е�
uint8 ExtenLFlag = 0;                           //������Ƿ���Ҫ���ߵı�־����
uint8 ExtenRFlag = 0;                           //�ұ����Ƿ���Ҫ���ߵı�־����

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
    tft180_show_gray_image(0, 0, image[0], image_w, image_h, image_w, image_h, 0);
    if(track_show)
    {
        for(uint8 i=imagestatus.OFFLine;i<=image_bottom_value;i++)
            {
                LimitL(Sideline_status_array[Ysite].rightline);  //�޷�
                LimitH(Sideline_status_array[Ysite].rightline);  //�޷�
                LimitL(Sideline_status_array[Ysite].leftline);  //�޷�
                LimitH(Sideline_status_array[Ysite].leftline);  //�޷�
                LimitL(Sideline_status_array[Ysite].midline);  //�޷�
                LimitH(Sideline_status_array[Ysite].midline);  //�޷�
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
// �������         ��ֵ��
// ����˵��
// ���ز���
// ʹ��ʾ��
// ��ע��Ϣ
//-------------------------------------------------------------------------------------------------------------------
void Binaryzation(void)
{
    My_Threshold   = (int)get_Threshold()    + My_Threshold_cha;  //1ms
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

    PicTemp = image[image_bottom_value];                                                //��PicTemp���ָ�����ָ��ͼ�������Pixle[image_bottom_value]
//    for (Xsite = line_midpoint; Xsite < image_side_width; Xsite++)                   //����line_midpoint�������У��������п�ʼһ��һ�е����ұ������ұ���
//    {
//      if (*(PicTemp + Xsite) == 0 && *(PicTemp + Xsite + 1) == 0)       //������������������ڵ㣬˵û�ҵ��˱��ߡ�
//      {
//        BottomBorderRight = Xsite;                                      //����һ�м�¼������Ϊ��һ�е��ұ���
//        break;                                                          //����ѭ��
//      }
//      else if (Xsite == (image_side_width-1))                                             //����ҵ��˵�58�ж���û���ֺڵ㣬˵����һ�еı��������⡣
//      {
//        BottomBorderRight = (image_side_width-1);                                         //����������Ĵ�����ǣ�ֱ�Ӽ���ͼ�����ұߵ���һ�У���79�У�������һ�е��ұ��ߡ�
//        break;                                                          //����ѭ��
//      }
//    }
//
//    for (Xsite = line_midpoint; Xsite > 0; Xsite--)                    //����39�������У��������п�ʼһ��һ�е���������������
//    {
//      if (*(PicTemp + Xsite) == 0 && *(PicTemp + Xsite - 1) == 0)       //������������������ڵ㣬˵û�ҵ��˱��ߡ�
//      {
//        BottomBorderLeft = Xsite;                                       //����һ�м�¼������Ϊ��һ�е������
//        break;                                                          //����ѭ��
//      }
//      else if (Xsite == 1)                                              //����ҵ��˵�1�ж���û���ֺڵ㣬˵����һ�еı��������⡣
//      {
//        BottomBorderLeft = 0;                                           //����������Ĵ�����ǣ�ֱ�Ӽ���ͼ������ߵ���һ�У���0�У�������һ�е�����ߡ�
//        break;                                                          //����ѭ��
//      }
//    }
    /*ȷ�������*/
    uint8 temp_min=(line_midpoint-track_width/2)-ImageScanInterval;
    uint8 temp_max=(line_midpoint-track_width/2)+ImageScanInterval;
    for (uint8 i = temp_min; i <=temp_max ; i++)                    //��������ɨ
      {
        if (*(PicTemp + i) != 0 && *(PicTemp + i - 1) == 0)   //����кڰ�����    1�ǰ� 0�Ǻ�
        {
            BottomBorderLeft = i;                           //�ǾͰ�����м�¼������Ϊ�����
          break;                                  //�ҵ��˾�����ѭ��������
        }
        else if (i == temp_max)                    //Ҫ��ɨ�����û�ҵ�
        {
          if (*(PicTemp + (temp_min + temp_max) / 2) != 0)            //����ɨ��������м��ǰ����ص�
          {
              for(uint8 j=(temp_min-1);j>0;j--)
              {
                  if (*(PicTemp + i) != 0 && *(PicTemp + i - 1) == 0)   //����кڰ�����    1�ǰ� 0�Ǻ�
                  {
                      BottomBorderLeft = i;                           //�ǾͰ�����м�¼������Ϊ�����
                      break;                                  //�ҵ��˾�����ѭ��������
                  }
                  if(j == 1)
                  {
                      BottomBorderLeft = 0;
                      break;
                  }
              }
              break;
          }
          else                                    //Ҫ��ɨ�����û�ҵ�������ɨ��������м��Ǻ����ص�
          {
              for(uint8 j=(temp_max+1);j<line_midpoint;j++)
              {
                  if (*(PicTemp + i) != 0 && *(PicTemp + i - 1) == 0)   //����кڰ�����    1�ǰ� 0�Ǻ�
                  {
                      BottomBorderLeft = i;                           //�ǾͰ�����м�¼������Ϊ�����
                      break;                                  //�ҵ��˾�����ѭ��������
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
    /*ȷ���ұ���*/
    temp_min=(line_midpoint+track_width/2)-ImageScanInterval;
    temp_max=(line_midpoint+track_width/2)+ImageScanInterval;
    for (uint8 i = temp_max; i <=temp_min ; i--)                    //��������ɨ
          {
            if (*(PicTemp + i) != 0 && *(PicTemp + i + 1) == 0)   //����кڰ�����    1�ǰ� 0�Ǻ�
            {
                BottomBorderRight = i;                           //�ǾͰ�����м�¼������Ϊ�����
              break;                                  //�ҵ��˾�����ѭ��������
            }
            else if (i == temp_min)                    //Ҫ��ɨ�����û�ҵ�
            {
              if (*(PicTemp + (temp_min + temp_max) / 2) != 0)            //����ɨ��������м��ǰ����ص�
              {
                  for(uint8 j=(temp_max+1);j<=image_side_width;j++)
                  {
                      if (*(PicTemp + i) != 0 && *(PicTemp + i + 1) == 0)   //����кڰ�����    1�ǰ� 0�Ǻ�
                      {
                          BottomBorderRight = i;                           //�ǾͰ�����м�¼������Ϊ�����
                          break;                                  //�ҵ��˾�����ѭ��������
                      }
                      if(j == image_side_width)
                      {
                          BottomBorderRight = image_side_width;
                          break;
                      }
                  }
                  break;
              }
              else                                    //Ҫ��ɨ�����û�ҵ�������ɨ��������м��Ǻ����ص�
              {
                  for(uint8 j=(temp_min-1);j>line_midpoint;j++)
                  {
                      if (*(PicTemp + i) != 0 && *(PicTemp + i + 1) == 0)   //����кڰ�����    1�ǰ� 0�Ǻ�
                      {
                          BottomBorderRight = i;                           //�ǾͰ�����м�¼������Ϊ�����
                          break;                                  //�ҵ��˾�����ѭ��������
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

    BottomCenter =(BottomBorderLeft + BottomBorderRight) / 2;           //�������ұ߽�������89�е�����
    Sideline_status_array[image_bottom_value].leftline = BottomBorderLeft;                        //�ѵ�89�е���߽�洢�����飬ע�⿴ImageDeal������ֵ��±꣬�ǲ������ö�Ӧ89��
    Sideline_status_array[image_bottom_value].rightline = BottomBorderRight;                      //�ѵ�89�е��ұ߽�洢�����飬ע�⿴ImageDeal������ֵ��±꣬�ǲ������ö�Ӧ89��
    Sideline_status_array[image_bottom_value].midline = BottomCenter;                                //�ѵ�89�е����ߴ洢�����飬    ע�⿴ImageDeal������ֵ��±꣬�ǲ������ö�Ӧ89��
    Sideline_status_array[image_bottom_value].wide = BottomBorderRight - BottomBorderLeft;          //�ѵ�89�е�������ȴ洢���飬ע�⿴ImageDeal������ֵ��±꣬�ǲ������ö�Ӧ89��
    Sideline_status_array[image_bottom_value].IsLeftFind = 'T';                                     //��¼��89�е����������ΪT���������ҵ�����ߡ�
    Sideline_status_array[image_bottom_value].IsRightFind = 'T';                                    //��¼��89�е��ұ�������ΪT���������ҵ��ұ��ߡ�

    /****************************************************End*******************************************************************************/
    /**************************************��������ͼ������У�59�У����ұ��ߴӶ�ȷ�����ߵĹ��� ********************************************************************/



    /**************************************�ڵ�59�������Ѿ�ȷ���������ȷ��58-54���������ߵĹ��� ******************************************/
    /****************************************************Begin*****************************************************************************/
    /*
         * ���漸�еĵ����߹����ҾͲ���׸���ˣ������ҵ�ע�Ͱѵ�89�е����߹������ã�
         * ��ô88��84�е����߾���ȫû���⣬��һģһ�����߼��͹��̡�
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
        /*ȷ�������*/
         temp_min=(Sideline_status_array[Ysite + 1].midline-track_width/2)-ImageScanInterval;
         temp_max=(Sideline_status_array[Ysite + 1].midline-track_width/2)+ImageScanInterval;
            for (uint8 i = temp_min; i <=temp_max ; i++)                    //��������ɨ
              {
                if (*(PicTemp + i) != 0 && *(PicTemp + i - 1) == 0)   //����кڰ�����    1�ǰ� 0�Ǻ�
                {
                    BottomBorderLeft = i;                           //�ǾͰ�����м�¼������Ϊ�����
                  break;                                  //�ҵ��˾�����ѭ��������
                }
                else if (i == temp_max)                    //Ҫ��ɨ�����û�ҵ�
                {
                  if (*(PicTemp + (temp_min + temp_max) / 2) != 0)            //����ɨ��������м��ǰ����ص�
                  {
                      for(uint8 j=(temp_min-1);j>0;j--)
                      {
                          if (*(PicTemp + i) != 0 && *(PicTemp + i - 1) == 0)   //����кڰ�����    1�ǰ� 0�Ǻ�
                          {
                              BottomBorderLeft = i;                           //�ǾͰ�����м�¼������Ϊ�����
                              break;                                  //�ҵ��˾�����ѭ��������
                          }
                          if(j == 1)
                          {
                              BottomBorderLeft = 0;
                              break;
                          }
                      }
                      break;
                  }
                  else                                    //Ҫ��ɨ�����û�ҵ�������ɨ��������м��Ǻ����ص�
                  {
                      for(uint8 j=(temp_max+1);j<line_midpoint;j++)
                      {
                          if (*(PicTemp + i) != 0 && *(PicTemp + i - 1) == 0)   //����кڰ�����    1�ǰ� 0�Ǻ�
                          {
                              BottomBorderLeft = i;                           //�ǾͰ�����м�¼������Ϊ�����
                              break;                                  //�ҵ��˾�����ѭ��������
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
            /*ȷ���ұ���*/
            temp_min=(Sideline_status_array[Ysite + 1].midline+track_width/2)-ImageScanInterval;
            temp_max=(Sideline_status_array[Ysite + 1].midline+track_width/2)+ImageScanInterval;
            for (uint8 i = temp_max; i <=temp_min ; i--)                    //��������ɨ
                  {
                    if (*(PicTemp + i) != 0 && *(PicTemp + i + 1) == 0)   //����кڰ�����    1�ǰ� 0�Ǻ�
                    {
                        BottomBorderRight = i;                           //�ǾͰ�����м�¼������Ϊ�����
                      break;                                  //�ҵ��˾�����ѭ��������
                    }
                    else if (i == temp_min)                    //Ҫ��ɨ�����û�ҵ�
                    {
                      if (*(PicTemp + (temp_min + temp_max) / 2) != 0)            //����ɨ��������м��ǰ����ص�
                      {
                          for(uint8 j=(temp_max+1);j<=image_side_width;j++)
                          {
                              if (*(PicTemp + i) != 0 && *(PicTemp + i + 1) == 0)   //����кڰ�����    1�ǰ� 0�Ǻ�
                              {
                                  BottomBorderRight = i;                           //�ǾͰ�����м�¼������Ϊ�����
                                  break;                                  //�ҵ��˾�����ѭ��������
                              }
                              if(j == image_side_width)
                              {
                                  BottomBorderRight = image_side_width;
                                  break;
                              }
                          }
                          break;
                      }
                      else                                    //Ҫ��ɨ�����û�ҵ�������ɨ��������м��Ǻ����ص�
                      {
                          for(uint8 j=(temp_min-1);j>line_midpoint;j++)
                          {
                              if (*(PicTemp + i) != 0 && *(PicTemp + i + 1) == 0)   //����кڰ�����    1�ǰ� 0�Ǻ�
                              {
                                  BottomBorderRight = i;                           //�ǾͰ�����м�¼������Ϊ�����
                                  break;                                  //�ҵ��˾�����ѭ��������
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
    /**************************************�ڵ�59�������Ѿ�ȷ���������ȷ��58-54���������ߵĹ��� ****************************************/
}
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
  imagestatus.OFFLine=2;     //����ṹ���Ա��֮���������︳ֵ������Ϊ��ImageStatus�ṹ������ĳ�Ա̫���ˣ�������ʱ��ֻ�õ���OFFLine�������������õ��������ĸ�ֵ��
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
    if(JumpPoint[0].type != 'T' || JumpPoint[1].type!= 'T')mid_choose_value++;
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
//    if(Sideline_status_array[Ysite].wide< track_width)imageflag.mid_choose=1;
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
    /**************************�����ұ��ߵ��ޱ���***************************/
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
        Sideline_status_array[Ysite].rightline =Sideline_status_array[ytemp_W_R].rightline -D_R * (ytemp_W_R - Ysite);  //����ҵ��� ��ô�Ի�׼�����ӳ���
      }
      LimitL(Sideline_status_array[Ysite].rightline);  //�޷�
      LimitH(Sideline_status_array[Ysite].rightline);  //�޷�
    }

    /**************************�����ұ��ߵ��ޱ���***************************/


    /**************************��������ߵ��ޱ���***************************/
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

      LimitL(Sideline_status_array[Ysite].leftline);  //�޷�
      LimitH(Sideline_status_array[Ysite].leftline);  //�޷�
    }

    /**************************��������ߵ��ޱ���***************************/
    /************************************����ȷ���ޱ��У���W�ࣩ�ı߽�****************************************************************/


    /************************************��������֮��������һЩ������������*************************************************/
      if (Sideline_status_array[Ysite].IsLeftFind == 'W'&&Sideline_status_array[Ysite].IsRightFind == 'W')
      {
          imagestatus.WhiteLine++;  //Ҫ�����Ҷ��ޱߣ�������+1
      }
     if (Sideline_status_array[Ysite].IsLeftFind == 'W' && Ysite < (image_bottom_value-4) )
     {
          imagestatus.Miss_Left_lines++;
     }
     if (Sideline_status_array[Ysite].IsRightFind == 'W'&& Ysite < (image_bottom_value-4) )
     {
          imagestatus.Miss_Right_lines++;
     }

      LimitL(Sideline_status_array[Ysite].leftline);   //�޷�
      LimitH(Sideline_status_array[Ysite].leftline);   //�޷�
      LimitL(Sideline_status_array[Ysite].rightline);  //�޷�
      LimitH(Sideline_status_array[Ysite].rightline);  //�޷�

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
// �������     �������
//------------------------------------------------------------------------------------------------------
void Element_Judgment(void)
{

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
    camera_tft180show();
}
