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

#include <vector>

#include "memory/atom.h"
#include "memory/atom_space.h"

namespace lizmem {

/*
 * MemorySubsystem
 *
 * Abstract base class for each of the six memory subsystems.
 * Each subsystem owns a partition of the shared AtomSpace and implements
 * domain-specific consolidation logic.
 */
class MemorySubsystem {
public:
	explicit MemorySubsystem(MemorySubsystemTag tag, AtomSpace &space)
	    : space_(space), tag_(tag) {}

	virtual ~MemorySubsystem() = default;

	MemorySubsystemTag tag() const noexcept { return tag_; }
	const char *name() const noexcept { return subsystemName(tag_); }

	/// Write (consolidate) @p atoms into the subsystem's AtomSpace partition.
	/// Implementations may filter, merge, or decay existing atoms.
	virtual void consolidate(const std::vector<MemoryAtom> &atoms) = 0;

	/// Return all atoms currently held by this subsystem.
	std::vector<MemoryAtom> atoms() const {
		return space_.bySubsystem(tag_);
	}

protected:
	AtomSpace &space_;

	/// Default filter: upsert every atom that belongs to this subsystem.
	void consolidateFiltered(const std::vector<MemoryAtom> &atoms) {
		for (const auto &a : atoms) {
			if (a.subsystem == tag_) {
				space_.upsert(a);
			}
		}
	}

private:
	MemorySubsystemTag tag_;
};

// ---------------------------------------------------------------------------
// Concrete subsystem implementations
// ---------------------------------------------------------------------------

/// Episodic memory: stores time-stamped episodes.
class EpisodicSubsystem : public MemorySubsystem {
public:
	explicit EpisodicSubsystem(AtomSpace &space)
	    : MemorySubsystem(MemorySubsystemTag::kEpisodic, space) {}

	void consolidate(const std::vector<MemoryAtom> &atoms) override {
		consolidateFiltered(atoms);
	}
};

/// Semantic memory: stores concepts and relationships.
class SemanticSubsystem : public MemorySubsystem {
public:
	explicit SemanticSubsystem(AtomSpace &space)
	    : MemorySubsystem(MemorySubsystemTag::kSemantic, space) {}

	void consolidate(const std::vector<MemoryAtom> &atoms) override {
		consolidateFiltered(atoms);
	}
};

/// Procedural memory: stores rules and procedures.
class ProceduralSubsystem : public MemorySubsystem {
public:
	explicit ProceduralSubsystem(AtomSpace &space)
	    : MemorySubsystem(MemorySubsystemTag::kProcedural, space) {}

	void consolidate(const std::vector<MemoryAtom> &atoms) override {
		consolidateFiltered(atoms);
	}
};

/// Working memory: holds the current cognitive state (short-term).
class WorkingSubsystem : public MemorySubsystem {
public:
	explicit WorkingSubsystem(AtomSpace &space)
	    : MemorySubsystem(MemorySubsystemTag::kWorking, space) {}

	void consolidate(const std::vector<MemoryAtom> &atoms) override {
		// Working memory replaces its contents on each consolidation cycle.
		for (uint64_t key : currentKeys_) {
			space_.erase(key);
		}
		currentKeys_.clear();

		for (const auto &a : atoms) {
			if (a.subsystem == MemorySubsystemTag::kWorking) {
				space_.upsert(a);
				currentKeys_.push_back(a.eternal_name);
			}
		}
	}

private:
	std::vector<uint64_t> currentKeys_;
};

/// Declarative memory: stores facts and propositions.
class DeclarativeSubsystem : public MemorySubsystem {
public:
	explicit DeclarativeSubsystem(AtomSpace &space)
	    : MemorySubsystem(MemorySubsystemTag::kDeclarative, space) {}

	void consolidate(const std::vector<MemoryAtom> &atoms) override {
		consolidateFiltered(atoms);
	}
};

/// Participatory memory: social and relational memory.
class ParticipatorySubsystem : public MemorySubsystem {
public:
	explicit ParticipatorySubsystem(AtomSpace &space)
	    : MemorySubsystem(MemorySubsystemTag::kParticipatory, space) {}

	void consolidate(const std::vector<MemoryAtom> &atoms) override {
		consolidateFiltered(atoms);
	}
};

}  // namespace lizmem
