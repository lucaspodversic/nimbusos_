//! Host aislado para extensiones de shell de terceros (proceso aparte).

#[derive(Default, Debug)]
pub struct ExtensionHost {
    pub loaded: Vec<String>,
}

impl ExtensionHost {
    pub fn load(&mut self, dll: &str) {
        // En prod: carga en proceso sandbox con ALPC.
        self.loaded.push(dll.into());
    }
}
