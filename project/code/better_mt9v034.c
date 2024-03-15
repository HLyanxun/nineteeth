/*
 * better_mt9v034.c
 *
 *  Created on: 2024��1��20��
 *      Author: pc
 */
#include "zf_common_headfile.h"
#include "math.h"
uint8 ex_mt9v03x_image_binaryzation[MT9V03X_H][MT9V03X_W];
uint8 ex_mt9v03x_image_binaryzation_flag;
int ex_binaryzation_conduct_adjustment=0;
uint16 ex_midline[MT9V03X_H];
uint16 ex_leftline[MT9V03X_H];
uint16 ex_rightline[MT9V03X_H];
uint8 ex_roundabout_type;   //1�󻷵�2Ϊ�һ���
uint8 ex_roundabout_state;
uint8 ex_zebra_crossing_flag;
uint8 ex_zebra_pass_count;
uint8 ex_crossroad_flag;
uint8 ex_crossroad_state;
//-------------------------------------------------------------------------------------------------------------------
// �������     ͨ�����Ǻ�������Ƕ�
//-------------------------------------------------------------------------------------------------------------------
float arctan(float x,float y)
{
    float temp;
    temp=atan(x/y);
    temp=(180/pi)*temp;
    return temp;
}
//-------------------------------------------------------------------------------------------------------------------
// �������      ͨ����򷨷�����ֵ��������ͷͼ���ֵ���������Ե�����ֵ
// ��ע��Ϣ      �ô�򷨼������ֵ������ô�򷨼���ִ�д�������ÿ10ִ֡��һ��ˢ�£��ԻҶ�ͼ��̶���ֵ�Ķ�ֵ������,������ֵ����0��������ֵ����255
//-------------------------------------------------------------------------------------------------------------------
void mt9v034_binaryzation_conduct(void)
{

    uint8 temp=0;
    int8 value=0;
    value=binaryzation_value();                                         //��ȡ��򷨼��������ֵ
    for(uint8 i=0; i<MT9V03X_H; i++)
    {
        for(uint8 j=0; j<MT9V03X_W; j++)
        {
            temp=mt9v03x_image[i][j];
            if(temp<=(value+ex_binaryzation_conduct_adjustment))                    //�ж������Ƿ������ֵ����Ϊ�ǣ�������0����ɫ������Ϊ��������255����ɫ��
            {
                ex_mt9v03x_image_binaryzation[i][j]=0;
            }
            else
            {
                ex_mt9v03x_image_binaryzation[i][j]=255;
            }
        }
    }
    ex_mt9v03x_image_binaryzation_flag=1;                               //���һ��ͼ���ֵ������󣬽���־λ��1
}

//--------------------------------------------------------------------------------------------------------------------
// �������     �������Ķ�ֵ��ͼ�������tft��Ļ��
// ��ע��Ϣ     ÿ���һ֡��ͼ��������һ��ͼ��ˢ�£�Ȼ�󽫱�־λ��0
//--------------------------------------------------------------------------------------------------------------------
void mt9v034_tft180_show(void)
{
    if(ex_mt9v03x_image_binaryzation_flag==1)
            {
                //tft180_show_gray_image(0, 0, ex_mt9v03x_image_binaryzation[0], MT9V03X_W, MT9V03X_H,MT9V03X_W/2, MT9V03X_H/2, 0);
                tft180_displayimage03x(ex_mt9v03x_image_binaryzation[0],MT9V03X_W/2, MT9V03X_H/2);       //tft��Ļ��ʾ��ֵ�������Ļ
                ex_mt9v03x_image_binaryzation_flag=0;            //����ֵ����ɱ�־λ��0
            }
}
//--------------------------------------------------------------------------------------------------------------------
// �������     �������������ӽ���ͼ��Ķ�ֵ������(�������⣬��ʱ�޷�ʹ��)
//--------------------------------------------------------------------------------------------------------------------
void sobe_binaryzation_conduct(void)
{
    int mt9v03x_output_image[MT9V03X_H][MT9V03X_W];
    sobelAutoThreshold(mt9v03x_image[0],mt9v03x_output_image[0]);
    tft180_show_binary_image(0,0,mt9v03x_output_image[0],MT9V03X_W,MT9V03X_H,(MT9V03X_W/2),(MT9V03X_H/2));
}
//--------------------------------------------------------------------------------------------------------------------
// �������     ֱ����������Ϊ��ֵ�Ķ�ֵ��ͼ��,ͨ������ex_binaryzation_conduct_adjustment���ڶ�ֵ����ֵ
// ��ע��Ϣ     ��������¼�봢�����飬����ʾ��ֵ��ͼ�񣬽����ڵ��ԺͶԱ�
//--------------------------------------------------------------------------------------------------------------------
void mt9v034_tft180_dajin_show(void)
{
    tft180_show_gray_image(0, 0, mt9v03x_image[0],MT9V03X_W, MT9V03X_H, MT9V03X_W/2, MT9V03X_H/2,(binaryzation_value()+ex_binaryzation_conduct_adjustment));
}

