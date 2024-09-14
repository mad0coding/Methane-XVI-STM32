该工程由STM32CubeMX创建，主要为使用其基于HAL库的USB功能
但融入了标准库文件，其他功能均在标准库环境下实现
为避免文件冲突，标准库文件不能引入main.c，只能引入LL_convert.c作为中介

旧版本USB结合了CDC和HID，且HID由键盘、鼠标、触控、媒体控制4个设备组成
即CDC1HID4，简为CH4，此即Methane-XVI名称的由来，Methane即甲烷，XVI则代表16键
新版本USB改为CustomHID和HID，不过名称仍不变

工程基于STM32F103C6T6芯片，32K FLASH，10K SRAM
因此程序空间和内存都较为紧张，故做了一些变量共用，并需要开启O2优化
可使用空间更大的STM32F103C8T6，但需要修改宏PARA_FLASH_ADDR
这是因为使用芯片本身的FLASH作为配置存储，对于C6T6，从0x08007C00即第31K开始存储
若改用C8T6以获得更大程序空间，需要改大该值

该工程的堆大小为0x500，栈大小为0x300

当前版本的摇杆中位值也存于FLASH中，可直接通过上位机校正，
存储地址为PARA_FLASH_ADDR-1024，大小共4字节，因此当前版本程序可使用的FLASH空间只有30K

















