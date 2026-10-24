//! Bucle del compositor: damage → blit → present.

use crate::damage_tracking::{DamageTracker, Rect};
use crate::multi_monitor::{Monitor, MonitorLayout};
use crate::vulkan_backend::VkSurface;

pub const MEMORY_BUDGET_BYTES: usize = 120 * 1024 * 1024;

#[derive(Debug)]
pub struct Compositor {
    pub surface: VkSurface,
    pub damage: DamageTracker,
    pub monitors: MonitorLayout,
    pub frame: u64,
}

impl Compositor {
    pub fn new(w: u32, h: u32) -> Self {
        let mut monitors = MonitorLayout::default();
        monitors.add(Monitor { id: 0, x: 0, y: 0, width: w, height: h, primary: true, scale_dpi: 96 });
        Self {
            surface: VkSurface::new(w, h),
            damage: DamageTracker::default(),
            monitors,
            frame: 0,
        }
    }

    pub fn mark_dirty(&mut self, r: Rect) { self.damage.damage(r); }

    pub fn compose_solid(&mut self, r: Rect, color: u32) {
        self.mark_dirty(r);
        let src: Vec<u32> = vec![color; (r.w.max(0) * r.h.max(0)) as usize];
        self.surface.blit(r.x.max(0) as u32, r.y.max(0) as u32,
                          r.w.max(0) as u32, r.h.max(0) as u32,
                          &src, r.w.max(0) as u32);
    }

    pub fn present(&mut self) -> usize {
        let regions = self.damage.take();
        self.frame += 1;
        regions.len()
    }

    pub fn within_budget(&self) -> bool {
        self.surface.memory_bytes() < MEMORY_BUDGET_BYTES
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn compose_and_budget() {
        let mut c = Compositor::new(800, 600);
        c.compose_solid(Rect { x: 10, y: 10, w: 100, h: 50 }, 0xFF0080FF);
        assert_eq!(c.present(), 1);
        assert!(c.within_budget());
        assert_eq!(c.surface.pixels[(10 * 800 + 10) as usize], 0xFF0080FF);
    }
}
