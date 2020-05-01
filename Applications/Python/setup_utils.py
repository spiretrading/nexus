import importlib.util
import os
import shutil
import socket
import sys
from unittest.mock import patch

def get_ip():
  with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as test_socket:
    try:
      test_socket.connect(('10.255.255.255', 1))
      return test_socket.getsockname()[0]
    except:
      return '127.0.0.1'


def run_subscript(path, arguments):
  try:
    spec = importlib.util.spec_from_file_location('subscript', path)
    subscript = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(subscript)
    with patch.object(sys, 'argv', [path] + arguments):
      subscript.main()
  finally:
    shutil.rmtree(os.path.join(os.path.dirname(path), '__pycache__'),
      ignore_errors=True)


def needs_quotes(value):
  if value.strip() != value or len(value) == 0:
    return True
  special_characters = [':', '{', '}', '[', ']', ',', '&', '*', '#', '?', '|',
    '-', '<', '>', '=', '!', '%', '@', '\\']
  for c in value:
    if c in special_characters:
      return True
  return False


def translate(source, variables):
  for key in variables.keys():
    if needs_quotes(variables[key]):
      index = source.find('$' + key)
      while index != -1:
        c = source.rfind('\n', 0, index) + 1
        q = False
        while c < index:
          if source[c] == '\"':
            q = not q
          c += 1
        if q:
          source = source.replace('$' + key, '%s' % variables[key], 1)
        else:
          source = source.replace('$' + key, '"%s"' % variables[key], 1)
        index = source.find('$' + key, index + 1)
    else:
      source = source.replace('$' + key, '%s' % variables[key])
  return source
