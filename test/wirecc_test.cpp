#include <wirecc/wirecc.h>
#include <iostream>
#include <cassert>
#include <vector>
#include <map>
#include <set>
#include <cstring>
#include <cstdlib>
#include <ctime>

using namespace WireCC;

void testAssert(bool condition, const char* message);
void printSummary();

void test_endian_functions() {
    std::cout << "\n=== Testing Endian Functions ===" << std::endl;

    // Test 64-bit encoding/decoding
    {
        uint64_t original = 0x123456789ABCDEF0ULL;
        uint8_t buffer[8];
        be64encode(original, buffer);
        uint64_t decoded = be64decode(buffer);
        testAssert(original == decoded, "be64encode/decode roundtrip");

        // Test specific byte order
        testAssert(buffer[0] == 0x12, "be64encode byte 0 correct");
        testAssert(buffer[1] == 0x34, "be64encode byte 1 correct");
        testAssert(buffer[7] == 0xF0, "be64encode byte 7 correct");
    }

    // Test 32-bit encoding/decoding
    {
        uint32_t original = 0x12345678;
        uint8_t buffer[4];
        be32encode(original, buffer);
        uint32_t decoded = be32decode(buffer);
        testAssert(original == decoded, "be32encode/decode roundtrip");

        testAssert(buffer[0] == 0x12, "be32encode byte 0 correct");
        testAssert(buffer[3] == 0x78, "be32encode byte 3 correct");
    }

    // Test 16-bit encoding/decoding
    {
        uint16_t original = 0x1234;
        uint8_t buffer[2];
        be16encode(original, buffer);
        uint16_t decoded = be16decode(buffer);
        testAssert(original == decoded, "be16encode/decode roundtrip");

        testAssert(buffer[0] == 0x12, "be16encode byte 0 correct");
        testAssert(buffer[1] == 0x34, "be16encode byte 1 correct");
    }
}

void test_byte_buffer() {
    std::cout << "\n=== Testing ByteBuffer ===" << std::endl;

    ByteBuffer buffer;

    // Test initial state
    testAssert(buffer.size() == 0, "ByteBuffer initial size is 0");
    testAssert(buffer.getPos() == 0, "ByteBuffer initial position is 0");

    // Test uint64_t operations
    {
        buffer.clear();
        uint64_t original = 0x123456789ABCDEF0ULL;
        buffer.writeU64(original);
        testAssert(buffer.size() == 8, "ByteBuffer size after writeU64");

        buffer.setPos(0);
        uint64_t read_val;
        buffer.readU64(read_val);
        testAssert(read_val == original, "ByteBuffer uint64_t roundtrip");
    }

    // Test unsigned int operations
    {
        buffer.clear();
        unsigned int original = 0x12345678;
        buffer.writeUint(original);
        testAssert(buffer.size() == 4, "ByteBuffer size after writeUint");

        buffer.setPos(0);
        unsigned int read_val;
        buffer.readUint(read_val);
        testAssert(read_val == original, "ByteBuffer uint roundtrip");
    }

    // Test int operations
    {
        buffer.clear();
        int original = -12345;
        buffer.writeInt(original);

        buffer.setPos(0);
        int read_val;
        buffer.readInt(read_val);
        testAssert(read_val == original, "ByteBuffer int roundtrip");
    }

    // Test string operations
    {
        buffer.clear();
        std::string original = "Hello, WireCC!";
        buffer.writeString(original);

        buffer.setPos(0);
        std::string read_val;
        buffer.readString(read_val);
        testAssert(read_val == original, "ByteBuffer string roundtrip");
    }

    // Test C-string operations
    {
        buffer.clear();
        const char* original = "C-style string";
        buffer.writeCstring(original);

        buffer.setPos(0);
        std::string read_val;
        buffer.readString(read_val);
        testAssert(read_val == std::string(original), "ByteBuffer C-string roundtrip");
    }

    // Test bool operations
    {
        buffer.clear();
        buffer.writeBool(true);
        buffer.writeBool(false);

        buffer.setPos(0);
        bool val1, val2;
        buffer.readBool(val1);
        buffer.readBool(val2);
        testAssert(val1 == true && val2 == false, "ByteBuffer bool roundtrip");
    }

    // Test ResourceSet operations
    {
        buffer.clear();
        ResourceSet original;
        original.insert(1);
        original.insert(5);
        original.insert(10);

        buffer.writeRset(original);

        buffer.setPos(0);
        ResourceSet read_val;
        buffer.readRset(read_val);
        testAssert(read_val == original, "ByteBuffer ResourceSet roundtrip");
    }

    // Test ByteBuffer operations
    {
        ByteBuffer buf1, buf2;
        buf1.writeUint(12345);
        buf1.writeString("test");

        buf2.writeBuffer(buf1);

        buf2.setPos(0);
        ByteBuffer buf3;
        buf2.readBuffer(buf3);

        buf3.setPos(0);
        unsigned int val;
        std::string str;
        buf3.readUint(val);
        buf3.readString(str);

        testAssert(val == 12345 && str == "test", "ByteBuffer buffer roundtrip");
    }

    // Test load and concat
    {
        uint8_t data[] = {0x01, 0x02, 0x03, 0x04};
        buffer.clear();
        buffer.load(data, 4);
        testAssert(buffer.size() == 4, "ByteBuffer load size");

        buffer.concat(data, 2);
        testAssert(buffer.size() == 6, "ByteBuffer concat size");
    }
}

