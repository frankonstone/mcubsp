"""ESP-IDF dependency checks and its virtual-environment convention."""
from pathlib import Path


def environment(state, names):
    if 'idf-python' not in names:
        return {}
    python = Path(state['tools']['idf-python']['path'])
    return {'IDF_PYTHON_ENV_PATH': str(python.parent.parent)}


def check(state, names, run, environment):
    if not {'esp-idf', 'idf-python', 'idf-tools'} <= set(names):
        return
    selected = state['tools']
    sdk = Path(selected['esp-idf']['path'])
    try:
        run([selected['idf-python']['path'], str(sdk / 'tools/idf_tools.py'),
             '--idf-path', str(sdk), 'check-python-dependencies'], env=environment)
    except ValueError as error:
        raise ValueError(f'ESP-IDF Python dependencies are incomplete; repair the pinned SDK environment and rerun setup. {error}') from error
