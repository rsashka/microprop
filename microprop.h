#pragma once

#ifndef MICROPROPERTY_H
#define MICROPROPERTY_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>
//#include <type_traits>
#include <algorithm>
#include <typeinfo>
#include <typeindex>

#ifndef STATIC_ASSERT
#define STATIC_ASSERT(expr) static_assert((expr), #expr)
#endif

#ifndef SCOPE
#define SCOPE(scope) scope
#endif

/*
 * Brief description of the data storage format.
 * 
 * Data serialization is based on Concise Binary Object Representation (CBOR) format, 
 * but only a subset of types are supported, the full standard is not implemented.
 * 
 * Properties are stored as key - value format without using the CBOR map type
 * For use property of type array, after the field key stored CBOR type array and values of the array elements.
 * Supported numeric arrays only.
 * The ID of the next field is located immediately after the last element of the array, also without using the CBOR map type.
 */
namespace microprop {

typedef unsigned int KeyType; ///< Only numbers are used as field identifiers

/* Used field types for storing data */
enum FieldType {
    Error = 0,
    Boolean,
    Integer,
    Blob, ///< Binary object of variable length
    String, ///< Equivalent to a null-terminated blob
    Array, ///< Numeric array stored count items. Can convert values without loss of precision, otherwise there will be an error while reading
};

class Encoder {
public:

    Encoder() : Encoder((uint8_t *) nullptr, 0) {
    }

    Encoder(uint8_t *data, size_t size) {
        AssignBuffer(data, size);
    }

    virtual ~Encoder() {
    }

    bool AssignBuffer(uint8_t *data, size_t size);

    inline size_t GetUsed() {
        return m_offset;
    }

    inline size_t GetSize() {
        return m_size;
    }

    inline uint8_t * GetBuffer() {
        return m_data;
    }

    template < typename T>
    typename std::enable_if<std::is_integral<T>::value && !std::is_same<bool, T>::value &&
    !std::is_same<int64_t, T>::value && !std::is_same<uint64_t, T>::value, bool>::type
    inline Write(KeyType id, T value) {
        return write_int((KeyType) id) && write_int(value);
    }

    template < typename T>
    typename std::enable_if<std::is_same<int64_t, T>::value || std::is_same<uint64_t, T>::value, bool>::type
    inline Write(KeyType id, T value) {
        return write_int((KeyType) id) && write_long(value);
    }

    template < typename T>
    typename std::enable_if<std::is_same<bool, T>::value, bool>::type
    inline Write(KeyType id, T value) {
        if (write_int((KeyType) id)) {
            if (value) {
                return put_byte((unsigned char) 0xf5);
            } else {
                return put_byte((unsigned char) 0xf4);
            }
        }
        return false;
    }

    inline bool Write(KeyType id, uint8_t *data, size_t size) {
        size_t temp = m_offset;
        if (write_int((KeyType) id, size) && write_cbor_int(2, size) && put_bytes((uint8_t *) data, size)) {
            return true;
        }
        m_offset = temp;
        return false;
    }

    inline bool WriteAsString(KeyType id, const char *str) {
        size_t len = strlen(str) + 1; // include null char 
        size_t temp = m_offset;
        if (write_int((KeyType) id, len) && write_cbor_int(3, len) && put_bytes((uint8_t *) str, len)) {
            return true;
        }
        m_offset = temp;
        return false;
    }

    SCOPE(protected) :


    template < typename T>
    typename std::enable_if<std::is_same<int64_t, T>::value || std::is_same<uint64_t, T>::value, bool>::type
    inline write_long(T value, size_t need_size = 0) {
        if (value < 0) {
            return write_cbor_long(1, (uint64_t) -(value + 1), need_size);
        } else {
            return write_cbor_long(0, (uint64_t) value, need_size);
        }
    }

    template < typename T>
    typename std::enable_if<std::is_integral<T>::value && !std::is_same<int64_t, T>::value && !std::is_same<uint64_t, T>::value, bool>::type
    inline write_int(T value, size_t need_size = 0) {
        if (value < 0) {
            return write_cbor_int(1, (uint32_t) -(value + 1), need_size);
        } else {
            return write_cbor_int(0, (uint32_t) value, need_size);
        }
    }

    inline bool put_byte(uint8_t value) {
        if (m_offset < m_size) {
            m_data[m_offset++] = value;
            return true;
        }
        return false;
    }

    inline bool put_bytes(uint8_t *data, size_t size) {
        if (size && m_offset + size - 1 < m_size) {
            memcpy(&m_data[m_offset], data, size);
            m_offset += size;
            return true;
        }
        return false;
    }

    inline bool has_bytes(uint8_t count) {
        return m_size - m_offset >= count;
    }

    bool write_cbor_int(int major_type, uint32_t value, size_t need_size = 0);
    bool write_cbor_long(int major_type, uint64_t value, size_t need_size = 0);


    SCOPE(private) :
    uint8_t *m_data;
    size_t m_size;
    size_t m_offset;
};

/*
 * 
 * 
 */

class Decoder {
public:

    Decoder() : Decoder((uint8_t *) nullptr, 0) {
    }