void test_bitmap() {
    std::cout << "\n=== Testing Bitmap ===" << std::endl;

    Bitmap bitmap(8);  // 8-bit bitmap for testing

    // Test initial state
    testAssert(bitmap.isEmpty(), "Bitmap initially empty");
    testAssert(!bitmap.isFull(), "Bitmap initially not full");
    testAssert(bitmap.getFlags() == 0, "Bitmap flags initially 0");

    // Test setting bits
    bitmap.set(0);
    bitmap.set(3);
    bitmap.set(7);

    // Test getFlags()
    uint64_t flags = bitmap.getFlags();
    testAssert(flags == 0b10010001, "Bitmap flags set correctly");

    testAssert(bitmap.isSet(0), "Bitmap bit 0 is set");
    testAssert(!bitmap.isSet(1), "Bitmap bit 1 is not set");
    testAssert(bitmap.isSet(3), "Bitmap bit 3 is set");
    testAssert(bitmap.isSet(7), "Bitmap bit 7 is set");
    testAssert(!bitmap.isEmpty(), "Bitmap not empty after setting bits");

    // Test unsetting bits
    bitmap.unset(3);
    testAssert(!bitmap.isSet(3), "Bitmap bit 3 unset");

    // Test full bitmap
    for (int i = 0; i < 8; i++) {
        bitmap.set(i);
    }
    testAssert(bitmap.isFull(), "Bitmap is full after setting all bits");

    // Test clear
    bitmap.clear();
    testAssert(bitmap.isEmpty(), "Bitmap empty after clear");
}

void test_iterator() {
    std::cout << "\n=== Testing Iterator ===" << std::endl;

    ResourceSet rset;
    rset.insert(1);
    rset.insert(3);
    rset.insert(5);

    ResourceIterator iter(rset);
    testAssert(iter.count == 3, "Iterator count correct");

    std::vector<ResourceId> collected;
    while (iter.current != iter.end) {
        collected.push_back(*iter.current);
        ++iter.current;
    }

    testAssert(collected.size() == 3, "Iterator collected all elements");
    testAssert(collected[0] == 1 && collected[1] == 3 && collected[2] == 5,
                "Iterator collected correct elements");

    // Test empty iterator
    ResourceIterator empty_iter;
    testAssert(empty_iter.count == 0, "Empty iterator count is 0");
}

void test_get_iterator_from_map() {
    std::cout << "\n=== Testing getIteratorFromMap ===" << std::endl;

    std::map<ResourceId, ResourceSet> resource_map;
    ResourceSet set1, set2;
    set1.insert(10);
    set1.insert(20);
    set2.insert(30);
    set2.insert(40);

    resource_map[1] = set1;
    resource_map[2] = set2;

    ResourceIterator iter1 = getIteratorFromMap(resource_map, 1);
    testAssert(iter1.count == 2, "getIteratorFromMap found correct set");

    ResourceIterator iter_invalid = getIteratorFromMap(resource_map, 999);
    testAssert(iter_invalid.count == 0, "getIteratorFromMap returns empty for invalid key");
}

