/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/common/random_access_set.h>
#include <aws/common/string.h>

#include <aws/testing/aws_test_harness.h>

static uint64_t s_hash_string_ptr(const void *item) {
    const struct aws_string *str = *(const struct aws_string **)item;
    return aws_hash_string((void *)str);
}

static bool s_hash_string_ptr_eq(const void *a, const void *b) {
    const struct aws_string *str_a = *(const struct aws_string **)a;
    const struct aws_string *str_b = *(const struct aws_string **)b;
    return aws_string_eq(str_a, str_b);
}

static int s_random_access_set_sanitize_fn(struct aws_allocator *allocator, void *ctx) {
    (void)ctx;

    struct aws_random_access_set list_with_map;
    ASSERT_SUCCESS(
        aws_random_access_set_init(&list_with_map, allocator, s_hash_string_ptr, aws_ptr_eq, NULL, sizeof(void *), 0));
    aws_random_access_set_clean_up(&list_with_map);
    return AWS_OP_SUCCESS;
}

AWS_TEST_CASE(random_access_set_sanitize_test, s_random_access_set_sanitize_fn)

static int s_random_access_set_insert_fn(struct aws_allocator *allocator, void *ctx) {
    (void)ctx;
    AWS_STATIC_STRING_FROM_LITERAL(foo, "foo");
    AWS_STATIC_STRING_FROM_LITERAL(bar, "bar");
    AWS_STATIC_STRING_FROM_LITERAL(foobar, "foobar");

    struct aws_random_access_set list_with_map;
    /* With only 1 initial element. */
    ASSERT_SUCCESS(aws_random_access_set_init(
        &list_with_map, allocator, s_hash_string_ptr, s_hash_string_ptr_eq, NULL, sizeof(struct aws_string *), 1));
    ASSERT_SUCCESS(aws_random_access_set_insert(&list_with_map, &foobar));
    ASSERT_SUCCESS(aws_random_access_set_insert(&list_with_map, &bar));
    ASSERT_SUCCESS(aws_random_access_set_insert(&list_with_map, &foo));

    /* You cannot have duplicates */
    ASSERT_FAILS(aws_random_access_set_insert(&list_with_map, &foobar));

    /* Check the size */
    ASSERT_UINT_EQUALS(aws_random_access_set_length(&list_with_map), 3);

    aws_random_access_set_clean_up(&list_with_map);
    return AWS_OP_SUCCESS;
}

AWS_TEST_CASE(random_access_set_insert_test, s_random_access_set_insert_fn)

static int s_random_access_set_get_random_fn(struct aws_allocator *allocator, void *ctx) {
    (void)ctx;
    AWS_STATIC_STRING_FROM_LITERAL(foo, "foo");

    struct aws_random_access_set list_with_map;
    /* With only 1 initial element. */
    ASSERT_SUCCESS(aws_random_access_set_init(
        &list_with_map, allocator, s_hash_string_ptr, s_hash_string_ptr_eq, NULL, sizeof(struct aws_string *), 1));
    struct aws_string *left_element = NULL;
    /* Fail to get any, when there is nothing in it. */
    ASSERT_FAILS(aws_random_access_set_get_random(&list_with_map, &left_element));
    ASSERT_SUCCESS(aws_random_access_set_insert(&list_with_map, &foo));

    /* Check the size */
    ASSERT_UINT_EQUALS(aws_random_access_set_length(&list_with_map), 1);
    ASSERT_SUCCESS(aws_random_access_set_get_random(&list_with_map, &left_element));
    ASSERT_TRUE(aws_string_eq(left_element, foo));

    aws_random_access_set_clean_up(&list_with_map);
    return AWS_OP_SUCCESS;
}

AWS_TEST_CASE(random_access_set_get_random_test, s_random_access_set_get_random_fn)

static int s_random_access_set_exist_fn(struct aws_allocator *allocator, void *ctx) {
    (void)ctx;
    AWS_STATIC_STRING_FROM_LITERAL(foo, "foo");
    AWS_STATIC_STRING_FROM_LITERAL(bar, "bar");

    struct aws_random_access_set list_with_map;
    ASSERT_SUCCESS(aws_random_access_set_init(
        &list_with_map, allocator, s_hash_string_ptr, s_hash_string_ptr_eq, NULL, sizeof(struct aws_string *), 1));
    ASSERT_SUCCESS(aws_random_access_set_insert(&list_with_map, &foo));

    ASSERT_TRUE(aws_random_access_set_exist(&list_with_map, &foo));
    ASSERT_FALSE(aws_random_access_set_exist(&list_with_map, &bar));

    aws_random_access_set_clean_up(&list_with_map);
    return AWS_OP_SUCCESS;
}

