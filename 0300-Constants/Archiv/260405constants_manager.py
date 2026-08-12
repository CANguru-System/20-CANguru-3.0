import os
import yaml

# ------------------------------------------------------------
# Zentrale Konstanten-Definition
# ------------------------------------------------------------

CONSTANTS = [

    # --------------------------------------------------------
    # DECODER-TYPEN (Core) auch featureID
    # --------------------------------------------------------
    dict(name="DECODERTYPE_Switch",   value="0", arch="core"),
    dict(name="DECODERTYPE_Signal",   value="1", arch="core"),
    dict(name="DECODERTYPE_Power",    value="2", arch="core"),
    dict(name="DECODERTYPE_Feedback", value="3", arch="core"),
    dict(name="DECODERTYPE_Bridge",   value="10", arch="core"),

    # --------------------------------------------------------
    # Power-Decoder
    # --------------------------------------------------------
    dict(name="POWER_CMD_SET_THRESHOLD", value="0x01", arch="power"),
    dict(name="POWER_CMD_GET_THRESHOLD", value="0x02", arch="power"),
    dict(name="POWER_CMD_SHUTDOWN",      value="0x03", arch="power"),
    # --------------------------------------------------------
    # UPDATE / FIRMWARE TRANSFER (Bridge + GUI)
    # --------------------------------------------------------
    dict(name="Single_decoder",       value="0xA0", arch="bridge"),
    dict(name="Group_decoders",       value="0xA1", arch="bridge"),
    dict(name="Decoder_Upload_End",   value="0xB2", arch="bridge"),
    dict(name="Decoder_Upload_Start", value="0xB0", arch="bridge"),
    dict(name="Decoder_Upload_Block", value="0xB3", arch="bridge"),
    dict(name="Decoder_Upload_Error", value="0xB4", arch="bridge"),

    # --------------------------------------------------------
    # OTA (Core + Bridge)
    # --------------------------------------------------------
    dict(name="BlockSizeGUI",       value="1024", arch="bridge"),
    dict(name="BlockSizeBridge",    value="1500", arch="bridge"),

    dict(name="BRIDGE_AP_CHANNEL",  value="1", arch="core"),

    dict(name="FRAME_DECODER_OTA_START_WIFI", value="0x45", arch="core"),
    dict(name="FRAME_DECODER_OTA_SUCCESS",    value="0x47", arch="core"),
    dict(name="FRAME_DECODER_OTA_ERROR",      value="0x48", arch="core"),

    dict(name="FRAME_GUI_OTA_START_WIFI",       value="0x50", arch="bridge"),
    dict(name="FRAME_GUI_OTA_START_WIFI_ACK",   value="0x51", arch="bridge"),
    dict(name="FRAME_GUI_OTA_RESULT",           value="0x52", arch="bridge"),
    dict(name="FRAME_GUI_OTA_OVERALL",          value="0x53", arch="bridge"),

    dict(name="FRAME_GUI_OTA_CURRENT_DECODER",  value="0x63", arch="bridge"),
    dict(name="FRAME_GUI_OTA_CURRENT_FINISHED", value="0x64", arch="bridge"),
    dict(name="FRAME_GUI_OTA_CURRENT_ERROR",    value="0x65", arch="bridge"),

    # --------------------------------------------------------
    # FIRMWARE Bridge <-> Decoder (Core)
    # --------------------------------------------------------
    dict(name="CMD_DECODER_FW_ACK",   value="0x71", arch="core"),
    dict(name="CMD_DECODER_FW_DONE",  value="0x72", arch="core"),

    # --------------------------------------------------------
    # FIRMWARE GUI <-> Bridge (Bridge)
    # --------------------------------------------------------
    dict(name="CMD_UPLOAD_BLOCK", value="0xF1", arch="bridge"),
    dict(name="CMD_ACK",          value="0xF2", arch="bridge"),

    # --------------------------------------------------------
    # ALIVE (Bridge)
    # --------------------------------------------------------
    dict(name="CMD_DECODER_RESET", value="0xBD", arch="core"),

    # --------------------------------------------------------
    # PROTOKOLL (Core + Bridge)
    # --------------------------------------------------------
    dict(name="MSG_START_DECODERS", value="0xC6", arch="core"),

    # --------------------------------------------------------
    # DECODER ALIVE (Core)
    # --------------------------------------------------------
    dict(name="FRAME_DECODER_HELLO",      value="0xD0", arch="core"),
    dict(name="FRAME_DECODER_ID_ASSIGN",  value="0xD1", arch="core"),
    dict(name="FRAME_DECODER_ID_ACK",     value="0xD2", arch="core"),
    dict(name="FRAME_DECODER_ALIVE",      value="0xD3", arch="core"),
    dict(name="FRAME_DECODER_ALIVE_ACK",  value="0xD4", arch="core"),
    dict(name="FRAME_DECODER_REJOIN",     value="0x75", arch="core"),
    dict(name="FRAME_DECODER_ADDED",      value="0x0A", arch="can"),

    # --------------------------------------------------------
    # RESET (Core)
    # --------------------------------------------------------
    dict(name="cmdReset", value="0xE0", arch="core"),

    # --------------------------------------------------------
    # PORTS (Bridge)
    # --------------------------------------------------------
    dict(name="BRIDGE_PORT",   value="5000", arch="bridge"),
    dict(name="GUI_PORT",      value="5001", arch="bridge"),
    dict(name="WDP_PORT_OUT",  value="15730", arch="bridge"),
    dict(name="WDP_PORT_IN",   value="15731", arch="bridge"),
    dict(name="HTTPPORT",      value="80", arch="bridge"),

    # --------------------------------------------------------
    # CANguru (CAN)
    # --------------------------------------------------------
    dict(name="wait_time_small", value="50", arch="can"),
    dict(name="bdrMonitor",      value="115200", arch="can"),
    dict(name="CAN_FRAME_SIZE",  value="13", arch="can"),

    # --------------------------------------------------------
    # FRAMEFELDER (CAN)
    # --------------------------------------------------------
    dict(name="CANcmd",  value="0x01", arch="can"),
    dict(name="hash0",   value="0x02", arch="can"),
    dict(name="hash1",   value="0x03", arch="can"),
    dict(name="Framelng",value="0x04", arch="can"),
    dict(name="data0",   value="0x05", arch="can"),
    dict(name="data1",   value="0x06", arch="can"),
    dict(name="data2",   value="0x07", arch="can"),
    dict(name="data3",   value="0x08", arch="can"),
    dict(name="data4",   value="0x09", arch="can"),
    dict(name="data5",   value="0x0A", arch="can"),
    dict(name="data6",   value="0x0B", arch="can"),
    dict(name="data7",   value="0x0C", arch="can"),

    # --------------------------------------------------------
    # MÄRKLIN BEFEHLE (Märklin)
    # --------------------------------------------------------
    #define SYS_CMD 0x00    //Systembefehle
    dict(name="LokDiscovery",    value="0x02", arch="maerklin"),
    dict(name="LokDiscovery_R",  value="0x03", arch="maerklin"),
    dict(name="Lok_Zyklus_Stopp", value="0x04", arch="maerklin"),
    dict(name="MFXBind",          value="0x04", arch="maerklin"),
    dict(name="MFXBind_R",        value="0x05", arch="maerklin"),
    dict(name="MFXVerify",        value="0x06", arch="maerklin"),
    dict(name="MFXVerify_R",      value="0x07", arch="maerklin"),
    dict(name="Lok_Speed",        value="0x08", arch="maerklin"),
    dict(name="Lok_Direction",    value="0x0A", arch="maerklin"),
    dict(name="Lok_Function",     value="0x0C", arch="maerklin"),
    dict(name="ReadConfig",       value="0x0E", arch="maerklin"),
    dict(name="ReadConfig_R",     value="0x0F", arch="maerklin"),
    dict(name="WriteConfig",      value="0x10", arch="maerklin"),
    dict(name="WriteConfig_R",    value="0x11", arch="maerklin"),
    dict(name="SWITCH_ACC",       value="0x16", arch="maerklin"),
    dict(name="SWITCH_ACC_R",     value="0x17", arch="maerklin"),
    dict(name="S88_Polling",      value="0x20", arch="maerklin"),
    dict(name="S88_EVENT",        value="0x22", arch="maerklin"),
    dict(name="S88_EVENT_R",      value="0x23", arch="maerklin"),
    dict(name="PING",             value="0x30", arch="maerklin"),
    dict(name="PING_R",           value="0x31", arch="maerklin"),
    dict(name="CONFIG_Status",    value="0x3A", arch="maerklin"),
    dict(name="CONFIG_Status_R",  value="0x3B", arch="maerklin"),
    dict(name="DataQuery",        value="0x40", arch="maerklin"),
    dict(name="ConfigDataStream", value="0x42", arch="maerklin"),

    # --------------------------------------------------------
    # MÄRKLIN Konstanten (Märklin)
    # --------------------------------------------------------
    dict(name="MM_ACC",           value="0x3000", arch="maerklin"),
    dict(name="DCC_ACC",          value="0x3800", arch="maerklin"),
    dict(name="MM_TRACK",         value="0x0000", arch="maerklin"),
    dict(name="DCC_TRACK",        value="0xC000", arch="maerklin"),

    # --------------------------------------------------------
    # MÄRKLIN DEVICE TYPES (Märklin)
    # --------------------------------------------------------
    dict(name="DEVTYPE_GFP",        value="0x0000", arch="maerklin"),
    dict(name="DEVTYPE_GB",         value="0x0010", arch="maerklin"),
    dict(name="DEVTYPE_CONNECT",    value="0x0020", arch="maerklin"),
    dict(name="DEVTYPE_MS2",        value="0x0030", arch="maerklin"),
    dict(name="DEVTYPE_WDEV",       value="0x00E0", arch="maerklin"),
    dict(name="DEVTYPE_CS2",        value="0x00FF", arch="maerklin"),
    dict(name="DEVTYPE_FirstCANguru",value="0x004F", arch="maerklin"),
    dict(name="DEVTYPE_BASE",       value="0x0050", arch="maerklin"),
    dict(name="DEVTYPE_RM",         value="0x0051", arch="maerklin"),
    dict(name="DEVTYPE_MAXISIGNAL", value="0x0052", arch="maerklin"),
    dict(name="DEVTYPE_STEPPER",    value="0x0053", arch="maerklin"),
    dict(name="DEVTYPE_CANBOOSTER", value="0x0054", arch="maerklin"),
    dict(name="DEVTYPE_SERVO",      value="0x0055", arch="maerklin"),
    dict(name="DEVTYPE_LIGHT",      value="0x0056", arch="maerklin"),
    dict(name="DEVTYPE_SIGNAL",     value="0x0057", arch="maerklin"),
    dict(name="DEVTYPE_LEDSIGNAL",  value="0x0058", arch="maerklin"),
    dict(name="DEVTYPE_CAR_CAR",    value="0x0059", arch="maerklin"),
    dict(name="DEVTYPE_CAR_RM",     value="0x005A", arch="maerklin"),
    dict(name="DEVTYPE_CAR_SERVO",  value="0x005B", arch="maerklin"),
    dict(name="DEVTYPE_TRAFFICLIGHT",value="0x005C", arch="maerklin"),
    dict(name="DEVTYPE_FREE1",      value="0x005D", arch="maerklin"),
    dict(name="DEVTYPE_FREE2",      value="0x005E", arch="maerklin"),
    dict(name="DEVTYPE_LastCANguru",value="0x005F", arch="maerklin"),

    # --------------------------------------------------------
    # Upload lokomotive.cs2
    # --------------------------------------------------------
    dict(name="CMD_FILE_BLOCK_CS2",     value="0xE6", arch="bridge"),
    dict(name="CMD_FILE_ACK_CS2",       value="0xE7", arch="bridge"),
]

