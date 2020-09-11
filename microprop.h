#pragma once

#ifndef MICROPROPERTY_H
#define MICROPROPERTY_H

#include <stdint.h>

#ifndef STATIC_ASSERT
#define STATIC_ASSERT(expr) static_assert((expr), #expr)
#endif

#ifndef SCOPE
#define SCOPE(scope) scope
#endif

/*
 * Micro property
 * ==============
 * Library for minimal overhead serializing data for maximum savings size when transferring 
 * in embedded devices with limited memory size and low speed communication lines.
 * 
 * Written on C ++ x11 using the template engine SFINAE (Substitution failure is not an error).
 * 
 * Key features:
 * ------------
 * - Does not allocate memory. Works with external buffer only.
 * - Overhead for fiexed size field - 1 byte (excluding field name length).
 * - Overhead for blob, string and array field - 2 byte (excluding field name length).
 * - Limits for field name size (field ID) - 16 byte as string, blob or fixed length numbers.
 * - Limit on the size of variable length fields - 252 bytes.
 * - The total size of serialized data is not limited.
 * - Supports serialization of 8, 16, 32, 64 bit integers, bool, float and double types.
 * - Supports blob as arrays bytes.
 * - Supports null terminated string.
 * - Supports serialization of one-dimensional arrays for all types of numbers.
 * - Supports read-only mode. For example, when storing settings in the program flash memory of the microcontrollers. 
 * Takes into account the possibility of placing a buffer of serialized data in the cleared flash memory.
 * - In edit mode not support update field. Only adding new data fields is allowed.
 * - Although it is possible to edit data by pointer in the buffer, if necessary. 
 * But if required, the ability to update data fields can be added.
 * 
 * The following field keys types are allowed:
 * -------------------------------------------
 * - const char * - null terminated string.
 * - const char *, sizeof(id) - variable length binary data.
 * - (u)int(8..64)_t - one integral number.
 * 
 * The following data types are allowed in fields:
 * -------------------------------------------
 * - (u)int(8..64)_t - one integral number.
 * - float|double - one floating point number
 * - uint8_t *id, sizeof(id) - variable length binary data.
 * - const char * - null terminated string.
 * - (u)int(8..64)_t[] - one-dimensional array of integral number
 * - (float|double)[] - one-dimensional array of floating point number
 * 
 * Null terminated character strings:
 * ---------------------------------
 * To append and read null terminated character strings, use functions with the AsString suffix.
 * When reading data of a string type, the size of the read data is returned without a null terminator.
 * 
 * Fast use:
 * --------
 * #include "microprop.h"
 * 
 * Microprop prop(buffer, sizeof (buffer)); // Make Microprop object and assign buffer
 * 
 * prop.FieldExist(string || integer); // Check the presence of a field by its identifier
 * prop.FieldType(string || integer); // Determine the data type of a field
 * 
 * prop.Append(string || integer, true)); // Add bool field
 * prop.Read(string || integer, var_bool)); // Read bool field
 * 
 * Can read data into a larger buffer(value), i.e. Append(name, int8_t) => Read(name, int64_t)
 * 
 * Slow and thoughtful use:
 * ------------------------
 * 
 * prop.AssignBuffer(buffer, sizeof (buffer)); // Assign a buffer to use for edit mode
 * prop.AssignBuffer((const)buffer, sizeof (buffer)); // Assign a buffer for read only mode
 * prop.AssignBuffer(buffer, sizeof (buffer), true); // Assign a buffer for read only mode
 * prop.FieldNext(ptr); // Get a pointer to the next field to iterate over the stored data
 * prop.FieldName(string || integer, size_t *length = nullptr); // Get a pointer to a field name id
 * prop.FieldDataSize(string || integer); // The size of the data stored in the field
 * 
 * prop.Append(string || blob || integer, value || array); // Adding different data types and field ID
 * prop.Read(string || blob || integer, value || array); // Reading different data Types and field ID
 * 
 * prop.Append(string || blob || integer, uint8_t *, size_t); // Adding blob field
 * prop.Read(string || blob || integer, uint8_t *, size_t); // Reading blob field
 * 
 * prop.AppendAsString(string || blob || integer, string); // Adding field as null terminated string
 * const char * ReadAsString(string || blob || integer); // Reading field as null terminated string
 * 
 * Complete example of a class with overridden field key type:
 * -----------------------------------------------------------
 * class Property : public Microprop {
 * public:
 *   enum ID {
 *    ID1, ID2, ID3
 *  };
 * 
 *  template <typename ... Types>
 *  inline const uint8_t * FieldExist(ID id, Types ... arg) {
 *    return Microprop::FieldExist((uint8_t) id, arg...);
 *  }
 * 
 *  template <typename ... Types>
 *  inline size_t Append(ID id, Types ... arg) {
 *    return Microprop::Append((uint8_t) id, arg...);
 *  }
 * 
 *  template <typename T>
 *  inline size_t Read(ID id, T & val) {
 *    return Microprop::Read((uint8_t) id, val);
 *  }
 * 
 *  inline size_t Read(ID id, uint8_t *data, size_t size) {
 *    return Microprop::Read((uint8_t) id, data, size);
 *  }
 * 
 *  template <typename ... Types>
 *  inline size_t AppendAsString(ID id, Types ... arg) {
 *    return Microprop::AppendAsString((uint8_t) id, arg...);
 *  }
 * 
 *  template <typename ... Types>
 *  inline const char * ReadAsString(ID id, Types... arg) {
 *    return Microprop::ReadAsString((uint8_t) id, arg...);
 *  }
 *};
 * 
 */

