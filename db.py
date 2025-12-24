import json
from pathlib import Path

CORPUS_DIR = Path("corpus")


def ensure_corpus():
    CORPUS_DIR.mkdir(exist_ok=True)


def doc_path(doc_id):
    return CORPUS_DIR / f"{doc_id}.txt"


def meta_path(doc_id):
    return CORPUS_DIR / f"{doc_id}.meta.json"


def exists(doc_id):
    return doc_path(doc_id).is_file()


def load_meta(doc_id):
    p = meta_path(doc_id)
    if not p.is_file():
        return None
    with open(p, "r", encoding="utf-8") as f:
        return json.load(f)


def save(doc_id, text, meta):
    ensure_corpus()
    with open(doc_path(doc_id), "w", encoding="utf-8") as f:
        f.write(text)
    with open(meta_path(doc_id), "w", encoding="utf-8") as f:
        json.dump(meta, f, indent=2, ensure_ascii=False)
