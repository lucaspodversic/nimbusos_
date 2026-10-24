//! Pestañas del explorador.

#[derive(Clone, Debug)]
pub struct Tab { pub path: String, pub title: String }

#[derive(Debug)]
pub struct TabStrip {
    pub tabs: Vec<Tab>,
    pub active: usize,
}

impl TabStrip {
    pub fn new() -> Self { Self { tabs: vec![], active: 0 } }
    pub fn open(&mut self, path: &str) {
        self.tabs.push(Tab { path: path.into(), title: path.into() });
        self.active = self.tabs.len() - 1;
    }
    pub fn active_mut(&mut self) -> Option<&mut Tab> {
        self.tabs.get_mut(self.active)
    }
    pub fn len(&self) -> usize { self.tabs.len() }
}
