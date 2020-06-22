#include "gtest/gtest.h"
#include "pm_ehash.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <unordered_set>

using namespace std;

TEST(RandomTest, YF) {
	PmEHash *ehash = new PmEHash;
	int result;
	kv tem;
	srand(time(0));
	for (int i = 0; i < 1000000; ++i) {
		tem.key = rand();
		tem.value = tem.key ^ 1;
		result = ehash->insert(tem);
//		GTEST_ASSERT_EQ(result, 0);
	}

	ehash->selfDestory();
}

TEST(BigTest, YF) {
	PmEHash* ehash = new PmEHash;
	kv tem;
	int result;
	srand(time(0));
	for (int i = 0; i < 2200000; ++i) {
		tem.key = i;
		tem.value = tem.key ^ 1;
		result = ehash->insert(tem);
		GTEST_ASSERT_EQ(result, 0);
	}
	printf("Step 1 done\n");

	uint64_t key, value;
	for (int i = 0; i < 1000000; ++i) {
		key = rand() % 2200000;
		result = ehash->search(key, value);
		GTEST_ASSERT_EQ(result, 0);
		GTEST_ASSERT_EQ(value, key ^ 1);
	}
	printf("Step 2 done\n");

	std::unordered_set<uint64_t> removed;
	for (int i = 0; i < 1000000; ++i) {
		key = rand() % 2200000;
		result = ehash->remove(key);
		if (removed.count(key)) {
			GTEST_ASSERT_EQ(result, -1);
		}
		else {
			GTEST_ASSERT_EQ(result, 0);
			removed.insert(key);
		}
	}
	printf("Step 3 done\n");

	std::unordered_set<uint64_t> updated;
	for (int i = 0; i < 700000; ++i) {
		key = rand() % 2200000;
		value = key;
		result = ehash->update((kv){key, value});
		if (removed.count(key)) {
			GTEST_ASSERT_EQ(result, -1);
		}
		else {
			GTEST_ASSERT_EQ(result, 0);
			updated.insert(key);
		}
	}
	printf("Step 4 done\n");

	for (int i = 0; i < 2200000; ++i) {
		key = i;
		result = ehash->search(key, value);
		if (removed.count(key)) {
			GTEST_ASSERT_EQ(result, -1);
			continue;
		}
		GTEST_ASSERT_EQ(result, 0);
		if (updated.count(key)) {
			GTEST_ASSERT_EQ(value, key);
		}
		else {
			GTEST_ASSERT_EQ(value, key ^ 1);
		}
	}
	printf("Step 5 done\n");

	ehash->selfDestory();
}

//TEST(InsertTest, SingleInsert) {
//    PmEHash* ehash = new PmEHash;
//    kv temp;
//    temp.key = temp.value = 1;
//    int result = ehash->insert(temp);
//    GTEST_ASSERT_EQ(result, 0);
//    uint64_t val;
//    result = ehash->search(1, val);
//    GTEST_ASSERT_EQ(result, 0);
//    GTEST_ASSERT_EQ(val, 1);
//    result = ehash->search(0, val);
//    GTEST_ASSERT_EQ(result, -1);
//    ehash->selfDestory();
//}
//
//TEST(InsertTest, DuplicateInsert) {
//    PmEHash* ehash = new PmEHash;
//    kv temp;
//    temp.key = temp.value = 1;
//    int result = ehash->insert(temp);
//    GTEST_ASSERT_EQ(result, 0);
//    result = ehash->insert(temp);
//    GTEST_ASSERT_EQ(result, -1);
//    ehash->selfDestory();
//}
//
//TEST(UpdateTest, SingleUpdate) {
//    PmEHash* ehash = new PmEHash;
//    kv temp;
//    temp.key = temp.value = 1;
//    int result = ehash->insert(temp);
//    GTEST_ASSERT_EQ(result, 0);
//    temp.value = 2;
//    result = ehash->update(temp);
//    GTEST_ASSERT_EQ(result, 0);
//    temp.key = 2;
//    result = ehash->update(temp);
//    GTEST_ASSERT_EQ(result, -1);
//    ehash->selfDestory();
//}
//
//TEST(SearchTest, SingleSearch) {
//    PmEHash* ehash = new PmEHash;
//    kv temp;
//    temp.key = temp.value = 1;
//    int result = ehash->insert(temp);
//    uint64_t val = 0;
//    result = ehash->search(1, val);
//    GTEST_ASSERT_EQ(result, 0);
//    GTEST_ASSERT_EQ(val, 1);
//    ehash->selfDestory();
//}
//
//TEST(RemoveTest, SingleRemove) {
//    PmEHash* ehash = new PmEHash;
//    kv temp;
//    temp.key = temp.value = 1;
//    int result = ehash->insert(temp);
//    GTEST_ASSERT_EQ(result, 0);
//    temp.key = temp.value = 2;
//    result = ehash->insert(temp);
//    GTEST_ASSERT_EQ(result, 0);
//    result = ehash->remove(1);
//    GTEST_ASSERT_EQ(result, 0);
//    result = ehash->remove(1);
//    GTEST_ASSERT_EQ(result, -1);
//    uint64_t val = 0;
//    result = ehash->search(1, val);
//    GTEST_ASSERT_EQ(result, -1);
//    GTEST_ASSERT_EQ(val, 0);
//    ehash->selfDestory();
//}
