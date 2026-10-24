/* NimbusOS — planificador por prioridad con colas de listos (clean-room).
 *
 * Diseño:
 *  - 32 niveles de prioridad (0..31), mayor número = más prioritario.
 *  - Cada nivel es una cola FIFO (Heads/Tails).
 *  - KeSchedule elige el hilo READY de mayor prioridad.
 *  - En host el "cambio de contexto" es cooperativo: el test llama a
 *    StartRoutine directamente; en bare-metal context_switch.asm haría el swap.
 */
#include "ke.h"
#include "../host/host_compat.h"

static KREADY_QUEUE g_Ready;
static PKTHREAD     g_Current;
static ULONG        g_NextThreadId = 1;
static ULONG64      g_InterruptTime;
static LONG         g_Initialized;

/* ---- tiempo ---- */
ULONG64 KeQueryInterruptTime(void) { return g_InterruptTime; }
void KeAdvanceTick(ULONG64 delta) { g_InterruptTime += delta; }

PKTHREAD KeGetCurrentThread(void) { return g_Current; }
void KeSetCurrentThread(PKTHREAD t) { g_Current = t; }

void KeReadyQueueInit(KREADY_QUEUE* q) {
    ULONG i;
    if (!q) return;
    nimbus_host_memset(q, 0, sizeof(*q));
    q->HighestReady = -1;
    for (i = 0; i < KE_PRIORITY_LEVELS; i++) {
        q->Heads[i] = NULL;
        q->Tails[i] = NULL;
    }
}

void KeReadyEnqueue(KREADY_QUEUE* q, PKTHREAD t) {
    LONG p;
    if (!q || !t) return;
    p = t->Priority;
    if (p < 0) p = 0;
    if (p >= KE_PRIORITY_LEVELS) p = KE_PRIORITY_LEVELS - 1;
    t->Priority = p;
    t->State = ThreadStateReady;
    t->ReadyNext = NULL;
    if (!q->Heads[p]) {
        q->Heads[p] = q->Tails[p] = t;
    } else {
        q->Tails[p]->ReadyNext = t;
        q->Tails[p] = t;
    }
    q->Count++;
    if (p > q->HighestReady) q->HighestReady = p;
}

PKTHREAD KeReadyDequeueHighest(KREADY_QUEUE* q) {
    LONG p;
    PKTHREAD t;
    if (!q || q->Count == 0) return NULL;
    for (p = q->HighestReady; p >= 0; p--) {
        if (!q->Heads[p]) continue;
        t = q->Heads[p];
        q->Heads[p] = t->ReadyNext;
        if (!q->Heads[p]) q->Tails[p] = NULL;
        t->ReadyNext = NULL;
        q->Count--;
        /* actualizar HighestReady */
        while (q->HighestReady >= 0 && !q->Heads[q->HighestReady])
            q->HighestReady--;
        return t;
    }
    q->HighestReady = -1;
    return NULL;
}

ULONG KeReadyCount(const KREADY_QUEUE* q) {
    return q ? q->Count : 0;
}

NTSTATUS KeInitialize(void) {
    if (g_Initialized) return STATUS_SUCCESS;
    KeReadyQueueInit(&g_Ready);
    g_Current = NULL;
    g_NextThreadId = 1;
    g_InterruptTime = 0;
    KeSyscallTableInit();
    g_Initialized = 1;
    return STATUS_SUCCESS;
}

void KeShutdown(void) {
    KeReadyQueueInit(&g_Ready);
    g_Current = NULL;
    g_Initialized = 0;
}

PKTHREAD KeSchedule(void) {
    PKTHREAD next = KeReadyDequeueHighest(&g_Ready);
    if (!next) return g_Current;
    if (g_Current && g_Current->State == ThreadStateRunning) {
        /* preempted → vuelve a ready si no terminó */
        if (g_Current->State != ThreadStateTerminated)
            KeReadyEnqueue(&g_Ready, g_Current);
    }
    g_Current = next;
    next->State = ThreadStateRunning;
    return next;
}

void KeYield(void) {
    PKTHREAD cur = g_Current;
    if (cur && cur->State == ThreadStateRunning) {
        KeReadyEnqueue(&g_Ready, cur);
    }
    KeSchedule();
}

NTSTATUS KeStartThread(PKTHREAD t, void (*fn)(void*), void* ctx) {
    if (!t || !fn) return STATUS_INVALID_PARAMETER;
    if (t->ThreadId == 0)
        t->ThreadId = g_NextThreadId++;
    t->StartRoutine = fn;
    t->StartContext = ctx;
    t->State = ThreadStateReady;
    if (t->Priority <= 0) t->Priority = 8;
    KeReadyEnqueue(&g_Ready, t);
    return STATUS_SUCCESS;
}

void KeTerminateThread(PKTHREAD t) {
    if (!t) return;
    t->State = ThreadStateTerminated;
    if (g_Current == t) g_Current = NULL;
}

/* Acceso interno a la cola global para tests. */
KREADY_QUEUE* KeGetReadyQueue(void) { return &g_Ready; }