class Microprop {
public:

    // Limits
    static constexpr uint8_t FIELDNAME_BIT = 4;
    static constexpr uint8_t FIELDNAME_MAX = 16;
    static constexpr uint8_t FIELDSIZE_MAX = 252U;

    /**
     * Field data types
     */
    enum Type {
        Removed = 0, ///< Reserved for use when editing fields. Now handled as an error.
        Bool = 1,
        Byte = 2,
        Word = 3,
        DWord = 4,
        DDWord = 5,
        Float = 6,
        Double = 7,
        Blob = 8, ///< (u)int8_t & or (u)int8_t * + size
        String = 9, ///< const char *
        Array16 = 10, ///< (u)int16_t &
        Array32 = 11, ///< (u)int32_t &
        Array64 = 12, ///< (u)int64_t &
        ArrayFloat = 13, ///< float &
        ArrayDouble = 14, ///< double &
        Error = 15, ///< Not saved when serializing data, but used when running and equal to the value when reading erased flash memory
    };

    Microprop(uint8_t *data, size_t size, bool read_only = false) {
        AssignBuffer(data, size, read_only);
    }

    explicit Microprop(const uint8_t *data, size_t size) {
        AssignBuffer(const_cast<uint8_t *> (data), size, true);
    }

    Microprop() : Microprop((uint8_t *) nullptr, 0) {
    }

    virtual ~Microprop() {
    }

    bool AssignBuffer(uint8_t *data, size_t size, bool read_only = false) {
        m_data = data;
        m_size = size;
        m_used = 0;
        m_read_only = read_only;
        return data && size;
    }

    bool AssignBuffer(const uint8_t *data, size_t size) {
        return AssignBuffer(const_cast<uint8_t *> (data), size, true);
    }

    inline void Clear() {
        m_used = 0;
    }

    inline size_t GetUsed() {
        return m_used;
    }

    inline size_t GetSize() {
        return m_size;
    }

    inline uint8_t * GetBuffer() {
        return m_data;
    }

    inline bool IsReadOnly() {
        return m_read_only;
    }

    /**
     * Check for the presence of a field with the specified identifier. The search always starts from the beginning of the buffer
     * @param name Field identifier as a null-terminated text string
     * @return Returns a buffer pointer to the beginning of the field with the specified ID
     */
    inline const uint8_t * FieldExist(const char *name) {
        return FieldExist(name, strlen(name));
    }

    /**
     * Check for the presence of a field with the specified identifier. The search always starts from the beginning of the buffer
     * @param name Field identifier as integer. The number is converted to begin endian order.
     * @return Returns a buffer pointer to the beginning of the field with the specified ID
     */
    template <typename T>
    typename std::enable_if<std::is_integral<T>::value, const uint8_t *>::type
    inline FieldExist(T name) {
        name = htobe(name);
        return FieldExist((const char *) &name, sizeof (name));
    }

