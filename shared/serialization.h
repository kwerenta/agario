#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#define serialize_u16(buffer, value) *(u16 *)(buffer) = htons(value)
#define serialize_u32(buffer, value) *(u32 *)(buffer) = htonl(value)

#define serialize_f32(buffer, value) *(f32 *)(buffer) = value

#define deserialize_u16(destination, buffer) destination = ntohs(*(u16 *)(buffer))
#define deserialize_u32(destination, buffer) destination = ntohl(*(u32 *)(buffer))

#define deserialize_f32(destination, buffer) destination = *(f32 *)(buffer)

#endif
