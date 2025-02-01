<?php
$filename = "../server/cmake-build-debug/results/ping_results.json";

if (!file_exists($filename)) {
    echo json_encode(["error" => "No results found"]);
    exit;
}

$data = file_get_contents($filename);
$pingResults = json_decode($data, true);

echo "<h1>Ping Results</h1>";
echo "<table border='1'>";
echo "<tr><th>Name</th><th>IP</th><th>Latency (ms)</th></tr>";

foreach ($pingResults as $host) {
    echo "<tr>";
    echo "<td>" . htmlspecialchars($host['name']) . "</td>";
    echo "<td>" . htmlspecialchars($host['ip']) . "</td>";
    echo "<td>" . ($host['latency_ms'] !== null ? $host['latency_ms'] . " ms" : "Failed") . "</td>";
    echo "</tr>";
}

echo "</table>";
