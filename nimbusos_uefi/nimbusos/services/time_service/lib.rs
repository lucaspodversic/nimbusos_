//! SNTP/hora.
#![allow(dead_code)]
#[derive(Default, Debug)]
pub struct Service { pub running: bool }
impl Service {
    pub fn start(&mut self) { self.running = true; }
    pub fn stop(&mut self) { self.running = false; }
}
#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn start_stop() {
        let mut s = Service::default();
        s.start();
        assert!(s.running);
        s.stop();
        assert!(!s.running);
    }
}
