const path = require('path');
module.paths.push(path.resolve(process.cwd(), 'node_modules'));
const MinifyPlugin = require("babel-minify-webpack-plugin");
const PROD = JSON.parse(process.env.PROD_ENV || '0');
const minifyOpts = {};
const minigyPluginOpts = {
  test: /\.js($|\?)/i,
};
module.exports = {
  devtool: PROD ? 'none' : 'source-map',
  entry: path.resolve(process.cwd(), 'source/index.tsx'),
  mode: PROD ? 'production' : 'development',
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
  plugins: PROD ? [new MinifyPlugin(minifyOpts, minigyPluginOpts)] : [],
  resolve: {
    symlinks: false,
    extensions: ['.ts', '.tsx', '.js', '.json']
  }
};
