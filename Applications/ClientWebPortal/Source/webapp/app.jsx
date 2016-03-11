requirejs.config({
  baseUrl: 'third-party',
  map: {
    '*': {
      'css': 'css'
    }
  },
  paths: {
    app: '..'
  }
});

requirejs(['app/login']);
