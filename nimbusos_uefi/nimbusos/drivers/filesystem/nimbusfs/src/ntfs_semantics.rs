//! Semántica NTFS: ACL, ADS, reparse points.

pub type StreamName = String;
pub type Uid = u32;

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Access {
    Read,
    Write,
    Execute,
    Full,
}

#[derive(Clone, Debug)]
pub struct Ace {
    pub uid: Uid,
    pub allow: bool,
    pub mask: u8, // bit0=R bit1=W bit2=X
}

impl Ace {
    pub fn allows(&self, a: Access) -> bool {
        if !self.allow { return false; }
        match a {
            Access::Read => self.mask & 1 != 0,
            Access::Write => self.mask & 2 != 0,
            Access::Execute => self.mask & 4 != 0,
            Access::Full => self.mask & 7 == 7,
        }
    }
}

#[derive(Clone, Debug, Default)]
pub struct Acl {
    pub aces: Vec<Ace>,
}

impl Acl {
    pub fn default_user() -> Self {
        Self {
            aces: vec![
                Ace { uid: 1000, allow: true, mask: 7 }, // dueño Full
                Ace { uid: 0, allow: true, mask: 7 },    // SYSTEM
            ],
        }
    }
    pub fn check(&self, uid: Uid, a: Access) -> bool {
        for ace in &self.aces {
            if ace.uid == uid {
                return ace.allows(a);
            }
        }
        false
    }
}

#[derive(Clone, Debug)]
pub struct ReparsePoint {
    pub tag: u32,
    pub target: String, // symlink / junction / mount
}

pub const IO_REPARSE_TAG_SYMLINK: u32 = 0xA000_000C;
pub const IO_REPARSE_TAG_MOUNT_POINT: u32 = 0xA000_0003;
