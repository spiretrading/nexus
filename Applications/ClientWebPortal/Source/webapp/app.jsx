requirejs.config({
  baseUrl: 'third-party',
  paths: {
    app: '..'
  }
});

requirejs(['app/login']);
