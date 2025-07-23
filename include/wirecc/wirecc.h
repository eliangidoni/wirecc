#ifndef WIRECC_H_
#define WIRECC_H_

/**
 * @file
 * @addtogroup wirecc WireCC
 * @{
 */

#include <set>
#include <map>
#include <vector>
#include <stdint.h>
#include <string>
#include <cassert>
#include <algorithm>
#include <cmath>

#if WIRECC_DEBUG == 0
#define WIRECC_ASSERT(cond) do{} while(0)
#else
#define WIRECC_ASSERT(cond) do{ assert(cond); } while(0)
#endif

/**
 * @brief Disables copy constructor and copy assignment operator for a class.
 * @param type The class type for which to disable copy operations
 * @details This macro declares the copy constructor and copy assignment operator
 *          without providing implementations, effectively preventing
 *          copying of objects of the specified type. This is useful for classes
 *          that manage resources and should not be copied (e.g., RAII classes,
 *          singleton classes, or classes with unique ownership semantics).
 * @note This macro should be placed in the private section of the class declaration.
 * @code
 * class UniqueResource {
 * public:
 *     UniqueResource() {}
 * private:
 *     WIRECC_DISABLE_COPY_AND_ASSIGN(UniqueResource);
 * };
 *
 * UniqueResource res1;           // OK
 * UniqueResource res2 = res1;    // Compilation error
 * res2 = res1;                   // Compilation error
 * @endcode
 */
#define WIRECC_DISABLE_COPY_AND_ASSIGN(type) \
        type(const type&); \
        type& operator=(const type&)

namespace WireCC {
        // Resource type definitions.
        typedef int ResourceId;
        const int RESOURCE_INVALID = -1;
        template<typename T>
        struct Iterator {
                typename T::const_iterator current, end;
                unsigned int count;
                Iterator() : current(), end(), count(0) { current = end; }
                explicit Iterator(const T& t) : current(t.begin()), end(t.end()), count(t.size()) {}
        };
        typedef std::set<ResourceId> ResourceSet;
        typedef Iterator<ResourceSet> ResourceIterator;

        /**
         * @brief Deallocates values in a container range.
         * @tparam T Iterator type for the container
         * @param from Iterator pointing to the beginning of the range
         * @param to Iterator pointing to the end of the range
         * @details Iterates through the range and deletes the second element of each pair,
         *          setting it to NULL after deletion. Useful for cleaning up maps of pointers.
         */
        template<typename T>
        void deallocValues(T from, T to)
        {
                while (from != to){
                        if (from->second != NULL){
                                delete from->second;
                                from->second = NULL;
                        }
                        ++from;
                }
        }

        /**
         * @brief Gets a ResourceIterator from a map for a specific ResourceId.
         * @tparam T Map type containing ResourceId keys
         * @param from The map to search in
         * @param rid The ResourceId to find
         * @return ResourceIterator for the found resource set, or empty iterator if not found
         */
        template<typename T>
        ResourceIterator getIteratorFromMap(const T& from, ResourceId rid)
        {
                typename T::const_iterator itr = from.find(rid);
                if (itr != from.end()) {
                        return ResourceIterator(itr->second);
                }
                return ResourceIterator();
        }

        /**
         * @brief Generates combinations of elements from a container.
         * @tparam T Container type
         */
        template<typename T>
        class CombinationGenerator
        {
        public:
                typedef std::vector<typename T::value_type> ElemList;
                /**
                 * @brief Constructs a combination generator.
                 * @param pool Pointer to the container to generate combinations from
                 * @param sampleSize Number of elements in each combination
                 */
                CombinationGenerator(const T * pool, unsigned int sampleSize)
                        : from(pool), greater(false), ssize(sampleSize) {}
                /**
                 * @brief Checks if there are more combinations available.
                 * @return true if more combinations can be generated, false otherwise
                 */
                bool hasNext() {return (greater || (bmap.empty() && from->size() >= ssize));}
                /**
                 * @brief Gets the next combination.
                 * @return Vector containing the elements of the next combination
                 */
                ElemList get()
                {
                        ElemList ret;
                        // Fill selection bitmap
                        if (bmap.empty()){
                                bmap.assign(from->size(), false);
                                std::fill(bmap.begin() + from->size() - ssize, bmap.end(), true);
                                for (typename T::const_iterator itr = from->begin(); itr != from->end(); ++itr) {
                                        elems.push_back(*itr);
                                }
                        }
                        // Store and return permutation
                        for (unsigned int i=0; i < bmap.size(); ++i){
                                if (bmap[i]) {
                                        ret.push_back(elems[i]);
                                }
                        }
                        greater = std::next_permutation(bmap.begin(), bmap.end());
                        return ret;
                }
        protected:
                std::vector<bool> bmap;
                ElemList elems;
                const T* from;
                bool greater;
                unsigned int ssize;
        };

