#include <arpa/inet.h>

#include "serialization.h"

void serialize_header(u8 *buffer, u8 action, u16 message_id) { serialize_u16(buffer, message_id | (action << 12)); }
void deserialize_header(const u8 *buffer, u8 *action, u16 *message_id) {
  u16 header = ntohs(*(u16 *)buffer);
  *action = header >> 12;
  *message_id = header & 0x0FFF;
}
