/* Test: SeAccessCheck con DENY antes que ALLOW. */
#include "test_common.h"
#include "se.h"

int main(void) {
    SID user, other, admins;
    PTOKEN token = NULL;
    ACL acl;
    ACCESS_MASK granted = 0;
    NTSTATUS st;

    EXPECT_STATUS(KiSystemStartup(), STATUS_SUCCESS);

    SeInitSid(&user, 5, 1001);
    SeInitSid(&other, 5, 1002);
    SeInitSid(&admins, 5, 512);

    EXPECT_STATUS(SeCreateToken(&token, &user, 2), STATUS_SUCCESS);
    EXPECT_STATUS(SeAddGroup(token, &admins), STATUS_SUCCESS);

    SeInitAcl(&acl);
    /* Deny WRITE al user, Allow READ|WRITE a admins — user está en admins,
     * pero DENY del user debe ganar sobre el allow del grupo para WRITE. */
    EXPECT_STATUS(SeAddAccessDeniedAce(&acl, &user, FILE_WRITE_DATA), STATUS_SUCCESS);
    EXPECT_STATUS(SeAddAccessAllowedAce(&acl, &admins, FILE_READ_DATA | FILE_WRITE_DATA), STATUS_SUCCESS);

    st = SeAccessCheck(&acl, token, FILE_WRITE_DATA, &granted);
    EXPECT_STATUS(st, STATUS_ACCESS_DENIED);

    st = SeAccessCheck(&acl, token, FILE_READ_DATA, &granted);
    EXPECT_STATUS(st, STATUS_SUCCESS);
    EXPECT_EQ(granted, FILE_READ_DATA);

    /* Other user: sin ACE → denied */
    {
        PTOKEN t2 = NULL;
        SeCreateToken(&t2, &other, 2);
        st = SeAccessCheck(&acl, t2, FILE_READ_DATA, &granted);
        EXPECT_STATUS(st, STATUS_ACCESS_DENIED);
        SeDereferenceToken(t2);
    }

    SeDereferenceToken(token);
    KiSystemShutdown();
    if (g_failures) { fprintf(stderr, "%d failures\n", g_failures); return 1; }
    printf("test_acl OK\n");
    return 0;
}
