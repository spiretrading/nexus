const { spawnSync } = require('node:child_process');
const crypto = require('node:crypto');
const fs = require('node:fs');
const path = require('node:path');

const root = fs.realpathSync(process.cwd());
const portal = fs.realpathSync(__dirname);
const stateFile = path.join(root, '.build_state.json');
const args = process.argv.slice(2);
const project = args.shift();
const command = args.shift();
let source = portal;
let action = 'release';
let dependenciesDirectory;
let state = { generated: [] };

function samePath(first, second) {
  if(!first || !second) {
    return false;
  }
  if(path.resolve(first) == path.resolve(second)) {
    return true;
  }
  const firstInfo = fs.statSync(first, { bigint: true, throwIfNoEntry: false });
  const secondInfo = fs.statSync(
    second, { bigint: true, throwIfNoEntry: false });
  return Boolean(firstInfo && secondInfo) && firstInfo.dev == secondInfo.dev &&
    firstInfo.ino == secondInfo.ino;
}

function inspect(filename) {
  return fs.lstatSync(filename, { throwIfNoEntry: false });
}

function save() {
  fs.writeFileSync(stateFile, JSON.stringify(state, null, 2) + '\n');
}

function collect(directory) {
  const result = [];
  if(!inspect(directory)) {
    return result;
  }
  for(const entry of fs.readdirSync(directory, { withFileTypes: true })) {
    const filename = path.join(directory, entry.name);
    result.push(filename);
    if(entry.isDirectory()) {
      result.push(...collect(filename));
    }
  }
  return result.sort();
}

function checkPath(directory, filename) {
  if(!filename.startsWith(directory + path.sep)) {
    throw new Error(`Generated path is outside ${directory}: ${filename}`);
  }
  let parent = path.dirname(filename);
  while(!inspect(parent)) {
    parent = path.dirname(parent);
  }
  const target = path.relative(directory, fs.realpathSync(parent));
  if(target == '..' || target.startsWith('..' + path.sep) ||
      path.isAbsolute(target)) {
    throw new Error(`Generated path crosses a directory link: ${filename}`);
  }
}

function remove(directory, filenames) {
  for(const relative of [...filenames].sort().reverse()) {
    const filename = path.resolve(directory, relative);
    checkPath(directory, filename);
    const info = inspect(filename);
    if(!info) {
      continue;
    }
    if(info.isDirectory() && !info.isSymbolicLink()) {
      if(fs.readdirSync(filename).length == 0) {
        fs.rmdirSync(filename);
      }
    } else {
      fs.unlinkSync(filename);
    }
  }
}

function run(command, args, directory, environment) {
  const result = spawnSync(command, args, {
    cwd: directory, stdio: 'inherit', env: environment
  });
  if(result.error) {
    throw result.error;
  }
  if(result.status != 0) {
    throw new Error(`${command} failed in ${directory}.`);
  }
}

function invoke(project, command, directory, arguments) {
  fs.mkdirSync(directory, { recursive: true });
  run(process.execPath,
    [__filename, project, command, ...arguments], directory, process.env);
}

function npm(args) {
  let command = 'npm';
  if(process.platform == 'win32') {
    command = process.env.ComSpec || 'cmd.exe';
    args = ['/d', '/s', '/c', 'npm ' + args.join(' ')];
  }
  run(command, args, root, process.env);
}

function forward() {
  for(const name of ['build', 'configure']) {
    if(process.platform == 'win32') {
      const filename = path.join(root, name + '.bat');
      let arguments = '';
      if(project == 'application') {
        arguments = ' -D="' + source + '"';
      }
      const header =
        '@ECHO OFF\r\nSETLOCAL DisableDelayedExpansion\r\nSET ARGS=%*\r\n';
      const invocation = 'node "' + __filename + '" ' + project + ' ' + name +
        arguments;
      const previous = header + invocation +
        ' %ARGS:\\=/%\r\nEXIT /B %ERRORLEVEL%\r\n';
      const info = inspect(filename);
      if(!info || info.isFile() &&
          fs.readFileSync(filename, 'utf8') == previous) {
        fs.writeFileSync(filename, header +
          'IF DEFINED ARGS SET ARGS=%ARGS:\\=/%\r\n' +
          invocation + ' %ARGS%\r\nEXIT /B %ERRORLEVEL%\r\n');
      }
    } else {
      const filename = path.join(root, name + '.sh');
      if(!inspect(filename)) {
        const quote = value => "'" + value.replaceAll("'", "'\\''") + "'";
        let arguments = '';
        if(project == 'application') {
          arguments = ' -D ' + quote(source);
        }
        fs.writeFileSync(filename, '#!/bin/bash\nexec node ' +
          quote(__filename) + ' ' + project + ' ' + name + arguments +
          ' "$@"\n', { mode: 0o755 });
      }
    }
  }
}