        /**
         * @brief Generates random elements from a map.
         * @tparam K Key type
         * @tparam V Value type
         */
        template<typename K, typename V>
        class RandomGenerator
        {
        public:
                /**
                 * @brief Constructs a random generator.
                 * @param pool Pointer to the map to generate random elements from
                 */
                RandomGenerator(std::map<K, V>* pool) : from(pool) {}
                /**
                 * @brief Resets the generator by clearing the internal element list.
                 */
                void reset() {elems.clear();}
                /**
                 * @brief Gets a random key from the map.
                 * @return A randomly selected key from the map
                 * @note The returned key is removed from the internal pool until reset() is called
                 */
                K get()
                {
                        // Fill random set
                        if (elems.empty()){
                                WIRECC_ASSERT(from->size() > 0);
                                typename std::map<K, V>::const_iterator itr = from->begin();
                                while (itr != from->end()){
                                        elems.push_back(itr->first);
                                        ++itr;
                                }
                        }
                        // Remove and return random element
                        typename std::vector<K>::iterator itr = elems.begin() + (rand() % elems.size());
                        K ret = *itr;
                        elems.erase(itr);
                        return ret;
                }
        protected:
                std::vector<K> elems;
                std::map<K, V>* from;
        };

        /**
         * @brief Encodes a 64-bit unsigned integer to big-endian byte array.
         * @param val The 64-bit value to encode
         * @param buf Buffer to store the encoded bytes (must be at least 8 bytes)
         */
        void be64encode(uint64_t val, uint8_t * buf)
        {
                buf[0] = ((val >> 56) & 0xff);
                buf[1] = ((val >> 48) & 0xff);
                buf[2] = ((val >> 40) & 0xff);
                buf[3] = ((val >> 32) & 0xff);
                buf[4] = ((val >> 24) & 0xff);
                buf[5] = ((val >> 16) & 0xff);
                buf[6] = ((val >> 8)  & 0xff);
                buf[7] = (val & 0xff);
        }

        /**
         * @brief Decodes a 64-bit unsigned integer from big-endian byte array.
         * @param buf Buffer containing the encoded bytes (must be at least 8 bytes)
         * @return The decoded 64-bit unsigned integer
         */
        uint64_t be64decode(const uint8_t * buf)
        {
                return ((uint64_t)(buf[7]<<0) | ((uint64_t)buf[6]<<8) |
                        ((uint64_t)buf[5]<<16) | ((uint64_t)buf[4]<<24) |
                        ((uint64_t)buf[3]<<32) | ((uint64_t)buf[2]<<40) |
                        ((uint64_t)buf[1]<<48) | ((uint64_t)buf[0]<<56));
        }

        /**
         * @brief Encodes a 32-bit unsigned integer to big-endian byte array.
         * @param val The 32-bit value to encode
         * @param buf Buffer to store the encoded bytes (must be at least 4 bytes)
         */
        void be32encode(uint32_t val, uint8_t * buf)
        {
                buf[0] = ((val >> 24) & 0xff);
                buf[1] = ((val >> 16) & 0xff);
                buf[2] = ((val >> 8)  & 0xff);
                buf[3] = (val & 0xff);
        }

        /**
         * @brief Decodes a 32-bit unsigned integer from big-endian byte array.
         * @param buf Buffer containing the encoded bytes (must be at least 4 bytes)
         * @return The decoded 32-bit unsigned integer
         */
        uint32_t be32decode(const uint8_t * buf)
        {
                return ((buf[3]<<0) | (buf[2]<<8) | (buf[1]<<16) | (buf[0]<<24));
        }

        /**
         * @brief Encodes a 16-bit unsigned integer to big-endian byte array.
         * @param val The 16-bit value to encode
         * @param buf Buffer to store the encoded bytes (must be at least 2 bytes)
         */
        void be16encode(uint16_t val, uint8_t * buf)
        {
                buf[0] = ((val >> 8) & 0xff);
                buf[1] = (val & 0xff);
        }