    /**
     * Check for the presence of a field with the specified identifier. The search always starts from the beginning of the buffer
     * @param name Field identifier as array of bytes of a given size
     * @return Returns a buffer pointer to the beginning of the field with the specified ID
     */
    const uint8_t * FieldExist(const char *name, uint8_t name_size) {
        if (!name || !name_size || !m_data) {
            return nullptr;
        }
        const uint8_t *buffer = m_data;
        const char *field_name;
        size_t field_name_size;
        while (buffer) {
            field_name = FieldName(buffer, &field_name_size);
            if (field_name_size == name_size && strncmp(field_name, name, name_size) == 0) {
                return buffer;
            }
            buffer = FieldNext(buffer);
        }
        return nullptr;
    }

    /**
     * Determine the data type of a field
     * @param buffer Pointer to the beginning of the field
     * @return Field data types
     */
    static inline Type FieldType(const uint8_t *buffer) {
        STATIC_ASSERT(0xFF >> FIELDNAME_BIT == Type::Error);
        STATIC_ASSERT(Type::Error == 0xF);
        return buffer ? static_cast<Type> (buffer[0] >> FIELDNAME_BIT) : Error;
    }

    /**
     * Get a pointer to a field name id
     * @param buffer Pointer to the beginning of the field
     * @param length Size of field name identifier in bytes, if required
     * @return Pointer to the field name, or nullptr on error. The number of bytes of the name returned in the length parameter
     */
    static inline const char * FieldName(const uint8_t *buffer, size_t *length = nullptr) {
        Type type = FieldType(buffer);
        //@todo Need to implement processing of remote fields?
        if (type != Error && type != Removed) {
            if (length) {
                *length = (buffer[0] & (0xFF >> (8 - FIELDNAME_BIT)));
            }
            // Most significant bit of type indicates data length is stored
            if (type >> 3) {
                return reinterpret_cast<const char *> (&buffer[2]);
            }
            return reinterpret_cast<const char *> (&buffer[1]);
        }
        if (length) {
            *length = 0;
        }
        return nullptr;
    }

    /**
     * The size of the data stored in the field. The length of strings is specified including the terminating null character
     * @param buffer Pointer to the beginning of the field
     * @return Size of field data or 0 on error.
     */
    static inline size_t FieldDataSize(const uint8_t *buffer) {
        switch (FieldType(buffer)) {
            case Bool: return 1;
            case Byte: return 1;
            case Word: return 2;
            case DWord: return 4;
            case DDWord: return 8;
            case Float: return 4;
            case Double: return 8;

            case Blob:
            case String:
            case Array16:
            case Array32:
            case Array64:
            case ArrayFloat:
            case ArrayDouble:
                return buffer[1];
            default:
                break;
                //Removed && Error - 0
        }
        return 0;
    }

    /**
     * Get a pointer to the next field to iterate over the stored data
     * @param buffer Pointer to field
     * @return Returns a buffer pointer to the next field, or nullptr on error.
     */
    const uint8_t * FieldNext(const uint8_t *buffer) {
        if (!buffer || !m_data) {
            return nullptr;
        }
        size_t used = ((size_t) buffer - (size_t) m_data);
        if (used >= m_size || (m_used && used >= m_used)) {
            return nullptr;
        }
        size_t len = FieldLength(buffer);
        Type type = FieldType(buffer);
        //@todo Need to implement processing of remote fields?
        if (type != Error && type != Removed && len > 0 && len <= FIELDSIZE_MAX && (len + used) <= m_size && (m_used == 0 || (len + used) <= m_used)) {
            return buffer + len;
        }
        return nullptr;
    }

    /*
     * 
     * Define helpers for SFINAE templates
     * 
     */

#define MP_TEMPLATE_INT_KEY_ONLY(TYPE, RESULT)     \
    template < typename TYPE> \
    typename std::enable_if<std::is_integral<TYPE>::value, RESULT>::type

#define MP_TEMPLATE_ARITHMETIC_ONLY(TYPE) \
    template < typename TYPE> \
    typename std::enable_if<std::is_arithmetic<TYPE>::value && !std::is_array<TYPE>::value && !std::is_reference<TYPE>::value && !std::is_pointer<TYPE>::value, size_t>::type

#define MP_TEMPLATE_ARITHMETIC_FOR_INT_KEY(N,T)     \
    template < typename N, typename T> \
    typename std::enable_if<std::is_integral<N>::value && std::is_arithmetic<T>::value && \
    !std::is_array<T>::value && !std::is_reference<T>::value && !std::is_pointer<T>::value, size_t>::type

#define MP_TEMPLATE_ARRAY_ONLY(T)     \
    template < typename T> \
    typename std::enable_if<(std::is_array<T>::value && std::is_arithmetic<typename std::remove_extent<T>::type>::value) || \
    (std::is_reference<T>::value && std::is_arithmetic<typename std::remove_reference<T>::type>::value), size_t>::type

#define MP_TEMPLATE_ARRAY_FOR_INT_KEY(N,T)     \
    template < typename N, typename T> \
    typename std::enable_if<std::is_integral<N>::value && \
    ((std::is_array<T>::value && std::is_arithmetic<typename std::remove_extent<T>::type>::value) || \
    (std::is_reference<T>::value && std::is_arithmetic<typename std::remove_reference<T>::type>::value)), size_t>::type