//----------------------------------------------------------------------------------------------------------------------
// �������     ͨ����򷨼����ֵ����ֵ
// ����ֵ         ��ֵ����ֵ
// ��ע��Ϣ     ͨ����򷨳����Ż��Ķ�ֵ����ֵ�㷨���������иĽ��ռ�
//----------------------------------------------------------------------------------------------------------------------

uint8 binaryzation_value(void)
{

    int GrayScale=256;//����256���Ҷȼ�
    int pixelCount[GrayScale];//ÿ���Ҷ�ֵ��ռ���ظ���
    float pixelPro[GrayScale];//ÿ���Ҷ�ֵ��ռ�����ر���
    int i,j,pixelSum=(MT9V03X_H*MT9V03X_W/4);//�����ص�
    static int8 threshold=0;//�����ֵ
    int gray_sum=0;//���ص�Ҷ��ܺ�
    memset(pixelCount, 0, GrayScale); //ʹ��memset������ʼ������
    memset(pixelPro, 0, GrayScale); //ʹ��memset������ʼ������
    static uint8 circulation_flag;
    if(circulation_flag>=3)
    {
    for(i=0;i<MT9V03X_H;i+=2)
    {
        for(j=0;j<MT9V03X_W;j+=2)
        {
            pixelCount[mt9v03x_image[i][j]]++;      //��Ҷȷֲ�
            gray_sum+=mt9v03x_image[i][j];          //�������ص�ĻҶ��ܺ�
        }
    }

        float w0,w1,u0Sum,u1Sum,u0,u1,u,variance=0, maxVariance = 0;
        w0=w1=u0Sum=u1Sum=u0=u1=u=0;
        //Ŀ������ռ��ͼ�����w0
        //����ͼ��ռ��ͼ�����w1
        //Ŀ��ƽ���Ҷ���ֵu0
        //����ƽ���Ҷ���ֵu1
        //��ƽ���Ҷ�ֵu
        //��䷽��variance
        //�����䷽��maxVariance
        //����ÿ������������ͼ���еı���
        for(i=0;i<GrayScale;i++)
                {
                    pixelPro[i]=(float)pixelCount[i]/pixelSum;      //�Ҷȷֲ���������������
                    u=i*pixelPro[i];//��ƽ���Ҷ�
                }
        for (i = 0; i < GrayScale; i++)     // i��Ϊ��ֵ ��ֵ��1-255����
        {

                   for (j = 0; j < GrayScale; j++)    //��Ŀ��ǰ���ͱ���
                   {
                       if (j <= i)   //ǰ������
                       {
                           w0 += pixelPro[j];
                           u0Sum += j * pixelPro[j];
                       }
                       else   //��������
                       {
                           w1 += pixelPro[j];
                           u1Sum += j * pixelPro[j];
                       }
                   }
                   u0 = u0Sum / w0;
                   u1 = u1Sum / w1;
                   variance = w0 * pow((u0 - u), 2) + w1 * pow((u1 - u), 2);//��䷽����㹫ʽ
            if(variance>maxVariance)
            {
                maxVariance=variance;
                threshold=(int)i;
            }
            if(variance<maxVariance)
            {

                break;
            }

        }
    }
        return threshold;
}

