/*
  Copyright 2018-2020, Barcelona Supercomputing Center (BSC), Spain
  Copyright 2015-2020, Johannes Gutenberg Universitaet Mainz, Germany

  This software was partially supported by the
  EC H2020 funded project NEXTGenIO (Project ID: 671951, www.nextgenio.eu).

  This software was partially supported by the
  ADA-FS project under the SPPEXA project funded by the DFG.

  SPDX-License-Identifier: MIT
*/

#include <catch2/catch.hpp>
#include <global/chunk_calc_util.hpp>
#include <fmt/format.h>

using namespace gkfs::util;
constexpr auto test_repetitions = 250u;

namespace {

/**
 * Check if @n is a power of two by (rather inefficiently)
 * performing successive divisions by 2 in an attempt to reach 1.
 *
 * @param n the number to check
 * @returns true if @n is a power of 2, false otherwise
 */
bool
check_power_of_2(uint64_t n) {
    if(n == 0) {
        return false;
    }

    while(n != 1) {
        if(n % 2 != 0) {
            return false;
        }
        n /= 2;
    }

    return true;
}

} // namespace


SCENARIO(" powers of 2 can be correctly detected ",
         "[utils][numeric][is_power_of_2]") {

    GIVEN(" a positive number ") {

        WHEN(" n is 0 ") {

            const uint64_t n = 0;

            THEN(" is_power_of_2(n) returns false ") {
                REQUIRE(is_power_of_2(n) == false);
            }
        }

        WHEN(" n is 1 ") {

            const uint64_t n = 1;

            THEN(" is_power_of_2(n) returns true ") {
                REQUIRE(is_power_of_2(n) == true);
            }
        }

        WHEN(" n is neither 0 nor 1 ") {

            AND_WHEN(" n is a power of 2 ") {

                const std::size_t n = GENERATE(
                        filter([](uint64_t m) { return check_power_of_2(m); },
                               range(0, 10000)));

                THEN(" is_power_of_2(n) returns false ") {
                    REQUIRE(is_power_of_2(n) == true);
                }
            }

            AND_WHEN(" n is not a power of 2 ") {

                const std::size_t n = GENERATE(
                        filter([](uint64_t m) { return !check_power_of_2(m); },
                               range(0, 10000)));

                THEN(" is_power_of_2(n) returns false ") {
                    REQUIRE(is_power_of_2(n) == false);
                }
            }
        }
    }
}

SCENARIO(" divisibility by powers of 2 can be correctly detected ",
         "[utils][numeric][is_divisible]") {

    GIVEN(" a number and a block_size ") {

        const uint64_t n = GENERATE(range(0, 1000), range(20000, 23000),
                                    std::numeric_limits<uint64_t>::max());
        const std::size_t block_size =
                GENERATE(filter([](uint64_t bs) { return is_power_of_2(bs); },
                                range(0, 10000)));

        CAPTURE(n, block_size);

        bool expected = n % block_size == 0;
        REQUIRE(is_divisible(n, block_size) == expected);
    }
}

SCENARIO(" offsets can be left-aligned to block size boundaries ",
         "[utils][numeric][chnk_lalign]") {

    GIVEN(" a block size ") {

        const std::size_t block_size =
                GENERATE(filter([](uint64_t bs) { return is_power_of_2(bs); },
                                range(0, 100000)));

        WHEN(" offset is 0 ") {

            const uint64_t offset = 0;

            CAPTURE(offset, block_size);

            THEN(" the left-aligned offset is 0 ") {
                const uint64_t aligned_offset = chnk_lalign(offset, block_size);
                REQUIRE(aligned_offset == 0);
            }
        }

        WHEN(" offset is smaller than block size ") {

            const uint64_t offset = GENERATE_COPY(
                    take(test_reps, random(std::size_t{0}, block_size - 1)));

            CAPTURE(offset, block_size);

            THEN(" the left-aligned offset is 0 ") {
                const uint64_t aligned_offset = chnk_lalign(offset, block_size);
                REQUIRE(aligned_offset == 0);
            }
        }

        WHEN(" offset is larger than block size ") {

            const uint64_t offset = GENERATE_COPY(
                    take(test_reps, random(block_size, block_size * 31)));

            CAPTURE(offset, block_size);

            THEN(" the left-aligned offset is the left boundary of the "
                 "containing block ") {
                const uint64_t aligned_offset = chnk_lalign(offset, block_size);
                const uint64_t exp_offset =
                        static_cast<uint64_t>(offset / block_size) * block_size;
                REQUIRE(aligned_offset == exp_offset);
            }
        }
    }
}