        /**
         * @brief Decodes a 16-bit unsigned integer from big-endian byte array.
         * @param buf Buffer containing the encoded bytes (must be at least 2 bytes)
         * @return The decoded 16-bit unsigned integer
         */
        uint16_t be16decode(const uint8_t * buf)
        {
                return ((buf[1]<<0) | (buf[0]<<8));
        }

        /**
         * @brief A byte buffer for reading and writing binary data.
         * @details Provides methods for serializing and deserializing various data types
         *          in big-endian format. Must call load() or clear() before use.
         * @warning Mixing byte reads and writes is not supported.
         * @warning Assumes two's complement representation for signed numbers.
         */
        class ByteBuffer
        {
        public:
                /**
                 * @brief Default constructor that clears the buffer.
                 */
                ByteBuffer() {clear();}

                /**
                 * @brief Reads a ByteBuffer from the current position.
                 * @param buffer The buffer to store the read data
                 */
                void readBuffer(ByteBuffer& buffer)
                {
                        unsigned int size;
                        readUint(size);
                        buffer.load(&buf[pos], size);
                        pos += size;
                }

                /**
                 * @brief Writes a ByteBuffer to the current position.
                 * @param b The buffer to write
                 */
                void writeBuffer(const ByteBuffer& b)
                {
                        writeUint(b.size());
                        buf.insert(buf.end(), b.data(), b.data()+b.size());
                        pos += b.size();
                }

                /**
                 * @brief Reads a 64-bit unsigned integer from the buffer.
                 * @param val Reference to store the read value
                 */
                void readU64(uint64_t& val)
                {
                        val = be64decode(&buf[pos]);
                        pos += sizeof(uint64_t);
                }

                /**
                 * @brief Writes a 64-bit unsigned integer to the buffer.
                 * @param val The value to write
                 */
                void writeU64(uint64_t val)
                {
                        buf.resize(buf.size() + sizeof(uint64_t));
                        be64encode(val, &buf[pos]);
                        pos += sizeof(uint64_t);
                }

                /**
                 * @brief Reads an unsigned integer from the buffer.
                 * @param val Reference to store the read value
                 */
                void readUint(unsigned int& val)
                {
                        val = be32decode(&buf[pos]);
                        pos += sizeof(uint32_t);
                }

                /**
                 * @brief Writes an unsigned integer to the buffer.
                 * @param val The value to write
                 */
                void writeUint(unsigned int val)
                {
                        buf.resize(buf.size() + sizeof(uint32_t));
                        be32encode(val, &buf[pos]);
                        pos += sizeof(uint32_t);
                }

                /**
                 * @brief Reads a signed integer from the buffer.
                 * @param val Reference to store the read value
                 */
                void readInt(int& val)
                {
                        unsigned int tmp;
                        readUint(tmp);
                        val = tmp;
                }

                /**
                 * @brief Writes a signed integer to the buffer.
                 * @param val The value to write
                 */
                void writeInt(int val)
                {
                        unsigned int tmp = val;
                        writeUint(tmp);
                }

                /**
                 * @brief Reads a ResourceSet from the buffer.
                 * @param val Reference to the ResourceSet to populate
                 */
                void readRset(ResourceSet& val)
                {
                        uint32_t size;
                        readUint(size);
                        while (size-- > 0){
                                ResourceId r;
                                readInt(r);
                                val.insert(r);
                        }
                }

                /**
                 * @brief Writes a ResourceSet to the buffer.
                 * @param val The ResourceSet to write
                 */
                void writeRset(const ResourceSet& val)
                {
                        uint32_t size = val.size();
                        writeUint(size);
                        for (ResourceIterator itr = ResourceIterator(val);
                             itr.current != itr.end; ++itr.current) {
                                writeInt(*itr.current);
                        }
                }

                /**
                 * @brief Reads a string from the buffer.
                 * @param val Reference to the string to populate
                 */
                void readString(std::string& val)
                {
                        uint32_t size;
                        readUint(size);
                        while (size-- > 0){
                                val.push_back(*(&buf[pos]));
                                ++pos;
                        }
                }

                /**
                 * @brief Writes a string to the buffer.
                 * @param val The string to write
                 */
                void writeString(const std::string& val)
                {
                        uint32_t size = val.size();
                        writeUint(size);
                        buf.resize(buf.size() + size);
                        for (unsigned int i=0; i < size; ++i){
                                *(&buf[pos]) = val[i];
                                ++pos;
                        }
                }

