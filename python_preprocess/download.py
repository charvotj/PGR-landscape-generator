import os
import zipfile
import requests
from lxml import etree
from tqdm import tqdm
from urllib.parse import urlparse

# =========================
# KONFIGURACE
# =========================

ROOT_FEED_URL = "https://atom.cuzk.gov.cz/DMR5G-ETRS89/DMR5G-ETRS89.xml"
ROOT_FEED_FILE = "DMR5G-ETRS89.xml" 
OUT_DIR = "DMR5G_CZ"
FEED_DIR = os.path.join(OUT_DIR, "feeds")
ZIP_DIR = os.path.join(OUT_DIR, "zip")
LAZ_DIR = os.path.join(OUT_DIR, "laz")

HEADERS = {
    "User-Agent": "DMR5G-downloader (academic use)"
}

TIMEOUT = 60

# =========================
# POMOCNÉ FUNKCE
# =========================

def ensure_dirs():
    for d in (FEED_DIR, ZIP_DIR, LAZ_DIR):
        os.makedirs(d, exist_ok=True)

def download(url, out_path):
    if os.path.exists(out_path):
        return
    r = requests.get(url, headers=HEADERS, timeout=TIMEOUT, stream=True)
    r.raise_for_status()
    with open(out_path, "wb") as f:
        for chunk in r.iter_content(chunk_size=1024 * 1024):
            if chunk:
                f.write(chunk)

def unzip(zip_path, out_dir):
    with zipfile.ZipFile(zip_path, "r") as z:
        z.extractall(out_dir)

def parse_xml(content):
    return etree.fromstring(content)

def parse_xml_file(path):
    with open(path, "rb") as f:
        return etree.fromstring(f.read())

def feed_filename_from_url(url):
    return os.path.basename(urlparse(url).path)

# =========================
# HLAVNÍ LOGIKA
# =========================

def main():
    ensure_dirs()

    root = None
    if not os.path.exists(ROOT_FEED_FILE):
        print("📥 Stahuji hlavní Atom feed...")
        root_xml = requests.get(ROOT_FEED_URL, headers=HEADERS, timeout=TIMEOUT).content
        with open(ROOT_FEED_FILE, "wb") as f:
            f.write(root_xml)
        root = parse_xml(root_xml)
    else:
        print("📄 Načítám hlavní Atom feed ze souboru...")
        root = parse_xml_file(ROOT_FEED_FILE)

    if root is None:
        return -1
    

    ns = {
        "atom": "http://www.w3.org/2005/Atom"
    }

    dataset_links = root.xpath("//atom:entry/atom:link[@rel='alternate']/@href", namespaces=ns)

    print(f"🗂 Nalezeno mapových listů: {len(dataset_links)}")

    for dataset_feed_url in tqdm(dataset_links, desc="Mapové listy"):
        try:
            feed_name = feed_filename_from_url(dataset_feed_url)
            feed_path = os.path.join(FEED_DIR, feed_name)

            # 1️⃣ Dataset feed – stáhnout / použít cache
            if not os.path.exists(feed_path):
                dataset_xml = requests.get(dataset_feed_url, headers=HEADERS, timeout=TIMEOUT).content
                with open(feed_path, "wb") as f:
                    f.write(dataset_xml)
            else:
                with open(feed_path, "rb") as f:
                    dataset_xml = f.read()

            dataset = parse_xml(dataset_xml)

            zip_links = dataset.xpath(
                "//atom:entry/atom:link[@rel='alternate']/@href",
                namespaces=ns
            )

            if not zip_links:
                processed += 1
                print("⚠️ ZIP nenalezen, přeskočeno")
                continue

            zip_url = zip_links[0]
            zip_name = os.path.basename(urlparse(zip_url).path)
            zip_path = os.path.join(ZIP_DIR, zip_name)

            # Odvozené laz jméno
            laz_file = os.path.join(LAZ_DIR, zip_name.replace(".zip", ".laz"))
            # zip se stáhne pouze pokud ještě není laz (ani zip)
            if not os.path.exists(laz_file):
                download(zip_url, zip_path)
                unzip(zip_path, LAZ_DIR)

        except Exception as e:
            print(f"⚠️ Chyba: {dataset_feed_url} → {e}")

    print("✅ Hotovo")

if __name__ == "__main__":
    main()