SCENARIO(" offsets can be right-aligned to block size boundaries ",
         "[utils][numeric][chnk_ralign]") {

    GIVEN(" a block size ") {

        const std::size_t block_size = GENERATE(filter(
                [](uint64_t bs) { return is_power_of_2(bs); }, range(0, 100000)));

        WHEN(" offset is 0 ") {

            const uint64_t offset = 0;

            CAPTURE(offset, block_size);

            THEN(" the right-aligned offset is block_size ") {
                const uint64_t aligned_offset = chnk_ralign(offset, block_size);
                const uint64_t expected_offset = block_size;
                REQUIRE(aligned_offset == expected_offset);
            }
        }

        WHEN(" offset is smaller than block_size ") {

            const uint64_t offset = GENERATE_COPY(
                    take(test_reps, random(std::size_t{0}, block_size - 1)));

            CAPTURE(offset, block_size);

            THEN(" the right-aligned offset is 0 ") {
                const uint64_t aligned_offset = chnk_ralign(offset, block_size);
                const uint64_t expected_offset = block_size;
                REQUIRE(aligned_offset == expected_offset);
            }
        }

        WHEN(" offset is larger than block_size ") {

            const uint64_t offset = GENERATE_COPY(
                    take(test_reps, random(block_size, block_size * 31)));

            CAPTURE(offset, block_size);

            THEN(" the right-aligned offset is the right boundary of the "
                 "containing block ") {
                const uint64_t aligned_offset = chnk_ralign(offset, block_size);
                const uint64_t expected_offset =
                        static_cast<uint64_t>(offset / block_size + 1) *
                        block_size;
                REQUIRE(aligned_offset == expected_offset);
            }
        }
    }
}

SCENARIO(" overrun distance can be computed correctly ",
         "[utils][numeric][chnk_lpad]") {

    GIVEN(" a block size ") {

        const std::size_t block_size = GENERATE(filter(
                [](uint64_t bs) { return is_power_of_2(bs); }, range(0, 100000)));

        WHEN(" offset is smaller than block_size ") {

            AND_WHEN(" offset equals 0 ") {

                const uint64_t offset = 0;

                CAPTURE(offset, block_size);

                THEN(" the computed overrun distance equals 0 ") {
                    const uint64_t overrun = chnk_lpad(offset, block_size);
                    const uint64_t expected_overrun = 0;
                    REQUIRE(overrun == expected_overrun);
                }
            }

            AND_WHEN(" 0 < offset < block_size ") {

                const uint64_t offset = GENERATE_COPY(take(
                        test_reps, random(std::size_t{0}, block_size - 1)));

                CAPTURE(offset, block_size);

                THEN(" the computed overrun distance equals offset ") {
                    const uint64_t overrun = chnk_lpad(offset, block_size);
                    const uint64_t expected_overrun = offset;
                    REQUIRE(overrun == expected_overrun);
                }
            }

            AND_WHEN(" offset equals block_size - 1 ") {

                const uint64_t offset = block_size - 1;

                CAPTURE(offset, block_size);

                THEN(" the computed overrun distance equals block_size - 1 ") {
                    const uint64_t overrun = chnk_lpad(offset, block_size);
                    const uint64_t expected_overrun = block_size - 1;
                    REQUIRE(overrun == expected_overrun);
                }
            }
        }

        WHEN(" offset equals block_size ") {

            const uint64_t offset = block_size;

            CAPTURE(offset, block_size);

            THEN(" the computed overrun distance equals 0 ") {
                const uint64_t overrun = chnk_lpad(offset, block_size);
                const uint64_t expected_overrun = 0;
                REQUIRE(overrun == expected_overrun);
            }
        }

        WHEN(" offset is larger than block_size ") {

            const uint64_t offset = GENERATE_COPY(
                    take(test_reps, random(block_size, block_size * 31)));

            CAPTURE(offset, block_size);

            THEN(" the computed overrun distance equals the difference between "
                 "offset and its closest block's left boundary ") {
                const uint64_t overrun = chnk_lpad(offset, block_size);
                const uint64_t expected_overrun =
                        offset -
                        static_cast<uint64_t>(offset / block_size) * block_size;
                REQUIRE(overrun == expected_overrun);
            }
        }
    }
}