    Decoder(uint8_t *data, size_t size) {
        AssignBuffer(data, size);
    }

    Decoder(const uint8_t *data, size_t size) {
        AssignBuffer((uint8_t *) data, size);
    }

    virtual ~Decoder() {
    }

    bool AssignBuffer(uint8_t *data, size_t size) {
        m_data = data;
        m_size = size;
        m_offset = 0;
        return data && size;
    }

    inline void Reset() {
        m_offset = 0;
    }

    inline size_t GetSize() {
        return m_size;
    }

    inline uint8_t * GetBuffer() {
        return m_data;
    }

    /**
     * Check for the presence of a field with the specified identifier. The search always starts from the beginning of the buffer
     * Inner pointer direction at data
     * @param id Field identifier
     * @return Returns true if the field with the specified ID found
     */
    bool FieldFind(KeyType id) {
        if (!m_data || !m_size) {
            return false;
        }
        m_offset = 0;
        KeyType field_id;
        while (FieldNext(field_id)) {
            if (field_id == id) {
                return true;
            }
        }
        return false;
    }

    /**
     * Skip to the field data and read ID next field
     * @return Returns true, if the next field present, or false on error data or end buffer.
     */
    bool FieldNext(KeyType & id) {
        if (!m_data || !m_size || m_offset >= m_size) {
            return false;
        }
        uint8_t type;
        if (m_offset != 0) {
            // Skip field data
            size_t data_size = FieldSize(&type);
            if (!data_size) {
                return false;
            }
            m_offset += data_size;
            size_t items_count;
            if (type == 4 && read_cbor_number(items_count)) { // if array field
                // Skip array items
                for (size_t i = 0; i < items_count; i++) {
                    data_size = FieldSize(&type);
                    // Supported numeric array only
                    if (!data_size || !(type == 0 || type != 1)) {
                        return false;
                    }
                    m_offset += data_size;
                }
            }
        }
        // read field id and move offset net CBOR value
        return read_cbor_number(id);
    }

    inline size_t FieldSize(uint8_t *type = nullptr) {

        if (!m_data || !m_size || !has_bytes(1)) {
            return 0;
        }

        unsigned char byte = m_data[m_offset];
        unsigned char major_type = byte >> 5;
        unsigned char minor_type = (unsigned char) (byte & 31);

        if (type) {
            *type = major_type;
        }

        switch (major_type) {
            case 0: // numbers
            case 1: // negative numbers
            case 4: // array
                if (minor_type < 24) {
                    return 1;
                } else if (minor_type == 24 && has_bytes(2)) { // 1 byte
                    return 2;
                } else if (minor_type == 25 && has_bytes(3)) { // 2 byte
                    return 3;
                } else if (minor_type == 26 && has_bytes(5)) { // 4 byte
                    return 5;
                } else if (minor_type == 27 && has_bytes(9)) { // 8 byte
                    return 9;
                }
            case 2: // blob
            case 3: // string
                if (minor_type < 24) {
                    return 1 + minor_type;
                } else if (minor_type == 24 && has_bytes(2)) { // 1 byte
                    return 1 + get_byte();
                } else if (minor_type == 25 && has_bytes(3)) { // 2 byte
                    return 1 + get_short();
                } else if (minor_type == 26 && has_bytes(5)) { // 4 byte
                    return 1 + get_int();
                } else if (minor_type == 27 && has_bytes(9)) { // 8 byte
                    return 1 + get_long();
                }
            case 7:
                if (minor_type <= 24) {
                    return 1;
                }
        }
        return 0;
    }

    template < typename T>
    typename std::enable_if<std::is_integral<T>::value && !std::is_same<bool, T>::value &&
    !std::is_same<int64_t, T>::value && !std::is_same<uint64_t, T>::value, bool>::type
    inline Read(KeyType id, T & value) {
        return FieldFind(id) && read_cbor_number(value);
    }

    template < typename T>
    typename std::enable_if<std::is_same<int64_t, T>::value || std::is_same<uint64_t, T>::value, bool>::type
    inline Read(KeyType id, T & value) {
        return FieldFind(id) && read_cbor_number(value);
    }

    template < typename T>
    typename std::enable_if<std::is_same<bool, T>::value, bool>::type
    Read(KeyType id, T &value) {

        if (!FieldFind(id) || !has_bytes(1)) {
            return false;
        }

        uint8_t byte = m_data[m_offset];
        uint8_t major_type = byte >> 5;
        uint8_t minor_type = (byte & 0x1F);

        if (major_type == 7) {
            if (minor_type == 20) {
                value = false;
                return true;
            } else if (minor_type == 21) {
                value = true;
                return true;
            }
        }
        return false;
    }

    bool Read(KeyType id, uint8_t *data, size_t size) {
        size_t blob_size;
        if (FieldFind(id) && read_cbor_number(blob_size)) {
            if (data && blob_size <= size && m_offset + blob_size <= m_size) {
                memcpy(data, &m_data[m_offset], blob_size);
                return true;
            }
        }
        return false;
    }

