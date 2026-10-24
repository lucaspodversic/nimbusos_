//! Damage tracking: solo recomponer regiones sucias.

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Rect { pub x: i32, pub y: i32, pub w: i32, pub h: i32 }

impl Rect {
    pub fn intersects(&self, o: &Rect) -> bool {
        self.x < o.x + o.w && self.x + self.w > o.x &&
        self.y < o.y + o.h && self.y + self.h > o.y
    }
    pub fn union(&self, o: &Rect) -> Rect {
        let x1 = self.x.min(o.x);
        let y1 = self.y.min(o.y);
        let x2 = (self.x + self.w).max(o.x + o.w);
        let y2 = (self.y + self.h).max(o.y + o.h);
        Rect { x: x1, y: y1, w: x2 - x1, h: y2 - y1 }
    }
    pub fn area(&self) -> i32 { self.w.max(0) * self.h.max(0) }
}

#[derive(Default, Debug)]
pub struct DamageTracker {
    pub dirty: Vec<Rect>,
}

impl DamageTracker {
    pub fn damage(&mut self, r: Rect) {
        // Fusiona si se solapa
        for d in &mut self.dirty {
            if d.intersects(&r) {
                *d = d.union(&r);
                return;
            }
        }
        self.dirty.push(r);
    }
    pub fn take(&mut self) -> Vec<Rect> {
        std::mem::take(&mut self.dirty)
    }
}
