# Modelo de drivers

- Por defecto **UMDF-style** (modo usuario) con IOMMU.
- En kernel solo storage/GPU/NIC/USB host.
- IRP model compatible a nivel semántico (`IRP_MJ_*`).
- Virtio preferido en QEMU.
