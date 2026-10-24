//! Escritorios y estaciones de ventana (WinSta0\Default).

use std::collections::HashMap;
use crate::window_manager::Hwnd;

#[derive(Default, Debug)]
pub struct Desktop {
    pub name: String,
    pub windows: Vec<Hwnd>,
}

#[derive(Default, Debug)]
pub struct Session {
    pub id: u32,
    pub desktops: HashMap<String, Desktop>,
    pub active: String,
}

impl Session {
    pub fn new(id: u32) -> Self {
        let mut s = Self { id, desktops: HashMap::new(), active: "Default".into() };
        s.desktops.insert("Default".into(), Desktop { name: "Default".into(), windows: vec![] });
        s
    }
    pub fn active_desktop(&mut self) -> &mut Desktop {
        let name = self.active.clone();
        self.desktops.entry(name.clone()).or_insert_with(|| Desktop { name, windows: vec![] })
    }
}
