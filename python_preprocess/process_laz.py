import os
import pdal
import numpy as np
from pathlib import Path
from scipy.spatial import Delaunay
from scipy.spatial import cKDTree

OUT_DIR = "DMR5G_CZ"
LAZ_DIR = Path(os.path.join(OUT_DIR, "laz"))
BIN_DIR = Path(os.path.join(OUT_DIR, "bin"))

# složka pro uložená binární data
BIN_DIR.mkdir(exist_ok=True)


def terrain_height(tree, terrain_z, x, y):
    ret = np.zeros(x.shape)
    for i in range(x.shape[0]):
        dist, idx = tree.query([x[i], y[i]], k=1)
        ret[i] =  terrain_z[idx]
    return ret


rewrite = True

# seznam LAZ souborů
laz_files = sorted(LAZ_DIR.glob("*.laz"))
# laz_files = sorted(LAZ_DIR.glob("616_5450.laz"))
# laz_files = sorted(LAZ_DIR.glob("766_5492.laz"))

for i, laz_file in enumerate(laz_files, 1):
    print(f"[{i}/{len(laz_files)}] Zpracovávám {laz_file.name} ...")
    
    # Cesta k výstupnímu souboru
    bin_file = BIN_DIR / (laz_file.stem + ".bin")
    bin_idx_file = BIN_DIR / (laz_file.stem + ".idx.bin")
    bin_trees_file = BIN_DIR / (laz_file.stem + ".trees.bin")
    if not rewrite and os.path.exists(bin_file):
        continue

    # PDAL pipeline – načteme všechny body
    pipeline_json = f"""
    {{
        "pipeline": [
            "{laz_file}"
        ]
    }}
    """
    pipeline = pdal.Pipeline(pipeline_json)
    pipeline.execute()
    
    arrays = pipeline.arrays[0]
    # Vytvoříme numpy float32 pole (-X,Z,Y) ... formát pro OpenGL
    points = np.vstack((-arrays['X'], arrays['Z'], arrays['Y'])).T.astype(np.float32)  

    # Normalizace s poměrem stran
    min_vals = points.min(axis=0)
    # max_vals = points.max(axis=0)

    # Největší rozsah všech os
    # ranges = max_vals - min_vals
    # max_range = ranges.max()

    # Posuneme na [0,1] a pak na [-1,1] podle max_range, zachováme poměr stran
    points[:,0] -= min_vals[0]
    points[:,2] -= min_vals[2]

    points /= 2000.0

    # points = Nx2 numpy array s X,Y
    points_xy = points[:, [0, 2]]  # ignorujeme Z pro triangulaci
    points_z = points[:, 1]  # Z samostatne

    tri = Delaunay(points_xy)
    triangles = tri.simplices.astype(np.uint32)  # Nx3 indexy do points

    # souřadnice stromů
    tree = cKDTree(points_xy)
    treeCount = 10000
    treeX = np.random.rand(treeCount)
    treeY = np.random.rand(treeCount)
    treeZ = terrain_height(tree, points_z, treeX, treeY)

    treePositions = np.vstack((treeX, treeZ, treeY)).T.astype(np.float32)  
    # treeZ = terrain_height(tree, points_z, 0.5, 0.5)

    
    
    # uložíme do binárního souboru připraveného pro C++
    points.tofile(bin_file)
    triangles.tofile(bin_idx_file)
    treePositions.tofile(bin_trees_file)
    
    print(f"  ✅ uložen {bin_file} ({points.shape[0]} bodů)")

print("Hotovo, všechna data připravena pro C++ / OpenGL.")
