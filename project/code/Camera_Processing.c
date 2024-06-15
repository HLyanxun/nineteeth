/*
 * Camera_Processing.c
 *
 *  Created on: 2024��3��17��
 *      Author: 21335
 */

#include <stdio.h>
#include "math.h"
#include "stdlib.h"
#include "zf_common_headfile.h"

#define M_PI       3.14159265358979323846   // pi
//ͼ��-----------------------------------------------------------------------------------------------------------------
uint8 image[image_h][image_w]={{0}};  //ʹ�õ�ͼ��
uint8 l_shade_255=0,r_shade_255=0; //����Ҳ���Ӱ  Ҳ���� ��ɫ����
uint8 centre_line=34;
float track_width=26;//�������
uint8 midline[92];
//���---------------------------------------------------------------------------------------------------------------
uint8  My_Threshold = 0, My_Threshold_1 = 0; //��ֵ��֮�����ֵ
float  My_Threshold_cha = 10;//��ֵ���� ֮���ӵ�ui������Խ��е���
//��͸��---------------------------------------------------------------------------------------------------------------
uint8_t *PerImg_ip[RESULT_ROW][RESULT_COL]={{0}};//�洢ͼ���ַ
//������---------------------------------------------------------------------------------------------------------------

#define USE_num image_h*3   //�����ҵ�������Ա��������˵300�����ܷ��£�������Щ�������ȷʵ�Ѷ����ඨ����һ��
int image_ok=0;
uint8 Find_Boundary_l[300][2]={{0}};//��߽��� 0Ϊy  1Ϊx �洢����Ϊ������ɨ������ı߽�
uint8 Find_Boundary_r[300][2]={{0}};//�ұ߽��� 0Ϊy  1Ϊx �洢����Ϊ������ɨ������ı߽�
uint8 Find_Boundary_z[300][2]={{0}};//�б߽��� 0Ϊy  1Ϊx �洢����Ϊ������ɨ������ı߽�
uint8 lost_line[300][2]={{0}};//0�� 1��   �Ƿ��� ��ֵ����0���� ����1����
uint8 lostline_flag_l=0,lostline_flag_r=0;  //���Ҷ��߱�־λ
uint8 Find_Boundary_centre[300][2]={{0}};//���� 0Ϊy  1Ϊx �洢����Ϊ������ɨ������ı߽�
uint16 l_data_statics=0;//ͳ������ܹ��ĳ���
uint16 r_data_statics=0;//ͳ���ұ��ܹ��ĳ���
uint8 hight_image = 0;//��ߵ�
uint16 centre=0,centre_last=centre_image,centre_s=0;//���ʹ�õ�����ֵ  centre_s ��Ϊ�˿�ǰ������̶�֮�� �ֶ�pd
//�����ж�---------------------------------------------------------------------------------------------------------------
uint8 curvature_l[6]={0},curvature_r[6]={0};  //���ұ������� ����012  �ж�3  Զ��4  ���5��� ����ֱ��
uint8 straight_l=0,straight_r=0;  //ֱ���жϱ�־λ 1Ϊ ����ֱ 2�ж�ֱ 3Զ��ֱ
uint8 Corners_l=0,Corners_r=0;  //����жϱ�־λ 1Ϊ ���
uint8 ramp_flag=0,ramp_widch[4]={0};

double curvature_l_l=0;

//ʮ��----------------------------------------------------------------------------------------------------------------
uint8 l_Down[4]={0},l_on[4]={0},r_Down[4]={0},r_on[4]={0}; //ʮ�ֵ��ĸ��յ� 0y1x2flag
int cross_flag=0;  //ʮ�ֱ�־λ


//������---------------------------------------------------------------------------------------------------------------
int zebra_crossing_flag = 0;  //�����߱�־λ
int region=0; //�����������
//����---------------------------------------------------------------------------------------------------------------
int L_island_flag = 0,R_island_flag = 0;  //���һ�����־λ
uint8 is_L_on[4]={0},is_R_on[4]={0};  //���� ���� ���� �����յ�0y1x
uint8 is_L_down[3]={0},is_R_down[3]={0};  //���� ���� ���� �����յ�0y1x
#define island_in_limit_x  35 //�����������Ƶ�λ
uint32 island_length=0;
int island_big=0;
//ֱ�߼���---------------------------------------------------------------------------------------------------------------
int run_flag=0,out_flag=0;
int speed_flag=0,speed_in_flag=0;
int check_flag = 0;//�������
float speed_increase=0; //ֱ�߼���
uint16 lr_en_speed=0;//����ƽ���������ۼ�ֵ
int16 l_en_speed=0,r_en_speed=0; //�� �� ��������ֵ
int16 l_en_speed_last=0,r_en_speed_last=0; //�� �� ��������һ�ε�ֵ
float speed_l=0,speed_r=0; //�ٶ�
float Servos_out; //������ֵ
attitude_t g_attitude;
attitude_t angle;  //����ʹ��
PID Motor_pid_l;
PID Motor_pid_r;


//--------------------------------------------------------------------------------------
// �������     ȥ����+��͸�Ӵ����ʼ��
// ��ע��Ϣ     ֻ�����һ�μ��ɣ����ʹ��ImageUsedָ�뼴��
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


