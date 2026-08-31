import argparse
import errno
import git
import os
import shutil
import signal
import subprocess
import sys
import tarfile
import time
import zipfile


def terminate(process):
  if sys.platform == 'win32':
    subprocess.call(['taskkill', '/F', '/T', '/PID', str(process.pid)],
      stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
  else:
    os.killpg(os.getpgid(process.pid), signal.SIGKILL)


def call(command, cwd, timeout):
  process = subprocess.Popen(
    command, stdout=subprocess.PIPE, cwd=cwd, stderr=subprocess.PIPE,
    start_new_session=True)
  try:
    output = process.communicate(timeout=timeout)
  except subprocess.TimeoutExpired:
    terminate(process)
    output = process.communicate()
    message = 'Timed out after %d seconds.' % timeout
    return (output[0], output[1] + message.encode('utf-8'), 1)
  return (output[0], output[1], process.returncode)


def makedirs(path):
  try:
    os.makedirs(path)
  except OSError as e:
    if e.errno != errno.EEXIST:
      raise


def make_tarfile(source, destination):
  with tarfile.open(destination, 'w:gz') as tar:
    for file in os.listdir(source):
      tar.add(os.path.join(source, file), arcname=file)


def make_zipfile(source, destination):
  archive = zipfile.ZipFile(destination, 'w', zipfile.ZIP_DEFLATED)
  source = os.path.abspath(source)
  for root, dirs, files in os.walk(source):
    for file in files:
      absolute_path = os.path.abspath(os.path.join(root, file))
      archive.write(os.path.join(root, file),
        arcname=absolute_path[len(source) + 1:])
  archive.close()


def copy_build(applications, version, name, source, path):
  errors = []
  destination_path = os.path.join(path, str(version))
  for application in applications:
    try:
      application_path = os.path.join(destination_path, application)
      makedirs(application_path)
      source_directory = os.path.join(source, 'Applications', application,
        'Application')
      for file in os.listdir(source_directory):
        file_path = os.path.join(source_directory, file)
        if os.path.isdir(file_path):
          shutil.copytree(file_path, os.path.join(application_path, file),
            dirs_exist_ok=True)
          continue
        if not os.path.isfile(file_path):
          continue
        extension = os.path.splitext(file_path)[1]
        if extension in ['.py', '.yml', '.csv']:
          shutil.copy2(file_path, os.path.join(application_path, file))
        if sys.platform == 'win32':
          if extension in ['.bat', '.exe']:
            shutil.copy2(file_path, os.path.join(application_path, file))
        else:
          if extension in ['', '.sh']:
            shutil.copy2(file_path, os.path.join(application_path, file))
    except OSError as e:
      errors.append('Failed to copy %s %s: %s' % (name, application, e))
  return errors


def clean_build(applications, source):
  if sys.platform == 'win32':
    executable_extension = '.exe'
  else:
    executable_extension = ''
  for application in applications:
    source_directory = os.path.join(source, 'Applications', application,
      'Application')
    if not os.path.isdir(source_directory):
      continue
    for file in os.listdir(source_directory):
      file_path = os.path.join(source_directory, file)
      if not os.path.isfile(file_path):
        continue
      if os.path.splitext(file_path)[1] == executable_extension:
        os.remove(file_path)


def python_libraries(repo_path):
  if sys.platform == 'win32':
    extension = '.pyd'
  else:
    extension = '.so'
  return [os.path.join(repo_path, 'Nexus', 'Dependencies', 'aspen',
    'Libraries', 'Release', 'aspen%s' % extension),
    os.path.join(repo_path, 'Nexus', 'Dependencies', 'Beam', 'Beam',
    'Libraries', 'Release', 'beam%s' % extension),
    os.path.join(repo_path, 'Nexus', 'Libraries', 'Release',
    'nexus%s' % extension)]


def copy_python_libraries(path, version, repo_path):
  python_path = os.path.join(path, str(version), 'Python')
  makedirs(python_path)
  for library in python_libraries(repo_path):
    if os.path.isfile(library):
      shutil.copy2(library, python_path)


def clean_python_libraries(repo_path):
  for library in python_libraries(repo_path):
    if os.path.isfile(library):
      os.remove(library)


def build_repo(repo, path, branch, timeout):
  commits = repo.git.rev_list('--first-parent', 'HEAD').split('\n')
  commits.reverse()
  builds = [int(d) for d in os.listdir(path)
    if d.isdecimal() and os.path.isdir(os.path.join(path, d))]
  builds.sort(reverse=True)
  latest = None
  if len(builds) != 0:
    for i in range(len(commits) - 1, -1, -1):
      version = int(repo.git.rev_list('--count', '--first-parent', commits[i]))
      if version in builds:
        latest = i
        break
  if latest is None:
    commits = [commits[-1]]
  else:
    commits = commits[latest + 1:]
  if sys.platform == 'win32':
    extension = 'bat'
  else:
    extension = 'sh'
  for commit in commits:
    version = int(repo.git.rev_list('--count', '--first-parent', commit))
    repo.git.checkout(commit)
    nexus_applications = ['AdministrationServer', 'ChartingServer',
      'ComplianceServer', 'DefinitionsServer', 'MarketDataRelayServer',
      'MarketDataServer', 'ReplayMarketDataFeedClient', 'RiskServer',
      'SimulationMarketDataFeedClient', 'SimulationOrderExecutionServer',
      'WebPortal']
    if sys.platform == 'win32':
      nexus_applications.append('Spire')
    beam_applications = ['AdminClient', 'ServiceLocator', 'UidServer']
    beam_path = os.path.join(repo.working_dir, 'Nexus', 'Dependencies', 'Beam')
    clean_build(nexus_applications, repo.working_dir)
    clean_build(beam_applications, beam_path)
    clean_python_libraries(repo.working_dir)
    result = []
    result.append(
      call([os.path.join(repo.working_dir, 'configure.%s' % extension)],
      repo.working_dir, timeout))
    if result[-1][2] == 0:
      result.append(
        call([os.path.join(repo.working_dir, 'build.%s' % extension)],
        repo.working_dir, timeout))
    terminal_output = b''
    for output in result:
      terminal_output += output[0] + b'\n\n\n\n'
    for output in result:
      terminal_output += output[1] + b'\n\n\n\n'
    destination_path = os.path.join(path, str(version))
    makedirs(destination_path)
    archive_path = None
    if result[-1][2] == 0:
      errors = copy_build(nexus_applications, version, 'Nexus',
        repo.working_dir, path)
      errors.extend(
        copy_build(beam_applications, version, 'Beam', beam_path, path))
      for error in errors:
        terminal_output += error.encode('utf-8') + b'\n'
      shutil.copy2(os.path.join(repo.working_dir, 'Applications', 'setup.py'),
        os.path.join(destination_path, 'setup.py'))
      shutil.copytree(os.path.join(repo.working_dir, 'Applications', 'Python'),
        os.path.join(destination_path, 'Python'), dirs_exist_ok=True)
      copy_python_libraries(path, version, repo.working_dir)
      if len(errors) == 0:
        if sys.platform == 'win32':
          for file in ['install_python.bat', 'setup.py']:
            shutil.copy2(os.path.join(repo.working_dir, 'Applications', file),
              os.path.join(destination_path, file))
          archive_path = os.path.join(path, 'nexus-%s.zip' % str(version))
          make_zipfile(destination_path, archive_path)
        else:
          for file in ['check.sh', 'install_python.sh', 'setup.py', 'start.sh',
              'stop.sh']:
            copy_path = os.path.join(destination_path, file)
            shutil.copy2(os.path.join(repo.working_dir, 'Applications', file),
              copy_path)
            if file.endswith('.sh'):
              with open(copy_path, 'r') as f:
                translation = f.read().replace('/Application', '')
              with open(copy_path, 'w') as f:
                f.write(translation)
          archive_path = os.path.join(path, 'nexus-%s.tar.gz' % str(version))
          make_tarfile(destination_path, archive_path)
      shutil.rmtree(destination_path)
      makedirs(destination_path)
    with open(os.path.join(destination_path, 'build.txt'), 'wb') as log_file:
      log_file.write(terminal_output)
    if archive_path is not None:
      shutil.move(archive_path, destination_path)


def main():
  parser = argparse.ArgumentParser(
    description='v1.0 Copyright (C) 2020 Spire Trading Inc.')
  parser.add_argument('-p', '--path', type=str, help='Destination path.',
    required=True)
  parser.add_argument('-t', '--period', type=int, help='Time period.',
    default=600)
  parser.add_argument('--timeout', type=int,
    help='Build timeout in seconds.', default=14400)
  args = parser.parse_args()
  makedirs(args.path)
  repo = git.Repo('.')
  try:
    branch = repo.active_branch.name
  except TypeError:
    sys.exit('HEAD is detached, check out the branch to build first.')
  while True:
    try:
      repo.git.fetch()
      repo.git.checkout('--force', branch)
      repo.git.reset('--hard', '@{upstream}')
    except:
      print('Failed to pull: ', sys.exc_info()[0])
    try:
      build_repo(repo, args.path, branch, args.timeout)
    except Exception as e:
      print('Failed to build: %s' % e)
    try:
      repo.git.checkout(branch)
    except Exception as e:
      print('Failed to check out %s: %s' % (branch, e))
    time.sleep(args.period)


if __name__ == '__main__':
  main()