SCENARIO(" underrun distance can be computed correctly ",
         "[utils][numeric][chnk_rpad]") {

    GIVEN(" a block size ") {

        const std::size_t block_size = GENERATE(filter(
                [](uint64_t bs) { return is_power_of_2(bs); }, range(0, 100000)));

        WHEN(" offset is smaller than block_size ") {

            AND_WHEN(" offset equals 0 ") {

                const uint64_t offset = 0;

                CAPTURE(offset, block_size);

                THEN(" the computed underrun distance equals block_size ") {
                    const uint64_t underrun = chnk_rpad(offset, block_size);
                    const uint64_t expected_underrun = block_size;
                    REQUIRE(underrun == expected_underrun);
                }
            }

            AND_WHEN(" 0 < offset < block_size ") {

                const uint64_t offset = GENERATE_COPY(take(
                        test_reps, random(std::size_t{0}, block_size - 1)));

                CAPTURE(offset, block_size);

                THEN(" the computed underrun distance equals offset ") {
                    const uint64_t underrun = chnk_rpad(offset, block_size);
                    const uint64_t expected_underrun = block_size - offset;
                    REQUIRE(underrun == expected_underrun);
                }
            }

            AND_WHEN(" offset equals block_size - 1 ") {

                const uint64_t offset = block_size - 1;

                CAPTURE(offset, block_size);

                THEN(" the computed underrun distance equals block_size - 1 ") {
                    const uint64_t underrun = chnk_rpad(offset, block_size);
                    const uint64_t expected_underrun = block_size - offset;
                    REQUIRE(underrun == expected_underrun);
                }
            }
        }

        WHEN(" offset equals block_size ") {

            const uint64_t offset = block_size;

            CAPTURE(offset, block_size);

            THEN(" the computed underrun distance equals block_size ") {
                const uint64_t underrun = chnk_rpad(offset, block_size);
                const uint64_t expected_underrun = block_size;
                REQUIRE(underrun == expected_underrun);
            }
        }

        WHEN(" offset is larger than block_size ") {

            const uint64_t offset = GENERATE_COPY(
                    take(test_reps, random(block_size, block_size * 31)));

            CAPTURE(offset, block_size);

            THEN(" the computed underrun distance equals the difference between "
                 "offset and its closest block's right boundary ") {
                const uint64_t underrun = chnk_rpad(offset, block_size);
                const uint64_t expected_underrun =
                        static_cast<uint64_t>(offset / block_size + 1) *
                                block_size -
                        offset;
                REQUIRE(underrun == expected_underrun);
            }
        }
    }
}

SCENARIO(" chunk IDs can be computed correctly ",
         "[utils][numeric][chnk_id_for_offset]") {

    GIVEN(" an offset and a block size ") {

        const std::size_t block_size = GENERATE(filter(
                [](uint64_t bs) { return is_power_of_2(bs); }, range(0, 100000)));

        WHEN(" offset is smaller than block_size ") {

            AND_WHEN(" offset equals 0 ") {

                const uint64_t offset = 0;

                CAPTURE(offset, block_size);

                THEN(" the computed chunk ID equals 0 ") {
                    const uint64_t id = chnk_id_for_offset(offset, block_size);
                    const uint64_t expected_id = 0;
                    REQUIRE(id == expected_id);
                }
            }

            AND_WHEN(" 0 < offset < block_size ") {

                const uint64_t offset = GENERATE_COPY(take(
                        test_reps, random(std::size_t{0}, block_size - 1)));

                CAPTURE(offset, block_size);

                THEN(" the computed chunk ID equals 0 ") {
                    const uint64_t id = chnk_id_for_offset(offset, block_size);
                    const uint64_t expected_id = 0;
                    REQUIRE(id == expected_id);
                }
            }

            AND_WHEN(" offset equals block_size - 1 ") {

                const uint64_t offset = block_size - 1;

                CAPTURE(offset, block_size);

                THEN(" the computed chunk ID equals 0 ") {
                    const uint64_t id = chnk_id_for_offset(offset, block_size);
                    const uint64_t expected_id = 0;
                    REQUIRE(id == expected_id);
                }
            }
        }

        WHEN(" offset equals block_size ") {

            const uint64_t offset = block_size;

            CAPTURE(offset, block_size);

            THEN(" the computed chunk ID equals 1 ") {
                const uint64_t id = chnk_id_for_offset(offset, block_size);
                const uint64_t expected_id = 1;
                REQUIRE(id == expected_id);
            }
        }

        WHEN(" offset is larger than block_size ") {

            AND_WHEN(" block_size < offset < 2^63 - 1 ") {

                const uint64_t offset = GENERATE_COPY(take(
                        test_reps,
                        random(block_size,
                               std::numeric_limits<uint64_t>::max() / 2 - 1)));

                CAPTURE(offset, block_size);

                THEN(" the computed chunk ID is equal to dividing the offset by "
                     "the block_size ") {

                    const uint64_t id = chnk_id_for_offset(offset, block_size);
                    const uint64_t expected_id = offset / block_size;
                    REQUIRE(id == expected_id);
                }
            }

            // The following test specifically exercises issue #137
            AND_WHEN(" offset == 2^63 ") {

                const uint64_t offset =
                        std::numeric_limits<uint64_t>::max() / 2 + 1;

                CAPTURE(offset, block_size);

                THEN(" the computed chunk ID is equal to dividing the offset by "
                     "the block_size ") {

                    const uint64_t id = chnk_id_for_offset(offset, block_size);
                    const uint64_t expected_id = offset / block_size;
                    REQUIRE(id == expected_id);
                }
            }

            // The following test specifically exercises issue #137
            AND_WHEN(" offset == 2^64 - 1") {

                const uint64_t offset = std::numeric_limits<uint64_t>::max();

                CAPTURE(offset, block_size);

                THEN(" the computed chunk ID is equal to dividing the offset by "
                     "the block_size ") {

                    const uint64_t id = chnk_id_for_offset(offset, block_size);
                    const uint64_t expected_id = offset / block_size;
                    REQUIRE(id == expected_id);
                }
            }
        }
    }
}
