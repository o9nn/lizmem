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
#include "memory/memory_schema.h"

#include <gtest/gtest.h>

using namespace lizmem;

static MemoryAtom makeEpisodic(uint64_t name, float salience) {
	return MemoryAtom{
	    name,
	    MemorySubsystemTag::kEpisodic,
	    salience,
	    EpisodicPayload{"ep-" + std::to_string(name), "test episode", {}}};
}

static MemoryAtom makeSemantic(uint64_t name, float salience) {
	return MemoryAtom{
	    name,
	    MemorySubsystemTag::kSemantic,
	    salience,
	    SemanticPayload{"concept-" + std::to_string(name), {}}};
}

static MemoryAtom makeWorking(uint64_t name, float salience) {
	return MemoryAtom{
	    name,
	    MemorySubsystemTag::kWorking,
	    salience,
	    WorkingPayload{"key-" + std::to_string(name), "value"}};
}

TEST(MemorySchemaTests, InitiallyEmpty) {
	MemorySchema schema;
	EXPECT_TRUE(schema.atomSpace().empty());
}

TEST(MemorySchemaTests, EpisodicConsolidate) {
	MemorySchema schema;
	schema.episodic().consolidate({makeEpisodic(2, 0.8f)});
	auto atoms = schema.episodic().atoms();
	ASSERT_EQ(atoms.size(), 1u);
	EXPECT_EQ(atoms[0].eternal_name, 2u);
	EXPECT_EQ(atoms[0].subsystem, MemorySubsystemTag::kEpisodic);
}

TEST(MemorySchemaTests, SemanticConsolidate) {
	MemorySchema schema;
	schema.semantic().consolidate({makeSemantic(3, 0.5f)});
	auto atoms = schema.semantic().atoms();
	ASSERT_EQ(atoms.size(), 1u);
	EXPECT_EQ(atoms[0].eternal_name, 3u);
}

TEST(MemorySchemaTests, WorkingMemoryReplacesOnConsolidate) {
	MemorySchema schema;
	schema.working().consolidate({makeWorking(7, 0.9f)});
	EXPECT_EQ(schema.working().atoms().size(), 1u);

	// A second consolidation replaces the working memory.
	schema.working().consolidate({makeWorking(11, 0.7f)});
	auto atoms = schema.working().atoms();
	ASSERT_EQ(atoms.size(), 1u);
	EXPECT_EQ(atoms[0].eternal_name, 11u);
}

TEST(MemorySchemaTests, SubsystemsAreIndependent) {
	MemorySchema schema;
	schema.episodic().consolidate({makeEpisodic(2, 0.8f)});
	schema.semantic().consolidate({makeSemantic(3, 0.6f)});
	EXPECT_EQ(schema.episodic().atoms().size(), 1u);
	EXPECT_EQ(schema.semantic().atoms().size(), 1u);
	EXPECT_EQ(schema.working().atoms().size(), 0u);
}

TEST(MemorySchemaTests, RootMatulaNumber) {
	EXPECT_EQ(MemorySchema::rootMatulaNumber(), 30030u);
}

TEST(MemorySchemaTests, SubsystemNames) {
	EXPECT_STREQ(subsystemName(MemorySubsystemTag::kEpisodic),      "Episodic");
	EXPECT_STREQ(subsystemName(MemorySubsystemTag::kSemantic),      "Semantic");
	EXPECT_STREQ(subsystemName(MemorySubsystemTag::kProcedural),    "Procedural");
	EXPECT_STREQ(subsystemName(MemorySubsystemTag::kWorking),       "Working");
	EXPECT_STREQ(subsystemName(MemorySubsystemTag::kDeclarative),   "Declarative");
	EXPECT_STREQ(subsystemName(MemorySubsystemTag::kParticipatory), "Participatory");
}

TEST(AtomSpaceTests, UpsertAndFind) {
	AtomSpace space;
	space.upsert(makeEpisodic(2, 0.5f));
	auto found = space.find(2);
	ASSERT_TRUE(found.has_value());
	EXPECT_EQ(found->eternal_name, 2u);
}

TEST(AtomSpaceTests, EraseReturnsCorrect) {
	AtomSpace space;
	space.upsert(makeEpisodic(2, 0.5f));
	EXPECT_TRUE(space.erase(2));
	EXPECT_FALSE(space.erase(2));
	EXPECT_FALSE(space.find(2).has_value());
}

TEST(AtomSpaceTests, BySalienceOrdering) {
	AtomSpace space;
	space.upsert(makeEpisodic(2, 0.3f));
	space.upsert(makeEpisodic(3, 0.9f));
	space.upsert(makeEpisodic(5, 0.6f));
	auto ordered = space.bySalience();
	ASSERT_EQ(ordered.size(), 3u);
	EXPECT_FLOAT_EQ(ordered[0].salience, 0.9f);
	EXPECT_FLOAT_EQ(ordered[1].salience, 0.6f);
	EXPECT_FLOAT_EQ(ordered[2].salience, 0.3f);
}

TEST(AtomSpaceTests, BySalienceThreshold) {
	AtomSpace space;
	space.upsert(makeEpisodic(2, 0.3f));
	space.upsert(makeEpisodic(3, 0.9f));
	auto high = space.bySalience(0.5f);
	ASSERT_EQ(high.size(), 1u);
	EXPECT_EQ(high[0].eternal_name, 3u);
}
