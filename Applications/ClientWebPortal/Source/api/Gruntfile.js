module.exports = function(grunt) {

  grunt.initConfig({
    clean: ['dist'],
    babel: {
      options: {
        sourceMap: true,
        presets: ['es2015']
      },
      files: {
        expand: true,
        cwd: 'src',
        src: ['**/*.js'],
        dest: 'dist'
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

  grunt.registerTask('default', ['clean', 'babel']);
  grunt.registerTask('dev', ['clean', 'babel', 'watch']);
};
