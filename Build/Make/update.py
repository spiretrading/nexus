import argparse
import distutils
from distutils import dir_util
import HTMLParser
import os
import re
import shutil
import subprocess
import urllib2

def call(command):
  return subprocess.Popen(command, shell=True, executable='/bin/bash',
    stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()

def user_call(command):
  return call('sudo -u $(logname) %s' % command)

class DirectoryHTMLParser(HTMLParser.HTMLParser):
  def __init__(self):
    HTMLParser.HTMLParser.__init__(self)
    self.link = None

  def handle_starttag(self, tag, attrs):
    if tag == 'a':
      for attr in attrs:
        if len(attr) > 1 and attr[0] == 'href':
          link = attr[1]
          m = re.search(r'(\d+)', link)
          if m is not None and (self.link is None or int(m.group(1)) >
              int(self.link)):
            self.link = m.group(1)

class FileHTMLParser(HTMLParser.HTMLParser):
  def __init__(self):
    HTMLParser.HTMLParser.__init__(self)
    self.link = None

  def handle_starttag(self, tag, attrs):
    if tag == 'a':
      for attr in attrs:
        if len(attr) > 1 and attr[0] == 'href':
          link = attr[1]
          m = re.search(r'nexus-\d+\.tar\.gz', link)
          if m is not None:
            self.link = m.group(0)

def main():
  parser = argparse.ArgumentParser(
    description='v1.0 Copyright (C) 2017 Eidolon Systems Ltd.')
  parser.add_argument('-s', '--site', type=str, help='Site hosting the build.',
    default='http://builds.spiretrading.com')
  args = parser.parse_args()
  response = urllib2.urlopen('%s/Nexus/Make' % args.site)
  parser = DirectoryHTMLParser()
  parser.feed(response.read())
  if parser.link is None:
    print 'Build not found.'
    return
  build_version = parser.link
  response = urllib2.urlopen('%s/Nexus/Make/%s' % (args.site, build_version))
  parser = FileHTMLParser()
  parser.feed(response.read())
  if parser.link is None:
    print 'Archive not found.'
    return
  archive = parser.link
  response = urllib2.urlopen('%s/Nexus/Make/%s/%s' %
    (args.site, build_version, archive))
  user_call('touch %s' % archive)
  output = open(archive, 'wb')
  output.write(response.read())
  output.close()
  user_call('tar -xzf %s' % archive)
  python_path = user_call('python -m site --user-site')[0].strip()
  user_call('cp ./Nexus/Libraries/*.so %s' % python_path)
  user_call('rm -rf ./Nexus/Libraries/')
  user_call('rm %s' % archive)

if __name__ == '__main__':
  main()
