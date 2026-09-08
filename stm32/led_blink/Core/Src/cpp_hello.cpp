#include "main.h"
#include <string.h>

/* huart1 在 main.c 中定义为全局变量，此处用 extern 引用它 */
extern UART_HandleTypeDef huart1;

extern "C" void cppHello(void)
{
    const char *msg = "C++ OK\r\n";
    HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
}
