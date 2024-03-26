#include <Arduino.h>
#if !defined(MESHMANAGER_H)
#define MESHMANAGER_H

void sendActionerInfo();
void receivedCallback(uint32_t from, String &msg);

#endif // MESHMANAGER_H