//
// Created by Kalenda, Jan on 02.10.2022.
//

#ifndef IFJ_LEX_ALL_H
#define IFJ_LEX_ALL_H

#include <string.h>

#define TEST(NAME, DESCRIPTION)                                                \
int NAME() {                                                                   \
    int STATUS = 0;                                                            \
    printf("\n[%s] %s\n\n", #NAME, DESCRIPTION);                               \

#define TEST_ASSERT(CONDITION)                                                 \
    if (CONDITION) {                                                           \
        PASS("Assertion suceeded: " #CONDITION);                               \
    } else {                                                                   \
        FAIL("Assertion failed: " #CONDITION);                                 \
        STATUS = 1;                                                            \
    }

#define ENDTEST(STREAM, FILE)                                                  \
    fclose(STREAM);                                                            \
    remove(FILE);                                                              \
    printf("\n");                                                              \
    return STATUS;                                                             \
}

#define FAIL(REASON) printf("[FAILED] %s\n", REASON)

#define PASS(REASON) printf("[PASSED] %s\n", REASON)

int write_file(const char *filename, const char *content);

int test_empty(void);

int test_only_prologue(void);

int test_only_prologue_with_spaces(void);

int test_only_prologue_with_newline(void);

int test_only_prologue_with_newline_and_spaces(void);

int test_only_prologue_with_newline_and_spaces_and_comment(void);

int test_only_prologue_with_newline_and_spaces_and_comment_and_newline(void);

int test_only_prologue_with_newline_and_spaces_and_comment_and_newline_and_spaces(void);

int test_only_prologue_with_newline_and_spaces_and_comment_and_newline_and_spaces_and_comment(void);

int test_only_prologue_with_newline_and_block_comments(void);

int test_only_prologue_with_newline_and_block_comments_and_newline(void);

int test_only_prologue_with_newline_and_block_comments_and_newline_and_spaces(void);

int test_only_prologue_with_newline_and_block_comments_and_newline_and_spaces_and_comment(void);

int test_only_prologue_with_multiline_block_comment(void);

int test_only_prologue_with_declare(void);

int test_only_prologue_with_comments_and_declare(void);

int test_only_prologue_with_block_comments_and_declare(void);

int test_only_prologue_with_more_block_comments_in_declare(void);

int test_only_prologue_with_multiline_block_comments_in_declare(void);

int test_only_prologue_with_multiline_block_comments_before_declare(void);

int test_only_prologue_with_multiline_block_comments_after_declare(void);

int test_prologue_declare_variable_string(void);

int test_prologue_declare_variable_int(void);

int test_prologue_declare_variable_float(void);

int test_prologue_declare_variable_int_as_string(void);

int test_prologue_declare_variable_float_as_string(void);

int test_prologue_declare_variable_float_with_exp(void);

int test_prologue_declare_variable_float_with_exp_as_string(void);

#endif //IFJ_LEX_ALL_H
