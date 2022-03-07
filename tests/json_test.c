/**
* Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
* SPDX-License-Identifier: Apache-2.0.
*/

#include <aws/testing/aws_test_harness.h>

#include <aws/common/json.h>

static char* s_test_json = "{\"array\":[1,2,3],\"boolean\":true,\"color\":\"gold\",\"null\":null,\"number\":123,\"object\":{\"a\":\"b\",\"c\":\"d\"}}";

AWS_TEST_CASE(test_json_parse_from_string, s_test_json_parse_from_string)
static int s_test_json_parse_from_string(struct aws_allocator *allocator, void *ctx) {
    (void)ctx;

    aws_json_module_init(allocator);
    struct aws_json_node *root = aws_json_node_from_string(s_test_json);

    ASSERT_NOT_NULL(root);
    AWS_ASSERT(aws_json_node_is_object(root));

    // Testing valid array
    struct aws_json_node* array_node = aws_json_object_get_node(root, "array");
    ASSERT_NOT_NULL(array_node);
    ASSERT_TRUE(aws_json_node_is_array(array_node));
    ASSERT_TRUE(aws_json_array_get_count(array_node) == 3);
    struct aws_json_node* array_node_one = aws_json_array_get_node(array_node, 0);
    ASSERT_NOT_NULL(array_node_one);
    ASSERT_TRUE(aws_json_node_is_number(array_node_one));
    ASSERT_TRUE(*aws_json_node_get_number(array_node_one) == (double)1);

    // Testing valid boolean
    struct aws_json_node* boolean_node = aws_json_object_get_node(root, "boolean");
    ASSERT_NOT_NULL(boolean_node);
    ASSERT_TRUE(aws_json_node_is_boolean(boolean_node));
    ASSERT_TRUE(aws_json_node_get_boolean(boolean_node));

    // Testing valid string
    struct aws_json_node* string_node = aws_json_object_get_node(root, "color");
    ASSERT_NOT_NULL(string_node);
    ASSERT_TRUE(aws_json_node_is_string(string_node));
    ASSERT_TRUE(strcmp(aws_json_node_get_string(string_node), "gold") == 0);

    // Testing valid number
    struct aws_json_node *number_node = aws_json_object_get_node(root, "number");
    ASSERT_NOT_NULL(number_node);
    ASSERT_TRUE(aws_json_node_is_number(number_node));
    ASSERT_TRUE(*aws_json_node_get_number(number_node) == (double)123);

    // Testing valid object
    struct aws_json_node *object_node = aws_json_object_get_node(root, "object");
    ASSERT_NOT_NULL(object_node);
    ASSERT_TRUE(aws_json_node_is_object(object_node) == true);
    struct aws_json_node *sub_object_node = aws_json_object_get_node(object_node, "a");
    ASSERT_NOT_NULL(sub_object_node);
    ASSERT_TRUE(aws_json_node_is_string(sub_object_node));
    ASSERT_TRUE(strcmp(aws_json_node_get_string(sub_object_node), "b") == 0);

    // Testing invalid object
    struct aws_json_node *invalid_object = aws_json_object_get_node(root, "invalid");
    ASSERT_NULL(invalid_object);
    ASSERT_NULL(aws_json_node_get_number(invalid_object));
    // Test getting invalid type of data
    ASSERT_NULL(aws_json_node_get_number(string_node));

    aws_json_node_delete(root);
    aws_json_module_cleanup();

    return 0;
}

AWS_TEST_CASE(test_json_parse_to_string, s_test_json_parse_to_string)
static int s_test_json_parse_to_string(struct aws_allocator *allocator, void *ctx) {
    (void)ctx;

    aws_json_module_init(allocator);

    struct aws_json_node *root = aws_json_make_node_object();

    struct aws_json_node *array = aws_json_make_node_array();
    aws_json_array_add_node(array, aws_json_make_node_number(1));
    aws_json_array_add_node(array, aws_json_make_node_number(2));
    aws_json_array_add_node(array, aws_json_make_node_number(3));
    aws_json_object_add_node(root, "array", array);

    aws_json_object_add_node(root, "boolean", aws_json_make_node_boolean(true));
    aws_json_object_add_node(root, "color", aws_json_make_node_string("gold"));
    aws_json_object_add_node(root, "null", aws_json_make_node_null());
    aws_json_object_add_node(root, "number", aws_json_make_node_number(123));

    struct aws_json_node *object = aws_json_make_node_object();
    aws_json_object_add_node(object, "a", aws_json_make_node_string("b"));
    aws_json_object_add_node(object, "c", aws_json_make_node_string("d"));
    aws_json_object_add_node(root, "object", object);

    char* result_string = aws_json_node_to_string(root);
    ASSERT_STR_EQUALS(s_test_json, result_string);

    aws_json_node_free((void *)result_string);
    aws_json_node_delete(root);
    aws_json_module_cleanup();

    return 0;
}