import os
import sys
import requests
import yaml
from datetime import datetime

# ---------- Resolve owner/repo ----------
def resolve_owner_repo():
    """
    Accepts:
      - argv[1] as 'owner/repo' or 'repo'
      - else uses GITHUB_REPOSITORY env ('owner/repo')
    Raises if neither present.
    """
    if len(sys.argv) > 1:
        arg = sys.argv[1].strip()
        if "/" in arg:
            owner, repo = arg.split("/", 1)
        else:
            owner = os.getenv("GITHUB_REPOSITORY", "").split("/")[0] or "ManiVaultStudio"
            repo = arg
        return owner, repo

    env_repo = os.getenv("GITHUB_REPOSITORY", "")
    if env_repo and "/" in env_repo:
        owner, repo = env_repo.split("/", 1)
        return owner, repo

    raise SystemExit("❌ Repo not specified. Pass '<owner>/<repo>' or set GITHUB_REPOSITORY.")

OWNER, REPO = resolve_owner_repo()
OUTPUT_PATH = f"{REPO.lower()}.md"

GITHUB_TOKEN = os.getenv("GITHUB_TOKEN")
HEADERS = {"Accept": "application/vnd.github+json"}
if GITHUB_TOKEN:
    HEADERS["Authorization"] = f"Bearer {GITHUB_TOKEN}"

# ---------- Helpers ----------
def fetch_default_branch():
    url = f"https://api.github.com/repos/{OWNER}/{REPO}"
    r = requests.get(url, headers=HEADERS)
    r.raise_for_status()
    return r.json().get("default_branch", "master")

def fetch_plugin_info(branch):
    url = f"https://raw.githubusercontent.com/{OWNER}/{REPO}/{branch}/PluginInfo.json"
    r = requests.get(url)
    r.raise_for_status()
    return r.json()

def fetch_contributors():
    url = f"https://api.github.com/repos/{OWNER}/{REPO}/contributors"
    r = requests.get(url, headers=HEADERS)
    r.raise_for_status()
    return [user["login"] for user in r.json()]

def fetch_readme_full(branch):
    """Return full README.md content (skip first heading line if present)."""
    raw_urls = [
        f"https://raw.githubusercontent.com/{OWNER}/{REPO}/{branch}/README.md",
        f"https://raw.githubusercontent.com/{OWNER}/{REPO}/{branch}/readme.md",
    ]
    for url in raw_urls:
        r = requests.get(url)
        if r.status_code == 200:
            lines = r.text.splitlines()
            if lines and lines[0].strip().startswith("#"):
                lines = lines[1:]
            return "\n".join(lines).strip()
    return ""

# ---------- Write output markdown ----------
def write_markdown(info, authors, readme_text=""):
    metadata = {
        "layout": "plugin",
        "name": info["name"],
        "shortname": info["name"].split()[0],
        "key": REPO.lower(),
        "type": info.get("type", "").lower(),
        "version": info["version"]["plugin"],
        "date": datetime.utcnow().strftime("%Y-%m-%d %H:%M:%S"),
        "organization": "ManiVault",
        "organization-link": "https://www.manivault.studio",
        "authors": authors,
        # NOTE: shortdescription intentionally removed (derived on site)
        "repo": f"https://github.com/{OWNER}/{REPO}",
    }

    # Build page body: README + repo link footer
    body_parts = []
    if readme_text:
        body_parts.append(readme_text)
    body_parts.append(f"\n---\n**Source repository:** https://github.com/{OWNER}/{REPO}\n")
    body = "\n".join(body_parts).lstrip("\n")

    with open(OUTPUT_PATH, "w", encoding="utf-8") as f:
        f.write("---\n")
        yaml.dump(metadata, f, sort_keys=False, allow_unicode=True)
        f.write("---\n\n")
        f.write(body)

    print(f"✅ Updated {OUTPUT_PATH} for {OWNER}/{REPO}")

# ---------- Main ----------
if __name__ == "__main__":
    branch = fetch_default_branch()
    info = fetch_plugin_info(branch)
    authors = fetch_contributors()
    readme_text = fetch_readme_full(branch)
    write_markdown(info, authors, readme_text=readme_text)
