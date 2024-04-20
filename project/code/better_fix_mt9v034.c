/*
 * better_fix_mt9v034.c
 *
 *  Created on: 2024��3��17��
 *      Author: pc
 */
#include "zf_common_headfile.h"
uint8 ex_mt9v03x_binarizeImage[MT9V03X_H/2][MT9V03X_W/2];   //����ͷ��ֵ�����ݴ洢����

int ImageScanInterval=6;                                    //ɨ�ߵķ�Χ

int ex_threshold;                                         //��򷨶�ֵ����ֵ

uint8 Ring_Help_Flag = 0;                                   //����������־

int Right_RingsFlag_Point1_Ysite, Right_RingsFlag_Point2_Ysite; //��Բ���жϵ�����������
int Left_RingsFlag_Point1_Ysite, Left_RingsFlag_Point2_Ysite;   //��Բ���жϵ�����������
int Point_Xsite,Point_Ysite;                                //�յ��������
int Repair_Point_Xsite,Repair_Point_Ysite;                  //���ߵ��������

uint8 ExtenLFlag = 0;                                       //������Ƿ���Ҫ���ߵı�־����
uint8 ExtenRFlag = 0;                                       //�ұ����Ƿ���Ҫ���ߵı�־����
uint16 wide_sum;//////////////////
static float BottomBorderRight = 79,              //59�е��ұ߽�
BottomBorderLeft = 0,                           //59�е���߽�
Bottommidline = 0;                               //59�е��е�
//uint8 Garage_Location_Flag = 0;                 //�жϿ�Ĵ���
//
//uint16 Ramp_num,Zebra_num,Garage_length,mode;
//int16 Garage_num=0;
//int32 Element_encoder1 = 0,Element_encoder2 = 0;//�µ��������ߡ��������������

static float DetR = 0, DetL = 0;                            //��Ų���б�ʵı���
static int IntervalLow = 0, IntervalHigh = 0;               //ɨ������������ޱ���
static int TFSite = 0, left_FTSite = 0,right_FTSite = 0;    //���߼���б�ʵ�ʱ����Ҫ�õĴ���еı�����
static int ytemp = 0;                                       //����е���ʱ����
static uint8* Pixel;                                        //һ�����浥��ͼ���ָ��

Sideline_status Sideline_status_array[60];

Image_Status imagestatus;
Image_Flag imageflag;
ChangePoint changepoint={0,0};
uint8 Half_Road_Wide[60] =                      //ֱ���������
{  4, 5, 5, 6, 6, 6, 7, 7, 8, 8,
   9, 9,10,10,10,11,12,12,13,13,
  13,14,14,15,15,16,16,17,17,17,
  18,18,19,19,20,20,20,21,21,22,
  23,23,23,24,24,25,25,25,26,26,
  27,28,28,28,29,30,31,31,31,32,
};
uint8 Half_Bend_Wide[60] =                      //����������
{   33,33,33,33,33,33,33,33,33,33,
    33,33,32,32,30,30,29,29,28,27,
    28,27,27,26,26,25,25,24,24,23,
    22,21,21,22,22,22,23,24,24,24,
    25,25,25,26,26,26,27,27,28,28,
    28,29,29,30,30,31,31,32,32,33,
};
/*****************ֱ���ж�******************/
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
    }
    return S;
}

void Straight_long_judge(void)     //���ؽ��С��1��Ϊֱ��
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

void Straight_long_handle(void)     //���ؽ��С��1��Ϊֱ��
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
//���ڱ������ֱ�����
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


