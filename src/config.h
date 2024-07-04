#ifndef CONFIG_H
#define CONFIG_H

extern const char* ssid;
extern const char* password;
extern const char* graphql_endpoint;

void sendGraphQLMutation();

#endif // CONFIG_H
