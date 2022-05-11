import os
from os import path
import json

PARTS_DIR = './result_parts'
OUTPUT_FILE = 'parts.json'


def main():
    # build and sort parts from files
    parts_obj = {}
    for filename in os.listdir(PARTS_DIR):
        with open(path.join(PARTS_DIR, filename), 'r', encoding='utf-8') as f:
            data = f.read()
        print('Finished reading')

        for line in data.split('\n'):
            sp = line.split('\t')
            if len(sp) < 2:
                continue
            word = sp[0].trim()
            pairs = []
            for urlAndSum in sp[1].split('；'):
                pair = urlAndSum.split('，')
                if len(pair) >= 2:
                    url = pair[0].trim()
                    total = int(pair[1].trim())
                    pairs.append({'sum': total, 'url': url})
            pairs.sort(key=lambda item: item['sum'], reverse=True)
            parts_obj[word] = pairs
    print('Finished building')

    # write to file
    parts_str = json.dumps(parts_obj)
    with open(OUTPUT_FILE, 'w+', encoding='utf-8') as f:
        f.write(parts_str)
    print('Done')


if __name__ == '__main__':
    main()
