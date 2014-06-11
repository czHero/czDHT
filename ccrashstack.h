#ifndef CCRASHSTACK_H
#define CCRASHSTACK_H

#include <windows.h>
#include <QString>

//
//  ccrashstack.h
//
//
//  Created by Lan huaiyu on 9/27/13.
//  Copyright 2013 CCrashStack. All rights reserved.
//

class CCrashStack
{
private:
    PEXCEPTION_POINTERS m_pException;

private:
    QString GetModuleByRetAddr(PBYTE Ret_Addr, PBYTE & Module_Addr);
    QString GetCallStack(PEXCEPTION_POINTERS pException);
    QString GetVersionStr();


public:
    CCrashStack(PEXCEPTION_POINTERS pException);

    QString GetExceptionInfo();
};

#endif // CCRASHSTACK_H
