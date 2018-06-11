const MinifyPlugin = require("babel-minify-webpack-plugin");
const path = require('path');
const webpack = require('webpack');

const PROD = JSON.parse(process.env.PROD_ENV || '0');
const minifyOpts = {};
const minigyPluginOpts = {
  test: /\.js($|\?)/i,
};

module.exports = {
  devtool: PROD ? 'none' : 'source-map',
  entry: './source/index.ts',
  module: {
    rules: [
      {
        test: /\.ts$/,
        loader: 'ts-loader'
      },
      {
        test: /\.js$/,
        loader: 'source-map-loader',
        enforce: 'pre'
      }
    ]
  },
  output: {
    filename: 'index.js',
    library: 'Nexus',
    libraryTarget: 'umd',
    path: path.resolve(__dirname, 'library/nexus'),
    umdNamedDefine: true
  },
  plugins: PROD ? [new MinifyPlugin(minifyOpts, minigyPluginOpts)] : [],
  resolve: {
    extensions: ['.ts', '.js']
  }
};
