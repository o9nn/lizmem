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

#include <array>
#include <cstdint>
#include <memory>

#include "memory/atom_space.h"
#include "memory/memory_subsystem.h"

namespace lizmem {

/*
 * MemorySchema
 *
 * The six-subsystem memory architecture.  One shared AtomSpace holds all
 * atoms; each MemorySubsystem manages its own typed partition.
 *
 * Matula prime indices assigned to each subsystem (following the echo-master
 * report convention):
 *
 *   Episodic      — p(1)  = 2
 *   Semantic      — p(2)  = 3
 *   Procedural    — p(3)  = 5
 *   Working       — p(4)  = 7
 *   Declarative   — p(5)  = 11
 *   Participatory — p(6)  = 13
 *
 * The product of all six primes (2*3*5*7*11*13 = 30030) is the Matula number
 * of the schema root — the node that has all six subsystems as children.
 */

static constexpr uint64_t kSchemaRootMatulaNumber = 2ULL * 3 * 5 * 7 * 11 * 13;

class MemorySchema {
public:
	MemorySchema()
	    : space_(),
	      episodic_(std::make_unique<EpisodicSubsystem>(space_)),
	      semantic_(std::make_unique<SemanticSubsystem>(space_)),
	      procedural_(std::make_unique<ProceduralSubsystem>(space_)),
	      working_(std::make_unique<WorkingSubsystem>(space_)),
	      declarative_(std::make_unique<DeclarativeSubsystem>(space_)),
	      participatory_(std::make_unique<ParticipatorySubsystem>(space_)) {}

	// Non-copyable; moving is fine.
	MemorySchema(const MemorySchema &) = delete;
	MemorySchema &operator=(const MemorySchema &) = delete;
	MemorySchema(MemorySchema &&) = default;
	MemorySchema &operator=(MemorySchema &&) = default;

	// Accessors for each subsystem.
	EpisodicSubsystem      &episodic()      { return *episodic_; }
	SemanticSubsystem      &semantic()      { return *semantic_; }
	ProceduralSubsystem    &procedural()    { return *procedural_; }
	WorkingSubsystem       &working()       { return *working_; }
	DeclarativeSubsystem   &declarative()   { return *declarative_; }
	ParticipatorySubsystem &participatory() { return *participatory_; }

	const EpisodicSubsystem      &episodic()      const { return *episodic_; }
	const SemanticSubsystem      &semantic()      const { return *semantic_; }
	const ProceduralSubsystem    &procedural()    const { return *procedural_; }
	const WorkingSubsystem       &working()       const { return *working_; }
	const DeclarativeSubsystem   &declarative()   const { return *declarative_; }
	const ParticipatorySubsystem &participatory() const { return *participatory_; }

	/// Direct access to the shared atom space.
	AtomSpace &atomSpace() noexcept { return space_; }
	const AtomSpace &atomSpace() const noexcept { return space_; }

	/// The Matula number of the schema root (product of all six subsystem
	/// primes, encoding the tree whose root has six leaf children).
	static uint64_t rootMatulaNumber() noexcept {
		return kSchemaRootMatulaNumber;
	}

private:
	AtomSpace space_;
	std::unique_ptr<EpisodicSubsystem>      episodic_;
	std::unique_ptr<SemanticSubsystem>      semantic_;
	std::unique_ptr<ProceduralSubsystem>    procedural_;
	std::unique_ptr<WorkingSubsystem>       working_;
	std::unique_ptr<DeclarativeSubsystem>   declarative_;
	std::unique_ptr<ParticipatorySubsystem> participatory_;
};

}  // namespace lizmem
