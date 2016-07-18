const webpack = require('webpack');
const path = require('path');
const ExtractTextPlugin = require('extract-text-webpack-plugin');
const CleanWebpackPlugin = require('clean-webpack-plugin');
const CopyWebpackPlugin = require('copy-webpack-plugin');

const nodeEnv = process.env.NODE_ENV || 'development';
const isProd = nodeEnv === 'production';

// Loaders
const copyHTML = {
  test: /\.html$/,
  loader: 'file',
  query: {
    name: '[name].[ext]'
  }
};

const extractCSS = {
  test: /\.css$/,
  loader: ExtractTextPlugin.extract("style-loader", "css-loader")
};

const extractSASS = {
  test: /\.scss$/,
  loaders: ExtractTextPlugin.extract('style-loader', 'css-loader!sass-loader')
};

const transpileES6JSX = {
  test: /\.(js|jsx)$/,
  exclude: /node_modules/,
  loaders: [
    {
      loader: 'babel',
      query: {
        cacheDirectory: true
      }
    }
  ]
};

const copyStaticFiles = {
  test: /\.(gif|png|jpg|jpeg|ttf|eot|svg|woff(2)?)(\?[a-z0-9]+)?$/,
  loader: 'file'
};

module.exports = {
  devtool: isProd ? 'hidden-source-map' : 'cheap-eval-source-map',
  context: path.join(__dirname, './src'),
  entry: {
    js: [
      'index'
    ],
    vendor: [
      'react',
      'react-dom',
      'bootstrap'
    ]
  },
  output: {
    path: path.join(__dirname, './dist'),
    filename: 'bundle.js'
  },
  module: {
    loaders: [
      copyHTML,
      extractCSS,
      extractSASS,
      transpileES6JSX,
      copyStaticFiles
    ]
  },
  resolve: {
    extensions: ['', '.js', '.jsx'],
    modules: [
      path.resolve('./src'),
      'node_modules'
    ],
    alias: {
      'bootstrap-css': '../node_modules/bootstrap/dist/css/bootstrap.min.css'
    }
  },
  plugins: [
    new CopyWebpackPlugin([
      {from: '../resources/favicons'},
      {from: '../resources/images', to: 'images'},
      {from: '../resources/js/jquery/jquery-2.2.4.min.js', to: 'js' }
    ]),
    new webpack.ProvidePlugin({
      $: "jquery",
      jQuery: "jquery"
    }),
    new webpack.optimize.CommonsChunkPlugin({
      name: 'vendor',
      minChunks: Infinity,
      filename: 'vendor.bundle.js'
    }),
    new webpack.LoaderOptionsPlugin({
      minimize: true,
      debug: false
    }),
    new webpack.optimize.UglifyJsPlugin({
      compress: {
        warnings: false
      },
      output: {
        comments: false
      },
      sourceMap: false
    }),
    new webpack.DefinePlugin({
      'process.env': {NODE_ENV: JSON.stringify(nodeEnv)}
    }),
    new ExtractTextPlugin('style.css'),
    new CleanWebpackPlugin(['dist'], {
      verbose: true
    })
  ]
};
