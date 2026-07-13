// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// MonoGame.Extended has no upstream tests for KeyedCollection.cs. Fresh tests below.
#include "CNA/Extended/Collections/KeyedCollection.hpp"

#include "System/Collections/Generic/KeyNotFoundException.hpp"

#include <gtest/gtest.h>

#include <string>

namespace CNA::Extended::Collections
{
    namespace
    {
        struct Person
        {
            int Id = 0;
            std::string Name;

            bool operator==(const Person& other) const { return Id == other.Id && Name == other.Name; }
        };

        KeyedCollection<int, Person> MakeCollection() { return KeyedCollection<int, Person>([](const Person& p) { return p.Id; }); }
    }

    TEST(KeyedCollectionTests, AddAndIndexByKey)
    {
        KeyedCollection<int, Person> collection = MakeCollection();
        collection.Add(Person{1, "Alice"});

        EXPECT_EQ(collection[1].Name, "Alice");
        EXPECT_EQ(collection.getCountProperty(), 1);
    }

    TEST(KeyedCollectionTests, IndexerThrowsForMissingKey)
    {
        KeyedCollection<int, Person> collection = MakeCollection();
        EXPECT_THROW((void)collection[42], System::Collections::Generic::KeyNotFoundException);
    }

    TEST(KeyedCollectionTests, ContainsKeyReflectsPresence)
    {
        KeyedCollection<int, Person> collection = MakeCollection();
        collection.Add(Person{1, "Alice"});

        EXPECT_TRUE(collection.ContainsKey(1));
        EXPECT_FALSE(collection.ContainsKey(2));
    }

    TEST(KeyedCollectionTests, ContainsChecksByDerivedKeyOfTheGivenValue)
    {
        KeyedCollection<int, Person> collection = MakeCollection();
        collection.Add(Person{1, "Alice"});

        // Contains checks the derived key, not full value equality -- a Person with the same Id
        // but a different Name is still "contained".
        EXPECT_TRUE(collection.Contains(Person{1, "SomeoneElse"}));
        EXPECT_FALSE(collection.Contains(Person{2, "Alice"}));
    }

    TEST(KeyedCollectionTests, TryGetValueReturnsFalseForMissingKey)
    {
        KeyedCollection<int, Person> collection = MakeCollection();
        Person value;
        EXPECT_FALSE(collection.TryGetValue(1, value));

        collection.Add(Person{1, "Alice"});
        EXPECT_TRUE(collection.TryGetValue(1, value));
        EXPECT_EQ(value.Name, "Alice");
    }

    TEST(KeyedCollectionTests, RemoveDeletesByDerivedKey)
    {
        KeyedCollection<int, Person> collection = MakeCollection();
        collection.Add(Person{1, "Alice"});

        EXPECT_TRUE(collection.Remove(Person{1, "Ignored"}));
        EXPECT_EQ(collection.getCountProperty(), 0);
        EXPECT_FALSE(collection.Remove(Person{1, "Ignored"}));
    }

    TEST(KeyedCollectionTests, ClearRemovesEverything)
    {
        KeyedCollection<int, Person> collection = MakeCollection();
        collection.Add(Person{1, "Alice"});
        collection.Add(Person{2, "Bob"});

        collection.Clear();
        EXPECT_EQ(collection.getCountProperty(), 0);
    }

    TEST(KeyedCollectionTests, CopyToAlwaysThrows)
    {
        KeyedCollection<int, Person> collection = MakeCollection();
        EXPECT_THROW(collection.CopyTo(nullptr, 0), std::logic_error);
    }

    TEST(KeyedCollectionTests, IterationVisitsAllValues)
    {
        KeyedCollection<int, Person> collection = MakeCollection();
        collection.Add(Person{1, "Alice"});
        collection.Add(Person{2, "Bob"});

        int visited = 0;
        bool sawAlice = false;
        bool sawBob = false;
        for (const Person& p : collection)
        {
            visited++;
            sawAlice = sawAlice || p.Name == "Alice";
            sawBob = sawBob || p.Name == "Bob";
        }

        EXPECT_EQ(visited, 2);
        EXPECT_TRUE(sawAlice);
        EXPECT_TRUE(sawBob);
    }

    TEST(KeyedCollectionTests, GetKeysAndGetValuesReflectContents)
    {
        KeyedCollection<int, Person> collection = MakeCollection();
        collection.Add(Person{1, "Alice"});
        collection.Add(Person{2, "Bob"});

        const std::vector<int> keys = collection.getKeysProperty();
        const std::vector<Person> values = collection.getValuesProperty();

        EXPECT_EQ(keys.size(), 2u);
        EXPECT_EQ(values.size(), 2u);
    }

    TEST(KeyedCollectionTests, IsReadOnlyIsAlwaysFalse)
    {
        const KeyedCollection<int, Person> collection = MakeCollection();
        EXPECT_FALSE(collection.getIsReadOnlyProperty());
    }
}
