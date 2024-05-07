/*
 * Camera_Process.c
 *
 *  Created on: 2024��4��29��
 *      Author: pc
 */
#include "zf_common_headfile.h"
#include "math.h"

#define ImageSensorMid (39)                    //ͼ�����Ļ�е�
#define track_width (28)
#define island_in_limit_x  55 //�����������Ƶ�λ
#define USE_num image_h*3   //�����ҵ�������Ա��������˵300�����ܷ��£�������Щ�������ȷʵ�Ѷ����ඨ����һ��

int16 threshold_fix;                                          //��ֵ����ֵ����

//static uint8* PicTemp;                          //һ�����浥��ͼ���ָ�����
uint8 l_shade_255=0,r_shade_255=0; //����Ҳ���Ӱ  Ҳ���� ��ɫ����
uint16 l_data_statics=0;//ͳ������ܹ��ĳ���
uint16 r_data_statics=0;//ͳ���ұ��ܹ��ĳ���

uint8 Find_Boundary_l[300][2]={{0}};//��߽��� 0Ϊy  1Ϊx �洢����Ϊ������ɨ������ı߽�
uint8 Find_Boundary_r[300][2]={{0}};//�ұ߽��� 0Ϊy  1Ϊx �洢����Ϊ������ɨ������ı߽�
uint8 Find_Boundary_z[300][2]={{0}};//�б߽��� 0Ϊy  1Ϊx �洢����Ϊ������ɨ������ı߽�
uint8 Find_Boundary_centre[300][2]={{0}};//���� 0Ϊy  1Ϊx �洢����Ϊ������ɨ������ı߽�

uint8 l_Down[4]={0},l_on[4]={0},r_Down[4]={0},r_on[4]={0}; //ʮ�ֵ��ĸ��յ� 0y1x
uint8 is_L_on[3]={0},is_R_on[3]={0};  //���� ���� ���� �����յ�0y1x
uint8 is_L_down[3]={0},is_R_down[3]={0};  //���� ���� ���� �����յ�0y1x

uint8 ex_mt9v03x_binarizeImage[image_h][image_w]={{0}};             //��������ͼ���������

//��͸��---------------------------------------------------------------------------------------------------------------
uint8 *PerImg_ip[RESULT_ROW][RESULT_COL]={{0}};//�洢ͼ���ַ

//Sideline_status Sideline_status_array[image_h];
Image_Status imagestatus;
ImageFlagtypedef imageflag;

