import argparse
import distutils
from distutils import dir_util
import errno
import git
import os
import shutil
import subprocess
import sys
import tarfile
import time
import zipfile

def call(command, cwd=None):
  return subprocess.Popen(command.split(), stdout=subprocess.PIPE, cwd=cwd,
    stderr=subprocess.PIPE).communicate()

def makedirs(path):
  try:
    os.makedirs(path)
  except OSError as e:
    if e.errno != errno.EEXIST:
      raise

def make_tarfile(source, destination):
  with tarfile.open(destination, "w:gz") as tar:
    for file in os.listdir(source):
      tar.add(os.path.join(source, file), arcname=file)

def make_zipfile(source, destination):
  archive = zipfile.ZipFile(destination, 'w', zipfile.ZIP_DEFLATED)
  for root, dirs, files in os.walk(source):
    for file in files:
      archive.write(os.path.join(root, file))
  archive.close()

def copy_build(applications, timestamp, name, source, path):
  try:
    destination_path = os.path.join(path, str(timestamp))
    for application in applications:
      application_path = os.path.join(destination_path, application)
      makedirs(application_path)
      source_directory = os.path.join(source, 'Applications', application,
        'Application')
      for file in os.listdir(source_directory):
        file_path = os.path.join(source_directory, file)
        if os.path.isfile(file_path):
          shutil.copy2(file_path, os.path.join(application_path, file))
    library_destination_path = os.path.join(destination_path, 'Libraries')
    makedirs(library_destination_path)
    library_source_path = os.path.join(source, name, 'Libraries', 'Release')
    for file in os.listdir(library_source_path):
      shutil.copy2(os.path.join(library_source_path, file),
        os.path.join(library_destination_path, file))
  except OSError:
    return

def build_repo(repo, path, branch):
  commits = sorted([commit for commit in repo.iter_commits(branch)],
    key = lambda commit: -int(commit.committed_date))
  builds = [int(d) for d in os.listdir(path) if os.path.isdir(
    os.path.join(path, d))]
  builds.sort(reverse=True)
  if len(builds) == 0:
    builds.append(int(commits[1].committed_date))
  for i in range(len(commits)):
    timestamp = int(commits[i].committed_date)
    if timestamp in builds:
      commits = commits[0:i]
      commits.reverse()
      break
  if sys.platform == 'win32':
    extension = 'bat'
  else:
    extension = 'sh'
  for commit in commits:
    build_path = os.path.join(os.getcwd(), 'build')
    shutil.rmtree(build_path, True)
    makedirs(build_path)
    timestamp = int(commit.committed_date)
    repo.git.checkout(commit.hexsha)
    result = []
    result.append(call(os.path.join(repo.working_dir, 'configure.%s -DD=%s' %
      (extension, os.path.join(os.getcwd(), 'Dependencies'))), build_path))
    result.append(call(os.path.join(build_path, 'build.%s' % extension),
      build_path))
    terminal_output = b''
    for output in result:
      terminal_output += output[0] + b'\n\n\n\n'
    for output in result:
      terminal_output += output[1] + b'\n\n\n\n'
    destination_path = os.path.join(path, str(timestamp))
    makedirs(destination_path)
    nexus_applications = ['AdministrationServer', 'AsxItchMarketDataFeedClient',
      'ChartingServer', 'ChiaMarketDataFeedClient', 'ComplianceServer',
      'CseMarketDataFeedClient', 'CtaMarketDataFeedClient', 'DefinitionsServer',
      'MarketDataRelayServer', 'MarketDataServer', 'RiskServer',
      'SimulationMarketDataFeedClient', 'SimulationOrderExecutionServer',
      'TmxIpMarketDataFeedClient', 'TmxTl1MarketDataFeedClient',
      'UtpMarketDataFeedClient', 'WebPortal']
    if sys.platform == 'win32':
      nexus_applications.append('Spire')
    copy_build(nexus_applications, timestamp, 'Nexus', build_path, path)
    beam_applications = ['AdminClient', 'RegistryServer', 'ServiceLocator',
      'UidServer']
    beam_path = os.path.join(os.getcwd(), 'Dependencies', 'Beam')
    copy_build(beam_applications, timestamp, 'Beam', beam_path, path)
    shutil.copy2(os.path.join(repo.working_dir, 'Applications', 'setup.py'),
      os.path.join(destination_path, 'setup.py'))
    if sys.platform == 'win32':
      archive_path = os.path.join(path, 'nexus-%s.zip' % str(timestamp))
      make_zipfile(destination_path, archive_path)
    else:
      for file in ['check.sh', 'copy_all.sh', 'start.sh', 'stop.sh']:
        shutil.copy2(os.path.join(repo.working_dir, 'Applications', file),
          os.path.join(destination_path, file))
      archive_path = os.path.join(path, 'nexus-%s.tar.gz' % str(timestamp))
      make_tarfile(destination_path, archive_path)
    shutil.rmtree(beam_path)
    shutil.rmtree(build_path)
    shutil.rmtree(destination_path)
    makedirs(destination_path)
    with open(os.path.join(destination_path, 'build.txt'), 'wb') as log_file:
      log_file.write(terminal_output)
    shutil.move(archive_path, destination_path)

def main():
  parser = argparse.ArgumentParser(
    description='v1.0 Copyright (C) 2019 Eidolon Systems Ltd.')
  parser.add_argument('-r', '--repo', type=str, help='Remote repository.',
    default='https://github.com/eidolonsystems/nexus.git')
  parser.add_argument('-p', '--path', type=str, help='Destination path.',
    required=True)
  parser.add_argument('-b', '--branch', type=str, help='Branch to build.',
    default='master')
  parser.add_argument('-t', '--period', type=int, help='Time period.',
    default=600)
  args = parser.parse_args()
  repo_path = os.path.join(os.getcwd(), 'Nexus')
  shutil.rmtree(repo_path, True)
  repo = git.Repo.clone_from(args.repo, repo_path)
  makedirs(args.path)
  while True:
    try:
      repo.git.checkout(args.branch)
      repo.git.pull()
    except:
      print('Failed to pull: ', sys.exc_info()[0])
    build_repo(repo, args.path, args.branch)
    time.sleep(args.period)

if __name__ == '__main__':
  main()
