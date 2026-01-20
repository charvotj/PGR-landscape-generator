import os
import zipfile
import requests
from lxml import etree
from tqdm import tqdm
from urllib.parse import urlparse
from shapely.geometry import Point, Polygon

# =========================
# KONFIGURACE
# =========================

ROOT_FEED_FILE = "DMR5G-ETRS89.xml" 

# =========================
# POMOCNÉ FUNKCE
# =========================

def parse_xml(content):
    return etree.fromstring(content)

def parse_xml_file(path):
    with open(path, "rb") as f:
        return etree.fromstring(f.read())


# --- 3️⃣ Funkce pro hledání bodu ---
def find_dataset_for_point(lat, lon, polygons):
    pt = Point(lon, lat)
    for polygon, link in polygons:
        if polygon.contains(pt):
            return link
    return None

# =========================
# HLAVNÍ LOGIKA
# =========================

def main():

    root = None
    if not os.path.exists(ROOT_FEED_FILE):
        return -1
    else:
        print("📄 Načítám hlavní Atom feed ze souboru...")
        root = parse_xml_file(ROOT_FEED_FILE)

    if root is None:
        return -1
    
    ns = {
        "atom": "http://www.w3.org/2005/Atom",
        "georss": "http://www.georss.org/georss"
    }

    polygons = []  # list of tuples (Polygon objekt, dataset link)

    entries = root.xpath("//atom:entry", namespaces=ns)
    for entry in entries:
        link = entry.xpath("atom:link[@rel='alternate']/@href", namespaces=ns)
        link = link[0] if link else None

        poly_text = entry.xpath("georss:polygon/text()", namespaces=ns)
        if not poly_text:
            continue
        coords = list(map(float, poly_text[0].split()))
        # polygon má formu [lat1, lon1, lat2, lon2, ...]
        points = [(coords[i+1], coords[i]) for i in range(0, len(coords), 2)]  # (x=lon, y=lat)
        polygon = Polygon(points)
        polygons.append((polygon, link))

    print(f"✅ Načteno {len(polygons)} polygonů/datasetů")

    # --- 4️⃣ Příklad použití ---
    lat = 49.5415519
    lon = 18.6990247

    dataset_link = find_dataset_for_point(lat, lon, polygons)
    if dataset_link:
        print(f"Bod ({lat}, {lon}) je v datasetu: {dataset_link}")
    else:
        print(f"Bod ({lat}, {lon}) nespadá do žádného polygonu")

    print("✅ Hotovo")

if __name__ == "__main__":
    main()
