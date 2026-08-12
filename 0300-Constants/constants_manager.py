import os
import yaml

# ------------------------------------------------------------
# Zentrale Konstanten-Definition
# ------------------------------------------------------------

CONSTANTS = [

# ============================================================
# constants
# ============================================================
dict(name="INVALID_ASSIGNED_ID", value="0x00", arch="core"),


# ============================================================
# payload_lengths
# ============================================================
dict(name="EXP_LNG_SWITCH_SETTINGS",    value="8", arch="core"),
dict(name="EXP_LNG_SIGNAL_SETTINGS",    value="8", arch="core"),
dict(name="EXP_LNG_POWER_SETTINGS",     value="2", arch="core"),
dict(name="EXP_LNG_FEEDBACK_SETTINGS",  value="16", arch="core"),
dict(name="EXP_LNG_ALIVE",              value="3", arch="core"),
dict(name="EXP_LNG_CAN_FRAME",          value="13", arch="core"),


# ============================================================
# BASE PROTOCOL FIELDS
# ============================================================
dict(name="canID",       value="0x00", arch="core"),
dict(name="startByte",   value="0x00", arch="core"),
dict(name="featureByte", value="0x01", arch="core"),
dict(name="commandByte", value="0x02", arch="core"),


# ============================================================
# FEATURE: BRIDGE
# ============================================================
dict(name="FEATURE_BRIDGE",     value="0x10", arch="core"),
dict(name="BRIDGE_START",       value="0x11", arch="core"), 


# ============================================================
# FEATURE: POWER DECODER
# ============================================================
dict(name="FEATURE_POWER",            value="0x20", arch="core"),
dict(name="POWER_CMD_SET_THRESHOLD",  value="0x21", arch="core"),
dict(name="POWER_CMD_GET_THRESHOLD",  value="0x22", arch="core"),
dict(name="POWER_CMD_SHUTDOWN",       value="0x23", arch="core"),
dict(name="POWER_CHG_TRACK_CURRENT",  value="0x24", arch="core"),
dict(name="POWER_CMD_GET_VERSION",    value="0x25", arch="core"),

# ============================================================
# FEATURE: SWITCH
# ============================================================
dict(name="FEATURE_SWITCH",          value="0x30", arch="core"),
dict(name="SWITCH_CMD_GET_SETTINGS", value="0x31", arch="core"),
dict(name="SWITCH_CMD_SET_SETTINGS", value="0x32", arch="core"),
dict(name="SWITCH_CMD_MOVE2START",   value="0x33", arch="core"),
dict(name="SWITCH_CMD_SET_ZERO",     value="0x34", arch="core"),
dict(name="SWITCH_CMD_MOVE2END",     value="0x35", arch="core"),
dict(name="SWITCH_CMD_SET_END",      value="0x36", arch="core"),
dict(name="SWITCH_CMD_SET_END_ACK",  value="0x37", arch="core"),
dict(name="SWITCH_CMD_TEST_RUN",     value="0x38", arch="core"),
dict(name="SWITCH_CMD_STOP_ALL",     value="0x39", arch="core"),
dict(name="SWITCH_CMD_SET_DIR",      value="0x3A", arch="core"),
dict(name="SWITCH_CMD_GET_VERSION",  value="0x3B", arch="core"),
dict(name="step_delay_min",          value="100", arch="core"),
dict(name="step_delay_std",          value="300", arch="core"),
dict(name="step_delay_max",          value="1000", arch="core"),
dict(name="step_delay_step",         value="50", arch="core"),
dict(name="stepsToEnd_min",          value="100", arch="core"),
dict(name="stepsToEnd_std",          value="1140", arch="core"),
dict(name="stepsToEnd_max",          value="1300", arch="core"),
dict(name="stepsToEnd_step",         value="10", arch="core"),

# ============================================================
# FEATURE: SIGNAL
# ============================================================
dict(name="FEATURE_SIGNAL",           value="0x40", arch="core"),
dict(name="SIGNAL_CMD_GET_SETTINGS",  value="0x41", arch="core"),
dict(name="SIGNAL_CMD_SET_SETTINGS",  value="0x42", arch="core"),
dict(name="SIGNAL_CMD_MOVE2START",    value="0x43", arch="core"),
dict(name="SIGNAL_CMD_SET_ZERO",      value="0x44", arch="core"),
dict(name="SIGNAL_CMD_MOVE2END",      value="0x45", arch="core"),
dict(name="SIGNAL_CMD_SET_END",       value="0x46", arch="core"),
dict(name="SIGNAL_CMD_SET_END_ACK",   value="0x47", arch="core"),
dict(name="SIGNAL_CMD_TEST_RUN",      value="0x48", arch="core"),
dict(name="SIGNAL_CMD_STOP_ALL",      value="0x49", arch="core"),
dict(name="SIGNAL_CMD_SET_DIR",       value="0x4A", arch="core"),
dict(name="SIGNAL_CMD_GET_VERSION",  value="0x4B", arch="core"),

# ============================================================
# FEATURE: FEEDBACK
# ============================================================
dict(name="FEATURE_FEEDBACK",              value="0x50", arch="core"),
dict(name="FEEDBACK_CMD_GET_SETTINGS",     value="0x51", arch="core"),
dict(name="FEEDBACK_CMD_SET_SETTINGS",     value="0x52", arch="core"),
dict(name="FEEDBACK_CMD_SET_SETTINGS_ACK", value="0x53", arch="core"),
dict(name="FEEDBACK_CMD_GET_VERSION",      value="0x54", arch="core"),


# ============================================================
# FEATURE: FIRMWARE & OTA für GUI und Bridge
# ============================================================
dict(name="FEATURE_FIRMWARE",               value="0x60", arch="core"),
dict(name="FRAME_GUI_OTA_RESULT",           value="0x61", arch="core"),
dict(name="FRAME_GUI_OTA_CURRENT_FINISHED", value="0x62", arch="core"),


# ============================================================
# FEATURE: ALIVE / DISCOVERY
# ============================================================
dict(name="FEATURE_ALIVE",              value="0x70", arch="core"),
dict(name="HANDLE_ALIVE",               value="0x71", arch="core"),
dict(name="FRAME_DECODER_ALIVE",        value="0x72", arch="core"),
dict(name="FRAME_DECODER_ALIVE_ACK",    value="0x73", arch="core"),
dict(name="FRAME_DECODER_HELLO",        value="0x74", arch="core"),
dict(name="FRAME_DECODER_ID_ASSIGN",    value="0x75", arch="core"),
dict(name="FRAME_DECODER_ID_ACK",       value="0x76", arch="core"),


# ============================================================
# FEATURE: CS2
# ============================================================
dict(name="FEATURE_CS2",        value="0x80", arch="core"),
dict(name="CMD_FILE_ACK_CS2",   value="0x81", arch="core"),
dict(name="CMD_FILE_BLOCK_CS2", value="0x82", arch="core"),
dict(name="FRAME_CAN_2DEC",     value="0x83", arch="core"),
dict(name="FRAME_CAN_2BRIDGE",  value="0x84", arch="core"),


# ============================================================
# FEATURE: SHOW BAR
# ============================================================
dict(name="FEATURE_SHOW_BAR",               value="0x90", arch="core"),
dict(name="FRAME_GUI_OTA_CURRENT_DECODER",  value="0x91", arch="core"),
dict(name="FRAME_GUI_OTA_OVERALL",          value="0x92", arch="core"),


# ============================================================
# FEATURE: DECODER UPLOAD
# ============================================================
dict(name="FEATURE_DECODER_UPLOAD", value="0xA0", arch="core"),
dict(name="DECODER_UPLOAD_START",   value="0xA1", arch="core"),
dict(name="DECODER_UPLOAD_BLOCK",   value="0xA2", arch="core"),
dict(name="DECODER_UPLOAD_END",     value="0xA3", arch="core"),
dict(name="DECODER_UPLOAD_ERROR",   value="0xA4", arch="core"),


# ============================================================
# FEATURE: SETTINGS
# ============================================================
dict(name="FEATURE_SETTING", value="0xB0", arch="core"),
dict(name="SETTING_CMD_RESET_TO_DEFAULTS", value="0xB1", arch="core"),
dict(name="SETTING_CMD_RESET_ACK",    value="0xB2", arch="core"),


# ============================================================
# FEATURE: OTA (DECODER)
# ============================================================
dict(name="FEATURE_OTA",        value="0xC0", arch="core"),
dict(name="OTA_START_WIFI",     value="0xC1", arch="core"),
dict(name="OTA_SUCCESS",        value="0xC2", arch="core"),
dict(name="OTA_ERROR",          value="0xC3", arch="core"),
dict(name="OTA_CURRENT_ERROR",  value="0xC4", arch="core"),
dict(name="OTA_FW_ACK",         value="0xC5", arch="core"),
dict(name="OTA_FW_DONE",        value="0xC6", arch="core"),
dict(name="OTA_UPLOAD_BLOCK",   value="0xC7", arch="core"),
dict(name="OTA_SINGLE_DECODER", value="0xC8", arch="core"),
dict(name="OTA_GROUP_DECODERS", value="0xC9", arch="core"),
dict(name="CMD_OTA_ACK",        value="0xCA", arch="core"),

# ============================================================
# FEATURE: DECODER CONTROL
# ============================================================
dict(name="FEATURE_DECODER",      value="0xD0", arch="core"),
dict(name="MSG_START_DECODERS",   value="0xD1", arch="core"),
dict(name="CMD_DECODER_RESET",    value="0xD2", arch="core"),
dict(name="FRAME_DECODER_REJOIN", value="0xD3", arch="core"),
dict(name="FRAME_DECODER_ADDED",  value="0xD4", arch="core"),


# ============================================================
# GENERIC COMMANDS
# ============================================================
dict(name="CMD_ACK", value="0xF0", arch="core"),


# ============================================================
# BLOCK SIZES & PORTS
# ============================================================
dict(name="BlockSizeGUI",      value="1024",     arch="core"),
dict(name="BlockSizeBridge",   value="BlockSizeGUI+100", arch="core"),
dict(name="BlockSizeDecoder",  value="BlockSizeGUI", arch="core"),
dict(name="BRIDGE_AP_CHANNEL", value="1",      arch="core"),
dict(name="BRIDGE_PORT",       value="5000",   arch="core"),
dict(name="GUI_PORT",          value="5001",   arch="core"),
dict(name="WDP_PORT_OUT",      value="15730",  arch="core"),
dict(name="WDP_PORT_IN",       value="15731",  arch="core"),
dict(name="HTTPPORT",          value="80",     arch="core"),
dict(name="bdrMonitor",        value="115200", arch="core"),
dict(name="wait_time_small",    value="100",   arch="core"),


# ============================================================
# CAN FRAME CONSTANTS
# ============================================================
dict(name="CAN_FRAME_SIZE", value="13", arch="core"),

dict(name="CANcmd",   value="0x01", arch="core"),
dict(name="hash0",     value="0x02", arch="core"),
dict(name="hash1",     value="0x03", arch="core"),
dict(name="Framelng",  value="0x04", arch="core"),
dict(name="data0",     value="0x05", arch="core"),
dict(name="data1",     value="0x06", arch="core"),
dict(name="data2",     value="0x07", arch="core"),
dict(name="data3",     value="0x08", arch="core"),
dict(name="data4",     value="0x09", arch="core"),
dict(name="data5",     value="0x0A", arch="core"),
dict(name="data6",     value="0x0B", arch="core"),
dict(name="data7",     value="0x0C", arch="core"),


# ============================================================
# LOK / MFX / S88 / CONFIG COMMANDS
# ============================================================
dict(name="FEATURE_BRIDGE2DECODER", value="0xFF", arch="core"),
dict(name="Sys_CMD",                value="0x00", arch="core"),
dict(name="LokDiscovery",           value="0x02", arch="core"),
dict(name="LokDiscovery_R",         value="0x03", arch="core"),
dict(name="Lok_Zyklus_Stopp",       value="0x04", arch="core"),
dict(name="MFXBind",                value="0x04", arch="core"),
dict(name="MFXBind_R",              value="0x05", arch="core"),
dict(name="MFXVerify",              value="0x06", arch="core"),
dict(name="MFXVerify_R",            value="0x07", arch="core"),
dict(name="Lok_Speed",              value="0x08", arch="core"),
dict(name="Lok_Direction",          value="0x0A", arch="core"),
dict(name="Lok_Function",           value="0x0C", arch="core"),
dict(name="ReadConfig",             value="0x0E", arch="core"),
dict(name="ReadConfig_R",           value="0x0F", arch="core"),
dict(name="WriteConfig",            value="0x10", arch="core"),
dict(name="WriteConfig_R",          value="0x11", arch="core"),
dict(name="SWITCH_ACC",             value="0x16", arch="core"),
dict(name="SWITCH_ACC_R",           value="0x17", arch="core"),
dict(name="S88_Polling",            value="0x20", arch="core"),
dict(name="S88_EVENT",              value="0x22", arch="core"),
dict(name="S88_EVENT_R",            value="0x23", arch="core"),
dict(name="PING",                   value="0x30", arch="core"),
dict(name="PING_R",                 value="0x31", arch="core"),
dict(name="CONFIG_Status",          value="0x3A", arch="core"),
dict(name="CONFIG_Status_R",        value="0x3B", arch="core"),
dict(name="DataQuery",              value="0x40", arch="core"),
dict(name="ConfigDataStream",       value="0x42", arch="core"),


# ============================================================
# PROTOCOL TYPES (MM / DCC)
# ============================================================
dict(name="MM_ACC",    value="0x3000", arch="core"),
dict(name="DCC_ACC",   value="0x3800", arch="core"),
dict(name="MM_TRACK",  value="0x0000", arch="core"),
dict(name="DCC_TRACK", value="0xC000", arch="core"),


# ============================================================
# DEVICE TYPES
# ============================================================
dict(name="DEVTYPE_GFP",          value="0x00", arch="core"),
dict(name="DEVTYPE_GB",           value="0x10", arch="core"),
dict(name="DEVTYPE_CONNECT",      value="0x20", arch="core"),
dict(name="DEVTYPE_MS2",          value="0x30", arch="core"),
dict(name="DEVTYPE_WDEV",         value="0xE0", arch="core"),
dict(name="DEVTYPE_CS2",          value="0xFF", arch="core"),
dict(name="DEVTYPE_FirstCANguru", value="0x4F", arch="core"),
dict(name="DEVTYPE_BASE",         value="0x50", arch="core"),
dict(name="DEVTYPE_SIGNAL",       value="0x51", arch="core"),
dict(name="DEVTYPE_SWITCH",       value="0x52", arch="core"),
dict(name="DEVTYPE_POWER",        value="0x53", arch="core"),
dict(name="DEVTYPE_RM",           value="0x54", arch="core"),
dict(name="DEVTYPE_MAXISIGNAL",   value="0x55", arch="core"),
dict(name="DEVTYPE_SERVO",        value="0x56", arch="core"),
dict(name="DEVTYPE_LIGHT",        value="0x57", arch="core"),
dict(name="DEVTYPE_LEDSIGNAL",    value="0x58", arch="core"),
dict(name="DEVTYPE_CAR_CAR",      value="0x59", arch="core"),
dict(name="DEVTYPE_CAR_RM",       value="0x5A", arch="core"),
dict(name="DEVTYPE_CAR_SERVO",    value="0x5B", arch="core"),
dict(name="DEVTYPE_TRAFFICLIGHT", value="0x5C", arch="core"),
dict(name="DEVTYPE_BRIDGE",       value="0x5D", arch="core"),
dict(name="DEVTYPE_FREE",         value="0x5E", arch="core"),
dict(name="DEVTYPE_LastCANguru",  value="0x5F", arch="core"),

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
