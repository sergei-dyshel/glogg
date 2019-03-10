#!/usr/bin/env python3

import json
import sys
try:
    import yaml
except ImportError:
    print('yaml module is required (pip install pyyaml)')
    sys.exit(1)
try:
    import jsonschema
except ImportError:
    print('jsonschema module is required (pip install jsonschema)')
    sys.exit(1)

def main():
    schema_path = sys.argv[1]
    docs = sys.argv[2:]
    schema = json.load(open(schema_path))
    for doc in docs:
        doc = yaml.load(open(doc))
        jsonschema.validate(instance=doc, schema=schema)

if __name__ == '__main__':
    main()