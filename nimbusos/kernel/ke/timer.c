/* NimbusOS — temporizadores one-shot y periódicos con DPC. */
#include "ke.h"
#include "../host/host_compat.h"

#define KE_MAX_TIMERS 64
static KTIMER* g_Timers[KE_MAX_TIMERS];
static ULONG   g_TimerCount;

void KeInitializeTimer(PKTIMER Timer) {
    if (!Timer) return;
    nimbus_host_memset(Timer, 0, sizeof(*Timer));
}

void KeSetTimer(PKTIMER Timer, ULONGLONG DueTick, LONG PeriodMs,
                void (*dpc)(void*), void* ctx) {
    ULONG i;
    if (!Timer) return;
    Timer->DueTime = DueTick;
    Timer->PeriodMs = PeriodMs;
    Timer->Active = 1;
    Timer->DpcRoutine = dpc;
    Timer->DpcContext = ctx;
    for (i = 0; i < g_TimerCount; i++)
        if (g_Timers[i] == Timer) return;
    if (g_TimerCount < KE_MAX_TIMERS)
        g_Timers[g_TimerCount++] = Timer;
}

void KeCancelTimer(PKTIMER Timer) {
    if (Timer) Timer->Active = 0;
}

void KeTimerTick(ULONGLONG Now) {
    ULONG i;
    for (i = 0; i < g_TimerCount; i++) {
        PKTIMER t = g_Timers[i];
        if (!t || !t->Active) continue;
        if (Now >= t->DueTime) {
            if (t->DpcRoutine) t->DpcRoutine(t->DpcContext);
            if (t->PeriodMs > 0)
                t->DueTime = Now + (ULONGLONG)t->PeriodMs;
            else
                t->Active = 0;
        }
    }
}
