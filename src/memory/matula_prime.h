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

#pragma once

#include "common/platform.h"

#include <cstdint>
#include <vector>

/*
 * Matula-Goebel prime addressing for memory atoms.
 *
 * Every rooted tree has a unique positive integer (its Matula number).
 * The encoding is:
 *   - The single-node tree has Matula number 1.
 *   - A tree whose root has children T_1, ..., T_k has Matula number
 *       p(n_1) * p(n_2) * ... * p(n_k)
 *     where n_i is the Matula number of T_i and p(n) is the n-th prime.
 *
 * In the lizmem memory system each memory atom carries a Matula prime as its
 * "eternal name".  Atoms that compose into a compound record are encoded as a
 * prime-product (their Matula number) so structural similarity reduces to
 * GCD arithmetic on integers.
 */

namespace lizmem {

/// Return the n-th prime (1-indexed: prime(1)==2, prime(2)==3, …).
/// Results for n up to kMaxPrimeIndex are cached; larger n are computed on
/// demand via a simple trial-division sieve.
uint64_t nthPrime(uint32_t n);

/// Return the Matula number of the tree formed by attaching children whose
/// Matula numbers are given by @p child_matula_numbers.
/// An empty child list yields 1 (the leaf node).
uint64_t matulaNumber(const std::vector<uint64_t> &child_matula_numbers);

/// Factor a Matula number back into the Matula numbers of its child subtrees.
/// For example matulaFactor(6) == {1, 1} because 6 = 2*3 = p(1)*p(2).
/// Returns an empty vector for the leaf value 1.
std::vector<uint64_t> matulaFactor(uint64_t matula_number);

/// Return true when @p n is a Matula prime (i.e. a prime number), which
/// signals that the corresponding tree is a rooted path and the atom is
/// "irreducible" — it cannot be decomposed further.
bool isMatulaPrime(uint64_t n);

/// The largest pre-cached prime index (prime(kMaxPrimeIndex) is guaranteed
/// to be available without heap allocation).
static constexpr uint32_t kMaxPrimeIndex = 512u;

}  // namespace lizmem
