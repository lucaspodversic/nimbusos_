//! NimbusFS — sistema de archivos nativo de NimbusOS (clean-room).
//!
//! Semántica compatible con NTFS: ACLs, Alternate Data Streams (ADS),
//! hard links, reparse points, journaling y snapshots.
//! Diseñado para probarse en host (`cargo test`) sin hardware.

#![allow(dead_code)]

pub mod superblock;
pub mod btree;
pub mod extents;
pub mod journal;
pub mod snapshots;
pub mod dedupe;
pub mod compression;
pub mod ntfs_semantics;
pub mod encryption;
pub mod trim;

use std::collections::HashMap;
use std::io;
use std::sync::{Arc, Mutex};

pub use superblock::{Superblock, MAGIC, NIMBUSFS_VERSION};
pub use btree::BTree;
pub use extents::{Extent, ExtentMap};
pub use journal::{Journal, JournalOp};
pub use snapshots::SnapshotStore;
pub use ntfs_semantics::{Acl, Ace, StreamName, ReparsePoint};

/// Identificador de inodo (análogo a MFT record number).
pub type InodeId = u64;

/// Inodo en memoria: metadatos + mapa de extents + streams ADS.
#[derive(Clone, Debug)]
pub struct Inode {
    pub id: InodeId,
    pub is_dir: bool,
    pub size: u64,
    pub link_count: u32,
    pub acl: Acl,
    pub extents: ExtentMap,
    /// Streams nombrados (ADS). "" = stream por defecto `$DATA`.
    pub streams: HashMap<StreamName, Vec<u8>>,
    pub reparse: Option<ReparsePoint>,
    pub children: HashMap<String, InodeId>, // solo directorios
}

impl Inode {
    pub fn file(id: InodeId) -> Self {
        Self {
            id,
            is_dir: false,
            size: 0,
            link_count: 1,
            acl: Acl::default_user(),
            extents: ExtentMap::new(),
            streams: HashMap::new(),
            reparse: None,
            children: HashMap::new(),
        }
    }
    pub fn dir(id: InodeId) -> Self {
        let mut i = Self::file(id);
        i.is_dir = true;
        i
    }
}

/// Volumen NimbusFS en memoria (host). En bare-metal usaría el block device.
pub struct Volume {
    pub sb: Superblock,
    pub inodes: HashMap<InodeId, Inode>,
    pub next_inode: InodeId,
    pub journal: Journal,
    pub snapshots: SnapshotStore,
    pub btree: BTree,
    /// Almacén de bloques crudos (simula disco).
    pub blocks: Vec<u8>,
    pub block_size: usize,
}

impl Volume {
    /// Crea un volumen vacío con tamaño `blocks * block_size` bytes.
    pub fn format(total_blocks: u64, block_size: usize) -> Self {
        assert!(block_size.is_power_of_two() && block_size >= 512);
        let mut sb = Superblock::new(total_blocks, block_size as u32);
        let mut vol = Self {
            sb: sb.clone(),
            inodes: HashMap::new(),
            next_inode: 2,
            journal: Journal::new(256),
            snapshots: SnapshotStore::new(),
            btree: BTree::new(),
            blocks: vec![0u8; (total_blocks as usize) * block_size],
            block_size,
        };
        // Inodo 1 = raíz
        let mut root = Inode::dir(1);
        root.children.insert(".".into(), 1);
        vol.inodes.insert(1, root);
        vol.btree.insert(b"/", 1);
        sb.root_inode = 1;
        vol.sb = sb;
        vol.write_superblock();
        vol
    }

    fn write_superblock(&mut self) {
        let bytes = self.sb.to_bytes();
        self.blocks[..bytes.len().min(self.block_size)]
            .copy_from_slice(&bytes[..bytes.len().min(self.block_size)]);
    }

    pub fn read_superblock(blocks: &[u8], block_size: usize) -> io::Result<Superblock> {
        Superblock::from_bytes(&blocks[..block_size.min(blocks.len())])
    }

    pub fn mkdir(&mut self, parent: InodeId, name: &str) -> io::Result<InodeId> {
        self.create(parent, name, true)
    }

    pub fn create_file(&mut self, parent: InodeId, name: &str) -> io::Result<InodeId> {
        self.create(parent, name, false)
    }

