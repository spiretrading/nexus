module.exports = function(grunt) {

  grunt.initConfig({
    clean: {
      transpiledTests: {
        src: ['spec/transpiled']
      }
    },
    babel: {
      options: {
        sourceMap: true,
        presets: ['es2015'],
        plugins: [
          ['module-resolver', {
            'root': ['./src']
          }]
        ]
      },
      appSrc: {
        files: [{
          expand: true,
          cwd: 'src',
          src: ['**/*.js'],
          dest: 'spec/transpiled/webapp'
        }]
      },
      tests: {
        files: [{
          expand: true,
          cwd: 'spec/src',
          src: ['**/*.js'],
          dest: 'spec/transpiled/test'
        }]
      }
    },
    exec: {
      jasmine: 'jasmine'
    }
  });

  grunt.loadNpmTasks('grunt-babel');
  grunt.loadNpmTasks('grunt-contrib-clean');
  grunt.loadNpmTasks('grunt-exec');

  grunt.registerTask('test', [
    'clean:transpiledTests',
    'babel:appSrc',
    'babel:tests',
    'exec:jasmine'
  ]);
};
