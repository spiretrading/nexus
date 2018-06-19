var path = require('path');
var webpack = require('webpack');
const MinifyPlugin = require("babel-minify-webpack-plugin");
const PROD = JSON.parse(process.env.PROD_ENV || '0');
const minifyOpts = {};
const minigyPluginOpts = {
  test: /\.js($|\?)/i,
};
module.exports = {
  entry: './source/index.ts',
  output: {
    filename: 'index.js',
    library: 'web_portal',
    libraryTarget: 'umd',
    path: path.resolve(__dirname, 'library/web_portal'),
    umdNamedDefine: true
  },
  resolve: {
    extensions: ['.ts', '.tsx', '.js', '.json']
  },
  devtool: PROD ? 'none' : 'source-map',
  plugins: PROD ? [new MinifyPlugin(minifyOpts, minigyPluginOpts)] : [],
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
};