    /*
     * 
     * 
     * Append field as simple numbers
     * 
     * 
     */

    MP_TEMPLATE_ARITHMETIC_FOR_INT_KEY(N, T)
    inline Append(N name, T value) {
        name = htobe(name);
        return Append((const char *) &name, sizeof (name), value);
    }

    MP_TEMPLATE_ARITHMETIC_ONLY(T)
    inline Append(const char *name, T value) {
        return Append(name, strlen(name), value);
    }

    MP_TEMPLATE_ARITHMETIC_ONLY(T)
    Append(const char *name, uint8_t name_size, T value) {
        if (m_read_only) {
            return false;
        }

        uint8_t data_size = CalcDataSize(value);
        if (!m_data || !name || !name_size || !data_size) {
            return false;
        }
        if ((data_size + name_size + 1U) > FIELDSIZE_MAX || (data_size + name_size + 1U + m_used) > m_size) {
            return false;
        }
        Type type = CalcDataType(value);
        m_data[m_used] = MakeFieldHeader(name, name_size, type);
        if (!m_data[m_used]) {
            return false;
        }
        m_used++;
        memcpy(&m_data[m_used], name, name_size);
        m_used += name_size;
        size_t result = Serialize(&m_data[m_used], m_size - m_used, value);
        m_used += result;
        return result;
    }

    /*
     * 
     * Read field as simple numbers and one-dimensional arrays for all types of numbers
     * 
     */

    template < typename T>
    inline size_t Read(const char *name, T & value) {
        return Read(name, strlen(name), value);
    }

    /*
     * Covers all variants for an integer key type, i.e. all types of numbers and arrays
     */
    template < typename N, typename T>
    typename std::enable_if<std::is_integral<N>::value && (std::is_arithmetic<T>::value ||
            (std::is_array<T>::value && std::is_arithmetic<typename std::remove_extent<T>::type>::value) ||
            (std::is_reference<T>::value && std::is_arithmetic<typename std::remove_reference<T>::type>::value)), size_t>::type
    inline Read(N name, T & value) {
        name = htobe(name);
        return Read((const char *) &name, sizeof (name), value);
    }

    template < typename T>
    size_t Read(const char *name, uint8_t name_size, T & value) {
        const uint8_t * field = FieldExist(name, name_size);
        uint8_t size = FieldDataSize(field);
        Type field_type = FieldType(field);
        if (field && size && field_type != Error) {
            Type data_type = CalcDataType(value);
            // For array check data type
            // Most significant bit of type indicates about of array data
            if ((data_type >> 3) && field_type != data_type) {
                return 0;
            }
            uint8_t offset = FieldHeaderSize(field);
            if (offset) {
                return Deserialize(const_cast<uint8_t *> (&field[offset]), size, value);
            }
        }
        return 0;
    }

    /*
     * 
     * Append fields as one-dimensional arrays for all types of numbers
     * 
     */

    MP_TEMPLATE_ARRAY_FOR_INT_KEY(N, T)
    inline Append(N name, T & value) {
        name = htobe(name);
        return Append((const char *) &name, sizeof (name), value);
    }

    MP_TEMPLATE_ARRAY_ONLY(T)
    inline Append(const char *name, T & value) {
        return Append(name, strlen(name), value);
    }

