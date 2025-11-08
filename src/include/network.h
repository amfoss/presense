#ifndef NETWORK_H
#define NETWORK_H

#include <stdint.h>

extern const bool is_enterprise;

void readMacAddress();
void init_peap();
void connectToWiFi();
void disablePromiscuousMode();
void enablePromiscuousMode();

#endif // NETWORK_H