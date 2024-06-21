const url = require('url');
const querystring = require('querystring');

// Read the query string from the QUERY_STRING environment variable
const queryString = process.env.QUERY_STRING || '';
const queryData = querystring.parse(queryString);

// Generate the HTML response
let response = '<html>';
response += '<head>';
response += '<title>Hello - CGI Node.js Script</title>';
response += '</head>';
response += '<body>';
response += '<h2>Hello CGI</h2>';

response += '<h3>Environment Variables:</h3>';
response += '<pre>';
for (let key in process.env) {
  response += `${key} = ${process.env[key]}<br>`;
}
response += '</pre>';

if (Object.keys(queryData).length > 0) {
  response += '<h3>Data:</h3>';
  response += '<pre>';
  for (let key in queryData) {
    response += `${key} = ${queryData[key]}<br>`;
  }
  response += '</pre>';

  if (queryData.name) {
    response += `<h3>Hello ${queryData.name}!</h3>`;
  } else {
    response += '<h3>btw what\'s your name ?!</h3>';
  }
} else {
  response += '<h3>No data received!</h3>';
}

response += '</body>';
response += '</html>';

// Send the response to stdout
console.log('Content-Type: text/html\r\n\r\n');
console.log(response);

