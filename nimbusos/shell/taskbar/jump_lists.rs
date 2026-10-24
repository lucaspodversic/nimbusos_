//! Jump lists por aplicación.

use std::collections::HashMap;

#[derive(Default, Debug)]
pub struct JumpLists {
    map: HashMap<String, Vec<String>>,
}

impl JumpLists {
    pub fn add_recent(&mut self, app: &str, path: &str) {
        let list = self.map.entry(app.into()).or_default();
        list.retain(|p| p != path);
        list.insert(0, path.into());
        list.truncate(15);
    }
    pub fn get(&self, app: &str) -> &[String] {
        self.map.get(app).map(|v| v.as_slice()).unwrap_or(&[])
    }
}
