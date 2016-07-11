var express = require('express');
var app = express();
var httpProxy = require('http-proxy');
var apiProxy = httpProxy.createProxyServer();
var localServer = 'http://localhost:8081';
var ubuntuServer = 'http://192.168.1.129:8080';

app.all('/api/*', function(req, res){
    console.log(JSON.stringify(req.headers));
    apiProxy.web(req, res, {
        target: ubuntuServer
    });
});
app.use(express.static('dist'));

var port = 8080;
app.listen(port, function () {
    console.log('Example app listening on port ' + port);
});