#include "../include/crypto.h"

#include "../include/secrets.h"
#include "mbedtls/md.h"

const char *secretKey = SECRET_KEY;

String createHash(String id, String dateStr) {
  Serial.println("Creating hash");

  const size_t keyLength = strlen(secretKey);

  uint8_t hash[32];
  mbedtls_md_context_t ctx;
  mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;
  const mbedtls_md_info_t *md_info = mbedtls_md_info_from_type(md_type);

  mbedtls_md_init(&ctx);
  mbedtls_md_setup(&ctx, md_info, 1);
  mbedtls_md_hmac_starts(&ctx, (const unsigned char *)secretKey, keyLength);

  String message = id + dateStr;
  mbedtls_md_hmac_update(&ctx, (const unsigned char *)message.c_str(),
                         message.length());
  mbedtls_md_hmac_finish(&ctx, hash);
  mbedtls_md_free(&ctx);

  String result;
  for (int i = 0; i < sizeof(hash); i++) {
    if (hash[i] < 16)
      result += '0';
    result += String(hash[i], HEX);
  }

  Serial.println("\nHash created");
  return result;
}

void createHMACMap() {
  Serial.print("HMAC date : ");
  Serial.println(dateStr);

  for (JsonVariant member : memberData["data"]["allMembers"].as<JsonArray>()) {
    String memberId = member["memberId"].as<String>();
    String hmac = createHash(memberId, dateStr);
    hmacMap[memberId] = hmac;
  }

  Serial.print("HMAC map size : ");
  Serial.println(hmacMap.size());

  memberData.clear();
}