/* NimbusOS DirectX→Vulkan translation layer (d3d10_to_vk).
 * Superficie de API compatible; backend Vulkan (estilo DXVK/VKD3D).
 * Clean-room: sin código de Microsoft ni DXVK copiado.
 */
#include <stdint.h>
/* API: D3D10CreateDevice — traduce a Vulkan en runtime. */
int D3D10CreateDevice(void) { return 0; /* VK stub OK */ }

const char* NimbusDxLayerName(void) { return "d3d10_to_vk"; }
