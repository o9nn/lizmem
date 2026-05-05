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
#include <functional>
#include <mutex>
#include <vector>

#include "memory/atom.h"

namespace lizmem {

/*
 * GlobalWorkspaceBroadcaster
 *
 * Central pub/sub hub for cognitive sync_events.  Every perception cycle that
 * completes produces a SyncEvent carrying per-subsystem salience scores and a
 * snapshot of the most salient atoms.  Subscribers (such as MemoryConsolidator)
 * are notified synchronously in the order they registered.
 *
 * Re-entrancy guard: if a broadcast is already in progress (e.g. a slow
 * subscriber triggers another event), the nested broadcast is dropped and an
 * overrun counter is incremented.  This mirrors the `tickInProgress` guard in
 * the echo-agent-loop optimization.
 */

/// Per-subsystem salience: index == static_cast<uint8_t>(MemorySubsystemTag).
using SubsystemSaliences = std::array<float, 6>;

/// A sync_event produced at the end of each cognitive perception cycle.
struct SyncEvent {
	/// Wall-clock sequence number; monotonically increasing.
	uint64_t sequence;

	/// Salience scores for each of the six memory subsystems.
	/// A subsystem with salience below the consolidator's threshold is skipped
	/// during this event.
	SubsystemSaliences saliences;

	/// Snapshot of the most-active atoms from the global workspace.
	std::vector<MemoryAtom> atoms;
};

using SyncEventCallback = std::function<void(const SyncEvent &)>;

class GlobalWorkspaceBroadcaster {
public:
	GlobalWorkspaceBroadcaster() : sequence_(0), in_progress_(false), overruns_(0) {}

	// Non-copyable.
	GlobalWorkspaceBroadcaster(const GlobalWorkspaceBroadcaster &) = delete;
	GlobalWorkspaceBroadcaster &operator=(const GlobalWorkspaceBroadcaster &) = delete;

	/// Register a subscriber callback.  Returns a handle (index) that can be
	/// used with unsubscribe().
	size_t subscribe(SyncEventCallback callback) {
		std::lock_guard<std::mutex> lock(mutex_);
		subscribers_.push_back(std::move(callback));
		return subscribers_.size() - 1;
	}

	/// Remove a subscriber by handle.  The slot is set to nullptr; later
	/// broadcasts will skip null entries.
	void unsubscribe(size_t handle) {
		std::lock_guard<std::mutex> lock(mutex_);
		if (handle < subscribers_.size()) {
			subscribers_[handle] = nullptr;
		}
	}

	/// Broadcast @p atoms to all subscribers as the next sync_event.
	/// If a broadcast is already in progress, the call is dropped (overrun).
	/// Thread-safe: broadcasts from concurrent threads are serialised by mutex.
	void broadcast(std::vector<MemoryAtom> atoms, SubsystemSaliences saliences) {
		std::unique_lock<std::mutex> lock(mutex_);

		if (in_progress_) {
			++overruns_;
			return;
		}

		in_progress_ = true;
		SyncEvent event{++sequence_, saliences, std::move(atoms)};

		// Copy subscribers so that mutations during dispatch are safe.
		std::vector<SyncEventCallback> snapshot(subscribers_);
		lock.unlock();

		for (const auto &cb : snapshot) {
			if (cb) {
				cb(event);
			}
		}

		lock.lock();
		in_progress_ = false;
	}

	/// Number of broadcast calls that were dropped due to re-entrancy.
	uint64_t overruns() const noexcept {
		std::lock_guard<std::mutex> lock(mutex_);
		return overruns_;
	}

	/// Current sequence number (number of completed broadcasts).
	uint64_t sequence() const noexcept {
		std::lock_guard<std::mutex> lock(mutex_);
		return sequence_;
	}

private:
	mutable std::mutex          mutex_;
	std::vector<SyncEventCallback> subscribers_;
	uint64_t                    sequence_;
	bool                        in_progress_;
	uint64_t                    overruns_;
};

}  // namespace lizmem
