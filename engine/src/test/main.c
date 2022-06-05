#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <unity.h>

#include "../core/memory/rbe_mem.h"
#include "../core/data_structures/rbe_hash_map.h"
#include "../core/data_structures/rbe_hash_map_string.h"
#include "../core/thread/rbe_pthread.h"
#include "../core/thread/rbe_thread_pool.h"

void rbe_hash_main_test();
void rbe_string_hashmap_test();

void rbe_thread_main_test();

void setUp() {}
void tearDown() {}

int main(int argv, char** args) {
    UNITY_BEGIN();
    RUN_TEST(rbe_hash_main_test);
    RUN_TEST(rbe_string_hashmap_test);
    RUN_TEST(rbe_thread_main_test);
    return UNITY_END();
}

// RBE Hash Test
void rbe_hash_main_test() {
    // Pointer test
    struct User {
        const char* name;
        int age;
    };
    // Map Creation
    RBEHashMap* map = rbe_hash_map_create(sizeof(char*), sizeof(struct User), 32);
    TEST_ASSERT_TRUE(map != NULL);
    struct User* user = RBE_MEM_ALLOCATE(struct User);
    user->name = "Hey";
    user->age = 32;
    char* hashMapKey = RBE_MEM_ALLOCATE_SIZE(sizeof(char*));
    hashMapKey = "test";
    // Adding a value to the map
    rbe_hash_map_add(map, hashMapKey, user);
    // Checking if key exists in map
    TEST_ASSERT_TRUE(rbe_hash_map_has(map, hashMapKey));
    // Testing getting a value from the hashmap
    struct User* returnedUser = (struct User*) rbe_hash_map_get(map, hashMapKey);
    TEST_ASSERT_TRUE(returnedUser != NULL);
    TEST_ASSERT_TRUE(strcmp(returnedUser->name, "Hey") == 0);
    TEST_ASSERT_EQUAL_INT(returnedUser->age, 32);
    // Erasing from map
    rbe_hash_map_erase(map, &hashMapKey);
    TEST_ASSERT_TRUE(!rbe_hash_map_has(map, &hashMapKey));
    // Test destory hash map
    rbe_hash_map_destroy(map);
    // Clean up
    RBE_MEM_FREE(user);
    RBE_MEM_FREE(hashMapKey);
}

void rbe_string_hashmap_test() {
    RBEStringHashMap* hashMap = rbe_string_hash_map_create(10);

    struct Fighter {
        int hp;
        int attack;
    };

    struct Fighter* fighter = RBE_MEM_ALLOCATE(struct Fighter);
    fighter->hp = 10;
    fighter->attack = 20;

    rbe_string_hash_map_add(hashMap, "one", fighter, sizeof(struct Fighter));

    char* stringValue = strdup("mike");
    rbe_string_hash_map_add(hashMap, "name", stringValue, strlen(stringValue) + 1);
    TEST_ASSERT_EQUAL(rbe_string_hash_map_has(hashMap, "name"), true);

    char* returnedName = rbe_string_hash_map_get(hashMap, "name");
    TEST_ASSERT_EQUAL_STRING(returnedName, "mike");
    rbe_string_hash_map_erase(hashMap, "name");
    TEST_ASSERT_EQUAL(rbe_string_hash_map_has(hashMap, "name"), false);
    rbe_string_hash_map_destroy(hashMap);

    RBEStringHashMap* hashMap2 = rbe_string_hash_map_create(10);
    rbe_string_hash_map_add_int(hashMap2, "one", 1);
    TEST_ASSERT_EQUAL(rbe_string_hash_map_has(hashMap2, "one"), true);
    TEST_ASSERT_EQUAL_INT(rbe_string_hash_map_get_int(hashMap2, "one"), 1);
    rbe_string_hash_map_destroy(hashMap2);
}

int test_thread_func(void* arg) {
    printf("Test\n");
    return 0;
}

void* print_message_func(void* ptr) {
    char* message = (char*) ptr;
    printf("%s\n", message);
}

void thread_worker(void* arg) {
    int *val = arg;
    int old = *val;

    *val += 1000;
    printf("tip = %d, old = %d, val = %d\n", pthread_self(), old, *val);

    if (*val % 2) {
        usleep(1000);
    }
}

void rbe_thread_main_test() {
    // P Thread
    char* message1 = "Thread1";
    char* message2 = "Thread2";

    pthread_t thread1;
    pthread_t thread2;
    int thread1Return = pthread_create(&thread1, NULL, print_message_func, (void*) message1);
    int thread2Return = pthread_create(&thread2, NULL, print_message_func, (void*) message2);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    printf("Thread 1 returns: %d\n", thread1Return);
    printf("Thread 2 returns: %d\n", thread2Return);

    // Thread Pool
    ThreadPool* tp;
    int* vals;
    const size_t numberOfElements = 100;
    printf("Number of cores = %d\n", pcthread_get_num_procs());
    tp = tpool_create(pcthread_get_num_procs() / 2);
    vals = calloc(numberOfElements, sizeof(*vals));

    for (size_t i = 0; i < numberOfElements; i++) {
        vals[i] = (int) i;
        tpool_add_work(tp, thread_worker, vals + i);
    }

    tpool_wait(tp);

    for (size_t i = 0; i < numberOfElements; i++) {
        printf("%d\n", vals[i]);
    }

    free(vals);
    tpool_destroy(tp);
}
