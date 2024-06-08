/*
 * remake_mt9v03x.c
 *
 *  Created on: 2024��5��31��
 *      Author: pc
 */
#include "zf_common_headfile.h"
#include "math.h"
//��͸��---------------------------------------------------
uint8 *PerImg_ip[RESULT_ROW][RESULT_COL]={{0}};//�洢ͼ���ַ
#define track_width 28
//ͼ��----------------------------------------------
uint8 image[image_h][image_w];
uint8 centre_line=35;
uint8 l_shade_255=0,r_shade_255=0; //����Ҳ���Ӱ  Ҳ���� ��ɫ����
//uint8 mt9v03x_zip_image[MziH][MziW];
//��ֵ--------------------------------------------------------
uint8 My_Threshold=0;
int Threshold_fix=0;
//������---------------------------------------------------------------------------------------------------------------
#define USE_num image_h*3   //�����ҵ�������Ա��������˵300�����ܷ��£�������Щ�������ȷʵ�Ѷ����ඨ����һ��
uint8 Find_Boundary_l[300][2]={{0}};//��߽��� 0Ϊy  1Ϊx �洢����Ϊ������ɨ������ı߽�
uint8 Find_Boundary_r[300][2]={{0}};//�ұ߽��� 0Ϊy  1Ϊx �洢����Ϊ������ɨ������ı߽�
uint8 Find_Boundary_z[300][2]={{0}};//�б߽��� 0Ϊy  1Ϊx �洢����Ϊ������ɨ������ı߽�
uint8 Find_Boundary_centre[300][2]={{0}};//���� 0Ϊy  1Ϊx �洢����Ϊ������ɨ������ı߽�
uint16 l_data_statics=0;//ͳ������ܹ��ĳ���
uint16 r_data_statics=0;//ͳ���ұ��ܹ��ĳ���
uint8 hightest = 0;//��ߵ�
uint16 centre=0,centre_last=0,centre_s=0;//���ʹ�õ�����ֵ  centre_s ��Ϊ�˿�ǰ������̶�֮�� �ֶ�pd
//�����ж�---------------------------------------------------------------------------------------------------------------
uint8 curvature_l[6]={0},curvature_r[6]={0};  //���ұ������� ����012  �ж�3  Զ��4  ���5��� ����ֱ��
uint8 straight_l=0,straight_r=0;  //ֱ���жϱ�־λ 1Ϊ ����ֱ 2�ж�ֱ 3Զ��ֱ
uint8 Corners_l=0,Corners_r=0;  //����жϱ�־λ 1Ϊ ���
//ʮ��----------------------------------------------------------------------------------------------------------------
//������---------------------------------------------------------------------------------------------------------------
int region=0; //�����������
uint8 l_Down[4]={0},l_on[4]={0},r_Down[4]={0},r_on[4]={0}; //ʮ�ֵ��ĸ��յ� 0y1x
//����---------------------------------------------------------------------------------------------------------------
uint8 is_L_on[3]={0},is_R_on[3]={0};  //���� ���� ���� �����յ�0y1x
uint8 is_L_down[3]={0},is_R_down[3]={0};  //���� ���� ���� �����յ�0y1x
#define island_in_limit_x  55 //�����������Ƶ�λ
//flag--------------------------------------------------------
ImageFlagStruct Imageflag;

