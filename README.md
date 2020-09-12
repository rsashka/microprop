Micro property
==============
Library for minimal overhead serializing data for maximum savings size when transferring in embedded devices with limited memory size and low speed communication lines.

Подробное описание на Хабре https://habr.com/ru/post/518846/

Written on C ++ x11 using the template engine SFINAE (Substitution failure is not an error).

Key features:
------------
- Does not allocate memory. Works with external buffer only.
- Overhead for fiexed size field - 1 byte (excluding field name length).
- Overhead for blob, string and array field - 2 byte (excluding field name length).
- Limits for field name size (field ID) - 16 byte as string, blob or fixed length numbers.
- Limit on the size of variable length fields - 252 bytes.
- The total size of serialized data is not limited.
- Supports serialization of 8, 16, 32, 64 bit integers, bool, float and double types.
- Supports blob as arrays bytes.
- Supports null terminated string.
- Supports serialization of one-dimensional arrays for all types of numbers.
- Supports read-only mode. For example, when storing settings in the program flash memory of the microcontrollers. Takes into account the possibility of placing a buffer of serialized data in the cleared flash memory.
- In edit mode not support update field. Only adding new data fields is allowed.
- Although it is possible to edit data by pointer in the buffer, if necessary. But if required, the ability to update data fields can be added.

The following field keys types are allowed:
-------------------------------------------
- const char * - null terminated string.
- const char *, sizeof(id) - variable length binary data.
- (u)int(8..64)_t - one integral number.

The following data types are allowed in fields:
-------------------------------------------
- (u)int(8..64)_t - one integral number.
- float|double - one floating point number
- uint8_t *id, sizeof(id) - variable length binary data.
- const char * - null terminated string.
- (u)int(8..64)_t[] - one-dimensional array of integral number
- (float|double)[] - one-dimensional array of floating point number
 
Null terminated character strings:
---------------------------------
To append and read null terminated character strings, use functions with the **AsString** suffix.
**When reading data of a string type, the size of the read data is returned without a null terminator.**

  
Fast use:
--------
```c++
#include "microprop.h"

Microprop prop(buffer, sizeof (buffer)); // Make Microprop object and assign buffer

prop.FieldExist(string || integer); // Check the presence of a field by its identifier
prop.FieldType(string || integer); // Determine the data type of a field

prop.Append(string || integer, true); // Add bool field
prop.Read(string || integer, var_bool); // Read bool field

``` 
Can read data into a larger buffer(value), i.e. Append(name, int8_t) => Read(name, int64_t)
 
Slow and thoughtful use:
------------------------
```c++

prop.AssignBuffer(buffer, sizeof (buffer)); // Assign buffer to use for edit mode
prop.AssignBuffer((const)buffer, sizeof (buffer)); // Assign buffer for read only mode
prop.AssignBuffer(buffer, sizeof (buffer), true); // Assign buffer for read only mode

prop.FieldNext(ptr); // Get a pointer to the next field to iterate over the stored data
prop.FieldName(string || integer, size_t *length = nullptr); // Get pointer to field name ID
prop.FieldDataSize(string || integer); // The size of the data stored in the field

prop.Append(string || blob || integer, value || array); // Adding any data types and field ID
prop.Read(string || blob || integer, value || array); // Reading any data types and field ID

prop.Append(string || blob || integer, uint8_t *, size_t); // Adding blob field
prop.Read(string || blob || integer, uint8_t *, size_t); // Reading blob field

prop.AppendAsString(string || blob || integer, string); // Adding field as null terminated string
const char * ReadAsString(string || blob || integer); // Reading field as null terminated string

```

Complete example of a class with overridden field key type:
------------------------
```c++

class Property : public Microprop {
public:
    enum ID {
    ID1, ID2, ID3
  };

  template <typename ... Types>
  inline const uint8_t * FieldExist(ID id, Types ... arg) {
    return Microprop::FieldExist((uint8_t) id, arg...);
  }

  template <typename ... Types>
  inline size_t Append(ID id, Types ... arg) {
    return Microprop::Append((uint8_t) id, arg...);
  }

  template <typename T>
  inline size_t Read(ID id, T & val) {
    return Microprop::Read((uint8_t) id, val);
  }

  inline size_t Read(ID id, uint8_t *data, size_t size) {
    return Microprop::Read((uint8_t) id, data, size);
  }

    
  template <typename ... Types>
  inline size_t AppendAsString(ID id, Types ... arg) {
    return Microprop::AppendAsString((uint8_t) id, arg...);
  }

  template <typename ... Types>
  inline const char * ReadAsString(ID id, Types... arg) {
    return Microprop::ReadAsString((uint8_t) id, arg...);
  }
};


```
