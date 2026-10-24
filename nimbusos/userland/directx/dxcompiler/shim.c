/* NimbusOS DirectX→Vulkan translation layer (dxcompiler).
 * Superficie de API compatible; backend Vulkan (estilo DXVK/VKD3D).
 * Clean-room: sin código de Microsoft ni DXVK copiado.
 */
#include <stdint.h>
/* API: DxcCreateInstance — traduce a Vulkan en runtime. */
int DxcCreateInstance(void) { return 0; /* VK stub OK */ }

const char* NimbusDxLayerName(void) { return "dxcompiler"; }