/*ImageUsed[0][0]����ͼ�����Ͻǵ�ֵ*/

/*�������ͷ��ʼ���󣬵���һ��ImagePerspective_Init���˺�ֱ�ӵ���ImageUsed   ��Ϊȥ������*/

//-------------------------------------------------------------------------------------------------------------------
// �������
// ����˵��
// ���ز���
// ʹ��ʾ��
// ��ע��Ϣ
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
// �������     otsuThreshold   ͼ��ѹ��
// ����˵��     *image��ͼ���ַ  width��ͼ���    height��ͼ���
// ���ز���
// ʹ��ʾ��     otsuThreshold( mt9v03x_image[0], MT9V03X_W, MT9V03X_H);
// ��ע��Ϣ
//-------------------------------------------------------------------------------------------------------------------
uint8 otsuThreshold(uint8 *image, uint16 width,  uint16 height)
{
    #define GrayScale 256
    int pixelCount[GrayScale] = {0};//ÿ���Ҷ�ֵ��ռ���ظ���
    float pixelPro[GrayScale] = {0};//ÿ���Ҷ�ֵ��ռ�����ر���
    int i,j;
    int Sumpix = width * height/4;   //�����ص�
    uint8 threshold = 0;
    uint8* data = image;  //ָ���������ݵ�ָ��


    //ͳ�ƻҶȼ���ÿ������������ͼ���еĸ���
    for (i = 0; i < height; i+=2)
    {
        for (j = 0; j < width; j+=2)
        {
            pixelCount[(int)data[i * width + j]]++;  //������ֵ��Ϊ����������±�
          //   pixelCount[(int)image[i][j]]++;    ������ָ�������
        }
    }
    float u = 0;
    for (i = 0; i < GrayScale; i++)
    {
        pixelPro[i] = (float)pixelCount[i] / Sumpix;   //����ÿ������������ͼ���еı���
        u += i * pixelPro[i];  //��ƽ���Ҷ�
    }


    float maxVariance=0.0;  //�����䷽��
    float w0 = 0, avgValue  = 0;  //w0 ǰ������ ��avgValue ǰ��ƽ���Ҷ�
    for(int i = 0; i < 256; i++)     //ÿһ��ѭ������һ��������䷽����� (����for����Ϊ1��)
    {
        w0 += pixelPro[i];  //���赱ǰ�Ҷ�iΪ��ֵ, 0~i �Ҷ�������ռ����ͼ��ı�����ǰ������
        avgValue  += i * pixelPro[i];

        float variance = pow((avgValue/w0 - u), 2) * w0 /(1 - w0);    //��䷽��
        if(variance > maxVariance)
        {
            maxVariance = variance;
            threshold = (uint8)i;
        }
    }


    return threshold;

}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//  @name           get_Threshold  //ָ��
//  @brief          �Ż�֮��ĵĴ�򷨡���򷨾���һ���ܹ����һ��ͼ����ѵ��Ǹ��ָ���ֵ��һ���㷨��
//  @brief          ����������ǿ������ʵ�ڲ�������ֱ�������ã�ʲô�����������޸ģ�ֻҪû�й���Ӱ�죬��ô������������ֵ��һ�����Եõ�һ��Ч��������Ķ�ֵ��ͼ��
//  @parameter      ex_mt9v03x_binarizeImage  ԭʼ�ĻҶ�ͼ������
//  @parameter      clo    ͼ��Ŀ�ͼ����У�
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

   //˫��ֵ����  �ȴ������һ����ֵ,����ֵ֮�µı�Ϊ��ɫ�ٽ���һ��
   //�ŵ�  ��ƫ��Ƭ���������Ч���Ƚϲ���
   //ȱ��  ��ʱ��
//    My_Threshold_1 =  //1ms

    My_Threshold   = (int)get_Threshold()    + (uint8)My_Threshold_cha;  //1ms


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
    int a=0;
    for(int i = image_h-3; i >= 0 ; i-- )//����͸��ͼ�������������,�ڽ���һ����ֵ�ݶȵݼ��Ķ�ֵ��
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
//    for(int i=0;i<89;i++)       //����� ȥ��
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
// �������     Find_Boundary()   Ѱ����������ұ߽���
// ����˵��
// ���ز���     find_Boundary_flag �ж��Ƿ��ҵ����ұ߽�
// ʹ��ʾ��
// ��ע��Ϣ
//-------------------------------------------------------------------------------------------------------------------

