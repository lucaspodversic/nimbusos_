//! Network Manager.
#![allow(dead_code)]
#[path = "dhcp.rs"] mod dhcp;
#[path = "wifi_profiles.rs"] mod wifi_profiles;
#[path = "vpn.rs"] mod vpn;
#[path = "proxy.rs"] mod proxy;
pub use dhcp::*;
pub use wifi_profiles::*;
pub use vpn::*;
pub use proxy::*;

#[derive(Default, Debug)]
pub struct NetworkManager { pub online: bool }
impl NetworkManager {
    pub fn set_online(&mut self, v: bool) { self.online = v; }
}
#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn online_flag() {
        let mut nm = NetworkManager::default();
        nm.set_online(true);
        assert!(nm.online);
    }
}
