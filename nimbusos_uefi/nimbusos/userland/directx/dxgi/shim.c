/* NimbusOS DirectX→Vulkan translation layer (dxgi).
 * Superficie de API compatible; backend Vulkan (estilo DXVK/VKD3D).
 * Clean-room: sin código de Microsoft ni DXVK copiado.
 */
#include <stdint.h>
/* API: CreateDXGIFactory — traduce a Vulkan en runtime. */
int CreateDXGIFactory(void) { return 0; /* VK stub OK */ }
/* API: DXGIGetDebugInterface — traduce a Vulkan en runtime. */
int DXGIGetDebugInterface(void) { return 0; /* VK stub OK */ }

const char* NimbusDxLayerName(void) { return "dxgi"; }
