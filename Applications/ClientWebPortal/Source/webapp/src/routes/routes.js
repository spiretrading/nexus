class CommonView {
  errorLoading(err) {
    console.error('Dynamic page loading failed', err);
  }

  loadRoute(cb) {
    return (module) => cb(null, module.default);
  }
}

export default CommonView