//---------------------------------------------------------------------------------------------------------------
// �������                 ͨ�����������ӽ��ж�ֵ��ͼ����
// ����˵��                 source          Դͼ������ͷֱ�Ӳɼ���ͼ��
// ����˵��                 target          �����Ե��ֵ��ͼ��
// ʹ��ʵ�����Ʋ⣩  sobeAutoThreshold(mt9v30x_image[0],mt9v30x_output_image[0]);
// ��ע��Ϣ                 �������������ӵı�Ե�����㷨��ռ������һ�㣬����ڴ�򷨵ļ��㣬�ڲ����ȹ��������Ÿ��õı�Եʶ����֣�ԭ�����ͼ�������㡣
//---------------------------------------------------------------------------------------------------------------
void sobelAutoThreshold (u8 source[MT9V03X_H/2][MT9V03X_W],u8 target[MT9V03X_H/2][MT9V03X_W])
{
    /** ����˴�С */
    short KERNEL_SIZE = 3;
    short xStart = KERNEL_SIZE / 2;
    short xEnd = MT9V03X_W - KERNEL_SIZE / 2;
    short yStart = KERNEL_SIZE / 2;
    short yEnd = MT9V03X_H - KERNEL_SIZE / 2;
    short i, j, k;
    short temp[3];
    for (i = yStart; i < yEnd; i++)
    {
        for (j = xStart; j < xEnd; j++)
        {
            /* ���㲻ͬ�����ݶȷ�ֵ  */
            temp[0] = -(short) source[i - 1][j - 1] + (short) source[i - 1][j + 1]      // {-1, 0, 1},
            - (short) source[i][j - 1] + (short) source[i][j + 1]                     // {-1, 0, 1},
            - (short) source[i + 1][j - 1] + (short) source[i + 1][j + 1];             // {-1, 0, 1};

            temp[1] = -(short) source[i - 1][j - 1] + (short) source[i + 1][j - 1]      // {-1, -1, -1},
            - (short) source[i - 1][j] + (short) source[i + 1][j]                      // { 0,  0,  0},
            - (short) source[i - 1][j + 1] + (short) source[i + 1][j + 1];             // { 1,  1,  1};

            temp[2] = -(short) source[i - 1][j] + (short) source[i][j - 1]              //  {0, -1, -1},
            - (short) source[i][j + 1] + (short) source[i + 1][j]                       //  {1,  0, -1},
            - (short) source[i - 1][j + 1] + (short) source[i + 1][j - 1];              //  {1,  1,  0};

            temp[3] = -(short) source[i - 1][j] + (short) source[i][j + 1]               // {-1, -1,  0},
            - (short) source[i][j - 1] + (short) source[i + 1][j]                       // {-1,  0,  1},
            - (short) source[i - 1][j - 1] + (short) source[i + 1][j + 1];              //  {0,  1,  1};

            temp[0] = abs(temp[0]);
            temp[1] = abs(temp[1]);
            temp[2] = abs(temp[2]);
            temp[3] = abs(temp[3]);

            /* �ҳ��ݶȷ�ֵ���ֵ  */
            for (k = 1; k < 3; k++)
            {
                if (temp[0] < temp[k])
                {
                    temp[0] = temp[k];
                }
            }

            /* ʹ�����ص����������ص�֮�͵�һ������    ��Ϊ��ֵ  */
            temp[3] =
                    (short) source[i - 1][j - 1] + (short) source[i - 1][j] + (short) source[i - 1][j + 1]
                    + (short) source[i][j - 1] + (short) source[i][j] + (short) source[i][j + 1]
                    + (short) source[i + 1][j - 1] + (short) source[i + 1][j] + (short) source[i + 1][j + 1];

            if (temp[0] > (temp[3] / 12.0f))
            {
                target[i][j] = 0xFF;
            }
            else
            {
                target[i][j] = 0x00;
            }
        }
    }
}
//-------------------------------------------------------------------------------------------------------------------------------
// �������     �������ʽ�ع�ϵ��
// ����˵��    d_X  ��������ݵ�xֵ
// ����˵��     d_Y   ��������ݵ�yֵ
// ����˵��     d_N   ��������ݵĸ���
// ����˵��     rank  ����ʽ�Ĵ���
// ����˵��     coeff �����ϵ��
// ��ע��Ϣ    doubleռ��8�ֽڣ�floatռ��4�ֽڣ����Կ����Ż�
//-------------------------------------------------------------------------------------------------------------------------------
void polyfit(double *d_X, double *d_Y, int d_N, int rank, double *coeff)
{
//    int rank;
//    rank=RANK_;
       if(RANK_!=rank)          //�жϴ����Ƿ�Ϸ�
           return;

    int i,j,k;
    double aT_A[RANK_ + 1][RANK_ + 1] = {0};
    double aT_Y[RANK_ + 1] = {0};


    for(i = 0; i < rank + 1; i++)   //��
    {
        for(j = 0; j < rank + 1; j++)   //��
        {
            for(k = 0; k < d_N; k++)
            {
                aT_A[i][j] +=  pow(d_X[k], i+j);        //At * A ���Ծ���
            }
        }
    }

    for(i = 0; i < rank + 1; i++)
    {
        for(k = 0; k < d_N; k++)
        {
            aT_Y[i] += pow(d_X[k], i) * d_Y[k];     //At * Y ���Ծ���
        }
    }

    //����Ϊ��˹����Ԫ����ȥ�������Է�����
    for(k = 0; k < rank + 1 - 1; k++)
    {
        int row = k;
        double mainElement = fabs(aT_A[k][k]);
        double temp = 0.0;

        //����Ԫ��
        for(i = k + 1; i < rank + 1 - 1; i++)
        {
            if( fabs(aT_A[i][i]) > mainElement )
            {
                mainElement = fabs(aT_A[i][i]);
                row = i;
            }
        }

        //��������
        if(row != k)
        {
            for(i = 0; i < rank + 1; i++)
            {
                temp = aT_A[k][i];
                aT_A[k][i] = aT_A[row][i];
                aT_A[row][i] = temp;
            }
            temp = aT_Y[k];
            aT_Y[k] = aT_Y[row];
            aT_Y[row] = temp;
        }


        //��Ԫ����
        for(i = k + 1; i < rank + 1; i++)
        {
            for(j = k + 1; j < rank + 1; j++)
            {
                aT_A[i][j] -= aT_A[k][j] * aT_A[i][k] / aT_A[k][k];
            }
            aT_Y[i] -= aT_Y[k] * aT_A[i][k] / aT_A[k][k];
        }
    }

    //�ش�����
    for(i = rank + 1 - 1; i >= 0; coeff[i] /= aT_A[i][i], i--)
    {
        for(j = i + 1, coeff[i] = aT_Y[i]; j < rank + 1; j++)
        {
            coeff[i] -= aT_A[i][j] * coeff[j];
        }
    }

    return;
}

