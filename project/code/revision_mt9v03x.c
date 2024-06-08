/*
 * revision_mt9v03x.c
 *
 *  Created on: 2024��4��28��
 *      Author: pc
 */
#include "zf_common_headfile.h"
//#include "revision_mt9v03x.h"

uint8* Image_Use[LCDH][LCDW];      //�����洢ѹ��֮��Ҷ�ͼ��Ķ�ά����
uint8 ex_mt9v03x_binarizeImage[LCDH][LCDW];          //ͼ����ʱ��������Ķ�ֵ��ͼ������
uint8 Threshold;                                //ͨ����򷨼�������Ķ�ֵ����ֵ
int ImageScanInterval=5;                        //ɨ�ߵķ�Χ
static uint8* PicTemp;                          //һ�����浥��ͼ���ָ�����
static int IntervalLow = 0, IntervalHigh = 0;   //ɨ������������ޱ���
static int Ysite = 0, Xsite = 0;                //Ysite����ͼ����У�Xsite����ͼ����С�
static int BottomBorderRight = 79,              //59�е��ұ߽�
BottomBorderLeft = 0,                           //59�е���߽�
BottomCenter = 0;                               //59�е��е�
uint8 ExtenLFlag = 0;                           //������Ƿ���Ҫ���ߵı�־����
uint8 ExtenRFlag = 0;                           //�ұ����Ƿ���Ҫ���ߵı�־����
uint8 Ring_Help_Flag = 0;                       //����������־
int Right_RingsFlag_Point1_Ysite, Right_RingsFlag_Point2_Ysite; //��Բ���жϵ�����������
int Left_RingsFlag_Point1_Ysite, Left_RingsFlag_Point2_Ysite;   //��Բ���жϵ�����������
int Point_Xsite,Point_Ysite;                   //�յ��������
int Repair_Point_Xsite,Repair_Point_Ysite;     //���ߵ��������
int Point_Ysite1;                               //�жϴ�СԲ��ʱ�õ�������
int Black;                                      //�жϴ�СԲ��ʱ�õĺڵ�����
int Less_Big_Small_Num;                         //�жϴ�СԲ��ʱ�õĶ���
int left_difference_num;                        //ʮ����������׼����39����ĺͣ�40-20�У�
int right_difference_num;                       //ʮ���ұ������׼����39����ĺͣ�40-20�У�
uint8 Garage_Location_Flag = 0;                 //�жϿ�Ĵ���
float Big_Small_Help_Gradient;               //��СԲ���ĸ����ж�б��
static int ytemp = 0;                           //����е���ʱ����
static int TFSite = 0, left_FTSite = 0,right_FTSite = 0;              //���߼���б�ʵ�ʱ����Ҫ�õĴ���еı�����
static float DetR = 0, DetL = 0;                //��Ų���б�ʵı���
Sideline_status Sideline_status_array[60];             //��¼������Ϣ�Ľṹ������
Sideline_status ImageDeal1[80];             //��¼������Ϣ�Ľṹ������
Image_Status imagestatus;                 //ͼ����ĵ�ȫ�ֱ���
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

float variance, variance_acc;                   //�ж�ֱ������ķ���
int variance_limit_long,variance_limit_short;   //��ֱ������ֱ���ķ����޶�ֵ
#define Middle_Center 39                        //��Ļ����

//int j=5;
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
//--------------------------------------------------------------------------------------------------------
// �������     ���㷶Χ�ڱ��ߵ�б�ʣ����Ƿ�Ϊֱ�߽����ж�
// ����˵��            dir      �ж�ģʽ1Ϊ�������ߣ�2Ϊ����ұ���
//          start    ��⿪ʼ�У��������£�
//          end      �������У��������£�
// ʹ��ʵ��       float a=Straight_Judge(1,imagestatus.Offline,50);
// ��ע��Ϣ      ������ʼ���������������߶ε�б�ʣ��ж�����ṹ�ķ��������С��1��ʱ��˵���߶����Ч���Ϻã�������Ϊ��һ��ֱ��
//--------------------------------------------------------------------------------------------------------
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


