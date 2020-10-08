#pragma once

#ifndef MICROPROPERTY_H
#define MICROPROPERTY_H

#include <stdint.h>
#include <stddef.h>
#include <algorithm>

#include <msgpack.h>

#ifndef STATIC_ASSERT
#define STATIC_ASSERT(expr) static_assert((expr), #expr)
#endif

#ifndef SCOPE
#define SCOPE(scope) scope
#endif

/*
 * Brief description of the data storage format.
 * 
 * Data serialization is based on Message Pack format.
 * 
 * Properties are stored pair in key - value format without using the MAP type.
 * For use property of type array, after the field key stored type array and data of the array elements.
 * Supported numeric arrays only.
 * The ID of the next field is located immediately after the last element of the array, also without using the MAP type.
 * 
 * Used fork msgpack for C/C++ https://github.com/msgpack/msgpack-c library,
 * where dynamic memory allocation was removed when packing and unpacking data from/to fixed static buffer.
 * From unpack array type, return after found array type code without unpaking array elements.
 */
namespace microprop {

typedef unsigned int KeyType; ///< Only numbers are used as field identifiers

class Encoder {
public:

    Encoder();

    Encoder(uint8_t *data, size_t size);

    virtual ~Encoder();

    bool AssignBuffer(uint8_t *data, size_t size);

    inline size_t GetUsed() {
        return m_offset;
    }

    inline size_t GetFree() {
        return (m_data && m_size && m_size >= m_offset ) ? m_size - m_offset : 0;
    }

    inline uint8_t * GetBuffer() {
        return m_data;
    }

    template < typename T>
    typename std::enable_if<std::is_arithmetic<T>::value, bool>::type
    inline Write(KeyType id, T value) {
        return id && msgpack_write(id) && msgpack_write(value);
    }

    template < typename T>
    typename std::enable_if<(std::is_array<T>::value && std::is_arithmetic<typename std::remove_extent<T>::type>::value) ||
    (std::is_reference<T>::value && std::is_arithmetic<typename std::remove_reference<T>::type>::value), size_t>::type
    inline Write(KeyType id, T & value) {
        size_t temp = m_offset;
        if (id && msgpack_write(id) && msgpack_pack_array(&m_pk, std::extent<T>::value) == 0) {
            for (size_t i = 0; i < std::extent<T>::value; i++) {
                if (!msgpack_write(value[i])) {
                    m_offset = temp;
                    return false;
                }
            }
            return true;
        }
        m_offset = temp;
        return false;
    }

    bool Write(KeyType id, uint8_t *data, size_t size);

    bool WriteAsString(KeyType id, const char *str);

    SCOPE(protected) :



    template < typename T>
    typename std::enable_if<std::is_arithmetic<T>::value, bool>::type
    inline msgpack_write(T value, size_t need_size = 0) {
        if (std::is_same<uint8_t, T>::value) {
            return msgpack_pack_uint8(&m_pk, value) == 0;
        } else if (std::is_same<uint16_t, T>::value) {
            return msgpack_pack_uint16(&m_pk, value) == 0;
        } else if (std::is_same<uint32_t, T>::value) {
            return msgpack_pack_uint32(&m_pk, value) == 0;
        } else if (std::is_same<uint64_t, T>::value) {
            return msgpack_pack_uint64(&m_pk, value) == 0;
        } else if (std::is_same<int8_t, T>::value) {
            return msgpack_pack_int8(&m_pk, value) == 0;
        } else if (std::is_same<int16_t, T>::value) {
            return msgpack_pack_int16(&m_pk, value) == 0;
        } else if (std::is_same<int32_t, T>::value) {
            return msgpack_pack_int32(&m_pk, value) == 0;
        } else if (std::is_same<int64_t, T>::value) {
            return msgpack_pack_int64(&m_pk, value) == 0;
        } else if (std::is_same<char, T>::value) {
            return msgpack_pack_char(&m_pk, value) == 0;
        } else if (std::is_same<short, T>::value) {
            return msgpack_pack_short(&m_pk, value) == 0;
        } else if (std::is_same<int, T>::value) {
            return msgpack_pack_int(&m_pk, value) == 0;
        } else if (std::is_same<long, T>::value) {
            return msgpack_pack_long(&m_pk, value) == 0;
        } else if (std::is_same<long long, T>::value) {
            return msgpack_pack_long_long(&m_pk, value) == 0;
        } else if (std::is_same<unsigned char, T>::value) {
            return msgpack_pack_unsigned_char(&m_pk, value) == 0;
        } else if (std::is_same<unsigned short, T>::value) {
            return msgpack_pack_unsigned_short(&m_pk, value) == 0;
        } else if (std::is_same<unsigned int, T>::value) {
            return msgpack_pack_unsigned_int(&m_pk, value) == 0;
        } else if (std::is_same<unsigned long, T>::value) {
            return msgpack_pack_unsigned_long(&m_pk, value) == 0;
        } else if (std::is_same<unsigned long long, T>::value) {
            return msgpack_pack_unsigned_long_long(&m_pk, value) == 0;
        } else if (std::is_same<float, T>::value) {
            return msgpack_pack_float(&m_pk, value) == 0;
        } else if (std::is_same<double, T>::value) {
            return msgpack_pack_double(&m_pk, value) == 0;
        } else if (std::is_same<bool, T>::value) {
            if (value) {
                return msgpack_pack_true(&m_pk) == 0;
            } else {
                return msgpack_pack_false(&m_pk) == 0;
            }
        }
        return false;
    }

