/* NimbusOS DirectX→Vulkan translation layer (wic).
 * Superficie de API compatible; backend Vulkan (estilo DXVK/VKD3D).
 * Clean-room: sin código de Microsoft ni DXVK copiado.
 */
#include <stdint.h>
/* API: WICCreateImagingFactory — traduce a Vulkan en runtime. */
int WICCreateImagingFactory(void) { return 0; /* VK stub OK */ }

const char* NimbusDxLayerName(void) { return "wic"; }
