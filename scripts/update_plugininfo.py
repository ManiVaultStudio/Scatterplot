import requests
import os
import sys
import yaml
from datetime import datetime

ORG = "ManiVaultStudio"
REPO = sys.argv[1] if len(sys.argv) > 1 else "Scatterplot"
OUTPUT_PATH = f"{REPO.lower()}.md"
GITHUB_TOKEN = os.getenv("GITHUB_TOKEN")

HEADERS = {
    "Accept": "application/vnd.github+json",
}
if GITHUB_TOKEN:
    HEADERS["Authorization"] = f"Bearer {GITHUB_TOKEN}"

# ---------- Fetch base info ----------
def fetch_plugin_info():
    url = f"https://raw.githubusercontent.com/{ORG}/{REPO}/master/PluginInfo.json"
    r = requests.get(url)
    r.raise_for_status()
    return r.json()

def fetch_contributors():
    url = f"https://api.github.com/repos/{ORG}/{REPO}/contributors"
    r = requests.get(url, headers=HEADERS)
    r.raise_for_status()
    return [user["login"] for user in r.json()]


# ---------- Fetch README content ----------
def fetch_readme_excerpt(max_lines=5):
    """
    Fetch the README content and return the first few non-empty lines as a string.
    Falls back gracefully if README not found.
    """
    url = f"https://raw.githubusercontent.com/{ORG}/{REPO}/master/README.md"
    r = requests.get(url)
    if r.status_code != 200:
        print("⚠️  No README.md found or couldn't fetch it.")
        return None

    lines = r.text.splitlines()
    excerpt = []
    for line in lines:
        clean = line.strip()
        if clean:  # skip empty lines
            excerpt.append(clean)
        if len(excerpt) >= max_lines:
            break

    if not excerpt:
        return None
    return " ".join(excerpt)

# ---------- Write output markdown ----------
def write_markdown(info, authors, longdescription=None):
    metadata = {
        "layout": "plugin",
        "name": info["name"],
        "shortname": info["name"].split()[0],
        "key": REPO.lower(),
        "type": info.get("type", "").lower(),
        "version": info["version"]["plugin"],
        "date": datetime.today().strftime("%Y-%m-%d %H:%M:%S"),
        "organization": "ManiVault",
        "organization-link": "https://www.manivault.studio",
        "authors": authors,
        "shortdescription": f"{info['type']} plugin with dependencies: {', '.join(info.get('dependencies', []))}",
    }

    if longdescription:
        metadata["longdescription"] = longdescription

    with open(OUTPUT_PATH, "w") as f:
        f.write("---\n")
        yaml.dump(metadata, f, sort_keys=False)
        f.write("---\n")

    print(f"✅ Updated {OUTPUT_PATH}")
    if longdescription:
        print("📝 Added longdescription from README.md")

if __name__ == "__main__":
    info = fetch_plugin_info()
    authors = fetch_contributors()
    readme_excerpt = fetch_readme_excerpt(max_lines=5)
    write_markdown(info, authors, longdescription=readme_excerpt)