# ------------------------------------------------------------
# Datei-Mapping
# ------------------------------------------------------------

ARCH_TO_HEADER = {
    "core":     "protocol_constants_core.h",
    "bridge":   "protocol_constants_bridge.h",
    "signal":   "protocol_constants_signal.h",
    "switch":   "protocol_constants_switch.h",
    "servo":    "protocol_constants_servo.h",
    "power": "protocol_constants_power.h",
    "can":      "protocol_constants_can.h",
    "maerklin": "protocol_constants_maerklin.h",
}

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
OUT_HEADERS = os.path.join(SCRIPT_DIR, "generated_constants")
OUT_YAML = os.path.join(SCRIPT_DIR, "protocol_constants.yaml")
OUT_CS = os.path.join(SCRIPT_DIR, "ProtocolConstants.cs")

HEADER_TEMPLATE = """#pragma once
// ------------------------------------------------------------
// Auto-generated by constants_manager.py
// DO NOT EDIT MANUALLY
// ------------------------------------------------------------

"""

# ------------------------------------------------------------
# Header-Generator
# ------------------------------------------------------------

def generate_headers():
    os.makedirs(OUT_HEADERS, exist_ok=True)
    buckets = {arch: [] for arch in ARCH_TO_HEADER.keys()}

    for c in CONSTANTS:
        buckets[c["arch"]].append(c)

    for arch, filename in ARCH_TO_HEADER.items():
        consts = buckets[arch]
        if not consts:
            continue
        path = os.path.join(OUT_HEADERS, filename)
        with open(path, "w", encoding="utf-8") as f:
            f.write(HEADER_TEMPLATE)
            for c in consts:
                f.write(f"#define {c['name']} {c['value']}\n")
        print(f"[C]  {path}")

# ------------------------------------------------------------
# YAML-Generator
# ------------------------------------------------------------

def generate_yaml():
    data = {
        "constants": {
            c["name"]: {"value": c["value"], "architecture": c["arch"]}
            for c in CONSTANTS
        }
    }
    with open(OUT_YAML, "w", encoding="utf-8") as f:
        yaml.safe_dump(data, f, sort_keys=False)
    print(f"[YAML] {OUT_YAML}")

# ------------------------------------------------------------
# C#-Generator
# ------------------------------------------------------------

def generate_cs():
    lines = []
    lines.append("// Auto-generated by constants_manager.py")
    lines.append("public static class ProtocolConstants")
    lines.append("{")
    for c in CONSTANTS:
        val = c["value"]
        lines.append(f"    public const int {c['name']} = {val};")
    lines.append("}")
    with open(OUT_CS, "w", encoding="utf-8") as f:
        f.write("\n".join(lines))
    print(f"[C#] {OUT_CS}")

# ------------------------------------------------------------
# Main
# ------------------------------------------------------------

def main():
    generate_headers()
    generate_yaml()
    generate_cs()
    print("Fertig – Konstanten aus einer Quelle generiert.")

if __name__ == "__main__":
    main()