    SCOPE(protected) :

    static int msgpack_callback(void* data, const char* buf, size_t len, void* callback_param);
    int callback_func(void* data, const char* buf, size_t len);

    SCOPE(private) :
    uint8_t *m_data;
    size_t m_size;
    size_t m_offset;
    msgpack_packer m_pk;

};

/*
 * 
 * 
 */

class Decoder {
public:

    Decoder();

    Decoder(uint8_t *data, size_t size);

    Decoder(const uint8_t *data, size_t size);

    virtual ~Decoder();

    bool AssignBuffer(uint8_t *data, size_t size);

    inline void Reset() {
        m_offset = 0;
    }

    inline size_t GetSize() {
        return m_size;
    }

    inline void TruncSize(size_t size) {
        if (m_size > size) {
            m_size = size;
        }
    }

    inline uint8_t * GetBuffer() {
        return m_data;
    }

    template < typename T>
    typename std::enable_if<std::is_arithmetic<T>::value, bool>::type
    inline Read(KeyType id, T & value) {
        return FieldFind(id) && msgpack_read(value);
    }

    /**
     * Check for the presence of a field with the specified identifier. The search always starts from the beginning of the buffer
     * Inner pointer direction at data
     * @param id Field identifier
     * @return Returns true if the field with the specified ID found
     */
    bool FieldFind(KeyType id);

    /**
     * Skip to the field data and read ID next field
     * @return Returns true, if the next field present, or false on error data or end buffer.
     */
    bool FieldNext(KeyType & id);

    template < typename T>
    typename std::enable_if<(std::is_array<T>::value && std::is_arithmetic<typename std::remove_extent<T>::type>::value) ||
    (std::is_reference<T>::value && std::is_arithmetic<typename std::remove_reference<T>::type>::value), size_t>::type
    inline Read(KeyType id, T & value) {
        if (FieldFind(id)) {
            msgpack_unpacked msg;
            msgpack_unpacked_init(&msg);

            if (msgpack_unpack_next(&msg, (const char *) m_data, m_size, &m_offset) >= 0) {

                assert(msg.zone == nullptr);

                msgpack_object array = msg.data;
                if (array.type == MSGPACK_OBJECT_ARRAY) {
                    size_t count = array.via.array.size;
                    if (std::extent<T>::value < count) {
                        return 0;
                    }
                    for (size_t i = 0; i < count; i++) {
                        if (!msgpack_read(value[i])) {
                            return 0;
                        }
                    }
                    return count;
                }
            }
        }
        return 0;
    }

    bool Read(KeyType id, uint8_t *data, size_t size);

    const char * ReadAsString(KeyType id, size_t *length = nullptr);

    /*
     * To use inner classes when customizing derived objects.
     */
    SCOPE(protected) :

    template < typename T>
    typename std::enable_if<std::is_arithmetic<T>::value, bool>::type
    msgpack_read(T & id) {
        msgpack_unpacked msg;
        msgpack_unpacked_init(&msg);

        if (msgpack_unpack_next(&msg, (const char *) m_data, m_size, &m_offset) >= 0) {

            assert(msg.zone == nullptr);

            msgpack_object value = msg.data;
            if (value.type == MSGPACK_OBJECT_POSITIVE_INTEGER) {
                T temp = (T) value.via.u64;
                if ((uint64_t) temp == value.via.u64) { // check overflow
                    id = temp;
                    return true;
                }
            } else if (value.type == MSGPACK_OBJECT_NEGATIVE_INTEGER) {
                T temp = (T) value.via.i64;
                if ((int64_t) temp == value.via.i64) { // check overflow
                    id = temp;
                    return true;
                }
            } else if (value.type == MSGPACK_OBJECT_BOOLEAN) {
                id = value.via.boolean;
                return true;
            } else if (value.type == MSGPACK_OBJECT_FLOAT || value.type == MSGPACK_OBJECT_FLOAT32 || value.type == MSGPACK_OBJECT_FLOAT64) {
                id = value.via.f64;
                return true;
            }
        }

        return false;
    }

    inline bool check_key_type(uint8_t value) {
        // Key ID can be a positive number only above zero
        // 
        // MessagePack int format family:
        // Int format family stores an integer in 1, 2, 3, 5, or 9 bytes.
        // positive fixnum stores 7-bit positive integer - 0XXXXXXX
        // negative fixnum stores 5-bit negative integer - 111YYYYY
        // uint 8 stores a 8-bit unsigned integer              - 0xcc
        // uint 16 stores a 16-bit big-endian unsigned integer - 0xcd
        // uint 32 stores a 32-bit big-endian unsigned integer - 0xce
        // uint 64 stores a 64-bit big-endian unsigned integer - 0xcf
        return (value && !(value & 0x80)) || ((value & 0xFC) == 0xCC);
    }

    SCOPE(private) :
    uint8_t *m_data;
    size_t m_size;
    size_t m_offset;
};

}
#endif /* MICROPROPERTY_H */

