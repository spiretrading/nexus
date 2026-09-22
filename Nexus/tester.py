import argparse
import importlib.machinery
import importlib.util
import os
import sys
import unittest

ROOT = os.path.dirname(os.path.realpath(__file__))
DIRECTORY = os.path.join(ROOT, 'Source', 'PythonTests')
CONFIGURATIONS = ('Debug', 'Release', 'RelWithDebInfo', 'MinSizeRel')
MODULES = ('nexus.pyd', 'nexus.so')


def _roots():
  roots = []
  for root in (os.getcwd(), ROOT):
    root = os.path.abspath(root)
    if root not in roots:
      roots.append(root)
  return roots


def _built_module(configuration):
  for root in _roots():
    selected = configuration
    config = os.path.join(root, 'CMakeFiles', 'config.txt')
    if not selected:
      if not os.path.isfile(config):
        continue
      with open(config) as source:
        selected = source.read().strip()
    if selected not in CONFIGURATIONS:
      return None
    for module in MODULES:
      path = os.path.join(root, 'Libraries', selected, module)
      if os.path.isfile(path):
        return path
    if os.path.isfile(config) or os.path.isdir(os.path.join(root, 'Libraries')):
      return None
  return None


def _load_module(name, path):
  loader = importlib.machinery.ExtensionFileLoader(name, path)
  spec = importlib.util.spec_from_file_location(name, path, loader=loader)
  module = importlib.util.module_from_spec(spec)
  sys.modules[name] = module
  loader.exec_module(module)
  print('Testing {}'.format(module.__file__), flush=True)


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('prefix', nargs='?', help='Test filename prefix.')
  parser.add_argument('--config', choices=CONFIGURATIONS,
    help='Build configuration (defaults to the saved build configuration).')
  args = parser.parse_args()
  if args.prefix:
    pattern = '{}*.py'.format(args.prefix)
  else:
    pattern = 'test_*.py'
  module = _built_module(args.config)
  if not module:
    print('No Nexus module found for the selected build configuration. '
      'Run build or specify --config.', file=sys.stderr)
    return 1
  directory = os.path.dirname(module)
  suffix = os.path.splitext(module)[1]
  modules = [(name, os.path.join(directory, name + suffix))
    for name in ('aspen', 'beam', 'nexus')]
  for name, path in modules:
    if not os.path.isfile(path):
      print('Missing built module: {}.'.format(path), file=sys.stderr)
      return 1
  for name, path in modules:
    _load_module(name, path)
  sys.path.insert(0, DIRECTORY)
  tests = unittest.defaultTestLoader.discover(DIRECTORY, pattern=pattern)
  if tests.countTestCases() == 0:
    print('No tests matched {}.'.format(pattern), file=sys.stderr)
    return 1
  result = unittest.TextTestRunner(verbosity=2).run(tests)
  return 0 if result.wasSuccessful() else 1


if __name__ == '__main__':
  sys.exit(main())
