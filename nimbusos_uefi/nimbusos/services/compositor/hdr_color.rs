//! Gestión HDR / color management stub.

#[derive(Clone, Copy, Debug, PartialEq)]
pub struct ColorSpace {
    pub max_nits: f32,
    pub srgb: bool,
}

impl ColorSpace {
    pub fn srgb() -> Self { Self { max_nits: 80.0, srgb: true } }
    pub fn hdr1000() -> Self { Self { max_nits: 1000.0, srgb: false } }
    pub fn tone_map(&self, linear: f32) -> f32 {
        if self.srgb { linear.clamp(0.0, 1.0) }
        else { (linear / (self.max_nits / 80.0)).clamp(0.0, 1.0) }
    }
}