//--------------------------------------------------------------------------------------------------------------------------
// �������     ����Ѳ�ߣ���������Ϣ��¼��ex_leftline,ex_midline,ex_rightline������
// ʹ��ʵ��     Horizontal_line()
// ��ע��Ϣ     ����������⣬�������һ�Ե���Row��Col����ֵ
//--------------------------------------------------------------------------------------------------------------------------
void horizontal_line(void)
{
  uint8 i,j;
  if(ex_mt9v03x_image_binaryzation[Row-1][Col/2]==0)                        //�ж�����������Ƿ�Ϊ��ɫ������ʼ�Ƿ���������
  {
    if(ex_mt9v03x_image_binaryzation[Row-1][5]==255)                      //�ж�ͼ������һ�У��������Ƿ�Ϊ��ɫ��ͼ�����½ǣ�
      ex_midline[Row]=5;                               //��Ԥ�����ֶ�Ϊ5
    else if(ex_mt9v03x_image_binaryzation[Row-1][Col-5]==255)
      ex_midline[Row]=Col-5;
    else
      ex_midline[Row]=Col/2;
  }
  else
    {
    ex_midline[Row]=Col/2;                             //����ǣ���ô�����ߵ�������¼Ϊ�����Ķ���֮һ����������ߣ�
    }

  for(i=Row-1;i>0;i--)
  {
    for(j=ex_midline[i+1];j>=0;j--)
    {
      if(ex_mt9v03x_image_binaryzation[i][j]==0||j==0)
      {
        ex_leftline[i]=j;                              //������趨Ϊi���Ҵ�������j��
        break;
      }
    }
    for(j=ex_midline[i+1];j<=Col-1;j++)
    {
      if(ex_mt9v03x_image_binaryzation[i][j]==0||j==Col-1)
      {
        ex_rightline[i]=j;
        break;
      }
    }
    ex_midline[i]=(ex_leftline[i]+ex_rightline[i])/2;   //�����ұ��ߵ��е���Ϊ��������
   if(ex_mt9v03x_image_binaryzation[i-1][ex_midline[i]]==0||i==0)
    {
   for(j=i;j>0;j--)
   {
    ex_midline[j]=ex_midline[i];
    ex_leftline[j]=ex_midline[i];
    ex_rightline[j]=ex_midline[i];
   }
      break;
    }
  }
}
//--------------------------------------------------------------------------------------------------------------------------
// �������     �Ҳ�ͼ���߼��
// ����ֵ       -1Ϊδ���� ����Ϊ������ʼ��
// ʹ��ʵ��     Lost_line_right();
//--------------------------------------------------------------------------------------------------------------------------
int8 lost_line_right(void)
{
  uint8 i;
  for(i=find_end;i>find_start;i--)
  if(ex_rightline[i]==(MT9V03X_W-1))     return i;
  return -1;
}
//--------------------------------------------------------------------------------------------------------------------------
// �������     ���ͼ���߼��
// ����ֵ       -1Ϊδ���� ����Ϊ������ʼ��
// ʹ��ʵ��     Lost_line_left();
//--------------------------------------------------------------------------------------------------------------------------
int8 lost_line_left(void)
{
    uint8 i;
  for(i=find_end;i>find_start;i--)
  if(ex_leftline[i]==0) return i;
  return -1;
}
//--------------------------------------------------------------------------------------------------------------------------
// �������     �յ���
// ����˵�� select      ѡ������Ĺյ�1���£�2���ϣ�3���£�4����
// ����˵�� location    ѡ�����x����y����,1Ϊx��2Ϊy
// ����ֵ          -1Ϊû�м�⵽�յ㣬-2��⵽�յ㣬�����Ѿ����ߣ��������عյ�����
//--------------------------------------------------------------------------------------------------------------------------
int16 change_detection(uint8 select,uint8 location)
{
    uint8 i;
    if(select==1)
    {
        for(i=find_end;i>find_start;i--)
            {
            if(ex_leftline[i]==0)
                {

                    if(location==2)
                    {
                        return (i+1);
                    }

                    if(location==1)
                    {
                        if(ex_leftline[i+1]!=0)return ex_leftline[i+1];
                        return -2;
                    }
                }

            }
    }
    if(select==2)
    {
        for(i=find_start;i<find_end;i++)
           {
               if(ex_leftline[i]==0)
               {
                   if(location==2)
                   {
                       return (i-1);
                   }

                   if(location==1)
                   {
                       if(ex_leftline[i-1]!=0)return ex_leftline[i-1];
                       return -2;
                   }
               }
           }
    }
   if(select==3)
   {
       for(i=find_end;i>find_start;i--)
                   {
                   if(ex_rightline[i]==0)
                       {

                           if(location==2)
                           {
                               return (i+1);
                           }

                           if(location==1)
                           {
                               if(ex_rightline[i+1]!=0)return ex_rightline[i+1];
                               return -2;
                           }
                       }

                   }
   }

   if(select==4)
   {
        for(i=find_start;i<find_end;i++)
                   {
                       if(ex_leftline[i]==0)
                       {
                           if(location==2)
                           {
                               return (i-1);
                           }

                           if(location==1)
                           {
                               if(ex_leftline[i-1]!=0)return ex_leftline[i-1];
                               return -2;
                           }
                       }
                   }
}

    return -1;
}
//-------------------------------------------------------------------------------------------------------------------------
// �������     ����������ж��������е㼦�ߣ�����ò��ϣ�
// ����ֵ         -1���������1Ϊ��ת��2Ϊ��ת
//-------------------------------------------------------------------------------------------------------------------------
int8 curve_detection(void)
{
    int8 left=0,right=0;
    left=lost_line_left();
    right=lost_line_right();
    if(left!=-1 && right==-1 && straight_line_judgment!=1)return 1;
    if(left==-1 && right!=-1 && straight_line_judgment!=1)return 2;
    return -1;
}

