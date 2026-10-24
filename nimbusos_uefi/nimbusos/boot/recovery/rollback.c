/* NimbusOS — rollback A/B. */
int BootRollbackToSlot(int slot) { return slot == 0 || slot == 1 ? 0 : -1; }
