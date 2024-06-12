<!DOCTYPE html>
<html>
    <head>
        <title>PHP Test</title>
    </head>
    <body>
        <?php echo '<p>Hello World</p>'; ?>
        <?php
        if ($_SERVER['REQUEST_METHOD'] === 'POST') {
            echo '<h3>POST Data:</h3>';
            echo '<pre>';
            $json = file_get_contents("php://input");
            $data = json_decode($json, true);
            print_r($data);
            echo '</pre>';
        }
        ?>
    </body>
</html>