//--------------------------------------------------------------------------------------------------------------------------
// �������     ʮ��·�ڼ��
// ����ֵ          �����ǰ����ʮ��·�ڣ��򷵻�1�����򷵻�-1��
// ʹ��ʵ��     crossroad_detection();
//--------------------------------------------------------------------------------------------------------------------------
int8 crossroad_detection(void)
{
    int8 left_down=0,right_down=0,left_up=0;
    uint8 i,j,crossroad_flag=0,crossroad=0;
    uint temp=0;
    left_down=change_detection(1,2);
    right_down=change_detection(3,2);
    if(left_down!=-1 && right_down!=-1 && ex_crossroad_flag!=1)           //�ж����߶�����
    {
        left_up=change_detection(2, 2);
        for(i=(left_down-5);i>(left_up+5);i--)    //��������Ƿ���ڴ����׵�
        {
            for(j=72;j>24;j--)
            {
                 if(ex_mt9v03x_image_binaryzation[i][j]==255)temp++;
                 if(temp>200)temp=100;
            }
        }
        if(temp<50) ex_crossroad_flag=1;            //������ڴ����׵�,����Ϊ����ʮ��·��
    }
    if(ex_crossroad_flag==1){crossroad=1;}else{crossroad=-1;}
    return crossroad;
}
//--------------------------------------------------------------------------------------------------------------------------
// �������     ʮ��·�ڴ���
// ʹ��ʵ��     crossroad_conduct();
//--------------------------------------------------------------------------------------------------------------------------