function copy(name) {
  const filename = path.join(root, name);
  const target = path.join(source, name);
  const content = fs.readFileSync(target);
  const info = inspect(filename);
  if(info) {
    if(samePath(fs.realpathSync(filename), fs.realpathSync(target))) {
      return;
    }
    if(info.isFile() && fs.readFileSync(filename).equals(content)) {
      return;
    }
    if(!info.isFile() || !state.generated.includes(name)) {
      throw new Error(`Refusing to overwrite the existing ${filename}.`);
    }
  } else {
    state.generated.push(name);
    save();
  }
  fs.writeFileSync(filename, content);
}

function configuration() {
  const files = ['package.json', 'package-lock.json', 'tsconfig.json'];
  if(project == 'application') {
    files.push('webpack.config.js');
  }
  return files;
}

function configureFiles() {
  if(samePath(root, source)) {
    return;
  }
  const filename = path.join(root, 'source');
  const target = path.join(source, 'source');
  if(inspect(filename)) {
    if(!samePath(fs.realpathSync(filename), fs.realpathSync(target))) {
      throw new Error(`Refusing to replace the existing ${filename}.`);
    }
  } else {
    let type = 'dir';
    if(process.platform == 'win32') {
      type = 'junction';
    }
    fs.symlinkSync(target, filename, type);
    state.generated.push('source');
    save();
  }
  for(const name of configuration()) {
    copy(name);
  }
  if(inspect(path.join(source, 'LICENSE'))) {
    copy('LICENSE');
  }
  forward();
}

function setup(directory) {
  if(samePath(process.env.WEB_PORTAL_SETUP_DIRECTORY, directory)) {
    return;
  }
  const nexus = path.resolve(portal, '../../../WebApi/build.js');
  const repository = path.join(directory, 'dali');
  const commit = 'a8d51439fbb84e9be79edd43662b653cd304f0e6';
  if(!inspect(repository)) {
    run('git', ['clone', 'https://github.com/spiretrading/dali', 'dali'],
      directory, process.env);
    run('git', ['checkout', commit], repository, process.env);
  } else {
    if(!inspect(path.join(repository, '.git'))) {
      throw new Error(
        `The Dali dependency is not a Git checkout: ${repository}`);
    }
    const result = spawnSync('git',
      ['merge-base', '--is-ancestor', commit, 'HEAD'], {
        cwd: repository, stdio: 'ignore'
      });
    if(result.error) {
      throw result.error;
    }
    if(result.status != 0) {
      run('git', ['fetch', 'origin'], repository, process.env);
      run('git', ['checkout', commit], repository, process.env);
    }
  }
  const webApi = path.join(directory, 'WebApi');
  fs.mkdirSync(webApi, { recursive: true });
  run(process.execPath,
    [nexus, 'configure', '-DD', directory], webApi, process.env);
  process.env.WEB_PORTAL_SETUP_DIRECTORY = directory;
}

function configureLibrary(directory) {
  if(samePath(process.env.WEB_PORTAL_LIBRARY_DIRECTORY, directory)) {
    return;
  }
  invoke('library', 'configure', path.join(directory, 'library'),
    ['-DD', directory]);
  process.env.WEB_PORTAL_LIBRARY_DIRECTORY = directory;
}