//-------------------------------------------------------------------------------------------------------------------
// �������     ImagePerspective_Init ��͸�ӳ�ʼ�� ��ʼ����һ�ξͿ�����
// ����˵��
// ���ز���
// ʹ��ʾ��
// ��ע��Ϣ  /*�������ͷ��ʼ���󣬵���һ��ImagePerspective_Init���˺�ֱ�ӵ���ImageUsed   ��Ϊ͸�ӽ��*/
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
// �������         ��ֵ��
// ����˵��         void
// ���ز���         void
// ʹ��ʾ��         Binaryzation();
// ��ע��Ϣ
//-------------------------------------------------------------------------------------------------------------------
uint8 My_Threshold=0;//��ֵ�������ֵ
void Binaryzation(void)
{
    My_Threshold = (int)my_adapt_threshold(mt9v03x_image[0],180, 248 )+(int)threshold_fix;//��򷨼������ֵ���������ֵ����ֵ
}
//-------------------------------------------------------------------------------------------------------------------
// �������     my_adapt_threshold  ���ζ�ֵ��
// ����˵��     *ex_mt9v03x_binarizeImage ͼ������ָ��
//        col    ͼ���У���
//        row    ͼ���У��ߣ�
// ���ز���     ͼ���ֵ����ֵ
// ʹ��ʾ��     my_adapt_threshold(mt9v03x_image[0],MT9V03X_W,MT9V03X_H)
// ��ע��Ϣ
//-------------------------------------------------------------------------------------------------------------------
uint8 my_adapt_threshold(uint8 *ex_mt9v03x_binarizeImage, uint16 col, uint16 row)   //ע�������ֵ��һ��Ҫ��ԭͼ��
{
   #define GrayScale 256
    uint16 width = col;
    uint16 height = row;
    int pixelCount[GrayScale];
    float pixelPro[GrayScale];
    int i, j, pixelSum = width * height/4;
    uint8 threshold = 0;
    uint8* data = ex_mt9v03x_binarizeImage;  //ָ���������ݵ�ָ��
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
// �������     my_adapt_threshold_2  �ڶ��ζ�ֵ��
// ����˵��     *ex_mt9v03x_binarizeImage ͼ������ָ��
//        col    ͼ���У���
//        row    ͼ���У��ߣ�
// ���ز���     ͼ���ֵ����ֵ
// ʹ��ʾ��     uint8 threshold = (int)my_adapt_threshold_2(mt9v03x_image[0],MT9V03X_W,MT9V03X_H)
// ��ע��Ϣ
//-------------------------------------------------------------------------------------------------------------------
//uint8 my_adapt_threshold_2(uint8 *ex_mt9v03x_binarizeImage, uint16 col, uint16 row)   //ע�������ֵ��һ��Ҫ��ԭͼ��
//{
//   #define GrayScale 256
//    uint16 width = col;
//    uint16 height = row;
//    int pixelCount[GrayScale];
//    float pixelPro[GrayScale];
//    int i, j, pixelSum = width * height/4;
//    uint8 threshold = 0;
//    uint8* data = ex_mt9v03x_binarizeImage;  //ָ���������ݵ�ָ��
//    for (i = 0; i < GrayScale; i++)
//    {
//        pixelCount[i] = 0;
//        pixelPro[i] = 0;
//    }
//
//    uint32 gray_sum=0;
//    //ͳ�ƻҶȼ���ÿ������������ͼ���еĸ���
//    for (i = 1; i < height; i+=2)
//    {
//        for (j = 1; j < width; j+=2)
//        {
//            if( data[i * width + j]  > My_Threshold_1 )
//            {
//                pixelCount[(int)data[i * width + j]]++ ;  //����ǰ�ĵ������ֵ��Ϊ����������±�
//                gray_sum+=(int)data[i * width + j] ;       //�Ҷ�ֵ�ܺ�
//            }
//            else
//            {
//                pixelCount[(int) My_Threshold_1]++ ;  //����ǰ�ĵ������ֵ��Ϊ����������±�
//                gray_sum+= (int) My_Threshold_1 ;       //�Ҷ�ֵ�ܺ�
//            }
//        }
//    }
//
//    //����ÿ������ֵ�ĵ�������ͼ���еı���
//
//    for (i = 0; i < GrayScale; i++)
//    {
//        pixelPro[i] = (float)pixelCount[i] / pixelSum;
//    }
//
//    //�����Ҷȼ�[0,255]
//    float w0, w1, u0tmp, u1tmp, u0, u1, u, deltaTmp, deltaMax = 0;
//
//
//        w0 = w1 = u0tmp = u1tmp = u0 = u1 = u = deltaTmp = 0;
//        for (j = 0; j < GrayScale; j++)
//        {
//
//                w0 += pixelPro[j];  //��������ÿ���Ҷ�ֵ�����ص���ռ����֮��   ���������ֵı���
//                u0tmp += j * pixelPro[j];  //�������� ÿ���Ҷ�ֵ�ĵ�ı��� *�Ҷ�ֵ
//
//               w1=1-w0;
//               u1tmp=gray_sum/pixelSum-u0tmp;
//
//                u0 = u0tmp / w0;              //����ƽ���Ҷ�
//                u1 = u1tmp / w1;              //ǰ��ƽ���Ҷ�
//                u = u0tmp + u1tmp;            //ȫ��ƽ���Ҷ�
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
// �������     image_draw  ���ڿ� ���� ��ͼ����ж�ֵ��
// ����˵��     void
// ���ز���     void
// ʹ��ʾ��     image_draw();
// ��ע��Ϣ     ��ͼ����ж�ֵ�������Ҹ����ĸ���������ض԰����������
//-------------------------------------------------------------------------------------------------------------------
void image_draw(void)
{

    int i=0,j=0;
    int a=0; //���籣��
    for(i = image_h-3; i >= 0 ; i-- )//����͸��ͼ�������������,�ڽ���һ����ֵ�ݶȵݼ��Ķ�ֵ��
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
    for(int i=0;i<89;i++)       //����� ȥ����ͨ�����������ĸ���������ص����жϵ�ǰλ��Ӧ����ʲô���أ���ɾ������㣩
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
// �������     otsuThreshold   ���װ���(ͼ��ѹ��)
// ����˵��     *ex_mt9v03x_binarizeImage��ͼ���ַ  width��ͼ���    height��ͼ���
// ���ز���
// ʹ��ʾ��     otsuThreshold( mt9v03x_image[0], MT9V03X_W, MT9V03X_H);
// ��ע��Ϣ
//-------------------------------------------------------------------------------------------------------------------
//uint8 otsuThreshold(uint8 *ex_mt9v03x_binarizeImage, uint16 width,  uint16 height)
//{
//    #define GrayScale 256
//    int pixelCount[GrayScale] = {0};//ÿ���Ҷ�ֵ��ռ���ظ���
//    float pixelPro[GrayScale] = {0};//ÿ���Ҷ�ֵ��ռ�����ر���
//    int i,j;
//    int Sumpix = width * height/4;   //�����ص�
//    uint8 threshold = 0;
//    uint8* data = ex_mt9v03x_binarizeImage;  //ָ���������ݵ�ָ��
//
//
//    //ͳ�ƻҶȼ���ÿ������������ͼ���еĸ���
//    for (i = 0; i < height; i+=2)
//    {
//        for (j = 0; j < width; j+=2)
//        {
//            pixelCount[(int)data[i * width + j]]++;  //������ֵ��Ϊ����������±�
//          //   pixelCount[(int)ex_mt9v03x_binarizeImage[i][j]]++;    ������ָ�������
//        }
//    }
//    float u = 0;
//    for (i = 0; i < GrayScale; i++)
//    {
//        pixelPro[i] = (float)pixelCount[i] / Sumpix;   //����ÿ������������ͼ���еı���
//        u += i * pixelPro[i];  //��ƽ���Ҷ�
//    }
//
//
//    float maxVariance=0.0;  //�����䷽��
//    float w0 = 0, avgValue  = 0;  //w0 ǰ������ ��avgValue ǰ��ƽ���Ҷ�
//    for(int i = 0; i < 256; i++)     //ÿһ��ѭ������һ��������䷽����� (����for����Ϊ1��)
//    {
//        w0 += pixelPro[i];  //���赱ǰ�Ҷ�iΪ��ֵ, 0~i �Ҷ�������ռ����ͼ��ı�����ǰ������
//        avgValue  += i * pixelPro[i];
//
//        float variance = pow((avgValue/w0 - u), 2) * w0 /(1 - w0);    //��䷽��
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
// �������     ��Ӱɨ�踨������
// ʹ��ʾ��     shade_compute();
// ��ע��Ϣ
//-------------------------------------------------------------------------------------------------------------------

void shade_compute(void)   //�ж��Ƿ�ɨ�赽��Ӱ
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
// �������     Find_Boundary()   Ѱ����������ұ߽���
// ����˵��
// ���ز���     find_Boundary_flag �ж��Ƿ��ҵ����ұ߽�
// ʹ��ʾ��     int aflag=Find_Boundary();
// ��ע��Ϣ
//-------------------------------------------------------------------------------------------------------------------
uint8 centre_line=35;
int Find_Boundary(void)
{
    int find_Boundary_flag = 0;

     l_data_statics=0;//ͳ������ܹ��ĳ���
     r_data_statics=0;//ͳ���ұ��ܹ��ĳ���

    if (imageflag.Zebra_Flag!=1) //��δ���ְ�����ʱ��ʹ��м�������ɨ��
    {
        for (int i = 0; i < 5; i++)// Y
        {
            for (int j = centre_line ; j >= 1; j--)   //�����  X
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

            for (int j = centre_line ; j <= image_w - 1; j++)   //�ұ���   X                                                                                                                                                                                              ����
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

    else//�����ְ�����ʱ��ʹ��������м�ɨ��
    {
        for (int i = 1; i < 4; i++)//0 1 �б߽�
        {
            for (int j = 1; j <= image_w / 2; j++)   //�����
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

            for (int j = image_w-1; j >= image_w / 2; j--)   //�ұ���                                                                                                                                                                                                      ����
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
    return find_Boundary_flag;  //���������Ƿ��ҵ���ʼ��
}
//-------------------------------------------------------------------------------------------------------------------
// �������     search                   ��������ʽ��ʼ�ұ߽�ĺ�������������е�࣬���õ�ʱ��Ҫ©�ˣ������������һ�������ꡣ
// ����˵��     break_flag              �������Ҫѭ���Ĵ���
// ����˵��     (*ex_mt9v03x_binarizeImage)[image_w]       ����Ҫ�����ҵ��ͼ�����飬�����Ƕ�ֵͼ,�����������Ƽ���  �ر�ע�⣬��Ҫ�ú궨��������Ϊ����������������ݿ����޷����ݹ���
// ����˵��     hightest                ��ѭ���������õ�����߸߶�
// ���ز���     void
// ʹ��ʾ��
// ��ע��Ϣ
//-------------------------------------------------------------------------------------------------------------------
void search(uint16 break_flag,uint8(*image_u)[image_w],uint8* hightest) //��ʱû�����
{

    if( Find_Boundary())
    {
        l_data_statics=2;//ͳ������ܹ��ĳ���
        r_data_statics=2;//ͳ���ұ��ܹ��ĳ���
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
                *hightest = (Find_Boundary_r[r_data_statics][1] + Find_Boundary_l[l_data_statics ][1]) >> 1;//ȡ����ߵ�
                break;
            }



        }
    }
}
//-------------------------------------------------------------------------------------------------------------------
// �������     ����/ֱ���ж�
// ����˵��     void
// ���ز���     void
// ʹ��ʾ��     curvature_judge();
// ��ע��Ϣ     ���жϽ�������ά����imageflag.image_element_rings��
//-------------------------------------------------------------------------------------------------------------------
void curvature_judge(void)
{
    int x[3]={0},y[3]={0};
//Զ��----------------------------------------------------------------------
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

//�ж�----------------------------------------------------------------------
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


//����2----------------------------------------------------------------------
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


//����1----------------------------------------------------------------------
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
//����0----------------------------------------------------------------------
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

//���6----------------------------------------------------------------------
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
        imageflag.straight[0]=1;              //����ֱ��
        if(imageflag.image_curvature[0][3] > 168 && abs( imageflag.image_curvature[0][3] - imageflag.image_curvature[0][1]) <= 15 )
        {
            imageflag.straight[0] = 2;  //�ж�
            if(imageflag.image_curvature[0][4] > 168 && abs( imageflag.image_curvature[0][4] - imageflag.image_curvature[0][1]) <= 15 )
            {
                imageflag.straight[0] = 3;       //ȫ��ֱ��
            }
        }
    }
    else imageflag.straight[0]=0;

     if(imageflag.image_curvature[1][0] > 168 && imageflag.image_curvature[1][1] > 165 && imageflag.image_curvature[1][2] > 165
      && func_abs( imageflag.image_curvature[1][2] - imageflag.image_curvature[1][0]) <= 20  && func_abs(imageflag.image_curvature[1][1] - imageflag.image_curvature[1][2]) <= 25
      && Find_Boundary_r[2][1]<60 )
     {
         imageflag.straight[1]=1;              //����ֱ��
         if(imageflag.image_curvature[1][3] > 168 && abs( imageflag.image_curvature[1][3] - imageflag.image_curvature[1][1]) <= 15 )
         {
             imageflag.straight[1] = 2;  //�ж�
             if(imageflag.image_curvature[1][4] > 168 && abs( imageflag.image_curvature[1][4] - imageflag.image_curvature[1][1]) <= 15 )
             {
                 imageflag.straight[1] = 3;       //ȫ��ֱ��
             }
         }
     }
     else imageflag.straight[1]=0;




     if( abs( 90- imageflag.image_curvature[0][5]) >= 15 && imageflag.straight[0]==0 )                   //��·
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
// �������     Stayguy_ADS  ���ߺ���
// ����˵��      x_start     ������ʼ�������
//        y_start     ������ʼ��������
//        x_start     ����Ŀ��������
//        y_start     ����Ŀ���������
// ���ز���     void
// ʹ��ʾ��     Stayguy_ADS(0,0,1,1);
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
// �������    Angel_compute  ��ȡ���������γɵļн�
// ����˵��      x2  ����ǵĶ��������
//        y2  ����ǵĶ���������
//        x1,y1,x3,y3����������������ϵĵ������
// ���ز���     void
// ʹ��ʾ��     Angel_compute(0,0,1,0,0,1);
// ��ע��Ϣ     ���ݼ������άͼ��ǶȲ������Ʋ�Ϊͨ��ƽ���ı��η������ö�άͼ�������ά�еĽǶ�
//-------------------------------------------------------------------------------------------------------------------
double Angel_compute(int x1, int y1, int x2, int y2, int x3, int y3)
 {
     double dotProduct = (double)((x2 - x1) * (x2 - x3) + (y2 - y1) * (y2 - y3));
     double magProduct =
             sqrt((double)((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1))) * sqrt((double)((x2 - x3) * (x2 - x3) + (y2 - y3) * (y2 - y3)));

     // ȷ���� [-1, 1] ��Χ��
     double cos_angle = dotProduct / magProduct;
//     cos_angle = cos_angle < -1 ? -1 : (cos_angle > 1 ? 1 : cos_angle);

     double radian_angle = acos(cos_angle);
     double degree_angle = radian_angle * 180.0 / M_PI;

     return degree_angle;
 }
//-------------------------------------------------------------------------------------------------------------------
// �������         find_cross_down ��ʮ�����¹յ�
// ����˵��         void
// ���ز���         void
// ʹ��ʾ��         find_cross_down_l();
// ��ע��Ϣ     `����Ѳ�����ñ������ݶ�ʮ��·�ڽ����жϣ�������ùյ�нǵĽǶ���һ����Χ�ڣ������ж�Ϊʮ��·�ڹյ㣬�����Ϊʮ��·�ڣ�����ʮ��·�ڹյ���Ϣ��¼����
//-------------------------------------------------------------------------------------------------------------------
void find_cross_down_l(void)
{
    int x[3]={0},y[3]={0};
    double angle=0;
    for(int i = 1 ;i <= 60 && i < l_data_statics; i += 2 )
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
            && ex_mt9v03x_binarizeImage[(y[0]+y[2])/2][(x[2]+x[0])/2] == 0)//ac�����е�һ���� ��ɫ

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
// �������         find_cross_down_r ��ʮ�����¹յ�
// ����˵��         void
// ���ز���         void
// ʹ��ʾ��         find_cross_down_r();
// ��ע��Ϣ     `����Ѳ�����ñ������ݶ�ʮ��·�ڽ����жϣ�������ùյ�нǵĽǶ���һ����Χ�ڣ������ж�Ϊʮ��·�ڹյ㣬�����Ϊʮ��·�ڣ�����ʮ��·�ڹյ���Ϣ��¼����
//-------------------------------------------------------------------------------------------------------------------
void find_cross_down_r(void)
{
    int x[3]={0},y[3]={0};
    double angle=0;
    for(int i = 1;i <= 60 && i < r_data_statics; i += 2 )
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
            && ex_mt9v03x_binarizeImage[(y[0]+y[2])/2][(x[2]+x[0])/2] == 0      //ac�����е�һ���� ��ɫ
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
    for( ;i < l_data_statics  ; i += 2 )
    {
        x[0] = Find_Boundary_l[i][1];     //a b c ����  �����b�ĽǶ�
        x[1] = Find_Boundary_l[i+8][1];
        x[2] = Find_Boundary_l[i+16][1];

        y[0]=Find_Boundary_l[i][0];
        y[1]=Find_Boundary_l[i+8][0];
        y[2]=Find_Boundary_l[i+16][0];


        if( y[0] > y[2] && y[1] > y[2]
            && x[0] < x[2]
            && x[0] > 2
            && ex_mt9v03x_binarizeImage[(y[0]+y[2])/2][(x[2]+x[0])/2] == 0)//ac�����е�һ���� ��ɫ

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
    for(; i < r_data_statics  ; i += 2 )
        {
        x[0] = Find_Boundary_r[i][1];     //a b c ����  �����b�ĽǶ�
        x[1] = Find_Boundary_r[i+8][1];
        x[2] = Find_Boundary_r[i+16][1];

        y[0]=Find_Boundary_r[i][0];
        y[1]=Find_Boundary_r[i+8][0];
        y[2]=Find_Boundary_r[i+16][0];


        if(    y[0] > y[2] && y[1] > y[2]
            && x[0] > x[2]
            && x[0] < 68
            && ex_mt9v03x_binarizeImage[(y[0]+y[2])/2][(x[2]+x[0])/2] == 0      //ac�����е�һ���� ��ɫ
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
    for(  int i = 3 ;i < r_data_statics  ; i += 2 )
    {
        x[0] = Find_Boundary_r[i][1];     //a b c ����  �����b�ĽǶ�
        x[1] = Find_Boundary_r[i+8][1];
        x[2] = Find_Boundary_r[i+16][1];

        y[0]=Find_Boundary_r[i][0];
        y[1]=Find_Boundary_r[i+8][0];
        y[2]=Find_Boundary_r[i+16][0];


        if( y[1] > y[0] && x[1] > x[2]
            && x[2] < 68
            && ex_mt9v03x_binarizeImage[(y[0]+y[2])/2][(x[2]+x[0])/2] == 0)//ac�����е�һ���� ��ɫ

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
    for(  int i = 3 ;i < l_data_statics  ; i += 2 )
    {
        x[0] = Find_Boundary_l[i][1];     //a b c ����  �����b�ĽǶ�
        x[1] = Find_Boundary_l[i+8][1];
        x[2] = Find_Boundary_l[i+16][1];

        y[0]=Find_Boundary_l[i][0];
        y[1]=Find_Boundary_l[i+8][0];
        y[2]=Find_Boundary_l[i+16][0];


        if( y[1] > y[0] && x[1] < x[2]
            && x[0] > 2
            && ex_mt9v03x_binarizeImage[(y[0]+y[2])/2][(x[2]+x[0])/2] == 0)//ac�����е�һ���� ��ɫ

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
// �������  cross_checkout ʮ�ֲ�����
// ����˵��
// ���ز���
// ʹ��ʾ��
// ��ע��Ϣ
//-------------------------------------------------------------------------------------------------------------------
void cross_checkout(void)
{

    if(imageflag.CrossRoad_Flag==1)
    {


        int x1,y1;
        if( (r_Down[2] + l_Down[2] + l_on[2] + r_on[2])==3)   // ����һ
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
               imageflag.CrossRoad_Flag = 1;  //��������
               //lr_en_speed=0;   //������������� ʹ�ý���ڶ��׶�
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


//    put_int32(6,(int)sqrt( ( l_on[0] - l_Down[0] )  *( l_on[0] - l_Down[0] )
//            +( ( l_on[1] - l_Down[1] )  * ( l_on[1] - l_Down[1] ) ) ));


    if(  l_on[2]==1 && l_Down[2]==1
      && func_abs(  track_width - (int)sqrt( ( l_on[0] - l_Down[0] )  *( l_on[0] - l_Down[0] )
        +( ( l_on[1] - l_Down[1] )  * ( l_on[1] - l_Down[1] ) ) ) )<= 4   )//�����������
    {
            imageflag.CrossRoad_Flag=4;
            //lr_en_speed=0;
    }
    else if(  r_on[2]==1 && r_Down[2]==1
          && func_abs(  track_width - (int)sqrt( ( r_on[0] - r_Down[0] )  *( r_on[0] - r_Down[0] )
            +( ( r_on[1] - r_Down[1] )  * ( r_on[1] - r_Down[1] ) ) ) )<= 4   )//�����������
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
// �������   cross_dispose  ����ʮ���ж�
// ����˵��
// ���ز���
// ʹ��ʾ��
// ��ע��Ϣ
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
//        if( //lr_en_speed >= 11000 )   //����������ǿ�����
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


    else if(imageflag.CrossRoad_Flag==4) //б��ʮ��
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
//        if( //lr_en_speed >= 4300 )   //����������ǿ�����
//        {
//            imageflag.CrossRoad_Flag = 2;
//            find_cross_on_l();
//            find_cross_on_r();
//            //lr_en_speed = 0;
//        }
    }




//��ʼʮ�ֲ���-------------------------------------------------------------

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
    for( int i = 3;i < l_data_statics  ; i += 2 )
    {
        x[0] = Find_Boundary_l[i][1];     //a b c ����  �����b�ĽǶ�
        x[1] = Find_Boundary_l[i+8][1];
        x[2] = Find_Boundary_l[i+16][1];

        y[0]=Find_Boundary_l[i][0];
        y[1]=Find_Boundary_l[i+8][0];
        y[2]=Find_Boundary_l[i+16][0];





        if( x[0] > 2 && x[2] > x[0]
            && ex_mt9v03x_binarizeImage[(y[0]+y[2])/2][(x[2]+x[0])/2] == 0)//ac�����е�һ���� ��ɫ

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
    for( int i = 3;i < r_data_statics  ; i += 2 )
    {
        x[0] = Find_Boundary_r[i][1];     //a b c ����  �����b�ĽǶ�
        x[1] = Find_Boundary_r[i+8][1];
        x[2] = Find_Boundary_r[i+16][1];

        y[0]=Find_Boundary_r[i][0];
        y[1]=Find_Boundary_r[i+8][0];
        y[2]=Find_Boundary_r[i+16][0];


        if( x[0] < 68 && x[0] > x[2]
            && ex_mt9v03x_binarizeImage[(y[0]+y[2])/2][(x[2]+x[0])/2] == 0)//ac�����е�һ���� ��ɫ

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
    for( int i = 3;i < r_data_statics  ; i += 2 )
    {
        x[0] = Find_Boundary_r[i][1];     //a b c ����  �����b�ĽǶ�
        x[1] = Find_Boundary_r[i+8][1];
        x[2] = Find_Boundary_r[i+16][1];

        y[0]=Find_Boundary_r[i][0];
        y[1]=Find_Boundary_r[i+8][0];
        y[2]=Find_Boundary_r[i+16][0];


        if( x[0] < 68 && y[2] < y[0] && x[2] > x[1]
            && ex_mt9v03x_binarizeImage[(y[0]+y[2])/2][(x[2]+x[0])/2] == 0)//ac�����е�һ���� ��ɫ

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
// �������      �����ж�
// ����˵��
// ���ز���
// ʹ��ʾ��
// ��ע��Ϣ
//-------------------------------------------------------------------------------------------------------------------
void island_judgment(void)
{
    if(imageflag.straight[1] >=2 && imageflag.straight[0] == 0 )
    {
        find_island_on_l();
        if(is_L_on[2]==1 && is_L_on[0] >= island_in_limit_x && is_L_on[0] <= island_in_limit_x +10 )  //����Ѱ�ҹյ� ���� �յ�߶�
        {
            imageflag.image_element_rings[0]=1;
//            if(g_attitude.yaw + 55 >= 180)
//            angle.yaw = g_attitude.yaw + 55 -360.0;   //�뻷��ʼ�Ƕ�
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
// �������      ��������
// ����˵��
// ���ز���
// ʹ��ʾ��
// ��ע��Ϣ
//-------------------------------------------------------------------------------------------------------------------
//void island_dispose(void)
//{
////    buzzer=10;
//
//    if(imageflag.image_element_rings[0] == 1 )  //�뻷�����Ϲյ�
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
//            angle.yaw = g_attitude.yaw + 170 -360.0;   //�뻷��ʼ�Ƕ�
//            else
//            angle.yaw = g_attitude.yaw + 170;
//        }
//
//
//    }
//    else  if(imageflag.image_element_rings[0] == 2)  //������ת �ҽǶ�
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
//    else if(imageflag.image_element_rings[0] == 3)  //Ѱ�����¹յ�
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
//    else if( imageflag.image_element_rings[0] == 4) //�����ϳ���
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
////��ʼ��������-------------------------------------------------------------
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
// �������     midline_scan()   ����ɨ��
// ����˵��
// ���ز���
// ʹ��ʾ��
// ��ע��Ϣ
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



void Inflexion_Point(void)  //�ĸ��յ�  ��ʾ
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
//    if(lr_en_speed>60000)lr_en_speed=0;

    is_L_on[2] = 0;
    is_L_down[2] = 0;

    is_L_on[0]=0;
}
//-------------------------------------------------------------------------------------------------------------------
// �������    Camera_tracking()   ͼ�����ܺ���
// ����˵��
// ���ز���
// ʹ��ʾ��
// ��ע��Ϣ
//-------------------------------------------------------------------------------------------------------------------
uint8 hightest = 0;//��ߵ�
uint16 centre=0,centre_last=0,centre_s=0;//���ʹ�õ�����ֵ  centre_s ��Ϊ�˿�ǰ������̶�֮�� �ֶ�pd
void Camera_tracking(void)
{

    empty_flag(); //��־λ���
    Binaryzation(); //�������ֵ
    image_draw();  //��ֵ�� ���� ȥ���
    shade_compute();//�����½���Ӱ

    search((uint16)USE_num, ex_mt9v03x_binarizeImage,&hightest);


//ֱ�� ��� �ж�-----------------------------
    curvature_judge();


//Ԫ���ж�-----------------------------------


    if(imageflag.Zebra_Flag==0 && imageflag.image_element_rings[0]==0 && imageflag.image_element_rings[1] == 0)
    {
        if(imageflag.CrossRoad_Flag == 0)
            cross_judgment_positive();//����ʮ��
        if(imageflag.CrossRoad_Flag == 0)
            cross_judgment_slanting();

        if(imageflag.CrossRoad_Flag != 0) //�����־λ����ʮ�ֲ�ͬ�Ĺ���
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
// �������
// ����˵��
// ���ز���
// ʹ��ʾ��
// ��ע��Ϣ
//-------------------------------------------------------------------------------------------------------------------



