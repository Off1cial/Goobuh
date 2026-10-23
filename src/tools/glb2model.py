#!/usr/bin/env python3
"""
glb2model.py - convert a .glb into the text model format:

    "Name"
    {
    F:<vertex count>
        v px py pz nx ny nz u v r g b a
        ...
        i 0 1 2 2 3 0 ...
    }

Usage:
    python3 glb2model.py input.glb [output.model] [--name Name]
                         [--scale 1.0] [--z-up] [--flip-v] [--srgb]

  --scale S   multiply all positions by S (e.g. 32 for metres -> Quake units)
  --z-up      convert glTF Y-up to Z-up  (x, y, z) -> (x, -z, y)
  --flip-v    use v = 1 - v
  --srgb      convert material/vertex colours from linear to sRGB before 0-255
Pure Python, no dependencies. Whole scene is baked into one vertex/index list.
"""
import sys, json, struct, math, argparse, os

COMP = {5120: ("b", 1), 5121: ("B", 1), 5122: ("h", 2), 5123: ("H", 2), 5125: ("I", 4), 5126: ("f", 4)}
NCOMP = {"SCALAR": 1, "VEC2": 2, "VEC3": 3, "VEC4": 4}
NORM_DIV = {5120: 127.0, 5121: 255.0, 5122: 32767.0, 5123: 65535.0}

# ---------- GLB parsing ----------
def load_glb(path):
    data = open(path, "rb").read()
    magic, ver, _ = struct.unpack_from("<4sII", data, 0)
    if magic != b"glTF":
        raise SystemExit("Not a .glb file")
    off, js, bin_chunk = 12, None, b""
    while off < len(data):
        ln, ctype = struct.unpack_from("<I4s", data, off)
        body = data[off+8: off+8+ln]
        if ctype == b"JSON": js = json.loads(body)
        elif ctype == b"BIN\0": bin_chunk = body
        off += 8 + ln
    return js, bin_chunk

def read_accessor(gltf, binbuf, idx):
    acc = gltf["accessors"][idx]
    fmt, size = COMP[acc["componentType"]]
    n = NCOMP[acc["type"]]
    count = acc["count"]
    out = []
    if "bufferView" in acc:
        bv = gltf["bufferViews"][acc["bufferView"]]
        base = bv.get("byteOffset", 0) + acc.get("byteOffset", 0)
        stride = bv.get("byteStride") or size * n
        for i in range(count):
            out.append(list(struct.unpack_from("<" + fmt * n, binbuf, base + i * stride)))
    else:
        out = [[0] * n for _ in range(count)]
    if acc.get("normalized") and acc["componentType"] in NORM_DIV:
        d = NORM_DIV[acc["componentType"]]
        out = [[max(c / d, -1.0) for c in v] for v in out]
    return out

# ---------- math ----------
def ident(): return [[1,0,0,0],[0,1,0,0],[0,0,1,0],[0,0,0,1]]

def mmul(a, b):
    return [[sum(a[i][k] * b[k][j] for k in range(4)) for j in range(4)] for i in range(4)]

def node_matrix(n):
    if "matrix" in n:
        m = n["matrix"]  # column-major
        return [[m[c*4 + r] for c in range(4)] for r in range(4)]
    tx, ty, tz = n.get("translation", [0, 0, 0])
    qx, qy, qz, qw = n.get("rotation", [0, 0, 0, 1])
    sx, sy, sz = n.get("scale", [1, 1, 1])
    r = [
        [1-2*(qy*qy+qz*qz), 2*(qx*qy-qz*qw),   2*(qx*qz+qy*qw)],
        [2*(qx*qy+qz*qw),   1-2*(qx*qx+qz*qz), 2*(qy*qz-qx*qw)],
        [2*(qx*qz-qy*qw),   2*(qy*qz+qx*qw),   1-2*(qx*qx+qy*qy)],
    ]
    return [[r[0][0]*sx, r[0][1]*sy, r[0][2]*sz, tx],
            [r[1][0]*sx, r[1][1]*sy, r[1][2]*sz, ty],
            [r[2][0]*sx, r[2][1]*sy, r[2][2]*sz, tz],
            [0, 0, 0, 1]]

def xform_point(m, p):
    return [m[i][0]*p[0] + m[i][1]*p[1] + m[i][2]*p[2] + m[i][3] for i in range(3)]

def det3(m):
    return (m[0][0]*(m[1][1]*m[2][2]-m[1][2]*m[2][1])
          - m[0][1]*(m[1][0]*m[2][2]-m[1][2]*m[2][0])
          + m[0][2]*(m[1][0]*m[2][1]-m[1][1]*m[2][0]))

def normal_matrix(m):
    """cofactor matrix of the upper 3x3 (== inverse transpose up to scale)"""
    a = [[m[r][c] for c in range(3)] for r in range(3)]
    cof = [[0]*3 for _ in range(3)]
    for r in range(3):
        for c in range(3):
            r1, r2 = [i for i in range(3) if i != r]
            c1, c2 = [i for i in range(3) if i != c]
            minor = a[r1][c1]*a[r2][c2] - a[r1][c2]*a[r2][c1]
            cof[r][c] = ((-1) ** (r + c)) * minor
    s = 1.0 if det3(a) >= 0 else -1.0
    return [[cof[r][c] * s for c in range(3)] for r in range(3)]