void crossroad_conduct(void)
{
    if(ex_crossroad_flag=1)//����Ѿ�����ʮ��·��
    {
        switch(ex_crossroad_state)//ʮ��·��״̬�ж�
        {
        case 1:
            if(lost_line_left()==-1 && lost_line_right==-1)ex_crossroad_state=2;//����ʮ��·�����
            break;
        case 2:
            if(lost_line_left()!=-1 && lost_line_right!=-1)ex_crossroad_state=3;//��ʮ��·��
            break;
        case 3:
            if(lost_line_left()==-1 && lost_line_right==-1){ex_crossroad_state=0;ex_crossroad_flag=0;}//�����־λ
            break;
        }
        if(ex_crossroad_state==1)//��ʮ��·�ڲ���
        {
            uint8 x1,y1,x2,y2;
            x1=change_detection(1,1);
            y1=change_detection(1,2);
            x2=change_detection(2,1);
            y2=change_detection(2,2);
            connect_line(x1,y1,x2,y2);
        }
        if(ex_crossroad_state==3)//��ʮ��·�ڲ���
        {
            uint8 x1,y1,x2,y2;
            x1=change_detection(1,1);
            y1=change_detection(1,2);
            x2=change_detection(2,1);
            y2=change_detection(2,2);
            connect_line(x1,y1,x2,y2);
        }
    }

}
//--------------------------------------------------------------------------------------------------------------------------
// �������     �����ж�
// ʹ��ʵ��     roundabout_detection();
//--------------------------------------------------------------------------------------------------------------------------
void roundabout_detection(void)
{
    if(lost_line_left()!=-1 || lost_line_right()!=-1)//����Ԫ��ʶ��
    {
        if(lost_line_left()!=-1 && lost_line_right()==-1)//�󻷵��ж�
        {
            if(straight_line_judgment(ex_rightline)==1)//�ų�����������ұ����Ƿ�Ϊֱ��
            {
                ex_roundabout_state=1;
                ex_roundabout_type=1;
            }
        }
        if(lost_line_left()==-1 && lost_line_right()!=-1)//�һ����ж�
        {
            if(straight_line_judgment(ex_leftline)==1)//�ų����������������Ƿ�Ϊֱ��
                        {
                            ex_roundabout_state=1;
                            ex_roundabout_type=2;
                        }
        }
    }
}
//--------------------------------------------------------------------------------------------------------------------------
// �������     ��������
// ʹ��ʵ��     roundabout_dispose();
//--------------------------------------------------------------------------------------------------------------------------

