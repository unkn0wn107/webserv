<!DOCTYPE html>
<html>
    <head>
        <title>PHP Test</title>
    </head>
    <body>
    <h1>Données reçues via POST</h1>
    <?php
    if ($_SERVER['REQUEST_METHOD'] === 'POST') {
        // Tenter de récupérer les données JSON
        $content = file_get_contents("php://input");
        $decoded = json_decode($content, true);

        if (json_last_error() === JSON_ERROR_NONE) {
            echo "<h2>Données JSON reçues :</h2>";
            echo "<pre>";
            print_r($decoded);
            echo "</pre>";
        } else {
            echo "<h2>Données POST traditionnelles :</h2>";
            echo "<pre>";
            print_r($_POST);
            echo "</pre>";
        }
    } else {
        echo "<p>Aucune donnée reçue. Veuillez utiliser le formulaire ci-dessous ou envoyer des données JSON.</p>";
    }
    ?>
    </body>
</html>
