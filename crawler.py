#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import json
import hashlib
import time
import xml.etree.ElementTree as ET
from pathlib import Path
import requests
from tqdm import tqdm

CORPUS_DIR = Path("corpus")
CORPUS_DIR.mkdir(exist_ok=True)

SOURCES = {
    "ai": "https://archive.org/download/stackexchange/ai.stackexchange.com.7z",
    "ds": "https://archive.org/download/stackexchange/datascience.stackexchange.com.7z"
}

REQUEST_DELAY = 1.0
MIN_WORDS = 100
MAX_DOCS_PER_SOURCE = 15000


def download_archive(url, dest):
    if dest.exists():
        return
    with requests.get(url, stream=True) as r:
        r.raise_for_status()
        total = int(r.headers.get("content-length", 0))
        with open(dest, "wb") as f, tqdm(
            desc=dest.name,
            total=total,
            unit="B",
            unit_scale=True,
            unit_divisor=1024,
        ) as bar:
            for chunk in r.iter_content(8192):
                f.write(chunk)
                bar.update(len(chunk))


def extract_posts_xml(archive_path, extract_to):
    if extract_to.exists():
        xml_path = extract_to / "Posts.xml"
        if xml_path.exists():
            return xml_path
    try:
        import py7zr
        with py7zr.SevenZipFile(archive_path, mode="r") as z:
            z.extractall(path=extract_to)
    except ImportError:
        sys.exit("pip install py7zr")
    xml_path = extract_to / "Posts.xml"
    if not xml_path.exists():
        raise FileNotFoundError(f"Posts.xml не найден в {extract_to}")
    return xml_path


def clean_html_text(html):
    import re
    return re.sub(r"<[^>]+>", "", html)


def process_posts_xml(xml_path, source_prefix, max_docs):
    tree = ET.parse(xml_path)
    root = tree.getroot()
    saved = 0
    for post in root.findall("row"):
        if saved >= max_docs:
            break
        if post.get("PostTypeId") != "1":
            continue
        post_id = post.get("Id")
        title = post.get("Title", "").strip()
        body = post.get("Body", "").strip()
        if not title or not body:
            continue
        clean_body = clean_html_text(body)
        full_text = f"{title}\n\n{clean_body}"
        if len(full_text.split()) < MIN_WORDS:
            continue
        doc_id = f"{source_prefix}_{post_id}"
        text_path = CORPUS_DIR / f"{doc_id}.txt"
        meta_path = CORPUS_DIR / f"{doc_id}.meta.json"
        content_hash = hashlib.md5(full_text.encode("utf-8")).hexdigest()
        meta = {
            "url": f"https://{source_prefix}.stackexchange.com/questions/{post_id}",
            "title": title,
            "source": "stackexchange",
            "fetch_time": int(time.time()),
            "content_hash": content_hash,
            "word_count": len(full_text.split()),
        }
        with open(text_path, "w", encoding="utf-8") as f:
            f.write(full_text)
        with open(meta_path, "w", encoding="utf-8") as f:
            json.dump(meta, f, indent=2, ensure_ascii=False)
        saved += 1
        if saved % 1000 == 0:
            print(f"  → Сохранено {saved} документов...")
        time.sleep(REQUEST_DELAY)
    print(f"Завершено: {saved} документов из {source_prefix}")
    return saved


def main():
    total_saved = 0
    for alias, archive_url in SOURCES.items():
        print(f"\n{'='*60}")
        print(f"Обработка источника: {alias}")
        print('='*60)
        archive_path = Path(f"{alias}.stackexchange.com.7z")
        extract_dir = Path(f"tmp_{alias}")
        download_archive(archive_url, archive_path)
        xml_path = extract_posts_xml(archive_path, extract_dir)
        count = process_posts_xml(xml_path, alias, MAX_DOCS_PER_SOURCE)
        total_saved += count
    print(f"\n Всего сохранено: {total_saved} документов")
    print(f" Корпус: {CORPUS_DIR.absolute()}")


if __name__ == "__main__":
    main()
