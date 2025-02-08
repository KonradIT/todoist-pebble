#!/usr/bin/python3

import json

def bump_version(version):
    # just bump the center char for now...
    parts = version.split(".")
    parts[1] = str(int(parts[1]) + 1)
    return ".".join(parts)

data = {}
with open("package.json", "r") as f:
    data = json.load(f)

old_version = data["version"]
new_version = bump_version(old_version)

print(f"Bumping version from {old_version} to {new_version}")
print("Enter to ok...")
input()

data["version"] = new_version

with open("package.json", "w") as f:
    json.dump(data, f, indent=4)
print("All done!")