function configureDependencies() {
  const filename = path.join(root, 'Dependencies');
  const requested = path.resolve(root,
    dependenciesDirectory || state.dependenciesDirectory || 'Dependencies');
  const info = inspect(filename);
  if(!samePath(requested, filename) && info && !info.isSymbolicLink()) {
    throw new Error(`Refusing to replace the existing ${filename}.`);
  }
  fs.mkdirSync(requested, { recursive: true });
  const target = fs.realpathSync(requested);
  if(!samePath(requested, filename)) {
    if(info && !samePath(fs.realpathSync(filename), target)) {
      fs.unlinkSync(filename);
    }
    if(!inspect(filename)) {
      let type = 'dir';
      if(process.platform == 'win32') {
        type = 'junction';
      }
      fs.symlinkSync(target, filename, type);
    }
  }
  state.dependenciesDirectory = target;
  save();
  setup(target);
  if(project == 'application') {
    configureLibrary(target);
  }
}

function configure() {
  if(project != 'dali') {
    configureDependencies();
  }
  configureFiles();
}

function hash(filenames) {
  const digest = crypto.createHash('sha256');
  for(const filename of filenames.sort()) {
    if(!fs.statSync(filename).isFile()) {
      continue;
    }
    digest.update(filename + '\0');
    digest.update(crypto.createHash('sha256').update(
      fs.readFileSync(filename)).digest());
  }
  return digest.digest('hex');
}

function buildDependencies() {
  if(project == 'dali') {
    return [];
  }
  const directory = state.dependenciesDirectory;
  if(!samePath(process.env.WEB_PORTAL_BUILD_DIRECTORY, directory)) {
    if(project == 'library') {
      invoke('dali', 'build', path.join(directory, 'dali'), []);
      const nexus = path.resolve(portal, '../../../WebApi/build.js');
      run(process.execPath, [nexus, 'build', '-DD', directory],
        path.join(directory, 'WebApi'), process.env);
    } else {
      invoke('library', 'build', path.join(directory, 'library'),
        ['-DD', directory]);
    }
  }
  const packages = ['Beam/WebApi', 'dali', 'WebApi'];
  if(project == 'application') {
    packages.push('library');
  }
  return packages.map(name => path.join(directory, name));
}

function outputName() {
  if(project == 'application') {
    return 'application';
  }
  return 'library';
}

function outputs() {
  const directory = path.join(root, outputName());
  const info = inspect(directory);
  if(!info) {
    return [];
  }
  if(info.isSymbolicLink() || !info.isDirectory()) {
    throw new Error(`The output path must be a directory: ${directory}`);
  }
  return [directory, ...collect(directory)].map(filename =>
    path.relative(root, filename));
}

function copyOutput(from, to, directory) {
  checkPath(directory, to);
  const info = inspect(to);
  if(info && (info.isSymbolicLink() || !info.isFile())) {
    throw new Error(`Refusing to overwrite the existing ${to}.`);
  }
  fs.mkdirSync(path.dirname(to), { recursive: true });
  fs.copyFileSync(from, to);
}

function resources() {
  const directory = path.join(portal, 'resources');
  for(const filename of collect(directory)) {
    if(fs.statSync(filename).isFile()) {
      copyOutput(filename, path.join(root, 'application/resources',
        path.relative(directory, filename)), root);
    }
  }
  copyOutput(path.join(source, 'source/index.html'),
    path.join(root, 'application/index.html'), root);
}

function deploy() {
  const destination = path.resolve(root, '../../Application/web_app');
  if(project != 'application' ||
      !samePath(source, path.join(portal, 'application')) ||
      !inspect(path.dirname(destination))) {
    return;
  }
  fs.mkdirSync(destination, { recursive: true });
  const directory = fs.realpathSync(destination);
  let previous = [];
  if(state.deployment && samePath(state.deployment.directory, directory)) {
    previous = state.deployment.files;
  }
  const output = path.join(root, 'application');
  const files = collect(output);
  const names = files.map(filename => path.relative(output, filename));
  const owned = names.filter(filename => previous.includes(filename) ||
    !inspect(path.join(directory, filename)));
  const updates = [];
  for(const filename of files) {
    if(!fs.statSync(filename).isFile()) {
      continue;
    }
    const target = path.join(directory, path.relative(output, filename));
    checkPath(directory, target);
    const info = inspect(target);
    if(info && (info.isSymbolicLink() || !info.isFile())) {
      throw new Error(`Refusing to overwrite the existing ${target}.`);
    }
    if(!info || !fs.readFileSync(filename).equals(fs.readFileSync(target))) {
      updates.push(path.relative(output, filename));
    }
  }
  const obsolete = previous.filter(filename => !names.includes(filename));
  if(updates.length == 0 && obsolete.length == 0) {
    return;
  }
  let staging;
  try {
    const tracked = [...previous, ...owned];
    if(updates.length != 0) {
      staging = fs.mkdtempSync(path.join(directory, '.deploy-'));
      const relative = path.relative(directory, staging);
      tracked.push(relative, ...names.map(name => path.join(relative, name)));
    }
    state.deployment = { directory, files: [...new Set(tracked)] };
    save();
    for(const name of updates) {
      copyOutput(path.join(output, name), path.join(staging, name), staging);
    }
    for(const name of updates) {
      const target = path.join(directory, name);
      fs.mkdirSync(path.dirname(target), { recursive: true });
      fs.renameSync(path.join(staging, name), target);
    }
    remove(directory, obsolete);
  } finally {
    if(staging) {
      fs.rmSync(staging, { recursive: true, force: true });
    }
  }
  state.deployment = { directory, files: owned };
  save();
}

