#!/usr/bin/env python3
"""package.py <out dir> <dest dir>

Release files of the Electron build, named as Electron's own releases name them
(arch armv7l, as @electron/get and electron-packager call 32-bit ARM):
  electron-v<version>-win32-armv7l.zip   the dist zip (electron:electron_dist_zip)
  node-v<version>-headers.tar.gz         node headers for native addons
  win-armv7l-node.lib                    node.lib for native addons (= electron.lib, the import
                                         library of electron.exe, as Electron ships it)
"""
import os, shutil, sys

out, dest = sys.argv[1:3]
version = open(os.path.join(out, "..", "..", "electron", "winrt", "VERSION")).read().strip()
FILES = {"dist.zip": f"electron-v{version}-win32-armv7l.zip",
         "gen/node_headers.tar.gz": f"node-v{version}-headers.tar.gz",
         "electron.lib": "win-armv7l-node.lib"}
os.makedirs(dest, exist_ok=True)
for src, name in FILES.items():
    p = os.path.join(out, src)
    if not os.path.isfile(p):
        sys.exit(f"missing {src}")
    shutil.copyfile(p, os.path.join(dest, name))
    print(f"{name}: {os.path.getsize(p) / 2**20:.1f} MiB")