void test_combination_generator() {
    std::cout << "\n=== Testing CombinationGenerator ===" << std::endl;

    std::set<int> pool;
    pool.insert(1);
    pool.insert(2);
    pool.insert(3);
    pool.insert(4);

    CombinationGenerator<std::set<int>> gen(&pool, 2);

    int combinations = 0;
    while (gen.hasNext()) {
        auto combo = gen.get();
        combinations++;
        testAssert(combo.size() == 2, "Combination has correct size");
    }

    // C(4,2) = 6 combinations
    testAssert(combinations == 6, "CombinationGenerator produced correct number of combinations");
}

void test_random_generator() {
    std::cout << "\n=== Testing RandomGenerator ===" << std::endl;

    std::map<int, std::string> pool;
    pool[1] = "one";
    pool[2] = "two";
    pool[3] = "three";

    RandomGenerator<int, std::string> gen(&pool);

    std::set<int> generated;
    for (int i = 0; i < 3; i++) {
        int val = gen.get();
        generated.insert(val);
        testAssert(pool.find(val) != pool.end(), "RandomGenerator returned valid key");
    }

    testAssert(generated.size() == 3, "RandomGenerator returned all unique keys");

    // Test reset
    gen.reset();
    int val = gen.get();
    testAssert(pool.find(val) != pool.end(), "RandomGenerator works after reset");
}

void test_dealloc_values() {
    std::cout << "\n=== Testing deallocValues ===" << std::endl;

    std::map<int, std::string*> test_map;
    test_map[1] = new std::string("test1");
    test_map[2] = new std::string("test2");
    test_map[3] = new std::string("test3");

    // This should deallocate all the string pointers
    deallocValues(test_map.begin(), test_map.end());

    // Check that pointers are set to NULL
    bool all_null = true;
    for (auto& pair : test_map) {
        if (pair.second != NULL) {
            all_null = false;
            break;
        }
    }

    testAssert(all_null, "deallocValues set all pointers to NULL");
}

void test_edge_cases() {
    std::cout << "\n=== Testing Edge Cases ===" << std::endl;

    // Test empty string in ByteBuffer
    {
        ByteBuffer buffer;
        std::string empty_str = "";
        buffer.writeString(empty_str);

        buffer.setPos(0);
        std::string read_str;
        buffer.readString(read_str);
        testAssert(read_str == empty_str, "ByteBuffer handles empty string");
    }

    // Test empty ResourceSet in ByteBuffer
    {
        ByteBuffer buffer;
        ResourceSet empty_set;
        buffer.writeRset(empty_set);

        buffer.setPos(0);
        ResourceSet read_set;
        buffer.readRset(read_set);
        testAssert(read_set.empty(), "ByteBuffer handles empty ResourceSet");
    }

    // Test single-bit bitmap
    {
        Bitmap bitmap(1);
        testAssert(bitmap.isEmpty(), "Single-bit bitmap initially empty");

        bitmap.set(0);
        testAssert(bitmap.isFull(), "Single-bit bitmap full after setting bit 0");
    }

    // Test zero-size combination generator
    {
        std::set<int> pool;
        pool.insert(1);
        pool.insert(2);

        CombinationGenerator<std::set<int>> gen(&pool, 0);
        testAssert(gen.hasNext(), "Zero-size combination generator has next");

        auto combo = gen.get();
        testAssert(combo.empty(), "Zero-size combination is empty");
    }
}

int main(void) {
    std::cout << "Running WireCC Tests..." << std::endl;

    // Seed random number generator for RandomGenerator tests
    srand(time(NULL));

    // Run all test suites
    test_endian_functions();
    test_byte_buffer();
    test_bitmap();
    test_iterator();
    test_get_iterator_from_map();
    test_combination_generator();
    test_random_generator();
    test_dealloc_values();
    test_edge_cases();

    printSummary();
}
