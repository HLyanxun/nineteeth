/*
 * remake_mt9v03x.h
 *
 *  Created on: 2024年5月31日
 *      Author: pc
 */

#ifndef REMAKE_MT9V03X_H_
#define REMAKE_MT9V03X_H_
#include "zf_common_headfile.h"

#define image_w     70
#define image_h     92
#define RESULT_ROW 90//结果图行列
#define RESULT_COL 68
#define         USED_ROW                248  //用于透视图的行列
#define         USED_COL                180
#define PER_IMG     mt9v03x_image//PER_IMG:用于透视变换的图像
#define ImageUsed   *PerImg_ip//*PerImg_ip定义使用的图像，ImageUsed为用于巡线和识别的图像
/*flag*/
typedef struct {
   uint8 run_flag;
   uint8 out_flag;
   uint8 cross_flag;     //0未进入  1扫描到了下两个拐点
   uint8 zebra_crossing_flag;
   uint8 L_island_flag = 0;
   uint8 R_island_flag = 0;  //左右环岛标志位
} ImageFlagStruct;// 边线情况

extern ImageFlagStruct Imageflag;



extern uint8 image[image_h][image_w];

void ImagePerspective_Init(void);


#endif /* REMAKE_MT9V03X_H_ */
