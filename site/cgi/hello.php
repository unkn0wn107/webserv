<?php
echo "Content-Type: text/html\r\n\r\n";
echo "<html>";
echo "<head>";
echo "<title>Hello - CGI PHP Script</title>";
echo "</head>";
echo "<body>";
echo "<h2>Hello CGI</h2>";
echo "<h3>Environment Variables:</h3>";
echo "<pre>";
foreach ($_SERVER as $var_name => $var_value) {
    echo $var_name . " = " . $var_value . "\n";
}
echo "</pre>";

// Query string handling
$query_string = $_SERVER['QUERY_STRING'];
if (!empty($query_string)) {
    parse_str($query_string, $form_pairs);
    echo "<h3>Query Strings:</h3>";
    echo "<pre>";
    foreach ($form_pairs as $key => $value) {
        echo $key . " = " . $value . "\n";
    }
    echo "</pre>";

    if (isset($form_pairs['name'])) {
        $name = $form_pairs['name'];
        echo "<h3>Hello " . htmlspecialchars($name) . "!</h3>";
    } else {
        echo "<h3>btw what's your name ?!</h3>";
    }
} else {
    echo "<h3>Error: no data received as query string!</h3>";
}

echo "</body>";
echo "</html>";
echo "\r\n";
?>
