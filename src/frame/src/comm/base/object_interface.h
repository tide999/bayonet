/*=============================================================================
#
#     FileName: object_interface.h
#         Desc: 基类的接口定义
#
#       Author: dantezhu
#        Email: zny2008@gmail.com
#     HomePage: http://www.vimer.cn
#
#      Created: 2011-06-08 16:35:01
#      Version: 0.0.1
#      History:
#               0.0.1 | dantezhu | 2011-06-08 16:35:01 | initialization
#
=============================================================================*/
#ifndef __OBJECT_INTERFACE_H__
#define __OBJECT_INTERFACE_H__
#include <typeinfo>
#include <iostream>
#include <string>
using namespace std;

class IObject
{
public:
    virtual ~IObject() {}
    
    /**
     * @brief   获取名字，可以用来做统计使用
     *
     * @return  名字
     */
    virtual const string Name()
    {
        const char * name = typeid(*this).name();

        char szTmp[strlen(name)+1];

        sscanf(name, "%*d%s", szTmp);

        return szTmp;
    }
};

#endif