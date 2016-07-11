var express = require('express');
var bodyParser = require('body-parser');

var app = express();

app.use(bodyParser.json());

app.post('/api/service_locator/login', function (req, res) {
  console.log(JSON.stringify(req.headers));
});

var port = 8081;
app.listen(port, function () {
  console.log('Example app listening on port ' + port);
});