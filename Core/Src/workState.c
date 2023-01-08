#include "workState.h"
#include "monitorTask.h"


static STATE workState = SWITCH_OFF_STATE;

void debugState(STATE state)
{
    if (state == SWITCH_OFF_STATE)     sendUART("SWITCH_OFF_STATE\r\n");
    if (state == SWITCH_ON_STATE)      sendUART("SWITCH_ON_STATE\r\n");
    if (state == ALARM_STATE)          sendUART("ALARM_STATE\r\n");
}

void setState(STATE state)
{
  if (workState != state)
  {
    debugState(state);
    workState = state; 
  }
}

STATE readState(void)
{
  return workState;
}

bool isState(STATE state)
{
  return (workState == state) ? true : false;
}
