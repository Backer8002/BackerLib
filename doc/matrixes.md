**Matrix Definition and basic operations on the data structure**
All integers should be assumed to be unsigned unless otherwise stated.

- struct Matrix
    Contains 6 values
    The first value is the pointer to the actuall 2D array. This 2D array migth consist of diffrent types of vars and therefore is a void pointer to pointer. 

    The second value is a 16-bit integer containing the value of the amount of columns the matrix has.

    The third value is a 16-bit integer containing the value of the amount of rows the matrix has.

    The fourth value is a 8-bit integer containing a value for what type of var the matrix is. 0 for 8-bit integer, 1 for 16-bit integer, 2 for 32-bit integer,3 for 32-bit signed integer. The type of value is determained by the modulus as to keep data usage low.

    The fifth value is a 32-bit integer representing the modulus the values in the matrix are working with. As a number in Z/(modulus). Mod 1 represents Real numbers.

    The sixth value represents the total size of the pointers to the inner arrays and of the values containd within the 2D array.

- matrixCreate
    Returns a pointer to struct Matrix. Returns void if failure occurs.

    Purpuse
        Creates a struct matrix from input vars and returns a pointer to the struct. It allocates the struct on the heap. This function is a way to ensure consistancy in the creation of the matrix. 

    Args 1 and 2 are the dimention of the matrix. Arg 1 = columns Arg 2 = Rows. Both 16-bit integers

    Arg 3 is the modulus of the matrix. This determains the size of the matrix vars as per requierment for operation. This is a 32-bit integer. 0 is not a valid input and will make the function return NULL.

    Arg 4 is a pointer to a 64-bit integer. This governs the memory usage as a matrix won't be allocated if it requiers more than this value. This will also decrement the value by the requierd size if the matrix is allocated.

- matrixFree
    Is a void funtion

    Purpose
        Frees the matrix struct and 2D array.
    
    Arg 1 is the pointer to the matrix struct.¨

    Arg 2 is a pointer to a 64-bit integer that is the max mem usage of the matrixes. This will increment the value by the sixth value in the matrixes struct.

- matrixGetPointer
    Returns a void pointer to an induvidual element in the 2D array of the matrix. Returns NULL if request is out of range.

    Purpose
        Easy access to induvidual elements so that they can be mutated.
    
    Arg 1 is the pointer to the matrix struct that the pointer is gotten from.

    Arg 2 and 3 is the row and column of the matrix. (column, row)

- matrixToStringUint8
    Returns a string of the matrix where each row is seperated by a newline. Returns an eamty string if the matrix uses another var type than 8-bit integer.
    Returns NULL if allocation fails for the string.

    Purpose
        To return a string representation of the matrix.
        Use matrixOutput for larger matrixes since this would get to large for a computer to store on larger varible types.

    Arg 1 is the pointer to the matrix struct.

- isWithinModulus
    Returns true if within the modulus range of the matrix
    Returns false otherwise.

    Purpose
        To check if a value should be able to be assigned to the matrix without breaking the conventions of math.

    Arg 1 is the pointer to the matrix struc.

    Arg 2 is the value to compare that is a 32-bit integer. *As real numbers don't have to be compared*