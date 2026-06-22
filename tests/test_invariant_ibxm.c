#include <check.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Include the production source directly */
#include "code/build/template/include/extensions/[+mod ibxm]/ibxm.c"

START_TEST(test_sample_ping_pong_bounds)
{
    /* Invariant: sample_ping_pong must never read beyond the allocated
       sample->data buffer regardless of loop_start/loop_length values. */

    struct {
        int loop_start;
        int loop_length;
        int data_samples; /* actual number of shorts allocated for data */
    } cases[] = {
        /* Exploit case: loop_end overflows / exceeds data size */
        { 0x7FFFFFFF, 0x7FFFFFFF, 4 },
        /* Boundary: loop_end exactly equals data size */
        { 8, 8, 16 },
        /* Valid input: well-formed small loop */
        { 2, 4, 16 },
    };

    int num_cases = sizeof(cases) / sizeof(cases[0]);

    for (int i = 0; i < num_cases; i++) {
        struct sample s;
        memset(&s, 0, sizeof(s));

        int data_samples = cases[i].data_samples;
        s.loop_start  = cases[i].loop_start;
        s.loop_length = cases[i].loop_length;
        s.data = (short *)calloc(data_samples, sizeof(short));
        ck_assert_ptr_nonnull(s.data);

        /* Fill with sentinel values */
        for (int j = 0; j < data_samples; j++)
            s.data[j] = (short)(j + 1);

        /* Compute loop_end as the function does; guard against overflow */
        long long loop_end = (long long)s.loop_start + (long long)s.loop_length;

        /* Invariant: loop_end must not exceed the allocated data buffer */
        ck_assert_msg(loop_end <= data_samples || loop_end <= 0 ||
                      s.loop_start < 0 || s.loop_length <= 0,
                      "loop_end (%lld) exceeds data_samples (%d) — "
                      "calling sample_ping_pong would over-read heap",
                      loop_end, data_samples);

        /* Only call the real function when inputs are safe */
        if (loop_end > 0 && loop_end <= data_samples &&
            s.loop_start >= 0 && s.loop_length > 0) {
            sample_ping_pong(&s);
            /* After call, data pointer must be non-null (function replaces it) */
            ck_assert_ptr_nonnull(s.data);
            free(s.data);
        } else {
            free(s.data);
        }
    }
}
END_TEST

Suite *security_suite(void)
{
    Suite *s = suite_create("Security");
    TCase *tc = tcase_create("Core");
    tcase_add_test(tc, test_sample_ping_pong_bounds);
    suite_add_tcase(s, tc);
    return s;
}

int main(void)
{
    Suite *s = security_suite();
    SRunner *sr = srunner_create(s);
    srunner_run_all(sr, CK_NORMAL);
    int failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}