void roundabout_dispose(void)
{
    if(ex_roundabout_type==1)//�󻷵���־
    {
        switch(ex_roundabout_state)//�󻷵�״̬����
        {
        case 1:
            if(change_detection(2,1)>not_lose_line_parameter)//�����߼���������
            {
                ex_roundabout_state=2;//����״̬����
            }
            break;
        case 2:
            if(roundabout_annular_detection()==1)//�����ߴ���Բ��
            {
                ex_roundabout_state=3;//����״̬����
            }
            break;
        case 3:
            if(lost_line_left()!=-1)//�����ߵڶ��ζ��ߣ��ҹյ㲹�߽��뻷��
            {
                ex_roundabout_state=4;//����״̬����
            }
            break;
        case 4:
            if(lost_line_left()==-1 && lost_line_right()==-1)//������뻷���������ڣ����߶�������
            {
                ex_roundabout_state=5;//����״̬����
            }
            break;
        case 5:
            if(lost_line_left()!=-1 && lost_line_right()!=-1)//�������������ҹյ㲹�߳�����
            {
                ex_roundabout_state=6;//����״̬����
            }
            break;
        case 6:
            if((lost_line_right()-change_detection(4, 2))<=5 && smartcar_state_detection()==1)//�ұ߼��������ߣ��������
            {
                ex_roundabout_state=7;//����״̬����
            }
            break;
        case 7:
            if((lost_line_left()-change_detection(2, 2))<=5 && (lost_line_left()-change_detection(2, 2))!=0)//�����߼���������
            {
                ex_roundabout_state=8;//����״̬����
            }
            break;
        case 8:
            if(straight_line_judgment(ex_leftline)==1 && straight_line_judgment(ex_rightline)==1)//��ȫ������
            {
                ex_roundabout_state=0;//����״̬����
                ex_roundabout_type=0;//������뻷����־
            }
            break;
        }
        if(ex_roundabout_state==1 || ex_roundabout_state==2)//����ͨ��������һ�ζ��ߣ��ִﻷ�����
        {
            uint8 x1,y1,x2,y2;
            x1=change_detection(1, 1);
            x2=change_detection(2, 1);
            y1=change_detection(1, 2);
            y2=change_detection(2, 2);
            connect_line(x1,y1,x2,y2);
        }
        if(ex_roundabout_state==4)//���߽��뻷��
        {
            uint8 x1,y1;
            x1=change_detection(2, 1);
            y1=change_detection(2, 2);
            connect_line(x1,y1,(MT9V03X_W-5),(MT9V03X_H-5));
        }
        if(ex_roundabout_state==6)//���߳�����
        {
            uint8 x1,y1,y2;
            x1=change_detection(3, 1);
            y1=change_detection(3, 2);
            y2=change_detection(2, 2);
            connect_line(x1,y1,10,y2);
        }
        if(ex_roundabout_state==7)
        {
            uint8 x1,y1,x2=0,y2;
            x1=change_detection(2, 1);
            y1=change_detection(2, 2);
            for(uint8 i=(y1+1);i<find_start;i++)
            {
                if(ex_leftline[i]>x2)
                    {
                        x2=ex_leftline[i];
                        y2=i;
                    }
            }
            connect_line(x1,y1,x2,y2);
            connect_line(x2,y2,(x2-1),find_end);
        }
    }
    if(ex_roundabout_type==2)//�һ����������󻷵������ϱ�д������Ӧ��ֻ���󻷵�
    {
        switch(ex_roundabout_state)
        {
        case 1:
            if(change_detection(4,1)>not_lose_line_parameter)
            {
                ex_roundabout_state=2;
            }
            break;
        case 2:
            if(roundabout_annular_detection()==2)
            {
                ex_roundabout_state=3;
            }
            break;
        case 3:
            if(lost_line_right()!=-1)
            {
                ex_roundabout_state=4;
            }
            break;
        case 4:
            if(lost_line_left()==-1 && lost_line_right()==-1)
            {
                ex_roundabout_state=5;
            }
            break;
        case 5:
            if(lost_line_left()!=-1 && lost_line_right()!=-1)
            {
                ex_roundabout_state=6;
            }
            break;
        case 6:
            if((lost_line_left()-change_detection(2, 2))<=5 && smartcar_state_detection()==1)
            {
                ex_roundabout_state=7;
            }
            break;
        case 7:
            if((lost_line_right()-change_detection(4, 2))<=5 && (lost_line_right()-change_detection(4, 2))!=0)
            {
                ex_roundabout_state=8;
            }
            break;
        case 8:
            if(straight_line_judgment(ex_leftline)==1 && straight_line_judgment(ex_rightline)==1)//��ȫ������
            {
                ex_roundabout_state=0;
                ex_roundabout_type=0;
            }
            break;
        }
        if(ex_roundabout_state==1 || ex_roundabout_state==2)
        {
            uint8 x1,y1,x2,y2;
            x1=change_detection(3, 1);
            x2=change_detection(4, 1);
            y1=change_detection(3, 2);
            y2=change_detection(4, 2);
            connect_line(x1,y1,x2,y2);
        }
        if(ex_roundabout_state==4)
        {
            uint8 x1,y1;
            x1=change_detection(4, 1);
            y1=change_detection(4, 2);
            connect_line(x1,y1,5,(MT9V03X_H-5));
        }
        if(ex_roundabout_state==6)
        {
            uint8 x1,y1,y2;
            x1=change_detection(1, 1);
            y1=change_detection(1, 2);
            y2=change_detection(3, 2);
            connect_line(x1,y1,(MT9V03X_W-10),y2);
        }
        if(ex_roundabout_state==7)
        {
            uint8 x1,y1,x2=0,y2;
            x1=change_detection(4, 1);
            y1=change_detection(4, 2);
            for(uint8 i=(y1+1);i<find_start;i++)
            {
                if(ex_rightline[i]>x2)
                {
                    x2=ex_rightline[i];
                    y2=i;
                }
            }
            connect_line(x1,y1,x2,y2);
            connect_line(x2,y2,(x2+1),find_end);
        }
    }
}