/*****************��������ж�******************/
uint32 break_road(uint8 line_start)//�������
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
uint32 white_point(uint8 line_end,uint8 line_start) //�׵�����
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
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//  @name           Image_CompressInit
//  @brief          ԭʼ�Ҷ�ͼ��ѹ������ ѹ����ʼ��
//  @brief          ���þ��ǽ�ԭʼ�ߴ�ĻҶ�ͼ��ѹ����������Ҫ�Ĵ�С���������ǰ�ԭʼ80��170�еĻҶ�ͼ��ѹ����60��80�еĻҶ�ͼ��
//  @brief          ΪʲôҪѹ������Ϊ�Ҳ���Ҫ��ô�����Ϣ��60*80ͼ����չʾ����Ϣԭ�����Ѿ��㹻��ɱ��������ˣ���Ȼ����Ը����Լ�������޸ġ�
//  @parameter      void
//  @return         void
//  @time           2022��2��18��
//  @Author
//  Sample usage:   Image_CompressInit();
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
void Image_CompressInit(void)
{
  int i, j, row, line;
  const float div_h = Mh / Lh, div_w = Mw / Lw;         //����ԭʼ��ͼ��ߴ��������Ҫ��ͼ��ߴ�ȷ����ѹ��������
  for (i = 0; i < LCDH; i++)                            //����ͼ���ÿһ�У��ӵ����е���59�С�
  {
    row = i * div_h + 0.5;
    for (j = 0; j < LCDW; j++)                          //����ͼ���ÿһ�У��ӵ����е���79�С�
    {
      line = j * div_w + 0.5;
      Image_Use[i][j] = &mt9v03x_image[row][line];       //mt9v03x_image����������ԭʼ�Ҷ�ͼ��Image_Use����洢������֮��Ҫ��ȥ�����ͼ�񣬵���Ȼ�ǻҶ�ͼ��Ŷ��ֻ��ѹ����һ�¶��ѡ�
    }
  }
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
uint8 get_Threshold(uint8* ex_mt9v03x_binarizeImage[][LCDW],uint16 col, uint16 row)
{
  #define GrayScale 256
  uint16 width = col;
  uint16 height = row;
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
      pixelCount[*ex_mt9v03x_binarizeImage[i][j]]++;       //����ǰ�����ص������ֵ���Ҷ�ֵ����Ϊ����������±ꡣ
      gray_sum += *ex_mt9v03x_binarizeImage[i][j];         //���������Ҷ�ͼ��ĻҶ�ֵ�ܺ͡�
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

//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//  @name           Get_BinaryImage
//  @brief          �Ҷ�ͼ���ֵ������
//  @brief          ����˼·���ǣ��ȵ���Get_Threshold���������õ���ֵ��Ȼ�����ԭʼ�Ҷ�ͼ���ÿһ�����ص㣬��ÿһ�����ص�ĻҶ�ֵ������ֵ�ƽϡ�
//  @brief          ������ֵ����Ͱ����Ǹ����ص��ֵ��ֵΪ1����Ϊ�׵㣩������͸�ֵΪ0����Ϊ�ڵ㣩����Ȼ����԰������ֵ��������ֻҪ���Լ����1��0˭�����˭����׾��С�
//  @brief          ������ǰ���ᵽ��60*80�������Ǿ�Ӧ��������ʲô��˼�˰ɣ��������ص��һ����80�����ص㣬һ��60�У�Ҳ����ѹ�����ÿһ��ͼ����4800�����ص㡣
//  @parameter      void
//  @return         void
//  @time           2022��2��17��
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
//        Threshold = get_Threshold(Image_Use, LCDW, LCDH);      //������һ���������ã�ͨ���ú������Լ����һ��Ч���ܲ���Ķ�ֵ����ֵ��
//    }
    Threshold = get_Threshold(Image_Use, LCDW, LCDH)+threshold_fix;      //������һ���������ã�ͨ���ú������Լ����һ��Ч���ܲ���Ķ�ֵ����ֵ��

  uint8 i, j = 0;
  for (i = 0; i < LCDH; i++)                                //������ά�����ÿһ��
  {
    for (j = 0; j < LCDW; j++)                              //������ά�����ÿһ��
    {
      if (*Image_Use[i][j] > Threshold)                      //��������ĻҶ�ֵ������ֵThreshold
          ex_mt9v03x_binarizeImage[i][j] = 255;                                  //��ô������ص�ͼ�Ϊ�׵�
      else                                                  //��������ĻҶ�ֵС����ֵThreshold
          ex_mt9v03x_binarizeImage[i][j] = 0;                                  //��ô������ص�ͼ�Ϊ�ڵ�
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
    for (i = H; i >= L; i--)                    //��������ɨ
    {
      if (*(p + i) != 0 && *(p + i - 1) == 0)   //����кڰ�����    1�ǰ� 0�Ǻ�
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
      if (*(p + i) != 0 && *(p + i + 1) == 0)   //����кڰ�����    1�ǰ� 0�Ǻ�
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

//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//  @name           Get_BaseLine
//  @brief          �ñ����ķ����õ�ͼ����������У�59-55�У��ı��ߺ�������Ϣ�������б��ߺ�������Ϣ��׼ȷ�ȷǳ�����Ҫ��ֱ��Ӱ�쵽����ͼ��Ĵ�������
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

    PicTemp = ex_mt9v03x_binarizeImage[59];                                                //��PicTemp���ָ�����ָ��ͼ�������Pixle[59]
    for (Xsite = ImageSensorMid; Xsite < 79; Xsite++)                   //����39�������У��������п�ʼһ��һ�е����ұ������ұ���
    {
      if (*(PicTemp + Xsite) == 0 && *(PicTemp + Xsite + 1) == 0)       //������������������ڵ㣬˵û�ҵ��˱��ߡ�
      {
        BottomBorderRight = Xsite;                                      //����һ�м�¼������Ϊ��һ�е��ұ���
        break;                                                          //����ѭ��
      }
      else if (Xsite == 78)                                             //����ҵ��˵�58�ж���û���ֺڵ㣬˵����һ�еı��������⡣
      {
        BottomBorderRight = 79;                                         //����������Ĵ�����ǣ�ֱ�Ӽ���ͼ�����ұߵ���һ�У���79�У�������һ�е��ұ��ߡ�
        break;                                                          //����ѭ��
      }
    }

    for (Xsite = ImageSensorMid; Xsite > 0; Xsite--)                    //����39�������У��������п�ʼһ��һ�е���������������
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

    BottomCenter =(BottomBorderLeft + BottomBorderRight) / 2;           //�������ұ߽�������59�е�����
    Sideline_status_array[59].leftline = BottomBorderLeft;                        //�ѵ�59�е���߽�洢�����飬ע�⿴ImageDeal������ֵ��±꣬�ǲ������ö�Ӧ59��
    Sideline_status_array[59].rightline = BottomBorderRight;                      //�ѵ�59�е��ұ߽�洢�����飬ע�⿴ImageDeal������ֵ��±꣬�ǲ������ö�Ӧ59��
    Sideline_status_array[59].midline = BottomCenter;                                //�ѵ�59�е����ߴ洢�����飬    ע�⿴ImageDeal������ֵ��±꣬�ǲ������ö�Ӧ59��
    Sideline_status_array[59].wide = BottomBorderRight - BottomBorderLeft;          //�ѵ�59�е�������ȴ洢���飬ע�⿴ImageDeal������ֵ��±꣬�ǲ������ö�Ӧ59��
    Sideline_status_array[59].IsLeftFind = 'T';                                     //��¼��59�е����������ΪT���������ҵ�����ߡ�
    Sideline_status_array[59].IsRightFind = 'T';                                    //��¼��59�е��ұ�������ΪT���������ҵ��ұ��ߡ�

    /****************************************************End*******************************************************************************/
    /**************************************��������ͼ������У�59�У����ұ��ߴӶ�ȷ�����ߵĹ��� ********************************************************************/



    /**************************************�ڵ�59�������Ѿ�ȷ���������ȷ��58-54���������ߵĹ��� ******************************************/
    /****************************************************Begin*****************************************************************************/
    /*
         * ���漸�еĵ����߹����ҾͲ���׸���ˣ������ҵ�ע�Ͱѵ�59�е����߹������ã�
         * ��ô58��54�е����߾���ȫû���⣬��һģһ�����߼��͹��̡�
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
  for (Ysite = 54 ; Ysite > imagestatus.OFFLine; Ysite--)                            //ǰ5����Get_BaseLine()���Ѿ�������ˣ����ڴ�55�д����Լ��趨�Ĳ�������OFFLine��
  {                                                                                  //��Ϊ̫ǰ���ͼ��ɿ��Բ��㣬����OFFLine�����ú��б�Ҫ��û��Ҫһֱ����ɨ����0�С�
    PicTemp = ex_mt9v03x_binarizeImage[Ysite];
    JumpPointtypedef JumpPoint[2];                                                   // JumpPoint[0]��������ߣ�JumpPoint[1]�����ұ��ߡ�

  /******************************ɨ�豾�е��ұ���******************************/
    IntervalLow  = Sideline_status_array[Ysite + 1].rightline  - ImageScanInterval;               //����һ�е��ұ��߼Ӽ�Interval��Ӧ���п�ʼɨ�豾�У�Intervalһ��ȡ5����Ȼ��Ϊ�˱���������԰����ֵ�ĵĴ�һ�㡣
    IntervalHigh = Sideline_status_array[Ysite + 1].rightline + ImageScanInterval;              //���������ֻ��Ҫ�����б�������5�Ļ����ϣ����10�е�������䣩ȥɨ�ߣ�һ������ҵ����еı����ˣ��������ֵ��ʵ����̫��
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
        Sideline_status_array[Ysite].rightline =Sideline_status_array[ytemp_W_R].rightline -D_R * (ytemp_W_R - Ysite);  //����ҵ��� ��ô�Ի�׼�����ӳ���
      }
      LimitL(Sideline_status_array[Ysite].rightline);  //�޷�
      LimitH(Sideline_status_array[Ysite].rightline);  //�޷�
    }
    /**************************�����ұ��ߵ��ޱ���***************************/


    /**************************��������ߵ��ޱ���***************************/
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

    /**************************��������ߵ��ޱ���***************************/
    /************************************����ȷ���ޱ��У���W�ࣩ�ı߽�****************************************************************/


    /************************************��������֮��������һЩ������������*************************************************/
      if (Sideline_status_array[Ysite].IsLeftFind == 'W'&&Sideline_status_array[Ysite].IsRightFind == 'W')
      {
          imagestatus.WhiteLine++;  //Ҫ�����Ҷ��ޱߣ�������+1
      }
     if (Sideline_status_array[Ysite].IsLeftFind == 'W' && Ysite < 55 )
     {
          imagestatus.Miss_Left_lines++;
     }
     if (Sideline_status_array[Ysite].IsRightFind == 'W'&&Ysite < 55)
     {
          imagestatus.Miss_Right_lines++;
     }

      LimitL(Sideline_status_array[Ysite].leftline);   //�޷�
      LimitH(Sideline_status_array[Ysite].leftline);   //�޷�
      LimitL(Sideline_status_array[Ysite].rightline);  //�޷�
      LimitH(Sideline_status_array[Ysite].rightline);  //�޷�

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
      /************************************��������֮��������һЩ������������*************************************************/
  }
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//  @name           Get_ExtensionLine
//  @brief          ������������þ��ǲ��ߣ�
//  @brief          ʮ��·�����������˵����ֱ�ж԰ɣ����������������ͷɨ�ߵ�ʱ���ǲ��ǻ����ɨ�������ߵ��������Ϊ�Ǽ��ж��ǰ�ɫ����Ҳ����ڰ�����㡣
//  @brief          ���԰���������ѱ����㷨��������ǲ�������������㷨����Ļ���������Щ�е����ұ߽��ǲ��ǾͲ����ˣ���Ӧ�������ǲ���Ҳ�����ˣ������ܱ�֤С����ֱ���
//  @brief          ��Ȼ��֤���ˣ��������ʱ��С�����ܾͻ��������������ߣ�ֱ����ת������ת�ˣ��ǲ���Υ�����������ˣ����ǲ��Ǿͼ��ˣ�����˵�����Ƿǳ���Ҫ��һ����
//  @parameter      void
//  @time           2023��2��21��
//  @Author
//  Sample usage:   Get_ExtensionLine();
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
void Get_ExtensionLine(void)
{
    //imagestatus.OFFLine=5;                                                  //����ṹ���Ա��֮���������︳ֵ������Ϊ��ImageStatus�ṹ������ĳ�Ա̫���ˣ�������ʱ��ֻ�õ���OFFLine�������������õ��������ĸ�ֵ��
    /************************************����ߵĲ��ߴ���*************************************************/
    if (imagestatus.WhiteLine >= 8)                                       //��������е���������8
        TFSite = 55;                                                      //�Ǿ͸�TFSite��ֵΪ55����������Ǵ����㲹��б�ʵ�һ��������
    left_FTSite=0;
    right_FTSite=0;
    if (ExtenLFlag != 'F')                                                //���ExtenLFlag��־��������F���ǾͿ�ʼ���в��߲�����
        for (Ysite = 54; Ysite >= (imagestatus.OFFLine + 4);Ysite--)        //�ӵ�54��ʼ����ɨ��һֱɨ���������漸�С�
        {
            PicTemp = ex_mt9v03x_binarizeImage[Ysite];
            if (Sideline_status_array[Ysite].IsLeftFind =='W')                            //������е������������W���ͣ�Ҳ�����ޱ������͡�
            {
                if (Sideline_status_array[Ysite + 1].leftline >= 70)                      //�����߽絽�˵�70���ұ�ȥ�ˣ��Ǵ���ʾ��Ǽ��������˵���Ѿ�����ˡ�
                {
                  imagestatus.OFFLine = Ysite + 1;                              //���������õĴ��������ǲ�����ֱ������ѭ����
                  break;
                }
                while (Ysite >= (imagestatus.OFFLine + 4))                      //�����߽��������Ǿͽ���whileѭ�����ţ�ֱ������ѭ������������
                {
                    Ysite--;                                                      //��������
                    if (Sideline_status_array[Ysite].IsLeftFind == 'T'
                      &&Sideline_status_array[Ysite - 1].IsLeftFind == 'T'
                      &&Sideline_status_array[Ysite - 2].IsLeftFind == 'T'
                      &&Sideline_status_array[Ysite - 2].leftline > 0
                      &&Sideline_status_array[Ysite - 2].leftline <70
                      )                                                         //���ɨ�����ޱ��е������������ж�����������
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


    /************************************�ұ��ߵĲ��ߴ���������ߴ���˼·һģһ����ע����*************************************************/
    if (imagestatus.WhiteLine >= 8)
    TFSite = 55;
    if (ExtenRFlag != 'F')
    for (Ysite = 54; Ysite >= (imagestatus.OFFLine + 4);Ysite--)
    {
      PicTemp = ex_mt9v03x_binarizeImage[Ysite];  //�浱ǰ��
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
      /************************************�ұ��ߵĲ��ߴ���������ߴ���˼·һģһ����ע����*************************************************/



}

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

void Search_Bottom_Line_OTSU(uint8 imageInput[LCDH][LCDW], uint8 row, uint8 col, uint8 Bottonline)
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
    for (int Xsite = col / 2+2; Xsite < LCDW-1; Xsite++)
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

void Search_Left_and_Right_Lines(uint8 imageInput[LCDH][LCDW], uint8 row, uint8 col, uint8 Bottonline)
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
                    if (Sideline_status_array[Right_Ysite].RightBoundary_First == 79 )
                        Sideline_status_array[Right_Ysite].RightBoundary_First = Right_Xsite;
                    Sideline_status_array[Right_Ysite].RightBoundary = Right_Xsite;
                }
                else//��ǰ��Ϊ��ɫ
                {
                    // �������ı� Right_Rirection  ��ʱ��90��
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
            Sideline_status_array[Ysite].RightBoundary_First = 79;

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
        if (Sideline_status_array[Ysite].RightBoundary > LCDW - 3)
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
//  @brief          ����ͼ���жϵ��Ӻ����������ж���Բ������.
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
//  @brief          ����ͼ���жϵ��Ӻ����������ж��µ�
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
//  @brief          ����ͼ���жϵ��Ӻ����������ж϶�·����.
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
//  @brief          ����ͼ���жϵ��Ӻ����������ж�·������.
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
////        Steer_Flag=1; //��������
////        Angle_block = Yaw_Now;//������һ�δ��
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
////        Steer_Flag=1; //��������
//        ICM20602_Clear_Flag++;
//        RoadBlock_Flag++;
//    }
//}
//
//void Auto_RoadBlock_Through(void)
//{
//    if( imagestatus.Det_True >= 0 )
//    {
//        RoadBlock_Thruough_Flag = 1;  //��
//        if( imagestatus.Det_True > 3 )
//         { Auto_Through_RoadBlock = 1;  }
//        else
//            Auto_Through_RoadBlock = 0;
//    }
//    else
//    {
//        RoadBlock_Thruough_Flag = 2;   //��
//        if( imagestatus.Det_True < -3 )
//         { Auto_Through_RoadBlock = 1;  }
//        else
//            Auto_Through_RoadBlock = 0;
//    }
//}
//--------------------------------------------------------------
//  @name           Element_Handle_RoadBlock()
//  @brief          ����ͼ������Ӻ�������������·������.
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
//        {//(�����޸ĵ�һ�������Ǵ��)
//            if ( ((( Yaw_Now-Angle_block > 25 && !Auto_Through_RoadBlock ) || ( Yaw_Now-Angle_block > 20 && Auto_Through_RoadBlock )) &&
//                          RoadBlock_Flag == 1 && !Bend_Thruough_RoadBlock_Flag ) ||
//               ( ( Yaw_Now-Angle_block > 45 && RoadBlock_Flag == 1 &&  Bend_Thruough_RoadBlock_Flag ) ) )
//            { Steer_Flag = 0; RoadBlock_Flag++; /*Stop = 1;*/ } // ·�ϵ�һ�δ�ǣ����������ж�  25
//            if ( RoadBlock_Flag == 2 )
//            { // ·�ϵڶ��Σ��ñ���������
//                RoadBlock_length = (Element_encoder1+Element_encoder2)/2;
//                if ( ( (( RoadBlock_length > (1123+RoadBlock_Length_Compensate) && !Auto_Through_RoadBlock ) || ( RoadBlock_length > 987 && Auto_Through_RoadBlock ))
//                        && !Bend_Thruough_RoadBlock_Flag ) || (RoadBlock_length > 678 && Bend_Thruough_RoadBlock_Flag) )
//                { RoadBlock_Flag++; Steer_Flag = 1;RoadBlock_length = 0; /*Stop = 1;*/ }//  �����޸ı���������  3500 1970
//                if(RoadBlock_Flag == 3 && !block_flag)   //�����ǵڶ��δ��
//                {
//                    Angle_block = Yaw_Now;
//                    block_flag = 1;
//                }
//            }
//            if( (((( Angle_block-Yaw_Now > 45 && !Auto_Through_RoadBlock ) || ( Angle_block-Yaw_Now > 40 && Auto_Through_RoadBlock )) &&
//                    RoadBlock_Flag == 3 && !Bend_Thruough_RoadBlock_Flag)  ||
//                 (Angle_block-Yaw_Now > 45 && RoadBlock_Flag == 3 &&  Bend_Thruough_RoadBlock_Flag)) || RoadBlock_Flag > 3 )  //�޸ĵ����������Ǵ��  50
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
//                if(Gray_Value>270)//���300
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
//        {//(�����޸ĵ�һ�������Ǵ��)
//            if ( ((( Angle_block-Yaw_Now > 25 && !Auto_Through_RoadBlock ) || ( Angle_block-Yaw_Now > 20 && Auto_Through_RoadBlock )) &&
//                                  RoadBlock_Flag == 1 && !Bend_Thruough_RoadBlock_Flag ) ||
//                       ( ( Angle_block-Yaw_Now > 45 && RoadBlock_Flag == 1 &&  Bend_Thruough_RoadBlock_Flag ) ) )
//            { Steer_Flag = 0; RoadBlock_Flag++; /*Stop = 1;*/ } // ·�ϵ�һ�δ�ǣ����������ж�
//            if ( RoadBlock_Flag == 2 )
//            { // ·�ϵڶ��Σ��ñ���������
//                RoadBlock_length = (Element_encoder1+Element_encoder2)/2;
//                if ( ( (( RoadBlock_length > (1423+RoadBlock_Length_Compensate) && !Auto_Through_RoadBlock ) || ( RoadBlock_length > 987 && Auto_Through_RoadBlock ))
//                                    && !Bend_Thruough_RoadBlock_Flag ) || (RoadBlock_length > 789 && Bend_Thruough_RoadBlock_Flag) )
//                { RoadBlock_Flag++; Steer_Flag = 1;RoadBlock_length = 0; /*Stop = 1;*/ }//  �����޸ı���������  3500 1970
//                if(RoadBlock_Flag == 3 && !block_flag)   //�����ǵڶ��δ��
//                {
//                    Angle_block = Yaw_Now;
//                    block_flag = 1;
//                }
//            }
//            if( (((( Yaw_Now-Angle_block > 45 && !Auto_Through_RoadBlock ) || ( Yaw_Now-Angle_block > 40 && Auto_Through_RoadBlock )) &&
//                            RoadBlock_Flag == 3 && !Bend_Thruough_RoadBlock_Flag)  ||
//                         (Yaw_Now-Angle_block > 45 && RoadBlock_Flag == 3 &&  Bend_Thruough_RoadBlock_Flag)) || RoadBlock_Flag > 3 )  //�޸ĵ����������Ǵ��  50
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
//                if(Gray_Value>270)//���400
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
//  @brief          ����ͼ���жϵ��Ӻ����������жϰ�����
//  @parameter      void
//  @time
//  @Author         MRCHEN
//  Sample usage:   Element_Judgment_Zebra();
//--------------------------------------------------------------
void Element_Judgment_Zebra(void)//�������ж�
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
        if(imagestatus.Miss_Left_lines > (imagestatus.Miss_Right_lines + 3))//�󳵿�
        {
            imageflag.Zebra_Flag = 1;
            Garage_Location_Flag++;
            gpio_set_level(B0, 1);
        }
        if((imagestatus.Miss_Left_lines + 3)<imagestatus.Miss_Right_lines)//�ҳ���
        {
            imageflag.Zebra_Flag = 2;
            Garage_Location_Flag++;
            gpio_set_level(B0, 1);
        }
    }

}


//--------------------------------------------------------------
//  @name           Element_Handle_Zebra()
//  @brief          ����ͼ������Ӻ������������������
//  @parameter      void
//  @time
//  @Author         MRCHEN
//  Sample usage:   Element_Handle_Zebra();
//--------------------------------------------------------------
//int Zebra_length = 0;
void Element_Handle_Zebra()//�����ߴ���
{
//    Zebra_length=((Element_encoder1+Element_encoder2)/2);
    static uint8 Thought_flag=0;
    if(imageflag.Zebra_Flag == 1)//�󳵿�
    {
        for (int Ysite = 59; Ysite > imagestatus.OFFLineBoundary + 1; Ysite--)
        {
             Sideline_status_array[Ysite].midline = Sideline_status_array[Ysite].RightBoundary  - Half_Road_Wide[Ysite];
        }
    }
    else if(imageflag.Zebra_Flag == 2)//�ҳ���
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
//  @brief          ����ͼ������Ӻ��������������µ�
//  @parameter      void
//  @time
//  @Author         MRCHEN
//  Sample usage:   Element_Handle_OutRoad();
//--------------------------------------------------------------
void Element_Handle_OutRoad()//��·����
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
    if(Gray_Value > 360 && imagestatus.OFFLine < 20)//���400
    {
        imageflag.Out_Road=0;

    }
}

//--------------------------------------------------------------
//  @name           Element_Handle_Ramp()
//  @brief          ����ͼ������Ӻ��������������µ�
//  @parameter      void
//  @time
//  @Author         MRCHEN
//  Sample usage:   Element_Handle_Ramp();
//--------------------------------------------------------------
//uint Ramp_length = 0;
//void Element_Handle_Ramp()//�µ�����
//{
//     Ramp_length = ((Element_encoder1+Element_encoder2)/2);
//
//     if( Ramp_length > Ramp_num )//    170���µ�
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
// �������     ���߸�������
// ����˵��     y_up        �����ϵ�
// ����˵��     y_down      �����µ�
// ����˵��     x_up        �����ϵ�
// ����˵��     x_down      �����µ�
// ����˵��     mode        0������ߣ�1���ұ���
//---------------------------------------------------------------------
void connect_line_subsidiary(uint8 y_up,uint8 y_down,uint8 x_up,uint8 x_down,uint8 mode)
{

    DetL =((float)(x_up-x_down)) /((float)(y_up - y_down));  //��������ߵĲ���б��
        if(mode==0)
        {
            for (ytemp = y_down; ytemp >= y_up; ytemp--)               //��ô�Ҿʹӵ�һ��ɨ������߽������ڶ��е�λ�ÿ�ʼ����һֱ���ߣ�����FTSite�С�
                   {
                       Sideline_status_array[ytemp].leftline =(int)(DetL * ((float)(ytemp - y_down))) +Sideline_status_array[y_down].leftline;     //������Ǿ���Ĳ��߲�����
                   }
        }
        else
        {
            for (ytemp = y_down; ytemp >= y_up; ytemp--)               //��ô�Ҿʹӵ�һ��ɨ������߽������ڶ��е�λ�ÿ�ʼ����һֱ���ߣ�����FTSite�С�
            {
                Sideline_status_array[ytemp].rightline =(int)(DetL * ((float)(ytemp - y_down))) +Sideline_status_array[y_down].rightline;     //������Ǿ���Ĳ��߲�����
            }
        }
}


//--------------------------------------------------------------
//  @name           Element_Handle_Left_Rings()
//  @brief          ����ͼ������Ӻ���������������Բ������.
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
        for(uint8 i=55;i>imagestatus.OFFLine;i--)//Ѱ�ҹյ�
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

       if(imageflag.image_element_rings_flag == 1 && Point_Ysite > 30)//��������Բ���м����ĵ�ʱ������Ǵ�Բ��
       {
           imageflag.image_element_rings_flag=2;
       }

       /*���ߴ���*/

       if(imageflag.image_element_rings_flag == 2)
       {
           connect_line_subsidiary(Point_Ysite,59,Point_Xsite,79,1);
       }
       /*���ߴ���*/
       /*Բ���ڴ���*/
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
       /*Բ���ڴ���*/
       /*������*/
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
    if (imageflag.image_element_rings_flag == 2 && num<10)
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
            /********/
//            Front_Wait_After_Enter_Ring_Count++;
            gpio_set_level(B0, 0);
        }
    }



    /***************************************����**************************************/
        //׼������  �����
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
        for(Ysite=55;Ysite>imagestatus.OFFLine;Ysite--)//���满��
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
        //����
        if(flag_Ysite_1 != 0)
        {
            for(Ysite=flag_Ysite_1;Ysite<60;Ysite++)
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
            for(Ysite=flag_Ysite_1-1;Ysite>10;Ysite--) //A���Ϸ�����ɨ��
            {
                for(Xsite=Sideline_status_array[Ysite+1].rightline-10;Xsite<Sideline_status_array[Ysite+1].rightline+2;Xsite++)
                {
                    if(ex_mt9v03x_binarizeImage[Ysite][Xsite]!= 0 && ex_mt9v03x_binarizeImage[Ysite][Xsite+1]==0)
                    {
                        Sideline_status_array[Ysite].rightline=Xsite;
                        //if(imageflag.ring_big_small==1)//��Բ���������
                            Sideline_status_array[Ysite].midline = (Sideline_status_array[Ysite].rightline + Sideline_status_array[Ysite].leftline)/2;
                        //else//СԲ�������
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
            if (ex_mt9v03x_binarizeImage[Ysite][23] != 0 && ex_mt9v03x_binarizeImage[Ysite-1][23] == 0)//28
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
            if (ex_mt9v03x_binarizeImage[Ysite][15] != 0 && ex_mt9v03x_binarizeImage[Ysite-1][15] == 0)
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
        //�ѳ��� �����
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
//  @brief          ����ͼ������Ӻ���������������Բ������.
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
    if (imageflag.image_element_rings_flag == 2 && num<10)
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
            /********/
//            Front_Wait_After_Enter_Ring_Count++;
            gpio_set_level(B0, 0);
        }
    }
    /***************************************����**************************************/
         //׼������  �����
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
        //����
        if(flag_Ysite_1!=0)
        {
            for(Ysite=flag_Ysite_1;Ysite<60;Ysite++)
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
            for(Ysite=flag_Ysite_1-1;Ysite>10;Ysite--) //A���Ϸ�����ɨ��
            {
                for(Xsite=Sideline_status_array[Ysite+1].leftline+8;Xsite>Sideline_status_array[Ysite+1].leftline-4;Xsite--)
                {
                    if(ex_mt9v03x_binarizeImage[Ysite][Xsite]!= 0 && ex_mt9v03x_binarizeImage[Ysite][Xsite-1]==0)
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
            if (ex_mt9v03x_binarizeImage[Ysite][57] != 0 && ex_mt9v03x_binarizeImage[Ysite-1][57] == 0)
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
            if (ex_mt9v03x_binarizeImage[Ysite][58] != 0 && ex_mt9v03x_binarizeImage[Ysite-1][58] == 0)
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
        //�ѳ��� �����
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
//  @brief          ����ͼ������Ӻ��������������������
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

/*Ԫ���жϺ���*/
void Scan_Element()
{
    if (       imageflag.Out_Road == 0   && imageflag.RoadBlock_Flag == 0
            && imageflag.Zebra_Flag == 0 && imageflag.image_element_rings == 0
            && imageflag.Ramp == 0       && imageflag.Bend_Road == 0
            &&  imageflag.straight_long== 0  )
    {
//        Element_Judgment_RoadBlock();       //·��
//        Element_Judgment_OutRoad();         //��·
        Element_Judgment_Left_Rings();      //��Բ��
        Element_Judgment_Right_Rings();     //��Բ��
        Element_Judgment_Zebra();           //������
        Element_Judgment_Bend();            //���
        Straight_long_judge();              //��ֱ��
    }
    if(imageflag.Bend_Road)
    {
        Element_Judgment_OutRoad();         //��·
        if(imageflag.Out_Road)
            imageflag.Bend_Road=0;
    }

    if(imageflag.Bend_Road)
    {
        Element_Judgment_Left_Rings();      //��Բ��
        Element_Judgment_Right_Rings();     //��Բ��
        if(imageflag.image_element_rings)
        {
            imageflag.Bend_Road=0;
        }
    }
//    if(imageflag.Bend_Road)
//    {
////        Element_Judgment_RoadBlock();         //·��
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
        Element_Judgment_Zebra();           //������
        if(imageflag.Zebra_Flag)
            imageflag.Bend_Road=0;
    }
}

/*Ԫ�ش�����*/
void Element_Handle()
{
    //Element_GetStraightLine_Bend();//ֱ������

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
    else if(imagestatus.WhiteLine >= 8) //ʮ�ִ���
        Get_ExtensionLine();
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//  @name           Flag_init
//  @brief          ��־λ��0
//  @parameter      void
//  @time           2023��2��19��
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
//  �������        һ��ȫ��ʼ��
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
void Init_overall(void)
{
    Image_CompressInit();
    Flag_init();
    mt9v03x_init();

}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//  @name           Image_Process
//  @brief          ����ͼ�������������������������е�ͼ�����Ӻ���
//  @parameter      void
//  @time           2023��2��19��
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
            Search_Border_OTSU(ex_mt9v03x_binarizeImage, LCDH, LCDW, LCDH - 2);//(MT9V03X_H/2-2)��λ����
        else
            imagestatus.OFFLineBoundary = 5;
        Scan_Element();
        Element_Handle();
        mt9v03x_finish_flag=0;
       }
}

