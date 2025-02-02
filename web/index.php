<?php
$hostsFile = 'hosts.json';
$resultsFile = 'ping_results.json';

// Load existing hosts
$hosts = file_exists($hostsFile) ? json_decode(file_get_contents($hostsFile), true) : [];

// Handle Add/Remove/Update actions
if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    if (isset($_POST['name']) && isset($_POST['ip'])) {
        // Add a new host
        $hosts[] = ["name" => $_POST['name'], "ip" => $_POST['ip']];
    } elseif (isset($_POST['remove'])) {
        // Remove host by index
        $index = $_POST['remove'];
        if (isset($hosts[$index])) {
            array_splice($hosts, $index, 1);
        }
    } elseif (isset($_POST['edit']) && isset($_POST['edit_name']) && isset($_POST['edit_ip'])) {
        // Edit host name and IP address
        $index = $_POST['edit'];
        if (isset($hosts[$index])) {
            $hosts[$index]['name'] = $_POST['edit_name'];
            $hosts[$index]['ip'] = $_POST['edit_ip'];
        }
    }

    // Save updated hosts list
    file_put_contents($hostsFile, json_encode($hosts, JSON_PRETTY_PRINT));

    // Redirect to avoid form resubmission issues
    header("Location: index.php");
    exit;
}

// Load ping results
$results = file_exists($resultsFile) ? json_decode(file_get_contents($resultsFile), true) : [];
?>

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Ping Latency Monitor</title>
    <style>
        body { font-family: Arial, sans-serif; text-align: center; }
        table { width: 75%; margin: auto; border-collapse: collapse; }
        th, td { border: 1px solid black; padding: 10px; }

        /* Add color classes */
        .latency-low { background-color: green; color: white; }
        .latency-medium { background-color: orange; color: white; }
        .latency-high { background-color: red; color: white; }
        .latency-loading { background-color: blue; color: white; } /* Blue for loading */

        /* Add striped row effect */
        table tr:nth-child(even) {
            background-color: #f2f2f2; /* Light grey for even rows */
        }

        table tr:nth-child(odd) {
            background-color: white; /* White for odd rows */
        }
    </style>
</head>
<body>
<h2>Manage Hosts</h2>

<!-- Add New Host -->
<form method="post">
    <input type="text" name="name" placeholder="Host Name" required>
    <input type="text" name="ip" placeholder="IP Address" required>
    <button type="submit">Add Host</button>
</form>

<!-- Display Existing Hosts -->
<table id="pingTable" border="1">
    <tr>
        <th>Name</th>
        <th>IP Address</th>
        <th>Actions</th>
        <th>Latency (ms)</th>
    </tr>
    <?php foreach ($hosts as $index => $host): ?>
        <tr data-host-id="<?= htmlspecialchars($host['ip']) ?>">
            <td><?= htmlspecialchars($host['name']) ?></td>
            <td><?= htmlspecialchars($host['ip']) ?></td>
            <td>
                <!-- Actions Container -->
                <div class="button-container">
                    <!-- Remove Button -->
                    <form method="post" style="display:inline;">
                        <input type="hidden" name="remove" value="<?= $index ?>">
                        <button type="submit">Remove</button>
                    </form>

                    <!-- Edit Button -->
                    <form method="post" style="display:inline;">
                        <input type="hidden" name="edit" value="<?= $index ?>">
                        <input type="text" name="edit_name" value="<?= htmlspecialchars($host['name']) ?>" required>
                        <input type="text" name="edit_ip" value="<?= htmlspecialchars($host['ip']) ?>" required>
                        <button type="submit">Edit</button>
                    </form>
                </div>
            </td>
            <td id="latency-<?= htmlspecialchars($host['ip']) ?>" class="latency-unknown latency-loading">Loading...</td> <!-- Latency cell with blue for loading -->
        </tr>
    <?php endforeach; ?>
</table>

<script>
    // Refresh every 1 second
    setInterval(updateLatency, 1000);
    updateLatency();

    async function updateLatency() {
        try {
            const response = await fetch('ping_results.json');
            const data = await response.json();

            // Iterate through the ping results and update latency
            data.forEach((host) => {
                const hostIP = host.ip;
                const latency = host.latency_ms;
                const latencyCell = document.getElementById(`latency-${hostIP}`);

                if (latencyCell) {
                    if (latency === null || latency === -1) {
                        latencyCell.textContent = 'Timeout';
                        latencyCell.className = 'latency-high';  // Red for timeout
                    } else if (latency < 10) {
                        latencyCell.textContent = `${latency} ms`;
                        latencyCell.className = 'latency-low';  // Green for low latency
                    } else {
                        latencyCell.textContent = `${latency} ms`;
                        latencyCell.className = 'latency-medium';  // Orange/Yellow for medium latency
                    }

                    // If the latency is still "Loading..." (i.e., newly added hosts)
                    if (latency === undefined) {
                        latencyCell.className = 'latency-loading';  // Blue for loading
                    }
                }
            });
        } catch (error) {
            console.error("Error fetching ping results:", error);
        }
    }
</script>
</body>
</html>
