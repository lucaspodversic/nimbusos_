//! Servidor de ventanas user-mode de NimbusOS.
#![allow(dead_code)]
#[path = "window_manager.rs"] pub mod window_manager;
#[path = "input_router.rs"] pub mod input_router;
#[path = "focus_z_order.rs"] pub mod focus_z_order;
#[path = "gdi_backend.rs"] pub mod gdi_backend;
#[path = "hit_testing.rs"] pub mod hit_testing;
#[path = "session_desktop.rs"] pub mod session_desktop;
#[path = "clipboard_server.rs"] pub mod clipboard_server;
#[path = "accessibility.rs"] pub mod accessibility;
