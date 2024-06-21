const url = require('url');

// Read POST data from stdin if the request method is POST
let postData = '';
if (process.env.REQUEST_METHOD === 'POST') {
  process.stdin.setEncoding('utf8');
  process.stdin.on('readable', () => {
    const chunk = process.stdin.read();
    if (chunk !== null) {
      postData += chunk;
    }
  });
}

process.stdin.on('end', () => {
  // Parse the POST data into an object
  let data = {};
  if (postData) {
    try {
      data = JSON.parse(postData);
    } catch (error) {
      console.error('Invalid JSON data in POST request');
    }
  }

  // Generate the HTML response
  let response = '<html>';
  response += '<head>';
  response += '<title>Hello - CGI Node.js Script</title>';
  response += '</head>';
  response += '<body>';
  response += '<h2>Hello CGI</h2>';

  if (Object.keys(data).length > 0) {
    response += '<h3>Data:</h3>';
    response += '<pre>';
    for (let key in data) {
      response += `${key} = ${data[key]}<br>`;
    }
    response += '</pre>';

    if (data.name) {
      response += `<h3>Hello ${data.name}!</h3>`;
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
});

