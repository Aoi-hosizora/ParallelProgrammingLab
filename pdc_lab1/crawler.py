from typing import List, Tuple
from bs4 import BeautifulSoup
import requests
import os
from os import path
import threading
import json

START_URL = 'https://news.mit.edu/'
MAX_THREAD = 16
TIMEOUT_SECOND = 5
SAVE_DIR = './result'
MAP_FILE = 'urls.json'

id2url_map = {}
id2url_lock = threading.Lock()


def main():
    if not path.exists(SAVE_DIR):
        os.makedirs(SAVE_DIR)
    if not path.isdir(SAVE_DIR):
        print(f'> Error: {SAVE_DIR} is not directory.')
        exit(-1)

    # get the first page
    urls = pipeline(START_URL)
    if not urls:
        print(f'Finished.')
        return

    # split to threads
    visited = [START_URL]
    mu = threading.Lock()
    if len(urls) > MAX_THREAD:
        queues = [[url] for url in urls[:MAX_THREAD]]
        for idx, url in enumerate(urls[MAX_THREAD:]):
            queues[idx % MAX_THREAD].append(url)
    else:
        queues = [[url] for url in urls]

    # thread handler
    def handler(queue: List[str]):
        while queue:
            url = queue.pop(0)
            with mu:
                if url in visited:
                    continue
                visited.append(url)
            urls = pipeline(url)
            if urls:
                queue.extend(urls)

    # start threads
    ths = []
    for queue in queues:
        th = threading.Thread(target=handler, kwargs={'queue': queue})
        ths.append(th)
        th.start()
    for th in ths:
        th.join()

    # unreachable
    print(f'Finished.')


def pipeline(url: str) -> List[str]:
    ok, content_or_error = http_get(url)
    if not ok:
        print(f'> Error: failed to get {url}: {content_or_error}.')
        return None
    with id2url_lock:
        global_id = len(id2url_map) + 1
        id2url_map[global_id] = url
        if global_id % 100 == 0:
            json_content = json.dumps(id2url_map, ensure_ascii=False, indent=2)
            with open(MAP_FILE, 'w+', encoding='utf-8') as f:
                f.write(json_content)
            print(f'> Info: id2url_map has been saved to {MAP_FILE}.')
    with open(path.join(SAVE_DIR, f'{global_id}.html'), 'w+', encoding='utf-8') as f:
        f.write(content_or_error)
    print(f'# = {global_id}, {url}')
    return get_urls(content_or_error)


def http_get(url: str) -> Tuple[bool, str]:
    ua = 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/100.0.4896.127 Safari/537.36'
    try:
        with requests.get(url, headers={'User-Agent': ua}, timeout=TIMEOUT_SECOND) as resp:
            if resp.status_code != 200:
                return False, 'status code is not 200'
            if not resp.headers['Content-Type'].startswith('text/html'):
                return False, 'content type is not text/html'
            content = resp.content.decode('utf-8').strip()
            if not content:
                return False, 'html page is empty'
            return True, content
    except Exception as ex:
        return False, str(ex)


def get_urls(content: str) -> List[str]:
    soup = BeautifulSoup(content, 'lxml')
    hrefs = []
    for node in soup.find_all('a', href=True):
        href = str(node['href']).strip().rstrip('/').replace('\n', '')
        hrefs.append(href)
    return hrefs


if __name__ == '__main__':
    main()
