from typing import List
import json
from bs4 import BeautifulSoup
import os
from os import path
import threading
import re

HTML_DIR = './result'
TXT_DIR = './result_txt'
MAP_FILE = 'urls.json'
MAX_THREAD = 8
NONEED_RE = re.compile(r'[^\w\s]')


def main():
    if not path.isdir(HTML_DIR):
        print(f'> Error: {HTML_DIR} is not found, or not a directory.')
        exit(-1)
    if not path.exists(TXT_DIR):
        os.makedirs(TXT_DIR)
    if not path.isdir(TXT_DIR):
        print(f'> Error: {TXT_DIR} is not directory.')
        exit(-1)

    # read map file
    with open(MAP_FILE, 'r', encoding='utf-8') as f:
        json_content = f.read()
    try:
        id2url_map = json.loads(json_content)
    except Exception as ex:
        print(f'> Error: {MAP_FILE} is invalid: {ex}')
        exit(-1)

    # list html filenames and split to threads
    filenames = os.listdir(HTML_DIR)
    file_ids_lists = [[] for _ in range(MAX_THREAD)]
    for idx, filename in enumerate(filenames):
        sp = path.splitext(filename)
        if sp[1] != '.html':
            continue
        file_id = sp[0]
        if file_id not in id2url_map:
            print(f'> Warning: {file_id}.html is not found in {MAP_FILE}')
            continue
        file_ids_lists[idx % MAX_THREAD].append(file_id)

# thread handler
def handler(file_ids: List[int]):
    for file_id in file_ids:
        html_filepath = path.join(HTML_DIR, f'{file_id}.html')
        with open(html_filepath, 'r', encoding='utf-8') as f:
            html_content = f.read()
        words = get_words(html_content)  # maybe empty
        txt_filepath = path.join(TXT_DIR, f'{file_id}.txt')
        with open(txt_filepath, 'w+', encoding='utf-8') as f:
            content = '{}；{}'.format(id2url_map[file_id].strip(), " ".join(words))  # 注: 全角分号
            f.write(content.replace('\n', ''))
        print(f'Info: {txt_filepath} has been written.')

    # start threads
    ths = []
    for file_ids in file_ids_lists:
        th = threading.Thread(target=handler, kwargs={'file_ids': file_ids})
        ths.append(th)
        th.start()
    for th in ths:
        th.join()

    print(f'Finished.')


def get_words(content: str) -> List[str]:
    soup = BeautifulSoup(content, 'lxml')
    content = soup.get_text()
    return NONEED_RE.sub(' ', content).split()


if __name__ == '__main__':
    main()