                /**
                 * @brief Writes a C-style string to the buffer.
                 * @param val The null-terminated string to write
                 */
                void writeCstring(const char * val)
                {
                        writeString(std::string(val));
                }

                /**
                 * @brief Reads a boolean value from the buffer.
                 * @param val Reference to store the read boolean value
                 */
                void readBool(bool& val)
                {
                        val = (*(&buf[pos]) != 0);
                        ++pos;
                }

                /**
                 * @brief Writes a boolean value to the buffer.
                 * @param val The boolean value to write
                 */
                void writeBool(bool val)
                {
                        buf.resize(buf.size() + 1);
                        *(&buf[pos]) = (val ? 1 : 0);
                        ++pos;
                }

                /**
                 * @brief Gets a pointer to the raw buffer data.
                 * @return Const pointer to the internal buffer
                 */
                const uint8_t * data() const {return &buf[0];}
                /**
                 * @brief Gets the size of the buffer.
                 * @return Size of the buffer in bytes
                 */
                unsigned int size() const {return buf.size();}
                /**
                 * @brief Clears the buffer and resets position to 0.
                 */
                void clear() {buf.clear(); pos = 0;}
                /**
                 * @brief Sets the current position in the buffer.
                 * @param newPos The new position to set
                 */
                void setPos(unsigned int newPos) {pos = newPos;}
                /**
                 * @brief Gets the current position in the buffer.
                 * @return Current position in bytes
                 */
                unsigned int getPos() const {return pos;}
                /**
                 * @brief Loads data into the buffer, clearing any existing content.
                 * @param data Pointer to the data to load
                 * @param size Size of the data in bytes
                 */
                void load(const uint8_t * data, unsigned int size)
                {
                        clear();
                        buf.insert(buf.begin(), data, data+size);
                }

                /**
                 * @brief Concatenates data to the end of the buffer.
                 * @param data Pointer to the data to concatenate
                 * @param size Size of the data in bytes
                 */
                void concat(const uint8_t * data, unsigned int size)
                {
                        buf.insert(buf.end(), data, data+size);
                        pos += size;
                }

        protected:
                std::vector<uint8_t> buf;
                unsigned int pos;
        };

        /**
         * @brief A bitmap class for managing bit flags.
         * @details Provides functionality to set, unset, and check individual bits
         *          within a 64-bit integer. Supports custom maximum bit counts.
         */
        class Bitmap
        {
        public:
                /**
                 * @brief Constructs a bitmap with specified maximum bits.
                 * @param maxBits Maximum number of bits to support (default: 64)
                 */
                Bitmap(uint8_t maxBits=64)
                {
                        clear();
                        mask = (uint64_t) std::pow(2, maxBits) - (uint64_t) 1;
                }

                /**
                 * @brief Checks if a specific bit is set.
                 * @param bit The bit position to check
                 * @return true if the bit is set, false otherwise
                 */
                bool isSet(unsigned int bit) const
                {
                        return ((flags & ((uint64_t) 1 << bit)) > 0);
                }

                /**
                 * @brief Checks if the bitmap is empty (no bits set).
                 * @return true if no bits are set, false otherwise
                 */
                bool isEmpty() const
                {
                        return (flags == (uint64_t) 0);
                }

                /**
                 * @brief Checks if the bitmap is full (all allowed bits set).
                 * @return true if all bits within the mask are set, false otherwise
                 */
                bool isFull() const
                {
                        return (flags == mask);
                }

                /**
                 * @brief Sets a specific bit.
                 * @param bit The bit position to set (must be < 64)
                 */
                void set(unsigned int bit)
                {
                        WIRECC_ASSERT(bit < 64);
                        flags |= ((uint64_t) 1 << bit);
                }

                /**
                 * @brief Unsets (clears) a specific bit.
                 * @param bit The bit position to unset
                 */
                void unset(unsigned int bit)
                {
                        flags &= (mask ^ ((uint64_t) 1 << bit));
                }

                /**
                 * @brief Clears all bits in the bitmap.
                 */
                void clear()
                {
                        flags = 0;
                }

                /**
                 * @brief Gets the current flags as a 64-bit unsigned integer.
                 * @return The current flags
                 */
                uint64_t getFlags() const
                {
                        return flags;
                }

        protected:
                uint64_t flags, mask;
        };
}

/** @} */
#endif