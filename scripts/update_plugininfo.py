import requests
import os
import sys
import yaml
from datetime import datetime

ORG = "ManiVaultStudio"
REPO = sys.argv[1] if len(sys.argv) > 1 else "Scatterplot"
OUTPUT_PATH = f"{REPO.lower()}.md"
GITHUB_TOKEN = os.getenv("GITHUB_TOKEN")

HEADERS = {"Accept": "application/vnd.github+json"}
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
def fetch_readme_excerpt(lines=5):
    """Return first `lines` lines from README.md (skipping initial heading)."""
    url = f"https://raw.githubusercontent.com/{ORG}/{REPO}/master/README.md"
    r = requests.get(url)
    if r.status_code != 200:
        return ""

    all_lines = r.text.splitlines()

    # Skip the first heading line (starts with '#')
    filtered = []
    heading_skipped = False
    for line in all_lines:
        if not heading_skipped and line.strip().startswith("#"):
            heading_skipped = True
            continue
        filtered.append(line)
        if len(filtered) >= lines:
            break

    return "\n".join(filtered)

# ---------- Write output markdown ----------
def write_markdown(info, authors, readme_snippet=""):
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

    with open(OUTPUT_PATH, "w", encoding="utf-8") as f:
        f.write("---\n")
        yaml.dump(metadata, f, sort_keys=False, allow_unicode=True)
        f.write("---\n\n")
        if readme_snippet:
            f.write(readme_snippet + "\n")

    print(f" Updated {OUTPUT_PATH}")

if __name__ == "__main__":
    info = fetch_plugin_info()
    authors = fetch_contributors()
    readme_excerpt = fetch_readme_excerpt(lines=5)
    write_markdown(info, authors, readme_snippet=readme_excerpt)
