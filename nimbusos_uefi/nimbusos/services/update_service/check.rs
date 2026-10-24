//! Comprueba actualizaciones firmadas.
#[derive(Debug)]
pub struct UpdateInfo { pub version: String, pub size: u64 }
pub fn check_for_updates() -> Option<UpdateInfo> { None }
