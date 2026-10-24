//! Compresión LZ77 simple (estilo LZ4 lite) para extents transparentes.

/// Comprime: [u32 le size][bloques: 0=lit(u8 len + bytes) | 1=match(u16 off, u8 len)]
pub fn compress_lz77(input: &[u8]) -> Vec<u8> {
    let mut out = Vec::with_capacity(input.len() + 8);
    out.extend_from_slice(&(input.len() as u32).to_le_bytes());
    let mut i = 0;
    while i < input.len() {
        let mut best = (0usize, 0usize); // (dist, len)
        let start = i.saturating_sub(4095);
        for j in start..i {
            let mut l = 0;
            while i + l < input.len() && input[j + l] == input[i + l] && l < 255 {
                l += 1;
                if j + l >= i { break; }
            }
            if l >= 4 && l > best.1 {
                best = (i - j, l);
            }
        }
        if best.1 >= 4 {
            out.push(1);
            out.extend_from_slice(&(best.0 as u16).to_le_bytes());
            out.push(best.1 as u8);
            i += best.1;
        } else {
            // literal run
            let lit_start = i;
            i += 1;
            while i < input.len() && i - lit_start < 255 {
                // peek if match ahead
                let mut has = false;
                let s = i.saturating_sub(4095);
                for j in s..i {
                    let mut l = 0;
                    while i + l < input.len() && j + l < i && input[j + l] == input[i + l] && l < 255 {
                        l += 1;
                    }
                    if l >= 4 { has = true; break; }
                }
                if has { break; }
                i += 1;
            }
            let lit = &input[lit_start..i];
            out.push(0);
            out.push(lit.len() as u8);
            out.extend_from_slice(lit);
        }
    }
    out
}

pub fn decompress_lz77(input: &[u8]) -> Result<Vec<u8>, &'static str> {
    if input.len() < 4 { return Err("corto"); }
    let size = u32::from_le_bytes(input[0..4].try_into().unwrap()) as usize;
    let mut out = Vec::with_capacity(size);
    let mut p = 4;
    while out.len() < size && p < input.len() {
        let tag = input[p]; p += 1;
        if tag == 0 {
            if p >= input.len() { return Err("lit trunc"); }
            let n = input[p] as usize; p += 1;
            if p + n > input.len() { return Err("lit oob"); }
            out.extend_from_slice(&input[p..p + n]);
            p += n;
        } else if tag == 1 {
            if p + 3 > input.len() { return Err("match trunc"); }
            let off = u16::from_le_bytes(input[p..p+2].try_into().unwrap()) as usize;
            let len = input[p + 2] as usize;
            p += 3;
            if off == 0 || off > out.len() { return Err("bad off"); }
            for _ in 0..len {
                let b = out[out.len() - off];
                out.push(b);
            }
        } else {
            return Err("tag");
        }
    }
    out.truncate(size);
    Ok(out)
}
