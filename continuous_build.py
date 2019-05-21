import argparse
import distutils
from distutils import dir_util
import errno
import git
import os
import shutil
import subprocess
import sys
import time

def call(command, cwd=None):
  return subprocess.Popen(command, stdout=subprocess.PIPE, cwd=cwd,
    stderr=subprocess.PIPE).communicate()

def copy_build(applications, timestamp, source, path):
  destination_path = os.path.join(path, str(timestamp))
  for application in applications:
    application_path = os.path.join(destination_path, application)
    os.makedirs(application_path)
    source_directory = os.path.join(source, 'Applications', application,
      'Application')
    for file in os.listdir(source_directory):
      file_path = os.path.join(source_directory, file)
      if os.path.isfile(file_path):
        shutil.copy2(file_path, os.path.join(application_path, file))
  library_destination_path = os.path.join(destination_path, 'Libraries')
  os.makedirs(library_destination_path)
  library_source_path = os.path.join(source, 'Libraries', 'Release')
  for file in os.listdir(library_source_path):
    shutil.copy2(os.path.join(library_source_path, file),
      os.path.join(library_destination_path, file))

def build_repo(repo, path):
  try:
    os.makedirs(path)
  except OSError as e:
    if e.errno != errno.EEXIST:
      raise
  call('git clone %s Nexus' % repo)
  repo = git.Repo('Nexus')
  commits = sorted([commit for commit in repo.iter_commits('master')],
    key = lambda commit: -int(commit.committed_date))
  builds = [int(d) for d in os.listdir(path) if os.path.isdir(
    os.path.join(path, d))]
  builds.sort(reverse=True)
  if len(builds) == 0:
    builds.append(int(commits[1].committed_date))
  for i in range(len(commits)):
    commit = commits[i]
    timestamp = int(commit.committed_date)
    if timestamp in builds:
      commits = commits[0:i]
      commits.reverse()
      break
  for commit in commits:
    timestamp = int(commit.committed_date)
    call('git checkout %s' % commit.hexsha, os.path.join(os.getcwd(), 'Nexus'))
    result = []
    if sys.platform == 'win32':
      extension = 'bat'
    else:
      extension = 'sh'
    '''
    result.append(call(os.path.join(os.getcwd(), 'Nexus',
      'configure.%s' % extension), os.path.join(os.getcwd(), 'Nexus')))
    result.append(call(os.path.join(os.getcwd(), 'Nexus',
      'build.%s' % extension), os.path.join(os.getcwd(), 'Nexus')))
    '''
    terminal_output = b''
    for output in result:
      terminal_output += output[0] + b'\n\n\n\n'
    for output in result:
      terminal_output += output[1] + b'\n\n\n\n'
    os.makedirs(os.path.join(path, str(timestamp)))
    nexus_applications = ['AdministrationServer', 'AsxItchMarketDataFeedClient',
      'ChartingServer', 'ChiaMarketDataFeedClient', 'ComplianceServer',
      'CseMarketDataFeedClient', 'CtaMarketDataFeedClient', 'DefinitionsServer',
      'MarketDataRelayServer', 'MarketDataServer', 'RiskServer',
      'SimulationMarketDataFeedClient', 'SimulationOrderExecutionServer',
      'TmxIpMarketDataFeedClient', 'TmxTl1MarketDataFeedClient',
      'UtpMarketDataFeedClient', 'WebPortal']
    copy_build(nexus_applications, timestamp,
      os.path.join(os.getcwd(), 'Nexus'), path)
    beam_applications = ['AdminClient', 'RegistryServer', 'ServiceLocator',
      'UidServer']
    copy_build(beam_applications, timestamp,
      os.path.join(os.getcwd(), 'Nexus', 'Dependencies', 'Beam'), path)
    '''
    user_call('cp ./Nexus/Applications/*.sh ./%s/' % str(timestamp))
    user_call('cp ./Nexus/Applications/*.sql ./%s/' % str(timestamp))
    user_call('cp ./Nexus/Applications/*.py ./%s/' % str(timestamp))
    user_call('mv Nexus Nexus_backup')
    user_call('mv ./%s Nexus' % str(timestamp))
    user_call('tar -czf nexus-%s.tar.gz Nexus' % str(timestamp))
    user_call('touch ./Nexus/build.txt')
    log_file = open('./Nexus/build.txt', 'wb')
    log_file.write(terminal_output)
    log_file.close()
    destination = os.path.join(path, str(timestamp))
    user_call('mkdir %s' % destination)
    user_call('mv nexus-%s.tar.gz %s' % (str(timestamp), destination))
    user_call('mv ./Nexus/build.txt %s' % destination)
    user_call('rm -rf ./Nexus')
    user_call('mv Nexus_backup Nexus')
    '''

def main():
  parser = argparse.ArgumentParser(
    description='v1.0 Copyright (C) 2019 Eidolon Systems Ltd.')
  parser.add_argument('-r', '--repo', type=str, help='Remote repository.',
    default='https://github.com/eidolonsystems/nexus.git')
  parser.add_argument('-p', '--path', type=str, help='Destination path.',
    required=True)
  parser.add_argument('-t', '--period', type=int, help='Time period.',
    default=600)
  args = parser.parse_args()
  while True:
    build_repo(args.repo, args.path)
    time.sleep(args.period)

if __name__ == '__main__':
  main()
