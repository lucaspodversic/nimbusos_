#!/usr/bin/env python3
"""NimbusOS — crea layout VHDX/raw con particiones A/B (esqueleto)."""
import argparse, json, pathlib, sys

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--layout", default="build/ab_partition_layout.json")
    ap.add_argument("-o", "--output", default="nimbusos.img")
    args = ap.parse_args()
    layout = json.loads(pathlib.Path(args.layout).read_text())
    pathlib.Path(args.output).write_text("NIMBUSOS_IMG " + json.dumps(layout)[:200])
    print(f"Wrote placeholder image {args.output}")
    return 0

if __name__ == "__main__":
    sys.exit(main())
