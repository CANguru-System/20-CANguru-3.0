#!/usr/bin/env python3
# -----------------------------------------
# KONFIGURATION
# -----------------------------------------

import yaml
import os
import re
from datetime import datetime

# Absoluter Pfad des Skripts
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))

# Eingabedatei IMMER relativ zum Skript
YAML_FILE = os.path.join(SCRIPT_DIR, "protocol_constants.yaml")

# Hilfsfunktion: macht aus einem relativen Pfad einen absoluten Pfad relativ zum Skript
def resolve(path):
    return os.path.abspath(os.path.join(SCRIPT_DIR, path))

# Ausgabepfade – werden später über resolve() absolut gemacht
OUTPUTS = {
    "GUI": "../0210 CANguru-Server/0210-CANguru-Server/Models/protocol_constants.cs",
    "Bridge": "../0213-CANguru-Bridge/include/protocol_constants.h",
    "Decoder": "../0215-CANguru-Decoder/include/protocol_constants.h",
}

EXTRA_DECODER_DIRS = [
    "../Decoder2",
    "../Decoder3",
]

# -----------------------------------------
# YAML LADEN
# -----------------------------------------

def load_yaml_raw():
    with open(YAML_FILE, "r", encoding="utf-8") as f:
        return f.readlines()

def load_yaml():
    with open(YAML_FILE, "r", encoding="utf-8") as f:
        return yaml.safe_load(f)["constants"]

# -----------------------------------------
# KOMMENTARE UND HEX-WERTE EXTRAHIEREN
# -----------------------------------------

def extract_comments(raw_lines):
    comments = {}
    last_comment_block = []

    for line in raw_lines:
        stripped = line.strip()

        if stripped.startswith("#"):
            last_comment_block.append(stripped[1:].strip())
            continue

        match = re.match(r"([A-Za-z0-9_]+)\s*:", stripped)
        if match:
            key = match.group(1)
            comments[key] = last_comment_block.copy()
            last_comment_block.clear()

    return comments

def detect_hex(raw_lines):
    hex_map = {}

    for line in raw_lines:
        stripped = line.strip()
        match = re.match(r"([A-Za-z0-9_]+)\s*:\s*(.+)", stripped)
        if match:
            key, value = match.group(1), match.group(2)
            value = value.split("#")[0].strip()

            if value.lower().startswith("0x"):
                hex_map[key] = value

    return hex_map

# -----------------------------------------
# CODE-GENERIERUNG OHNE LEERZEILEN
# -----------------------------------------

def generate_cs(constants, comments, hex_map):
    lines = []
    lines.append("namespace CANguru.Models")
    lines.append("{")
    lines.append("    public static class ProtocolConstants")
    lines.append("    {")

    for name, value in constants.items():
        if name in comments:
            for c in comments[name]:
                lines.append(f"        // {c}")

        if name in hex_map:
            lines.append(f"        public const int {name} = {hex_map[name]};")
        else:
            lines.append(f"        public const int {name} = {value};")

    lines.append("    }")
    lines.append("}")
    return "\n".join(lines)

def generate_h(constants, comments, hex_map):
    lines = []
    lines.append("#pragma once")

    for name, value in constants.items():
        if name in comments:
            for c in comments[name]:
                lines.append(f"// {c}")

        if name in hex_map:
            lines.append(f"#define {name} {hex_map[name]}")
        else:
            lines.append(f"#define {name} {value}")

    return "\n".join(lines)

# -----------------------------------------
# BACKUP + DATEIEN SCHREIBEN
# -----------------------------------------

def backup_file(path):
    if not os.path.exists(path):
        return

    timestamp = datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
    backup_path = f"{path}.bak_{timestamp}"
    os.rename(path, backup_path)
    print(f"🔄 Backup angelegt: {backup_path}")

def write_file(path, content):
    abs_path = resolve(path)
    os.makedirs(os.path.dirname(abs_path), exist_ok=True)

    # Backup anlegen
    backup_file(abs_path)

    # Datei schreiben
    with open(abs_path, "w", encoding="utf-8") as f:
        f.write(content)

    print(f"✔ Datei erzeugt: {abs_path}")

# -----------------------------------------
# MAIN
# -----------------------------------------

def main():
    print(f"Arbeitsverzeichnis: {os.getcwd()}")
    print(f"Skriptverzeichnis:  {SCRIPT_DIR}")
    print(f"YAML-Datei:         {YAML_FILE}\n")

    raw = load_yaml_raw()
    constants = load_yaml()
    comments = extract_comments(raw)
    hex_map = detect_hex(raw)

    write_file(OUTPUTS["GUI"], generate_cs(constants, comments, hex_map))
    write_file(OUTPUTS["Bridge"], generate_h(constants, comments, hex_map))
    write_file(OUTPUTS["Decoder"], generate_h(constants, comments, hex_map))

    for d in EXTRA_DECODER_DIRS:
        path = os.path.join(d, "protocol_constants.h")
        write_file(path, generate_h(constants, comments, hex_map))

    print("\nFertig – alle Dateien wurden sicher, sauber und mit Backups erzeugt.")

if __name__ == "__main__":
    main()
