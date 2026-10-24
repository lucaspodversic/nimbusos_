# Actualizaciones A/B

1. Descarga a ranura inactiva.
2. Verifica firma.
3. `commit_swap` al reiniciar (bootmgr `ab_select`).
4. Rollback automático si el boot falla N veces.
5. Hot-patch de servicios sin reinicio cuando el ABI lo permite.
