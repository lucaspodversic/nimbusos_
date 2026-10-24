//! Shell de NimbusOS — presupuesto ≤ 70 MB, sin WebView/XAML.
#![allow(dead_code)]

pub mod taskbar {
    #[path = "taskbar.rs"]
    pub mod taskbar_impl;
    #[path = "tray.rs"]
    pub mod tray;
    #[path = "notifications.rs"]
    pub mod notifications;
    #[path = "jump_lists.rs"]
    pub mod jump_lists;
    pub use taskbar_impl::*;
    pub use tray::*;
    pub use notifications::*;
    pub use jump_lists::*;
}
pub mod start_menu {
    #[path = "start.rs"]
    pub mod start;
    #[path = "app_index.rs"]
    pub mod app_index;
    #[path = "search_ui.rs"]
    pub mod search_ui;
    pub use start::*;
    pub use app_index::*;
    pub use search_ui::*;
}
pub mod explorer {
    #[path = "explorer_main.rs"]
    pub mod explorer_main;
    #[path = "file_view.rs"]
    pub mod file_view;
    #[path = "nav_pane.rs"]
    pub mod nav_pane;
    #[path = "context_menu.rs"]
    pub mod context_menu;
    #[path = "shell_extensions_host.rs"]
    pub mod shell_extensions_host;
    #[path = "thumbnails.rs"]
    pub mod thumbnails;
    #[path = "tabs.rs"]
    pub mod tabs;
    pub use explorer_main::*;
    pub use file_view::*;
    pub use nav_pane::*;
    pub use context_menu::*;
    pub use tabs::*;
}
pub mod settings_app {
    #[path = "system.rs"]
    pub mod system;
    #[path = "display.rs"]
    pub mod display;
    #[path = "network.rs"]
    pub mod network;
    #[path = "apps.rs"]
    pub mod apps;
    #[path = "accounts.rs"]
    pub mod accounts;
    #[path = "privacy.rs"]
    pub mod privacy;
    #[path = "update.rs"]
    pub mod update;
    #[path = "memory_monitor.rs"]
    pub mod memory_monitor;
    pub use system::*;
    pub use memory_monitor::*;
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn taskbar_pins() {
        let mut tb = taskbar::Taskbar::new();
        tb.pin("explorer");
        tb.pin("settings");
        assert_eq!(tb.pinned.len(), 2);
    }
    #[test]
    fn start_search() {
        let idx = start_menu::AppIndex::seed();
        let r = idx.search("settings");
        assert!(r.iter().any(|a| a.app_id == "settings"));
    }
    #[test]
    fn explorer_nav() {
        let mut ex = explorer::Explorer::new();
        ex.navigate("C:\\Users");
        assert_eq!(ex.cwd, "C:\\Users");
        assert_eq!(ex.tabs.len(), 1);
    }
}
