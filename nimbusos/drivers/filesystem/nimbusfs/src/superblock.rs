//! Superbloque on-disk de NimbusFS.
//! Contiene magic, versión, geometría y puntero a raíz.

use std::io;

/// Magic ASCII "NIFS" (NimbusFS).
pub const MAGIC: u32 = 0x5346_494E; // 'N''I''F''S' little-endian as NIFS
pub const NIMBUSFS_VERSION: u32 = 1;

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct Superblock {
    pub magic: u32,
    pub version: u32,
    pub block_size: u32,
    pub total_blocks: u64,
    pub root_inode: u64,
    pub uuid: [u8; 16],
    pub label: [u8; 32],
}

impl Superblock {
    pub fn new(total_blocks: u64, block_size: u32) -> Self {
        Self {
            magic: MAGIC,
            version: NIMBUSFS_VERSION,
            block_size,
            total_blocks,
            root_inode: 1,
            uuid: [0x4E, 0x49, 0x4D, 0x42, 0x55, 0x53, 0x46, 0x53,
                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01],
            label: {
                let mut l = [0u8; 32];
                l[..8].copy_from_slice(b"NimbusOS");
                l
            },
        }
    }

    pub fn to_bytes(&self) -> Vec<u8> {
        let mut v = Vec::with_capacity(128);
        v.extend_from_slice(&self.magic.to_le_bytes());
        v.extend_from_slice(&self.version.to_le_bytes());
        v.extend_from_slice(&self.block_size.to_le_bytes());
        v.extend_from_slice(&self.total_blocks.to_le_bytes());
        v.extend_from_slice(&self.root_inode.to_le_bytes());
        v.extend_from_slice(&self.uuid);
        v.extend_from_slice(&self.label);
        v
    }

    pub fn from_bytes(buf: &[u8]) -> io::Result<Self> {
        if buf.len() < 76 {
            return Err(io::Error::new(io::ErrorKind::UnexpectedEof, "superblock corto"));
        }
        let magic = u32::from_le_bytes(buf[0..4].try_into().unwrap());
        if magic != MAGIC {
            return Err(io::Error::new(io::ErrorKind::InvalidData, "magic incorrecto"));
        }
        Ok(Self {
            magic,
            version: u32::from_le_bytes(buf[4..8].try_into().unwrap()),
            block_size: u32::from_le_bytes(buf[8..12].try_into().unwrap()),
            total_blocks: u64::from_le_bytes(buf[12..20].try_into().unwrap()),
            root_inode: u64::from_le_bytes(buf[20..28].try_into().unwrap()),
            uuid: buf[28..44].try_into().unwrap(),
            label: buf[44..76].try_into().unwrap(),
        })
    }
}
