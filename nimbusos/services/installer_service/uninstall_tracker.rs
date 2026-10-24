//! Registro de desinstalación.
use std::collections::HashMap;
#[derive(Default, Debug)]
pub struct UninstallDb { pub entries: HashMap<String, String> }
impl UninstallDb {
  pub fn register(&mut self, product: &str, uninstall_cmd: &str) {
    self.entries.insert(product.into(), uninstall_cmd.into());
  }
}
