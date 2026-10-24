//! Curvas de animación ligeras (sin WebView).

pub fn ease_out_cubic(t: f32) -> f32 {
    let t = t.clamp(0.0, 1.0);
    1.0 - (1.0 - t).powi(3)
}

pub fn lerp(a: f32, b: f32, t: f32) -> f32 {
    a + (b - a) * t.clamp(0.0, 1.0)
}

#[derive(Debug)]
pub struct Animation {
    pub from: f32,
    pub to: f32,
    pub duration_ms: u32,
    pub elapsed_ms: u32,
}

impl Animation {
    pub fn value(&self) -> f32 {
        let t = self.elapsed_ms as f32 / self.duration_ms.max(1) as f32;
        lerp(self.from, self.to, ease_out_cubic(t))
    }
    pub fn done(&self) -> bool { self.elapsed_ms >= self.duration_ms }
}
