#ifndef __WORK_STATE_H
#define __WORK_STATE_H

#include <stdbool.h>

typedef enum 
{
  SWITCH_OFF_STATE = 0,
  SWITCH_ON_STATE,
  ALARM_STATE
} STATE;

void setState(STATE state);
STATE readState(void);
bool isState(STATE state);

#endif /* __WORK_STATE_H */
