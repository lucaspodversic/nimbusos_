//! Mapa de extents: rangos lógicos → bloques físicos.

#[derive(Clone, Debug, Default)]
pub struct Extent {
    pub logical_off: u64,
    pub length: u64,
    pub physical_block: u64,
}

#[derive(Clone, Debug, Default)]
pub struct ExtentMap {
    pub extents: Vec<Extent>,
}

impl ExtentMap {
    pub fn new() -> Self { Self::default() }
    pub fn set(&mut self, logical_off: u64, length: u64, physical_block: u64) {
        // Simplificado: un solo extent cubriendo el archivo
        self.extents.clear();
        self.extents.push(Extent { logical_off, length, physical_block });
    }
    pub fn find(&self, logical: u64) -> Option<&Extent> {
        self.extents.iter().find(|e| logical >= e.logical_off && logical < e.logical_off + e.length)
    }
    pub fn total_length(&self) -> u64 {
        self.extents.iter().map(|e| e.length).sum()
    }
}
