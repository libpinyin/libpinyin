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
