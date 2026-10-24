//! Perfiles WiFi.
use std::collections::HashMap;
#[derive(Default, Debug)]
pub struct WifiProfiles { pub map: HashMap<String,String> }
impl WifiProfiles {
  pub fn add(&mut self, ssid: &str, psk: &str) { self.map.insert(ssid.into(), psk.into()); }
}
