
# Types documentation
Contains basic data structures and functions

## Flags

### Common Flags
These are occupying the first 8 bits in the flags integer.
>0b1
>This object is valid

>0b10  
>This object is located on the heap.
  
>0b100  
>This object's mutex exists


### Type flags
The last 16 bits are reserved for object type information.
>0x00010000  
>This object may be used as a Container object.

>0x00020000  
>This object may be used as a DynamicContainer object.

>0x00040000  
>This object's array may not be continuous.  
>A
> **true** means occupied and **false** means unoccupied.

>0x00080000  
>This object's array may not be continuous and it does not store which indexes are used.  
>0x00040000 shall be specified.  
>Instead of a BitSet the object may provide a function pointer to get an iterator. This function pointer shall be prototyped as follows:
>```c++
>bool (*)(Iterator*,bool backwardsIterating)
>```
>Where the return value **true** shall mean that an element can be provided.

>0x00400000  
>This object's array consists of pointers to objects.

>0x00800000  
> This object's array shall not be sorted. It must be qualified as a Container for this to have effect.

### Object flags
The bits ranged from 9-16 are reserved for object specific purposes.

#### hashMapCuckoo flags

>0x0100
>The last insert was not working, try inserting that if a problem occurs

## Iterator
All types that derive from Container shall be able to create an iterator unless **0x00080000** is specified. Other types
may provide an iterator.
To obtain an iterator a function named **iteratorGet** exists with the following prototype:
```c++
Iterator iteratorGet(DataTypeHeader* obj)
```

To increment the iterator a function **iteratorInc** exists and to decrement a function **iteratorDec** exists.
The functions return false if there existed no more elements otherwise they return true.
```c++
bool iteratorInc(Iterator*)
bool iteratorDec(Iterator*)
```
To obtain the pointer to object from iterator a function **iteratorGet** exists and for a constant value **iteratorGetConst**.
NULL is returned if the iterator was not pointing to an obj.
```c++
void* const iteratorGet(Iterator*)
const void* const iteratorGetConst(iterator*)
```