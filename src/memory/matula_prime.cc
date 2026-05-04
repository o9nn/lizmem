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

#include "memory/matula_prime.h"

#include <algorithm>
#include <cassert>
#include <stdexcept>

namespace lizmem {

namespace {

// ---------------------------------------------------------------------------
// Compile-time / startup prime table built by a segmented sieve.
// We cache the first kMaxPrimeIndex primes; further queries fall back to
// trial division.
// ---------------------------------------------------------------------------

std::vector<uint64_t> buildPrimeTable(uint32_t count) {
	std::vector<uint64_t> primes;
	primes.reserve(count);
	primes.push_back(2);

	for (uint64_t candidate = 3; primes.size() < count; candidate += 2) {
		bool is_prime = true;
		for (uint64_t p : primes) {
			if (p * p > candidate) {
				break;
			}
			if (candidate % p == 0) {
				is_prime = false;
				break;
			}
		}
		if (is_prime) {
			primes.push_back(candidate);
		}
	}
	return primes;
}

const std::vector<uint64_t> &primeTable() {
	static std::vector<uint64_t> table = buildPrimeTable(kMaxPrimeIndex);
	return table;
}

// Extend the cached table on demand, returning the n-th prime (1-indexed).
uint64_t slowNthPrime(uint32_t n) {
	// We already have at least kMaxPrimeIndex primes; extend until we reach n.
	static std::vector<uint64_t> extended = primeTable();

	while (extended.size() < static_cast<size_t>(n)) {
		uint64_t candidate = extended.back() + 2;
		while (true) {
			bool is_prime = true;
			for (uint64_t p : extended) {
				if (p * p > candidate) {
					break;
				}
				if (candidate % p == 0) {
					is_prime = false;
					break;
				}
			}
			if (is_prime) {
				extended.push_back(candidate);
				break;
			}
			candidate += 2;
		}
	}
	return extended[n - 1];
}

}  // namespace

uint64_t nthPrime(uint32_t n) {
	if (n == 0) {
		throw std::out_of_range("nthPrime: n must be >= 1");
	}
	if (n <= kMaxPrimeIndex) {
		return primeTable()[n - 1];
	}
	return slowNthPrime(n);
}

uint64_t matulaNumber(const std::vector<uint64_t> &child_matula_numbers) {
	if (child_matula_numbers.empty()) {
		return 1;  // leaf node
	}
	uint64_t result = 1;
	for (uint64_t child_m : child_matula_numbers) {
		if (child_m == 0) {
			throw std::invalid_argument("matulaNumber: child Matula number must be >= 1");
		}
		// p(child_m) is the child_m-th prime
		uint64_t p = nthPrime(static_cast<uint32_t>(child_m));
		if (result > UINT64_MAX / p) {
			throw std::overflow_error("matulaNumber: product would overflow uint64_t");
		}
		result *= p;
	}
	return result;
}

std::vector<uint64_t> matulaFactor(uint64_t matula_number) {
	if (matula_number == 0) {
		throw std::invalid_argument("matulaFactor: Matula number must be >= 1");
	}
	if (matula_number == 1) {
		return {};
	}

	std::vector<uint64_t> children;
	uint64_t remaining = matula_number;

	// Factor remaining into primes, then convert each prime p to its 1-based
	// index (which is the child Matula number).
	const auto &table = primeTable();
	uint32_t idx = 1;  // 1-based prime index

	for (uint64_t p : table) {
		if (p * p > remaining) {
			break;
		}
		while (remaining % p == 0) {
			children.push_back(idx);
			remaining /= p;
		}
		++idx;
	}

	if (remaining > 1) {
		// remaining itself is prime; find its 1-based index.
		// First check the cached table:
		uint32_t prime_idx = 0;
		auto it = std::find(table.begin(), table.end(), remaining);
		if (it != table.end()) {
			prime_idx = static_cast<uint32_t>(std::distance(table.begin(), it)) + 1;
		} else {
			// Linear search beyond cache (rare).
			prime_idx = kMaxPrimeIndex + 1;
			while (nthPrime(prime_idx) != remaining) {
				++prime_idx;
			}
		}
		children.push_back(prime_idx);
	}

	return children;
}

bool isMatulaPrime(uint64_t n) {
	if (n < 2) {
		return false;
	}
	for (uint64_t p : primeTable()) {
		if (p * p > n) {
			break;
		}
		if (n % p == 0) {
			return false;
		}
	}
	return true;
}

}  // namespace lizmem
