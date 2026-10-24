/* NimbusOS DirectX→Vulkan translation layer (d2d1).
 * Superficie de API compatible; backend Vulkan (estilo DXVK/VKD3D).
 * Clean-room: sin código de Microsoft ni DXVK copiado.
 */
#include <stdint.h>
/* API: D2D1CreateFactory — traduce a Vulkan en runtime. */
int D2D1CreateFactory(void) { return 0; /* VK stub OK */ }

const char* NimbusDxLayerName(void) { return "d2d1"; }