/*****************��������ж�******************/
uint32 break_road(uint8 line_start)//�������
{
    short i,j;
    int bai=0;
    for(i=58;i>line_start;i--)
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
uint32 white_point(uint8 line_end,uint8 line_start) //�׵�����
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
uint32 black_point(uint8 line_end,uint8 line_start) //�ڵ�����
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
// �������     �õ����߼����ߵ�����
// ����˵��             p        ָ�򴫽��������һ��ָ�������
// ����˵��            type     ֻ����L������R���ֱ����ɨ����ߺ�ɨ�ұ��ߡ�
// ����˵��             L        ɨ����������� ��Ҳ���Ǵ���һ�п�ʼɨ��
// ����˵��             H        ɨ����������� ��Ҳ����һֱɨ����һ�С�
// ����˵��             Q        ��һ���ṹ��ָ��������Լ�����ȥ��������ṹ������ĳ�Ա��
// ʹ��ʾ��     Get_Border_And_SideType(Pixel, 'R', IntervalLow, IntervalHigh,&jumppoint[1])
// ��ע��Ϣ     ��Pixel(Pixel�Ǹ�ָ�룬ָ��һ������)��IntervalLow�п�ʼɨ��ɨ��IntervalHigh�У�Ȼ��ѵõ��ı������ڵ��кͱ������ͼ�¼��JumpPoint�ṹ���С�
//-----------------------------------------------------------------------------------------------------------------
void Get_Border_And_SideType(uint8* p,uint8 type,int L,int H,Jumppoint* Q)
{
  int i = 0;
  if (type == 'L')                              //���Type��L(Left),��ɨ������ߡ�
  {
    for (i = H; i >= L; i--)                    //��������ɨ
    {
      if (*(p + i) == 255 && *(p + i - 1) != 255)   //����кڰ�����    1�ǰ� 0�Ǻ�
      {
        Q->point = i;                           //�ǾͰ�����м�¼������Ϊ�����
        Q->type = 'T';                          //���Ұ���һ�е������������䣬�������ͼ�ΪT��������������
        break;                                  //�ҵ��˾�����ѭ��������
      }
      else if (i == (L + 1))                    //Ҫ��ɨ�����û�ҵ�
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
    for (i = L; i <= H; i++)                    //��������ɨ
    {
      if (*(p + i) == 255 && *(p + i + 1) != 255)   //����кڰ�����    1�ǰ� 0�Ǻ�
      {
        Q->point = i;                           //�ǾͰ�����м�¼������Ϊ�ұ���
        Q->type = 'T';                          //���Ұ���һ�е������������䣬�������ͼ�ΪT��������������
        break;                                  //�ҵ��˾�����ѭ��������
      }
      else if (i == (H - 1))                    //Ҫ��ɨ�����û�ҵ�
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

//----------------------------------------------------------------------------------------------------------------------
// �������     ͨ����򷨼����ֵ����ֵ
// ʹ��ʵ�� ��int bestThreshold = otsuThreshold(inputImage);
// ��ע��Ϣ     ͨ����򷨳����Ż��Ķ�ֵ����ֵ�㷨���������иĽ��ռ䣬���ڶ�ʱ���У���ʱִ��(�ݲ������Ѷ�ʱʱ�䣬����2ms)
//----------------------------------------------------------------------------------------------------------------------

int otsuThreshold(const uint8 inputImage[MT9V03X_H][MT9V03X_W])
{
    int histogram[256] = {0};
    float sum = 0.0;
    float sumB = 0.0;
    int wB = 0;
    int wF = 0;
    float varMax = 0.0;
    int threshold = 0;

    // ��������ֵ��ֱ��ͼ
    for (int i = 0; i < MT9V03X_H; i++)
    {
        for (int j = 0; j < MT9V03X_W; j++)
        {
            histogram[inputImage[i][j]]++;
        }
    }

    // �����ܺ�
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
// �������     ������ͷ���ݽ��ж�ֵ���󴢴浽�µĶ�ά������
// ʹ��ʵ��     binarizeImage(mt9v03x_image,ex_mt9v03x_binarizeImage)
//---------------------------------------------------------------------------------------
//void binarizeImage(const uint8 inputImage[MT9V03X_H][MT9V03X_W], uint8 outputImage[MT9V03X_H][MT9V03X_W])
//{
//    int THRESHOLD=otsuThreshold(mt9v03x_image);
//    for (int i = 0; i < MT9V03X_H; i++) {
//        for (int j = 0; j < MT9V03X_W; j++) {
//            if (inputImage[i][j] > THRESHOLD) {
//                outputImage[i][j] = 255; // ����Ϊ��ɫ
//            } else {
//                outputImage[i][j] = 0;   // ����Ϊ��ɫ
//            }
//        }
//    }
//}
void binarizeImage(void)
{
    if(mt9v03x_finish_flag)
         {
         int THRESHOLD=ex_threshold+10;
//         if(THRESHOLD<170)THRESHOLD=170;

         for (int i = 0; i < MT9V03X_H; i+=2) {
             for (int j = 0; j < MT9V03X_W; j+=2) {
                 if (mt9v03x_image[i][j] > THRESHOLD) {
                     ex_mt9v03x_binarizeImage[i/2][j/2] = 255; // ����Ϊ��ɫ
                 } else {
                     ex_mt9v03x_binarizeImage[i/2][j/2] = 0;   // ����Ϊ��ɫ
                 }
             }
         }
         }

}
//---------------------------------------------------------------------------------------
// �������     �Զ�ֵ�����ݽ��н��봦�����ڴ�ռ�ù�����ʱ������ʹ�ã�
//---------------------------------------------------------------------------------------
//void medianFilter(uint8 binaryImage[IMAGE_HEIGHT][IMAGE_WIDTH]) {
//    uint8 tempImage[IMAGE_HEIGHT][IMAGE_WIDTH];
//
//    for (int row = 1; row < IMAGE_HEIGHT - 1; row += 2) {
//        for (int col = 1; col < IMAGE_WIDTH - 1; col += 2) {
//            // ��ȡ3x3������ֵ
//            uint8 pixels[9] = {
//                binaryImage[row-1][col-1], binaryImage[row-1][col], binaryImage[row-1][col+1],
//                binaryImage[row][col-1], binaryImage[row][col], binaryImage[row][col+1],
//                binaryImage[row+1][col-1], binaryImage[row+1][col], binaryImage[row+1][col+1]
//            };
//
//            // ������ֵ��������
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
//            // ȡ��ֵ��Ϊ��ǰ���ص�ֵ
//            tempImage[row][col] = pixels[4];
//        }
//    }
//
//    // ���������ͼ�����ݿ�����ԭ����
//    for (int i = 0; i < IMAGE_HEIGHT; i++) {
//        for (int j = 0; j < IMAGE_WIDTH; j++) {
//            binaryImage[i][j] = tempImage[i][j];
//        }
//    }
//}
//--------------------------------------------------------------------------------------------------------------------------
// �������     ��־λ��ʼ��
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
// �������    ����Ѱ���޸���(�ױ�Ѱ��)
//--------------------------------------------------------------------------------------------------------------------------
void baseline_get(void)
{
    /*������еױ����ߣ����ߵ�����*/

    Pixel=ex_mt9v03x_binarizeImage[59];
        for(uint8 i=45;i<MT9V03X_W/2-2;i++)
            {
                if (*(Pixel + i) == 0 && *(Pixel + i + 1) == 0)       //������������������ڵ㣬˵û�ҵ��˱��ߡ�
                {
                    BottomBorderRight = i;                                      //����һ�м�¼������Ϊ��һ�е��ұ���
                    break;                                                          //����ѭ��
                }
                else if (i == MT9V03X_W/2-3)                                             //����ҵ��˵�58�ж���û���ֺڵ㣬˵����һ�еı��������⡣
                {
                    BottomBorderRight = MT9V03X_W/2-3;                                         //����������Ĵ������ǣ�ֱ�Ӽ���ͼ�����ұߵ���һ�У���79�У�������һ�е��ұ��ߡ�
                    break;                                                          //����ѭ��
                }
            }
        for(uint8 i=45;i>0;i--)
            {
                if (*(Pixel + i) == 0 && *(Pixel + i - 1) == 0)       //������������������ڵ㣬˵û�ҵ��˱��ߡ�
                {
                    BottomBorderLeft = i;                                      //����һ�м�¼������Ϊ��һ�е��ұ���
                    break;                                                          //����ѭ��
                }
                else if (i == MT9V03X_W/2-3)                                             //����ҵ��˵�58�ж���û���ֺڵ㣬˵����һ�еı��������⡣
                {
                    BottomBorderLeft = 0;                                         //����������Ĵ������ǣ�ֱ�Ӽ���ͼ�����ұߵ���һ�У���79�У�������һ�е��ұ��ߡ�
                    break;                                                          //����ѭ��
                }
            }


        Bottommidline =(BottomBorderLeft + BottomBorderRight) / 2.0;
        Sideline_status_array[59].leftline = BottomBorderLeft;                        //�ѵ�59�е���߽�洢�����飬ע�⿴Sideline_status_array������ֵ��±꣬�ǲ������ö�Ӧ59��
        Sideline_status_array[59].rightline = BottomBorderRight;                      //�ѵ�59�е��ұ߽�洢�����飬ע�⿴Sideline_status_array������ֵ��±꣬�ǲ������ö�Ӧ59��
        Sideline_status_array[59].midline=Bottommidline;
        Sideline_status_array[59].wide=(BottomBorderRight-BottomBorderLeft);
        Sideline_status_array[59].De_m=(Bottommidline-(MT9V03X_W/4-2));
        Sideline_status_array[59].IsLeftFind = 'T';                                     //��¼��59�е����������ΪT���������ҵ�����ߡ�
        Sideline_status_array[59].IsRightFind = 'T';                                    //��¼��59�е��ұ�������ΪT���������ҵ��ұ��ߡ�
        /*�����������ĵױ����ߣ�ɨ��58��54��*/
    for(uint8 j=58;j>54;j--)
    {
        Pixel=ex_mt9v03x_binarizeImage[j];
    for(uint8 i=Sideline_status_array[j+1].midline;i<MT9V03X_W/2-2;i++)
        {
            if (*(Pixel + i) == 0 && *(Pixel + i + 1) == 0)       //������������������ڵ㣬˵û�ҵ��˱��ߡ�
            {
                BottomBorderRight = i;                                      //����һ�м�¼������Ϊ��һ�е��ұ���
                break;                                                          //����ѭ��
            }
            else if (i == MT9V03X_W/2-1)                                             //����ҵ��˵�58�ж���û���ֺڵ㣬˵����һ�еı��������⡣
            {
                BottomBorderRight = MT9V03X_W/2-2;                                         //����������Ĵ������ǣ�ֱ�Ӽ���ͼ�����ұߵ���һ�У���79�У�������һ�е��ұ��ߡ�
                break;                                                          //����ѭ��
            }
        }
    for(uint8 i=Sideline_status_array[i+1].midline;i>0;i--)
        {
            if (*(Pixel + i) == 0 && *(Pixel + i - 1) == 0)       //������������������ڵ㣬˵û�ҵ��˱��ߡ�
            {
                BottomBorderLeft = i;                                      //����һ�м�¼������Ϊ��һ�е��ұ���
                break;                                                          //����ѭ��
            }
            else if (i == MT9V03X_W/2-2)                                             //����ҵ��˵�58�ж���û���ֺڵ㣬˵����һ�еı��������⡣
            {
                BottomBorderLeft = 0;                                         //����������Ĵ������ǣ�ֱ�Ӽ���ͼ�����ұߵ���һ�У���79�У�������һ�е��ұ��ߡ�
                break;                                                          //����ѭ��
            }
        }
    Bottommidline =(BottomBorderLeft + BottomBorderRight) / 2;
    Sideline_status_array[j].midline=Bottommidline;
    Sideline_status_array[j].leftline=BottomBorderLeft;
    Sideline_status_array[j].rightline=BottomBorderRight;
    Sideline_status_array[j].wide=(BottomBorderRight-BottomBorderLeft);
    Sideline_status_array[j].De_m=(Bottommidline-(MT9V03X_W/4-2));
    Sideline_status_array[j].IsLeftFind = 'T';                                     //��¼��59�е����������ΪT���������ҵ�����ߡ�
    Sideline_status_array[j].IsRightFind = 'T';
    }

}
//-------------------------------------------------------------------------------------------------------------------------
// �������     �����еı��߼�¼
//-------------------------------------------------------------------------------------------------------------------------
void allline_get(void)
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

    for(uint8 i=54;i>imagestatus.OFFLine;i--)
    {
        Pixel=ex_mt9v03x_binarizeImage[i];
        Jumppoint jumppoint[2];

        /******************************ɨ�豾�е��ұ���******************************/
        IntervalLow  = Sideline_status_array[i + 1].rightline - ImageScanInterval;       //����һ�е��ұ��߼Ӽ�Interval��Ӧ���п�ʼɨ�豾�У�Intervalһ��ȡ5����Ȼ��Ϊ�˱���������԰����ֵ�ĵĴ�һ�㡣
        IntervalHigh = Sideline_status_array[i + 1].rightline + ImageScanInterval;       //���������ֻ��Ҫ�����б�������5�Ļ����ϣ����10�е�������䣩ȥɨ�ߣ�һ������ҵ����еı����ˣ��������ֵ��ʵ����̫��
        LimitL(IntervalLow);                                                             //������ǶԴ���Get_Border_And_SideType()������ɨ���������һ���޷�������
        LimitH(IntervalHigh);                                                            //������һ�еı����ǵ�2�У�����2-5=-3��-3�ǲ��Ǿ�û��ʵ�������ˣ���ô����-3���أ�
        Get_Border_And_SideType(Pixel, 'R', IntervalLow, IntervalHigh,&jumppoint[1]);    //ɨ���õ�һ���Ӻ������Լ�����ȥ�������߼���
        /******************************ɨ�豾�е��ұ���******************************/

        /******************************ɨ�豾�е������******************************/
        IntervalLow =Sideline_status_array[i + 1].leftline  -ImageScanInterval;          //����һ�е�����߼Ӽ�Interval��Ӧ���п�ʼɨ�豾�У�Intervalһ��ȡ5����Ȼ��Ϊ�˱���������԰����ֵ�ĵĴ�һ�㡣
        IntervalHigh =Sideline_status_array[i + 1].leftline +ImageScanInterval;          //���������ֻ��Ҫ�����б�������5�Ļ����ϣ����10�е�������䣩ȥɨ�ߣ�һ������ҵ����еı����ˣ��������ֵ��ʵ����̫��
        LimitL(IntervalLow);                                                             //������ǶԴ���Get_Border_And_SideType()������ɨ���������һ���޷�������
        LimitH(IntervalHigh);                                                            //������һ�еı����ǵ�2�У�����2-5=-3��-3�ǲ��Ǿ�û��ʵ�������ˣ���ô����-3���أ�
        Get_Border_And_SideType(Pixel, 'L', IntervalLow, IntervalHigh,&jumppoint[0]);    //ɨ���õ�һ���Ӻ������Լ�����ȥ�������߼���
        /******************************ɨ�豾�е����*******************************/

        /*
                       ����Ĵ���Ҫ���뿴�����������ҵ����ڸ���ħ�Ļ���
                        ����ذ�GetJumpPointFromDet()����������߼�������
                        ������������������棬��T������W������H��������־����ʲô��
                        һ��Ҫ�㶮!!!��Ȼ�Ļ������鲻Ҫ���¿��ˣ���Ҫ��ĥ�Լ�!!!
        */
        //������е���������ڲ��������䣬����10���㶼�ǰ׵㡣
        if (jumppoint[0].type =='W')                                                                     //������е���������ڲ��������䣬����10���㶼�ǰ׵㡣
        {
            Sideline_status_array[i].leftline =Sideline_status_array[i + 1].leftline;                  //��ô���е�����߾Ͳ�����һ�еı��ߡ�
        }
        else                                                                                             //������е����������T������H���
        {
            Sideline_status_array[i].leftline = jumppoint[0].point;                                  //��ôɨ�赽�ı����Ƕ��٣��Ҿͼ�¼�����Ƕ��١�
        }

         if (jumppoint[1].type == 'W')                                                                //������е��ұ������ڲ��������䣬����10���㶼�ǰ׵㡣
         {
             Sideline_status_array[i].rightline =Sideline_status_array[i + 1].rightline;            //��ô���е��ұ��߾Ͳ�����һ�еı��ߡ�
         }
         else                                                                                         //������е��ұ�������T������H���
         {
             Sideline_status_array[i].rightline = jumppoint[1].point;                                 //��ôɨ�赽�ı����Ƕ��٣��Ҿͼ�¼�����Ƕ��١�
         }

         Sideline_status_array[i].IsLeftFind =jumppoint[0].type;                                  //��¼�����ҵ�����������ͣ���T����W������H��������ͺ��������õģ���Ϊ�һ�Ҫ��һ��������
         Sideline_status_array[i].IsRightFind = jumppoint[1].type;                                //��¼�����ҵ����ұ������ͣ���T����W������H��������ͺ��������õģ���Ϊ�һ�Ҫ��һ��������
         /*
                            ����Ϳ�ʼ��W��H���͵ı��߷ֱ���д����� ΪʲôҪ������
                            ����㿴����GetJumpPointFromDet�����߼���������T W H�������ͷֱ��Ӧʲô�����
                            �����Ӧ��֪��W��H���͵ı��߶����ڷ��������ͣ������ǲ���Ҫ������
                            ��һ���ֵĴ���˼·��Ҫ�Լ�������ʱ��úõ�ȥ��ĥ������ע������û������˵����ġ���
                            ʵ���벻ͨ�������Ұɣ�
          */

            /************************************����ȷ��������(��H��)�ı߽�*************************************/

         if (( Sideline_status_array[i].IsLeftFind == 'H' || Sideline_status_array[i].IsRightFind == 'H'))
             {
               /**************************��������ߵĴ�����***************************/
               if (Sideline_status_array[i].IsLeftFind == 'H')
               {
                 for (uint8 j = (Sideline_status_array[i].leftline + 1);j <= (Sideline_status_array[i].rightline - 1);j++)                                                           //���ұ���֮������ɨ��
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
               /**************************��������ߵĴ�����***************************/


               /**************************�����ұ��ߵĴ�����***************************/
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
            /**************************�����ұ��ߵĴ�����***************************/

          /*****************************����ȷ��������ı߽�******************************/

         /************************************����ȷ���ޱ��У���W�ࣩ�ı߽�****************************************************************/
             int ysite = 0;
             uint8 L_found_point = 0;
             uint8 R_found_point = 0;
             /**************************��������ߵ��ޱ���***************************/
             if (Sideline_status_array[i].IsRightFind == 'W' && i > 10 && i < 50)
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
                   Sideline_status_array[i].rightline =Sideline_status_array[ytemp_W_R].rightline -D_R * (ytemp_W_R - i);  //����ҵ��� ��ô�Ի�׼�����ӳ���
               }
               LimitL(Sideline_status_array[i].rightline);  //�޷�
               LimitH(Sideline_status_array[i].rightline);  //�޷�
             }
             /**************************�����ұ��ߵ��ޱ���***************************/


             /**************************��������ߵ��ޱ���***************************/
             if (Sideline_status_array[i].IsLeftFind == 'W' && i > 10 && i < 50 )
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
                 if (L_found_point > 8)              //�ҵ���׼б�ʱ�  ���ӳ�������ȷ���ޱ�
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

               LimitL(Sideline_status_array[i].leftline);  //�޷�
               LimitH(Sideline_status_array[i].leftline);  //�޷�
             }

             /**************************��������ߵ��ޱ���***************************/

             /************************************����ȷ���ޱ��У���W�ࣩ�ı߽�****************************************************************/
             /************************************��������֮��������һЩ������������*************************************************/
                  if (Sideline_status_array[i].IsLeftFind == 'W' && Sideline_status_array[i].IsRightFind == 'W')
                  {
                      imagestatus.WhiteLine++;  //Ҫ�����Ҷ��ޱߣ�������+1
                  }
                 if (Sideline_status_array[i].IsLeftFind == 'W' && i < 55 )
                 {
                     imagestatus.Miss_Left_lines++;
                 }
                 if (Sideline_status_array[i].IsRightFind == 'W'&& i < 55)
                 {
                     imagestatus.Miss_Right_lines++;
                 }

                  LimitL(Sideline_status_array[i].leftline);   //�޷�
                  LimitH(Sideline_status_array[i].leftline);   //�޷�
                  LimitL(Sideline_status_array[i].rightline);  //�޷�
                  LimitH(Sideline_status_array[i].rightline);  //�޷�

                  Sideline_status_array[i].wide =Sideline_status_array[i].rightline - Sideline_status_array[i].leftline;
                  Sideline_status_array[i].midline =(Sideline_status_array[i].rightline + Sideline_status_array[i].leftline) / 2;
                  Sideline_status_array[i].De_m=(Sideline_status_array[i].midline-(MT9V03X_W/4-2));
                  int16 temp;
                  temp=Sideline_status_array[i].midline-Sideline_status_array[i+1].midline;
                  temp=func_abs(temp);
                  if(temp>3){changepoint.y=i;changepoint.num+=1;}

                  if (Sideline_status_array[i].wide <= 7)//�ж��쳣ѭ��
                  {
                      imagestatus.OFFLine = i + 1;
                      break;
                  }
                  else if (Sideline_status_array[i].rightline <= 10||Sideline_status_array[i].leftline >= 80)//�ж�ʻ������
                  {
                      imagestatus.OFFLine = i + 1;
                      break;
                  }
                  /************************************��������֮��������һЩ������������*************************************************/

    }
}
/*�Ͻ��������ַ���ɨ�ߣ���Ϊ����Բ�����ж�Ԫ�صĵڶ�����*/
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//  @name           Search_Bottom_Line_OTSU
//  @brief          ��ȡ�ײ����ұ���
//  @param          imageInput[IMAGE_ROW][IMAGE_COL]        �����ͼ������
//  @param          Row                                     ͼ���Ysite
//  @param          Col                                     ͼ���Xsite
//  @return         Bottonline                              �ױ���ѡ��
//  @time           2022��10��9��
//  @Author         ������
//  Sample usage:   Search_Bottom_Line_OTSU(imageInput, Row, Col, Bottonline);
//--------------------------------------------------------------------------------------------------------------------------------------------

void Search_Bottom_Line_OTSU(uint8 imageInput[LCDH][LCDW], uint8 row, uint8 col, uint8 Bottonline)
{

    //Ѱ����߽߱�
    for (int Xsite = col / 2-2; Xsite > 1; Xsite--)
    {
        if (imageInput[Bottonline][Xsite] == 255 && imageInput[Bottonline][Xsite - 1] == 0)
        {
            Sideline_status_array[Bottonline].LeftBoundary = Xsite;//��ȡ�ױ������
            break;
        }
    }
    for (int Xsite = col / 2+2; Xsite < LCDW-1; Xsite++)
    {
        if (imageInput[Bottonline][Xsite] == 255 && imageInput[Bottonline][Xsite + 1] == 0)
        {
            Sideline_status_array[Bottonline].RightBoundary = Xsite;//��ȡ�ױ��ұ���
            break;
        }
    }


}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//  @name           Search_Left_and_Right_Lines
//  @brief          ͨ��sobel��ȡ���ұ���
//  @param          imageInput[IMAGE_ROW][IMAGE_COL]        �����ͼ������
//  @param          Row                                     ͼ���Ysite
//  @param          Col                                     ͼ���Xsite
//  @param          Bottonline                              �ױ���ѡ��
//  @return         ��
//  Sample usage:   Search_Left_and_Right_Lines(imageInput, Row, Col, Bottonline);
//--------------------------------------------------------------------------------------------------------------------------------------------

void Search_Left_and_Right_Lines(uint8 imageInput[LCDH][LCDW],uint8 row,uint8 col,uint8 Bottonline)
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
                    if (Sideline_status_array[Right_Ysite].RightBoundary_First == 93 )
                        Sideline_status_array[Right_Ysite].RightBoundary_First = Right_Xsite;
                    Sideline_status_array[Right_Ysite].RightBoundary = Right_Xsite;
                }
                else//��ǰ��Ϊ��ɫ
                {
                    // �������ı� Right_Rirection  ��ʱ��90��
                    Right_Ysite = Right_Ysite + Right_Rule[1][2 * Right_Rirection + 1];
                    Right_Xsite = Right_Xsite + Right_Rule[1][2 * Right_Rirection];
                    if (Sideline_status_array[Right_Ysite].RightBoundary_First == 93)
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
//  @param          Row                                     ͼ���Ysite
//  @param          Col                                     ͼ���Xsite
//  @param          Bottonline                              �ױ���ѡ��
//  @return         ��
//  @time           2022��10��7��
//  @Author         ������
//  Sample usage:   Search_Border_OTSU(mt9v03x_image, IMAGE_ROW, IMAGE_COL, IMAGE_ROW-8);
//--------------------------------------------------------------------------------------------------------------------------------------------

void Search_Border_OTSU(uint8 imageInput[LCDH][LCDW], uint8 row, uint8 col, uint8 Bottonline)
{
    imagestatus.WhiteLine_L = 0;
    imagestatus.WhiteLine_R = 0;
    //imagestatus.OFFLine = 1;
    /*�����±߽紦��*/
    for (int Xsite = 0; Xsite < LCDW; Xsite++)
    {
        imageInput[0][Xsite] = 0;
        imageInput[Bottonline + 1][Xsite] = 0;
    }
    /*�����ұ߽紦��*/
    for (int Ysite = 0; Ysite < LCDH; Ysite++)
    {
            Sideline_status_array[Ysite].LeftBoundary_First = 0;
            Sideline_status_array[Ysite].RightBoundary_First = 93;

            imageInput[Ysite][0] = 0;
            imageInput[Ysite][LCDW - 1] = 0;
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
        if (Sideline_status_array[Ysite].RightBoundary > LCDW - 5)
        {
            imagestatus.WhiteLine_R++;
        }
    }
}


//--------------------------------------------------------------------------------------------------------------------------
// �������     �Զ����ߺ���������ʮ��·�ڣ�
//--------------------------------------------------------------------------------------------------------------------------
void auto_extension_line(void)
{

    /************************************����ߵĲ��ߴ���*************************************************/
    if (imagestatus.WhiteLine >= 8)                                       //��������е���������8
        TFSite = 55;                                                      //�Ǿ͸�TFSite��ֵΪ55����������Ǵ����㲹��б�ʵ�һ��������
    left_FTSite=0;
    if (ExtenLFlag != 'F')                                                //���ExtenLFlag��־��������F���ǾͿ�ʼ���в��߲�����
        for (uint8 Ysite = 54; Ysite >= (imagestatus.OFFLine + 4);Ysite--)        //�ӵ�54��ʼ����ɨ��һֱɨ���������漸�С�
        {
            Pixel = ex_mt9v03x_binarizeImage[Ysite];
            if (Sideline_status_array[Ysite].IsLeftFind =='W')                            //������е������������W���ͣ�Ҳ�����ޱ������͡�
            {
                if (Sideline_status_array[Ysite + 1].leftline >= 80)                      //�����߽絽�˵�80���ұ�ȥ�ˣ��Ǵ���ʾ��Ǽ��������˵���Ѿ�����ˡ�
                {
                    imagestatus.OFFLine = Ysite + 1;                              //���������õĴ����������ǲ�������ֱ������ѭ����
                    break;
                }
                while     (Ysite >= (imagestatus.OFFLine + 4))                      //�����߽��������Ǿͽ���whileѭ�����ţ�ֱ������ѭ������������
                {
                    Ysite--;                                                      //��������
                    if (Sideline_status_array[Ysite].IsLeftFind == 'T' && Sideline_status_array[Ysite - 1].IsLeftFind == 'T' && Sideline_status_array[Ysite - 2].IsLeftFind == 'T'
 && Sideline_status_array[Ysite - 2].leftline > 0 && Sideline_status_array[Ysite - 2].leftline <80)     //���ɨ�����ޱ��е������������ж�����������
                    {
                        left_FTSite = Ysite - 2;                                         //�ǾͰ�ɨ������һ�е��������д���FTsite����
                        break;                                                      //����whileѭ��
                    }
                }
                DetL =((float)(Sideline_status_array[left_FTSite].leftline -Sideline_status_array[TFSite].leftline)) /((float)(left_FTSite - TFSite));  //��������ߵĲ���б��
                if (left_FTSite > imagestatus.OFFLine)                              //���FTSite�洢����һ����ͼ�񶥱�OFFline������
                    for (ytemp = TFSite; ytemp >= left_FTSite; ytemp--)               //��ô�Ҿʹӵ�һ��ɨ������߽������ڶ��е�λ�ÿ�ʼ����һֱ���ߣ�����FTSite�С�
                    {
                        Sideline_status_array[ytemp].leftline =(int)(DetL * ((float)(ytemp - TFSite))) +Sideline_status_array[TFSite].leftline;     //������Ǿ���Ĳ��߲�����
                    }
            }
            else                                                              //ע�⿴������else���ĸ�if��һ�ԣ�������߼���ϵ��
                TFSite = Ysite + 2;                                             //����ΪʲôҪYsite+2����û����ע�����潲������Լ����ɡ�
        }
    /************************************����ߵĲ��ߴ���*************************************************/


      /************************************�ұ��ߵĲ��ߴ�����������ߴ���˼·һģһ����ע����*************************************************/
      if (imagestatus.WhiteLine >= 8)//�����趨TFSite��Ϊ�˱���ʮ��·���������߶���Ҫ���ߵ�����£����ұ߲��ߺ�����ͻ
      TFSite = 55;
      if (ExtenRFlag != 'F')
      for (uint8 Ysite = 54; Ysite >= (imagestatus.OFFLine + 4);Ysite--)
      {
        Pixel = ex_mt9v03x_binarizeImage[Ysite];  //�浱ǰ��
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
        /************************************�ұ��ߵĲ��ߴ�����������ߴ���˼·һģһ����ע����*************************************************/

}
//--------------------------------------------------------------------------------------------------------------------------
// �������     ·���ж�
// ��ע��Ϣ     �������У�����3~13�н����жϣ�����ܿ��ȷ���һ��ֵ�ڵ�ͻ�䣬�����ж�Ϊ����·�ϣ�ͨ��midline��λ���ж��ϰ���λ��
// �Ľ����     ͨ����·ʶ����и���Ԫ���ų�
//--------------------------------------------------------------------------------------------------------------------------
uint8 RoadBlocktype_flag=0;//1Ϊ�ұߣ�2Ϊ���
void Element_Judgment_RoadBlock(void)
{
    if(imageflag.RoadBlock_Flag==1)return;

    wide_sum=0;
    static uint16 wide_sum_last=0;
    uint8 Right_Num=0,Left_Num=0;
//    uint16 De_sum;
    if(imagestatus.OFFLine>10)
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
//  �������        ·�ϴ���
//--------------------------------------------------------------
//--------------------------------------------------------------
//  @name           Element_Judgment_Left_Rings()
//  @brief          ����ͼ���жϵ��Ӻ����������ж���Բ������.
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
        || Sideline_status_array[52].IsLeftFind == 'W'
        || Sideline_status_array[53].IsLeftFind == 'W'
        || Sideline_status_array[54].IsLeftFind == 'W'
        || Sideline_status_array[55].IsLeftFind == 'W'
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
//  @brief          ����ͼ���жϵ��Ӻ����������ж���Բ������.
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
        || Sideline_status_array[52].IsRightFind == 'W'
        || Sideline_status_array[53].IsRightFind == 'W'
        || Sideline_status_array[54].IsRightFind == 'W'
        || Sideline_status_array[55].IsRightFind == 'W'
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
        if (Sideline_status_array[Ysite - 1].RightBoundary_First - Sideline_status_array[Ysite].RightBoundary_First > 4)
        {
            Right_RingsFlag_Point1_Ysite = Ysite;
            break;
        }
    }
    for (int Ysite = 58; Ysite > ring_ysite; Ysite--)
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
// �������     Ԫ��ʶ��
//------------------------------------------------------------------

void Scan_Element(void)
{
    if (          imageflag.RoadBlock_Flag == 0
            && imageflag.Zebra_Flag == 0 && imageflag.image_element_rings == 0
                  && imageflag.Bend_Road == 0
            &&  imageflag.straight_long== 0  )
    {
//        Statu = Normal;                     //
//        Element_Judgment_RoadBlock();       //·��

        Element_Judgment_Left_Rings();      //��Բ��
        Element_Judgment_Right_Rings();     //��Բ��
        Element_Judgment_Zebra();           //������
        Element_Judgment_Bend();            //���
        Straight_long_judge();              //��ֱ��
    }
//    if(imageflag.Bend_Road)
//    {
//        Element_Judgment_OutRoad();         //��·
//        if(imageflag.Out_Road)
//            imageflag.Bend_Road=0;
//    }
//    if(imageflag.Ramp)
//    {
//        Element_Judgment_OutRoad();         //��·
//        if(imageflag.Ramp)
//            imageflag.Bend_Road=0;
//    }
//    if(imageflag.Bend_Road)
//    {
//        Element_Judgment_RoadBlock();         //·��
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
        Element_Judgment_Ramp();            //�µ�
    }*/
    if(imageflag.Bend_Road)
    {
        Element_Judgment_Zebra();           //������
        if(imageflag.Zebra_Flag)
            imageflag.Bend_Road=0;
    }
}
//--------------------------------------------------------------
//  @name           Element_Handle_Left_Rings()
//  @brief          ����ͼ�������Ӻ���������������Բ������.
//  @parameter      void
//  @time
//  @Author         MRCHEN
//  Sample usage:   Element_Handle_Left_Rings();
//-------------------------------------------------------------
void Element_Handle_Left_Rings()
{
    /****************��СԲ���ж�*****************/
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
//        if(Black > 12) //��Բ��
//            imageflag.ring_big_small = 1;
//        else                        //СԲ��
//            {imageflag.ring_big_small = 2;
//             BeeOn;}
//    }

    /***************************************�ж�**************************************/
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
        //׼������
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
        //�յ�Բ��
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
        //����
    if(imageflag.image_element_rings_flag == 5 && imagestatus.Miss_Right_lines>15)
    {
        imageflag.image_element_rings_flag = 6;
    }
        //����СԲ��
    if(imageflag.image_element_rings_flag == 6 && imagestatus.Miss_Right_lines<7)
    {
        //Stop = 1;
        imageflag.image_element_rings_flag = 7;
    }
        //���� ��Բ���ж�
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
        //���� СԲ���ж�
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
        //������
    if (imageflag.image_element_rings_flag == 8)
    {
        if (    Straight_Judge(2, imagestatus.OFFLine+15, 50) < 1
             && imagestatus.Miss_Right_lines < 8
             && imagestatus.OFFLine < 7)    //�ұ�Ϊֱ���ҽ�ֹ�У�ǰհֵ����С

            imageflag.image_element_rings_flag = 9;
    }
        //����Բ������
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
//            Front_Wait_After_Enter_Ring_Count++;
            gpio_set_level(B0, 0);
        }
    }



    /***************************************����**************************************/
        //׼������  �������
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
        //����  ����
    if  ( imageflag.image_element_rings_flag == 5
        ||imageflag.image_element_rings_flag == 6
        )
    {
        int  flag_Xsite_1=0;
        int flag_Ysite_1=0;
        float Slope_Rings=0;
        for(int Ysite=55;Ysite>imagestatus.OFFLine;Ysite--)//���满��
        {
            for(int Xsite=Sideline_status_array[Ysite].leftline + 1;Xsite<Sideline_status_array[Ysite].rightline - 1;Xsite++)
            {
                if(  ex_mt9v03x_binarizeImage[Ysite][Xsite] == 1 && ex_mt9v03x_binarizeImage[Ysite][Xsite + 1] == 0)
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
            for(int Ysite=imagestatus.OFFLine+1;Ysite<30;Ysite++)
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
        //����
        if(flag_Ysite_1 != 0)
        {
            for(int Ysite=flag_Ysite_1;Ysite<60;Ysite++)
            {
                Sideline_status_array[Ysite].rightline=flag_Xsite_1+Slope_Rings*(Ysite-flag_Ysite_1);
                //if(imageflag.ring_big_small==1)//��Բ���������
                    Sideline_status_array[Ysite].midline = (Sideline_status_array[Ysite].rightline + Sideline_status_array[Ysite].leftline)/2;
                //else//СԲ�������
                //    Sideline_status_array[Ysite].midline = Sideline_status_array[Ysite].rightline - Half_Bend_Wide[Ysite];
//                if(Sideline_status_array[Ysite].midline<0)
//                    Sideline_status_array[Ysite].midline = 0;
            }
            Sideline_status_array[flag_Ysite_1].rightline=flag_Xsite_1;
            for(int Ysite=flag_Ysite_1-1;Ysite>10;Ysite--) //A���Ϸ�����ɨ��
            {
                for(int Xsite=Sideline_status_array[Ysite+1].rightline-10;Xsite<Sideline_status_array[Ysite+1].rightline+2;Xsite++)
                {
                    if(ex_mt9v03x_binarizeImage[Ysite][Xsite]==1 && ex_mt9v03x_binarizeImage[Ysite][Xsite+1]==0)
                    {
                        Sideline_status_array[Ysite].rightline=Xsite;
                        //if(imageflag.ring_big_small==1)//��Բ���������
                            Sideline_status_array[Ysite].midline = (Sideline_status_array[Ysite].rightline + Sideline_status_array[Ysite].leftline)/2;
                        //else//СԲ�������
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
        //���� С���������� �󻷲���
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
        //��Բ������ ����
    if (imageflag.image_element_rings_flag == 8 && imageflag.ring_big_small == 1)    //��Բ��
    {
        Repair_Point_Xsite = 20;
        Repair_Point_Ysite = 0;
        for (int Ysite = 40; Ysite > 5; Ysite--)
        {
            if (ex_mt9v03x_binarizeImage[Ysite][23] == 1 && ex_mt9v03x_binarizeImage[Ysite-1][23] == 0)//28
            {
                Repair_Point_Xsite = 23;
                Repair_Point_Ysite = Ysite-1;
                imagestatus.OFFLine = Ysite + 1;  //��ֹ�����¹滮
                break;
            }
        }
        for (int Ysite = 57; Ysite > Repair_Point_Ysite-3; Ysite--)         //����
        {
            Sideline_status_array[Ysite].rightline = (Sideline_status_array[58].rightline - Repair_Point_Xsite) * (Ysite - 58) / (58 - Repair_Point_Ysite)  + Sideline_status_array[58].rightline;
            Sideline_status_array[Ysite].midline = (Sideline_status_array[Ysite].rightline + Sideline_status_array[Ysite].leftline) / 2;
        }
    }
        //СԲ������ ����
    if (imageflag.image_element_rings_flag == 8 && imageflag.ring_big_small == 2)    //СԲ��
    {
        Repair_Point_Xsite = 0;
        Repair_Point_Ysite = 0;
        for (int Ysite = 55; Ysite > 5; Ysite--)
        {
            if (ex_mt9v03x_binarizeImage[Ysite][15] == 1 && ex_mt9v03x_binarizeImage[Ysite-1][15] == 0)
            {
                Repair_Point_Xsite = 15;
                Repair_Point_Ysite = Ysite-1;
                imagestatus.OFFLine = Ysite + 1;  //��ֹ�����¹滮
                break;
            }
        }
        for (int Ysite = 57; Ysite > Repair_Point_Ysite-3; Ysite--)         //����
        {
            Sideline_status_array[Ysite].rightline = (Sideline_status_array[58].rightline - Repair_Point_Xsite) * (Ysite - 58) / (58 - Repair_Point_Ysite)  + Sideline_status_array[58].rightline;
            Sideline_status_array[Ysite].midline = (Sideline_status_array[Ysite].rightline + Sideline_status_array[Ysite].leftline) / 2;
        }
    }
        //�ѳ��� �������
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
//  @brief          ����ͼ�������Ӻ���������������Բ������.
//  @parameter      void
//  @time
//  @Author         MRCHEN
//  Sample usage:   Element_Handle_Right_Rings();
//-------------------------------------------------------------
void Element_Handle_Right_Rings()
{
    /****************��СԲ���ж�*****************/
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
//        if(Black > 12) //��Բ��
//            imageflag.ring_big_small = 1;
//        else          //СԲ��
//            {imageflag.ring_big_small = 2;
//             BeeOn;}
//    }
    /****************�ж�*****************/
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
        //׼������
    if (imageflag.image_element_rings_flag == 1 && num>15)
    {
        imageflag.image_element_rings_flag = 2;
    }
    if (imageflag.image_element_rings_flag == 2 && num<5)
    {
        imageflag.image_element_rings_flag = 5;
        //Stop = 1;
    }
        //�յ�Բ��
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
        //����
    if(imageflag.image_element_rings_flag == 5 && imagestatus.Miss_Left_lines>15)
    {
      //  Stop = 1;
        imageflag.image_element_rings_flag = 6;
    }
        //����СԲ��
    if(imageflag.image_element_rings_flag == 6 && imagestatus.Miss_Left_lines<7)
    {
        imageflag.image_element_rings_flag = 7;
       // Stop=1;


        //����Ŀǰֱ�Ӹ��ƴ�Բ�� ��СԲ���ж�������



//        imageflag.ring_big_small = 1 ;
    }
        //���� ��Բ���ж�
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
        //���� СԲ���ж�
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
        //������
    if (imageflag.image_element_rings_flag == 8)
    {
         if (   Straight_Judge(1, imagestatus.OFFLine+15, 50) < 1
             && imagestatus.Miss_Left_lines < 8
             && imagestatus.OFFLine < 7)    //�ұ�Ϊֱ���ҽ�ֹ�У�ǰհֵ����С
            {imageflag.image_element_rings_flag = 9;

            }
    }

    //����Բ������
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
//            Front_Wait_After_Enter_Ring_Count++;
            gpio_set_level(B0, 0);
        }
    }
    /***************************************����**************************************/
         //׼������  �������
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

        //����  ����
    if (   imageflag.image_element_rings_flag == 5
        || imageflag.image_element_rings_flag == 6
       )
    {
        int flag_Xsite_1=0;
        int  flag_Ysite_1=0;
        float Slope_Right_Rings = 0;
        for(int Ysite=55;Ysite>imagestatus.OFFLine;Ysite--)
        {
            for(int Xsite=Sideline_status_array[Ysite].leftline + 1;Xsite<Sideline_status_array[Ysite].rightline - 1;Xsite++)
            {
                if(ex_mt9v03x_binarizeImage[Ysite][Xsite]==1 && ex_mt9v03x_binarizeImage[Ysite][Xsite+1]==0)
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
        for(int Ysite=imagestatus.OFFLine+1;Ysite<30;Ysite++)
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
        //����
        if(flag_Ysite_1!=0)
        {
            for(int Ysite=flag_Ysite_1;Ysite<60;Ysite++)
            {
                Sideline_status_array[Ysite].leftline=flag_Xsite_1+Slope_Right_Rings*(Ysite-flag_Ysite_1);
                //if(imageflag.ring_big_small==2)//СԲ���Ӱ��
                //    Sideline_status_array[Ysite].midline=Sideline_status_array[Ysite].leftline+Half_Bend_Wide[Ysite];//���
//              else//��Բ�����Ӱ��
                    Sideline_status_array[Ysite].midline=(Sideline_status_array[Ysite].leftline+Sideline_status_array[Ysite].rightline)/2;//���
                //if(Sideline_status_array[Ysite].midline>79)
                //    Sideline_status_array[Ysite].midline=79;
            }
            Sideline_status_array[flag_Ysite_1].leftline=flag_Xsite_1;
            for(int Ysite=flag_Ysite_1-1;Ysite>10;Ysite--) //A���Ϸ�����ɨ��
            {
                for(int Xsite=Sideline_status_array[Ysite+1].leftline+8;Xsite>Sideline_status_array[Ysite+1].leftline-4;Xsite--)
                {
                    if(ex_mt9v03x_binarizeImage[Ysite][Xsite]==1 && ex_mt9v03x_binarizeImage[Ysite][Xsite-1]==0)
                    {
                     Sideline_status_array[Ysite].leftline=Xsite;
                     Sideline_status_array[Ysite].wide=Sideline_status_array[Ysite].rightline-Sideline_status_array[Ysite].leftline;
                  //   if(imageflag.ring_big_small==2)//СԲ���Ӱ��
                  //       Sideline_status_array[Ysite].midline=Sideline_status_array[Ysite].leftline+Half_Bend_Wide[Ysite];//���
                   //  else//��Բ�����Ӱ��
                         Sideline_status_array[Ysite].midline=(Sideline_status_array[Ysite].leftline+Sideline_status_array[Ysite].rightline)/2;//���
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
        //���ڲ�����
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

        //��Բ������ ����
    if (imageflag.image_element_rings_flag == 8 && imageflag.ring_big_small == 1)  //��Բ��
    {
        Repair_Point_Xsite = 60;
        Repair_Point_Ysite = 0;
        for (int Ysite = 50; Ysite > 5; Ysite--)
        {
            if (ex_mt9v03x_binarizeImage[Ysite][57] == 1 && ex_mt9v03x_binarizeImage[Ysite-1][57] == 0)
            {
                Repair_Point_Xsite = 57;
                Repair_Point_Ysite = Ysite-1;
                imagestatus.OFFLine = Ysite + 1;  //��ֹ�����¹滮
                        //  ips200_show_uint(200,200,Repair_Point_Ysite,2);
                break;
            }
        }
        for (int Ysite = 57; Ysite > Repair_Point_Ysite-3; Ysite--)         //����
        {
            Sideline_status_array[Ysite].leftline = (Sideline_status_array[58].leftline - Repair_Point_Xsite) * (Ysite - 58) / (58 - Repair_Point_Ysite)  + Sideline_status_array[58].leftline;
            Sideline_status_array[Ysite].midline = (Sideline_status_array[Ysite].rightline + Sideline_status_array[Ysite].leftline) / 2;
        }
    }
        //СԲ������ ����
    if (imageflag.image_element_rings_flag == 8 && imageflag.ring_big_small == 2)  //СԲ��
    {
        Repair_Point_Xsite = 79;
        Repair_Point_Ysite = 0;
        for (int Ysite = 40; Ysite > 5; Ysite--)
        {
            if (ex_mt9v03x_binarizeImage[Ysite][58] == 1 && ex_mt9v03x_binarizeImage[Ysite-1][58] == 0)
            {
                Repair_Point_Xsite = 58;
                Repair_Point_Ysite = Ysite-1;
                imagestatus.OFFLine = Ysite + 1;  //��ֹ�����¹滮
                        //  ips200_show_uint(200,200,Repair_Point_Ysite,2);
                break;
            }
        }
        for (int Ysite = 55; Ysite > Repair_Point_Ysite-3; Ysite--)         //����
        {
            Sideline_status_array[Ysite].leftline = (Sideline_status_array[58].leftline - Repair_Point_Xsite) * (Ysite - 58) / (58 - Repair_Point_Ysite)  + Sideline_status_array[58].leftline;
            Sideline_status_array[Ysite].midline = (Sideline_status_array[Ysite].rightline + Sideline_status_array[Ysite].leftline) / 2;
        }
    }
        //�ѳ��� �������
    if (imageflag.image_element_rings_flag == 9 || imageflag.image_element_rings_flag == 10)
    {
        for (int Ysite = 59; Ysite > imagestatus.OFFLine; Ysite--)
        {
            Sideline_status_array[Ysite].midline = Sideline_status_array[Ysite].leftline + Half_Road_Wide[Ysite];
        }
    }
}

//--------------------------------------------------------------
//  @name           Element_Judgment_Zebra()
//  @brief          ����ͼ���жϵ��Ӻ����������жϰ�����
//  @parameter      void
//  @time
//  @Author         MRCHEN
//  Sample usage:   Element_Judgment_Zebra();
//--------------------------------------------------------------
uint8 Garage_Location_Flag = 0;                 //�жϿ�Ĵ���
void Element_Judgment_Zebra()//�������ж�
{
    if(imageflag.Zebra_Flag || imageflag.image_element_rings == 1 || imageflag.image_element_rings == 2
            || imageflag.Out_Road == 1 || imageflag.RoadBlock_Flag == 1) return;
    int NUM = 0,net = 0;
    if(imagestatus.OFFLineBoundary<20)
    {
        for (int Ysite = 20; Ysite < 33; Ysite++)
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
//  �������        �����ߴ���
//------------------------------------------------------------------

void Element_Handle_Zebra(void)
{
    int left_num=0,right_num=0;

    if(imageflag.Zebra_Flag == 1)//�����־λ
        {
            for (int Ysite = 59; Ysite > imagestatus.OFFLineBoundary + 1; Ysite--)
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
//// �������     ���ʶ���޸�
////----------------------------------------------------------------
//int Miss_Left_Num = 0 ;
//int Miss_Right_Num = 0;
//void Element_Judgment_Bend_fix(void)
//{
//    if(Sideline_status_array[imagestatus.OFFLine+1].leftline > 30
//                && imagestatus.Miss_Left_lines < 4
//                && imagestatus.Miss_Right_lines > 8
//                && Straight_Judge(1, imagestatus.OFFLine+2, 58) > 1)
//    {
//
//    }
//}
//--------------------------------------------------------------
//  @name           Element_Judgment_Bend()
//  @brief          ����ͼ���жϵ��Ӻ����������ж������������.
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
            && imagestatus.Miss_Right_lines > 8
            && Straight_Judge(1, imagestatus.OFFLine+2, 58) > 1)
    {

        imageflag.Bend_Road = 1;
//        BeeOn;
    }
    if(Sideline_status_array[imagestatus.OFFLine+1].rightline < 50
            && imagestatus.Miss_Right_lines < 4
            && imagestatus.Miss_Left_lines > 8
            && Straight_Judge(2, imagestatus.OFFLine+2, 58) > 1)
    {

        imageflag.Bend_Road = 2;
//        BeeOn;
    }
}
//--------------------------------------------------------------
//  @name           Element_Handle_Bend()
//  @brief          ����ͼ�������Ӻ��������������������
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
    //--------------------------------------------------------------
    //  @name           Element_GetStraightLine_Bend()
    //  @brief          ����ͼ�������Ӻ��������������������
    //  @parameter      void
    //  @time
    //  @Author         MRCHEN
    //  Sample usage:   Element_Handle_Bend();
    //--------------------------------------------------------------
    void Element_GetStraightLine_Bend()
    {
        if(imagestatus.OFFLine < 4 && imagestatus.WhiteLine_R>10 && Straight_Judge(1, 20, 50)<1)
        {
            for (int Ysite = 59; Ysite > imagestatus.OFFLine; Ysite--)
            {
                Sideline_status_array[Ysite].midline = Sideline_status_array[Ysite].leftline + Half_Road_Wide[Ysite];
            }
        }
        if(imagestatus.OFFLine < 4 && imagestatus.WhiteLine_L>10 && Straight_Judge(2, 20, 50)<1)
        {
            for (int Ysite = 59; Ysite > imagestatus.OFFLine; Ysite--)
            {
                Sideline_status_array[Ysite].midline = Sideline_status_array[Ysite].rightline - Half_Road_Wide[Ysite];
            }
        }
    }
    /*Ԫ�ش�������*/
    void Element_Handle(void)
    {
        Element_GetStraightLine_Bend();//ֱ������

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
        else if(imagestatus.WhiteLine >= 8) //ʮ�ִ���
            auto_extension_line();
    }
////--------------------------------------------------------------------------------------------------------------------------
//// �������     ����Ѳ�ߣ���������Ϣ��¼��ex_leftline,ex_midline,ex_rightline������
//// ʹ��ʵ��     horizontal_line_fix(ex_mt9v03x_binarizeImage,ex_leftline,ex_rightline,ex_midline);
////--------------------------------------------------------------------------------------------------------------------------
//void horizontal_line_fix(uint8 binaryImage[IMAGE_HEIGHT][IMAGE_WIDTH], uint8 leftLine[IMAGE_HEIGHT], uint8 rightLine[IMAGE_HEIGHT], uint8 midLine[IMAGE_HEIGHT])
//{
//    static int lastMid = IMAGE_WIDTH / 2; // ��ʼ����һ������λ��Ϊͼ��������
//    for (int row = find_start; row < find_end; row++)
//    {
//        int left = -1;
//        int right = -1;
//
//        // ����һ������λ�ÿ�ʼ������Ѱ�ұ���
//        for (int col = lastMid; col > 0; col--)
//        {
//            if(col<=5){left = 5;}
//            else if (binaryImage[row][col] == 0)
//            {
//                left = col;
//                break;
//            }
//
//        }
//        for (int col = lastMid; col < IMAGE_WIDTH-1; col++)
//        {
//            if (binaryImage[row][col] == 0)
//            {
//                right = col;
//                break;
//            }
//            if(col>=IMAGE_WIDTH-6)right = IMAGE_WIDTH-6;
//        }
//
//        // ������һ������λ��
//        lastMid = (left + right) / 2;
//
//        // ������洢����Ӧ������
//        leftLine[row] = left;
//        rightLine[row] = right;
//        midLine[row] = lastMid;
//
//    }
////    int i, j;
////
//       for (i = 0; i < IMAGE_HEIGHT; i++) {
//           int leftIndex = 0;
//           int rightIndex = IMAGE_WIDTH - 1;
//           int midIndex = IMAGE_WIDTH / 2;
//
//           for (j = 0; j < IMAGE_WIDTH; j++) {
//               if (binaryImage[i][j] == 0) {
//                   // �ҵ�����ߵ�λ��
//                   leftLine[leftIndex] = j;
//                   leftIndex++;
//                   // �ҵ��ұ��ߵ�λ��
//                   rightLine[rightIndex] = j;
//                   rightIndex--;
//                   // �ҵ����ߵ�λ��
//                   midLine[j] = i;
//               }
//           }
//       }
//}

////------------------------------------------------------------------------------------------------------------------------------
//// �������     Ѱ�Ҷ�����ʼ��_�����
//// ʹ��ʵ��     findLaneStartRow_left(ex_leftline);
////------------------------------------------------------------------------------------------------------------------------------
//int findLaneStartRow_left(uint8 leftLine[IMAGE_HEIGHT])
//{
//    int startRow = -1;  // ��ʼ��Ϊ-1����ʾδ�ҵ����߿�ʼ��
//
//    // ��ͼ��ײ��򶥲�����
//    for (uint8 i = IMAGE_HEIGHT - 1; i >= 0; i--) {
//        if (leftLine[i] == 0 )
//        {
//            startRow = i;
//            break;  // �ҵ����߿�ʼ�У��˳�ѭ��
//        }
//    }
//
//    return startRow;
//}
////------------------------------------------------------------------------------------------------------------------------------
//// �������     Ѱ�Ҷ�����ʼ��_�ұ���
//// ʹ��ʵ��     findLaneStartRow_left(ex_rightline);
////------------------------------------------------------------------------------------------------------------------------------
//int findLaneStartRow_right(uint8 rightLine[IMAGE_HEIGHT])
//{
//    int startRow = -1;
//    for(uint8 i = IMAGE_HEIGHT - 1; i >= 0; i--)
//    {
//        if(rightLine[i] == IMAGE_HEIGHT - 1)
//        {
//            startRow = i;
//            break;
//        }
//    }
//    return startRow;
//}

////-------------------------------------------------------------------------------------------------------------------------------
//// �������     �յ���
//// ʹ��ʾ��     detectCornerBothLines(ex_leftline,ex_rightline,ex_leftCorners,ex_rightCorners,&ex_leftCornerCount,&ex_rightCornerCount)
//// ��ע��Ϣ     ��
////-------------------------------------------------------------------------------------------------------------------------------
//void detectCornerBothLines(uint8 leftLine[IMAGE_HEIGHT], uint8 rightLine[IMAGE_HEIGHT], CornerPoint leftCorners[], CornerPoint rightCorners[], int* leftCornerCount, int* rightCornerCount) {
//    *leftCornerCount = 0; // ��ʼ������߹յ����Ϊ0
//    *rightCornerCount = 0; // ��ʼ���ұ��߹յ����Ϊ0
//
//    for (int i = 2; i < IMAGE_HEIGHT - 2; i += 2) { // ÿ���м��һ��
//        if ((leftLine[i] - leftLine[i-2]) * (leftLine[i+2] - leftLine[i]) < 0) {
//            // ���������ͬһ�г��ְ���ʱ����Ϊ��һ���յ�
//            leftCorners[*leftCornerCount].x = leftLine[i]; // ��¼����߹յ�ĺ�����
//            leftCorners[*leftCornerCount].y = i; // ��¼����߹յ��������Ϊ�к�
//            (*leftCornerCount)++; // ����߹յ������1
//        }
//
//        if ((rightLine[i] - rightLine[i-2]) * (rightLine[i+2] - rightLine[i]) < 0) {
//            // ���ұ�����ͬһ�г��ְ���ʱ����Ϊ��һ���յ�
//            rightCorners[*rightCornerCount].x = rightLine[i]; // ��¼�ұ��߹յ�ĺ�����
//            rightCorners[*rightCornerCount].y = i; // ��¼�ұ��߹յ��������Ϊ�к�
//            (*rightCornerCount)++; // �ұ��߹յ������1
//        }
//    }
//}
//-----------------------------------------------------------------------------------------------------------------------------
// �������     ���ߺ���
// ����˵�� x1  �жϵ�1��x����
// ����˵�� y1  �жϵ�1��y����
// ����˵�� x2  �жϵ�2��x����
// ����˵�� y2  �жϵ�2��y����
// ʹ��ʵ��     line(0,0,20,30);
//-----------------------------------------------------------------------------------------------------------------------------
//void connect_line(uint8 x1,uint8 y1,uint8 x2,uint8 y2)
//{
//  short i,j,swap;
//  float k;
//  if(y1>y2)
//  {
//    swap = x1;
//    x1 = x2;
//    x2 = swap;
//    swap = y1;
//    y1 = y2;
//    y2 = swap;
//  }
//  if(x1==x2)
//  {
//    for(i=y1;i<y2+1;i++)
//        ex_mt9v03x_binarizeImage[i][x1]=0;
//  }
//  else if(y1==y2)
//  {
//    for(i=x1;i<x2+1;i++)
//        ex_mt9v03x_binarizeImage[y1][i]=0;
//  }
//  else
//  {
//    k = ((float)x2-(float)x1)/((float)y2-(float)y1);
//    for(i=y1;i<=y2;i++)
//        ex_mt9v03x_binarizeImage[i][(short)(x1+(i-y1)*k)]=0;
//  }
//}

//
////--------------------------------------------------------------------------------------------------------------------------
//// �������     ��ֱ�߽����ж�(���������ж�)
//// ����˵�� arr     ��������
//// ����ֵ       1Ϊֱ�� 0Ϊ��ֱ��
//// ʹ��ʵ��     Straight_line_judgment(left_arr);
//// ��ע��Ϣ     ֻ�ж�30-100
////--------------------------------------------------------------------------------------------------------------------------
//uint8 straight_line_judgment(uint8 arr[Row])
//{
//  short i,sum=0;
//  float kk;
//  kk=((float)arr[90]-(float)arr[20])/70.0;//����kֵ
//  sum = 0;
//  for(i=20;i<=90;i++)
//    if(((arr[20]+(float)(i-20)*kk)-arr[i])<=20) sum++;//�������ֵ��ʵ��ֵ�Ĳ���С�ڵ���35�������
//    else break;
//  if(sum>68 && kk>-1.1 && kk<1.1) return 1;
//  else return 0;
//}
//