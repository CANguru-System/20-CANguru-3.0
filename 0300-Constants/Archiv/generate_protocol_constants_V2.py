import re

INPUT_FILE = "protocol_constants.h"

CORE_OUT     = "protocol_constants_core.h"
SIGNAL_OUT   = "protocol_constants_signal.h"
BRIDGE_OUT   = "protocol_constants_bridge.h"
GUI_OUT      = "protocol_constants_gui.h"

core = []
signal = []
bridge = []
gui = []

with open(INPUT_FILE, "r") as f:
    for line in f:
        m = re.match(r"#define\s+([A-Za-z0-9_]+)\s+(.*)", line)
        if not m:
            continue

        name = m.group(1)
        value = m.group(2)

        # --- CORE ---
        if (
            name.startswith("FRAME_DECODER_") or
            name.startswith("MSG_START_DECODERS") or
            name.startswith("BRIDGE_AP_CHANNEL") or
            name.startswith("ALIVE_INTERVAL_MS") or
            name.startswith("DECODERTYPE_") or
            name.startswith("FRAME_DECODER_OTA_")
        ):
            core.append(line)
            continue

        # --- SIGNAL FEATURE ---
        if (
            "SIGNAL" in name or
            name.startswith("TYPE_SIGNAL") or
            name.startswith("DEVTYPE_SIGNAL")
        ):
            signal.append(line)
            continue

        # --- BRIDGE ---
        if (
            name.startswith("MSG_START_BRIDGE") or
            name.startswith("BRIDGE_PORT") or
            name.startswith("WDP_PORT_")
        ):
            bridge.append(line)
            continue

        # --- GUI ---
        if (
            name.startswith("FRAME_GUI_") or
            name.startswith("GUI_")
        ):
            gui.append(line)
            continue

        # Default: Core (sicherste Variante)
        core.append(line)

def write_file(filename, lines):
    with open(filename, "w") as f:
        f.write("// Auto-generated\n\n")
        for l in lines:
            f.write(l)

write_file(CORE_OUT, core)
write_file(SIGNAL_OUT, signal)
write_file(BRIDGE_OUT, bridge)
write_file(GUI_OUT, gui)

print("Done.")
