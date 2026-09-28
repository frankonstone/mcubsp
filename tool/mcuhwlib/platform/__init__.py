"""Small adapters for SDK behavior that cannot be declared in the tool lock."""
from . import esp_idf

_ADAPTERS = {'esp-idf': esp_idf}


def _adapter(name):
    try:
        return _ADAPTERS[name]
    except KeyError as error:
        raise ValueError(f'Unknown tool platform adapter: {name}') from error


def check(name, state, names, run, environment):
    _adapter(name).check(state, names, run, environment)


def environment(name, state, names):
    return _adapter(name).environment(state, names)
