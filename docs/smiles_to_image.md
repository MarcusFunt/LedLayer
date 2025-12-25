# SMILES to Molecule Image Helper

The `scripts/smiles_to_image.py` helper converts a SMILES string into a 2D molecular drawing saved as PNG or SVG. It is a standalone utility and does not depend on Arduino tooling.

## Requirements

Install [RDKit](https://www.rdkit.org/) in your Python environment:

```bash
pip install rdkit-pypi
```

## Usage

Render a SMILES string directly:

```bash
python scripts/smiles_to_image.py --smiles "CCO" --output ethanol.png
```

Read the SMILES from a file or stdin:

```bash
python scripts/smiles_to_image.py --input molecule.smi --output molecule.svg
cat "C1=CC=CC=C1" | python scripts/smiles_to_image.py --smiles - --output benzene.png
```

Use `--size` to set the image resolution (width height in pixels):

```bash
python scripts/smiles_to_image.py --smiles "O=C=O" --output co2.png --size 500 400
```
