#ifndef CONFIG_H
#define CONFIG_H

extern const char* ssid;
extern const char* password;
extern const char* graphql_endpoint;

#include <vector>
#include <string>

extern std::vector<std::string> foundMacAddresses;

extern void sendGraphQLMutation();

#endif // CONFIG_H
