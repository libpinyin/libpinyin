/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2016 Peng Wu <alexepico@gmail.com>
 *  
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 * 
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef FACADE_CHEWING_TABLE2_H
#define FACADE_CHEWING_TABLE2_H

#include "novel_types.h"
#include "chewing_large_table2.h"

namespace pinyin{

/**
 * FacadeChewingTable2:
 *
 * The facade class of chewing large table2.
 *
 */

class FacadeChewingTable2{
private:
    ChewingLargeTable2 * m_system_chewing_table;
    ChewingLargeTable2 * m_user_chewing_table;

    void reset() {
        if (m_system_chewing_table) {
            delete m_system_chewing_table;
            m_system_chewing_table = NULL;
        }

        if (m_user_chewing_table) {
            delete m_user_chewing_table;
            m_user_chewing_table = NULL;
        }
    }

public:
    /**
     * FacadeChewingTable2::FacadeChewingTable2:
     *
     * The constructor of the FacadeChewingTable2.
     *
     */
    FacadeChewingTable2() {
        m_system_chewing_table = NULL;
        m_user_chewing_table = NULL;
    }

    /**
     * FacadeChewingTable2::~FacadeChewingTable2:
     *
     * The destructor of the FacadeChewingTable2.
     *
     */
    ~FacadeChewingTable2() {
        reset();
    }

    bool load(const char * system_filename,
              const char * user_filename) {
        reset();

        bool result = false;
        if (system_filename) {
            m_system_chewing_table = new ChewingLargeTable2;
            result = m_system_chewing_table->attach
                (system_filename, ATTACH_READONLY) || result;
        }
        if (user_filename) {
            m_user_chewing_table = new ChewingLargeTable2;
            result = m_user_chewing_table->load_db
                (user_filename) || result;
        }
        return result;
    }

    bool store(const char * new_user_filename) {
        if (NULL == m_user_chewing_table)
            return false;
        return m_user_chewing_table->store_db(new_user_filename);
    }
};

};

#endif
