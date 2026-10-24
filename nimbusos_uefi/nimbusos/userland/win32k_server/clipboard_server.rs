//! Portapapeles del servidor de ventanas.

use std::collections::HashMap;

#[derive(Default, Debug)]
pub struct Clipboard {
    formats: HashMap<u32, Vec<u8>>, // CF_* → data
    pub sequence: u32,
}

impl Clipboard {
    pub const CF_TEXT: u32 = 1;
    pub const CF_UNICODETEXT: u32 = 13;

    pub fn set(&mut self, format: u32, data: Vec<u8>) {
        self.formats.insert(format, data);
        self.sequence += 1;
    }
    pub fn get(&self, format: u32) -> Option<&[u8]> {
        self.formats.get(&format).map(|v| v.as_slice())
    }
    pub fn empty(&mut self) {
        self.formats.clear();
        self.sequence += 1;
    }
}
