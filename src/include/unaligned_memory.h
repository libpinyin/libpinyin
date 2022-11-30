/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2022 Matias Larsson
 *  
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef UNALIGNED_MEMORY_H
#define UNALIGNED_MEMORY_H

#include <cstring>

/**
 * UnalignedMemory: Safe unaligned memory access.
 * 
 * Some instruction sets, or some instructions in some instruction sets
 * require that memory access is aligned to a specific boundary. These
 * instructions may trap on unaligned access.
 * 
 * This class provides methods to load and store values at unaligned
 * addresses. It ensures that the compiler doesn't generate instructions
 * that could trap on the unaligned memory access.
 */

namespace pinyin{
    template <typename T>
    class UnalignedMemory{
    public:
        /**
         * Read a value from a possibly unaligned memory address.
         * 
         */
        static T load(const void * src) {
            T value;
            memcpy(&value, src, sizeof(T));
            return value;
        }

        /**
         * Store a value into a possibly unaligned memory address.
         * 
         */
        static void store(T value, void * dest) {
            memcpy(dest, &value, sizeof(T));
        }
    };
};


#endif
