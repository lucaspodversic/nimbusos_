/* NimbusOS — helpers de tests host. */
#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nt.h"

static int g_failures;

#define EXPECT_TRUE(cond) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        g_failures++; \
    } \
} while (0)

#define EXPECT_EQ(a,b) do { \
    long long _a=(long long)(a), _b=(long long)(b); \
    if (_a != _b) { \
        fprintf(stderr, "FAIL %s:%d: %s (%lld) != %s (%lld)\n", \
                __FILE__, __LINE__, #a, _a, #b, _b); \
        g_failures++; \
    } \
} while (0)

#define EXPECT_STATUS(st, expect) EXPECT_EQ((NTSTATUS)(st), (NTSTATUS)(expect))