    fn create(&mut self, parent: InodeId, name: &str, is_dir: bool) -> io::Result<InodeId> {
        if name.is_empty() || name.contains('/') || name.contains('\\') {
            return Err(io::Error::new(io::ErrorKind::InvalidInput, "nombre inválido"));
        }
        let parent_inode = self.inodes.get_mut(&parent)
            .ok_or_else(|| io::Error::new(io::ErrorKind::NotFound, "padre inexistente"))?;
        if !parent_inode.is_dir {
            return Err(io::Error::new(io::ErrorKind::Other, "padre no es directorio"));
        }
        if parent_inode.children.contains_key(name) {
            return Err(io::Error::new(io::ErrorKind::AlreadyExists, "ya existe"));
        }
        let id = self.next_inode;
        self.next_inode += 1;
        parent_inode.children.insert(name.to_string(), id);
        let inode = if is_dir {
            let mut d = Inode::dir(id);
            d.children.insert(".".into(), id);
            d.children.insert("..".into(), parent);
            d
        } else {
            Inode::file(id)
        };
        self.inodes.insert(id, inode);
        self.btree.insert(name.as_bytes(), id);
        self.journal.append(if is_dir { JournalOp::MkDir { parent, name: name.into(), id } }
                            else { JournalOp::Create { parent, name: name.into(), id } });
        Ok(id)
    }

    pub fn lookup(&self, parent: InodeId, name: &str) -> Option<InodeId> {
        self.inodes.get(&parent)?.children.get(name).copied()
    }

    pub fn write_data(&mut self, id: InodeId, offset: u64, data: &[u8]) -> io::Result<usize> {
        let inode = self.inodes.get_mut(&id)
            .ok_or_else(|| io::Error::new(io::ErrorKind::NotFound, "inodo"))?;
        if inode.is_dir {
            return Err(io::Error::new(io::ErrorKind::Other, "es directorio"));
        }
        let stream = inode.streams.entry(String::new()).or_default();
        let end = offset as usize + data.len();
        if stream.len() < end {
            stream.resize(end, 0);
        }
        stream[offset as usize..end].copy_from_slice(data);
        inode.size = stream.len() as u64;
        // Actualiza extents lógicos (un extent continuo simplificado)
        inode.extents.set(0, inode.size, id); // block id = inode id como proxy
        self.journal.append(JournalOp::Write { id, offset, len: data.len() as u64 });
        Ok(data.len())
    }

    pub fn read_data(&self, id: InodeId, offset: u64, buf: &mut [u8]) -> io::Result<usize> {
        let inode = self.inodes.get(&id)
            .ok_or_else(|| io::Error::new(io::ErrorKind::NotFound, "inodo"))?;
        let stream = inode.streams.get("").map(|v| v.as_slice()).unwrap_or(&[]);
        if offset as usize >= stream.len() {
            return Ok(0);
        }
        let avail = stream.len() - offset as usize;
        let n = avail.min(buf.len());
        buf[..n].copy_from_slice(&stream[offset as usize..offset as usize + n]);
        Ok(n)
    }

    /// Escribe un Alternate Data Stream (`file:stream`).
    pub fn write_ads(&mut self, id: InodeId, stream: &str, data: &[u8]) -> io::Result<()> {
        let inode = self.inodes.get_mut(&id)
            .ok_or_else(|| io::Error::new(io::ErrorKind::NotFound, "inodo"))?;
        inode.streams.insert(stream.to_string(), data.to_vec());
        Ok(())
    }

    pub fn read_ads(&self, id: InodeId, stream: &str) -> io::Result<Vec<u8>> {
        let inode = self.inodes.get(&id)
            .ok_or_else(|| io::Error::new(io::ErrorKind::NotFound, "inodo"))?;
        inode.streams.get(stream).cloned()
            .ok_or_else(|| io::Error::new(io::ErrorKind::NotFound, "ADS"))
    }

