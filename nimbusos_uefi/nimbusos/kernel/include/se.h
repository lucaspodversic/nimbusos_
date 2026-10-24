/* NimbusOS — Security Reference Monitor (tokens, SIDs, ACLs). */
#pragma once
#include "ntdef.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SE_MAX_SUBAUTHORITIES 8
#define SE_MAX_GROUPS         16
#define SE_MAX_ACES           32

typedef struct _SID {
    UCHAR Revision;          /* siempre 1 */
    UCHAR SubAuthorityCount;
    UCHAR IdentifierAuthority[6];
    ULONG SubAuthority[SE_MAX_SUBAUTHORITIES];
} SID, *PSID;

typedef enum _ACE_TYPE {
    AceTypeAllowed = 0,
    AceTypeDenied  = 1
} ACE_TYPE;

typedef struct _ACE {
    ACE_TYPE    Type;
    ACCESS_MASK Mask;
    SID         Sid;
} ACE, *PACE;

typedef struct _ACL {
    ULONG AceCount;
    ACE   Aces[SE_MAX_ACES];
} ACL, *PACL;

typedef struct _TOKEN {
    SID         User;
    ULONG       GroupCount;
    SID         Groups[SE_MAX_GROUPS];
    ULONG       Privileges;      /* bitmask de privilegios */
    ULONG       IntegrityLevel;  /* 0=untrusted .. 4=system */
    LONG        IsAppContainer;
    SID         AppContainerSid;
    volatile LONG RefCount;
} TOKEN, *PTOKEN;

/* Máscaras de acceso documentadas (subconjunto). */
#define GENERIC_READ           0x80000000u
#define GENERIC_WRITE          0x40000000u
#define GENERIC_EXECUTE        0x20000000u
#define GENERIC_ALL            0x10000000u
#define DELETE                 0x00010000u
#define READ_CONTROL           0x00020000u
#define WRITE_DAC              0x00040000u
#define WRITE_OWNER            0x00080000u
#define SYNCHRONIZE            0x00100000u
#define FILE_READ_DATA         0x0001u
#define FILE_WRITE_DATA        0x0002u
#define FILE_EXECUTE           0x0020u
#define KEY_READ               0x20019u
#define KEY_WRITE              0x20006u

NTSTATUS SeInitialize(void);
void     SeShutdown(void);

void     SeInitSid(PSID Sid, ULONG Authority, ULONG SubAuth0);
int      SeEqualSid(const SID* A, const SID* B);
void     SeCopySid(PSID Dst, const SID* Src);
const char* SeFormatSid(const SID* Sid, char* Buf, SIZE_T Cap);

NTSTATUS SeCreateToken(PTOKEN* Out, const SID* User, ULONG Integrity);
void     SeReferenceToken(PTOKEN Token);
void     SeDereferenceToken(PTOKEN Token);
NTSTATUS SeAddGroup(PTOKEN Token, const SID* Group);
NTSTATUS SeEnablePrivilege(PTOKEN Token, ULONG PrivBit);

void     SeInitAcl(PACL Acl);
NTSTATUS SeAddAccessAllowedAce(PACL Acl, const SID* Sid, ACCESS_MASK Mask);
NTSTATUS SeAddAccessDeniedAce(PACL Acl, const SID* Sid, ACCESS_MASK Mask);

/* access_check: denegaciones primero, luego allows (modelo NT documentado). */
NTSTATUS SeAccessCheck(const ACL* Acl, const TOKEN* Token, ACCESS_MASK Desired, ACCESS_MASK* Granted);

NTSTATUS SeCreateAppContainerToken(PTOKEN* Out, const SID* Capability);
ULONG    SeGetIntegrityLevel(const TOKEN* Token);
void     SeAuditEvent(const char* Action, const SID* User, NTSTATUS Result);

/* Privilegios */
#define SE_PRIV_DEBUG          (1u << 0)
#define SE_PRIV_BACKUP         (1u << 1)
#define SE_PRIV_RESTORE        (1u << 2)
#define SE_PRIV_SHUTDOWN       (1u << 3)
#define SE_PRIV_LOAD_DRIVER    (1u << 4)

#ifdef __cplusplus
}
#endif
