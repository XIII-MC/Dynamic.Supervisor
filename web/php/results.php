<?php
$resultsFile = '/etc/gteam/dynamic/supervisor/results/ping_results.json';
$cpuFile = '/etc/gteam/dynamic/supervisor/results/monitor_results.json';

$results = [];
if (file_exists($resultsFile)) {

    $results = json_decode(file_get_contents($resultsFile), true);

}

$cpuUsageByIP = [];
if (file_exists($cpuFile)) {

    $cpuData = json_decode(file_get_contents($cpuFile), true);

    foreach ($cpuData as $host) {

        if (isset($host['ip'], $host['response'])) {

            $response = json_decode($host['response'], true);
            if (isset($response['cpu_usage_percent'])) {

                $cpuUsageByIP[$host['ip']] = $response['cpu_usage_percent'];

            }

        }

    }

}

foreach ($results as &$host) {

    $host['cpu_usage'] = $cpuUsageByIP[$host['ip']] ?? 0;

}

header('Content-Type: application/json');
echo json_encode($results);