    MP_TEMPLATE_ARRAY_ONLY(T)
    Append(const char *name, uint8_t name_size, T & value) {
        if (m_read_only) {
            return false;
        }

        uint8_t extra_size = FieldExtraSize(CalcDataType(value));
        uint8_t data_size = CalcDataSize(value);
        if (!m_data || !name || !name_size || !data_size) {
            return false;
        }
        if ((data_size + name_size + extra_size) > FIELDSIZE_MAX || (data_size + name_size + extra_size + m_used) > m_size) {
            return false;
        }
        Type type = CalcDataType(value);
        m_data[m_used] = MakeFieldHeader(name, name_size, type);
        if (!m_data[m_used]) {
            return false;
        }
        m_used++;
        if (extra_size) {
            m_data[m_used] = CalcDataSize(value);
            m_used++;
        }
        memcpy(&m_data[m_used], name, name_size);
        m_used += name_size;
        size_t result = Serialize(&m_data[m_used], m_size - m_used, value);
        m_used += result;
        return result;
    }

    /*
     * 
     * Generic methods for append binary data
     * 
     */
    MP_TEMPLATE_INT_KEY_ONLY(T, size_t)
    inline Append(T name, uint8_t *data, size_t data_size, Type type = Type::Blob) {
        name = htobe(name);
        return Append((const char *) &name, sizeof (name), data, data_size, type);
    }

    inline size_t Append(const char *name, uint8_t *data, size_t data_size, Type type = Type::Blob) {
        return Append(name, strlen(name), data, data_size, type);
    }

    size_t Append(const char *name, uint8_t name_size, uint8_t *data, size_t data_size, Type type = Type::Blob) {
        if (m_read_only || !m_data || !name || !name_size || !data || !data_size) {
            return false;
        }
        uint8_t extra_size = FieldExtraSize(type) + 1;
        if ((data_size + name_size + extra_size) > FIELDSIZE_MAX || (data_size + name_size + extra_size + m_used) > m_size) {
            return false;
        }
        m_data[m_used] = MakeFieldHeader(name, name_size, type);
        if (!m_data[m_used]) {
            return false;
        }
        m_used++;
        m_data[m_used] = data_size;
        m_used++;
        memcpy(&m_data[m_used], name, name_size);
        m_used += name_size;
        memcpy(&m_data[m_used], data, data_size);
        m_used += data_size;
        if (type == String) {
            m_data[m_used] = '\0';
            m_used++;
        }
        return data_size;
    }

    /*
     * 
     * Generic methods for read binary data
     * 
     */

    MP_TEMPLATE_INT_KEY_ONLY(T, size_t)
    inline Read(T name, uint8_t *data, size_t size) {
        name = htobe(name);
        return Read((const char *) &name, sizeof (name), data, size);
    }

    inline size_t Read(const char *name, uint8_t *data, size_t data_size) {
        return Read(name, strlen(name), data, data_size);
    }

    size_t Read(const char *name, uint8_t name_size, uint8_t *data, size_t size) {
        const uint8_t * field = FieldExist(name, name_size);
        uint8_t blob_size = FieldDataSize(field);
        uint8_t offset = FieldHeaderSize(field);
        Type field_type = FieldType(field);
        if (field && blob_size && offset && field_type == Blob) {
            memcpy(data, const_cast<uint8_t *> (&field[offset]), blob_size);
            return blob_size;
        }
        return 0;
    }

    /*
     * 
     * Append fields as string
     * 
     */
    inline size_t AppendAsString(const char *name, const char * str) {
        return AppendAsString(name, strlen(name), str);
    }

    MP_TEMPLATE_INT_KEY_ONLY(T, size_t)
    inline AppendAsString(T name, const char * str) {
        name = htobe(name);
        return AppendAsString((const char *) &name, sizeof (T), str);
    }

    inline size_t AppendAsString(const char *name, uint8_t name_size, const char * str) {
        return Append(name, name_size, (uint8_t *) str, strlen(str), String);
    }

    /*
     * 
     * Read fields as string
     * 
     */

    inline const char * ReadAsString(const char * name, size_t *length = nullptr) {
        return ReadAsString(name, strlen(name), length);
    }

    MP_TEMPLATE_INT_KEY_ONLY(T, const char *)
    inline ReadAsString(T name, size_t *length = nullptr) {
        name = htobe(name);
        return ReadAsString((const char *) &name, sizeof (name), length);
    }

    const char * ReadAsString(const char * name, size_t name_len, size_t *length = nullptr) {
        const uint8_t * field = FieldExist(name, name_len);
        Type type = FieldType(field);
        uint8_t offset = FieldHeaderSize(field);
        if (field && offset && type == String) {
            if (length) {
                *length = FieldDataSize(field);
            }
            return (const char *) &field[offset];
        }
    }

