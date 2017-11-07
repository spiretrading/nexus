module.exports = function(grunt) {

  grunt.initConfig({
    clean: {
      dist: {
        src: ['dist']
      },
      tempDist: {
        src: ['temp-dist']
      },
      transpiledTests: {
        src: ['spec/transpiled']
      }
    },
    babel: {
      options: {
        sourceMap: true,
        presets: ['es2015']
      },
      dev: {
        files: [{
          expand: true,
          cwd: 'src',
          src: ['**/*.js'],
          dest: 'dist'
        }]
      },
      tests: {
        files: [{
          expand: true,
          cwd: 'spec/src',
          src: ['**/*.js'],
          dest: 'spec/transpiled'
        }]
      },
      prod: {
        files: [{
          expand: true,
          cwd: 'src',
          src: ['**/*.js'],
          dest: 'temp-dist'
        }]
      }
    },
    uglify: {
      prod: {
        files: [{
          expand: true,
          cwd: 'temp-dist',
          src: ['**/*.js'],
          dest: 'dist'
        }]
      }
    },
    watch: {
      files: ['src/**/*.js'],
      tasks: ['babel']
    }
  });

  grunt.loadNpmTasks('grunt-babel');
  grunt.loadNpmTasks('grunt-contrib-watch');
  grunt.loadNpmTasks('grunt-contrib-clean');
  grunt.loadNpmTasks('grunt-contrib-uglify');

  grunt.registerTask('build-test', [
    'clean:transpiledTests',
    'babel:tests'
  ]);

  grunt.registerTask('build-dev', [
    'clean:dist',
    'clean:transpiledTests',
    'babel:dev',
    'babel:tests'
  ]);

  grunt.registerTask('update-dev', [
    'clean:dist',
    'babel:dev',
    'watch'
  ]);

  grunt.registerTask('build-prod', [
    'clean:dist',
    'babel:prod',
    'uglify:prod',
    'clean:tempDist'
  ]);
};
