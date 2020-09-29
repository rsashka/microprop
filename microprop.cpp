#include "microprop.h"

using namespace microprop;

Encoder::Encoder() : Encoder((uint8_t *) nullptr, 0) {
}

Encoder::Encoder(uint8_t *data, size_t size) {
    AssignBuffer(data, size);
}

Encoder::~Encoder() {
}

bool Encoder::AssignBuffer(uint8_t *data, size_t size) {
    m_data = data;
    m_size = size;
    m_offset = 0;
    msgpack_packer_init(&m_pk, data, &msgpack_callback, this);
    return data && size;
}

bool Encoder::Write(KeyType id, uint8_t *data, size_t size) {
    size_t temp = m_offset;
    if(id && msgpack_write(id) && msgpack_pack_bin_with_body(&m_pk, data, size) == 0) {
        return true;
    }
    m_offset = temp;
    return false;
}

bool Encoder::WriteAsString(KeyType id, const char *str) {
    size_t len = strlen(str) + 1; // include null char 
    size_t temp = m_offset;
    if(id && msgpack_write(id) && msgpack_pack_str_with_body(&m_pk, str, len) == 0) {
        return true;
    }
    m_offset = temp;
    return false;
}

int Encoder::callback_func(void* data, const char* buf, size_t len) {
    assert(m_data == data);
    if(m_offset + len <= m_size) {
        memcpy(&m_data[m_offset], buf, len);
        m_offset += len;
        return 0;
    }
    return -1;
}

int Encoder::msgpack_callback(void* data, const char* buf, size_t len, void* callback_param) {
    Encoder * obj = (Encoder *) callback_param;
    if(obj) {
        return obj->callback_func(data, buf, len);
    }
    return -1;
}

/*
 * 
 */
Decoder::Decoder() : Decoder((uint8_t *) nullptr, 0) {
}

Decoder::Decoder(uint8_t *data, size_t size) {
    AssignBuffer(data, size);
}

Decoder::Decoder(const uint8_t *data, size_t size) {
    AssignBuffer((uint8_t *) data, size);
}

Decoder::~Decoder() {
}

bool Decoder::AssignBuffer(uint8_t *data, size_t size) {
    m_data = data;
    m_size = size;
    m_offset = 0;
    return data && size;
}

bool Decoder::FieldFind(KeyType id) {
    if(!m_data || !m_size || !check_key_type(m_data[0])) {
        return false;
    }
    m_offset = 0;
    KeyType field_id;
    while(FieldNext(field_id)) {
        if(field_id == id) {
            return true;
        }
    }
    return false;
}

bool Decoder::FieldNext(KeyType & id) {
    if(!m_data || !m_size || m_offset >= m_size) {
        return false;
    }
    if(m_offset != 0) {
        // Skip field data

        msgpack_unpacked msg;
        msgpack_unpacked_init(&msg);

        if(msgpack_unpack_next(&msg, (const char *) m_data, m_size, &m_offset) < 0) {
            return false;
        }

        assert(msg.zone == nullptr);

        // if the field data is an array, skip all its elements
        msgpack_object array = msg.data;
        if(array.type == MSGPACK_OBJECT_ARRAY) {
            size_t count = array.via.array.size;
            for (size_t i = 0; i < count; i++) {
                if(msgpack_unpack_next(&msg, (const char *) m_data, m_size, &m_offset) < 0) {
                    return false;
                }
                if(m_offset+(count-i) > m_size) {
                    return false;
                }
            }
        }

    }
    // read field id and move offset next msgpack value
    return check_key_type(m_data[m_offset]) && msgpack_read(id);
}

bool Decoder::Read(KeyType id, uint8_t *data, size_t size) {
    msgpack_unpacked msg;
    msgpack_unpacked_init(&msg);

    if(FieldFind(id) && msgpack_unpack_next(&msg, (const char *) m_data, m_size, &m_offset)) {

        // Must use only fixed static buffer
        assert(msg.zone == nullptr);

        msgpack_object value = msg.data;
        if(value.type == MSGPACK_OBJECT_BIN) {
            if(value.via.bin.size <= size) {
                memcpy(data, value.via.bin.ptr, value.via.bin.size);
                return true;
            }
        }
    }
    return false;
}

const char * Decoder::ReadAsString(KeyType id, size_t *length) {
    msgpack_unpacked msg;
    msgpack_unpacked_init(&msg);

    if(FieldFind(id) && msgpack_unpack_next(&msg, (const char *) m_data, m_size, &m_offset)) {

        // Must use only fixed static buffer
        assert(msg.zone == nullptr);

        msgpack_object value = msg.data;
        if(value.type == MSGPACK_OBJECT_STR && value.via.str.size) {
            if(length) {
                *length = value.via.str.size;
            }
            return value.via.str.ptr;
        }
    }
    if(length) {
        *length = 0;
    }
    return nullptr;
}