    /*
     * 
     * Converting to and from network byte order for integers
     * 
     */
    template <typename T>
    typename std::enable_if<std::is_arithmetic<T>::value && !std::is_reference<T>::value && !std::is_array<T>::value, T>::type
    inline static constexpr htobe(T value, char* ptr = 0) noexcept {
        return
#if __BYTE_ORDER == __LITTLE_ENDIAN
        ptr = reinterpret_cast<char*> (&value),
                std::reverse(ptr, ptr + sizeof (T)),
#endif
                value;
    }

    template <typename T>
    typename std::enable_if<std::is_reference<T>::value && std::is_arithmetic<typename std::remove_reference<T>::type>::value, T>::type
    inline static constexpr htobe(T & value, char* ptr = 0) noexcept {
        return
#if __BYTE_ORDER == __LITTLE_ENDIAN
        ptr = reinterpret_cast<char*> (&value),
                std::reverse(ptr, ptr + sizeof (T)),
#endif
                value;
    }

    template <typename T>
    typename std::enable_if<std::is_arithmetic<T>::value && !std::is_reference<T>::value && !std::is_array<T>::value, T>::type
    inline static constexpr betoh(T value, char* ptr = 0) noexcept {
        STATIC_ASSERT(std::is_arithmetic<T>::value);
        return
#if __BYTE_ORDER == __LITTLE_ENDIAN
        ptr = reinterpret_cast<char*> (&value),
                std::reverse(ptr, ptr + sizeof (T)),
#endif
                value;
    }

    template <typename T>
    typename std::enable_if<std::is_reference<T>::value && std::is_arithmetic<typename std::remove_reference<T>::type>::value, T>::type
    inline static constexpr betoh(T & value, char* ptr = 0) noexcept {
        STATIC_ASSERT(std::is_arithmetic<T>::value);
        return
#if __BYTE_ORDER == __LITTLE_ENDIAN
        ptr = reinterpret_cast<char*> (&value),
                std::reverse(ptr, ptr + sizeof (T)),
#endif
                value;
    }

    /*
     * To use inner classes when customizing derived objects.
     */
    SCOPE(protected) :

    template < typename T>
    typename std::enable_if<std::is_pointer<T>::value ||
    (std::is_array<T>::value && std::is_arithmetic<typename std::remove_extent<T>::type>::value) ||
    (std::is_reference<T>::value && std::is_arithmetic<typename std::remove_reference<T>::type>::value), Type>::type
    inline static CalcDataType(T &value) {
        if ((std::is_same < uint8_t, typename std::remove_reference<T>::type>::value ||
                std::is_same < int8_t, typename std::remove_reference<T>::type >::value ||
                std::is_same < uint8_t, typename std::remove_extent<T>::type >::value ||
                std::is_same < int8_t, typename std::remove_extent<T>::type >::value) && sizeof (value[0]) == 1) {
            return Blob;
        } else if ((std::is_same < uint16_t, typename std::remove_reference<T>::type>::value ||
                std::is_same < int16_t, typename std::remove_reference<T>::type >::value ||
                std::is_same < uint16_t, typename std::remove_extent<T>::type >::value ||
                std::is_same < int16_t, typename std::remove_extent<T>::type >::value) && sizeof (value[0]) == 2) {
            return Array16;
        } else if ((std::is_same < uint32_t, typename std::remove_reference<T>::type>::value ||
                std::is_same < int32_t, typename std::remove_reference<T>::type >::value ||
                std::is_same < uint32_t, typename std::remove_extent<T>::type >::value ||
                std::is_same < int32_t, typename std::remove_extent<T>::type >::value) && sizeof (value[0]) == 4) {
            return Array32;
        } else if ((std::is_same < uint64_t, typename std::remove_reference<T>::type>::value ||
                std::is_same < int64_t, typename std::remove_reference<T>::type >::value ||
                std::is_same < uint64_t, typename std::remove_extent<T>::type >::value ||
                std::is_same < int64_t, typename std::remove_extent<T>::type >::value) && sizeof (value[0]) == 8) {
            return Array64;
        } else if ((std::is_same < float, typename std::remove_reference<T>::type>::value ||
                std::is_same < float, typename std::remove_extent<T>::type >::value) && sizeof (value[0]) == 4) {
            return ArrayFloat;
        } else if ((std::is_same < double, typename std::remove_reference<T>::type>::value ||
                std::is_same < double, typename std::remove_extent<T>::type >::value) && sizeof (value[0]) == 8) {
            return ArrayDouble;
        }
        return Error;
    }