function build() {
  configure();
  const dependencies = buildDependencies();
  const manifest = JSON.parse(fs.readFileSync(path.join(root, 'package.json')));
  const packages = {
    ...manifest.dependencies, ...manifest.devDependencies,
    ...manifest.peerDependencies
  };
  const dependencyHash = hash([
    path.join(source, 'package.json'), path.join(source, 'package-lock.json'),
    ...dependencies.map(directory => path.join(directory, 'package.json'))
  ]);
  const installed = Object.entries(packages).every(([name, version]) => {
    const filename = path.join(root, 'node_modules', name);
    if(!fs.existsSync(path.join(filename, 'package.json'))) {
      return false;
    }
    return !version.startsWith('file:') ||
      samePath(fs.realpathSync(filename),
        fs.realpathSync(path.resolve(root, version.slice(5))));
  });
  if(state.dependencies != dependencyHash || !installed ||
      !inspect(path.join(root, 'node_modules/.package-lock.json'))) {
    delete state.dependencies;
    delete state.build;
    save();
    npm(['ci', '--include=dev']);
    state.dependencies = dependencyHash;
    save();
  }
  const inputs = [__filename, ...configuration().map(name =>
    path.join(source, name)), ...collect(path.join(source, 'source'))];
  inputs.push(...dependencies.map(directory =>
    path.join(directory, '.build_state.json')));
  if(project == 'application') {
    inputs.push(...collect(path.join(portal, 'resources')));
  }
  let current = hash(inputs);
  if(project == 'application') {
    current += ':' + action;
  }
  if(state.build == current && state.outputs && state.outputs.length != 0 &&
      state.outputs.every(filename => inspect(path.join(root, filename)))) {
    deploy();
    if(!inspect(path.join(root, 'mod_time.txt'))) {
      fs.writeFileSync(path.join(root, 'mod_time.txt'), current + '\n');
    }
    return;
  }
  delete state.build;
  save();
  fs.rmSync(path.join(root, 'mod_time.txt'), { force: true });
  const generated = state.generated.filter(filename =>
    filename == outputName() || filename.startsWith(outputName() + path.sep));
  remove(root, generated);
  state.snapshot = outputs();
  save();
  try {
    if(project == 'application') {
      let production = '1';
      if(action == 'debug') {
        production = '0';
      }
      run(process.execPath, ['node_modules/webpack/bin/webpack.js'], root,
        { ...process.env, PROD_ENV: production });
      resources();
    } else {
      npm(['run', 'build']);
    }
    state.outputs = outputs();
    deploy();
    fs.writeFileSync(path.join(root, 'mod_time.txt'), current + '\n');
    state.build = current;
  } finally {
    const previous = new Set(state.snapshot);
    state.generated = [...new Set([...state.generated,
      ...outputs().filter(filename => !previous.has(filename))])].filter(
        filename => inspect(path.join(root, filename)));
    delete state.snapshot;
    save();
  }
}

