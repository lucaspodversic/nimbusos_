//! Multi-monitor: lista de salidas y layout.

#[derive(Clone, Debug)]
pub struct Monitor {
    pub id: u32,
    pub x: i32,
    pub y: i32,
    pub width: u32,
    pub height: u32,
    pub primary: bool,
    pub scale_dpi: u32,
}

#[derive(Default, Debug)]
pub struct MonitorLayout {
    pub monitors: Vec<Monitor>,
}

impl MonitorLayout {
    pub fn add(&mut self, m: Monitor) { self.monitors.push(m); }
    pub fn primary(&self) -> Option<&Monitor> {
        self.monitors.iter().find(|m| m.primary).or_else(|| self.monitors.first())
    }
    pub fn at_point(&self, x: i32, y: i32) -> Option<&Monitor> {
        self.monitors.iter().find(|m| {
            x >= m.x && y >= m.y &&
            x < m.x + m.width as i32 && y < m.y + m.height as i32
        })
    }
}