    template < typename T>
    typename std::enable_if<std::is_arithmetic<T>::value && !std::is_array<T>::value && !std::is_reference<T>::value && !std::is_pointer<T>::value, Type>::type
    inline static CalcDataType(T value) {
        if (std::is_same< bool, typename std::remove_cv<T>::type >::value && sizeof (T) == 1) {
            return Bool;
        } else if ((std::is_same < int8_t, typename std::remove_cv<T>::type >::value || std::is_same < uint8_t, typename std::remove_cv<T>::type >::value) && sizeof (T) == 1) {
            return Byte;
        } else if ((std::is_same < int16_t, typename std::remove_cv<T>::type >::value || std::is_same < uint16_t, typename std::remove_cv<T>::type >::value) && sizeof (T) == 2) {
            return Word;
        } else if ((std::is_same < int32_t, typename std::remove_cv<T>::type >::value || std::is_same < uint32_t, typename std::remove_cv<T>::type >::value) && sizeof (T) == 4) {
            return DWord;
        } else if ((std::is_same < int64_t, typename std::remove_cv<T>::type >::value || std::is_same < uint64_t, typename std::remove_cv<T>::type >::value) && sizeof (T) == 8) {
            return DDWord;
        } else if (std::is_same< float, typename std::remove_cv<T>::type >::value && sizeof (T) == 4) {
            return Float;
        } else if (std::is_same< double, typename std::remove_cv<T>::type >::value && sizeof (T) == 8) {
            return Double;
        }
        return Error;
    }

    inline static Type CalcDataType(const char * value) {
        return String;
    }

    template < typename T>
    typename std::enable_if<(std::is_arithmetic<T>::value ||
            (std::is_array<T>::value && std::is_arithmetic<typename std::remove_extent<T>::type>::value) ||
            (std::is_reference<T>::value && std::is_arithmetic<typename std::remove_reference<T>::type>::value)), size_t>::type
    inline static CalcDataSize(T &value) {
        return sizeof (T);
    }

    inline static size_t CalcDataSize(uint8_t *value, size_t len) {
        if (value) {
            return len;
        }
        return 0;
    }

    /*
     * When serializing strings, immediately save the null character to the end, 
     * so that it is convenient to process them like ordinary strings without having 
     * to copy the data to a temporary buffer or count characters in the string.
     */
    inline static size_t CalcDataSize(const char *value) {
        if (value) {
            return strlen(value) + 1;
        }
        return 0;
    }

    /*
     * 
     */
    template < typename T>
    typename std::enable_if<(std::is_array<T>::value && std::is_arithmetic<typename std::remove_extent<T>::type>::value) ||
    (std::is_reference<T>::value && std::is_arithmetic<typename std::remove_reference<T>::type>::value), size_t>::type
    inline static CalcFieldSize(const char *name, T &value) {
        size_t size = CalcDataSize(value);
        if (size && name) {
            return strlen(name) + size + 1;
        }
        return 0;
    }

    template < typename T>
    typename std::enable_if<std::is_arithmetic<T>::value && !std::is_array<T>::value && !std::is_reference<T>::value && !std::is_pointer<T>::value, size_t>::type
    inline static CalcFieldSize(const char *name, T &value) {
        size_t size = CalcDataSize(value);
        if (size && name) {
            return strlen(name) + size;
        }
        return 0;
    }

    template < typename T>
    inline static size_t CalcFieldSize(const char *name, T &value, size_t size) {
        size = CalcDataSize(value, size);
        if (size && name) {
            return strlen(name) + size + 1;
        }
        return 0;
    }

    inline static size_t CalcFieldSize(const char *name, const char * str) {
        size_t size = CalcDataSize(str);
        if (size && name) {
            return strlen(name) + size + 1;
        }
        return 0;
    }

    static inline uint8_t FieldExtraSize(Type type) {
        switch (type) {
            case Blob:
            case Array16:
            case Array32:
            case Array64:
            case ArrayFloat:
            case ArrayDouble:
                return 1;
            case String:
                return 2;
            default:
                return 0;
        }
    }

    static inline uint8_t FieldHeaderSize(const uint8_t *buffer) {
        Type type = FieldType(buffer);
        if (type != Error) {
            uint8_t size = (buffer[0] & (0xFF >> (8 - FIELDNAME_BIT)));
            return size + 1 + (type >> 3); // Most significant bit of type indicates data length is stored
        }
        return 0;
    }

