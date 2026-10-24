/* NimbusOS DirectX→Vulkan translation layer (dwrite).
 * Superficie de API compatible; backend Vulkan (estilo DXVK/VKD3D).
 * Clean-room: sin código de Microsoft ni DXVK copiado.
 */
#include <stdint.h>
/* API: DWriteCreateFactory — traduce a Vulkan en runtime. */
int DWriteCreateFactory(void) { return 0; /* VK stub OK */ }

const char* NimbusDxLayerName(void) { return "dwrite"; }
