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

#include <chrono>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "memory/matula_prime.h"

namespace lizmem {

/*
 * MemoryAtom
 *
 * The fundamental unit of the lizmem memory system.  Each atom has:
 *  - an "eternal name" expressed as a Matula prime (or compound Matula number)
 *  - a subsystem tag identifying which of the six memory subsystems owns it
 *  - a salience score in [0, 1] reflecting activation strength
 *  - a payload that varies by subsystem type
 *  - a creation timestamp for episodic ordering
 */

/// Six memory subsystems following the regima-cognitive-ai architecture.
enum class MemorySubsystemTag : uint8_t {
	kEpisodic      = 0,
	kSemantic      = 1,
	kProcedural    = 2,
	kWorking       = 3,
	kDeclarative   = 4,
	kParticipatory = 5,
};

/// Human-readable name for a subsystem tag.
inline const char *subsystemName(MemorySubsystemTag tag) {
	switch (tag) {
	case MemorySubsystemTag::kEpisodic:      return "Episodic";
	case MemorySubsystemTag::kSemantic:      return "Semantic";
	case MemorySubsystemTag::kProcedural:    return "Procedural";
	case MemorySubsystemTag::kWorking:       return "Working";
	case MemorySubsystemTag::kDeclarative:   return "Declarative";
	case MemorySubsystemTag::kParticipatory: return "Participatory";
	}
	return "Unknown";
}

/// Payload variants for each subsystem type.
struct EpisodicPayload {
	std::string              episode_id;
	std::string              description;
	std::chrono::system_clock::time_point timestamp;
};

struct SemanticPayload {
	std::string concept_name;
	std::vector<std::string> relations;
};

struct ProceduralPayload {
	std::string rule_name;
	std::string rule_body;
};

struct WorkingPayload {
	std::string key;
	std::string value;
};

struct DeclarativePayload {
	std::string predicate;
	std::vector<std::string> arguments;
};

struct ParticipatorPayload {
	std::string participant_id;
	std::string relation_type;
	std::string target_id;
};

using AtomPayload = std::variant<
	EpisodicPayload,
	SemanticPayload,
	ProceduralPayload,
	WorkingPayload,
	DeclarativePayload,
	ParticipatorPayload>;

/// A single memory atom.
struct MemoryAtom {
	/// Eternal name: a Matula prime (for leaf atoms) or a product of Matula
	/// primes (for compound atoms formed by matulaNumber()).
	uint64_t           eternal_name;

	MemorySubsystemTag subsystem;
	float              salience;  ///< Activation strength in [0, 1]
	AtomPayload        payload;

	std::chrono::system_clock::time_point created_at;

	/// Convenience constructor.
	MemoryAtom(uint64_t name, MemorySubsystemTag sub, float sal,
	           AtomPayload pl,
	           std::chrono::system_clock::time_point ts =
	               std::chrono::system_clock::now())
	    : eternal_name(name),
	      subsystem(sub),
	      salience(sal),
	      payload(std::move(pl)),
	      created_at(ts) {}
};

}  // namespace lizmem