    static inline uint8_t MakeFieldHeader(const char *name, Type type) {
        if (name && type && strlen(name) <= FIELDNAME_MAX) {
            return ((uint8_t) type << FIELDNAME_BIT) | (strlen(name) & (0xFF >> (8 - FIELDNAME_BIT)));
        }
        return 0;
    }

    static inline uint8_t MakeFieldHeader(const char *name, size_t name_size, Type type) {
        if (name && type && name_size <= FIELDNAME_MAX) {
            return ((uint8_t) type << FIELDNAME_BIT) | (name_size & (0xFF >> (8 - FIELDNAME_BIT)));
        }
        return 0;
    }

    static inline size_t FieldLength(const uint8_t *buffer) {
        size_t result = 0;
        FieldName(buffer, &result);
        Type type = FieldType(buffer);
        if (result && type != Error) {
            result += FieldDataSize(buffer) + FieldExtraSize(type) + 1;
        }
        return result;
    }

    template <typename T>
    typename std::enable_if<std::is_arithmetic<T>::value && !std::is_array<T>::value && !std::is_reference<T>::value, size_t>::type
    static inline Serialize(uint8_t *buffer, const size_t size, const T data) {
        if (size<sizeof (data)) {
            return 0;
        }
        if (std::is_integral<T>::value) {
            T value = htobe(data);
            memcpy(buffer, &value, sizeof (data));
        } else {
            // Float point not converted
            memcpy(buffer, &data, sizeof (data));
        }
        return sizeof (data);
    }

    template <typename T>
    typename std::enable_if<std::is_array<T>::value && std::is_arithmetic<typename std::remove_extent<T>::type>::value, size_t>::type
    static inline Serialize(uint8_t *buffer, const size_t size, const T & data) {
        if (size<sizeof (data)) {
            return 0;
        }
        memcpy(buffer, &data, sizeof (data));
        if (std::is_integral<typename std::remove_extent<T>::type>::value) {
            typename std::remove_extent<T>::type *ptr = (typename std::remove_extent<T>::type *) (buffer);
            for (size_t count = 0; count < sizeof (data) / sizeof (data[0]); count++) {
                ptr[count] = htobe(ptr[count]);
            }
        }
        return sizeof (data);
    }

    template <typename T>
    typename std::enable_if<std::is_integral<T>::value, size_t>::type
    static inline Deserialize(const uint8_t *buffer, const size_t size, T & data) {
        if (size != sizeof (data)) {
            memset(&data, 0, sizeof (data));
        }
        uint8_t byte;
        uint16_t word;
        uint32_t dword;
        uint64_t ddword;
        switch (size) {
            case 1:
                memcpy(&byte, buffer, size);
                data = byte;
                break;
            case 2:
                memcpy(&word, buffer, size);
                data = betoh(word);
                break;
            case 4:
                memcpy(&dword, buffer, size);
                data = betoh(dword);
                break;
            case 8:
                memcpy(&ddword, buffer, size);
                data = betoh(ddword);
                break;
            default:
                return 0;
        }
        return size;
    }

    template <typename T>
    typename std::enable_if<!std::is_integral<T>::value && !std::is_array<T>::value && !std::is_reference<T>::value, size_t>::type
    static inline Deserialize(const uint8_t *buffer, const size_t size, T & data) {
        if (sizeof (data) < size) {
            return 0;
        }
        memcpy(&data, buffer, sizeof (data));
        return size;
    }

    template <typename T>
    typename std::enable_if<std::is_array<T>::value && std::is_arithmetic<typename std::remove_extent<T>::type>::value, size_t>::type
    static inline Deserialize(const uint8_t *buffer, const size_t size, T & data) {
        if (sizeof (data) < size) {
            return 0;
        }
        memcpy(&data, buffer, sizeof (data));
        if (std::is_integral<typename std::remove_extent<T>::type>::value) {
            typename std::remove_extent<T>::type *ptr = (typename std::remove_extent<T>::type *) (&data);
            for (size_t count = 0; count < sizeof (data) / sizeof (data[0]); count++) {
                ptr[count] = htobe(ptr[count]);
            }
        }
        return size;
    }


    SCOPE(private) :
    uint8_t *m_data;
    size_t m_size;
    size_t m_used;
    bool m_read_only;

};

#endif /* MICROPROPERTY_H */