//-------------------------------------------------------------------------------------------------------
//  @name           Image_CompressInit
//  @brief          ԭʼ�Ҷ�ͼ��ѹ������ ѹ����ʼ��
//  @brief          ���þ��ǽ�ԭʼ�ߴ�ĻҶ�ͼ��ѹ����������Ҫ�Ĵ�С���������ǰ�ԭʼMh��Mw�еĻҶ�ͼ��ѹ����Lh��Lw�еĻҶ�ͼ��
//  @brief          ΪʲôҪѹ������Ϊ�Ҳ���Ҫ��ô�����Ϣ��60*80ͼ����չʾ����Ϣԭ�����Ѿ��㹻��ɱ��������ˣ���Ȼ����Ը����Լ�������޸ġ�
//  Sample usage:   Image_CompressInit();
//-------------------------------------------------------------------------------------------------------
//void Image_CompressInit(void)
//{
//  int i, j, row,line;
//  const float div_h = MT9V03X_H / MziH, div_w = MT9V03X_W / MziW;      //����ԭʼ��ͼ��ߴ��������Ҫ��ͼ��ߴ�ȷ����ѹ��������
//  for (i = 0; i < MziH; i++)                            //����ͼ���ÿһ�У��ӵ����е���59�С�
//  {
//    row = i * div_h + 0.5;
//    for (j = 0; j < MziW; j++)                          //����ͼ���ÿһ�У��ӵ����е���79�С�
//    {
//      line = j * div_w + 0.5;
////      mt9v03x_zip_image[i][j] = &mt9v03x_image[row][line];       //mt9v03x_image����������ԭʼ�Ҷ�ͼ��Image_Use����洢������֮��Ҫ��ȥ�����ͼ�񣬵���Ȼ�ǻҶ�ͼ��Ŷ��ֻ��ѹ����һ�¶��ѡ�
//    }
//  }
//}
//----------------------------------------------------------------
// �������     ��͸�ӳ�ʼ��
// ��ע˵��     ��ʼ����ʹ��ImageUsed��Ϊʶ�������飬��ʡ��������ͼ���ʱ��
//----------------------------------------------------------------
void ImagePerspective_Init(void)
{
    static uint8 BlackColor = 0;
    double change_un_Mat[3][3] ={{0.588870,-0.660767,54.049404},{-0.000000,0.346768,8.682904},{-0.000000,-0.007336,0.816300}};
//    double change_un_Mat[3][3] ={{0.651993,-0.669770,56.195344},{0.000000,0.410437,8.050314},{0.000000,-0.007320,0.856428}};
//    double change_un_Mat[3][3] ={{0.588870,-0.681798,55.290238},{0.000000,0.346768,8.682904},{0.000000,-0.007336,0.816300}};

    for (int i = 0; i < RESULT_COL;i++) {
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
//-----------------------------------------------------------------
// �������     ��ͼ����Ϣ�Ӵ�����ָ�뵼��
// ��ע��Ϣ     ͼ����ʾ������
//-----------------------------------------------------------------
//void transform_show_array(void)
//{
//    if (mt9v03x_finish_flag == 1)
//    {
//        for(int i=0;i<RESULT_ROW;i++)
//        {
//            for(int j=0;j<RESULT_COL;j++)
//            {
//                image[i][j]=ImageUsed[i][j];
//            }
//        }
////        tft180_show_gray_image(0,0,mt9v03x_show_image[0],RESULT_COL,RESULT_ROW,RESULT_COL,RESULT_ROW,0);
//        mt9v03x_finish_flag = 0;
//    }
//}
//-------------------------------------------------------------------
// �������     ���Ѱ��ͼ����ֵ
//-------------------------------------------------------------------
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
                    threshold = j;
                }
                if (deltaTmp < deltaMax)
                {
                    break;
                }
        }

    return threshold;
}
//-----------------------------------------------------------------------------------------
// �������     ��ͼ����ж�ֵ�������Լ�����㴦��
//-----------------------------------------------------------------------------------------
void image_draw(void)
{
    My_Threshold=my_adapt_threshold(mt9v03x_image[0],MT9V03X_W,MT9V03X_H)+Threshold_fix;
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
                image[i+1][j+1]=255;
//                image[i+1][j+1]=ImageUsed[i][j];
            else
            {
                image[i+1][j+1]=0;
//                image[i+1][j+1]=ImageUsed[i][j];
                if(i < 88 && i > 86)a++;
            }


        }
        if(Imageflag.run_flag==1)
        {
            if(a>50)
            {
             Imageflag.out_flag=1; Imageflag.run_flag=0;
            }
            a=0;
        }
    }

    uint16 N_zaoddian=0;
    for(int i=0;i<89;i++)       //����� ȥ��
        for(int j=1;j<70;j++)
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
    for(int i = 13 ; i < 20; i++)//��68
        for(int j=78;j<86;j++)//��90
            if( image[j][i] == 255 ) l_shade_255++;


    for( int i = 61 ; i < 68 ; i ++)
        for(int j=78;j<86;j++)
            if( image[j][i] == 255 )   r_shade_255++;
}
//-------------------------------------------------------------------------------------------------------------------
// �������     Find_Boundary()   Ѱ�ҵױߵ����ұ߽���
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

    if (Imageflag.zebra_crossing_flag!=1) //��δ���ְ�����ʱ��ʹ��м�������ɨ��
    {
        for (int i = 0; i < 5; i++)// Y
        {
            for (int j = centre_line ; j >= 1; j--)   //�����  X
            {
                if (image[90 - i][j-1] == 0 &&image[90 - i][j] == 0 && image[90 - i][j + 1] == 255 && image[90 - i][j + 2] == 255)
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
                if (image[90 - i][j+1] == 0 &&image[90 - i][j] == 0 && image[90 - i][j - 1] == 255 && image[90 - i][j - 2] == 255)
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
                if (image[90 - i][j] == 0 && image[90 - i][j + 1] == 255 && image[90 - i][j + 2] == 255)
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
                if (image[90 - i][j] == 0 && image[90 - i][j - 1] == 255 && image[90 - i][j - 2] == 255)
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
// �������     search                ��������ʽ��ʼ�ұ߽�ĺ�������������е�࣬���õ�ʱ��Ҫ©�ˣ������������һ�������ꡣ
// ����˵��     break_flag              �������Ҫѭ���Ĵ���
// ����˵��     (*image)[image_w]       ����Ҫ�����ҵ��ͼ�����飬�����Ƕ�ֵͼ,�����������Ƽ���  �ر�ע�⣬��Ҫ�ú궨��������Ϊ����������������ݿ����޷����ݹ���
// ����˵��     hightest                ��ѭ���������õ�����߸߶�
// ���ز���
// ʹ��ʾ��
// ��ע��Ϣ
//-------------------------------------------------------------------------------------------------------------------
void search(uint16 break_flag,uint8(*image_u)[image_w],uint8* hightest)
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
            /*���Ѳ��*/
            if(  Find_Boundary_l[ l_data_statics ][0] >= Find_Boundary_r[ r_data_statics ][0]  )  //��ʼ  ���
            {
                /*��������������һ�����ظ�����*/
                for(int j=0;j<8;j++)
                {
                  if(direction_l<0)     {direction_l+=8;}
                  else if(direction_l>=8)    {direction_l-=8;}
                  search_filds_l[0]=Find_Boundary_l[l_data_statics][0]+seeds_l[direction_l][1];     //y
                  search_filds_l[1]=Find_Boundary_l[l_data_statics][1]+seeds_l[direction_l][0];     //x

                  if(image_u[search_filds_l[0]][search_filds_l[1]]==0)
                  {
                          Find_Boundary_l[l_data_statics+1][0]=search_filds_l[0];
                          Find_Boundary_l[l_data_statics+1][1]=search_filds_l[1];


                      direction_l-=2;  //����ҵ���ɫ�ĵ� �����������������������
                      break;
                  }

                  direction_l++;  //�����1
                }
                l_data_statics++;//��¼����ߵĳ���+1
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
// �������     �����ж�
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
    x[2]=Find_Boundary_l[80][1];

    y[0]=Find_Boundary_l[10][0];
    y[1]=Find_Boundary_l[40][0];
    y[2]=Find_Boundary_l[80][0];
    curvature_l[4]=(uint8)Angel_compute(x[0],y[0],x[1],y[1],x[2],y[2]);//��н�

    x[0]=Find_Boundary_r[10][1];
    x[1]=Find_Boundary_r[40][1];
    x[2]=Find_Boundary_r[80][1];

    y[0]=Find_Boundary_r[10][0];
    y[1]=Find_Boundary_r[40][0];
    y[2]=Find_Boundary_r[80][0];
    curvature_r[4]=(uint8)Angel_compute(x[0],y[0],x[1],y[1],x[2],y[2]);

//�ж�----------------------------------------------------------------------
    x[0]=Find_Boundary_l[6][1];
    x[1]=Find_Boundary_l[30][1];
    x[2]=Find_Boundary_l[55][1];

    y[0]=Find_Boundary_l[6][0];
    y[1]=Find_Boundary_l[30][0];
    y[2]=Find_Boundary_l[55][0];
    curvature_l[3]=(uint8)Angel_compute(x[0],y[0],x[1],y[1],x[2],y[2]);

    x[0]=Find_Boundary_r[6][1];
    x[1]=Find_Boundary_r[30][1];
    x[2]=Find_Boundary_r[55][1];

    y[0]=Find_Boundary_r[6][0];
    y[1]=Find_Boundary_r[30][0];
    y[2]=Find_Boundary_r[55][0];
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
//����0----------------------------------------------------------------------
    x[0]=Find_Boundary_l[4][1];
    x[1]=Find_Boundary_l[13][1];
    x[2]=Find_Boundary_l[22][1];

    y[0]=Find_Boundary_l[4][0];
    y[1]=Find_Boundary_l[13][0];
    y[2]=Find_Boundary_l[22][0];
    curvature_l[0]=(uint8)Angel_compute(x[0],y[0],x[1],y[1],x[2],y[2]);

    x[0]=Find_Boundary_r[4][1];
    x[1]=Find_Boundary_r[13][1];
    x[2]=Find_Boundary_r[22][1];

    y[0]=Find_Boundary_r[4][0];
    y[1]=Find_Boundary_r[13][0];
    y[2]=Find_Boundary_r[22][0];
    curvature_r[0]=(uint8)Angel_compute(x[0],y[0],x[1],y[1],x[2],y[2]);

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



    if(curvature_l[0] > 168 && curvature_l[1] > 165 && curvature_l[2] > 165
     && func_abs( curvature_l[2] - curvature_l[0]) <= 20  && func_abs(curvature_l[1] - curvature_l[2]) <= 25
     && Find_Boundary_l[2][1]>10 )
    {
        straight_l=1;              //����ֱ��
        if(curvature_l[3] > 168 && abs( curvature_l[3] - curvature_l[1]) <= 15 )
        {
            straight_l = 2;  //�ж�
            if(curvature_l[4] > 168 && abs( curvature_l[4] - curvature_l[1]) <= 15 )
            {
                straight_l = 3;       //ȫ��ֱ��
            }
        }
    }
    else straight_l=0;

     if(curvature_r[0] > 168 && curvature_r[1] > 165 && curvature_r[2] > 165
      && func_abs( curvature_r[2] - curvature_r[0]) <= 20  && func_abs(curvature_r[1] - curvature_r[2]) <= 25
      && Find_Boundary_r[2][1]<60 )
     {
         straight_r=1;              //����ֱ��
         if(curvature_r[3] > 168 && abs( curvature_r[3] - curvature_r[1]) <= 15 )
         {
             straight_r = 2;  //�ж�
             if(curvature_r[4] > 168 && abs( curvature_r[4] - curvature_r[1]) <= 15 )
             {
                 straight_r = 3;       //ȫ��ֱ��
             }
         }
     }
     else straight_r=0;




     if( abs( 90- curvature_l[5]) >= 15 && straight_l==0 )                   //��·
     {
         Corners_l=1;
     }
     else Corners_l=0;
     if( abs( 90- curvature_r[5]) >= 15 && straight_r==0 )
     {
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
                image[y_start][x_start+1] = 0;
                y_start += y_dir;
            }
            break;
        }

        if(func_abs(y_start - y_end) > func_abs(x_start - x_end))
        {
            while(y_start != y_end)
            {
                image[y_start][x_start] = 0;
                image[y_start][x_start+1] = 0;
                y_start += y_dir;
                x_start = (int16)(((float)y_start - temp_b) / temp_rate);
            }
        }
        else
        {
            while(x_start != x_end)
            {
                image[y_start][x_start] = 0;
                image[y_start][x_start+1] = 0;
                x_start += x_dir;
                y_start = (int16)((float)x_start * temp_rate + temp_b);
            }
        }
    }while(0);
}
//-------------------------------------------------------------------------------------------------------------------
// �������   Angel_compute  ��ȡ���������γɵļн�
// ����˵��
// ���ز���   ����ļн�
// ʹ��ʾ��
// ��ע��Ϣ
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
// ����˵��
// ���ز���
// ʹ��ʾ��
// ��ע��Ϣ
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
            && image[(y[0]+y[2])/2][(x[2]+x[0])/2] == 0)//ac�����е�һ���� ��ɫ

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
            && image[(y[0]+y[2])/2][(x[2]+x[0])/2] == 0      //ac�����е�һ���� ��ɫ
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
            && image[(y[0]+y[2])/2][(x[2]+x[0])/2] == 0)//ac�����е�һ���� ��ɫ

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
            && image[(y[0]+y[2])/2][(x[2]+x[0])/2] == 0      //ac�����е�һ���� ��ɫ
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
            && image[(y[0]+y[2])/2][(x[2]+x[0])/2] == 0)//ac�����е�һ���� ��ɫ

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
            && image[(y[0]+y[2])/2][(x[2]+x[0])/2] == 0)//ac�����е�һ���� ��ɫ

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

    if(Imageflag.cross_flag==1)
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



    else if( Imageflag.cross_flag==2 || Imageflag.cross_flag==3 )
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


    else if(Imageflag.cross_flag==4)
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
               Imageflag.cross_flag = 1;  //��������

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
            Imageflag.cross_flag=4;

    }
    else if(  r_on[2]==1 && r_Down[2]==1
          && func_abs(  track_width - (int)sqrt( ( r_on[0] - r_Down[0] )  *( r_on[0] - r_Down[0] )
            +( ( r_on[1] - r_Down[1] )  * ( r_on[1] - r_Down[1] ) ) ) )<= 4   )//�����������
        {
                Imageflag.cross_flag=4;

        }
    else
    {
        l_on[2]=0; l_Down[2]=0;
        r_on[2]=0; r_Down[2]=0;
        Imageflag.cross_flag=0;
    }

}
//-------------------------------------------------------------------------------------------------------------------
// �������   cross_dispose  ����ʮ�ִ���
// ����˵��
// ���ز���
// ʹ��ʾ��
// ��ע��Ϣ
//-------------------------------------------------------------------------------------------------------------------
void cross_dispose(void)
{

//    buzzer=10;

    if(Imageflag.cross_flag==1)
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

            Imageflag.cross_flag = 2;
            find_cross_on_l();
            find_cross_on_r();
            lr_en_speed=0;
        }
        if( lr_en_speed >= 11000 )   //����������ǿ�����
        {
            Imageflag.cross_flag = 2;
            find_cross_on_l();
            find_cross_on_r();
            lr_en_speed = 0;
        }
    }
    else if(Imageflag.cross_flag==2||Imageflag.cross_flag==3)
    {
        find_cross_on_l();
        find_cross_on_r();

        cross_checkout();


        if(l_shade_255 <= 5 && r_shade_255 <= 5)
        {
            Imageflag.cross_flag = 3;
            find_cross_on_l();
            find_cross_on_r();
        }
        if( lr_en_speed >= 7000 )
        {
            Imageflag.cross_flag = 0;
        }


    }


    else if(Imageflag.cross_flag==4) //б��ʮ��
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
            Imageflag.cross_flag=1;
            find_cross_down_l();
            find_cross_down_r();
            find_cross_on_l();
            find_cross_on_r();
        }
        if( (l_shade_255 >= 20 && r_shade_255 >= 20) ||(l_on[2] == 1 && r_on[2] == 1 ))
        {
            Imageflag.cross_flag = 2;
            find_cross_on_l();
            find_cross_on_r();
            lr_en_speed=0;
        }
        if( lr_en_speed >= 4300 )   //����������ǿ�����
        {
            Imageflag.cross_flag = 2;
            find_cross_on_l();
            find_cross_on_r();
            lr_en_speed = 0;
        }
    }




