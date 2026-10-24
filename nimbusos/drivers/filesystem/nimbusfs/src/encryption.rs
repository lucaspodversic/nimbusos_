//! Cifrado de extents (XTS-AES placeholder: XOR con keystream derivado).
//! En producción: AES-XTS vía crypto_service. Aquí un PRNG determinista para tests.

pub fn derive_keystream(key: &[u8; 32], block_index: u64, len: usize) -> Vec<u8> {
    let mut out = vec![0u8; len];
    let mut state = u64::from_le_bytes(key[0..8].try_into().unwrap()) ^ block_index;
    for i in 0..len {
        state = state.wrapping_mul(6364136223846793005).wrapping_add(1);
        out[i] = (state >> 33) as u8;
    }
    out
}

pub fn encrypt_block(key: &[u8; 32], block_index: u64, plain: &[u8]) -> Vec<u8> {
    let ks = derive_keystream(key, block_index, plain.len());
    plain.iter().zip(ks).map(|(a, b)| a ^ b).collect()
}

pub fn decrypt_block(key: &[u8; 32], block_index: u64, cipher: &[u8]) -> Vec<u8> {
    encrypt_block(key, block_index, cipher) // XOR es involutivo
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn roundtrip() {
        let key = [0x42u8; 32];
        let p = b"datos secretos de NimbusOS!!!!";
        let c = encrypt_block(&key, 7, p);
        assert_ne!(&c[..], &p[..]);
        let d = decrypt_block(&key, 7, &c);
        assert_eq!(d, p);
    }
}
