#pragma once
#include <vlutil/vlNetTypes.h>


//typedef DtNetU8 DtNetByteArray10[10];
typedef unsigned char byte;
class DtNetLogInInfo10
{
public:
    //DtNetU8     DtCharacterSet;
#define DtMaxLogInInfoLength                  10
    DtNetU8     DtText[DtMaxLogInInfoLength];
};