void shade_compute(void)   //�ж��Ƿ�ɨ�赽��Ӱ
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
// �������     Find_Boundary()   Ѱ����������ұ߽���
// ����˵��
// ���ز���     find_Boundary_flag �ж��Ƿ��ҵ����ұ߽�
// ʹ��ʾ��
// ��ע��Ϣ
//-------------------------------------------------------------------------------------------------------------------
int Find_Boundary(void)
{
    int find_Boundary_flag = 0;

     l_data_statics=0;//ͳ������ܹ��ĳ���
     r_data_statics=0;//ͳ���ұ��ܹ��ĳ���


     if (zebra_crossing_flag!=1)  {//��δ���ְ�����ʱ��ʹ��м�������ɨ�� Ϊɶ ��i = 2 ��ΪһȦ�Ǹ��ڿ� ������Ѳ������߽���
         for (uint8 i = 2; i < 3; i++){// Y
             for (uint8 j = centre_line ; j >= 0; j--){//�����  X
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

             for (uint8 j = centre_line ; j <= image_w - 1; j++){ //�ұ���   X
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
//         for (uint8 i = 2; i < 8; i++){//0 1 �б߽�
//             for (uint8 j = 0; j <= image_w / 2; j++){ //�����
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
//             for (uint8 j = image_w-1; j >= image_w / 2; j--){  //�ұ���
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
     return find_Boundary_flag;  //���������Ƿ��ҵ���ʼ��
}

//-------------------------------------------------------------------------------------------------------------------
// �������     search                ��������ʽ��ʼ�ұ߽�ĺ�������������е�࣬���õ�ʱ��Ҫ©�ˣ������������һ�������ꡣ
// ����˵��     break_flag              �������Ҫѭ���Ĵ���
// ����˵��     (*image)[image_w]       ����Ҫ�����ҵ��ͼ�����飬�����Ƕ�ֵͼ,�����������Ƽ���  �ر�ע�⣬��Ҫ�ú궨��������Ϊ����������������ݿ����޷����ݹ���
// ����˵��     hightest                ��ѭ���������õ�����߸߶�
// ���ز���
// ʹ��ʾ��
// ��ע��Ϣ
//-------------------------------------------------------------------------------------------------------------------
void search(uint16 break_flag,uint8(*image_u)[image_w],uint8* hight)
{

    if( Find_Boundary() >= 2){
        l_data_statics=0;//ͳ������ܹ��ĳ���
        r_data_statics=0;//ͳ���ұ��ܹ��ĳ���
        //����˸�����
        static int8 seeds_l[8][2] = { {-1,1},{0,1},{1,1},{1,0},{1,-1},{0,-1},{-1,-1},{-1,0}, };
        //{-1,-1},{0,-1},{+1,-1},6 5 4      �������ʱ��
        //{-1, 0},       {+1, 0},7   3
        //{-1,+1},{0,+1},{+1,+1},0 1 2
        static int8 seeds_r[8][2] = { {1, 1},{0,1},{-1,1},{-1,0},{-1,-1},{0,-1},{1,-1},{1,0}, };
        //{-1,-1},{0,-1},{+1,-1},4 5 6       �����˳ʱ��
        //{-1, 0},       {+1, 0},3   7
        //{-1,+1},{0,+1},{+1,+1},2 1 0


        uint8 search_filds_l[2] = { 0}; //ֻ��һ����ֵ�������ֵ��֮����ʾͼ���ĳ�����xy��ֵ
        uint8 search_filds_r[2] = { 0};
        int direction_l=3,direction_r=3;//�����������ߵķ���  0-7
        //��������ѭ��
        while (break_flag--)//���ѭ������_
        {
            if(  Find_Boundary_l[ l_data_statics ][0] >= Find_Boundary_r[ r_data_statics ][0]  )  //��ʼ  ���
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
                      direction_l-=2;  //����ҵ���ɫ�ĵ� �ͻ�ı����ķ���
                      break;
                  }
                  direction_l++;  //���Ǻ�ɫ �����1
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
                        direction_r-=2;  //����ҵ���ɫ�ĵ� �ͻ�ı����ķ���
                        break;
                    }
                    direction_r++;  //���Ǻ�ɫ �����1
                }
                r_data_statics++;

            }


            if (func_abs(Find_Boundary_r[r_data_statics][0] - Find_Boundary_l[l_data_statics][0]) < 2
                && func_abs(Find_Boundary_r[r_data_statics][1] - Find_Boundary_l[l_data_statics ][1]) < 3
                && r_data_statics>=10 && l_data_statics>=10 )
            {
                *hight = (Find_Boundary_r[r_data_statics][0]+Find_Boundary_l[l_data_statics][0])/2.0;//ȡ����ߵ�
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
// �������
// ����˵��
// ���ز���
// ʹ��ʾ��
// ��ע��Ϣ
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
// �������     ������ұ����Ƿ���
// ����˵��
// ���ز���
// ʹ��ʾ��
// ��ע��Ϣ
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
// �������     ������������
// ����˵��     line ��������
// ����˵��     point  ÿ�����ӵĵ���
// ����˵��     step һ���� ����������Ĳ�
// ���ز���
// ʹ��ʾ��
// ��ע��Ϣ     ֱ����curvatureThreePoints�ڲ��������ƽ�������⺯������
//-------------------------------------------------------------------------------------------------------------------
double calculateAverageCurvature(uint8 line[300][2],int n,int step,int point) {
    double totalCurvature = 0.0;
    int count = 0; // �����˶��ٸ�����

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
// �������
// ����˵��
// ���ز���
// ʹ��ʾ��
// ��ע��Ϣ
//-------------------------------------------------------------------------------------------------------------------
void curvature_judge(void)
{
    int x[3]={0},y[3]={0};
//Զ��----------------------------------------------------------------------
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

//�ж�----------------------------------------------------------------------
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


//����2----------------------------------------------------------------------
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


//����1----------------------------------------------------------------------
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

//���6----------------------------------------------------------------------
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

//���߸���----------------------------------------------------------------------
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
        straight_l=1;              //����ֱ��
        if(curvature_l[3] > 168 && abs( curvature_l[3] - curvature_l[1]) <= 15 ){
            straight_l = 2;  //�ж�
            if(curvature_l[4] > 168 && abs( curvature_l[4] - curvature_l[1]) <= 15 ){
                straight_l = 3;       //ȫ��ֱ��
            }
        }
    }
    else straight_l=0;

     if( curvature_r[1] > 165 && curvature_r[2] > 165
      && func_abs(curvature_r[1] - curvature_r[2]) <= 25
      && Find_Boundary_r[2][1]<60 ){
         straight_r=1;              //����ֱ��
         if(curvature_r[3] > 168 && abs( curvature_r[3] - curvature_r[1]) <= 15 ){
             straight_r = 2;  //�ж�
             if(curvature_r[4] > 168 && abs( curvature_r[4] - curvature_r[1]) <= 15 ){
                 straight_r = 3;       //ȫ��ֱ��
             }
         }
     }
     else straight_r=0;




     if( abs( 90- curvature_l[5]) >= 15 && straight_l==0 ){//��·
         Corners_l=1;
     }
     else Corners_l=0;
     if( abs( 90- curvature_r[5]) >= 15 && straight_r==0 ){
         Corners_r=1;
     }
     else Corners_r=0;

}

//-------------------------------------------------------------------------------------------------------------------
// �������     Stayguy_ADS  ���ߺ���
// ����˵��
// ���ز���
// ʹ��ʾ��
// ��ע��Ϣ
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
// �������   Angel_compute  ��ȡ���������γɵļн�
// ����˵��
// ���ز���
// ʹ��ʾ��
// ��ע��Ϣ
//-------------------------------------------------------------------------------------------------------------------
double Angel_compute(int x1, int y1, int x2, int y2, int x3, int y3)
 {
     double dotProduct = (double)((x2 - x1) * (x2 - x3) + (y2 - y1) * (y2 - y3));
     double magProduct =
             (sqrt((double)((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1)))) *
             (sqrt((double)((x2 - x3) * (x2 - x3) + (y2 - y3) * (y2 - y3))));

     // ȷ���� [-1, 1] ��Χ��
     double cos_angle = dotProduct / magProduct;
     cos_angle = cos_angle < -1.0 ? -1.0 : (cos_angle > 1.0 ? 1.0 : cos_angle);
     double radian_angle = acos(cos_angle);
     double degree_angle = radian_angle * 180.0 / M_PI;

     return degree_angle;
 }
//-------------------------------------------------------------------------------------------------------------------
// �������         find_cross_down ��ʮ�����¹յ�
// ����˵��
// ���ز���
// ʹ��ʾ��
// ��ע��Ϣ
//-------------------------------------------------------------------------------------------------------------------
void find_cross_down_l(void)
{
    int x[3]={0},y[3]={0};
    double angle=0;
    for(int i = 1 ;i <= 90 && i < l_data_statics - 12 ; i += 2 )
    {
        x[0] = Find_Boundary_l[i][1];     //a b c ����  �����b�ĽǶ�
        x[1] = Find_Boundary_l[i+6][1];
        x[2] = Find_Boundary_l[i+12][1];

        y[0]=Find_Boundary_l[i][0];
        y[1]=Find_Boundary_l[i+6][0];
        y[2]=Find_Boundary_l[i+12][0];


        if( y[0] > y[2]
            && x[0] >= x[2] &&  x[1] > x[2]
            && x[2] > 2
            && image[(y[0]+y[2])/2][(x[2]+x[0])/2] == 0)//ac�����е�һ���� ��ɫ

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
// �������         find_cross_down ��ʮ�����¹յ�
// ����˵��
// ���ز���
// ʹ��ʾ��
// ��ע��Ϣ
//-------------------------------------------------------------------------------------------------------------------
void find_cross_down_r(void)
{
    int x[3]={0},y[3]={0};
    double angle=0;
    for(int i = 1;i <= 90 && i < r_data_statics - 12; i += 2 )
    {
        x[0] = Find_Boundary_r[i][1];     //a b c ����  �����b�ĽǶ�
        x[1] = Find_Boundary_r[i+6][1];
        x[2] = Find_Boundary_r[i+12][1];

        y[0]=Find_Boundary_r[i][0];
        y[1]=Find_Boundary_r[i+6][0];
        y[2]=Find_Boundary_r[i+12][0];


        if(    y[0] > y[2]
            && x[0] <= x[2] && x[2] > x[1]
            && x[2] < 68
            && image[(y[0]+y[2])/2][(x[2]+x[0])/2] == 0      //ac�����е�һ���� ��ɫ
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
// �������         find_cross_on ��ʮ�����Ϲյ�
// ����˵��
// ���ز���
// ʹ��ʾ��
// ��ע��Ϣ
//-------------------------------------------------------------------------------------------------------------------
void find_cross_on_l(void)
{
    int x[3]={0},y[3]={0};
    double angle=0;
    int i = 3;
    if(l_Down[2]==1)i=l_Down[3];
    for( ;i < l_data_statics - 16 ; i += 2 )
    {
        x[0] = Find_Boundary_l[i][1];     //a b c ����  �����b�ĽǶ�
        x[1] = Find_Boundary_l[i+6][1];
        x[2] = Find_Boundary_l[i+12][1];

        y[0]=Find_Boundary_l[i][0];
        y[1]=Find_Boundary_l[i+6][0];
        y[2]=Find_Boundary_l[i+12][0];


        if( y[0] > y[2] && y[1] > y[2]
            && x[0] < x[2]
            && x[0] > 2
            && image[(y[0]+y[2])/2][(x[2]+x[0])/2] == 0)//ac�����е�һ���� ��ɫ

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
// �������         find_cross_on ��ʮ�� �� �Ϲյ�
// ����˵��
// ���ز���
// ʹ��ʾ��
// ��ע��Ϣ
//-------------------------------------------------------------------------------------------------------------------
void find_cross_on_r(void)
{
    int x[3]={0},y[3]={0};
    double angle=0;
    int i = 3;
    if(r_Down[2]==1)i=r_Down[3];
    for(; i < r_data_statics - 12 ; i += 2 )
        {
        x[0] = Find_Boundary_r[i][1];     //a b c ����  �����b�ĽǶ�
        x[1] = Find_Boundary_r[i+6][1];
        x[2] = Find_Boundary_r[i+12][1];

        y[0]=Find_Boundary_r[i][0];
        y[1]=Find_Boundary_r[i+6][0];
        y[2]=Find_Boundary_r[i+12][0];


        if(    y[0] > y[2] && y[1] > y[2]
            && x[0] > x[2]
            && x[0] < 68
            && image[(y[0]+y[2])/2][(x[2]+x[0])/2] == 0      //ac�����е�һ���� ��ɫ
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
// �������         find_cross_on ��ʮ�����Ϲյ�   ���� �ұ���ɨ������Ϲյ�
// ����˵��
// ���ز���
// ʹ��ʾ��
// ��ע��Ϣ
//-------------------------------------------------------------------------------------------------------------------
void find_incline_cross_on_l(void)
{
    int x[3]={0},y[3]={0};
    double angle=0;
    for(  int i = 3 ;i < r_data_statics - 16 ; i += 2 )
    {
        x[0] = Find_Boundary_r[i][1];     //a b c ����  �����b�ĽǶ�
        x[1] = Find_Boundary_r[i+8][1];
        x[2] = Find_Boundary_r[i+16][1];

        y[0]=Find_Boundary_r[i][0];
        y[1]=Find_Boundary_r[i+8][0];
        y[2]=Find_Boundary_r[i+16][0];


        if( y[1] > y[0] && x[1] > x[2]
            && x[2] < 68
            && image[(y[0]+y[2])/2][(x[2]+x[0])/2] == 0)//ac�����е�һ���� ��ɫ

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
// �������         find_cross_on ��ʮ�����Ϲյ�   ���� �ұ���ɨ������Ϲյ�
// ����˵��
// ���ز���
// ʹ��ʾ��
// ��ע��Ϣ
//-------------------------------------------------------------------------------------------------------------------
void find_incline_cross_on_r(void)
{
    int x[3]={0},y[3]={0};
    double angle=0;
    for(  int i = 3 ;i < l_data_statics - 16 ; i += 2 )
    {
        x[0] = Find_Boundary_l[i][1];     //a b c ����  �����b�ĽǶ�
        x[1] = Find_Boundary_l[i+8][1];
        x[2] = Find_Boundary_l[i+16][1];

        y[0]=Find_Boundary_l[i][0];
        y[1]=Find_Boundary_l[i+8][0];
        y[2]=Find_Boundary_l[i+16][0];


        if( y[1] > y[0] && x[1] < x[2]
            && x[0] > 2
            && image[(y[0]+y[2])/2][(x[2]+x[0])/2] == 0)//ac�����е�һ���� ��ɫ

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
// �������  cross_checkout ʮ�ֲ�����
// ����˵��
// ���ز���
// ʹ��ʾ��
// ��ע��Ϣ
//-------------------------------------------------------------------------------------------------------------------
void cross_checkout(void)
{

    if(cross_flag==1)
    {
        int x1,y1;
        if( (r_Down[2] + l_Down[2] + l_on[2] + r_on[2])==3)   // ����һ
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
// �������   cross_judgment_first ����ʮ���ж� ������еĻ����ǽ���ʮ�ֱ�־λ
// ����˵��
// ���ز���
// ʹ��ʾ��
// ��ע��Ϣ
//-------------------------------------------------------------------------------------------------------------------
void cross_judgment_positive(void)
{
    find_cross_down_l();
    find_cross_down_r();

    if(l_Down[2] == 1 && r_Down[2] == 1)   //Ѱ�ҵ��յ�
       {
           if(  func_abs(  track_width - (int)sqrt( ( l_Down[0] - r_Down[0] )  *( l_Down[0] - r_Down[0] )
                   +( ( l_Down[1] - r_Down[1] )  * ( l_Down[1] - r_Down[1] ) ) ) )<= 3   )//�����������

           {
               cross_flag = 1;  //��������
               lr_en_speed=0;   //������������� ʹ�ý���ڶ��׶�
           }
       }


}
//-------------------------------------------------------------------------------------------------------------------
// �������   cross_judgment_slanting   б��ʮ���ж� ������еĻ����ǽ���ʮ�ֱ�־λ
// ����˵��
// ���ز���
// ʹ��ʾ��
// ��ע��Ϣ
//-------------------------------------------------------------------------------------------------------------------
void cross_judgment_slanting(void)
{

    find_cross_down_l();
    find_cross_down_r();

    find_incline_cross_on_l();
    find_incline_cross_on_r();

    if(  l_on[2]==1 && l_Down[2]==1
      && func_abs(  track_width - (int)sqrt( ( l_on[0] - l_Down[0] )  *( l_on[0] - l_Down[0] )
        +( ( l_on[1] - l_Down[1] )  * ( l_on[1] - l_Down[1] ) ) ) )<= 5   )//�����������
    {
        cross_flag=4;
        lr_en_speed=0;
    }
    else if(  r_on[2]==1 && r_Down[2]==1
          && func_abs(  track_width - (int)sqrt( ( r_on[0] - r_Down[0] )  *( r_on[0] - r_Down[0] )
            +( ( r_on[1] - r_Down[1] )  * ( r_on[1] - r_Down[1] ) ) ) )<= 5 )//�����������
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
// �������   cross_dispose  ����ʮ���ж�
// ����˵��
// ���ز���
// ʹ��ʾ��
// ��ע��Ϣ
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
        if( lr_en_speed >= 11000 )   //����������ǿ�����
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


    else if(cross_flag==4) //б��ʮ��
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
        if( lr_en_speed >= 4300 )   //����������ǿ�����
        {
            cross_flag = 2;
            find_cross_on_l();
            find_cross_on_r();
            lr_en_speed = 0;
        }
    }




//��ʼʮ�ֲ���-------------------------------------------------------------

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
// �������      �����ж�
// ����˵��
// ���ز���
// ʹ��ʾ��
// ��ע��Ϣ
//-------------------------------------------------------------------------------------------------------------------
void find_island_on_l(void)
{
    int x[3]={0},y[3]={0};
    double angle=0;
    for( int i = 3;i < l_data_statics - 16 ; i += 2 )
    {
        x[0] = Find_Boundary_l[i][1];     //a b c ����  �����b�ĽǶ�
        x[1] = Find_Boundary_l[i+8][1];
        x[2] = Find_Boundary_l[i+16][1];

        y[0]=Find_Boundary_l[i][0];
        y[1]=Find_Boundary_l[i+8][0];
        y[2]=Find_Boundary_l[i+16][0];

        if( x[0] > 2 && x[2] > x[0]
            && image[(y[0]+y[2])/2][(x[2]+x[0])/2] == 0)//ac�����е�һ���� ��ɫ
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
// �������      �����ж�
// ����˵��
// ���ز���
// ʹ��ʾ��
// ��ע��Ϣ
//-------------------------------------------------------------------------------------------------------------------
void find_island_on_r(void)
{
    int x[3]={0},y[3]={0};
    double angle=0;
    for( int i = 3;i < r_data_statics - 16 ; i += 2 )
    {
        x[0] = Find_Boundary_r[i][1];     //a b c ����  �����b�ĽǶ�
        x[1] = Find_Boundary_r[i+8][1];
        x[2] = Find_Boundary_r[i+16][1];

        y[0]=Find_Boundary_r[i][0];
        y[1]=Find_Boundary_r[i+8][0];
        y[2]=Find_Boundary_r[i+16][0];





        if( x[0] < 67 && x[0] > x[2]
            && image[(y[0]+y[2])/2][(x[2]+x[0])/2] == 0)//ac�����е�һ���� ��ɫ

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
// �������      �����ж�
// ����˵��
// ���ز���
// ʹ��ʾ��
// ��ע��Ϣ
//-------------------------------------------------------------------------------------------------------------------
void find_island_incline_on_l(void)
{
    int x[3]={0},y[3]={0};
    double angle=0;
    for( int i = 3;i < r_data_statics - 16 ; i += 2 )
    {
        x[0] = Find_Boundary_r[i][1];     //a b c ����  �����b�ĽǶ�
        x[1] = Find_Boundary_r[i+8][1];
        x[2] = Find_Boundary_r[i+16][1];

        y[0]=Find_Boundary_r[i][0];
        y[1]=Find_Boundary_r[i+8][0];
        y[2]=Find_Boundary_r[i+16][0];


        if( x[0] < 68 && x[0] > x[2]
            && image[(y[0]+y[2])/2][(x[2]+x[0])/2] == 0)//ac�����е�һ���� ��ɫ

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
// �������      �����ж�
// ����˵��
// ���ز���
// ʹ��ʾ��
// ��ע��Ϣ
//-------------------------------------------------------------------------------------------------------------------
void find_island_incline_on_r(void)
{
    int x[3]={0},y[3]={0};
    double angle=0;
    for( int i = 3;i < l_data_statics - 16; i += 2 )
    {
        x[0] = Find_Boundary_l[i][1];     //a b c ����  �����b�ĽǶ�
        x[1] = Find_Boundary_l[i+8][1];
        x[2] = Find_Boundary_l[i+16][1];

        y[0]=Find_Boundary_l[i][0];
        y[1]=Find_Boundary_l[i+8][0];
        y[2]=Find_Boundary_l[i+16][0];


        if( x[0] > 1 && x[2] > x[0]
            && image[(y[0]+y[2])/2][(x[2]+x[0])/2] == 0)//ac�����е�һ���� ��ɫ

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
// �������      �����ж�
// ����˵��
// ���ز���
// ʹ��ʾ��
// ��ע��Ϣ
//-------------------------------------------------------------------------------------------------------------------
void find_island_down_l(void)
{
    int x[3]={0},y[3]={0};
    double angle=0;
    for( int i = 3;i < r_data_statics -16 ; i += 2 )
    {
        x[0] = Find_Boundary_r[i][1];     //a b c ����  �����b�ĽǶ�
        x[1] = Find_Boundary_r[i+8][1];
        x[2] = Find_Boundary_r[i+16][1];

        y[0]=Find_Boundary_r[i][0];
        y[1]=Find_Boundary_r[i+8][0];
        y[2]=Find_Boundary_r[i+16][0];


        if( x[0] < 68 && y[2] < y[0] && x[2] > x[1]
            && image[(y[0]+y[2])/2][(x[2]+x[0])/2] == 0)//ac�����е�һ���� ��ɫ

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
// �������      �����ж�
// ����˵��
// ���ز���
// ʹ��ʾ��
// ��ע��Ϣ
//-------------------------------------------------------------------------------------------------------------------
void find_island_down_r(void)
{
    int x[3]={0},y[3]={0};
    double angle=0;
    for( int i = 3;i < l_data_statics -16  ; i += 2 )
    {
        x[0] = Find_Boundary_l[i][1];     //a b c ����  �����b�ĽǶ�
        x[1] = Find_Boundary_l[i+8][1];
        x[2] = Find_Boundary_l[i+16][1];

        y[0]=Find_Boundary_l[i][0];
        y[1]=Find_Boundary_l[i+8][0];
        y[2]=Find_Boundary_l[i+16][0];


        if( x[0] > 1
           && y[2] < y[0]
           && x[1] > x[2]
           && x[1] > x[0]
           && image[(y[0]+y[2])/2][(x[2]+x[0])/2] == 0)//ac�����е�һ���� ��ɫ

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
// �������      ������ȼ��
// ����˵��      Ҫ������������
// ���ز���      ��ɫ����
// ʹ��ʾ��
// ��ע��Ϣ
//-------------------------------------------------------------------------------------------------------------------
uint8 track_judgment(uint8 i)
{
    uint8 track_widch=0; //������ȼ������
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
// �������      �����ж�
// ����˵��
// ���ز���
// ʹ��ʾ��
// ��ע��Ϣ
//-------------------------------------------------------------------------------------------------------------------
void island_judgment(void)
{
    if(straight_r >=2 && straight_l == 0 )
    {
        find_island_on_l();
        if(is_L_on[2]==1
                && is_L_on[0] >= island_in_limit_x       //�߶ȴ���ĳ��ֵ�Ž�����
                && is_L_on[0] <= island_in_limit_x +10
                && track_judgment(is_L_on[0]+3)>track_width+10
                                                                )    //����Ѱ�ҹյ� ���� �յ�߶�

        {
            L_island_flag=1;
            if(g_attitude.yaw + 70 >= 180)
            angle.yaw = g_attitude.yaw + 70 -360.0;   //�뻷��ʼ�Ƕ�
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
                && track_judgment(is_R_on[0]+3)>track_width+10)  //����Ѱ�ҹյ� ���� �յ�߶�
        {
            R_island_flag=1;
            if(g_attitude.yaw - 70 <= -180)
            angle.yaw = g_attitude.yaw - 70 +360.0;   //�뻷��ʼ�Ƕ�
            else
            angle.yaw = g_attitude.yaw - 70;
        }
        else  R_island_flag=0;
    }
}


//-------------------------------------------------------------------------------------------------------------------
// �������      �����ж�
// ����˵��
// ���ز���
// ʹ��ʾ��
// ��ע��Ϣ
//-------------------------------------------------------------------------------------------------------------------
float f_abs(float x)
{
    if(x>=0) return x ;
    else return -x;
}
void island_dispose_L(void)
{
//    buzzer=10;


    if(L_island_flag == 1 )  //�뻷�����Ϲյ�
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
            angle.yaw = g_attitude.yaw + 180 -360.0;   //�뻷��ʼ�Ƕ�
            else
            angle.yaw = g_attitude.yaw + 180;
        }
        lr_en_speed=0;

    }
    else  if(L_island_flag == 2){//������ת �ҽǶ�
        if(f_abs(g_attitude.yaw - angle.yaw)<10.0){
            L_island_flag = 3;
            find_island_down_l();
            if(g_attitude.yaw + 100 >=180)
            angle.yaw = g_attitude.yaw + 100 -360.0;
            else
            angle.yaw = g_attitude.yaw + 100;
        }

    }
    else if(L_island_flag == 3){ //Ѱ�����¹յ�
        find_island_down_l();
        if(f_abs(g_attitude.yaw - angle.yaw)<10.0){
            lr_en_speed = 0;
            L_island_flag = 4;
            find_island_on_l();
        }
    }
    else if( L_island_flag == 4) {//�����ϳ���
        find_island_on_l();
        if(lr_en_speed >= 9000
//                && lostline_flag_l == 0 && lostline_flag_r == 0
                && abs(track_judgment(55)-track_width)<=3)
        L_island_flag=0;
    }

//��ʼ��������-------------------------------------------------------------


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
// �������      �����ж�
// ����˵��
// ���ز���
// ʹ��ʾ��
// ��ע��Ϣ
//-------------------------------------------------------------------------------------------------------------------
void island_dispose_R(void)
{
//    buzzer=10;

    if(R_island_flag == 1 )  {//�뻷�����Ϲյ�
        find_island_on_r();
        if(is_R_on[2]==0){
            find_island_incline_on_r();
        }

        if(f_abs(g_attitude.yaw - angle.yaw)<10.0){
            R_island_flag = 2;
            if(g_attitude.yaw - 180 <= -180)  //���б仯�Ƕ�
            angle.yaw = g_attitude.yaw - 180 +360.0;   //�뻷��ʼ�Ƕ�
            else
            angle.yaw = g_attitude.yaw - 180;
        }
    }
    else  if(R_island_flag == 2) { //������ת �ҽǶ�
        if(f_abs(g_attitude.yaw - angle.yaw)<10.0  ){// �����Ͻǳ��� ����
            R_island_flag = 3;
            find_island_down_r();
            if(g_attitude.yaw - 100 <= -180)
            angle.yaw = g_attitude.yaw - 100 +360.0;
            else
            angle.yaw = g_attitude.yaw - 100;
        }
    }
    else if(R_island_flag == 3)  {//Ѱ�����¹յ�
        find_island_down_r();
        if(f_abs(g_attitude.yaw - angle.yaw)<10.0){
            lr_en_speed = 0;
            R_island_flag = 4;
            find_island_on_r();
        }
    }
    else if( R_island_flag == 4){//�����ϳ���
        find_island_on_r();
        if(lr_en_speed >= 9000
                && lostline_flag_l == 0 && lostline_flag_r == 0)
            R_island_flag=0;
    }







//��ʼ��������-------------------------------------------------------------

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
// �������      �µ����
// ����˵��
// ���ز���
// ʹ��ʾ��
// ��ע��Ϣ
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
// �������     �������ݼ�¼�뷵�س���Ƕȵ���
// ����˵��     mode �������ݵ�����  0-����������ƫ������ڵ�·��࣬�������ڵ�·�Ҳ� 1-����Ƕ�ƫ��
// ���ز���     Ѳ�߽Ƕȼ�����Ƕ�����
// ʹ��ʾ��     int angle = midline_and_anglereturn();
// ��ע��Ϣ
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
// �������     midline_scan()   ����ɨ��
// ����˵��
// ���ز���
// ʹ��ʾ��
// ��ע��Ϣ
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
// �������      �������
// ����˵��
// ���ز���
// ʹ��ʾ��
// ��ע��Ϣ
//-------------------------------------------------------------------------------------------------------------------
//void centre_



void Inflexion_Point(void)  //�ĸ��յ�  ��ʾ
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
// �������    Camera_tracking()   ͼ�����ܺ���
// ����˵��
// ���ز���
// ʹ��ʾ��
// ��ע��Ϣ
//-------------------------------------------------------------------------------------------------------------------
void Camera_tracking(void)
{

    empty_flag(); //��־λ���
    Binaryzation(); //�������ֵ
    image_draw();  //��ֵ�� ���� ȥ���
    shade_compute();//�����½���Ӱ
    search((uint16)USE_num,image,&hight_image);
//�жϱ�֡�Ƿ������� ��������� �Ͳ�����Ԫ���ж�
    if(image_ok == 1){
//ֱ�� ��� �ж�-----------------------------
        curvature_judge();
//        curvature_l_l=calculateAverageCurvature(Find_Boundary_l,l_data_statics,20,10);
//���Ҷ����ж�-----------------------------
        lost_line_judge();
//Ԫ���ж�-----------------------------------
        if(zebra_crossing_flag==0 && L_island_flag==0 && R_island_flag == 0 && ramp_flag==0 ){
            if(cross_flag == 0 && lostline_flag_l == 1 && lostline_flag_r == 1){//����ʮ��
                cross_judgment_positive();
            }
            if(cross_flag == 0 && lostline_flag_l == 1 && lostline_flag_r == 1){//б��ʮ��
                cross_judgment_slanting();
            }
            if(cross_flag !=0){ //�����־λ����ʮ�ֲ�ͬ�Ĺ���
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
// �������     ��ʱ����ȡ����ٶȣ����ռ�ö�ʱ��5���Ҳ�ռ�ö�ʱ��4��
// ����˵��
// ���ز���
// ʹ��ʾ��
// ��ע��Ϣ
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
                 || (Motor_pid_l.Out>=4000 && l_en_speed <= speed_l*0.3))){// ��ת����
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
//     else if(buzzer==0)         gpio_set_level(P33_10, 0);   //������ ֹͣ����
}