//��ʼʮ�ֲ���-------------------------------------------------------------

    if(Imageflag.cross_flag == 1 )
    {
       if( l_Down[2] == 1 && l_on[2] == 1)     Stayguy_ADS(l_Down[1]-1,l_Down[0]+1,l_on[1],l_on[0]-1);
       if( r_Down[2] == 1 && r_on[2] == 1)     Stayguy_ADS(r_Down[1]+1,r_Down[0]+1,r_on[1],r_on[0]-1);

       if( l_Down[2] == 0 && l_on[2] == 1 )    Stayguy_ADS(21,90,l_on[1],l_on[0]-1);
       if( r_Down[2] == 0 && r_on[2] == 1 )    Stayguy_ADS(48,90,r_on[1],r_on[0]-1);
    }

    else if(Imageflag.cross_flag==2  ||  Imageflag.cross_flag == 3 )
    {
       if(l_on[2]==1)     Stayguy_ADS(21,90,l_on[1],l_on[0]-1);
       if(r_on[2]==1)     Stayguy_ADS(48,90,r_on[1],r_on[0]-1);
    }
    else if(Imageflag.cross_flag==4)
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
            && image[(y[0]+y[2])/2][(x[2]+x[0])/2] == 0)//ac�����е�һ���� ��ɫ

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
            && image[(y[0]+y[2])/2][(x[2]+x[0])/2] == 0)//ac�����е�һ���� ��ɫ

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
            && image[(y[0]+y[2])/2][(x[2]+x[0])/2] == 0)//ac�����е�һ���� ��ɫ

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
    if(straight_r >=2 && straight_l == 0 )
    {
        find_island_on_l();
        if(is_L_on[2]==1 && is_L_on[0] >= island_in_limit_x && is_L_on[0] <= island_in_limit_x +10 )  //����Ѱ�ҹյ� ���� �յ�߶�
        {
            Imageflag.L_island_flag=1;
            if(g_attitude.yaw + 55 >= 180)
            angle.yaw = g_attitude.yaw + 55 -360.0;   //�뻷��ʼ�Ƕ�
            else
            angle.yaw = g_attitude.yaw + 55;
        }
        else  Imageflag.L_island_flag=0;

    }
    else if (straight_l>2 && straight_r == 0)
    {

    }
}


