//! Backend Vulkan (host: software framebuffer stub).

#[derive(Clone, Debug)]
pub struct VkSurface {
    pub width: u32,
    pub height: u32,
    pub pixels: Vec<u32>, // ARGB8888
}

impl VkSurface {
    pub fn new(width: u32, height: u32) -> Self {
        Self { width, height, pixels: vec![0; (width * height) as usize] }
    }
    pub fn clear(&mut self, color: u32) {
        self.pixels.fill(color);
    }
    pub fn blit(&mut self, x: u32, y: u32, w: u32, h: u32, src: &[u32], src_stride: u32) {
        for row in 0..h {
            let dy = y + row;
            if dy >= self.height { break; }
            for col in 0..w {
                let dx = x + col;
                if dx >= self.width { break; }
                let si = (row * src_stride + col) as usize;
                let di = (dy * self.width + dx) as usize;
                if si < src.len() {
                    self.pixels[di] = src[si];
                }
            }
        }
    }
    pub fn memory_bytes(&self) -> usize {
        self.pixels.len() * 4
    }
}
