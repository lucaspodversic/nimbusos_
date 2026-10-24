//! Eventos de presión de memoria.

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum PressureLevel { Normal, Moderate, Critical }

pub fn classify(used_mb: u32, ceiling_mb: u32) -> PressureLevel {
    let pct = used_mb as f32 / ceiling_mb.max(1) as f32;
    if pct >= 0.95 { PressureLevel::Critical }
    else if pct >= 0.80 { PressureLevel::Moderate }
    else { PressureLevel::Normal }
}
