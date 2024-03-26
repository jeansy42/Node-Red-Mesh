#include "painlessMesh.h"

#if !defined(TASKSNODEMANAGER_H)
#define TASKSNODEMANAGER_H
void verifyIfShouldReinit();
void sendActionerInfo();
void waitingForConfiguratios();

extern Task taskWaitingForConfigurations;
extern Task taskSendActionerInformation;
extern Task taskVerifyIfShouldReinit;
// Sensor de porta
extern Task taskGetDoorSensorInfo;
extern Task taskSendDoorSensorInfo;
#endif