/* NimbusOS — Kernel Executive: planificador y sincronización. */
#pragma once
#include "ntdef.h"

#ifdef __cplusplus
extern "C" {
#endif

struct _KTHREAD;
struct _KPROCESS;

typedef enum _KTHREAD_STATE {
    ThreadStateInitialized = 0,
    ThreadStateReady,
    ThreadStateRunning,
    ThreadStateWaiting,
    ThreadStateTerminated
} KTHREAD_STATE;

typedef struct _KTHREAD {
    ULONG               ThreadId;
    KTHREAD_STATE       State;
    LONG                Priority;       /* 0..31, mayor = más prioritario */
    ULONG               AffinityMask;
    struct _KPROCESS*   Process;
    void              (*StartRoutine)(void* ctx);
    void*               StartContext;
    struct _KTHREAD*    ReadyNext;
    struct _KWAIT_BLOCK* WaitBlockList;
    ULONG               WaitStatus;
} KTHREAD, *PKTHREAD;

typedef struct _KPROCESS {
    ULONG       ProcessId;
    CHAR        ImageName[16];
    LONG        BasePriority;
    ULONG       AffinityMask;
    PKTHREAD    ThreadList;   /* lista simple enlazada por ReadyNext reuse: usamos lista aparte */
    ULONG       ThreadCount;
} KPROCESS, *PKPROCESS;

/* Cola de listos por prioridad (32 niveles). */
#define KE_PRIORITY_LEVELS 32

typedef struct _KREADY_QUEUE {
    PKTHREAD Heads[KE_PRIORITY_LEVELS];
    PKTHREAD Tails[KE_PRIORITY_LEVELS];
    ULONG    Count;
    LONG     HighestReady; /* -1 si vacía */
} KREADY_QUEUE;

/* Tiempo de interrupción (host: simulado). */
ULONG64  KeQueryInterruptTime(void);
void     KeAdvanceTick(ULONG64 delta);

NTSTATUS KeInitialize(void);
void     KeShutdown(void);

PKTHREAD KeGetCurrentThread(void);
void     KeSetCurrentThread(PKTHREAD t);

void     KeReadyQueueInit(KREADY_QUEUE* q);
void     KeReadyEnqueue(KREADY_QUEUE* q, PKTHREAD t);
PKTHREAD KeReadyDequeueHighest(KREADY_QUEUE* q);
ULONG    KeReadyCount(const KREADY_QUEUE* q);

/* Planificador cooperativo en host: selecciona el siguiente hilo READY. */
PKTHREAD KeSchedule(void);
void     KeYield(void);
NTSTATUS KeStartThread(PKTHREAD t, void (*fn)(void*), void* ctx);
void     KeTerminateThread(PKTHREAD t);

/* Sync */
typedef struct _KSPIN_LOCK {
    volatile LONG Lock;
} KSPIN_LOCK, *PKSPIN_LOCK;

void KeInitializeSpinLock(PKSPIN_LOCK Lock);
void KeAcquireSpinLock(PKSPIN_LOCK Lock);
void KeReleaseSpinLock(PKSPIN_LOCK Lock);

typedef struct _KEVENT {
    volatile LONG Signaled;
    LONG          ManualReset; /* 1 = notification, 0 = sync/auto */
    CHAR          Name[32];
} KEVENT, *PKEVENT;

void     KeInitializeEvent(PKEVENT Event, LONG ManualReset, LONG InitialState);
LONG     KeSetEvent(PKEVENT Event);
LONG     KeResetEvent(PKEVENT Event);
LONG     KeReadStateEvent(PKEVENT Event);
NTSTATUS KeWaitForSingleObject(PKEVENT Event, ULONG TimeoutMs);

typedef struct _KMUTEX {
    volatile LONG OwnerThreadId; /* 0 = libre */
    LONG          RecursionCount;
    KEVENT        Available;
} KMUTEX, *PKMUTEX;

void     KeInitializeMutex(PKMUTEX Mutex);
NTSTATUS KeAcquireMutex(PKMUTEX Mutex, ULONG TimeoutMs);
void     KeReleaseMutex(PKMUTEX Mutex);

typedef struct _KSEMAPHORE {
    volatile LONG Count;
    LONG          Limit;
    KEVENT        Available;
} KSEMAPHORE, *PKSEMAPHORE;

void     KeInitializeSemaphore(PKSEMAPHORE Sem, LONG Count, LONG Limit);
NTSTATUS KeWaitForSemaphore(PKSEMAPHORE Sem, ULONG TimeoutMs);
LONG     KeReleaseSemaphore(PKSEMAPHORE Sem, LONG Adjustment);

typedef struct _KTIMER {
    ULONGLONG DueTime; /* tick absoluto */
    LONG      PeriodMs;
    LONG      Active;
    void    (*DpcRoutine)(void* ctx);
    void*     DpcContext;
} KTIMER, *PKTIMER;

void KeInitializeTimer(PKTIMER Timer);
void KeSetTimer(PKTIMER Timer, ULONGLONG DueTick, LONG PeriodMs, void (*dpc)(void*), void* ctx);
void KeCancelTimer(PKTIMER Timer);
void KeTimerTick(ULONGLONG Now);

/* DPC / APC simplificados */
typedef struct _KDPC {
    void (*DeferredRoutine)(void* ctx);
    void* DeferredContext;
    LONG  Queued;
} KDPC, *PKDPC;

void KeInitializeDpc(PKDPC Dpc, void (*fn)(void*), void* ctx);
void KeInsertQueueDpc(PKDPC Dpc);
void KeFlushDpcQueue(void);

typedef struct _KAPC {
    void (*KernelRoutine)(void* ctx);
    void* NormalContext;
    LONG  Inserted;
} KAPC, *PKAPC;

void KeInitializeApc(PKAPC Apc, void (*fn)(void*), void* ctx);
void KeInsertQueueApc(PKAPC Apc);
void KeDeliverApcs(void);

NTSTATUS KeWaitMultiple(PKEVENT* Events, ULONG Count, LONG WaitAll, ULONG TimeoutMs);

void KeBugCheck(ULONG Code, ULONG_PTR a1, ULONG_PTR a2, ULONG_PTR a3, ULONG_PTR a4);
void KeDispatchException(ULONG Code, void* Address);

/* Syscall table */
typedef NTSTATUS (*PSYSCALL_HANDLER)(ULONG_PTR a1, ULONG_PTR a2, ULONG_PTR a3, ULONG_PTR a4);
#define NIMBUS_SYSCALL_MAX 256
void     KeSyscallTableInit(void);
void     KeRegisterSyscall(ULONG Number, PSYSCALL_HANDLER Handler);
NTSTATUS KeInvokeSyscall(ULONG Number, ULONG_PTR a1, ULONG_PTR a2, ULONG_PTR a3, ULONG_PTR a4);

/* Efficiency cores (hint de afinidad) */
void KeSetEfficiencyPreference(PKTHREAD t, LONG PreferEfficient);

/* Interno/tests: cola global de listos. */
KREADY_QUEUE* KeGetReadyQueue(void);

#ifdef __cplusplus
}
#endif