    const char * ReadAsString(KeyType id, size_t *length = nullptr) {
        size_t blob_size;
        if (FieldFind(id) && read_cbor_number(blob_size)) {
            if (m_offset + blob_size <= m_size) {
                if (length) {
                    *length = blob_size;
                }
                return (const char *) &m_data[m_offset];
            }
        }
        if (length) {
            *length = 0;
        }
        return nullptr;
    }

    //    /**
    //     * Get a pointer to a field name id
    //     * @param buffer Pointer to the beginning of the field
    //     * @param length Size of field name identifier in bytes, if required
    //     * @return Pointer to the field name, or nullptr on error. The number of bytes of the name returned in the length parameter
    //     */
    //    static inline const char * FieldName(const uint8_t *buffer, size_t *length = nullptr) {
    //        Type type = FieldType(buffer);
    //        //@todo Need to implement processing of remote fields?
    //        if (type != Error && type != Removed) {
    //            if (length) {
    //                *length = (buffer[0] & (0xFF >> (8 - FIELDNAME_BIT)));
    //            }
    //            // Most significant bit of type indicates data length is stored
    //            if (type >> 3) {
    //                return reinterpret_cast<const char *> (&buffer[2]);
    //            }
    //            return reinterpret_cast<const char *> (&buffer[1]);
    //        }
    //        if (length) {
    //            *length = 0;
    //        }
    //        return nullptr;
    //    }
    //
    //    /**
    //     * The size of the data stored in the field. The length of strings is specified including the terminating null character
    //     * @param buffer Pointer to the beginning of the field
    //     * @return Size of field data or 0 on error.
    //     */
    //    static inline size_t FieldDataSize(const uint8_t *buffer) {
    //        switch (FieldType(buffer)) {
    //            case Bool: return 1;
    //            case Byte: return 1;
    //            case Word: return 2;
    //            case DWord: return 4;
    //            case DDWord: return 8;
    //            case Float: return 4;
    //            case Double: return 8;
    //
    //            case Blob:
    //            case String:
    //            case Array16:
    //            case Array32:
    //            case Array64:
    //            case ArrayFloat:
    //            case ArrayDouble:
    //                return buffer[1];
    //            default:
    //                break;
    //                //Removed && Error - 0
    //        }
    //        return 0;
    //    }


    //    /*
    //     * 
    //     * Define helpers for SFINAE templates
    //     * 
    //     */
    //
    //#define MP_TEMPLATE_INT_KEY_ONLY(TYPE, RESULT)     
//    template < typename TYPE> 
//    typename std::enable_if<std::is_integral<TYPE>::value, RESULT>::type
    //
    //#define MP_TEMPLATE_ARITHMETIC_ONLY(TYPE) 
//    template < typename TYPE> 
//    typename std::enable_if<std::is_arithmetic<TYPE>::value && !std::is_array<TYPE>::value && !std::is_reference<TYPE>::value && !std::is_pointer<TYPE>::value, size_t>::type
    //
    //#define MP_TEMPLATE_ARITHMETIC_FOR_INT_KEY(N,T)     
//    template < typename N, typename T> 
//    typename std::enable_if<std::is_integral<N>::value && std::is_arithmetic<T>::value && 
//    !std::is_array<T>::value && !std::is_reference<T>::value && !std::is_pointer<T>::value, size_t>::type
    //
    //#define MP_TEMPLATE_ARRAY_ONLY(T)     
//    template < typename T> 
//    typename std::enable_if<(std::is_array<T>::value && std::is_arithmetic<typename std::remove_extent<T>::type>::value) || 
//    (std::is_reference<T>::value && std::is_arithmetic<typename std::remove_reference<T>::type>::value), size_t>::type
    //
    //#define MP_TEMPLATE_ARRAY_FOR_INT_KEY(N,T)     
//    template < typename N, typename T> 
//    typename std::enable_if<std::is_integral<N>::value && 
//    ((std::is_array<T>::value && std::is_arithmetic<typename std::remove_extent<T>::type>::value) || 
//    (std::is_reference<T>::value && std::is_arithmetic<typename std::remove_reference<T>::type>::value)), size_t>::type
    //

    /*
     * 
     * 
     * Append field as simple numbers
     * 
     * 
     */

    //    template < typename T>
    //    typename std::enable_if<std::is_same<bool, T>::value, size_t>::type

