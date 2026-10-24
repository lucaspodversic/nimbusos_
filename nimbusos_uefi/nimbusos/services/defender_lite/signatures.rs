#[derive(Default, Debug)]
pub struct SignatureDb { pub sigs: Vec<Vec<u8>> }
impl SignatureDb {
    pub fn new() -> Self { Self::default() }
    pub fn add(&mut self, s: &[u8]) { self.sigs.push(s.to_vec()); }
    pub fn matches(&self, data: &[u8]) -> bool {
        self.sigs.iter().any(|s| data.windows(s.len()).any(|w| w == s.as_slice()))
    }
}
