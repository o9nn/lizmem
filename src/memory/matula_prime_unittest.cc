/*
   Copyright 2013-2014 EditShare, 2013-2015 Skytechnology sp. z o.o.

   This file is part of LizardFS.

   LizardFS is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, version 3.

   LizardFS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with LizardFS. If not, see <http://www.gnu.org/licenses/>.
 */

#include "common/platform.h"
#include "memory/matula_prime.h"

#include <gtest/gtest.h>

using namespace lizmem;

TEST(MatulaPrimeTests, FirstTenPrimes) {
	EXPECT_EQ(nthPrime(1),  2u);
	EXPECT_EQ(nthPrime(2),  3u);
	EXPECT_EQ(nthPrime(3),  5u);
	EXPECT_EQ(nthPrime(4),  7u);
	EXPECT_EQ(nthPrime(5), 11u);
	EXPECT_EQ(nthPrime(6), 13u);
	EXPECT_EQ(nthPrime(7), 17u);
	EXPECT_EQ(nthPrime(8), 19u);
	EXPECT_EQ(nthPrime(9), 23u);
	EXPECT_EQ(nthPrime(10), 29u);
}

TEST(MatulaPrimeTests, MaxCachedPrime) {
	// The 512th prime is 3671.
	EXPECT_EQ(nthPrime(kMaxPrimeIndex), 3671u);
}

TEST(MatulaPrimeTests, PrimeBeyondCache) {
	// The 600th prime is 4409.
	EXPECT_EQ(nthPrime(600), 4409u);
}

TEST(MatulaPrimeTests, InvalidIndexThrows) {
	EXPECT_THROW(nthPrime(0), std::out_of_range);
}

TEST(MatulaPrimeTests, LeafNodeIsOne) {
	EXPECT_EQ(matulaNumber({}), 1u);
}

TEST(MatulaPrimeTests, SingleChildLeaf) {
	// Tree with one leaf child (Matula number 1 => p(1) = 2).
	EXPECT_EQ(matulaNumber({1}), 2u);
}

TEST(MatulaPrimeTests, TwoLeafChildren) {
	// p(1)*p(1) = 2*2 = 4.
	EXPECT_EQ(matulaNumber({1, 1}), 4u);
}

TEST(MatulaPrimeTests, TreeWithTwoDifferentChildren) {
	// p(1)*p(2) = 2*3 = 6.
	EXPECT_EQ(matulaNumber({1, 2}), 6u);
}

TEST(MatulaPrimeTests, FactorLeafOne) {
	EXPECT_EQ(matulaFactor(1), std::vector<uint64_t>{});
}

TEST(MatulaPrimeTests, FactorSinglePrime) {
	// 2 = p(1) => one child with Matula number 1.
	EXPECT_EQ(matulaFactor(2), (std::vector<uint64_t>{1}));
}

TEST(MatulaPrimeTests, FactorCompositeMatulaNumber) {
	// 6 = 2*3 = p(1)*p(2).
	auto children = matulaFactor(6);
	ASSERT_EQ(children.size(), 2u);
	EXPECT_EQ(children[0], 1u);
	EXPECT_EQ(children[1], 2u);
}

TEST(MatulaPrimeTests, RoundTrip) {
	std::vector<uint64_t> children = {1, 2, 3};
	uint64_t m = matulaNumber(children);
	EXPECT_EQ(matulaFactor(m), children);
}

TEST(MatulaPrimeTests, IsMatulaPrime_Primes) {
	EXPECT_TRUE(isMatulaPrime(2));
	EXPECT_TRUE(isMatulaPrime(3));
	EXPECT_TRUE(isMatulaPrime(5));
	EXPECT_TRUE(isMatulaPrime(97));
}

TEST(MatulaPrimeTests, IsMatulaPrime_Composites) {
	EXPECT_FALSE(isMatulaPrime(1));
	EXPECT_FALSE(isMatulaPrime(4));
	EXPECT_FALSE(isMatulaPrime(6));
	EXPECT_FALSE(isMatulaPrime(30030));
}

TEST(MatulaPrimeTests, SchemaRootMatulaNumber) {
	// 2*3*5*7*11*13 = 30030
	EXPECT_EQ(2u * 3 * 5 * 7 * 11 * 13, 30030u);
}

TEST(MatulaPrimeTests, InvalidMatulaFactorThrows) {
	EXPECT_THROW(matulaFactor(0), std::invalid_argument);
}