    //
    //    MP_TEMPLATE_ARITHMETIC_FOR_INT_KEY(N, T)
    //    inline Append(N name, T value) {
    //        name = htobe(name);
    //        return Append((const char *) &name, sizeof (name), value);
    //    }
    //
    //    MP_TEMPLATE_ARITHMETIC_ONLY(T)
    //    inline Append(const char *name, T value) {
    //        return Append(name, strlen(name), value);
    //    }
    //
    //    MP_TEMPLATE_ARITHMETIC_ONLY(T)
    //    Append(const char *name, uint8_t name_size, T value) {
    //        if (m_read_only) {
    //            return false;
    //        }
    //
    //        uint8_t data_size = CalcDataSize(value);
    //        if (!m_data || !name || !name_size || !data_size) {
    //            return false;
    //        }
    //        if ((data_size + name_size + 1U) > FIELDSIZE_MAX || (data_size + name_size + 1U + m_used) > m_size) {
    //            return false;
    //        }
    //        Type type = CalcDataType(value);
    //        m_data[m_used] = MakeFieldHeader(name, name_size, type);
    //        if (!m_data[m_used]) {
    //            return false;
    //        }
    //        m_used++;
    //        memcpy(&m_data[m_used], name, name_size);
    //        m_used += name_size;
    //        size_t result = Serialize(&m_data[m_used], m_size - m_used, value);
    //        m_used += result;
    //        return result;
    //    }
    //
    //    /*
    //     * 
    //     * Read field as simple numbers and one-dimensional arrays for all types of numbers
    //     * 
    //     */
    //
    //    template < typename T>
    //    inline size_t Read(const char *name, T & value) {
    //        return Read(name, strlen(name), value);
    //    }
    //
    //    /*
    //     * Covers all variants for an integer key type, i.e. all types of numbers and arrays
    //     */
    //    template < typename N, typename T>
    //    typename std::enable_if<std::is_integral<N>::value && (std::is_arithmetic<T>::value ||
    //            (std::is_array<T>::value && std::is_arithmetic<typename std::remove_extent<T>::type>::value) ||
    //            (std::is_reference<T>::value && std::is_arithmetic<typename std::remove_reference<T>::type>::value)), size_t>::type
    //    inline Read(N name, T & value) {
    //        name = htobe(name);
    //        return Read((const char *) &name, sizeof (name), value);
    //    }
    //
    //    template < typename T>
    //    size_t Read(const char *name, uint8_t name_size, T & value) {
    //        const uint8_t * field = FieldExist(name, name_size);
    //        uint8_t size = FieldDataSize(field);
    //        Type field_type = FieldType(field);
    //        if (field && size && field_type != Error) {
    //            Type data_type = CalcDataType(value);
    //            // For array check data type
    //            // Most significant bit of type indicates about of array data
    //            if ((data_type >> 3) && field_type != data_type) {
    //                return 0;
    //            }
    //            uint8_t offset = FieldHeaderSize(field);
    //            if (offset) {
    //                return Deserialize(const_cast<uint8_t *> (&field[offset]), size, value);
    //            }
    //        }
    //        return 0;
    //    }
    //
    //    /*
    //     * 
    //     * Append fields as one-dimensional arrays for all types of numbers
    //     * 
    //     */
    //
    //    MP_TEMPLATE_ARRAY_FOR_INT_KEY(N, T)
    //    inline Append(N name, T & value) {
    //        name = htobe(name);
    //        return Append((const char *) &name, sizeof (name), value);
    //    }
    //
    //    MP_TEMPLATE_ARRAY_ONLY(T)
    //    inline Append(const char *name, T & value) {
    //        return Append(name, strlen(name), value);
    //    }
    //
    //    MP_TEMPLATE_ARRAY_ONLY(T)
    //    Append(const char *name, uint8_t name_size, T & value) {
    //        if (m_read_only) {
    //            return false;
    //        }
    //
    //        uint8_t extra_size = FieldExtraSize(CalcDataType(value));
    //        uint8_t data_size = CalcDataSize(value);
    //        if (!m_data || !name || !name_size || !data_size) {
    //            return false;
    //        }
    //        if ((data_size + name_size + extra_size) > FIELDSIZE_MAX || (data_size + name_size + extra_size + m_used) > m_size) {
    //            return false;
    //        }
    //        Type type = CalcDataType(value);
    //        m_data[m_used] = MakeFieldHeader(name, name_size, type);
    //        if (!m_data[m_used]) {
    //            return false;
    //        }
    //        m_used++;
    //        if (extra_size) {
    //            m_data[m_used] = CalcDataSize(value);
    //            m_used++;
    //        }
    //        memcpy(&m_data[m_used], name, name_size);
    //        m_used += name_size;
    //        size_t result = Serialize(&m_data[m_used], m_size - m_used, value);
    //        m_used += result;
    //        return result;
    //    }
    //
    //    /*
    //     * 
    //     * Generic methods for append binary data
    //     * 
    //     */
    //    MP_TEMPLATE_INT_KEY_ONLY(T, size_t)
    //    inline Append(T name, uint8_t *data, size_t data_size, Type type = Type::Blob) {
    //        name = htobe(name);
    //        return Append((const char *) &name, sizeof (name), data, data_size, type);
    //    }
    //
    //    inline size_t Append(const char *name, uint8_t *data, size_t data_size, Type type = Type::Blob) {
    //        return Append(name, strlen(name), data, data_size, type);
    //    }
    //
    //    size_t Append(const char *name, uint8_t name_size, uint8_t *data, size_t data_size, Type type = Type::Blob) {
    //        if (m_read_only || !m_data || !name || !name_size || !data || !data_size) {
    //            return false;
    //        }
    //        uint8_t extra_size = FieldExtraSize(type) + 1;
    //        if ((data_size + name_size + extra_size) > FIELDSIZE_MAX || (data_size + name_size + extra_size + m_used) > m_size) {
    //            return false;
    //        }
    //        m_data[m_used] = MakeFieldHeader(name, name_size, type);
    //        if (!m_data[m_used]) {
    //            return false;
    //        }
    //        m_used++;
    //        m_data[m_used] = data_size;
    //        m_used++;
    //        memcpy(&m_data[m_used], name, name_size);
    //        m_used += name_size;
    //        memcpy(&m_data[m_used], data, data_size);
    //        m_used += data_size;
    //        if (type == String) {
    //            m_data[m_used] = '\0';
    //            m_used++;
    //        }
    //        return data_size;
    //    }
    //
    //    /*
    //     * 
    //     * Generic methods for read binary data
    //     * 
    //     */
    //
    //    MP_TEMPLATE_INT_KEY_ONLY(T, size_t)
    //    inline Read(T name, uint8_t *data, size_t size) {
    //        name = htobe(name);
    //        return Read((const char *) &name, sizeof (name), data, size);
    //    }
    //
    //    inline size_t Read(const char *name, uint8_t *data, size_t data_size) {
    //        return Read(name, strlen(name), data, data_size);
    //    }
    //
    //    size_t Read(const char *name, uint8_t name_size, uint8_t *data, size_t size) {
    //        const uint8_t * field = FieldExist(name, name_size);
    //        uint8_t blob_size = FieldDataSize(field);
    //        uint8_t offset = FieldHeaderSize(field);
    //        Type field_type = FieldType(field);
    //        if (field && blob_size && offset && field_type == Blob) {
    //            memcpy(data, const_cast<uint8_t *> (&field[offset]), blob_size);
    //            return blob_size;
    //        }
    //        return 0;
    //    }
    //
    //    /*
    //     * 
    //     * Append fields as string
    //     * 
    //     */
    //    inline size_t AppendAsString(const char *name, const char * str) {
    //        return AppendAsString(name, strlen(name), str);
    //    }
    //
    //    MP_TEMPLATE_INT_KEY_ONLY(T, size_t)
    //    inline AppendAsString(T name, const char * str) {
    //        name = htobe(name);
    //        return AppendAsString((const char *) &name, sizeof (T), str);
    //    }
    //
    //    inline size_t AppendAsString(const char *name, uint8_t name_size, const char * str) {
    //        return Append(name, name_size, (uint8_t *) str, strlen(str), String);
    //    }
    //
    //    /*
    //     * 
    //     * Read fields as string
    //     * 
    //     */
    //
    //    inline const char * ReadAsString(const char * name, size_t *length = nullptr) {
    //        return ReadAsString(name, strlen(name), length);
    //    }
    //
    //    MP_TEMPLATE_INT_KEY_ONLY(T, const char *)
    //    inline ReadAsString(T name, size_t *length = nullptr) {
    //        name = htobe(name);
    //        return ReadAsString((const char *) &name, sizeof (name), length);
    //    }
    //
    //    const char * ReadAsString(const char * name, size_t name_len, size_t *length = nullptr) {
    //        const uint8_t * field = FieldExist(name, name_len);
    //        Type type = FieldType(field);
    //        uint8_t offset = FieldHeaderSize(field);
    //        if (field && offset && type == String) {
    //            if (length) {
    //                *length = FieldDataSize(field);
    //            }
    //            return (const char *) &field[offset];
    //        }
    //    }
    //
    //    /*
    //     * 
    //     * Converting to and from network byte order for integers
    //     * 
    //     */
    //    template <typename T>
    //    typename std::enable_if<std::is_arithmetic<T>::value && !std::is_reference<T>::value && !std::is_array<T>::value, T>::type
    //    inline static constexpr htobe(T value, char* ptr = 0) noexcept {
    //        return
    //#if __BYTE_ORDER == __LITTLE_ENDIAN
    //        ptr = reinterpret_cast<char*> (&value),
    //                std::reverse(ptr, ptr + sizeof (T)),
    //#endif
    //                value;
    //    }
    //
    //    template <typename T>
    //    typename std::enable_if<std::is_reference<T>::value && std::is_arithmetic<typename std::remove_reference<T>::type>::value, T>::type
    //    inline static constexpr htobe(T & value, char* ptr = 0) noexcept {
    //        return
    //#if __BYTE_ORDER == __LITTLE_ENDIAN
    //        ptr = reinterpret_cast<char*> (&value),
    //                std::reverse(ptr, ptr + sizeof (T)),
    //#endif
    //                value;
    //    }
    //
    //    template <typename T>
    //    typename std::enable_if<std::is_arithmetic<T>::value && !std::is_reference<T>::value && !std::is_array<T>::value, T>::type
    //    inline static constexpr betoh(T value, char* ptr = 0) noexcept {
    //        STATIC_ASSERT(std::is_arithmetic<T>::value);
    //        return
    //#if __BYTE_ORDER == __LITTLE_ENDIAN
    //        ptr = reinterpret_cast<char*> (&value),
    //                std::reverse(ptr, ptr + sizeof (T)),
    //#endif
    //                value;
    //    }
    //
    //    template <typename T>
    //    typename std::enable_if<std::is_reference<T>::value && std::is_arithmetic<typename std::remove_reference<T>::type>::value, T>::type
    //    inline static constexpr betoh(T & value, char* ptr = 0) noexcept {
    //        STATIC_ASSERT(std::is_arithmetic<T>::value);
    //        return
    //#if __BYTE_ORDER == __LITTLE_ENDIAN
    //        ptr = reinterpret_cast<char*> (&value),
    //                std::reverse(ptr, ptr + sizeof (T)),
    //#endif
    //                value;
    //    }

