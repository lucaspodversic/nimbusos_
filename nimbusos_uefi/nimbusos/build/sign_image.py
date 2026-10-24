#!/usr/bin/env python3
"""NimbusOS — firma de imagen (stub: hash SHA-256)."""
import argparse, hashlib, pathlib, sys

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("image")
    ap.add_argument("-o", "--sig", default=None)
    args = ap.parse_args()
    data = pathlib.Path(args.image).read_bytes()
    h = hashlib.sha256(data).hexdigest()
    sig = args.sig or (args.image + ".sha256")
    pathlib.Path(sig).write_text(h + "\n")
    print(f"SHA-256 {h} -> {sig}")
    return 0

if __name__ == "__main__":
    sys.exit(main())
