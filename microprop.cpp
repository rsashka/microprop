#include "microprop.h"

using namespace microprop;

bool Encoder::AssignBuffer(uint8_t *data, size_t size) {
    m_data = data;
    m_size = size;
    m_offset = 0;
    return data && size;
}

bool Encoder::write_cbor_int(int major_type, uint32_t value, size_t need_size) {
    major_type <<= 5;
    if(value < 24 && has_bytes(1 + need_size)) {
        put_byte((unsigned char) (major_type | value));
        return true;
    } else if(value < 256 && has_bytes(2 + need_size)) {
        put_byte((unsigned char) (major_type | 24));
        put_byte((unsigned char) value);
        return true;
    } else if(value < 65536 && has_bytes(3 + need_size)) {
        put_byte((unsigned char) (major_type | 25));
        put_byte((unsigned char) (value >> 8));
        put_byte((unsigned char) value);
        return true;
    } else if(has_bytes(5 + need_size)) {
        put_byte((unsigned char) (major_type | 26));
        put_byte((unsigned char) (value >> 24));
        put_byte((unsigned char) (value >> 16));
        put_byte((unsigned char) (value >> 8));
        put_byte((unsigned char) value);
        return true;
    }
    return false;
}

bool Encoder::write_cbor_long(int major_type, uint64_t value, size_t need_size) {
    major_type <<= 5;
    if(value < 24ULL && has_bytes(1 + need_size)) {
        put_byte((unsigned char) (major_type | value));
        return true;
    } else if(value < 256ULL && has_bytes(2 + need_size)) {
        put_byte((unsigned char) (major_type | 24));
        put_byte((unsigned char) value);
        return true;
    } else if(value < 65536ULL && has_bytes(3 + need_size)) {
        put_byte((unsigned char) (major_type | 25));
        put_byte((unsigned char) (value >> 8));
        put_byte((unsigned char) value);
        return true;
    } else if(value < 4294967296ULL && has_bytes(5 + need_size)) {
        put_byte((unsigned char) (major_type | 26));
        put_byte((unsigned char) (value >> 24));
        put_byte((unsigned char) (value >> 16));
        put_byte((unsigned char) (value >> 8));
        put_byte((unsigned char) value);
        return true;
    } else if(has_bytes(9 + need_size)) {
        put_byte((unsigned char) (major_type | 27));
        put_byte((unsigned char) (value >> 56));
        put_byte((unsigned char) (value >> 48));
        put_byte((unsigned char) (value >> 40));
        put_byte((unsigned char) (value >> 32));
        put_byte((unsigned char) (value >> 24));
        put_byte((unsigned char) (value >> 16));
        put_byte((unsigned char) (value >> 8));
        put_byte((unsigned char) value);
        return true;
    }
    return false;
}
