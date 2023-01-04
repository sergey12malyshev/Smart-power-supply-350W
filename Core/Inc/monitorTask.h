#ifndef __MONITOR_H
#define __MONITOR_H

void monitorTask(void);

void clear_uart_buff(void);
void UART_receve_IT(void);
void sendUART_WARNING(void);
void sendUART_hello(void);

#endif /* __MONITOR_H */