def normalize(v):
    l = math.sqrt(sum(x*x for x in v)) or 1.0
    return [x / l for x in v]

def srgb(c):
    c = min(max(c, 0.0), 1.0)
    return 12.92 * c if c <= 0.0031308 else 1.055 * (c ** (1/2.4)) - 0.055

# ---------- conversion ----------
def convert(path, scale, z_up, flip_v, use_srgb):
    gltf, binbuf = load_glb(path)
    verts, indices = [], []   # verts: 12-float lists

    def conv_axis(v):
        return [v[0], -v[2], v[1]] if z_up else v

    def emit_mesh(mesh, world):
        nm = normal_matrix(world)
        flip_winding = det3(world) < 0
        for prim in mesh["primitives"]:
            if prim.get("mode", 4) != 4:
                print("  skipping non-triangle primitive", file=sys.stderr); continue
            attrs = prim["attributes"]
            pos = read_accessor(gltf, binbuf, attrs["POSITION"])
            nor = read_accessor(gltf, binbuf, attrs["NORMAL"]) if "NORMAL" in attrs else None
            uv  = read_accessor(gltf, binbuf, attrs["TEXCOORD_0"]) if "TEXCOORD_0" in attrs else None
            col = read_accessor(gltf, binbuf, attrs["COLOR_0"]) if "COLOR_0" in attrs else None
            if "indices" in prim:
                idx = [i[0] for i in read_accessor(gltf, binbuf, prim["indices"])]
            else:
                idx = list(range(len(pos)))

            base_col = [1, 1, 1, 1]
            if "material" in prim:
                mat = gltf["materials"][prim["material"]]
                base_col = mat.get("pbrMetallicRoughness", {}).get("baseColorFactor", [1, 1, 1, 1])

            start = len(verts)
            for i, p in enumerate(pos):
                wp = [c * scale for c in xform_point(world, p)]
                if nor:
                    n = nor[i]
                    wn = normalize([sum(nm[r][c] * n[c] for c in range(3)) for r in range(3)])
                else:
                    wn = [0.0, 1.0, 0.0]
                wp, wn = conv_axis(wp), conv_axis(wn)
                u, v = (uv[i][0], uv[i][1]) if uv else (0.0, 0.0)
                if flip_v: v = 1.0 - v
                c = list(col[i]) + [1.0] * (4 - len(col[i])) if col else [1, 1, 1, 1]
                c = [c[k] * base_col[k] for k in range(4)]
                rgb = [srgb(x) if use_srgb else min(max(x, 0.0), 1.0) for x in c[:3]]
                a = min(max(c[3], 0.0), 1.0)
                rgba = [int(round(x * 255)) for x in rgb + [a]]
                verts.append(wp + wn + [u, v] + rgba)

            for t in range(0, len(idx) - 2, 3):
                a, b, c = idx[t], idx[t+1], idx[t+2]
                if flip_winding: b, c = c, b
                indices.extend([start + a, start + b, start + c])

    def walk(ni, parent):
        n = gltf["nodes"][ni]
        world = mmul(parent, node_matrix(n))
        if "mesh" in n:
            emit_mesh(gltf["meshes"][n["mesh"]], world)
        for ch in n.get("children", []):
            walk(ch, world)

    scene = gltf["scenes"][gltf.get("scene", 0)]
    for r in scene["nodes"]:
        walk(r, ident())
    return verts, indices

def fmt(x):
    s = f"{x:.6f}".rstrip("0").rstrip(".")
    return "0" if s in ("-0", "") else s

def write_model(path, name, verts, indices):
    with open(path, "w") as f:
        f.write(f'"{name}"\n{{\nF:{len(verts)}\n')
        for v in verts:
            floats = " ".join(fmt(x) for x in v[:8])
            cols = " ".join(str(int(x)) for x in v[8:])
            f.write(f"    v {floats} {cols}\n")
        f.write("    i " + " ".join(str(i) for i in indices) + "\n}\n")

if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("input")
    ap.add_argument("output", nargs="?")
    ap.add_argument("--name")
    ap.add_argument("--scale", type=float, default=1.0)
    ap.add_argument("--z-up", action="store_true")
    ap.add_argument("--flip-v", action="store_true")
    ap.add_argument("--srgb", action="store_true")
    a = ap.parse_args()
    stem = os.path.splitext(os.path.basename(a.input))[0]
    out = a.output or os.path.splitext(a.input)[0] + ".model"
    verts, indices = convert(a.input, a.scale, a.z_up, a.flip_v, a.srgb)
    write_model(out, a.name or stem, verts, indices)
    print(f"{out}: {len(verts)} vertices, {len(indices)//3} triangles")
