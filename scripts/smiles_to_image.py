#!/usr/bin/env python3
"""
Convert a SMILES string into a 2D molecular drawing saved as PNG or SVG.

Example:
    python scripts/smiles_to_image.py --smiles "CCO" --output ethanol.png

Requires RDKit: pip install rdkit-pypi
"""
from __future__ import annotations

import argparse
import sys
from pathlib import Path

try:
    from rdkit import Chem
    from rdkit.Chem import AllChem, Draw
except ImportError as exc:  # pragma: no cover - runtime dependency notice
    sys.stderr.write("This script requires RDKit. Install with 'pip install rdkit-pypi'.\n")
    raise


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Render a SMILES string to a 2D molecular drawing (PNG or SVG)."
    )
    parser.add_argument(
        "--smiles",
        help="SMILES string to render. Use '-' to read the SMILES from stdin.",
    )
    parser.add_argument(
        "--input",
        type=Path,
        help="Path to a text file containing a SMILES string (first line is used).",
    )
    parser.add_argument(
        "--output",
        type=Path,
        required=True,
        help="Output image path (e.g., molecule.png or molecule.svg).",
    )
    parser.add_argument(
        "--size",
        type=int,
        nargs=2,
        metavar=("WIDTH", "HEIGHT"),
        default=(400, 300),
        help="Image size in pixels (default: 400 300).",
    )
    return parser.parse_args()


def read_smiles(args: argparse.Namespace) -> str:
    if args.smiles:
        if args.smiles == "-":
            return sys.stdin.read().strip()
        return args.smiles.strip()
    if args.input:
        return args.input.read_text().splitlines()[0].strip()
    raise SystemExit("Please provide a SMILES string with --smiles or --input.")


def build_molecule(smiles: str):
    molecule = Chem.MolFromSmiles(smiles)
    if molecule is None:
        raise SystemExit(f"Invalid SMILES string: {smiles}")
    AllChem.Compute2DCoords(molecule)
    return molecule


def save_drawing(molecule, output_path: Path, size: tuple[int, int]) -> None:
    output_path.parent.mkdir(parents=True, exist_ok=True)
    suffix = output_path.suffix.lower()
    if suffix == ".svg":
        drawer = Draw.rdMolDraw2D.MolDraw2DSVG(size[0], size[1])
        drawer.DrawMolecule(molecule)
        drawer.FinishDrawing()
        output_path.write_text(drawer.GetDrawingText())
    else:
        image = Draw.MolToImage(molecule, size=size)
        image.save(output_path)


def main() -> None:
    args = parse_args()
    smiles = read_smiles(args)
    molecule = build_molecule(smiles)
    save_drawing(molecule, args.output, tuple(args.size))


if __name__ == "__main__":
    main()
