//! Instalador MSI/MSIX.
#![allow(dead_code)]
pub mod msi_engine {
    #[path = "msi_parser.rs"]
    pub mod msi_parser;
    #[path = "msi_transactions.rs"]
    pub mod msi_transactions;
    #[path = "custom_actions.rs"]
    pub mod custom_actions;
    #[path = "msi_rollback.rs"]
    pub mod msi_rollback;
    pub use msi_parser::*;
}
pub mod msix_engine {
    #[path = "package_reader.rs"]
    pub mod package_reader;
    #[path = "manifest.rs"]
    pub mod manifest;
    #[path = "containerization.rs"]
    pub mod containerization;
    #[path = "dependencies.rs"]
    pub mod dependencies;
    pub use package_reader::*;
    pub use manifest::*;
}
#[path = "legacy_exe_monitor.rs"]
mod legacy_exe_monitor;
#[path = "uninstall_tracker.rs"]
mod uninstall_tracker;
pub use legacy_exe_monitor::*;
pub use uninstall_tracker::*;
