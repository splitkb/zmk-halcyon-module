import json
import sys

# --- Configuration ---
# Set to False to disable left controller builds configured as central (no dongle mode)
BUILD_LEFT_STUDIO = True 
# ---------------------

def add_build(target_list, board, shields, artifact, mode=None):
    """Helper to append a build entry."""
    entry = {
        "board": board,
        "shield": " ".join(shields),
        "artifact-name": artifact
    }
    
    if mode == "studio":
        entry["cmake-args"] = "-DCONFIG_ZMK_STUDIO=y"
        entry["snippet"] = "studio-rpc-usb-uart"
    elif mode == "peripheral":
        entry["cmake-args"] = "-DCONFIG_ZMK_SPLIT_ROLE_CENTRAL=n"
        
    target_list.append(entry)

with open("keyboards.json", "r") as f:
    data = json.load(f)

builds = []

for kb in data["keyboards"]:
    # 1. Build Dongles (Main Dongle Hardware)
    for dongle in data["dongles"]:
        add_build(builds, dongle["name"], [f"{kb['name']}_dongle"] + ["mod_cirque_central"], f"{kb['short_name']}_dongle", mode="studio")
        # TODO: Add module variations for dongles if needed

    # 2. Build Controllers
    for c in data["controllers"]:
        if c["type"] != "wireless": continue
        
        for bat in data["battery_modules"]:
            if kb["split"] == "true":
                # --- Non-Module Builds (Base + Battery only) ---
                s_left_base = [f"{kb['name']}_left", bat["name"]]
                s_right_base = [f"{kb['name']}_right", bat["name"]]
                art_base_nomod = f"{kb['short_name']}_{{}}_{bat['short_name']}"

                # Left side for dongle usage (Peripheral) -> Now just "_left"
                add_build(builds, c["name"], s_left_base, art_base_nomod.format("left"), mode="peripheral")
                
                # Left side for Studio usage (Central) -> Now "_left_central"
                if BUILD_LEFT_STUDIO:
                    add_build(builds, c["name"], s_left_base + ["mod_cirque_central"], art_base_nomod.format("left") + "_central", mode="studio")
                
                # Right side
                add_build(builds, c["name"], s_right_base, art_base_nomod.format("right"))

                # --- Module Builds ---
                for mod in data["modules"]:
                    # Common variables for module iteration
                    s_left = s_left_base + [mod["name"]]
                    s_right = s_right_base + [mod["name"]]
                    art_base = f"{kb['short_name']}_{{}}_{bat['short_name']}_{mod['short_name']}"
                    
                    # Logic specific to module type
                    if mod["type"] == "encoder":
                        if mod["name"].endswith("_left"):
                            # Left side for dongle usage
                            add_build(builds, c["name"], s_left, 
                                      art_base.format("left"), mode="peripheral")
                            # Left side for Studio usage
                            if BUILD_LEFT_STUDIO:
                                add_build(builds, c["name"], s_left + ["mod_cirque_central"], art_base.format("left") + "_central", mode="studio")
                        elif mod["name"].endswith("_right"):
                            add_build(builds, c["name"], s_right, art_base.format("right"))
                    
                    elif mod["type"] == "pointing":
                        if mod["name"].endswith("cirque_central_hw"):
                            if BUILD_LEFT_STUDIO:
                                add_build(builds, c["name"], s_left, 
                                          art_base.format("left") + "_central", mode="studio")
                        elif mod["name"].endswith("cirque_hw_left"):
                            add_build(builds, c["name"], s_left, art_base.format("left"), mode="peripheral")
                        elif mod["name"].endswith("cirque_hw_right"):
                            add_build(builds, c["name"], s_right, art_base.format("right"))
                    
                    else: # Default/Other modules
                        # Left side for dongle usage
                        add_build(builds, c["name"], s_left, art_base.format("left"), mode="peripheral")
                        # Left side for Studio usage
                        if BUILD_LEFT_STUDIO:
                            add_build(builds, c["name"], s_left + ["mod_cirque_central"], art_base.format("left") + "_central", mode="studio")
                        # Right side
                        add_build(builds, c["name"], s_right, art_base.format("right"))
            else:
                # TODO: Handle non-split keyboards
                pass

# Output
sys.stdout = open("build.yaml", "w")

print("---\ninclude:")
for b in builds:
    print(f"- board: {b['board']}")
    print(f"  shield: {b['shield']}")
    print(f"  artifact-name: {b['artifact-name']}")
    if "cmake-args" in b:
        print(f"  cmake-args: {b['cmake-args']}")
    if "snippet" in b:
        print(f"  snippet: {b['snippet']}")

# Settings Resets
for c in data["controllers"]:
    if c["type"] == "wireless":
        print(f"- board: {c['name']}\n  shield: settings_reset")

for d in data["dongles"]:
    print(f"- board: {d['name']}\n  shield: settings_reset")
