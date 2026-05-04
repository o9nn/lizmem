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
#include "memory/memory_consolidator.h"

#include <gtest/gtest.h>

using namespace lizmem;

static MemoryAtom makeAtom(MemorySubsystemTag tag, uint64_t name, float sal) {
	switch (tag) {
	case MemorySubsystemTag::kEpisodic:
		return MemoryAtom{name, tag, sal,
		                  EpisodicPayload{"ep", "desc", {}}};
	case MemorySubsystemTag::kSemantic:
		return MemoryAtom{name, tag, sal,
		                  SemanticPayload{"concept", {}}};
	case MemorySubsystemTag::kProcedural:
		return MemoryAtom{name, tag, sal,
		                  ProceduralPayload{"rule", "body"}};
	case MemorySubsystemTag::kWorking:
		return MemoryAtom{name, tag, sal,
		                  WorkingPayload{"k", "v"}};
	case MemorySubsystemTag::kDeclarative:
		return MemoryAtom{name, tag, sal,
		                  DeclarativePayload{"pred", {}}};
	case MemorySubsystemTag::kParticipatory:
		return MemoryAtom{name, tag, sal,
		                  ParticipatoryPayload{"A", "knows", "B"}};
	}
	// unreachable
	return MemoryAtom{name, tag, sal, WorkingPayload{}};
}

static SubsystemSaliences allHigh() {
	SubsystemSaliences s{};
	s.fill(1.0f);
	return s;
}

static SubsystemSaliences allLow() {
	SubsystemSaliences s{};
	s.fill(0.0f);
	return s;
}

TEST(MemoryConsolidatorTests, ConsolidatesOnHighSalience) {
	MemorySchema schema;
	GlobalWorkspaceBroadcaster broadcaster;
	MemoryConsolidator consolidator(schema, broadcaster);

	std::vector<MemoryAtom> atoms = {
	    makeAtom(MemorySubsystemTag::kEpisodic, 2, 0.9f),
	    makeAtom(MemorySubsystemTag::kSemantic, 3, 0.8f),
	};
	broadcaster.broadcast(atoms, allHigh());

	EXPECT_EQ(consolidator.consolidationCount(), 1u);
	EXPECT_EQ(consolidator.skippedCount(), 0u);
	EXPECT_EQ(schema.episodic().atoms().size(), 1u);
	EXPECT_EQ(schema.semantic().atoms().size(), 1u);
}

TEST(MemoryConsolidatorTests, SkipsOnLowSalience) {
	MemorySchema schema;
	GlobalWorkspaceBroadcaster broadcaster;
	MemoryConsolidator consolidator(schema, broadcaster, 0.5f);

	std::vector<MemoryAtom> atoms = {
	    makeAtom(MemorySubsystemTag::kEpisodic, 2, 0.9f),
	};
	broadcaster.broadcast(atoms, allLow());

	EXPECT_EQ(consolidator.consolidationCount(), 0u);
	EXPECT_EQ(consolidator.skippedCount(), 1u);
	EXPECT_EQ(schema.episodic().atoms().size(), 0u);
}

TEST(MemoryConsolidatorTests, SelectiveSubsystemConsolidation) {
	MemorySchema schema;
	GlobalWorkspaceBroadcaster broadcaster;
	MemoryConsolidator consolidator(schema, broadcaster, 0.5f);

	// Only episodic salience is above threshold.
	SubsystemSaliences saliences{};
	saliences.fill(0.0f);
	saliences[static_cast<uint8_t>(MemorySubsystemTag::kEpisodic)] = 0.9f;

	std::vector<MemoryAtom> atoms = {
	    makeAtom(MemorySubsystemTag::kEpisodic, 2, 0.9f),
	    makeAtom(MemorySubsystemTag::kSemantic, 3, 0.8f),
	};
	broadcaster.broadcast(atoms, saliences);

	EXPECT_EQ(consolidator.consolidationCount(), 1u);
	// Episodic atom was written, semantic was not (salience below threshold).
	EXPECT_EQ(schema.episodic().atoms().size(), 1u);
	EXPECT_EQ(schema.semantic().atoms().size(), 0u);
}

TEST(MemoryConsolidatorTests, MultipleBroadcasts) {
	MemorySchema schema;
	GlobalWorkspaceBroadcaster broadcaster;
	MemoryConsolidator consolidator(schema, broadcaster);

	for (uint64_t i = 2; i <= 11; ++i) {
		broadcaster.broadcast(
		    {makeAtom(MemorySubsystemTag::kEpisodic, i, 0.5f)}, allHigh());
	}

	EXPECT_EQ(consolidator.consolidationCount(), 10u);
	EXPECT_EQ(schema.episodic().atoms().size(), 10u);
}

TEST(GlobalWorkspaceBroadcasterTests, SequenceIncrements) {
	GlobalWorkspaceBroadcaster broadcaster;
	EXPECT_EQ(broadcaster.sequence(), 0u);
	broadcaster.broadcast({}, allHigh());
	EXPECT_EQ(broadcaster.sequence(), 1u);
	broadcaster.broadcast({}, allHigh());
	EXPECT_EQ(broadcaster.sequence(), 2u);
}

TEST(GlobalWorkspaceBroadcasterTests, UnsubscribeSilences) {
	GlobalWorkspaceBroadcaster broadcaster;
	int call_count = 0;
	size_t handle = broadcaster.subscribe([&](const SyncEvent &) { ++call_count; });
	broadcaster.broadcast({}, allHigh());
	EXPECT_EQ(call_count, 1);
	broadcaster.unsubscribe(handle);
	broadcaster.broadcast({}, allHigh());
	EXPECT_EQ(call_count, 1);
}

TEST(GlobalWorkspaceBroadcasterTests, NoOverrunsOnNormal) {
	GlobalWorkspaceBroadcaster broadcaster;
	broadcaster.broadcast({}, allHigh());
	broadcaster.broadcast({}, allHigh());
	EXPECT_EQ(broadcaster.overruns(), 0u);
}