    /*
     * To use inner classes when customizing derived objects.
     */
    SCOPE(protected) :

    //    template < typename T>
    //    typename std::enable_if<std::is_pointer<T>::value ||
    //    (std::is_array<T>::value && std::is_arithmetic<typename std::remove_extent<T>::type>::value) ||
    //    (std::is_reference<T>::value && std::is_arithmetic<typename std::remove_reference<T>::type>::value), Type>::type
    //    inline static CalcDataType(T &value) {
    //        if ((std::is_same < uint8_t, typename std::remove_reference<T>::type>::value ||
    //                std::is_same < int8_t, typename std::remove_reference<T>::type >::value ||
    //                std::is_same < uint8_t, typename std::remove_extent<T>::type >::value ||
    //                std::is_same < int8_t, typename std::remove_extent<T>::type >::value) && sizeof (value[0]) == 1) {
    //            return Blob;
    //        } else if ((std::is_same < uint16_t, typename std::remove_reference<T>::type>::value ||
    //                std::is_same < int16_t, typename std::remove_reference<T>::type >::value ||
    //                std::is_same < uint16_t, typename std::remove_extent<T>::type >::value ||
    //                std::is_same < int16_t, typename std::remove_extent<T>::type >::value) && sizeof (value[0]) == 2) {
    //            return Array16;
    //        } else if ((std::is_same < uint32_t, typename std::remove_reference<T>::type>::value ||
    //                std::is_same < int32_t, typename std::remove_reference<T>::type >::value ||
    //                std::is_same < uint32_t, typename std::remove_extent<T>::type >::value ||
    //                std::is_same < int32_t, typename std::remove_extent<T>::type >::value) && sizeof (value[0]) == 4) {
    //            return Array32;
    //        } else if ((std::is_same < uint64_t, typename std::remove_reference<T>::type>::value ||
    //                std::is_same < int64_t, typename std::remove_reference<T>::type >::value ||
    //                std::is_same < uint64_t, typename std::remove_extent<T>::type >::value ||
    //                std::is_same < int64_t, typename std::remove_extent<T>::type >::value) && sizeof (value[0]) == 8) {
    //            return Array64;
    //        } else if ((std::is_same < float, typename std::remove_reference<T>::type>::value ||
    //                std::is_same < float, typename std::remove_extent<T>::type >::value) && sizeof (value[0]) == 4) {
    //            return ArrayFloat;
    //        } else if ((std::is_same < double, typename std::remove_reference<T>::type>::value ||
    //                std::is_same < double, typename std::remove_extent<T>::type >::value) && sizeof (value[0]) == 8) {
    //            return ArrayDouble;
    //        }
    //        return Error;
    //    }
    //
    //    template < typename T>
    //    typename std::enable_if<std::is_arithmetic<T>::value && !std::is_array<T>::value && !std::is_reference<T>::value && !std::is_pointer<T>::value, Type>::type
    //    inline static CalcDataType(T value) {
    //        if (std::is_same< bool, typename std::remove_cv<T>::type >::value && sizeof (T) == 1) {
    //            return Bool;
    //        } else if ((std::is_same < int8_t, typename std::remove_cv<T>::type >::value || std::is_same < uint8_t, typename std::remove_cv<T>::type >::value) && sizeof (T) == 1) {
    //            return Byte;
    //        } else if ((std::is_same < int16_t, typename std::remove_cv<T>::type >::value || std::is_same < uint16_t, typename std::remove_cv<T>::type >::value) && sizeof (T) == 2) {
    //            return Word;
    //        } else if ((std::is_same < int32_t, typename std::remove_cv<T>::type >::value || std::is_same < uint32_t, typename std::remove_cv<T>::type >::value) && sizeof (T) == 4) {
    //            return DWord;
    //        } else if ((std::is_same < int64_t, typename std::remove_cv<T>::type >::value || std::is_same < uint64_t, typename std::remove_cv<T>::type >::value) && sizeof (T) == 8) {
    //            return DDWord;
    //        } else if (std::is_same< float, typename std::remove_cv<T>::type >::value && sizeof (T) == 4) {
    //            return Float;
    //        } else if (std::is_same< double, typename std::remove_cv<T>::type >::value && sizeof (T) == 8) {
    //            return Double;
    //        }
    //        return Error;
    //    }
    //
    //    inline static Type CalcDataType(const char * value) {
    //        return String;
    //    }
    //
    //    template < typename T>
    //    typename std::enable_if<(std::is_arithmetic<T>::value ||
    //            (std::is_array<T>::value && std::is_arithmetic<typename std::remove_extent<T>::type>::value) ||
    //            (std::is_reference<T>::value && std::is_arithmetic<typename std::remove_reference<T>::type>::value)), size_t>::type
    //    inline static CalcDataSize(T &value) {
    //        return sizeof (T);
    //    }
    //
    //    inline static size_t CalcDataSize(uint8_t *value, size_t len) {
    //        if (value) {
    //            return len;
    //        }
    //        return 0;
    //    }
    //
    //    /*
    //     * When serializing strings, immediately save the null character to the end, 
    //     * so that it is convenient to process them like ordinary strings without having 
    //     * to copy the data to a temporary buffer or count characters in the string.
    //     */
    //    inline static size_t CalcDataSize(const char *value) {
    //        if (value) {
    //            return strlen(value) + 1;
    //        }
    //        return 0;
    //    }
    //
    //    /*
    //     * 
    //     */
    //
    //    static inline uint8_t FieldExtraSize(Type type) {
    //        switch (type) {
    //            case Blob:
    //            case Array16:
    //            case Array32:
    //            case Array64:
    //            case ArrayFloat:
    //            case ArrayDouble:
    //                return 1;
    //            case String:
    //                return 2;
    //            default:
    //                return 0;
    //        }
    //    }
    //
    //    static inline uint8_t FieldHeaderSize(const uint8_t *buffer) {
    //        Type type = FieldType(buffer);
    //        if (type != Error) {
    //            uint8_t size = (buffer[0] & (0xFF >> (8 - FIELDNAME_BIT)));
    //            return size + 1 + (type >> 3); // Most significant bit of type indicates data length is stored
    //        }
    //        return 0;
    //    }
    //
    //    static inline uint8_t MakeFieldHeader(const char *name, Type type) {
    //        if (name && type && strlen(name) <= FIELDNAME_MAX) {
    //            return ((uint8_t) type << FIELDNAME_BIT) | (strlen(name) & (0xFF >> (8 - FIELDNAME_BIT)));
    //        }
    //        return 0;
    //    }
    //
    //    static inline uint8_t MakeFieldHeader(const char *name, size_t name_size, Type type) {
    //        if (name && type && name_size <= FIELDNAME_MAX) {
    //            return ((uint8_t) type << FIELDNAME_BIT) | (name_size & (0xFF >> (8 - FIELDNAME_BIT)));
    //        }
    //        return 0;
    //    }
    //
    //    static inline size_t FieldLength(const uint8_t *buffer) {
    //        size_t result = 0;
    //        FieldName(buffer, &result);
    //        Type type = FieldType(buffer);
    //        if (result && type != Error) {
    //            result += FieldDataSize(buffer) + FieldExtraSize(type) + 1;
    //        }
    //        return result;
    //    }
    //
    //    template <typename T>
    //    typename std::enable_if<std::is_arithmetic<T>::value && !std::is_array<T>::value && !std::is_reference<T>::value, size_t>::type
    //    static inline Serialize(uint8_t *buffer, const size_t size, const T data) {
    //        if (size<sizeof (data)) {
    //            return 0;
    //        }
    //        if (std::is_integral<T>::value) {
    //            T value = htobe(data);
    //            memcpy(buffer, &value, sizeof (data));
    //        } else {
    //            // Float point not converted
    //            memcpy(buffer, &data, sizeof (data));
    //        }
    //        return sizeof (data);
    //    }
    //
    //    template <typename T>
    //    typename std::enable_if<std::is_array<T>::value && std::is_arithmetic<typename std::remove_extent<T>::type>::value, size_t>::type
    //    static inline Serialize(uint8_t *buffer, const size_t size, const T & data) {
    //        if (size<sizeof (data)) {
    //            return 0;
    //        }
    //        memcpy(buffer, &data, sizeof (data));
    //        if (std::is_integral<typename std::remove_extent<T>::type>::value) {
    //            typename std::remove_extent<T>::type *ptr = (typename std::remove_extent<T>::type *) (buffer);
    //            for (size_t count = 0; count < sizeof (data) / sizeof (data[0]); count++) {
    //                ptr[count] = htobe(ptr[count]);
    //            }
    //        }
    //        return sizeof (data);
    //    }
    //
    //    template <typename T>
    //    typename std::enable_if<std::is_integral<T>::value, size_t>::type
    //    static inline Deserialize(const uint8_t *buffer, const size_t size, T & data) {
    //        if (size != sizeof (data)) {
    //            memset(&data, 0, sizeof (data));
    //        }
    //        uint8_t byte;
    //        uint16_t word;
    //        uint32_t dword;
    //        uint64_t ddword;
    //        switch (size) {
    //            case 1:
    //                memcpy(&byte, buffer, size);
    //                data = byte;
    //                break;
    //            case 2:
    //                memcpy(&word, buffer, size);
    //                data = betoh(word);
    //                break;
    //            case 4:
    //                memcpy(&dword, buffer, size);
    //                data = betoh(dword);
    //                break;
    //            case 8:
    //                memcpy(&ddword, buffer, size);
    //                data = betoh(ddword);
    //                break;
    //            default:
    //                return 0;
    //        }
    //        return size;
    //    }
    //
    //    template <typename T>
    //    typename std::enable_if<!std::is_integral<T>::value && !std::is_array<T>::value && !std::is_reference<T>::value, size_t>::type
    //    static inline Deserialize(const uint8_t *buffer, const size_t size, T & data) {
    //        if (sizeof (data) < size) {
    //            return 0;
    //        }
    //        memcpy(&data, buffer, sizeof (data));
    //        return size;
    //    }
    //
    //    template <typename T>
    //    typename std::enable_if<std::is_array<T>::value && std::is_arithmetic<typename std::remove_extent<T>::type>::value, size_t>::type
    //    static inline Deserialize(const uint8_t *buffer, const size_t size, T & data) {
    //        if (sizeof (data) < size) {
    //            return 0;
    //        }
    //        memcpy(&data, buffer, sizeof (data));
    //        if (std::is_integral<typename std::remove_extent<T>::type>::value) {
    //            typename std::remove_extent<T>::type *ptr = (typename std::remove_extent<T>::type *) (&data);
    //            for (size_t count = 0; count < sizeof (data) / sizeof (data[0]); count++) {
    //                ptr[count] = htobe(ptr[count]);
    //            }
    //        }
    //        return size;
    //    }