AWS_TEST_CASE(random_access_set_exist_test, s_random_access_set_exist_fn)

static int s_random_access_set_remove_fn(struct aws_allocator *allocator, void *ctx) {
    (void)ctx;
    (void)ctx;
    AWS_STATIC_STRING_FROM_LITERAL(foo, "foo");
    AWS_STATIC_STRING_FROM_LITERAL(bar, "bar");
    AWS_STATIC_STRING_FROM_LITERAL(foobar, "foobar");

    struct aws_random_access_set list_with_map;
    /* With only 1 initial element. */
    ASSERT_SUCCESS(aws_random_access_set_init(
        &list_with_map, allocator, s_hash_string_ptr, s_hash_string_ptr_eq, NULL, sizeof(struct aws_string *), 1));
    ASSERT_SUCCESS(aws_random_access_set_insert(&list_with_map, &bar));
    ASSERT_SUCCESS(aws_random_access_set_insert(&list_with_map, &foobar));
    ASSERT_SUCCESS(aws_random_access_set_insert(&list_with_map, &foo));

    ASSERT_SUCCESS(aws_random_access_set_remove(&list_with_map, &foo));
    /* Check the size */
    ASSERT_UINT_EQUALS(aws_random_access_set_length(&list_with_map), 2);

    /* Should be fine to remove something not existing anymore? */
    ASSERT_SUCCESS(aws_random_access_set_remove(&list_with_map, &foo));

    /* Remove all beside foobar, so, if we get one random, it will be foobar */
    ASSERT_SUCCESS(aws_random_access_set_remove(&list_with_map, &bar));
    ASSERT_UINT_EQUALS(aws_random_access_set_length(&list_with_map), 1);
    struct aws_string *left_element = NULL;
    ASSERT_SUCCESS(aws_random_access_set_get_random(&list_with_map, &left_element));
    ASSERT_TRUE(aws_string_eq(left_element, foobar));

    /* Remove last thing and make sure everything should still work */
    ASSERT_SUCCESS(aws_random_access_set_remove(&list_with_map, &foobar));
    ASSERT_UINT_EQUALS(aws_random_access_set_length(&list_with_map), 0);
    ASSERT_SUCCESS(aws_random_access_set_insert(&list_with_map, &foo));
    ASSERT_UINT_EQUALS(aws_random_access_set_length(&list_with_map), 1);
    ASSERT_SUCCESS(aws_random_access_set_get_random(&list_with_map, &left_element));
    ASSERT_TRUE(aws_string_eq(left_element, foo));

    aws_random_access_set_clean_up(&list_with_map);
    return AWS_OP_SUCCESS;
}

AWS_TEST_CASE(random_access_set_remove_test, s_random_access_set_remove_fn)

static void s_aws_string_destroy_callback(void *key) {
    struct aws_string *str = *(struct aws_string **)key;
    aws_string_destroy(str);
}

static int s_random_access_set_owns_element_fn(struct aws_allocator *allocator, void *ctx) {
    (void)ctx;
    /* If we copy the aws string itself, the underlying data will be copied as a pointer, if the data is
     * less than the size of a pointer, we will be fine. The long string will test against it. */
    struct aws_string *foo = aws_string_new_from_c_str(allocator, "foo123456");
    struct aws_string *bar = aws_string_new_from_c_str(allocator, "bar7894156132121");
    struct aws_string *foobar = aws_string_new_from_c_str(allocator, "foobar970712389709123");

    struct aws_random_access_set list_with_map;
    /* With only 1 initial element. Add clean up for the string */
    ASSERT_SUCCESS(aws_random_access_set_init(
        &list_with_map,
        allocator,
        s_hash_string_ptr,
        s_hash_string_ptr_eq,
        s_aws_string_destroy_callback,
        sizeof(struct aws_string *),
        1));
    ASSERT_SUCCESS(aws_random_access_set_insert(&list_with_map, &foobar));
    ASSERT_SUCCESS(aws_random_access_set_insert(&list_with_map, &bar));
    ASSERT_SUCCESS(aws_random_access_set_insert(&list_with_map, &foo));

    /* You cannot have duplicates */
    ASSERT_FAILS(aws_random_access_set_insert(&list_with_map, &foobar));

    ASSERT_SUCCESS(aws_random_access_set_remove(&list_with_map, &foo));
    ASSERT_SUCCESS(aws_random_access_set_remove(&list_with_map, &foobar));

    /* Check the size */
    ASSERT_UINT_EQUALS(aws_random_access_set_length(&list_with_map), 1);

    aws_random_access_set_clean_up(&list_with_map);
    return AWS_OP_SUCCESS;
}

AWS_TEST_CASE(random_access_set_owns_element_test, s_random_access_set_owns_element_fn)
