import importlib.util
import json
import os
import re
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


def translate(source, variables):
  placeholder = r'\$([A-Za-z_][A-Za-z_0-9]*)'

  def encode(value):
    return json.dumps(str(value), ensure_ascii=False).replace(
      '\x85', r'\u0085').replace('\u2028', r'\u2028').replace(
      '\u2029', r'\u2029')

  def replace(match):
    token = match.group()
    if token.startswith('"'):
      return re.sub(placeholder, lambda item:
        encode(variables[item[1]])[1:-1] if item[1] in variables
          else item[0], token)
    if token.startswith("'"):
      value = token[1:-1].replace("''", "'")
      value = re.sub(placeholder, lambda item:
        str(variables[item[1]]) if item[1] in variables else item[0], value)
      return encode(value)
    if token[1:] in variables:
      return encode(variables[token[1:]])
    return token

  return re.sub(r'"(?:[^"\\]|\\.)*"|\'(?:[^\']|\'\')*\'|' + placeholder,
    replace, source)