    template <typename T>
    typename std::enable_if<std::is_integral<T>::value, bool>::type
    inline read_cbor_number(T &value) {
        if (!has_bytes(1)) {
            return false;
        }
        unsigned char type = get_byte();
        unsigned char major_type = type >> 5;
        unsigned char minor_type = (unsigned char) (type & 31);

        if (major_type == 0) {
            if (minor_type < 24) {
                value = minor_type;
                return true;
            } else if (minor_type == 24 && has_bytes(1)) { // 1 byte
                value = get_byte();
                return true;
            } else if (minor_type == 25 && sizeof (T) >= 2) { // 2 byte
                value = get_short();
                return true;
            } else if (minor_type == 26 && sizeof (T) >= 4) { // 4 byte
                value = get_int();
                return true;
            } else if (minor_type == 27 && sizeof (T) >= 8) { // 8 byte
                value = get_long();
                return true;
            }
        } else if (major_type == 1) {// negative integer
            if (minor_type < 24) {
                value = -1 - minor_type;
                return true;
            } else if (minor_type == 24) { // 1 byte
                value = -(int) get_byte() - 1;
                return true;
            } else if (minor_type == 25 && sizeof (T) >= 2) { // 2 byte
                value = -(int) get_short() - 1;
                return true;
            } else if (minor_type == 26 && sizeof (T) >= 4) { // 4 byte
                value = -(int) get_int() - 1;
                return true;
            } else if (minor_type == 27 && sizeof (T) >= 8) { // 8 byte
                value = -(int) get_long() - 1;
                return true;
            }
        } else if (major_type == 2) {// blob
            if (minor_type < 24) {
                value = minor_type;
                return true;
            } else if (minor_type == 24 && has_bytes(1)) { // 1 byte
                value = get_byte();
                return true;
            } else if (minor_type == 25 && sizeof (T) >= 2) { // 2 byte
                value = get_short();
                return true;
            } else if (minor_type == 26 && sizeof (T) >= 4) { // 4 byte
                value = get_int();
                return true;
            } else if (minor_type == 27 && sizeof (T) >= 8) { // 8 byte
                value = get_long();
                return true;
            }
        } else if (major_type == 3) {// string
            if (minor_type < 24) {
                value = minor_type;
                return true;
            } else if (minor_type == 24 && has_bytes(1)) { // 1 byte
                value = get_byte();
                return true;
            } else if (minor_type == 25 && sizeof (T) >= 2) { // 2 byte
                value = get_short();
                return true;
            } else if (minor_type == 26 && sizeof (T) >= 4) { // 4 byte
                value = get_int();
                return true;
            } else if (minor_type == 27 && sizeof (T) >= 8) { // 8 byte
                value = get_long();
                return true;
            }
        }
        return false;
    }

