
# basicFunctions documentation
Contains basic data structures and functions

## Flags

### commonFlags
These are occuping the first 8 bits in the flags integer.
>0b1  
>Is allocated on the heap
  
>0b10  
>This objects mutex does exist
  
>0b100  
>This objects main list consists of pointers


### hashMapCuckoo flags

>0b10000000
>The last insert was not working, try inserting that if a problem occurs