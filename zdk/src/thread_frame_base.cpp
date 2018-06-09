//
// $Id$
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "zdk/utility.h"
#include "zdk/thread_util.h"

#ifdef DEBUG
 #include <iostream>
#endif

using namespace std;


addr_t
thread_frame_base(Thread& thread, Frame& frame, Symbol& func)
{
#if defined (__i386__) || defined(__x86_64__)
    if (thread.is_32_bit())
    {
        uint32_t code[4] = { 0, 0, 0, 0 };
        size_t n = 0;

        assert(func.addr() >= func.offset());

        addr_t addr = func.addr() - func.offset();

        thread_read(thread, addr, code, &n);

        // check for the following function prologue,
        // generated by GCC 4.1.x
        //
        //  LEA ECX, [ESP + 0x04]
        //  AND ESP, BYTE 0xf0 // round up to 16
        //  PUSH DWORD [ECX + 0xfc]
        //  PUSH EBP
        //  MOV EBP, ESP
        //  [  PUSH EBX  ]
        //  PUSH ECX
        if ((code[0] == 0x04244c8d || code[0] == 0x04244ccc)
          && code[1] == 0xfff0e483 && code[2] == 0x8955fc71)
        {
        #ifdef DEBUG
            clog << __func__ << ": GCC 4.1.x prologue detected" << endl;
        #endif
            addr = frame.frame_pointer();
            if ((code[3] & 0xffffff) == 0x5153e5) // push EBX?
            {
                addr -= 8; // 2 registers saved above EBP
            }
            else
            {
                addr -= 4; // 1 reg saved above EBP
            }
            thread_read(thread, addr, addr, &n);
            return addr;
        }
    }
#endif // x86

    return frame.frame_pointer();
}

// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
