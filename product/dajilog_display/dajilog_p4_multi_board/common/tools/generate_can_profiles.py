#!/usr/bin/env python3
import argparse, pathlib, re, sys
import yaml

def ident(value): return re.sub(r"[^A-Za-z0-9_]", "_", value)
def require(ok, message):
    if not ok: raise ValueError(message)
def load(path):
    with path.open(encoding="utf-8") as stream: return yaml.safe_load(stream)

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--protocol-dir", required=True, type=pathlib.Path)
    ap.add_argument("--devices", required=True, type=pathlib.Path)
    ap.add_argument("--out-c", required=True, type=pathlib.Path)
    ap.add_argument("--out-h", required=True, type=pathlib.Path)
    args = ap.parse_args()
    profiles = {}
    for path in sorted(args.protocol_dir.glob("*.yaml")):
        data = load(path); name = data["profile"]["name"]
        require(name not in profiles, f"duplicate profile: {name}")
        require(data["can"]["frame_format"] == "standard", f"{name}: only standard CAN supported")
        for fname, frame in data["frames"].items():
            require(0 <= int(frame["default_id"]) <= 0x7ff, f"{name}.{fname}: invalid ID")
            require(0 <= int(frame["dlc"]) <= 8, f"{name}.{fname}: invalid DLC")
        profiles[name] = data
    manifest = load(args.devices); bitrate = int(manifest["bus"]["bitrate"])
    devices = manifest.get("devices", []); instances = set(); rx_ids = set()
    for device in devices:
        name = device["instance"]; require(name not in instances, f"duplicate instance: {name}"); instances.add(name)
        pname = device["profile"]; require(pname in profiles, f"{name}: unknown profile {pname}")
        profile = profiles[pname]; require(int(profile["can"]["bitrate"]) == bitrate, f"{name}: bitrate mismatch")
        for fname, fdef in profile["frames"].items():
            require(fname in device["frames"], f"{name}: missing frame {fname}")
            can_id = int(device["frames"][fname]["id"]); require(0 <= can_id <= 0x7ff, f"{name}.{fname}: invalid ID")
            if fdef["direction"] == "rx" and device.get("enabled", True):
                require(can_id not in rx_ids, f"duplicate enabled RX ID: 0x{can_id:03X}"); rx_ids.add(can_id)
    args.out_h.parent.mkdir(parents=True, exist_ok=True)
    args.out_h.write_text('''#pragma once
#include <stddef.h>
#include "dajilog_can_profile.h"
#ifdef __cplusplus
extern "C" {
#endif
extern const dajilog_can_protocol_profile_t DAJILOG_CAN_PROFILES[];
extern const size_t DAJILOG_CAN_PROFILE_COUNT;
extern const dajilog_can_device_config_t DAJILOG_CAN_DEVICES[];
extern const size_t DAJILOG_CAN_DEVICE_COUNT;
#ifdef __cplusplus
}
#endif
''', encoding="utf-8")
    headers = sorted({data["codec"]["header"] for data in profiles.values()})
    lines = ['#include "dajilog_can_profiles_generated.h"']
    lines += [f'#include "{header}"' for header in headers]
    lines += ['', 'const dajilog_can_protocol_profile_t DAJILOG_CAN_PROFILES[] = {']
    indexes = {}
    for index, (name, data) in enumerate(profiles.items()):
        indexes[name] = index; p=data["profile"]; frames=data["frames"]
        tx=next(v for v in frames.values() if v["direction"]=="tx"); rx=next(v for v in frames.values() if v["direction"]=="rx")
        codec=data["codec"]
        lines += ['    {', f'        .name="{name}", .display_name="{p["display_name"]}",',
                  f'        .version={int(p["version"])}U, .bitrate={int(data["can"]["bitrate"])}U,',
                  f'        .default_command_id=0x{int(tx["default_id"]):03X}U, .default_status_id=0x{int(rx["default_id"]):03X}U,',
                  f'        .command_dlc={int(tx["dlc"])}U, .status_dlc={int(rx["dlc"])}U,',
                  f'        .decoded_size=sizeof({codec["decoded_type"]}),',
                  f'        .encode_command={codec["encode"]}, .decode_status={codec["decode"]},', '    },']
    lines += ['};', 'const size_t DAJILOG_CAN_PROFILE_COUNT=sizeof(DAJILOG_CAN_PROFILES)/sizeof(DAJILOG_CAN_PROFILES[0]);', '',
              'const dajilog_can_device_config_t DAJILOG_CAN_DEVICES[] = {']
    for d in devices:
        profile=profiles[d["profile"]]; frames=profile["frames"]
        txn=next(n for n,v in frames.items() if v["direction"]=="tx"); rxn=next(n for n,v in frames.items() if v["direction"]=="rx")
        lines += ['    {', f'        .instance_name="{d["instance"]}", .display_name="{d["display_name"]}",',
                  f'        .profile_index={indexes[d["profile"]]}U, .command_id=0x{int(d["frames"][txn]["id"]):03X}U,',
                  f'        .status_id=0x{int(d["frames"][rxn]["id"]):03X}U, .timeout_ms={int(frames[rxn].get("timeout_ms",1000))}U,',
                  f'        .enabled={"true" if d.get("enabled",True) else "false"},', '    },']
    if devices:
        lines += ['};', 'const size_t DAJILOG_CAN_DEVICE_COUNT=sizeof(DAJILOG_CAN_DEVICES)/sizeof(DAJILOG_CAN_DEVICES[0]);', '']
    else:
        lines[-1] = 'const dajilog_can_device_config_t DAJILOG_CAN_DEVICES[1] = { { 0 } };'
        lines += ['const size_t DAJILOG_CAN_DEVICE_COUNT=0;', '']
    args.out_c.write_text("\n".join(lines), encoding="utf-8")

if __name__ == "__main__":
    try: main()
    except (KeyError, TypeError, ValueError) as error:
        print(f"CAN profile generation failed: {error}", file=sys.stderr); sys.exit(2)