//--------------------------------------------------------------------------------------------------------------------------
// �������     ����������
// ����ֵ         1���������,����smartcar_state_parament�ĽǶȽ������أ���ΧΪ-90��90
//--------------------------------------------------------------------------------------------------------------------------
int8 smartcar_state_detection(void)
{
    if(straight_line_judgment(ex_midline)==1)
    {
        uint8 temp;
        temp=ex_midline[20]-ex_midline[55];
        temp=arctan(temp,35);
        if(func_abs(temp)<smartcar_state_parament)return 1;
        return temp;
    }
}
//--------------------------------------------------------------------------------------------------------------------------
// �������     ���μ��
// ����ֵ         �Ƿ��⵽���Σ�1Ϊ���Σ�2Ϊ�һ��Σ�-1δ��⵽���Σ�
// ʹ��ʵ��     roundabout_annular_detection();
//--------------------------------------------------------------------------------------------------------------------------
int8 roundabout_annular_detection(void)
{
    uint8 temp_l=0,temp_r=0;
    uint8 increase=0,reduce=0;
    temp_l=lost_line_left();
    temp_r=lost_line_right();
    if((temp_l!=-1 && temp_r==-1)||(temp_l==-1 && temp_r!=-1))
    {
        if(temp_l!=-1)
        {
            for(uint8 i=temp_l;i>find_start;i--)
                {
                    if(ex_leftline[i]>ex_leftline[i+1] && ex_leftline[i-1]>ex_leftline[i])increase++;
                    if(ex_leftline[i]<ex_leftline[i+1] && ex_leftline[i-1]<ex_leftline[i])reduce++;
                    if(increase>=annular_peremeter && reduce>=annular_peremeter)
                        {
                            return 1;
                        }
                }
        }
        else
        {
            for(uint8 i=temp_r;i>find_start;i--)
                {
                if(ex_rightline[i]>ex_rightline[i+1] && ex_rightline[i-1]>ex_rightline[i])increase++;
                if(ex_rightline[i]<ex_rightline[i+1] && ex_rightline[i-1]<ex_rightline[i])reduce++;
                if(increase>=annular_peremeter && reduce>=annular_peremeter)
                {
                    return 2;
                }
                }
        }
    }
    return -1;
}
//--------------------------------------------------------------------------------------------------------------------------
// �������     ��ֱ�߽����ж�
// ����˵�� arr     ��������
// ����ֵ       1Ϊֱ�� 0Ϊ��ֱ��
// ʹ��ʵ��     Straight_line_judgment(left_arr);
// ��ע��Ϣ     ֻ�ж�30-100
//--------------------------------------------------------------------------------------------------------------------------
uint8 straight_line_judgment(uint16 arr[Row])
{
  short i,sum=0;
  float kk;
  kk=((float)arr[90]-(float)arr[20])/70.0;//����kֵ
  sum = 0;
  for(i=20;i<=90;i++)
    if(((arr[20]+(float)(i-20)*kk)-arr[i])<=35) sum++;//�������ֵ��ʵ��ֵ�Ĳ���С�ڵ���35�������
    else break;
  if(sum>68 && kk>-1.1 && kk<1.1) return 1;
  else return 0;
}
//-----------------------------------------------------------------------------------------------------------------------------
// �������     ���ߺ���
// ����˵�� x1  �жϵ�1��x����
// ����˵�� y1  �жϵ�1��y����
// ����˵�� x2  �жϵ�2��x����
// ����˵�� y2  �жϵ�2��y����
// ʹ��ʵ��     line(0,0,20,30);
//-----------------------------------------------------------------------------------------------------------------------------
void connect_line(uint8 x1,uint8 y1,uint8 x2,uint8 y2)
{
  short i,j,swap;
  float k;
  if(y1>y2)
  {
    swap = x1;
    x1 = x2;
    x2 = swap;
    swap = y1;
    y1 = y2;
    y2 = swap;
  }
  if(x1==x2)
  {
    for(i=y1;i<y2+1;i++)
        ex_mt9v03x_image_binaryzation[i][x1]=0;
  }
  else if(y1==y2)
  {
    for(i=x1;i<x2+1;i++)
        ex_mt9v03x_image_binaryzation[y1][i]=0;
  }
  else
  {
    k = ((float)x2-(float)x1)/((float)y2-(float)y1);
    for(i=y1;i<=y2;i++)
        ex_mt9v03x_image_binaryzation[i][(short)(x1+(i-y1)*k)]=0;
  }
}
//------------------------------------------------------------------------------------------------------------------------------
// �������     ������ʶ�𣨺�����Ҫ����ʵ������޸ģ�
//------------------------------------------------------------------------------------------------------------------------------
void zebra_crossing(void)
{
    for(uint8 hang = find_start;hang<find_end;hang++)
    {
        uint8 garage_count=0,white_black,black_white ;
        for(uint8 lie = 10;lie<100;lie++)
         {
               if(ex_mt9v03x_image_binaryzation[hang][lie]==255)//ͨ��ͻ�䣨�յ㣩�жϰ�����
               {
                   white_black=1;
               }
               else
               {
                   white_black=0;
               }

               if(white_black!=black_white)//����ǹյ㣬��԰����߽��м���
               {
                 black_white = white_black;
                 garage_count++;
               }
               if(garage_count>11)//������յ���ִ���������ֵȷ��Ϊ�����ߣ�ͬʱ�԰�����ͨ��������1
               {
                   ex_zebra_pass_count++;
               }
           }
        if(ex_zebra_pass_count>2)
           {
                ex_zebra_crossing_flag=1;
               break;
           }
       }
}
