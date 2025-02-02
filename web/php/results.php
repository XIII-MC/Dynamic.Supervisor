<?php
$resultsFile = '/etc/dynamic/supervisor/results/ping_results.json';

if (file_exists($resultsFile)) {

    $results = json_decode(file_get_contents($resultsFile), true);

    if (!is_array($results)) {

        $results = [];

    }

    echo json_encode($results);

} else {

    echo json_encode([]);

}
