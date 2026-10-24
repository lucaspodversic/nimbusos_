//! Compositor GPU (Vulkan backend stub) — presupuesto ≤ 120 MB.
#![allow(dead_code)]
#[path = "compositor_main.rs"] mod compositor_main;
#[path = "vulkan_backend.rs"] mod vulkan_backend;
#[path = "damage_tracking.rs"] mod damage_tracking;
#[path = "animations.rs"] mod animations;
#[path = "hdr_color.rs"] mod hdr_color;
#[path = "multi_monitor.rs"] mod multi_monitor;
pub use compositor_main::*;
pub use vulkan_backend::*;
pub use damage_tracking::*;
pub use animations::*;
pub use hdr_color::*;
pub use multi_monitor::*;
