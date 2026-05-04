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

#include <algorithm>
#include <functional>
#include <optional>
#include <unordered_map>
#include <vector>

#include "memory/atom.h"

namespace lizmem {

/*
 * AtomSpace
 *
 * A typed container that maps Matula eternal names to MemoryAtom objects.
 * Atoms belonging to a specific subsystem can be efficiently enumerated via
 * the per-subsystem index.
 *
 * Thread safety: not internally synchronised — callers must serialise access
 * when used from multiple threads.
 */
class AtomSpace {
public:
	/// Insert or replace an atom.  The atom's eternal_name is used as the key.
	void upsert(MemoryAtom atom) {
		uint64_t key = atom.eternal_name;
		MemorySubsystemTag sub = atom.subsystem;

		auto existing = atoms_.find(key);
		if (existing != atoms_.end()) {
			// Remove the old entry from the subsystem index.
			removeFromIndex(sub, key);
		}
		atoms_.insert_or_assign(key, std::move(atom));
		subsystem_index_[static_cast<uint8_t>(sub)].push_back(key);
	}

	/// Remove an atom by its eternal name.  Returns true when an atom was
	/// erased, false when no atom with that name existed.
	bool erase(uint64_t eternal_name) {
		auto it = atoms_.find(eternal_name);
		if (it == atoms_.end()) {
			return false;
		}
		MemorySubsystemTag sub = it->second.subsystem;
		atoms_.erase(it);
		removeFromIndex(sub, eternal_name);
		return true;
	}

	/// Retrieve an atom by eternal name.
	std::optional<MemoryAtom> find(uint64_t eternal_name) const {
		auto it = atoms_.find(eternal_name);
		if (it == atoms_.end()) {
			return std::nullopt;
		}
		return it->second;
	}

	/// Return all atoms belonging to @p subsystem.
	std::vector<MemoryAtom> bySubsystem(MemorySubsystemTag subsystem) const {
		std::vector<MemoryAtom> result;
		const auto &index = subsystem_index_[static_cast<uint8_t>(subsystem)];
		result.reserve(index.size());
		for (uint64_t key : index) {
			auto it = atoms_.find(key);
			if (it != atoms_.end()) {
				result.push_back(it->second);
			}
		}
		return result;
	}

	/// Return all atoms with salience >= @p threshold, sorted descending.
	std::vector<MemoryAtom> bySalience(float threshold = 0.0f) const {
		std::vector<MemoryAtom> result;
		for (const auto &[key, atom] : atoms_) {
			if (atom.salience >= threshold) {
				result.push_back(atom);
			}
		}
		std::sort(result.begin(), result.end(),
		          [](const MemoryAtom &a, const MemoryAtom &b) {
			          return a.salience > b.salience;
		          });
		return result;
	}

	/// Total number of atoms in the space.
	std::size_t size() const noexcept {
		return atoms_.size();
	}

	bool empty() const noexcept {
		return atoms_.empty();
	}

	/// Remove all atoms.
	void clear() {
		atoms_.clear();
		for (auto &idx : subsystem_index_) {
			idx.clear();
		}
	}

private:
	void removeFromIndex(MemorySubsystemTag sub, uint64_t key) {
		auto &index = subsystem_index_[static_cast<uint8_t>(sub)];
		index.erase(std::remove(index.begin(), index.end(), key), index.end());
	}

	std::unordered_map<uint64_t, MemoryAtom> atoms_;
	// One vector per subsystem (indexed by MemorySubsystemTag cast to uint8_t).
	std::vector<uint64_t> subsystem_index_[6];
};

}  // namespace lizmem
