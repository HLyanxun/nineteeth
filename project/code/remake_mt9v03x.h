/*
 * remake_mt9v03x.h
 *
 *  Created on: 2024��5��31��
 *      Author: pc
 */

#ifndef REMAKE_MT9V03X_H_
#define REMAKE_MT9V03X_H_
#include "zf_common_headfile.h"

#define image_w     70
#define image_h     92
#define RESULT_ROW 90//���ͼ����
#define RESULT_COL 68
#define         USED_ROW                248  //����͸��ͼ������
#define         USED_COL                180
#define PER_IMG     mt9v03x_image//PER_IMG:����͸�ӱ任��ͼ��
#define ImageUsed   *PerImg_ip//*PerImg_ip����ʹ�õ�ͼ��ImageUsedΪ����Ѳ�ߺ�ʶ���ͼ��
/*flag*/
typedef struct {
   uint8 run_flag;
   uint8 out_flag;
   uint8 cross_flag;     //0δ����  1ɨ�赽���������յ�
   uint8 zebra_crossing_flag;
   uint8 L_island_flag = 0;
   uint8 R_island_flag = 0;  //���һ�����־λ
} ImageFlagStruct;// �������

extern ImageFlagStruct Imageflag;



extern uint8 image[image_h][image_w];

void ImagePerspective_Init(void);


#endif /* REMAKE_MT9V03X_H_ */
