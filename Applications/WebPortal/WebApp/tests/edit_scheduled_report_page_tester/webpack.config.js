const path = require('node:path');
module.paths.push(path.resolve(process.cwd(), 'node_modules'));
let mode = 'development';
let devtool = 'source-map';
if(process.env.PROD_ENV == '1') {
  mode = 'production';
  devtool = false;
}
module.exports = {
  devtool,
  entry: path.resolve(process.cwd(), 'source/index.tsx'),
  ignoreWarnings: [
    {
      message: /Failed to parse source map/,
    }
  ],
  mode,
  module: {
    rules: [
      {
        test: /\.tsx?$/,
        loader: 'ts-loader'
      },
      {
        enforce: 'pre',
        test: /\.js$/,
        loader: 'source-map-loader'
      }
    ]
  },
  output: {
    path: path.resolve(process.cwd(), 'application'),
    filename: 'bundle.js'
  },
  performance: {
    hints: false
  },
  resolve: {
    alias: {
      react: path.resolve('node_modules/react'),
      'react-dom': path.resolve('node_modules/react-dom'),
      'react-router': path.resolve('node_modules/react-router'),
      'react-router-dom': path.resolve('node_modules/react-router-dom'),
    },
    symlinks: false,
    extensions: ['.ts', '.tsx', '.js', '.json']
  }
};
