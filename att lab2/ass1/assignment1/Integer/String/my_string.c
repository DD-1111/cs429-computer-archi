#include "my_string.h"


size_t my_strlen ( const char * str ) 
{
    size_t length;  
  
    length = 0;  
  
    while (str[length] != '\0') {
        length++;
    } 
  
    return length;  
}

char * my_strcpy ( char * destination, const char * source )
{
    size_t i = 0;
    while (source[i] != '\0')
    {
        destination[i] = source[i];
        i++;
    }
    destination[i] = '\0';
    return destination;
}


char * my_strncpy ( char * destination, const char * source, size_t num)
{
    size_t i;
    while (i < num && source[i] != '\0') {
        destination[i] = source[i];
        i++;
    }
    while (i < num) {
        destination[i] = '\0';
        i++;
    }
    return destination;
}

void* my_memmove (void* destination, const void* source, size_t num)
{
    char *temp;
    temp = malloc(my_strlen(source));
    my_strncpy (temp, source, num);
    my_strncpy (destination, temp, num);
    return temp;
}
 
