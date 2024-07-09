################################################################################
# MRS Version: 1.9.1
# 自动生成的文件。不要编辑！
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
F:/H/desktop/document/CHV307_Library-master/CHV307_Library-master/Seekfree_CH32V307VCT6_Opensource_Library/project/code/Camera_Processing.c \
F:/H/desktop/document/CHV307_Library-master/CHV307_Library-master/Seekfree_CH32V307VCT6_Opensource_Library/project/code/remake_camera.c \
F:/H/desktop/document/CHV307_Library-master/CHV307_Library-master/Seekfree_CH32V307VCT6_Opensource_Library/project/code/swj.c 

OBJS += \
./code/Camera_Processing.o \
./code/remake_camera.o \
./code/swj.o 

C_DEPS += \
./code/Camera_Processing.d \
./code/remake_camera.d \
./code/swj.d 


# Each subdirectory must supply rules for building sources it contributes
code/Camera_Processing.o: F:/H/desktop/document/CHV307_Library-master/CHV307_Library-master/Seekfree_CH32V307VCT6_Opensource_Library/project/code/Camera_Processing.c
	@	@	riscv-none-embed-gcc -march=rv32imafc -mabi=ilp32f -msmall-data-limit=8 -mno-save-restore -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -pedantic -Wunused -Wuninitialized -Wall  -g -I"F:\H\desktop\document\CHV307_Library-master\CHV307_Library-master\Seekfree_CH32V307VCT6_Opensource_Library\Libraries\doc" -I"F:\H\desktop\document\CHV307_Library-master\CHV307_Library-master\Seekfree_CH32V307VCT6_Opensource_Library\libraries\zf_components" -I"F:\H\desktop\document\CHV307_Library-master\CHV307_Library-master\Seekfree_CH32V307VCT6_Opensource_Library\libraries\sdk\Core" -I"F:\H\desktop\document\CHV307_Library-master\CHV307_Library-master\Seekfree_CH32V307VCT6_Opensource_Library\libraries\sdk\Ld" -I"F:\H\desktop\document\CHV307_Library-master\CHV307_Library-master\Seekfree_CH32V307VCT6_Opensource_Library\libraries\sdk\Peripheral" -I"F:\H\desktop\document\CHV307_Library-master\CHV307_Library-master\Seekfree_CH32V307VCT6_Opensource_Library\libraries\sdk\Startup" -I"F:\H\desktop\document\CHV307_Library-master\CHV307_Library-master\Seekfree_CH32V307VCT6_Opensource_Library\project\user\inc" -I"F:\H\desktop\document\CHV307_Library-master\CHV307_Library-master\Seekfree_CH32V307VCT6_Opensource_Library\libraries\zf_common" -I"F:\H\desktop\document\CHV307_Library-master\CHV307_Library-master\Seekfree_CH32V307VCT6_Opensource_Library\libraries\zf_device" -I"F:\H\desktop\document\CHV307_Library-master\CHV307_Library-master\Seekfree_CH32V307VCT6_Opensource_Library\project\code" -I"F:\H\desktop\document\CHV307_Library-master\CHV307_Library-master\Seekfree_CH32V307VCT6_Opensource_Library\libraries\zf_driver" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@	@
