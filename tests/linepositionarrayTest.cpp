#include "gmock/gmock.h"

#include "log.h"

#include "data/linepositionarray.h"

using namespace std;
using namespace testing;

class LinePositionArraySmall: public testing::Test {
  public:
    LinePositionArray line_array;

    LinePositionArraySmall() {
        line_array.append( 4 );
        line_array.append( 8 );
        line_array.append( 10 );
        // A longer (>128) line
        line_array.append( 345 );
        // An even longer (>16384) line
        line_array.append( 20000 );
        // And a short one again
        line_array.append( 20020 );
    }
};

TEST_F( LinePositionArraySmall, HasACorrectSize ) {
    ASSERT_THAT( line_array.size(), Eq( 6 ) );
}

TEST_F( LinePositionArraySmall, RememberAddedLines ) {
    ASSERT_THAT( line_array[0], Eq( 4 ) );
    ASSERT_THAT( line_array[1], Eq( 8 ) );
    ASSERT_THAT( line_array[2], Eq( 10 ) );
    ASSERT_THAT( line_array[3], Eq( 345 ) );
    ASSERT_THAT( line_array[4], Eq( 20000 ) );
    ASSERT_THAT( line_array[5], Eq( 20020 ) );

    // This one again to eliminate caching effects
    ASSERT_THAT( line_array[3], Eq( 345 ) );
}

TEST_F( LinePositionArraySmall, FakeLFisNotKeptWhenAddingAfterIt ) {
    line_array.setFakeFinalLF();
    ASSERT_THAT( line_array[5], Eq( 20020 ) );
    line_array.append( 20030 );
    ASSERT_THAT( line_array[5], Eq( 20030 ) );
}

class LinePositionArrayConcatOperation: public LinePositionArraySmall {
  public:
    FastLinePositionArray other_array;

    LinePositionArrayConcatOperation() {
        other_array.append( 150000 );
        other_array.append( 150023 );
    }
};

TEST_F( LinePositionArrayConcatOperation, SimpleConcat ) {
    line_array.append_list( other_array );

    ASSERT_THAT( line_array.size(), Eq( 8 ) );

    ASSERT_THAT( line_array[0], Eq( 4 ) );
    ASSERT_THAT( line_array[1], Eq( 8 ) );

    ASSERT_THAT( line_array[5], Eq( 20020 ) );
    ASSERT_THAT( line_array[6], Eq( 150000 ) );
    ASSERT_THAT( line_array[7], Eq( 150023 ) );
}

TEST_F( LinePositionArrayConcatOperation, DoesNotKeepFakeLf ) {
    line_array.setFakeFinalLF();
    ASSERT_THAT( line_array[5], Eq( 20020 ) );

    line_array.append_list( other_array );
    ASSERT_THAT( line_array[5], Eq( 150000 ) );
    ASSERT_THAT( line_array.size(), Eq( 7 ) );
}

class LinePositionArrayLong: public testing::Test {
  public:
    LinePositionArray line_array;

    LinePositionArrayLong() {
        // Add 255 lines (of various sizes
        for ( int i = 0; i < 255; i++ )
            line_array.append( i * 4 );
        // Line no 256
        line_array.append( 255 * 4 );
    }
};

TEST_F( LinePositionArrayLong, LineNo128HasRightValue ) {
    // Add line no 257
    line_array.append( 255 * 4 + 10 );
    ASSERT_THAT( line_array[256], Eq( 255 * 4 + 10 ) );
}

TEST_F( LinePositionArrayLong, FakeLFisNotKeptWhenAddingAfterIt ) {
    for ( uint64_t i = 0; i < 1000; ++i ) {
        uint64_t pos = ( 257LL * 4 ) + i*35LL;
        line_array.append( pos );
        line_array.setFakeFinalLF();
        ASSERT_THAT( line_array[256 + i], Eq( pos ) );
        line_array.append( pos + 21LL );
        ASSERT_THAT( line_array[256 + i], Eq( pos + 21LL ) );
    }
}


class LinePositionArrayBig: public testing::Test {
  public:
    LinePositionArray line_array;

    LinePositionArrayBig() {
        line_array.append( 4 );
        line_array.append( 8 );
        // A very big line
        line_array.append( UINT32_MAX - 10 );
        line_array.append( (uint64_t) UINT32_MAX + 10LL );
        line_array.append( (uint64_t) UINT32_MAX + 30LL );
        line_array.append( (uint64_t) 2*UINT32_MAX );
        line_array.append( (uint64_t) 2*UINT32_MAX + 10LL );
        line_array.append( (uint64_t) 2*UINT32_MAX + 1000LL );
        line_array.append( (uint64_t) 3*UINT32_MAX );
    }
};

TEST_F( LinePositionArrayBig, IsTheRightSize ) {
    ASSERT_THAT( line_array.size(), 9 );
}

TEST_F( LinePositionArrayBig, HasRightData ) {
    ASSERT_THAT( line_array[0], Eq( 4 ) );
    ASSERT_THAT( line_array[1], Eq( 8 ) );
    ASSERT_THAT( line_array[2], Eq( UINT32_MAX - 10 ) );
    ASSERT_THAT( line_array[3], Eq( (uint64_t) UINT32_MAX + 10LL ) );
    ASSERT_THAT( line_array[4], Eq( (uint64_t) UINT32_MAX + 30LL ) );
    ASSERT_THAT( line_array[5], Eq( (uint64_t) 2LL*UINT32_MAX ) );
    ASSERT_THAT( line_array[6], Eq( (uint64_t) 2LL*UINT32_MAX + 10LL ) );
    ASSERT_THAT( line_array[7], Eq( (uint64_t) 2LL*UINT32_MAX + 1000LL ) );
    ASSERT_THAT( line_array[8], Eq( (uint64_t) 3LL*UINT32_MAX ) );
}

TEST_F( LinePositionArrayBig, FakeLFisNotKeptWhenAddingAfterIt ) {
    for ( uint64_t i = 0; i < 1000; ++i ) {
        uint64_t pos = 3LL*UINT32_MAX + 524LL + i*35LL;
        line_array.append( pos );
        line_array.setFakeFinalLF();
        ASSERT_THAT( line_array[9 + i], Eq( pos ) );
        line_array.append( pos + 21LL );
        ASSERT_THAT( line_array[9 + i], Eq( pos + 21LL ) );
    }
}


class LinePositionArrayBigConcat: public testing::Test {
  public:
    LinePositionArray line_array;
    FastLinePositionArray other_array;

    LinePositionArrayBigConcat() {
        line_array.append( 4 );
        line_array.append( 8 );

        other_array.append( UINT32_MAX + 10 );
        other_array.append( UINT32_MAX + 30 );
    }
};

TEST_F( LinePositionArrayBigConcat, SimpleBigConcat ) {
    line_array.append_list( other_array );

    ASSERT_THAT( line_array.size(), Eq( 4 ) );

    ASSERT_THAT( line_array[0], Eq( 4 ) );
    ASSERT_THAT( line_array[1], Eq( 8 ) );
    ASSERT_THAT( line_array[2], Eq( UINT32_MAX + 10 ) );
    ASSERT_THAT( line_array[3], Eq( UINT32_MAX + 30 ) );
}