function clean() {
  remove(root, state.generated);
  const destination = path.resolve(root, '../../Application/web_app');
  if(state.deployment && inspect(destination) &&
      samePath(fs.realpathSync(destination), state.deployment.directory)) {
    remove(state.deployment.directory, state.deployment.files);
  }
  if(action == 'reset') {
    for(const name of ['node_modules', 'Dependencies']) {
      const filename = path.join(root, name);
      const info = inspect(filename);
      if(info && info.isSymbolicLink()) {
        fs.unlinkSync(filename);
      } else {
        fs.rmSync(filename, { recursive: true, force: true });
      }
    }
  }
  for(const name of ['mod_time.txt', '.build_hash.txt', '.build_state.json']) {
    fs.rmSync(path.join(root, name), { force: true });
  }
}

function projects() {
  if(project == 'portal') {
    return ['library', 'application', 'tests'];
  }
  return fs.readdirSync(source, { withFileTypes: true }).filter(entry =>
    entry.isDirectory() && inspect(path.join(source, entry.name,
      'package.json')) && inspect(path.join(source, entry.name,
      'webpack.config.js'))).map(entry => entry.name).sort();
}

function orchestrate() {
  forward();
  const cleaning = command == 'build' && ['clean', 'reset'].includes(action);
  let dependencies = dependenciesDirectory || state.dependenciesDirectory;
  if(!dependencies) {
    dependencies = path.resolve(root, '../library/Dependencies');
    if(project == 'portal') {
      dependencies = path.join(root, 'library/Dependencies');
    }
  }
  if(!cleaning) {
    fs.mkdirSync(dependencies, { recursive: true });
    dependencies = fs.realpathSync(dependencies);
    state.dependenciesDirectory = dependencies;
    save();
    setup(dependencies);
    configureLibrary(dependencies);
    if(command == 'build' &&
        !samePath(process.env.WEB_PORTAL_BUILD_DIRECTORY, dependencies)) {
      invoke('library', 'build', path.join(dependencies, 'library'),
        ['-DD', dependencies]);
      process.env.WEB_PORTAL_BUILD_DIRECTORY = dependencies;
    }
  }
  const children = projects();
  if(cleaning) {
    children.reverse();
  }
  for(const name of children) {
    let child = name;
    const arguments = [action, '-DD', dependencies];
    if(project == 'tests') {
      child = 'application';
      arguments.push('-D', path.join(source, name));
    }
    const directory = path.join(root, name);
    if(project == 'portal' && child == 'application' &&
        command == 'build' && !cleaning) {
      fs.mkdirSync(path.resolve(root, '../Application'), { recursive: true });
    }
    if(!cleaning || inspect(directory)) {
      invoke(child, command, directory, arguments);
    }
  }
  if(cleaning) {
    clean();
  }
}

function main() {
  if(!['portal', 'library', 'application', 'tests', 'dali'].includes(project) ||
      !['build', 'configure', 'setup'].includes(command)) {
    throw new Error('Invalid project or command.');
  }
  if(project == 'dali') {
    source = root;
  } else if(project != 'portal') {
    source = path.join(portal, project);
  }
  for(let i = 0; i < args.length; ++i) {
    const argument = args[i];
    const option = argument.split('=')[0];
    if(option == '-D' || option == '-DD') {
      let value = argument.slice(option.length + 1);
      if(argument == option) {
        value = args[++i];
      }
      if(!value || value.startsWith('-')) {
        throw new Error(`${option} requires a path argument.`);
      }
      if(option == '-DD') {
        dependenciesDirectory = path.resolve(root, value);
      } else if(project == 'application') {
        source = fs.realpathSync(path.resolve(root, value));
      } else {
        throw new Error('-D is only supported for application builds.');
      }
    } else if(['release', 'debug', 'relwithdebinfo', 'minsizerel',
        'clean', 'reset'].includes(argument.toLowerCase())) {
      action = argument.toLowerCase();
    } else {
      throw new Error(`Invalid build argument: ${argument}`);
    }
  }
  if(command == 'setup') {
    setup(root);
    if(project == 'application' || project == 'tests') {
      configureLibrary(root);
    }
    return;
  }
  if(inspect(stateFile)) {
    state = JSON.parse(fs.readFileSync(stateFile, 'utf8'));
    delete state.snapshot;
  }
  if(project == 'portal' || project == 'tests') {
    orchestrate();
  } else if(command == 'configure') {
    configure();
  } else if(action == 'clean' || action == 'reset') {
    clean();
  } else {
    build();
  }
}

try {
  main();
} catch(error) {
  console.error(error.message);
  process.exitCode = 1;
}
