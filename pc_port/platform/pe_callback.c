/*
 * Phase 6D-S — Callback registry implementation.
 */
#include "pe_callback.h"
#include <stdio.h>

static PECallback g_callback   = NULL;
static int        g_reg_count  = 0;
static int        g_init       = 0;

void PE_Callback_Init(void)
{
    g_callback  = NULL;
    g_reg_count = 0;
    g_init      = 1;
}

void PE_Callback_Reset(void)
{
    g_callback = NULL;
}

void PE_Callback_Register(PECallback callback)
{
    g_callback = callback;
    g_reg_count++;
}

PECallback PE_Callback_Get(void)
{
    return g_callback;
}

void PE_Callback_Invoke(void)
{
    if (g_callback) {
        g_callback();
    }
}

int PE_Callback_RegistrationCount(void)
{
    return g_reg_count;
}
