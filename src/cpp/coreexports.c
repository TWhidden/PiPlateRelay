#include <spibase.h>

int create_new_class(void** ppClassPtr, int param1, int param2 )
{
 auto x = new SPIBase(param1, param2);

*ppClassPtr = x;
return 0;
}