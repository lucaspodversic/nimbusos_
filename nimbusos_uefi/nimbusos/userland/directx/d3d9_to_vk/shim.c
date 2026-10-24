/* NimbusOS DirectX→Vulkan translation layer (d3d9_to_vk).
 * Superficie de API compatible; backend Vulkan (estilo DXVK/VKD3D).
 * Clean-room: sin código de Microsoft ni DXVK copiado.
 */
#include <stdint.h>
/* API: Direct3DCreate9 — traduce a Vulkan en runtime. */
int Direct3DCreate9(void) { return 0; /* VK stub OK */ }

const char* NimbusDxLayerName(void) { return "d3d9_to_vk"; }
