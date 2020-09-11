Micro property
==============
Library for minimal overhead serializing data for maximum savings size when transferring in embedded devices with limited memory size and low speed communication lines.

Written on C ++ x11 using the template engine SFINAE (Substitution failure is not an error).

Key features:
------------
- Does not allocate memory. Works with external buffer only.
- Overhead for fiexed size field - 1 byte (excluding field name length).
- Overhead for blob, string and array field - 2 byte (excluding field name length).
- Limits for field name size (field ID) - 16 byte as string, blob or fixed length numbers.
- Limit on the size of variable length fields - 252 bytes.
- The total size of serialized data is not limited.
- Supports serialization of 8, 16, 32, 64 bit integers and floating and double precision.
- Supports serialization of one-dimensional arrays for all types of numbers.
- Supports read-only mode. For example, when storing settings in the program flash memory of the microcontrollers. Takes into account the possibility of placing a buffer of serialized data in the cleared flash memory.
- In edit mode not support update field. Only adding new data fields is allowed.
- Although it is possible to edit data by pointer in the buffer, if necessary. But if required, the ability to update data fields can be added.
 
Fast use:
--------
```c++
#include "microprop.h"

Microprop prop(buffer, sizeof (buffer)); // Make Microprop object and assign buffer

prop.FieldExist(string || integer || enum); // Check the presence of a field by its identifier
prop.FieldType(string || integer || enum); // Determine the data type of a field

prop.Append(string || integer || enum, true)); // Add data field
prop.Read(string || integer || enum, var_bool)); // Read data field

``` 
Can read data into a larger buffer(value), i.e. Append(name, int8_t) => Read(name, int64_t)
 
Slow and thoughtful use:
------------------------
```c++

prop.AssignBuffer(buffer, sizeof (buffer)); // Assign a buffer to use for edit mode
prop.AssignBuffer((const)buffer, sizeof (buffer)); // Assign a buffer for read only mode
prop.AssignBuffer(buffer, sizeof (buffer), true); // Assign a buffer for read only mode

prop.FieldNext(ptr); // Get a pointer to the next field to iterate over the stored data
prop.FieldName(string || integer || enum, size_t *length = nullptr); // Get a pointer to a field name id
prop.FieldDataSize(string || integer || enum); // The size of the data stored in the field

prop.Append(string || integer || enum, value || array)); // Adding different data types and field ID
prop.Read(string || integer || enum, value || array)); // Reading different ata Types and field ID

```
