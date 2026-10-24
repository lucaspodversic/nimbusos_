//! Vista de archivos (lista/iconos).

#[derive(Clone, Debug)]
pub struct FileEntry {
    pub name: String,
    pub is_dir: bool,
    pub size: u64,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum ViewMode { Icons, List, Details }

#[derive(Debug)]
pub struct FileView {
    pub mode: ViewMode,
    pub entries: Vec<FileEntry>,
}

impl FileView {
    pub fn sort_by_name(&mut self) {
        self.entries.sort_by(|a, b| a.name.to_lowercase().cmp(&b.name.to_lowercase()));
    }
}
