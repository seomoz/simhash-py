#define CATCH_CONFIG_MAIN
#include "catch.hpp"

/* Include internals */
#include "cyclic.hpp"
/* And our tokenizer */
#include "tokenizers/strspn.h"
/* Include simhash */
#include "hash.hpp"

#include <string>
#include <stdint.h>

TEST_CASE("cyclic", "The cyclic hashing function stuff") {
    SECTION("rotation", "It can rotate integers by one bit") {
        /* Given these inputs... */
        uint64_t inputs[] = {
            0xDEADBEEFDEADBEEF, 0xAAAAAAAAAAAAAAAA, 0x5555555555555555
        };
        /* We should get these outputs */
        uint64_t outputs[] = {
            0xBD5B7DDFBD5B7DDF, 0x5555555555555555, 0xAAAAAAAAAAAAAAAA
        };
        /* How many examples do I have? */
        size_t count = 3;

        for (size_t i = 0; i < count; ++i) {
            REQUIRE(
                Simhash::Cyclic<uint64_t>::rotate(inputs[i]) == outputs[i]);
        }
    }

    SECTION("multiple rotation", "It can rotate by many bits") {
        REQUIRE(Simhash::Cyclic<uint64_t>::rotate(0xDEADBEEFDEADBEEF, 10)
            == 0xB6FBBF7AB6FBBF7A);
        REQUIRE(Simhash::Cyclic<uint64_t>::rotate(0xABCDEFFFABCDEFFF, 32)
            == 0xABCDEFFFABCDEFFF);
        REQUIRE(Simhash::Cyclic<uint64_t>::rotate(0xABCDEFFFABCDEFFF, 64)
            == 0xABCDEFFFABCDEFFF);
        /* Really large offsets should be rounded down and treated as ok */
        REQUIRE(Simhash::Cyclic<uint64_t>::rotate(0xABCDEFFFABCDEFFF, 1024)
            == 0xABCDEFFFABCDEFFF);
    }

    SECTION("cycling", "It can keep track of a cyclic hash correctly") {
        /* First, we'll instantiate a cyclic hasher with a window of 5 */
        Simhash::Cyclic<uint64_t> cyclic(5);
        /* Next, we're going to give this hash a sequence of longer than the
         * window, and then we'd like to make sure that the hashes that come
         * up are in fact different, and furthermore, that repeated sequences
         * that are sufficiently long will yield the same values */
        uint64_t values[] = {
            0xCAFEBABECAFEBABE,
            0xAAAAAAAAAAAAAAAA,
            0x5555555555555555,
            0xDEADBEEFDEADBEEF,
            0xFEEDEDFEEDEDFEED,
            0xBD5B7DDFBD5B7DDF,
            0xABEDABEDABEDABED,
            0xBADEBADEBADEBADE
        };
        const size_t count = 8;
        /* Now, we'll push each of these onto the cyclic hasher twice. The
         * first pass through, we expect the numbers to be different from the
         * second pass for the first four numbers. After that point, though,
         * the window should have sufficient time to reset */
        uint64_t results[2*count];
        for (size_t i = 0; i < 2; ++i) {
            for (size_t j = 0; j < count; ++j) {
                results[i*count+j] = cyclic.push(values[j]);
            }
        }
        REQUIRE(results[0] != results[ 8]); /* Unequal */
        REQUIRE(results[1] != results[ 9]);
        REQUIRE(results[2] != results[10]);
        REQUIRE(results[3] != results[11]);
        REQUIRE(results[4] == results[12]); /* Equal */
        REQUIRE(results[5] == results[13]);
        REQUIRE(results[6] == results[14]);
        REQUIRE(results[7] == results[15]);
    }
}

TEST_CASE("tokenizers", "We can get tokens from a string") {
    /* This is a string that we'd like to use for testing the tokenizer */
    const char* test_string = "what's new?How5is _ the ^# stuff";

    SECTION("strspn", "We can use the strspn tokenizer") {
        /* We need an instance of our actual tokenizer */
        Simhash::Strspn tokenizer;

        const char *current(NULL);
        const char *next(NULL);
        current = test_string;

        int64_t expected[] = {
            4, // 'what'
            1, // 's'
            3, // 'new'
            3, // 'How'
            2, // 'is'
            0, // '_'
            0, // ' '
            3, // 'the'
            0, // '^'
            0, // '#'
            0, // ' '
            5, // 'stuff'
        };

        for (size_t i = 0; i < 12; ++i) {
            next = tokenizer(current);
            REQUIRE((next - current) == expected[i]);
            current = next + 1;
        }

        /* Now we should be at the end */
        next = tokenizer(current);
        REQUIRE(next == NULL);
    }
}

TEST_CASE("simhash", "We can perform simhash") {
    std::string jabberwocky = "Twas brillig, and the slithy toves"
        "  Did gyre and gimble in the wabe:"
        "All mimsy were the borogoves,"
        "  And the mome raths outgrabe."
        "Beware the Jabberwock, my son!"
        "  The jaws that bite, the claws that catch!"
        "Beware the Jubjub bird, and shun"
        "  The frumious Bandersnatch!"
        "He took his vorpal sword in hand:"
        "  Long time the manxome foe he sought --"
        "So rested he by the Tumtum tree,"
        "  And stood awhile in thought."
        "And, as in uffish thought he stood,"
        "  The Jabberwock, with eyes of flame,"
        "Came whiffling through the tulgey wood,"
        "  And burbled as it came!"
        "One, two! One, two! And through and through"
        "  The vorpal blade went snicker-snack!"
        "He left it dead, and with its head"
        "  He went galumphing back."
        "And, has thou slain the Jabberwock?"
        "  Come to my arms, my beamish boy!"
        "O frabjous day! Callooh! Callay!'"
        "  He chortled in his joy.";

    std::string pope = "There once was a man named Pope"
        "who loved an oscilloscope."
        "  and the cyclical trace"
        "  of their carnal embrace"
        "had a damned-near-infinite slope";

    SECTION("Near-duplicates", "Can find near-duplicates") {
        /* Let's instantiate a simhasher */
        Simhash::Simhash<> hasher;
        uint64_t a = hasher(jabberwocky.c_str(), jabberwocky.length());
        /* Update jabberwocky to include who wrote it */
        jabberwocky += " - Lewis Carroll in 'Alice In Wonderland'";
        uint64_t b = hasher(jabberwocky.c_str(), jabberwocky.length());

        /* Now, make sure that the number of bits by which they differ is 
         * small */
        a = a ^ b;
        size_t count = 0;
        while (a) {
            a = a & (a - 1); ++count;
        }
        REQUIRE(count != 0);
        REQUIRE(count <= 3);

        /* Now, let's make sure that we can verify that two strings are 
         * nothing alike */
        a = hasher(pope.c_str(), pope.length());
        a = a ^ b;
        count = 0;
        while (a) {
            a = a & (a - 1); ++count;
        }
        REQUIRE(count > 5);
    }
}