    inline bool has_bytes(uint8_t count) {
        return m_size - m_offset >= count;
    }

    inline unsigned char get_byte() {
        return m_data[m_offset++];
    }

    inline unsigned short get_short() {
        unsigned short value = ((unsigned short) m_data[m_offset] << 8) | ((unsigned short) m_data[m_offset + 1]);
        m_offset += 2;
        return value;
    }

    inline unsigned int get_int() {
        unsigned int value =
                ((unsigned int) m_data[m_offset ] << 24) |
                ((unsigned int) m_data[m_offset + 1] << 16) |
                ((unsigned int) m_data[m_offset + 2] << 8) |
                ((unsigned int) m_data[m_offset + 3]);
        m_offset += 4;
        return value;
    }

    inline unsigned long long get_long() {
        unsigned long long value = ((unsigned long long) m_data[m_offset] << 56) |
                ((unsigned long long) m_data[m_offset + 1] << 48) | ((unsigned long long) m_data[m_offset + 2] << 40) |
                ((unsigned long long) m_data[m_offset + 3] << 32) | ((unsigned long long) m_data[m_offset + 4] << 24) |
                ((unsigned long long) m_data[m_offset + 5] << 16) | ((unsigned long long) m_data[m_offset + 6] << 8) |
                ((unsigned long long) m_data[m_offset + 7]);
        m_offset += 8;
        return value;
    }


    SCOPE(private) :
    uint8_t *m_data;
    size_t m_size;
    size_t m_offset;
    bool m_read_only;

};
}
#endif /* MICROPROPERTY_H */