//-------------------------------------------------------------------------------------------------------------------
// �������      �����ж�
// ����˵��
// ���ز���
// ʹ��ʾ��
// ��ע��Ϣ
//-------------------------------------------------------------------------------------------------------------------
void island_dispose(void)
{
//    buzzer=10;

    if(Imageflag.L_island_flag == 1 )  //�뻷�����Ϲյ�
    {
        find_island_on_l();

        if(is_L_on[2]==1 && is_L_on[0] <= island_in_limit_x-3)Imageflag.L_island_flag = 0;

        if(is_L_on[2]==0)
        {
            find_island_incline_on_l();
        }

        if(f_abs(g_attitude.yaw - angle.yaw)<5.0)
        {
            Imageflag.L_island_flag = 2;
            if(g_attitude.yaw + 170 >=180)
            angle.yaw = g_attitude.yaw + 170 -360.0;   //�뻷��ʼ�Ƕ�
            else
            angle.yaw = g_attitude.yaw + 170;
        }


    }
    else  if(Imageflag.L_island_flag == 2)  //������ת �ҽǶ�
    {
        if(f_abs(g_attitude.yaw - angle.yaw)<5.0)
        {
            Imageflag.L_island_flag = 3;

            find_island_down_l();


            if(g_attitude.yaw + 100 >=180)
            angle.yaw = g_attitude.yaw + 100 -360.0;
            else
            angle.yaw = g_attitude.yaw + 100;
        }
    }
    else if(Imageflag.L_island_flag == 3)  //Ѱ�����¹յ�
    {
        find_island_down_l();
        if(f_abs(g_attitude.yaw - angle.yaw)<5.0)
        {
            lr_en_speed = 0;
            Imageflag.L_island_flag = 4;
            find_island_on_l();
        }


    }
    else if( Imageflag.L_island_flag == 4) //�����ϳ���
    {
        find_island_on_l();

        if(lr_en_speed >= 9000)Imageflag.L_island_flag=0;
    }







//��ʼ��������-------------------------------------------------------------

    if(Imageflag.L_island_flag==1 && is_L_on[2]==1)
    {
        Stayguy_ADS(50,91,is_L_on[1] ,is_L_on[0]);
    }


    else if(Imageflag.L_island_flag==3 )
    {
        find_island_down_l();
        if(is_L_down[2] == 1)
        {
            Stayguy_ADS(1,60,is_L_down[1] ,is_L_down[0]);
            lr_en_speed=0;
        }
        else
        {
            Stayguy_ADS(1,65,50,90);
        }

    }
    else if(Imageflag.L_island_flag==4 )
    {

        if(is_L_on[2] == 1)
        {
            Stayguy_ADS(is_L_on[1],is_L_on[0],15,90);
        }
        else
        {
            Stayguy_ADS(1,37,15,90);
        }
    }



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
    if(straight_l>=2 && Imageflag.L_island_flag == 0 && Imageflag.R_island_flag == 0 && Imageflag.cross_flag==0)
    {
        for(int i=0;i<=l_data_statics;i++)
        {
           Find_Boundary_z[i][1]=Find_Boundary_l[i][1]+track_width/2;
           Find_Boundary_z[i][0]=Find_Boundary_l[i][0];
           image[Find_Boundary_z[i][0]][Find_Boundary_z[i][1]]=0;
        }
    }
    else if(straight_r>=2 && Imageflag.L_island_flag == 0 && Imageflag.R_island_flag == 0 && Imageflag.cross_flag==0)
    {
        for(int i=0;i<=r_data_statics;i++)
        {
           Find_Boundary_z[i][1]=Find_Boundary_r[i][1]-track_width/2;
           Find_Boundary_z[i][0]=Find_Boundary_r[i][0];
           image[Find_Boundary_z[i][0]][Find_Boundary_z[i][1]]=0;
        }
    }
    else
    {
        for(int i=0;i<=(r_data_statics+l_data_statics)/2;i++)
        {

           Find_Boundary_z[i][1]=(Find_Boundary_r[i][1]+Find_Boundary_l[i][1])/2;
           Find_Boundary_z[i][0]=(Find_Boundary_r[i][0]+Find_Boundary_l[i][0])/2;
           image[Find_Boundary_z[i][0]][Find_Boundary_z[i][1]]=0;
        }
    }

}



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

         ips200_draw_point( Find_Boundary_l[i][1] + 100+2, Find_Boundary_l[i][0] +150,RGB565_RED);
         ips200_draw_point( Find_Boundary_l[i][1] + 100+ 3, Find_Boundary_l[i][0] +150,RGB565_RED);
         ips200_draw_point( Find_Boundary_l[i][1] + 100 + 1, Find_Boundary_l[i][0] +150,RGB565_RED);
         ips200_draw_point( Find_Boundary_l[i][1] + 100 +4, Find_Boundary_l[i][0] +150 ,RGB565_RED);


       }
       for(int i=0 ; i < r_data_statics ; i++)
       {
         if(Find_Boundary_r[i][1] == Find_Boundary_r[i-1][1] && Find_Boundary_r[i][0] == Find_Boundary_r[i-1][0])
         {
             break;
         }
         ips200_draw_point( Find_Boundary_r[i][1] + 100-2 , Find_Boundary_r[i][0] +150 ,RGB565_BLUE);
         ips200_draw_point( Find_Boundary_r[i][1] + 100-3, Find_Boundary_r[i][0]+150,RGB565_BLUE);
         ips200_draw_point( Find_Boundary_r[i][1] + 100-4, Find_Boundary_r[i][0]+150 ,RGB565_BLUE);
         ips200_draw_point( Find_Boundary_r[i][1] + 100-1, Find_Boundary_r[i][0] +150,RGB565_BLUE);




       }

       for(int i=0;i<=(r_data_statics+l_data_statics)/2;i++)
       {

           ips200_draw_point( Find_Boundary_z[i][1] + 100 , Find_Boundary_z[i][0] + 150 ,RGB565_PURPLE);
           ips200_draw_point( Find_Boundary_z[i][1] + 101 , Find_Boundary_z[i][0] + 150 ,RGB565_PURPLE);
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

    search((uint16)USE_num, image,&hightest);


//ֱ�� ��� �ж�-----------------------------
    curvature_judge();


//Ԫ���ж�-----------------------------------


    if(Imageflag.zebra_crossing_flag==0 && Imageflag.L_island_flag==0 && Imageflag.R_island_flag == 0)
    {
        if(Imageflag.cross_flag == 0)
            cross_judgment_positive();//����ʮ��
        if(Imageflag.cross_flag == 0)
            cross_judgment_slanting();

        if(Imageflag.cross_flag != 0) //�����־λ����ʮ�ֲ�ͬ�Ĺ���
            cross_dispose();
    }

    if(Imageflag.cross_flag == 0 && Imageflag.zebra_crossing_flag==0)
    {
        if(Imageflag.L_island_flag == 0 && Imageflag.R_island_flag == 0 )
            island_judgment();
        if(Imageflag.L_island_flag != 0 && Imageflag.R_island_flag == 0 )
            island_dispose();

        else if(Imageflag.L_island_flag == 0 && Imageflag.R_island_flag != 0)
        {

        }

    }

//    put_int32(0,Imageflag.L_island_flag);





    search((uint16)USE_num, image,&hightest);
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

