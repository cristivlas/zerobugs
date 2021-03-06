#ifndef GENERIC_ATTR_H__5D0B31B3_52AD_4337_81EF_6DCD2AA4ED7B
#define GENERIC_ATTR_H__5D0B31B3_52AD_4337_81EF_6DCD2AA4ED7B
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

#include "zdk/config.h"
#include "zdk/stdexcept.h"
#include <dwarf.h>
#include <string.h>
#include <iomanip>
#include <string>
#include <typeinfo>
#include "attr.h"
#include "error.h"
#include "interface.h"
#include "utils.h"
#include "log.h"


namespace Dwarf
{
    using namespace std;


    template<Dwarf_Half A, typename T>
    CLASS GenericAttr : public Attribute
    {
    public:
        typedef T value_type;

        GenericAttr(Dwarf_Debug dbg, Dwarf_Die die)
            : Attribute(dbg, die, A)
        { }

        value_type value() const
        {
            value_type v = value_type();
            union
            {
                Dwarf_Unsigned u;
                Dwarf_Signed s;
                Dwarf_Bool b;
                Dwarf_Addr a;
                // char* str;
            } tmp = { 0 };

            Dwarf_Error err = 0;

            const Dwarf_Half f = this->form();
            switch (f)
            {
            case DW_FORM_addr:
                if (dwarf_formaddr(attr(), &tmp.a, &err) == DW_DLV_ERROR)
                {
                    THROW_ERROR(dbg(), err);
                }
                v = tmp.a;
                break;

            case DW_FORM_flag:
                if (dwarf_formflag(attr(), &tmp.b, &err) == DW_DLV_ERROR)
                {
                    THROW_ERROR(dbg(), err);
                }
                v = tmp.b;
                break;

            case DW_FORM_data1:
            case DW_FORM_data2:
            case DW_FORM_data4:
            case DW_FORM_data8:
            case DW_FORM_udata:
                if (dwarf_formudata(attr(), &tmp.u, &err) == DW_DLV_ERROR)
                {
                    THROW_ERROR(dbg(), err);
                }
                v = tmp.u;
                break;

            case DW_FORM_ref1:
            case DW_FORM_ref2:
            case DW_FORM_ref4:
            case DW_FORM_ref8:
            case DW_FORM_ref_addr:
                if (dwarf_global_formref(attr(), &tmp.u, &err) == DW_DLV_ERROR)
                {
                    THROW_ERROR(dbg(), err);
                }
                v = tmp.u;
                break;

            case DW_FORM_sdata:
                if (dwarf_formsdata(attr(), &tmp.s, &err) == DW_DLV_ERROR)
                {
                    THROW_ERROR(dbg(), err);
                }
                v = tmp.s;
                break;

    /* for T == char* use str()
            case DW_FORM_strp:
            case DW_FORM_string:
     */
            default:
                LOG_WARN << "GenericAttr<0x" << hex << A << ">\n"
                         << typeid(*this).name()
                         << ": unhandled form=0x"
                         << hex << f << dec << endl;
            }
            return v;
        }


        string str() const
        {
            char* s = 0;
            Dwarf_Error err = 0;

            switch(this->form())
            {
            case DW_FORM_strp:
            case DW_FORM_string:
                if (dwarf_formstring(this->attr(), &s, &err) == DW_DLV_ERROR)
                {
                    THROW_ERROR(dbg(), err);
                }
                break;

            default:
                LOG_WARN << "form=" << hex << this->form() << dec << endl;
                throw logic_error("str() invoked with incorrect attribute form");
            }

            string result;
            if (s)
            {
                result = s;
                dwarf_dealloc(dbg(), s, DW_DLA_STRING);
            }
            return result;
        }


        vector<char> block() const
        {
            Dwarf_Block* blkp = NULL;
            Dwarf_Error err = 0;
            vector<char> data;

            switch (this->form())
            {
            case DW_FORM_block:
            case DW_FORM_block1:
            case DW_FORM_block2:
            case DW_FORM_block4:
                if (dwarf_formblock(this->attr(), &blkp, &err) == DW_DLV_ERROR)
                {
                    THROW_ERROR(dbg(), err);
                }
                if (blkp)
                {
                    data.resize(blkp->bl_len);
                    memcpy(&data[0], blkp->bl_data, blkp->bl_len);
                }
                break;

            //default:
            //    throw logic_error("block() invoked with incorrect attribute form");
            }
            return data;
        }
    };
}

#endif // GENERIC_ATTR_H__5D0B31B3_52AD_4337_81EF_6DCD2AA4ED7B
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