code/remake_camera.o: F:/H/desktop/document/CHV307_Library-master/CHV307_Library-master/Seekfree_CH32V307VCT6_Opensource_Library/project/code/remake_camera.c
	@	@	riscv-none-embed-gcc -march=rv32imafc -mabi=ilp32f -msmall-data-limit=8 -mno-save-restore -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -pedantic -Wunused -Wuninitialized -Wall  -g -I"F:\H\desktop\document\CHV307_Library-master\CHV307_Library-master\Seekfree_CH32V307VCT6_Opensource_Library\Libraries\doc" -I"F:\H\desktop\document\CHV307_Library-master\CHV307_Library-master\Seekfree_CH32V307VCT6_Opensource_Library\libraries\zf_components" -I"F:\H\desktop\document\CHV307_Library-master\CHV307_Library-master\Seekfree_CH32V307VCT6_Opensource_Library\libraries\sdk\Core" -I"F:\H\desktop\document\CHV307_Library-master\CHV307_Library-master\Seekfree_CH32V307VCT6_Opensource_Library\libraries\sdk\Ld" -I"F:\H\desktop\document\CHV307_Library-master\CHV307_Library-master\Seekfree_CH32V307VCT6_Opensource_Library\libraries\sdk\Peripheral" -I"F:\H\desktop\document\CHV307_Library-master\CHV307_Library-master\Seekfree_CH32V307VCT6_Opensource_Library\libraries\sdk\Startup" -I"F:\H\desktop\document\CHV307_Library-master\CHV307_Library-master\Seekfree_CH32V307VCT6_Opensource_Library\project\user\inc" -I"F:\H\desktop\document\CHV307_Library-master\CHV307_Library-master\Seekfree_CH32V307VCT6_Opensource_Library\libraries\zf_common" -I"F:\H\desktop\document\CHV307_Library-master\CHV307_Library-master\Seekfree_CH32V307VCT6_Opensource_Library\libraries\zf_device" -I"F:\H\desktop\document\CHV307_Library-master\CHV307_Library-master\Seekfree_CH32V307VCT6_Opensource_Library\project\code" -I"F:\H\desktop\document\CHV307_Library-master\CHV307_Library-master\Seekfree_CH32V307VCT6_Opensource_Library\libraries\zf_driver" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@	@
code/swj.o: F:/H/desktop/document/CHV307_Library-master/CHV307_Library-master/Seekfree_CH32V307VCT6_Opensource_Library/project/code/swj.c
	@	@	riscv-none-embed-gcc -march=rv32imafc -mabi=ilp32f -msmall-data-limit=8 -mno-save-restore -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -pedantic -Wunused -Wuninitialized -Wall  -g -I"F:\H\desktop\document\CHV307_Library-master\CHV307_Library-master\Seekfree_CH32V307VCT6_Opensource_Library\Libraries\doc" -I"F:\H\desktop\document\CHV307_Library-master\CHV307_Library-master\Seekfree_CH32V307VCT6_Opensource_Library\libraries\zf_components" -I"F:\H\desktop\document\CHV307_Library-master\CHV307_Library-master\Seekfree_CH32V307VCT6_Opensource_Library\libraries\sdk\Core" -I"F:\H\desktop\document\CHV307_Library-master\CHV307_Library-master\Seekfree_CH32V307VCT6_Opensource_Library\libraries\sdk\Ld" -I"F:\H\desktop\document\CHV307_Library-master\CHV307_Library-master\Seekfree_CH32V307VCT6_Opensource_Library\libraries\sdk\Peripheral" -I"F:\H\desktop\document\CHV307_Library-master\CHV307_Library-master\Seekfree_CH32V307VCT6_Opensource_Library\libraries\sdk\Startup" -I"F:\H\desktop\document\CHV307_Library-master\CHV307_Library-master\Seekfree_CH32V307VCT6_Opensource_Library\project\user\inc" -I"F:\H\desktop\document\CHV307_Library-master\CHV307_Library-master\Seekfree_CH32V307VCT6_Opensource_Library\libraries\zf_common" -I"F:\H\desktop\document\CHV307_Library-master\CHV307_Library-master\Seekfree_CH32V307VCT6_Opensource_Library\libraries\zf_device" -I"F:\H\desktop\document\CHV307_Library-master\CHV307_Library-master\Seekfree_CH32V307VCT6_Opensource_Library\project\code" -I"F:\H\desktop\document\CHV307_Library-master\CHV307_Library-master\Seekfree_CH32V307VCT6_Opensource_Library\libraries\zf_driver" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@	@

