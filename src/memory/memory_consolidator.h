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

#include <atomic>
#include <cstdint>

#include "memory/global_workspace_broadcaster.h"
#include "memory/memory_schema.h"

namespace lizmem {

/*
 * MemoryConsolidator
 *
 * Subscribes to GlobalWorkspaceBroadcaster and consolidates incoming atoms
 * into the MemorySchema.  Each subsystem is only written when its salience
 * in the sync_event meets or exceeds the consolidation threshold.
 *
 * Design mirrors the echo-agent-loop re-entrancy guard:
 *  - Slow consolidation must not block the next perception cycle.
 *  - The broadcaster's in_progress guard ensures no nested calls stack up.
 *  - consolidation_count_ tracks successful consolidation cycles.
 *  - skipped_count_ tracks cycles where all subsystems were below threshold.
 */
class MemoryConsolidator {
public:
	/// @param schema      The MemorySchema to write into.
	/// @param broadcaster The GlobalWorkspaceBroadcaster to subscribe to.
	/// @param threshold   Minimum per-subsystem salience to trigger a write.
	MemoryConsolidator(MemorySchema &schema,
	                   GlobalWorkspaceBroadcaster &broadcaster,
	                   float threshold = 0.1f)
	    : schema_(schema),
	      threshold_(threshold),
	      consolidation_count_(0),
	      skipped_count_(0) {
		handle_ = broadcaster.subscribe(
		    [this](const SyncEvent &event) { onSyncEvent(event); });
	}

	/// Unsubscribing is not automatic on destruction to avoid lifetime
	/// issues with the broadcaster.  Call detach() explicitly if needed.
	void detach(GlobalWorkspaceBroadcaster &broadcaster) {
		broadcaster.unsubscribe(handle_);
	}

	/// Number of sync_events that triggered at least one subsystem write.
	uint64_t consolidationCount() const noexcept {
		return consolidation_count_.load(std::memory_order_relaxed);
	}

	/// Number of sync_events where every subsystem was below threshold.
	uint64_t skippedCount() const noexcept {
		return skipped_count_.load(std::memory_order_relaxed);
	}

	float threshold() const noexcept { return threshold_; }

private:
	void onSyncEvent(const SyncEvent &event) {
		bool any_written = false;

		auto write_if_salient = [&](MemorySubsystemTag tag,
		                            MemorySubsystem &subsystem) {
			float salience = event.saliences[static_cast<uint8_t>(tag)];
			if (salience >= threshold_) {
				subsystem.consolidate(event.atoms);
				any_written = true;
			}
		};

		write_if_salient(MemorySubsystemTag::kEpisodic,      schema_.episodic());
		write_if_salient(MemorySubsystemTag::kSemantic,      schema_.semantic());
		write_if_salient(MemorySubsystemTag::kProcedural,    schema_.procedural());
		write_if_salient(MemorySubsystemTag::kWorking,       schema_.working());
		write_if_salient(MemorySubsystemTag::kDeclarative,   schema_.declarative());
		write_if_salient(MemorySubsystemTag::kParticipatory, schema_.participatory());

		if (any_written) {
			consolidation_count_.fetch_add(1, std::memory_order_relaxed);
		} else {
			skipped_count_.fetch_add(1, std::memory_order_relaxed);
		}
	}

	MemorySchema               &schema_;
	float                       threshold_;
	std::atomic<uint64_t>       consolidation_count_;
	std::atomic<uint64_t>       skipped_count_;
	size_t                      handle_;
};

}  // namespace lizmem