    pub fn hard_link(&mut self, target: InodeId, parent: InodeId, name: &str) -> io::Result<()> {
        {
            let p = self.inodes.get_mut(&parent)
                .ok_or_else(|| io::Error::new(io::ErrorKind::NotFound, "padre"))?;
            if p.children.contains_key(name) {
                return Err(io::Error::new(io::ErrorKind::AlreadyExists, "existe"));
            }
            p.children.insert(name.to_string(), target);
        }
        let t = self.inodes.get_mut(&target)
            .ok_or_else(|| io::Error::new(io::ErrorKind::NotFound, "target"))?;
        t.link_count += 1;
        self.journal.append(JournalOp::HardLink { target, parent, name: name.into() });
        Ok(())
    }

    pub fn snapshot(&mut self, label: &str) -> u64 {
        let clone: HashMap<InodeId, Inode> = self.inodes.clone();
        self.snapshots.take(label, clone)
    }

    pub fn restore_snapshot(&mut self, id: u64) -> io::Result<()> {
        let snap = self.snapshots.get(id)
            .ok_or_else(|| io::Error::new(io::ErrorKind::NotFound, "snapshot"))?;
        self.inodes = snap.clone();
        Ok(())
    }
}

/// Volumen compartido thread-safe para el driver UMDF/host.
pub type SharedVolume = Arc<Mutex<Volume>>;

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn format_and_superblock_roundtrip() {
        let vol = Volume::format(64, 4096);
        assert_eq!(vol.sb.magic, MAGIC);
        let sb2 = Volume::read_superblock(&vol.blocks, 4096).unwrap();
        assert_eq!(sb2.total_blocks, 64);
        assert_eq!(sb2.root_inode, 1);
    }

    #[test]
    fn mkdir_create_write_read() {
        let mut vol = Volume::format(128, 4096);
        let dir = vol.mkdir(1, "home").unwrap();
        let f = vol.create_file(dir, "hola.txt").unwrap();
        vol.write_data(f, 0, b"NimbusOS").unwrap();
        let mut buf = [0u8; 16];
        let n = vol.read_data(f, 0, &mut buf).unwrap();
        assert_eq!(&buf[..n], b"NimbusOS");
        assert_eq!(vol.lookup(dir, "hola.txt"), Some(f));
    }

    #[test]
    fn ads_and_hardlink() {
        let mut vol = Volume::format(64, 4096);
        let f = vol.create_file(1, "doc.txt").unwrap();
        vol.write_ads(f, "Zone.Identifier", b"[ZoneTransfer]\nZoneId=3").unwrap();
        assert_eq!(vol.read_ads(f, "Zone.Identifier").unwrap(), b"[ZoneTransfer]\nZoneId=3");
        vol.hard_link(f, 1, "doc2.txt").unwrap();
        assert_eq!(vol.lookup(1, "doc2.txt"), Some(f));
        assert_eq!(vol.inodes[&f].link_count, 2);
    }

    #[test]
    fn journal_and_snapshot() {
        let mut vol = Volume::format(64, 4096);
        vol.create_file(1, "a").unwrap();
        let snap = vol.snapshot("antes");
        vol.create_file(1, "b").unwrap();
        assert!(vol.lookup(1, "b").is_some());
        vol.restore_snapshot(snap).unwrap();
        assert!(vol.lookup(1, "b").is_none());
        assert!(vol.lookup(1, "a").is_some());
        assert!(vol.journal.len() >= 1);
    }

    #[test]
    fn compression_roundtrip() {
        let raw = b"aaaaaaaaaaaaaaaaaaaaaaaaaaaaNimbusOS";
        let c = compression::compress_lz77(raw);
        let d = compression::decompress_lz77(&c).unwrap();
        assert_eq!(d, raw);
    }

    #[test]
    fn btree_insert_find() {
        let mut t = BTree::new();
        t.insert(b"zebra", 10);
        t.insert(b"apple", 1);
        t.insert(b"mango", 5);
        assert_eq!(t.get(b"apple"), Some(1));
        assert_eq!(t.get(b"nope"), None);
        let keys: Vec<_> = t.keys_sorted();
        assert_eq!(keys, vec![b"apple".to_vec(), b"mango".to_vec(), b"zebra".to_vec()]);
    }

    #[test]
    fn acl_access_check() {
        let acl = Acl::default_user();
        assert!(acl.check(1000, ntfs_semantics::Access::Read));
        assert!(!acl.check(9999, ntfs_semantics::Access::Write));
    }
}
