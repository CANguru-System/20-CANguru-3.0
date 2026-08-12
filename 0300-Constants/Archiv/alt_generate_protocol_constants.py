#!/usr/bin/env python3
# -----------------------------------------
# KONFIGURATION
# -----------------------------------------

# Eingabedatei
import yaml
import os
import re

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
YAML_FILE = os.path.join(SCRIPT_DIR, "protocol_constants.yaml")

# Ausgabepfade
OUTPUTS = {
    "GUI": "../0210 CANguru-Server/0210-CANguru-Server/Models/protocol_constants.cs",
    "Bridge": "../0213-CANguru-Bridge/include/protocol_constants.h",
    "Decoder": "../0215-CANguru-Decoder/include/protocol_constants.h",
}

EXTRA_DECODER_DIRS = [
    "../Decoder2",
    "../Decoder3",
]

def load_yaml_raw():
    with open(YAML_FILE, "r", encoding="utf-8") as f:
        return f.readlines()

def load_yaml():
    with open(YAML_FILE, "r", encoding="utf-8") as f:
        return yaml.safe_load(f)["constants"]

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

        lines.append("")

    lines.append("    }")
    lines.append("}")
    return "\n".join(lines)

def generate_h(constants, comments, hex_map):
    lines = []
    lines.append("#pragma once\n")

    for name, value in constants.items():
        if name in comments:
            for c in comments[name]:
                lines.append(f"// {c}")

        if name in hex_map:
            lines.append(f"#define {name} {hex_map[name]}")
        else:
            lines.append(f"#define {name} {value}")

        lines.append("")

    return "\n".join(lines)

def write_file(path, content):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", encoding="utf-8") as f:
        f.write(content)
    print(f"✔ Datei erzeugt: {path}")

def main():
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

    print("\nFertig – alle Dateien wurden mit Hex-Erkennung und Kommentaren erzeugt.")

if __name__ == "__main__":
    main()
