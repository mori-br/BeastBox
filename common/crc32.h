#ifndef CRC32_H
#define CRC32_H

#include <stdint.h>
#include <QByteArray>

class CRC32
{
private:
    CRC32();
    ~CRC32();

public:
    static uint32_t crc32(uint32_t crc, uint8_t data);
    static uint32_t crc32(uint8_t *buffer, uint32_t length);

    static uint32_t crc32(const QByteArray &arr);
};

#endif // CRC32_H
