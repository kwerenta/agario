#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include "types.h"

#define serialize_u16(buffer, value) *(u16 *)(buffer) = htons(value)
#define serialize_u32(buffer, value) *(u32 *)(buffer) = htonl(value)

#define serialize_f32(buffer, value) *(f32 *)(buffer) = value

#define deserialize_u16(destination, buffer) destination = ntohs(*(u16 *)(buffer))
#define deserialize_u32(destination, buffer) destination = ntohl(*(u32 *)(buffer))

#define deserialize_f32(destination, buffer) destination = *(f32 *)(buffer)

// 16-bit header structure:
// aaaa mmmm mmmm mmmm
// a - action
// m - message_id
//
void serialize_header(u8 *buffer, u8 action, u16 message_id);
void deserialize_header(const u8 *buffer, u8 *action, u16 *message_id);

#endif
