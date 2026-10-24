//! Backend GDI software (bitmap en memoria).

#[derive(Clone, Debug)]
pub struct SoftDc {
    pub width: u32,
    pub height: u32,
    pub pixels: Vec<u32>,
}

impl SoftDc {
    pub fn new(w: u32, h: u32) -> Self {
        Self { width: w, height: h, pixels: vec![0xFFFFFFFF; (w*h) as usize] }
    }
    pub fn fill_rect(&mut self, x: i32, y: i32, w: i32, h: i32, color: u32) {
        for row in y.max(0)..(y+h).min(self.height as i32) {
            for col in x.max(0)..(x+w).min(self.width as i32) {
                self.pixels[(row as u32 * self.width + col as u32) as usize] = color;
            }
        }
    }
    pub fn set_pixel(&mut self, x: i32, y: i32, color: u32) {
        if x >= 0 && y >= 0 && (x as u32) < self.width && (y as u32) < self.height {
            self.pixels[(y as u32 * self.width + x as u32) as usize] = color;
        }
    }
}
